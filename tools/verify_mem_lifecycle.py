# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Lifecycle wrappers: isolated dependency contracts and real memory integration."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS)
from verify_mem_destroy import DestroySession, inspect_original as inspect_destroy
from verify_mem_free import PoolFixture, ARENA, ARENA_SIZE
from verify_mem_alloc import oracle as alloc_oracle
from verify_matching import BOOKKEEPING_FIELDS, PRESERVED, STACK, STOP
from build_decomp import build, STARTUP_EXPORTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
ROUTINES = {
    'Gfx_InstallSurfaceDispatch': (0x5b690,146,'3cb7a08c993e9d2ce4db93a2ebe96e9165728ef6bae5b5014371ebaef3de41bf'),
    'Gfx_SelectBackend': (0x56af0,30,'d1fcb40ea924f27f8afef2a4b1e804ee70ce3fed2ff4f9e74a376877e7c3c4cd'),
    'Lisa_PrintVersion': (0x5b4f0,34,'719ae03224b3f4a4ab08fa52985cd65454730bd8de870f8afe632d9216951802'),
    'Input_ResetCallbacks': (0x55ab0,13,'c301c37754413d115bac22b922a9ed480b4080011c559a4485b7ea044270fd59'),
    'Mem_InitSystem': (0x5b170,41,'e4026b0791ef97b56f8fa7013dd568dfd4772d64e2638d850a217c9d338ce2b8'),
    'Mem_ShutdownSystem': (0x5b1a0,16,'75f6e3f589d50b5cb564113e1270c4f2a5df39c0614d41516b520d0d3ee064a3')}
DEPENDENCIES = {
    'Gfx_InitPrimitiveState': (0x5e610,326,'fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3'),
    'Gfx_InstallSpriteDispatch': (0x56e60,181,'215bc3324a371904ec72c0e780eec9b178b47f01e195eaaac6124781c612a8b6')}
MEMORY = {'Mem_InitHandles':(0x5b1f0,76), 'Mem_ShutdownHandles':(0x5b240,155),
    'Mem_NextHandleId':(0x5b1b0,58), 'Mem_RegisterHandle':(0x5b360,120),
    'Mem_ReleaseHandleId':(0x5b410,62)}
ARGUMENTS = {'Gfx_SelectBackend':1, 'Mem_CreatePool':1, 'Mem_DestroyPool':1,
    'Mem_RegisterHandle':1, 'Mem_ReleaseHandleId':1, 'Mem_Alloc':2, 'Mem_Free':2,
    'malloc':1, 'free':1, 'callback':1}
CALLBACK = 0x7200000
DLL = BUILD / 'mem_lifecycle_validation.dll'
BANNER_STRINGS = {0x4bab20: b'Compilation 0.91.0',
    0x4bab5c: b'\nLisa 2 Development System, %s\n',
    0x4bab3c: b'Copyright (c) UDS, 1995-1996\n\n'}
BANNER_ARGS = [(BANNER_STRINGS[0x4bab5c], BANNER_STRINGS[0x4bab20]),
    (BANNER_STRINGS[0x4bab3c],)]
RESOURCE_FIELDS = [('g_inputKeyEventCallback',0x50de14,4), ('g_inputPollCallback',0x50e678,4)]
SURFACE_SLOTS = [('g_surfaceConfigure', 4568880), ('g_surfaceOpen', 4568896), ('g_surfaceClose', 4570480), ('g_surfaceReset', 4571232), ('g_surfaceConfigureSurface', 4571472), ('g_surfaceBlit', 4571488), ('g_surfaceCopyPixels', 4571600), ('g_surfaceClear', 4572112), ('g_surfacePresent', 4572336), ('g_surfaceReserved', 4572448), ('g_surfaceLock', 4572464), ('g_surfaceUnlock', 4572800), ('g_surfaceSetPalette', 4572880), ('g_surfaceRestore', 4572976)]
STATE_FIELDS = BOOKKEEPING_FIELDS + RESOURCE_FIELDS + [(name,0x50eb68+4*i,4) for i,(name,_) in enumerate(SURFACE_SLOTS)] + [('validation_gfxDispatchWords',0x50eba0,80)]
# Authentic consumer extents/stack contracts; recovered independently of C types.
SURFACE_CONSUMERS = [
    (0x56b10,30,4,0,'0aefb3b8a35847643dbeb6d4c15ca363c8fb17a65cf8c0bf1a1e9df12262f907'),
    (0x56bc0,19,0,1,'52be6802ced83d2ed0f4d075befb6cb8a27c422283385d5948efbf9807fee826'),
    (0x56be0,6,0,2,'38113c56bd5b8bad3c416585d5886f408e4c6e8aa5712adc29541c69099eaac4'),
    (0x56bf0,19,0,3,'12f76bae9f55a6539d23baf465bf52e0b2450cf8cc41121eb16f100448679412'),
    (0x56c60,35,5,4,'a6101b85671afd849e7b08e410e5db71224d69dfdaddd303e64e7e1f572ea8e3'),
    (0x56b30,50,8,5,'299c846485b18251d0a57631f64aba15d902a277fc877bf5465eb08e5efa2689'),
    (0x56b70,55,9,6,'8bfc4e41d08ca864cab508cc3db8631f6f1dacef871033cbdfc54d9c23e8996a'),
    (0x56bb0,15,1,7,'652dfcd5dbaf40c20c1d2f5745962ddee365aa37d367a97cba92f986ba2e2685'),
    (0x56c90,15,1,8,'90586f74ec900ac8213237cc025768b3c50c8782ddbb0130505452bcea742f01'),
    (0x56c10,20,2,10,'46a5ae6e26d29bf872659b827ff00209cb3cae73e67617b72aa1e49bf4a528d3'),
    (0x56c30,15,1,11,'23960952e78fdc4df291be1b843aa2b238ceee65bdc1baa79770bb2e88857aa5'),
    (0x56c40,15,1,12,'2f73ffb25305361147dd76513ce9b4125182485565ad60410532f9cc6f60f308'),
    (0x56c50,6,0,13,'9bac4d3e6b644d4ea3199af28c48924cd6366565dfb7bf1f29fe153d81102e31'),
]
SURFACE_TARGETS = [
    (0x5b730,6,'7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7'),
    (0x5b740,1575,'4626e6dbe7c24d021c37141e72e616e63bc43ebdd89cec16ea45012cf98acdc9'),
    (0x5bd70,745,'9f9272e9f5f76e0318c38fc925e2bc4d03b304c169f0daf740b995b6ec055745'),
    (0x5c060,229,'60b2e80bfcaf2f7199f8b91e331b6ee717b18925e975d0845d2196fbcfc90090'),
    (0x5c150,6,'7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7'),
    (0x5c160,108,'e716f58cbe637634909a5de39555b22a4f99b391b14673f17879ecf036ccdde2'),
    (0x5c1d0,497,'ab0e1a366b4876c8807819b388b91435c556b4546a92764e400e2eee3ecd3c39'),
    (0x5c3d0,210,'dd3bd132e2b814763aab3ab5f58c6b162ddf69bb41a46f98e2cdd5a9b87df8d6'),
    (0x5c4b0,98,'0a2b21a1bf12a224948977d5410f15eee668a81d3245a9d644f8f0846af6412e'),
    (0x5c520,6,'7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7'),
    (0x5c530,326,'35592a3ed0bfaa7af2ea2a5d0ca5116f8f7f3f225a78705c31cc9ddfa8b3b512'),
    (0x5c680,71,'0e6fdf8dbb9b1279859342662207e6693a69b37cf823335fd050d2c6aae1c7af'),
    (0x5c6d0,83,'cce59efd58c21d910a12512b15c5641b586e6beac7a4fd5daad80a56c991f5f0'),
    (0x5c730,179,'b5fbe03e6aab008cadf961db34a86c9d25525993c52b3a60dbe72a09591d6f71'),
]

ISOLATED_HELPERS = ('Lisa_PrintVersion','Input_ResetCallbacks','Gfx_SelectBackend',
    'Mem_InitPools','Mem_InitHandles','Mem_ShutdownHandles','Mem_ShutdownPools')
MODELED_STARTUP = set(STARTUP_EXPORTS) - {'Gfx_SelectBackend','Gfx_InstallSurfaceDispatch'}
INPUTS = ['decomp/src/lisa3d.c', 'decomp/include/lisa3d.h', 'decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/mem.c','decomp/include/mem.h','decomp/target.json',
    'tools/verify_mem_lifecycle.py','tools/verify_mem_destroy.py','tools/verify_mem_pools.py',
    'tools/verify_mem_alloc.py','tools/verify_mem_free.py','tools/verify_matching.py',
    'tools/build_decomp.py','tools/verify_font_cleanup.py','tools/windows_target.py',
    'tools/windows_tracking.py']


def record(name,kind,outcome,cases=0,**details):
    record_run(ROUTINES[name][0],kind,outcome,inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases,command=('uv run python tools/verify_mem_lifecycle.py'
            if __name__=='__main__' else 'uv run tools/verify_matching.py'),details=details)


def inspect_original(pe):
    inspect_destroy(pe)
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,(r,n,digest) in (ROUTINES|DEPENDENCIES).items():
        raw=pe.get_data(r,n);ins=list(md.disasm(raw,0x400000+r))
        assert hashlib.sha256(raw).hexdigest()==digest
        assert sum(i.size for i in ins)==n
        assert fpo[r]==(n,0,1 if name=='Gfx_SelectBackend' else 0,0)
        assert ins[-1].mnemonic==('jmp' if name=='Gfx_InitPrimitiveState' else 'ret')
    for name,(r,n,_) in ROUTINES.items():
        if name in ('Input_ResetCallbacks','Lisa_PrintVersion','Gfx_SelectBackend','Gfx_InstallSurfaceDispatch'):continue
        ins=list(md.disasm(pe.get_data(r,n),0x400000+r))
        expected=[(0x45b170,0x45b4f0),(0x45b175,0x45ad50),(0x45b17a,0x45b1f0),
            (0x45b17f,0x455ab0),(0x45b184,0x45e610),(0x45b18b,0x456af0)] if name=='Mem_InitSystem' else [(0x45b1a0,0x45b240),(0x45b1a5,0x45b140)]
        assert [(i.address,int(i.op_str,16)) for i in ins if i.mnemonic=='call']==expected
        assert len(ins)==(10 if name=='Mem_InitSystem' else 4)
        assert not any(e.type==3 and r<=e.rva<r+n for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries)
    selector=list(md.disasm(pe.get_data(0x56af0,30),0x456af0))
    assert [(i.mnemonic,i.op_str) for i in selector]==[
        ('mov','ecx, dword ptr [esp + 4]'),('test','ecx, ecx'),('jne','0x456b08'),
        ('call','0x45b690'),('call','0x456e60'),('mov','eax, 1'),('ret',''),
        ('mov','eax, 2'),('ret','')]
    assert pe.get_data(0x56b0e,2)==b'\xcc'*2
    relocations={e.rva:struct.unpack('<I',pe.get_data(e.rva,4))[0]
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type==3}
    assert not any(0x56af0<=r<0x56b0e for r in relocations)
    surface=[0x45b730,0x45b740,0x45bd70,0x45c060,0x45c150,0x45c160,
        0x45c1d0,0x45c3d0,0x45c4b0,0x45c520,0x45c530,0x45c680,0x45c6d0,0x45c730]
    sprite=[(0x50eba0+4*i,v) for i,v in enumerate([
        0x456f20,0x456f30,0x456f40,0x456f50,0x456f60,0x457020,0x457250,0x4571b0,0x45c830])]
    sprite += [(0x50ebd8,0x461360),(0x50ebd4,0x45c920),(0x50ebdc,0x45c940),
        (0x50ebe0,0x45c960),(0x50ebe4,0x461a80),(0x50ebec,0x457110),(0x50ebe8,0x457170)]
    for r,n,stores,calls in [(0x5b690,146,[(0x50eb68+4*i,v) for i,v in enumerate(surface)],[]),
            (0x56e60,181,sprite,[0x45d840,0x45c9f0,0x45c7f0])]:
        ins=list(md.disasm(pe.get_data(r,n),0x400000+r))
        assert fpo[r]==(n,0,0,0)
        assert [(i.op_str) for i in ins if i.mnemonic=='mov'] == [
            'dword ptr [%s], %s'%(hex(a),hex(v)) for a,v in stores]+['eax, 1']
        assert [int(i.op_str,16) for i in ins if i.mnemonic=='call']==calls
        assert [(e,relocations[e]) for e in sorted(relocations) if r<=e<r+n]==[
            pair for i,(a,v) in enumerate(stores) for pair in [(r+10*i+2,a),(r+10*i+6,v)]]
    for r,n,argc,slot,digest in SURFACE_CONSUMERS:
        assert fpo[r]==(n,0,argc,0)
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest
        instructions=list(md.disasm(pe.get_data(r,n),0x400000+r))
        assert any(i.mnemonic in ('call','jmp') and i.op_str==
            'dword ptr [%s]'%hex(0x50eb68+4*slot) for i in instructions)
        if argc:
            cleanup=hex(4*argc) if 4*argc>=10 else str(4*argc)
            assert any(i.mnemonic=='add' and i.op_str=='esp, '+cleanup
                for i in instructions)
    for r,n,digest in SURFACE_TARGETS:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest
        instructions=list(md.disasm(pe.get_data(r,n),0x400000+r))
        assert sum(i.size for i in instructions)==n and instructions[-1].mnemonic=='ret'
    assert pe.get_data(0x5c3c1,15)==b'\xcc'*15 # Frame-pointer target has no FPO.
    # Reserved slot: no callable consumer found in FPO scan or relocations.
    assert [(i.address,i.mnemonic) for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if '0x50eb8c' in i.op_str]==[(0x45b6ea,'mov')]
    data=next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.data')
    assert data.VirtualAddress+data.SizeOfRawData<=0x10eb68
    assert 0x10eb68+136<=data.VirtualAddress+data.Misc_VirtualSize
    assert [i.address for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic=='call' and i.op_str=='0x456af0']==[0x412502,0x45b18b]
    assert pe.get_data(0x12500,10)==bytes.fromhex('6a00e8e945040083c404')
    for r,n,digest in [(0x5d840,84,'445faf9112110b216014f95c057ab6d572a81fec3fa64a867b9d5f03739c1d43'),
            (0x5c9f0,84,'922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e'),
            (0x5c7f0,62,'41862be4869238f9f9261e170ab377306cd2aa18afbbd6761a59bf2f32a5b132')]:
        assert fpo[r]==(n,0,0,0) and hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest
    assert pe.get_data(0xbace4,8)==bytes(8) # Two lazy allocation flags: file zero.
    banner=list(md.disasm(pe.get_data(0x5b4f0,34),0x45b4f0))
    assert [(i.mnemonic,i.op_str) for i in banner]==[
        ('push','0x4bab20'),('push','0x4bab5c'),('call','0x46a500'),
        ('add','esp, 8'),('push','0x4bab3c'),('call','0x46a500'),
        ('add','esp, 4'),('xor','eax, eax'),('ret','')]
    assert [(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type==3 and 0x5b4f0<=e.rva<0x5b512]==[
            (0x5b4f1,0x4bab20),(0x5b4f6,0x4bab5c),(0x5b503,0x4bab3c)]
    assert pe.get_data(0x5b512,14)==b'\xcc'*14
    for va,value in BANNER_STRINGS.items():
        assert pe.get_data(va-0x400000,len(value)+1)==value+b'\0'
        section=pe.get_section_by_rva(va-0x400000)
        assert section.Name.rstrip(b'\0')==b'.data'
        assert va-0x400000+len(value)+1<=section.VirtualAddress+section.SizeOfRawData
    assert fpo[0x6a500]==(61,0,1,0x202)
    assert hashlib.sha256(pe.get_data(0x6a500,61)).hexdigest()==        'ed5882e648efe6314e582da142973e428c0c42ba90e311b3da568581151fc447'
    assert [i.address for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic=='call' and i.op_str=='0x45b4f0']==[0x45b170]

    # Independently authenticate consumer ABI/ownership, not inherited names.
    for r,n,digest in [(0x560c0,26,'3b23ff815d34e56903eb4477a3cb227730eae66e254731b7902b62fed7d10a8b'),
            (0x56160,19,'2d1dd1ad615325f87eae45141b3a1d7f53a09f9d2646d1adcd17f93e16c76f58'),
            (0x55ef0,376,'ac1bfc5738cf5a68eef9491b90049eb23453052f9350d545c82586248ec92f33'),
            (0x55c60,436,'6ade6c95c919cb9ea6d550e552623c057dbefcdef7b325d552d883a7ad782efe')]:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest and fpo[r][0]==n
    assert fpo[0x560c0]==(26,0,2,0) and fpo[0x56160]==(19,0,5,0)
    for va,expected in [(0x50de14,[0x55ab3,0x560c2]),
            (0x50e678,[0x55ab8,0x55f4a,0x56166])]:
        assert [e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
            if e.type==3 and struct.unpack('<I',pe.get_data(e.rva,4))[0]==va]==expected
    assert any(s.dll==b'WINMM.dll' and i.name==b'timeSetEvent' and i.address==0x64c474
        for s in pe.DIRECTORY_ENTRY_IMPORT for i in s.imports)
    assert pe.get_data(0x55abd,3)==b'\xcc'*3
    relocs=[(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type==3 and 0x55ab0<=e.rva<0x55abd]
    assert relocs==[(0x55ab3,0x50de14),(0x55ab8,0x50e678)]
    data=next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.data')
    for _,va,n in RESOURCE_FIELDS:
        assert data.VirtualAddress+data.SizeOfRawData<=va-0x400000
        assert va-0x400000+n<=data.VirtualAddress+data.Misc_VirtualSize
    assert pe.get_data(0x5b199,7)==b'\xcc'*7
    assert pe.get_data(0x5b1b0,1)==b'\x83'  # Next entry directly follows shutdown RET.
    for site,target in [(0x412170,0x45b170),(0x4122a7,0x45b1a0),(0x412502,0x456af0)]:
        raw=pe.get_data(site-0x400000,5)
        assert raw[0]==0xe8 and site+5+struct.unpack_from('<i',raw,1)[0]==target
    for target,site in [(0x45b170,0x412170),(0x45b1a0,0x4122a7)]:
        assert not any(e.type==3 and struct.unpack('<I',pe.get_data(e.rva,4))[0]==target
            for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries)
        calls=[i.address for r,(n,_,_,_) in fpo.items()
            for i in md.disasm(pe.get_data(r,n),0x400000+r)
            if i.mnemonic=='call' and i.op_str==hex(target)]
        assert calls==[site]  # Bounded FPO scan, not a whole-program absence proof.
    for r,n in MEMORY.values():assert fpo[r][0]==n


def verify_surface_consumer_abi(pe):
    """Execute original forwarding wrappers after original installation.

    Target-entry sinks validate argument order and caller cleanup only. They do
    not execute target graphics behavior or certify the production prototypes.
    """
    cpu=Uc(UC_ARCH_X86,UC_MODE_32)
    cpu.mem_map(0x400000,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095)
    cpu.mem_write(0x400000,pe.get_memory_mapped_image())
    cpu.mem_map(STACK,0x10000);cpu.mem_map(STOP,0x1000)
    sp=STACK+0x8000
    cpu.mem_write(sp,struct.pack('<I',STOP));cpu.reg_write(UC_X86_REG_ESP,sp)
    cpu.emu_start(0x45b690,STOP,count=100)
    assert cpu.reg_read(UC_X86_REG_EAX)==1
    cases=0
    for r,n,argc,slot,_ in SURFACE_CONSUMERS:
        target=SURFACE_SLOTS[slot][1]
        for seed in (0,0x80000000,0xffffffff):
            args=tuple((seed+0x1020304*(i+1))&0xffffffff for i in range(argc))
            seen=[]
            def sink(uc,address,length,unused):
                if address==target:
                    current=uc.reg_read(UC_X86_REG_ESP)
                    actual=struct.unpack('<'+'I'*argc,uc.mem_read(current+4,argc*4)) if argc else ()
                    assert actual==args;seen.append(address)
                    uc.reg_write(UC_X86_REG_EAX,0) # Avoid secondary sprite tail call.
                    uc.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);uc.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
                    uc.reg_write(UC_X86_REG_ESP,current+4)
                    uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(current,4))[0])
                else:assert 0x400000+r<=address<0x400000+r+n
            hook=cpu.hook_add(UC_HOOK_CODE,sink)
            cpu.mem_write(sp,struct.pack('<'+'I'*(argc+1),STOP,*args))
            before=bytes(cpu.mem_read(sp,128));saved=[0x13572468+i for i in range(4)]
            for register,value in zip(PRESERVED,saved):cpu.reg_write(register,value)
            cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
            cpu.emu_start(0x400000+r,STOP,count=100)
            assert seen==[target] and cpu.reg_read(UC_X86_REG_ESP)==sp+4
            assert cpu.reg_read(UC_X86_REG_EAX)==0
            assert [cpu.reg_read(register) for register in PRESERVED]==saved
            assert bytes(cpu.mem_read(sp,128))==before
            cpu.hook_del(hook);cases+=1
    return cases


class Oracle:
    """Independent instruction-derived effects; explicit arbitrary boundary models."""
    def __init__(self,state,isolated=False,actions=None,returns=None):
        self.state={k:bytearray(v) for k,v in state.items()}
        self.isolated=isolated;self.actions=actions or {};self.returns=returns or {}
        self.events=[];self.boundaries=[];self.indices={};self.top=None

    def snapshot(self):return {k:bytes(v) for k,v in self.state.items()}

    def read(self,name,off=0,n=4):
        self.events.append(('read',name,off,n))
        return int.from_bytes(self.state[name][off:off+n],'little')

    def write(self,name,off,value,n=4):
        self.events.append(('write',name,off,n,value))
        self.state[name][off:off+n]=value.to_bytes(n,'little')

    def boundary(self,name,args):
        self.boundaries.append((name,args,self.snapshot()))
        self.events.append(('boundary',name,args))
        index=self.indices.get(name,0);self.indices[name]=index+1
        for field,off,n,value in self.actions.get((name,index),()):
            self.state[field][off:off+n]=value.to_bytes(n,'little')
        values=self.returns.get(name,())
        return values[index] if index<len(values) else 0xdeadbeef

    def call(self,name,args=()):
        if name!=self.top:self.events.append(('call',name,args))
        if name in MODELED_STARTUP or (self.isolated and name in ISOLATED_HELPERS):
            return self.boundary(name,args)
        if name=='Gfx_InstallSurfaceDispatch':
            for field,target in SURFACE_SLOTS:self.write(field,0,target)
            return 1
        if name=='Gfx_SelectBackend':
            if args[0]!=0:return 2
            self.call('Gfx_InstallSurfaceDispatch');self.call('Gfx_InstallSpriteDispatch');return 1
        if name=='Lisa_PrintVersion':
            for arguments in BANNER_ARGS:self.call('printf',arguments)
            return 0
        if name=='Input_ResetCallbacks':
            self.write('g_inputKeyEventCallback',0,0);self.write('g_inputPollCallback',0,0);return None
        if name=='Mem_InitSystem':
            for dep in ['Lisa_PrintVersion','Mem_InitPools','Mem_InitHandles',
                        'Input_ResetCallbacks','Gfx_InitPrimitiveState','Gfx_SelectBackend']:
                self.call(dep,(0,) if dep=='Gfx_SelectBackend' else ())
            return 1
        if name=='Mem_ShutdownSystem':
            self.call('Mem_ShutdownHandles');self.call('Mem_ShutdownPools');return 1
        if name=='Mem_InitPools':
            for i in range(256):self.write('table',4*i,0)
            self.call('Mem_CreatePool',('DEFAULT',));return 1
        if name=='Mem_CreatePool':
            slot=next((i for i in range(256) if self.read('table',4*i)==0),256)
            if slot==256:return 0xffffffff
            pointer=self.call('malloc',(320,))
            if pointer==0:return 0xffffffff
            self.write('table',4*slot,pointer);off=pointer-ARENA
            for i,value in enumerate(b'DEFAULT'):
                self.events += [('read','name',i,1),('read','name',i,1)]
                self.write('arena',off+i,value,1)
            self.events.append(('read','name',7,1));self.write('arena',off+7,0,1)
            for i in range(64):self.write('arena',off+64+4*i,0)
            return slot
        if name=='Mem_InitHandles':
            flag=self.read('g_memHandlesInitialized')
            if flag==1:return 1
            self.write('g_memHandlesInitialized',0,1)
            for i in range(200):self.write('g_memHandleStatus',4*i,0)
            for i in range(200):self.write('g_memHandleIds',2*i,i+1,2)
            self.write('g_memHandleCursor',0,0,2);return 1
        if name=='Mem_ShutdownHandles':
            if self.read('g_memHandlesInitialized')==0:return 1
            self.write('g_memHandlesInitialized',0,0)
            for flag in (0x10000,0x20000):
                for i in range(200):
                    if self.read('g_memHandleStatus',4*i)==1 and self.read('g_memHandleFlags',4*i)==flag:
                        self.write('g_memHandleStatus',4*i,0)
                        param=self.read('g_memHandleParameters',4*i)
                        assert self.read('g_memHandleCallbacks',4*i)==CALLBACK
                        self.call('callback',(param,))
            return 1
        if name=='Mem_ShutdownPools':
            for i in range(256):
                if self.read('table',4*i):self.call('Mem_DestroyPool',(i,))
            return 1
        if name=='Mem_DestroyPool':
            slot=args[0];pool=self.read('table',4*slot)-ARENA
            for p in range(64):
                page=self.read('arena',pool+64+4*p)
                if not page:continue
                for b in range(64):
                    block=self.read('arena',page-ARENA+4*b)
                    if not block:continue
                    for r in range(16):
                        off=block-ARENA+8*r
                        if self.read('arena',off+4):self.call('free',(self.read('arena',off),))
                    self.call('free',(block,))
                self.call('free',(page,))
            self.call('free',(ARENA+pool,));self.write('table',4*slot,0);return 1
        if name=='Mem_RegisterHandle':
            if self.read('g_memHandlesInitialized')==0:return 0
            for i in range(200):
                if self.read('g_memHandleStatus',4*i)==0:
                    context=self.read('g_memPendingContext')
                    self.write('g_memHandleStatus',4*i,1);self.write('g_memHandleFlags',4*i,0x10000)
                    self.write('g_memHandleContexts',4*i,context);self.write('g_memRegisteredHandleIds',4*i,args[0])
                    self.write('g_memHandleParameters',4*i,self.read('g_memPendingParameter'))
                    self.write('g_memHandleCallbacks',4*i,self.read('g_memPendingCallback'));return i
            return 0xffffffff
        if name=='Mem_Alloc':
            values=self.returns.get('malloc',())
            result,final,events,snapshots,requests=alloc_oracle(self.state['arena'],args[0],args[1],values,{})
            for event in events:
                kind,*rest=event
                if kind=='table':self.read('table',rest[0],rest[1])
                elif kind=='read':self.read('arena',*rest)
                elif kind=='write':
                    off,n,value=rest;self.write('arena',off,value,n)
                elif kind=='malloc':
                    size,pointer=rest;assert self.call('malloc',(size,))==pointer
                else:raise AssertionError(event)
            assert bytes(self.state['arena'])==final;return result
        if name in ('malloc','free','callback','printf'):return self.boundary(name,args)
        raise AssertionError('Unmodeled routine '+name)

    def invoke(self,name,args=()):
        self.top=name;result=self.call(name,args)
        return result,self.snapshot(),self.events,self.boundaries


class LifecycleSession(DestroySession):
    def __init__(self,pe,symbols=None):
        super().__init__(pe,symbols)
        self.to_original = {} if symbols is None else {symbols['Gfx_Surface'+name[len('g_surface'):]+'Native']:va for name,va in SURFACE_SLOTS}
        self.to_rebuilt = {v:k for k,v in self.to_original.items()}
        self.fields=[(name,va if symbols is None else symbols[name],n) for name,va,n in STATE_FIELDS]
        self.entries.update({name:0x400000+r if symbols is None else symbols[name]
            for name,(r,_,_) in (ROUTINES|DEPENDENCIES).items()})
        self.entries.update({name:0x400000+r if symbols is None else symbols[name] for name,(r,n) in MEMORY.items()})
        self.entries.update(malloc=self.malloc,free=self.free,callback=CALLBACK,
            printf=0x46a500 if symbols is None else symbols['printf'])
        if symbols is None:self.ranges.extend((0x400000+r,n) for r,n in MEMORY.values())
        self.ranges.extend((0x400000+r,n) for r,n,_ in ROUTINES.values()) if symbols is None else None
        self.cpu.mem_map(CALLBACK,0x1000)
        self.by_address={addr:name for name,addr in self.entries.items() if name in
            set(ROUTINES)|set(DEPENDENCIES)|set(MEMORY)|{'Mem_InitPools','Mem_CreatePool',
            'Mem_ShutdownPools','Mem_DestroyPool','Mem_Alloc','Mem_Free','malloc','free','callback','printf'}}

    def canonical_word(self,name,value):
        return self.to_original.get(value,value) if name.startswith('g_surface') else value

    def state(self):
        return {'table':bytes(self.cpu.mem_read(self.table,1024)),
            'arena':bytes(self.cpu.mem_read(ARENA,ARENA_SIZE)),
            **{name:(struct.pack('<I',self.canonical_word(name,struct.unpack('<I',self.cpu.mem_read(addr,4))[0]))
                if name.startswith('g_surface') else bytes(self.cpu.mem_read(addr,n))) for name,addr,n in self.fields}}

    def reset_state(self,state):
        self.reset(state['table'],state['arena'])
        for name,addr,n in self.fields:
            data=state[name]
            if name.startswith('g_surface'):data=struct.pack('<I',self.to_rebuilt.get(int.from_bytes(data,'little'),int.from_bytes(data,'little')))
            self.cpu.mem_write(addr,data)

    def region(self,address,n):
        for name,addr,length in self.fields:
            if addr<=address and address+n<=addr+length:return name,address-addr
        if self.default<=address and address+n<=self.default+8:return 'name',address-self.default
        if self.table<=address and address+n<=self.table+1024:return 'table',address-self.table
        if ARENA<=address and address+n<=ARENA+ARENA_SIZE:return 'arena',address-ARENA
        if STACK<=address and address+n<=STACK+0x10000:return None
        raise AssertionError('Untracked data access %x'%address)

    def read(self,cpu,access,address,n,value,unused):
        region=self.region(address,n)
        if region:self.events.append(('read',*region,n))

    def write(self,cpu,access,address,n,value,unused):
        region=self.region(address,n)
        if region:self.events.append(('write',*region,n,self.canonical_word(region[0],value)))

    def hook(self,cpu,address,n,unused):
        name=self.by_address.get(address)
        if name:
            sp=cpu.reg_read(UC_X86_REG_ESP);argc=ARGUMENTS.get(name,0)
            args=struct.unpack('<'+'I'*argc,cpu.mem_read(sp+4,argc*4)) if argc else ()
            if name=='printf':
                # Decode actual stack pointers, including terminating NUL bytes.
                # The second call has no vararg: its caller stack is not read as one.
                pointer=struct.unpack('<I',cpu.mem_read(sp+4,4))[0]
                def string_at(pointer):
                    assert self.base<=pointer<self.base+self.image_size
                    raw=bytes(cpu.mem_read(pointer,min(128,self.base+self.image_size-pointer)))
                    assert b'\0' in raw
                    return raw.split(b'\0',1)[0]
                fmt=string_at(pointer)
                assert fmt in (BANNER_ARGS[0][0],BANNER_ARGS[1][0])
                args=(fmt,)
                if fmt==BANNER_ARGS[0][0]:
                    args+=(string_at(struct.unpack('<I',cpu.mem_read(sp+8,4))[0]),)
                if self.base==0x400000:
                    assert pointer==(0x4bab5c if len(args)==2 else 0x4bab3c)
                    if len(args)==2:assert struct.unpack('<I',cpu.mem_read(sp+8,4))[0]==0x4bab20
            if name=='Mem_CreatePool':
                assert args==(self.default,);args=('DEFAULT',)
            if name!=self.top:self.events.append(('call',name,args))
            modeled=name in MODELED_STARTUP or name in ('malloc','free','callback','printf') or (
                self.isolated and name in ISOLATED_HELPERS)
            if modeled:
                assert self.boundary_index<len(self.expected_boundaries),'Unexpected boundary'
                expected_name,expected_args,state=self.expected_boundaries[self.boundary_index]
                assert (name,args)==(expected_name,expected_args)
                assert self.state()==state,'Boundary-entry state mismatch '+name
                self.boundary_index+=1;self.events.append(('boundary',name,args))
                index=self.indices.get(name,0);self.indices[name]=index+1
                addresses={key:addr for key,addr,length in self.fields}|{'table':self.table,'arena':ARENA}
                for field,off,length,value in self.actions.get((name,index),()):
                    cpu.mem_write(addresses[field]+off,value.to_bytes(length,'little'))
                values=self.returns.get(name,());result=values[index] if index<len(values) else 0xdeadbeef
                cpu.reg_write(UC_X86_REG_EAX,result);cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1)
                cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2);cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
                cpu.reg_write(UC_X86_REG_EIP,struct.unpack('<I',cpu.mem_read(sp,4))[0])
                cpu.reg_write(UC_X86_REG_ESP,sp+4);return
        assert any(a<=address<a+length for a,length in self.ranges),'Escaped authenticated memory bodies'

    def invoke_case(self,name,args,expected,isolated=False,actions=None,returns=None,seed=0):
        eax,state,events,self.expected_boundaries=expected
        self.top=name;self.isolated=isolated;self.actions=actions or {};self.returns=returns or {}
        self.events=[];self.indices={};self.boundary_index=0
        cpu=self.cpu;sp=STACK+0x8000;cpu.mem_write(sp,struct.pack('<'+'I'*(1+len(args)),STOP,*args))
        stack=bytes(cpu.mem_read(sp,0x8000));rng=random.Random(seed)
        saved=[rng.getrandbits(32) for _ in PRESERVED]
        for reg,value in zip(PRESERVED,saved):cpu.reg_write(reg,value)
        cpu.reg_write(UC_X86_REG_EAX,rng.getrandbits(32));cpu.reg_write(UC_X86_REG_ESP,sp)
        cpu.reg_write(UC_X86_REG_EFLAGS,2);before=bytes(cpu.mem_read(self.base,self.image_size))
        cpu.emu_start(self.entries[name],STOP,count=5000000)
        assert cpu.reg_read(UC_X86_REG_EIP)==STOP and cpu.reg_read(UC_X86_REG_ESP)==sp+4
        assert [cpu.reg_read(r) for r in PRESERVED]==saved
        assert bytes(cpu.mem_read(sp,0x8000))==stack and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        after=bytearray(cpu.mem_read(self.base,self.image_size))
        for key,addr,length in self.fields+[('table',self.table,1024)]:
            off=addr-self.base;after[off:off+length]=before[off:off+length]
        assert bytes(after)==before,'Unrelated image changed'
        if self.events!=events:
            k=next((i for i,(a,b) in enumerate(zip(self.events,events)) if a!=b),min(len(self.events),len(events)))
            raise AssertionError('Event mismatch %d: %s != %s; counts %d/%d'%(k,self.events[k:k+3],events[k:k+3],len(self.events),len(events)))
        assert self.boundary_index==len(self.expected_boundaries)
        assert self.state()==state
        if eax is not None:assert cpu.reg_read(UC_X86_REG_EAX)==eax
        return state


def verify_mem_lifecycle():
    verify_target();phase='compilation';counts={name:0 for name in ROUTINES}
    try:
        original=pefile.PE(str(TARGET));inspect_original(original)
        consumer_cases=verify_surface_consumer_abi(original)
        build(dll=DLL)
        for name in ROUTINES:record(name,'compilation','pass',scope='Complete production mem.c plus extracted production Input_ResetCallbacks, Lisa_PrintVersion, Gfx_SelectBackend and Gfx_InstallSurfaceDispatch declarations/bodies plus fourteen typed production globals; strict C89; provisional Clang/LLD; explicit nonreturning startup and CRT fixtures')
        rebuilt=pefile.PE(str(DLL));assert not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
        symbols={s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        for name,(r,n,_) in ROUTINES.items():
            equal=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,n)==original.get_data(r,n)
            record(name,'raw_bytes','pass' if equal else 'different',scope='Raw prefix diagnostic only; instruction equality unverified')
        a,b=LifecycleSession(original),LifecycleSession(rebuilt,symbols);phase='emulation'
        rng=random.Random(0x5b170);isolated_counts={name:0 for name in ROUTINES};persistent=0
        def fresh(flag=0):
            state={name:rng.randbytes(n) for name,_,n in STATE_FIELDS}
            state.update(table=bytes(1024),arena=bytes(PoolFixture().data))
            state['g_memHandlesInitialized']=struct.pack('<I',flag)
            state['g_memHandleStatus']=bytes(800)
            return state
        def compare(name,state,args=(),isolated=False,actions=None,returns=None,reset=True):
            expected=Oracle(state,isolated,actions,returns).invoke(name,args)
            if reset:
                for s in (a,b):s.reset_state(state)
            outputs=[s.invoke_case(name,args,expected,isolated,actions,returns,sum(counts.values())) for s in (a,b)]
            assert outputs[0]==outputs[1]
            if name in counts:counts[name]+=1
            if isolated:isolated_counts[name]+=1
            return outputs[0]
        # Any nonzero raw selector returns 2, even with the sign bit set.
        # Zero ignores independent initializer results and preserves their mutations.
        selectors=[1,2,0x7fffffff,0x80000000,0xffffffff]+[rng.getrandbits(32) or 1 for _ in range(128)]
        for selector in selectors:
            state=compare('Gfx_SelectBackend',fresh(),(selector,))
            compare('Gfx_SelectBackend',state,(selector,),reset=False)
        # Full real fourteen-store installer, arbitrary initial words, repeated
        # restoration after the later sprite boundary mutates one surface slot.
        for value in [0,1,2,0x80000000,0xffffffff]+[rng.getrandbits(32) for _ in range(128)]:
            state=fresh()
            for field,_ in SURFACE_SLOTS:state[field]=struct.pack('<I',value)
            state=compare('Gfx_InstallSurfaceDispatch',state)
            compare('Gfx_InstallSurfaceDispatch',state,reset=False)
            actions={('Gfx_InstallSpriteDispatch',0):[
                ('g_surfaceOpen',0,4,value),('validation_gfxDispatchWords',76,4,value),
                ('g_inputPollCallback',0,4,value)]}
            state=compare('Gfx_SelectBackend',fresh(),(0,),actions=actions,
                returns={'Gfx_InstallSpriteDispatch':[value]})
            compare('Gfx_SelectBackend',state,(0,),actions=actions,
                returns={'Gfx_InstallSpriteDispatch':[value]},reset=False)
        # Both printf returns are ignored: exercise independent failure/high-bit
        # pairs and repeated execution without resetting the CPU image/state.
        pairs=[(x,y) for x in [0,1,2,0x80000000,0xffffffff]
            for y in [0,1,2,0x80000000,0xffffffff]]
        pairs += [(rng.getrandbits(32),rng.getrandbits(32)) for _ in range(128)]
        for x,y in pairs:
            actions={('printf',0):[('g_inputKeyEventCallback',0,4,x)],
                ('printf',1):[('g_inputPollCallback',0,4,y)]}
            state=compare('Lisa_PrintVersion',fresh(),actions=actions,returns={'printf':[x,y]})
            compare('Lisa_PrintVersion',state,actions=actions,returns={'printf':[y,x]},reset=False)

        # Standalone reset covers arbitrary raw pointer words and repeated clearing.
        for value in [0,1,2,0x80000000,0xffffffff]+[rng.getrandbits(32) for _ in range(128)]:
            state=fresh();state['g_inputKeyEventCallback']=struct.pack('<I',value)
            state['g_inputPollCallback']=struct.pack('<I',value^0xffffffff)
            state=compare('Input_ResetCallbacks',state)
            compare('Input_ResetCallbacks',state,reset=False)
        # Every dependency receives all representative failure/high-bit EAX values.
        # Models mutate tracked state independently; no helper result short-circuits.
        for name,deps in [('Mem_InitSystem',['Lisa_PrintVersion','Mem_InitPools','Mem_InitHandles',
                'Input_ResetCallbacks','Gfx_InitPrimitiveState','Gfx_SelectBackend']),
                ('Mem_ShutdownSystem',['Mem_ShutdownHandles','Mem_ShutdownPools'])]:
            for dep in deps:
                for value in [0,1,2,0x80000000,0xffffffff]:
                    actions={(d,0):[('g_memPendingContext',0,4,rng.getrandbits(32)),
                        ('arena',0xff000+i*4,4,rng.getrandbits(32))] for i,d in enumerate(deps)}
                    compare(name,fresh(rng.getrandbits(32)),isolated=True,actions=actions,returns={dep:[value]})
            for _ in range(64):
                compare(name,fresh(rng.getrandbits(32)),isolated=True,
                    returns={d:[rng.getrandbits(32)] for d in deps})
        # Real init: flag exactly one skips handle reset; other values reset.
        for flag in [0,1,2,0x80000000,0xffffffff]:
            for pointer in [0,ARENA]:
                state=fresh(flag);state['table']=rng.randbytes(1024)
                compare('Mem_InitSystem',state,returns={'malloc':[pointer]})
        # Changes made by early/late startup boundaries are observed by real helpers.
        for flag in [0,1,2,0xffffffff]:
            actions={
                ('printf',0):[('g_memHandlesInitialized',0,4,flag),
                    ('g_inputKeyEventCallback',0,4,0xffffffff),('g_inputPollCallback',0,4,0x12345678)],
                ('printf',1):[('g_inputPollCallback',0,4,0x87654321)],
                ('Gfx_InitPrimitiveState',0):[('g_memPendingCallback',0,4,CALLBACK)],
                ('Gfx_InstallSpriteDispatch',0):[('g_memHandlesInitialized',0,4,0)]}
            compare('Mem_InitSystem',fresh(2),actions=actions,returns={'malloc':[ARENA]})
        # Every handle slot in both passes; callbacks precede any pool destruction.
        for i in range(200):
            for flag in [0x10000,0x20000]:
                state=fresh(1);f=PoolFixture();block=f.block(63,63)
                for r in range(16):f.word(block+8*r+4,0)
                f.match(63,63,15,pointer=0xffffffff,size=1);state['arena']=bytes(f.data)
                table=bytearray(1024);struct.pack_into('<I',table,(i%256)*4,ARENA);state['table']=bytes(table)
                for key,value in [('g_memHandleStatus',1),('g_memHandleFlags',flag),
                        ('g_memHandleCallbacks',CALLBACK),('g_memHandleParameters',i)]:
                    data=bytearray(state[key]);struct.pack_into('<I',data,4*i,value);state[key]=bytes(data)
                compare('Mem_ShutdownSystem',state)
        for flag in [0,1,2,0x80000000,0xffffffff]:
            for _ in range(8):
                state=fresh(flag);status=[rng.choice([0,1,2,0xffffffff]) for i in range(200)]
                state['g_memHandleStatus']=struct.pack('<200I',*status)
                state['g_memHandleFlags']=struct.pack('<200I',*[rng.choice([0,0x10000,0x20000,0x30000]) for i in range(200)])
                state['g_memHandleCallbacks']=struct.pack('<200I',*([CALLBACK]*200))
                compare('Mem_ShutdownSystem',state)
        # Cross-phase mutations: callback inserts/removes pools, schedules a later
        # second-pass callback, and CRT changes handles after their scan has ended.
        state=fresh(1);state['table']=struct.pack('<256I',ARENA,*([0]*255))
        for key,value in [('g_memHandleStatus',1),('g_memHandleFlags',0x10000),('g_memHandleCallbacks',CALLBACK)]:
            data=bytearray(state[key]);struct.pack_into('<I',data,0,value);state[key]=bytes(data)
        actions={('callback',0):[('table',0,4,0),('table',1020,4,ARENA+0x2000),
            ('g_memHandleStatus',796,4,1),('g_memHandleFlags',796,4,0x20000),
            ('g_memHandleCallbacks',796,4,CALLBACK)],
            ('free',0):[('g_memHandlesInitialized',0,4,1),('g_memHandleStatus',0,4,1)]}
        compare('Mem_ShutdownSystem',state,actions=actions)
        # Persistent startup, real allocation/registration, callback -> pool teardown,
        # repeated shutdown, default failure, and repeated startup orphaning.
        state=fresh(0)
        for s in (a,b):s.reset_state(state)
        def step(name,args=(),actions=None,returns=None):
            nonlocal state,persistent
            state=compare(name,state,args,actions=actions,returns=returns,reset=False);persistent+=1
        step('Mem_InitSystem',returns={'malloc':[ARENA]})
        step('Mem_Alloc',(0,123),returns={'malloc':[ARENA+0xc0000,ARENA+0xc1000,ARENA+0xc2000]})
        for key,value in [('g_memPendingCallback',CALLBACK),('g_memPendingParameter',0xffffffff)]:
            state[key]=struct.pack('<I',value)
        for s in (a,b):s.reset_state(state)  # Explicit client setup, same in both CPUs.
        step('Mem_RegisterHandle',(42,))
        step('Mem_ShutdownSystem',actions={('callback',0):[('g_memPendingContext',0,4,0x12345678)]})
        step('Mem_ShutdownSystem')
        step('Mem_InitSystem',returns={'malloc':[0]})
        step('Mem_ShutdownSystem')
        step('Mem_InitSystem',returns={'malloc':[ARENA+0x2000]})
        step('Mem_InitSystem',returns={'malloc':[ARENA]})
        step('Mem_ShutdownSystem')
        for name in ROUTINES:
            record(name,'emulation','pass',cases=counts[name],isolated_cases=isolated_counts[name],
                original_consumer_abi_cases=consumer_cases if name=='Gfx_InstallSurfaceDispatch' else None,
                persistent_invocations=(133 if name=='Gfx_InstallSurfaceDispatch' else 266 if name=='Gfx_SelectBackend' else 153 if name=='Lisa_PrintVersion' else
                    persistent if name in ('Mem_InitSystem','Mem_ShutdownSystem') else 0),
                persistent_integrated_startups=4 if name in ('Lisa_PrintVersion','Input_ResetCallbacks','Gfx_SelectBackend','Gfx_InstallSurfaceDispatch') else None,
                integrated_startups=18 if name in ('Input_ResetCallbacks','Lisa_PrintVersion','Gfx_SelectBackend','Gfx_InstallSurfaceDispatch') else None,
                scope=('266 standalone fourteen-store installers with arbitrary initial words and persistent repetition; real selector and 18 integrated startups; relocated function identity, exact ordered stores, untouched sprite/holes and full-state/ABI/image checks' if name=='Gfx_InstallSurfaceDispatch' else '532 standalone selectors: 266 nonzero calls without effects, 266 zero calls with real ordered surface installation and ignored sprite boundary returns/mutations; 18 integrated startups; ordered calls, full tracked state and boundary snapshots, EAX/stack/nonvolatile/DF/unrelated-image checks' if name=='Gfx_SelectBackend' else '306 standalone calls, exact NUL-terminated printf format/vararg bytes and order; independent return pairs, CRT boundary mutations, repeated persistent calls; 18 integrated startups; EAX=0, full tracked state, stack/nonvolatile/DF/unrelated-image checks' if name=='Lisa_PrintVersion' else '266 standalone ordered two-dword clearing calls, repeated persistent clearing; 18 integrated startups; complete tracked state and boundary snapshots; nonvolatile registers/stack/DF/unrelated-image checks; void EAX excluded' if name=='Input_ResetCallbacks' else 'Exact ordered calls/arguments and accesses; complete handles, roots, one-MiB heap and boundary-entry snapshots; ABI/stack/unrelated-image checks; real pool/handle initialization and teardown; 400 callback slot/pass cases, cross-phase mutation and persistent real allocation/registration'),
                limitations='Primitive initializer, sprite dispatch initializer, CRT printf/heap and callbacks modeled; no instruction equality, original linked layout, native startup/graphics/heap/game parity, invalid/aliased storage, concurrency or general reentry validation')
        print('PASS: %d Gfx_SelectBackend; %d Lisa_PrintVersion; %d Input_ResetCallbacks; %d Mem_InitSystem and %d Mem_ShutdownSystem differential invocations; isolated %s; %d persistent invocations.'%(counts['Gfx_SelectBackend'],counts['Lisa_PrintVersion'],counts['Input_ResetCallbacks'],counts['Mem_InitSystem'],counts['Mem_ShutdownSystem'],isolated_counts,persistent))
        print('PASS: %d standalone production surface installers; real selector/startup integration.'%counts['Gfx_InstallSurfaceDispatch'])
        return counts
    except Exception as exc:
        for name in ROUTINES:record(name,phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    verify_mem_lifecycle()
