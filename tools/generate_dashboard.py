#!/usr/bin/env python3
"""Render the active Windows snapshot. db.py update keeps every export consistent."""
import argparse
import json
import sys


def generate_html(data):
    compact = {key:data[key] for key in ('target','metadata','summary','functions','milestones','snapshot_id','generated_at')}
    # Never let a symbol, comment or evidence string terminate the data script.
    payload = json.dumps(compact,ensure_ascii=False).replace('<','\\u003c').replace('&','\\u0026')
    return TEMPLATE.replace('@@SNAPSHOT@@',data['snapshot_id']).replace('@@DATA@@',payload)


TEMPLATE = '''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="progress-snapshot" content="@@SNAPSHOT@@"><title>Ignition · Windows reconstruction</title>
<style>
:root{color-scheme:dark;--bg:#10161c;--panel:#18222b;--line:#2c3a46;--muted:#99aaba;--text:#edf4f8;--accent:#73e8c0}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:15px/1.5 system-ui,sans-serif}main{max-width:1580px;padding:32px;margin:auto}
header{display:flex;justify-content:space-between;align-items:start;gap:24px}h1{font-size:32px;margin:4px 0;font-weight:650;letter-spacing:-1px}h2{font-size:18px;margin:0 0 12px}
.eyebrow{color:var(--accent);font-size:11px;letter-spacing:2px;font-weight:700}.muted,small{color:var(--muted)}.identity{text-align:right;font-size:12px;max-width:530px;overflow-wrap:anywhere}
.notice{border-left:3px solid #dca65d;background:#231f1a;padding:12px 18px;margin:22px 0;color:#edd9ba;font-size:13px}
.cards{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;margin-bottom:24px}.card,.panel{background:var(--panel);border:1px solid var(--line);border-radius:12px}
.card{padding:18px}.card b{display:block;font-size:28px;letter-spacing:-1px}.card span{font-size:12px;color:var(--muted)}.card small{display:block;font-size:11px;margin-top:6px}
.controls{display:flex;gap:10px;align-items:center;flex-wrap:wrap;margin-bottom:16px}input,select,button{font:inherit;color:var(--text);background:#111a22;border:1px solid var(--line);border-radius:7px;padding:8px 12px}
input{min-width:220px;flex:1}button{cursor:pointer}button.active{border-color:var(--accent);color:var(--accent)}button:hover{background:#263743}
.workspace{display:grid;grid-template-columns:minmax(0,1fr) 330px;gap:18px}.map{position:relative;overflow:hidden}.map-head{display:flex;justify-content:space-between;gap:10px;padding:16px 18px;border-bottom:1px solid var(--line);font-size:12px}
canvas{width:100%;height:550px;display:block;cursor:pointer}.legend{display:flex;gap:16px;flex-wrap:wrap;padding:14px 18px;border-top:1px solid var(--line);font-size:11px;color:var(--muted)}.dot{width:8px;height:8px;display:inline-block;margin-right:6px;border-radius:2px}
aside{padding:20px;overflow-wrap:anywhere}dl{margin:0}dt{color:var(--muted);font-size:11px;margin-top:12px;text-transform:uppercase;letter-spacing:.8px}dd{margin:3px 0;font-size:13px}a{color:var(--accent)}code{font-size:12px}#selected-title{font-size:19px;line-height:1.3;margin-top:6px}
.evidence{padding:8px 10px;border-radius:7px;background:#111a22;margin:6px 0;font-size:12px}.evidence.pass{border-left:3px solid #73e8c0}.evidence.stale{border-left:3px solid #dca65d}.lower{display:grid;grid-template-columns:1fr 1fr;gap:18px;margin-top:18px}.lower .panel{padding:20px}
.milestone{display:flex;justify-content:space-between;border-top:1px solid var(--line);padding:9px 0;font-size:13px}.pill{color:#dca65d;font-size:11px}footer{margin-top:24px;color:var(--muted);font-size:11px;overflow-wrap:anywhere}
@media(max-width:1000px){main{padding:20px}.workspace{grid-template-columns:1fr}canvas{height:460px}.cards{grid-template-columns:repeat(2,1fr)}header{display:block}.identity{text-align:left;margin-top:12px}.lower{grid-template-columns:1fr}}
@media(max-width:520px){h1{font-size:26px}main{padding:14px}.cards{gap:8px}.card{padding:12px}.card b{font-size:23px}canvas{height:380px}.controls>*{max-width:100%}}
</style></head><body><main>
<header><div><div class="eyebrow">IGNITION / STANDARD WINDOWS RELEASE</div><h1>Reconstruction progress</h1><div class="muted">Every block is an inventoried routine extent. Bigger block = more original bytes.</div></div><div class="identity" id="identity"></div></header>
<div class="notice">Inventory coverage is incomplete. FPO candidates include unclassified library/runtime code and may contain data. These counts measure inventoried extents, not overall game completion.</div>
<section class="cards" id="cards"></section>
<div class="controls"><button data-view="functions" class="active">Functions</button><button data-view="modules">Modules</button><button data-view="memory">Address order</button>
<select id="class-filter" aria-label="Routine classification"><option value="all">All classifications</option><option value="game">Game</option><option value="crt">CRT / library</option><option value="thunk">Thunks</option><option value="unknown">Unclassified</option></select>
<select id="stage-filter" aria-label="Reconstruction stage"><option value="all">All stages</option><option value="reconstructed">C reconstructed</option><option value="analyzed">Analyzed</option><option value="named">Named</option><option value="unidentified">Unidentified</option></select>
<input id="search" placeholder="Find a routine, address or module…" aria-label="Search functions"><button id="reset">Reset</button></div>
<section class="workspace"><div class="panel map"><div class="map-head"><span id="map-title">FUNCTION MAP</span><span id="count"></span></div><canvas id="map" aria-label="Interactive byte-weighted Windows function treemap" tabindex="0"></canvas><div class="legend" id="legend"></div></div>
<aside class="panel"><div class="eyebrow">ROUTINE INSPECTOR</div><h2 id="selected-title">Select a block</h2><div id="detail" class="muted">Hover to inspect. Click to pin. Filter to C reconstructed to inspect the first recovered routine.</div></aside></section>
<section class="lower"><div class="panel"><h2>Evidence stays separate</h2><div id="validation"></div><p class="muted" style="font-size:12px">Only current passing runs count. Source, harness or artifact changes mark their evidence stale. Reconstructed C does not imply instruction equality or native gameplay parity.</p></div>
<div class="panel"><h2>Playable milestones</h2><div id="milestones"></div></div></section>
<footer id="footer"></footer></main>
<script id="progress-data" type="application/json">@@DATA@@</script>
<script>
'use strict';
const data=JSON.parse(document.getElementById('progress-data').textContent),summary=data.summary;
const colors={unidentified:'#36444f',named:'#c49b44',analyzed:'#2b929c',reconstructed:'#4a9ade'};
const labels={unidentified:'Unidentified',named:'Named',analyzed:'Analyzed',reconstructed:'C reconstructed'};
const $=id=>document.getElementById(id),bytes=n=>n<1024?n+' B':(n/1024).toFixed(1)+' KiB',addr=n=>'0x'+n.toString(16).toUpperCase().padStart(8,'0');
function textElement(tag,text,parent,cls){const e=document.createElement(tag);e.textContent=text;if(cls)e.className=cls;parent.appendChild(e);return e}
textElement('div',data.target.binary,$('identity'));textElement('div','Snapshot '+data.snapshot_id.slice(0,12)+' · '+data.generated_at,$('identity'));
textElement('small','SHA-256 '+data.target.sha256,$('identity'));
textElement('div','Validation compiler: '+(data.target.validation_compiler||'unknown')+' · Original compiler unconfirmed',$('identity'));
for(const [title,value,note] of [['Inventoried candidates',summary.functions.toLocaleString(),'FPO-seeded; coverage incomplete'],['C reconstructed',summary.stages.reconstructed,'of '+summary.functions+' inventoried candidates'],['Recovered extents',bytes(summary.reconstructed_extent_bytes),'of '+bytes(summary.recorded_extent_bytes)+' unique inventoried bytes'],['Emulation validated',summary.current_passes.emulation,'Current passing differential runs']]){
 const c=textElement('div','',$('cards'),'card');textElement('span',title,c);textElement('b',String(value),c);textElement('small',note,c);}
for(const stage of Object.keys(colors)){const e=textElement('span','',$('legend'));const dot=textElement('i','',e,'dot');dot.style.background=colors[stage];textElement('span',labels[stage],e);}
for(const [kind,label] of [['compilation','Compilation'],['raw_bytes','Raw code-byte equality'],['instructions','Relocation-aware instructions'],['linked','Linked binary equality'],['emulation','Differential emulation'],['native','Native routine runtime']]){
 const row=textElement('div','',$('validation'),'milestone');textElement('span',label,row);textElement('span',summary.current_passes[kind]+' routines with current passing evidence',row,'muted');}
for(const m of data.milestones){const row=textElement('div','',$('milestones'),'milestone');textElement('span',m.title,row);textElement('span',m.state,row,'pill');}
$('footer').textContent='Generated from active Windows SQLite · '+data.metadata.inventory_basis+' · Extent overlap: '+summary.overlap_bytes+' B · Unknown sizes: '+summary.unknown_sizes+' (minimum-area blocks) · Snapshot '+data.snapshot_id;
let view='functions',rects=[],hover=null,pinned=null;
const canvas=$('map'),ctx=canvas.getContext('2d');
function visible(){const q=$('search').value.toLowerCase(),cl=$('class-filter').value,stage=$('stage-filter').value;return data.functions.filter(f=>(cl==='all'||f.classification===cl)&&(stage==='all'||f.analysis_stage===stage)&&(!q||[f.symbol_name,f.module||'',addr(f.va),addr(f.rva)].some(v=>v.toLowerCase().includes(q))));}
function layout(items,x,y,w,h,out){if(!items.length)return;if(items.length===1){out.push({item:items[0],x,y,w,h});return;}let total=items.reduce((s,f)=>s+Math.max(1,f.byte_size||0),0),sum=0,best=1,delta=Infinity;
 for(let i=1;i<items.length;i++){sum+=Math.max(1,items[i-1].byte_size||0);const d=Math.abs(total/2-sum);if(d<delta){best=i;delta=d;}}
 const a=items.slice(0,best),b=items.slice(best),ratio=a.reduce((s,f)=>s+Math.max(1,f.byte_size||0),0)/total;
 if(w>=h){layout(a,x,y,w*ratio,h,out);layout(b,x+w*ratio,y,w*(1-ratio),h,out);}else{layout(a,x,y,w,h*ratio,out);layout(b,x,y+h*ratio,w,h*(1-ratio),out);}}
function render(){const box=canvas.getBoundingClientRect(),ratio=Math.min(devicePixelRatio||1,2);canvas.width=Math.round(box.width*ratio);canvas.height=Math.round(box.height*ratio);ctx.setTransform(ratio,0,0,ratio,0,0);ctx.clearRect(0,0,box.width,box.height);
 let items=visible();$('count').textContent=items.length+' candidates · '+bytes(items.reduce((s,f)=>s+(f.byte_size||0),0));$('map-title').textContent=view==='memory'?'ADDRESS-ORDERED EXTENTS':view==='modules'?'MODULE MAP':'FUNCTION MAP';rects=[];
 if(view==='modules'){const groups=new Map();for(const f of items){const key=f.module||'Unassigned';if(!groups.has(key))groups.set(key,[]);groups.get(key).push(f);}items=[...groups].map(([symbol_name,members])=>({symbol_name,members,byte_size:members.reduce((s,f)=>s+(f.byte_size||0),0),analysis_stage:members.every(f=>f.analysis_stage==='reconstructed')?'reconstructed':'unidentified'}));}
 if(view==='memory'){items.sort((a,b)=>a.rva-b.rva);const lanes=8,total=items.reduce((s,f)=>s+(f.byte_size||1),0),target=total/lanes,sets=[[]];let used=0;for(const f of items){if(used+(f.byte_size||1)>target&&sets.length<lanes&&sets.at(-1).length){sets.push([]);used=0;}sets.at(-1).push(f);used+=f.byte_size||1;}
 sets.forEach((set,lane)=>{const sum=set.reduce((s,f)=>s+(f.byte_size||1),0);let x=0;for(const item of set){const w=box.width*(item.byte_size||1)/sum;rects.push({item,x,y:lane*box.height/lanes,w,h:box.height/lanes-3});x+=w;}});
 }else{items.sort((a,b)=>(b.byte_size||0)-(a.byte_size||0));layout(items,0,0,box.width,box.height,rects);}
 draw();if(!rects.length){ctx.fillStyle='#99aaba';ctx.font='15px system-ui';ctx.fillText('No routines match these filters.',24,40);}}
function draw(){for(const r of rects){ctx.globalAlpha=hover&&hover!==r.item&&r.item.rva!==pinned?0.65:1;ctx.fillStyle=colors[r.item.analysis_stage]||colors.unidentified;ctx.fillRect(r.x,r.y,r.w,r.h);ctx.strokeStyle=r.item===hover||r.item.rva===pinned?'#edf4f8':'#10161c';ctx.lineWidth=r.item===hover||r.item.rva===pinned?2:0.7;ctx.strokeRect(r.x+.3,r.y+.3,Math.max(0,r.w-.6),Math.max(0,r.h-.6));if(r.w>90&&r.h>30){ctx.fillStyle='#d5e3ed';ctx.font='11px system-ui';ctx.save();ctx.beginPath();ctx.rect(r.x+4,r.y,r.w-8,r.h);ctx.clip();ctx.fillText(r.item.symbol_name,r.x+8,r.y+20);ctx.restore();}}ctx.globalAlpha=1;}
function detail(f){$('selected-title').textContent=f.symbol_name;$('detail').replaceChildren();if(f.members){textElement('div',f.members.length+' routines · '+bytes(f.byte_size),$('detail'));textElement('p','Double-click this block to filter to the module.', $('detail'));return;}
 const dl=document.createElement('dl');$('detail').appendChild(dl);for(const [label,value] of [['VA / RVA',addr(f.va)+' / '+addr(f.rva)],['Original extent',bytes(f.byte_size||0)],['Extent evidence',f.extent_origin+' · '+f.extent_confidence],['Classification',f.classification],['Reconstruction',labels[f.analysis_stage]],['Source',f.source_path||'Not reconstructed'],['ABI',f.abi||'Unknown'],['Evidence',f.evidence_path||'FPO metadata only']]){textElement('dt',label,dl);textElement('dd',value,dl);}
 textElement('dt','Validation',dl);for(const kind of ['compilation','raw_bytes','instructions','linked','emulation','native']){const r=f.latest_runs[kind];textElement('div',kind+': '+(r?(r.stale?'STALE · ':'')+r.outcome+(r.case_count?' · '+r.case_count+' cases':''):'not evaluated'),dl,'evidence '+(r?.stale?'stale':r?.outcome==='pass'?'pass':''));}
 if(f.imported_evidence)textElement('p','Historical milestone evidence preserved; current counts come from recorded runs.',$('detail'),'muted');}
function hit(e){const box=canvas.getBoundingClientRect(),x=e.clientX-box.left,y=e.clientY-box.top;return rects.find(r=>x>=r.x&&x<r.x+r.w&&y>=r.y&&y<r.y+r.h)?.item;}
canvas.addEventListener('pointermove',e=>{hover=hit(e);draw();if(hover&&pinned===null)detail(hover);});canvas.addEventListener('pointerleave',()=>{hover=null;draw();});canvas.addEventListener('click',e=>{const f=hit(e);if(f){pinned=f.rva??null;detail(f);draw();}});
canvas.addEventListener('dblclick',e=>{const f=hit(e);if(f?.members){$('search').value=f.symbol_name==='Unassigned'?'sub_':f.symbol_name;view='functions';document.querySelectorAll('[data-view]').forEach(b=>b.classList.toggle('active',b.dataset.view===view));render();}});
canvas.addEventListener('keydown',e=>{if(['ArrowLeft','ArrowRight','Enter'].includes(e.key)&&rects.length){e.preventDefault();let index=rects.findIndex(r=>r.item===hover);if(e.key!=='Enter')index=(index+(e.key==='ArrowRight'?1:-1)+rects.length)%rects.length;hover=rects[Math.max(0,index)].item;if(e.key==='Enter')pinned=hover.rva??null;detail(hover);draw();}});
for(const id of ['class-filter','stage-filter','search'])$(id).addEventListener('input',()=>{pinned=null;hover=null;render();});
document.querySelectorAll('[data-view]').forEach(b=>b.addEventListener('click',()=>{view=b.dataset.view;pinned=null;hover=null;document.querySelectorAll('[data-view]').forEach(a=>a.classList.toggle('active',a===b));render();}));
$('reset').addEventListener('click',()=>{$('search').value='';$('class-filter').value='all';$('stage-filter').value='all';pinned=null;hover=null;render();});
new ResizeObserver(render).observe(canvas.parentElement);render();
window.progressDashboard={data,visible,layout,render};
</script></body></html>
'''


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check',action='store_true')
    args=parser.parse_args(argv)
    from windows_tracking import export
    try:
        data=export(check=args.check)
        print(f"Windows dashboard snapshot {data['snapshot_id'][:12]} {'current' if args.check else 'generated'}.")
        return 0
    except (OSError,ValueError) as exc:
        print(str(exc),file=sys.stderr)
        return 1


if __name__=='__main__':
    sys.exit(main())
