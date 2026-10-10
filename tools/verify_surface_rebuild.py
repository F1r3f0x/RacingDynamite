# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Native display rebuild: independently recovered ownership/creation lifecycle."""
import copy
import hashlib
import random
import struct
import pefile
from verify_surface_open import State as OpenState, Session as OpenSession, FIELDS as OPEN_FIELDS, RECORDS, ARENA, SO, PO, DO, MODE, DRAW, PALETTE, WIDTH, HEIGHT, DEPTH, COUNT, WINDOW, LIMIT_TEXT, BACK_TEXT, MASK, LOST, Fault, signed, descriptor, native_surface, INPUTS as OPEN_INPUTS
from windows_target import ROOT,BUILD,verify_target
from build_decomp import build
from windows_tracking import record_run

if not __debug__:raise RuntimeError('Verification requires assertions')
ENTRIES=0x512850
FIELDS=OPEN_FIELDS+[('g_nativeRebuildPaletteEntries',ENTRIES,1024)]
DLL=BUILD/'surface_rebuild_validation.dll'
INPUTS=OPEN_INPUTS+['tools/verify_surface_rebuild.py']

class State(OpenState):
    def __init__(self,seed,mode=0,count=1):
        super().__init__(seed,mode,count)
        self.data[ENTRIES]=bytearray(random.Random(seed+ENTRIES).randbytes(1024))
    def snapshot(self):return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])
    def rebuild(self):
        p=RECORDS[0]
        if self.read(MODE)!=0:
            surface=self.read(p+44)
            if surface:
                self.method('Release',surface);self.write(p+44,0);self.write(p,0)
        for p in RECORDS[6:]:
            surface=self.read(p+44)
            if surface:
                self.method('Release',surface);self.write(p+44,0);self.write(p,0)
        if self.read(MODE)!=0 and self.read(PALETTE)!=0:
            self.method('Release',self.read(PALETTE));self.write(PALETTE,0)
        self.initialize()
        if self.read(MODE)!=0:
            depth=self.read(DEPTH);height=self.read(HEIGHT);width=self.read(WIDTH);draw=self.read(DRAW)
            if self.method('SetDisplayMode',draw,(width,height,depth))[0]:return 0
        desc=descriptor()
        if self.read(MODE)!=0:
            desc=descriptor(w4=0x21,w20=self.read(COUNT),w104=0x218);draw=self.read(DRAW)
            if self.method('CreateSurface',draw,(desc,RECORDS[0]+44,0))[0]:return 0
            depth=self.primary();bound=self.read(COUNT);self.write(RECORDS[0]+36,depth)
            if signed(bound)>=5:
                self.api('MessageBoxA',(self.read(WINDOW),LIMIT_TEXT,0,0));return 0
            if signed(self.read(COUNT))>0:
                chain=self.read(RECORDS[0]+44);index=0
                while True:
                    reply,chain=self.method('GetAttachedSurface',chain,(4 if index==0 else 16,'chain'))
                    if reply:
                        self.api('MessageBoxA',(self.read(WINDOW),BACK_TEXT,0,0));return 0
                    p=RECORDS[1+index];width=self.read(WIDTH);self.write(p+44,chain);height=self.read(HEIGHT)
                    self.write(p,1);depth=self.read(DEPTH);self.write(p+4,1)
                    self.write(p+28,width);self.write(p+32,height)
                    index+=1;bound=self.read(COUNT);self.write(p+36,depth)
                    if index>=signed(bound):break
        if self.read(MODE)!=0:
            if self.read(PALETTE)!=0:
                self.method('Release',self.read(PALETTE));self.write(PALETTE,0)
            for k in range(256):
                for channel in range(3):
                    b,o=self.region(ENTRIES+4*k+channel,1);value=0 if k==0 else 255;b[o]=value
                    self.events.append(('write',ENTRIES+4*k+channel,1,value))
            draw=self.read(DRAW)
            if self.method('CreatePalette',draw,(68,ENTRIES,PALETTE,0))[0]:return 0
            palette=self.read(PALETTE);surface=self.read(RECORDS[0]+44)
            if self.method('SetPalette',surface,(palette,))[0]:return 0
        self.restore();self.api('ShowWindow',(self.read(WINDOW),5));return 1
    def apply(self):
        self.events=[];self.fault=None;self.seen={};self.surface_number=0
        try:return self.rebuild()
        except Fault:return None

class Session(OpenSession):
    field_specs=FIELDS
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        self.symbols['Gfx_SurfaceOpenNative']=0x45bd70 if original else self.symbols['Gfx_SurfaceRebuildNative']


def inspect(pe):
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[0x5bd70]==(745,29,0,0x140e)
    assert hashlib.sha256(pe.get_data(0x5bd70,745)).hexdigest()=='9f9272e9f5f76e0318c38fc925e2bc4d03b304c169f0daf740b995b6ec055745'
    assert pe.get_data(0x5b6a6,4)==struct.pack('<I',0x50eb70)
    assert pe.get_data(0x5b6aa,4)==struct.pack('<I',0x45bd70)


def verify_surface_rebuild():
    original=pefile.PE(data=verify_target());inspect(original);build(dll=DLL);compiled=pefile.PE(str(DLL))
    coverage={'comparisons':0,'persistent_followups':0,'faults':0,'mutations':0}
    def sequence(state,repeats=1):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for repeat in range(repeats):
            policy=copy.deepcopy(state);result=state.apply()
            for session in sessions:session.run(state,result,coverage['comparisons'],copy.deepcopy(policy))
            coverage['comparisons']+=1;coverage['persistent_followups']+=int(repeat>0);coverage['faults']+=int(state.fault is not None)
            coverage['mutations']+=int(any(row[2] and index<state.seen.get(name,0) for (name,index),row in state.script.items()))
            if state.fault:break
    for mode in (0,1,2,MASK):
        for count in (0,1,2,4,5,MASK):sequence(State(coverage['comparisons'],mode,count),3)
    for mode in (0,1):
        for p in RECORDS:
            state=State(81,mode)
            for q in RECORDS:state.put(q+44,0)
            state.put(p+44,SO);sequence(state,2)
        for name in ('SetDisplayMode','CreateSurface','GetAttachedSurface','CreatePalette','SetPalette'):
            for reply in (1,0x80004005,LOST,MASK):
                state=State(82,mode,4);output=SO if name in ('CreateSurface','GetAttachedSurface') else PO if name=='CreatePalette' else None
                state.script[name,0]=(reply,output,[]);sequence(state,2)
    for occurrence in (0,1,6,20,21):
        for mode in (0,1):
            state=State(83,mode,4);state.script['Release',occurrence]=(MASK,None,[(MODE,1-mode),(PALETTE,PO),(COUNT,1),(WIDTH,320),(HEIGHT,240)])
            sequence(state,2)
    for mode in (0,1):
        for name,index,changes in (
          ('SetDisplayMode',0,[(MODE,0)]),('CreateSurface',0,[(COUNT,2),(WIDTH,800),(HEIGHT,600)]),
          ('GetAttachedSurface',0,[(COUNT,1),(DEPTH,32),(PALETTE,PO)]),
          ('CreatePalette',0,[(MODE,0),(RECORDS[0],2)]),('SetPalette',0,[(WINDOW,0x19000700)]),
          ('IsLost',0,[(RECORDS[0]+44,SO+16)]),('Restore',0,[(RECORDS[0]+4,0x12345678)])):
            state=State(84,mode,4);reply,output=state.response(name,(SO,));state.seen={};state.surface_number=0
            if name=='IsLost':reply=LOST
            if name=='Restore':state.script['IsLost',0]=(LOST,None,[])
            state.script[name,index]=(reply,output,changes);sequence(state,2)
    for mode in (0,1):
        for p in (DRAW,PALETTE)+tuple(q+44 for q in RECORDS):
            state=State(85,mode)
            for q in RECORDS:state.put(q+44,0)
            state.put(p,0x80000000);sequence(state)
    details={'scope':'Real C89 SDK rebuild and real initializer/restoration; SDK replies modeled only in differential emulation','coverage':coverage,
      'checks':'Authenticated FPO/hash/dispatch; independent raw-address oracle; ordered release/clear/init/create/metadata/palette/restore/show, all arguments/descriptor snapshots/full bank/global/both-palette/arena/unrelated image, live callbacks, HRESULTs, persistent ownership/flags/faults and cdecl/stdcall/nonvolatile/DF',
      'limitations':'Native windowed validation recorded separately using original WndProc reference boundary; no native fullscreen/palette or original compiler/link layout/instruction equality or full game/startup/menu/render/input/audio/races. Unknown adjacent aliases, out-of-bank count growth, arbitrary reentry/concurrency and native fault frames/atomicity unverified.'}
    for kind in ('compilation','emulation'):
        record_run(0x5bd70,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage['comparisons'] if kind=='emulation' else 0,command='uv run python tools/verify_surface_rebuild.py',details=details)
    native=native_surface(DLL,rebuild=True)
    print('PASS actual SDK rebuild:',native['stdout'],flush=True)
    record_run(0x5bd70,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=3,command='uv run python tools/verify_surface_rebuild.py',details=native)
    print('PASS native surface rebuild:',coverage,flush=True)
    return coverage

if __name__=='__main__':verify_surface_rebuild()
