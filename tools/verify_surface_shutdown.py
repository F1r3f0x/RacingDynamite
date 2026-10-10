# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Native resource shutdown: ordered real SDK Release calls and retained state."""
import copy
import hashlib
import struct
import pefile
from verify_surface_open import State as OpenState, Session as OpenSession, FIELDS, RECORDS, ARENA, SO, CO, PO, DO, MODE, DRAW, CLIPPER, PALETTE, VTABLES, Fault, native_surface, INPUTS as OPEN_INPUTS
from windows_target import ROOT,BUILD,verify_target
from build_decomp import build
from windows_tracking import record_run

if not __debug__:raise RuntimeError('Verification requires assertions')
DLL=BUILD/'surface_shutdown_validation.dll'
INPUTS=OPEN_INPUTS+['tools/verify_surface_shutdown.py']

class State(OpenState):
    def apply(self):
        self.events=[];self.fault=None;self.seen={};self.surface_number=0
        try:
            if self.read(MODE)==0 and self.read(CLIPPER)!=0:
                self.method('Release',self.read(CLIPPER))
            for p in RECORDS[:6]:
                if self.read(MODE)==0:
                    surface=self.read(p+44)
                    if surface:
                        self.method('Release',surface);self.write(p+44,0);self.write(p,0)
            for p in RECORDS[6:]:
                surface=self.read(p+44)
                if surface:
                    self.method('Release',surface);self.write(p+44,0);self.write(p,0)
            if self.read(MODE)!=0 and self.read(PALETTE)!=0:
                self.method('Release',self.read(PALETTE));self.write(PALETTE,0)
            if self.read(DRAW)!=0:
                self.method('Release',self.read(DRAW));self.write(DRAW,0)
            return 1
        except Fault:return None

class Session(OpenSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        # Reuse the generic execution/ABI harness, changing only the entrypoint.
        self.symbols['Gfx_SurfaceOpenNative']=0x45c060 if original else self.symbols['Gfx_SurfaceShutdownNative']


def inspect(pe):
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[0x5c060]==(229,0,0,0x209)
    assert hashlib.sha256(pe.get_data(0x5c060,229)).hexdigest()=='60b2e80bfcaf2f7199f8b91e331b6ee717b18925e975d0845d2196fbcfc90090'
    assert pe.get_data(0x5b6b0,4)==struct.pack('<I',0x50eb74)
    assert pe.get_data(0x5b6b4,4)==struct.pack('<I',0x45c060)


def verify_surface_shutdown():
    original=pefile.PE(data=verify_target());inspect(original);build(dll=DLL);compiled=pefile.PE(str(DLL))
    coverage={'comparisons':0,'persistent_followups':0,'faults':0,'mutations':0,'shared_surfaces':0}
    def sequence(state,repeats=1,shared=False):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for repeat in range(repeats):
            policy=copy.deepcopy(state);result=state.apply()
            for session in sessions:session.run(state,result,coverage['comparisons'],copy.deepcopy(policy))
            coverage['comparisons']+=1;coverage['persistent_followups']+=int(repeat>0)
            coverage['faults']+=int(state.fault is not None);coverage['shared_surfaces']+=int(shared)
            coverage['mutations']+=int(any(row[2] and index<state.seen.get(name,0) for (name,index),row in state.script.items()))
            if state.fault:break
    for mode in (0,1,2,0xffffffff):
        for pattern in range(4):
            state=State(pattern,mode)
            for k,p in enumerate(RECORDS):state.put(p+44,SO+16*k if pattern==0 or pattern==2 and k%2 or pattern==3 and k in (0,5,6,25) else 0)
            sequence(state,3)
        for k,p in enumerate(RECORDS):
            state=State(k,mode);state.put(CLIPPER,0);state.put(PALETTE,0);state.put(DRAW,0)
            for q in RECORDS:state.put(q+44,0)
            state.put(p+44,SO+16*k);sequence(state,2)
        state=State(42,mode)
        for p in RECORDS:state.put(p+44,SO)
        sequence(state,3,True)
        for slot in (CLIPPER,PALETTE,DRAW):
            state=State(43,mode)
            for p in RECORDS:state.put(p+44,0)
            state.put(slot,0);sequence(state,2)
    for occurrence in (0,1,5,6,20,25,26,27):
        for mode in (0,1):
            state=State(44,mode)
            for k,p in enumerate(RECORDS):state.put(p+44,SO+16*k)
            changes=[(MODE,1-mode),(RECORDS[0]+4,0xaabbccdd),(RECORDS[5]+44,SO),(DRAW,DO),(PALETTE,PO)]
            state.script['Release',occurrence]=(0xffffffff,None,changes);sequence(state,2)
    for mode in (0,1):
        for p in [CLIPPER,PALETTE,DRAW]+[q+44 for q in RECORDS]:
            state=State(45,mode)
            for q in RECORDS:state.put(q+44,0)
            state.put(p,0x80000000);sequence(state)
    # Simulated release invalidates an object; repeated windowed shutdown must
    # retain the stale clipper and fault on its next unchecked vtable read.
    state=State(46,0)
    for p in RECORDS:state.put(p+44,0)
    state.script['Release',0]=(0,None,[(CO,0x80000000)]);sequence(state,2)
    details={'scope':'Real C89 SDK resource shutdown; only IUnknown Release replies/clobbers/mutations modeled',
      'coverage':coverage,'checks':'Authenticated FPO/hash/dispatch target; independent raw-address oracle, ordered live mode/pointer reads, captured Release this and callback snapshots, clear pointer then active, unchanged markers/opaque/geometry/type/palette bytes, all known banks/globals/arena/unrelated image, persistent calls/stale clipper, shared interfaces, faults and cdecl/stdcall/nonvolatile/DF.',
      'limitations':'Native windowed lifecycle recorded separately with original WndProc reference boundary; no fullscreen/native game parity or instruction equality/original compiler/link layout, arbitrary reentry/concurrency/unknown adjacent aliases or native fault frames/atomicity. No native full game/startup/menu/render/input/audio/races.'}
    for kind in ('compilation','emulation'):
        record_run(0x5c060,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage['comparisons'] if kind=='emulation' else 0,command='uv run python tools/verify_surface_shutdown.py',details=details)
    native=native_surface(DLL,shutdown=True)
    record_run(0x5c060,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=4,command='uv run python tools/verify_surface_shutdown.py',details=native)
    print('PASS native resource shutdown:',coverage,flush=True)
    print('PASS actual SDK shutdown:',native['stdout'],flush=True)
    return coverage

if __name__=='__main__':verify_surface_shutdown()
