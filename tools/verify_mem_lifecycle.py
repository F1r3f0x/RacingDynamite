# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Lifecycle wrappers: isolated dependency contracts and real memory integration."""
import hashlib
import random
import struct
import pefile
from windows_target import validate_platform_imports
from windows_target import validation_symbols
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_MEM_WRITE
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
    'Gfx_LinkSpritePackingNode': (0x61690,33,'698f357becf28ec039300d9e01f27d48f7a04c675486b3506e7ca6107dded174'),
    'Gfx_InitSpritePackingState': (0x614d0,61,'c7c2d85ffd985343137e2d89bee0193091d6bc304ebb041e360202275906b7e5'),
    'Gfx_InitDefaultSpriteDescriptor': (0x611d0,53,'bb24fdcef5b8b3fdd31746c899ca6523d467479ebc00098f5be527e9e414d70a'),
    'Gfx_InitSpriteWorkspaceA': (0x5d840,84,'445faf9112110b216014f95c057ab6d572a81fec3fa64a867b9d5f03739c1d43'),
    'Gfx_InitSpriteWorkspaceB': (0x5c9f0,84,'922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e'),
    'Gfx_InitSpriteHandles': (0x5c7f0,62,'41862be4869238f9f9261e170ab377306cd2aa18afbbd6761a59bf2f32a5b132'),
    'Gfx_CopySpriteDescriptor': (0x612e0,114,'75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858'),
    'Gfx_InstallSpriteDispatch': (0x56e60,181,'215bc3324a371904ec72c0e780eec9b178b47f01e195eaaac6124781c612a8b6'),
    'Gfx_InstallSurfaceDispatch': (0x5b690,146,'3cb7a08c993e9d2ce4db93a2ebe96e9165728ef6bae5b5014371ebaef3de41bf'),
    'Gfx_SelectBackend': (0x56af0,30,'d1fcb40ea924f27f8afef2a4b1e804ee70ce3fed2ff4f9e74a376877e7c3c4cd'),
    'Lisa_PrintVersion': (0x5b4f0,34,'719ae03224b3f4a4ab08fa52985cd65454730bd8de870f8afe632d9216951802'),
    'Input_ResetCallbacks': (0x55ab0,13,'c301c37754413d115bac22b922a9ed480b4080011c559a4485b7ea044270fd59'),
    'Mem_InitSystem': (0x5b170,41,'e4026b0791ef97b56f8fa7013dd568dfd4772d64e2638d850a217c9d338ce2b8'),
    'Mem_ShutdownSystem': (0x5b1a0,16,'75f6e3f589d50b5cb564113e1270c4f2a5df39c0614d41516b520d0d3ee064a3')}
DEPENDENCIES = {
    'Gfx_InitPrimitiveState': (0x5e610,326,'fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3')}
MEMORY = {'Mem_InitHandles':(0x5b1f0,76), 'Mem_ShutdownHandles':(0x5b240,155),
    'Mem_NextHandleId':(0x5b1b0,58), 'Mem_RegisterHandle':(0x5b360,120),
    'Mem_ReleaseHandleId':(0x5b410,62)}
ARGUMENTS = {'Gfx_LinkSpritePackingNode':3, 'Gfx_CopySpriteDescriptor':2, 'Gfx_SelectBackend':1, 'Mem_CreatePool':1, 'Mem_DestroyPool':1,
    'Mem_RegisterHandle':1, 'Mem_ReleaseHandleId':1, 'Mem_Alloc':2, 'Mem_Free':2,
    'malloc':1, 'free':1, 'callback':1}
CALLBACK = 0x7200000
PRIMITIVE_RETURN = CALLBACK + 0x100
DLL = BUILD / 'mem_lifecycle_validation.dll'
BANNER_STRINGS = {0x4bab20: b'Compilation 0.91.0',
    0x4bab5c: b'\nLisa 2 Development System, %s\n',
    0x4bab3c: b'Copyright (c) UDS, 1995-1996\n\n'}
BANNER_ARGS = [(BANNER_STRINGS[0x4bab5c], BANNER_STRINGS[0x4bab20]),
    (BANNER_STRINGS[0x4bab3c],)]
RESOURCE_FIELDS = [('g_inputKeyEventCallback',0x50de14,4), ('g_inputPollCallback',0x50e678,4)]
SURFACE_SLOTS = [('g_surfaceConfigure', 4568880), ('g_surfaceOpen', 4568896), ('g_surfaceRebuild', 4570480), ('g_surfaceShutdown', 4571232), ('g_surfaceConfigureSurface', 4571472), ('g_surfaceBlit', 4571488), ('g_surfaceCopyPixels', 4571600), ('g_surfaceClear', 4572112), ('g_surfacePresent', 4572336), ('g_surfaceReserved', 4572448), ('g_surfaceLock', 4572464), ('g_surfaceUnlock', 4572800), ('g_surfaceSetPalette', 4572880), ('g_surfaceRestore', 4572976)]
SPRITE_SLOTS = [('g_spriteOpen', 5303200, 4550432, 'Gfx_SpriteOpenNative'), ('g_spriteShutdown', 5303204, 4550448, 'Gfx_SpriteShutdownNative'), ('g_spriteClose', 5303208, 4550464, 'Gfx_SpriteCloseNative'), ('g_spriteOption', 5303212, 4550480, 'Gfx_SpriteOptionNative'), ('g_spriteConfigure', 5303216, 4550496, 'Gfx_SpriteConfigureNative'), ('g_spriteSetClip', 5303220, 4550688, 'Gfx_SpriteSetClipNative'), ('g_spriteDrawList', 5303224, 4551248, 'Gfx_SpriteDrawListNative'), ('g_spriteDraw', 5303228, 4551088, 'Gfx_DrawSpriteNative'), ('g_spriteHandleOp', 5303232, 4573232, 'Gfx_SpriteHandleOpNative'), ('g_spriteImageOp', 5303256, 4592480, 'Gfx_SpriteImageOpNative'), ('g_spriteCreateDescriptor', 5303252, 4573472, 'Gfx_SpriteCreateDescriptorNative'), ('g_spriteFreeDescriptor', 5303260, 4573504, 'Gfx_SpriteFreeDescriptorNative'), ('g_spriteCopyDescriptor', 5303264, 4573536, 'Gfx_SpriteCopyDescriptorNative'), ('g_spriteReserved', 5303268, 4594304, 'Gfx_SpriteReservedNative'), ('g_spriteGetState', 5303276, 4550928, 'Gfx_SpriteGetStateNative'), ('g_spriteSetState', 5303272, 4551024, 'Gfx_SpriteSetStateNative')]
SPRITE_CONSUMERS = [(355264, 19, 0, 0, '52be6802ced83d2ed0f4d075befb6cb8a27c422283385d5948efbf9807fee826'), (355312, 19, 0, 1, '12f76bae9f55a6539d23baf465bf52e0b2450cf8cc41121eb16f100448679412'), (355488, 6, 0, 2, 'f54576d17cf81a5b459c28177c5d638406200de58ced731d331e852d85c3b108'), (355504, 15, 1, 3, 'd5f47caf1c6baad1796a505d1ba148815dc490a6e2214fc8a00f3cc3233a4c64'), (355520, 35, 5, 4, '55920e775768dfba2721249cf60a49b924696a84bfe47a8ad80af5f2da278154'), (355568, 30, 4, 5, '9f7d76bab0660dc8db60d9fea1ab576779f890831a2109d76b88fd01dd461208'), (355600, 15, 1, 6, '12a4a53eb2752ffee638c4a6ad965e4e19bf62d9f28382d673ed854136bc9e0b'), (355616, 25, 3, 7, 'e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623'), (355648, 20, 2, 8, 'f76b5ff746f37d91b851c1a25161196baa69ffe94b4bfe1250a607da04aa97bc'), (355776, 20, 2, 14, 'a36dcdae6600a974858cc66f8c213e5b0e94e5aaa755c35fd54d6ea49daf9772'), (355808, 20, 2, 13, '4b892e967e307cfd490ce8c9e026a2c2f96f7ca77b1aceafccf2eb388847f4fc'), (355840, 15, 1, 15, 'ac56b05bb0bb49106928975f6c55be891221b8a15887fc446520ac74fcf4b15c'), (355856, 20, 2, 16, '4df461e4396b427c52bde31bb1c9b2c9173c009db8427aa2fd4cb41f0bdbc1ed'), (355888, 15, 1, 17, 'ed08e50628bdf08f53ad96b88e5d9dab86c6b337186a18f8d8e6951da1221f22'), (355904, 15, 1, 19, '9d4fe8577c937219b06bcef289111aecbca1ca42481219197dc5b00852d4b5e2'), (355920, 15, 1, 18, 'd4a6aa806bb86c3e10bc4475cdc896c4ece70748edf944be89dccc226f876fda')]
SPRITE_TARGETS = [(356128, 6, '2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67'), (356144, 6, '2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67'), (356160, 6, '2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67'), (356176, 3, '4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e'), (356192, 187, 'b9bcf21802d579d076091d7908499681c374a5786d4b5c46ce5d981135950c93'), (356384, 225, '223c2cd20c7f79c950d9fc173d170037f4bde1047b285142be51fe551173dbcb'), (356944, 96, '12d36bd728eefff1bcccf08ecd1e7a52c39ef5d1ce833f2baeee78a7ae52b50d'), (356784, 145, 'f2fe1b07e3a37fe080d3cfcaeadad8e3bae3d2ca326e16c4a99e6ac563e01dac'), (378928, 207, '77a47f976fb496d9826b327cdf81fa38f527ee1a1fb3ce7085124b6ab39a0ec7'), (398176, 329, '81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020'), (379168, 24, 'fd5cdd32124a607730e33cc3717641031757e7524d4bc8252ea6d7a0fe506b0e'), (379200, 19, '6fbd0f1c63dd28502523b19906d18ca89eee90c5df9045d4c2471ca7147d7795'), (379232, 24, '7f21b8803e5134922b22aefa7896ec5d43d3aa997dddb3ad20939b8639a7cfc4'), (400000, 3, '4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e'), (356624, 92, '9be413481f65ac603e7873f46b10cff7a09f8de06c3a0904b100e8b4f77a18ae'), (356720, 63, '635cc8612df0e75ae8e7760faa6cd17b0730605c52c19d15184a89158f2092c7')]
SPRITE_STORAGE = [('g_nativeSpriteFreeList',0x512c58,8000),
    ('g_nativeSpriteScratch',0x514b98,64), ('g_nativeSpriteHandles',0x514bd8,24000),
    ('g_nativeSpriteFreeCursor',0x51a998,4), ('g_nativeSpriteDefault',0x51fb88,64),
    ('g_nativeNextImageId',0x520380,4), ('g_nativeImageCapacity',0x520388,4), ('g_nativeImageRecords',0x52038c,4)]
WORKSPACE_FIELDS = [('g_spriteWorkspace'+side+field,va,4)
    for side,addresses in [('A',(0x4bace8,0x51aa50,0x51aa5c,0x51aa60,0x51aa64)),
                          ('B',(0x4bace4,0x51a9cc,0x51a9d8,0x51a9dc,0x51a9e0))]
    for field,va in zip(('Allocated','Count','Buffer0','Buffer1','Buffer2'),addresses)]
PACKING_FIELDS = [('g_spritePackingTemplate',0x51fc00,24),
    ('g_spritePackingBuckets',0x51ff58,1028),('g_spritePackingPages',0x51fe40,4)]
STATE_FIELDS = PACKING_FIELDS + WORKSPACE_FIELDS + SPRITE_STORAGE + BOOKKEEPING_FIELDS + RESOURCE_FIELDS + [(name,0x50eb68+4*i,4) for i,(name,_) in enumerate(SURFACE_SLOTS)] + [(name,va,4) for name,va,_,_ in SPRITE_SLOTS] + [('validation_gfxDispatchWords',0x50ebc4,16)]
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
MODELED_STARTUP = {'Gfx_InitPrimitiveState'}
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
        expected_fpo=(n,0,3,0) if name=='Gfx_LinkSpritePackingNode' else (n,0,0,0x103) if name=='Gfx_InitSpritePackingState' else (n,0,2,0x208) if name=='Gfx_CopySpriteDescriptor' else (n,0,1 if name=='Gfx_SelectBackend' else 0,0)
        assert fpo[r]==expected_fpo
        assert ins[-1].mnemonic==('jmp' if name=='Gfx_InitPrimitiveState' else 'ret')
    for name,(r,n,_) in ROUTINES.items():
        if name not in ('Mem_InitSystem','Mem_ShutdownSystem'):continue
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
    link=list(md.disasm(pe.get_data(0x61690,33),0x461690))
    assert [(i.mnemonic,i.op_str) for i in link]==[
        ('mov','edx, dword ptr [esp + 0xc]'),('mov','ecx, dword ptr [esp + 8]'),
        ('mov','eax, dword ptr [esp + 4]'),('mov','dword ptr [ecx + 0x14], edx'),
        ('mov','dword ptr [ecx + 0x10], eax'),('test','eax, eax'),
        ('je','0x4616a9'),('mov','dword ptr [eax + 0x14], ecx'),
        ('test','edx, edx'),('je','0x4616b0'),
        ('mov','dword ptr [edx + 0x10], ecx'),('ret','')]
    assert pe.get_data(0x616b1,15)==b'\xcc'*15
    assert not any(0x61690<=r<0x616b1 or v==0x461690 for r,v in relocations.items())
    assert [(r,i.address) for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic=='call' and i.op_str=='0x461690']==[
            (0x61530,0x461652),(0x616c0,0x46179f)]
    # Both callers supply (old node, new node, old next), clean three words,
    # and overwrite EAX before use. No caller body is executed by link tests.
    for r,n,lo,hi,expected in [
        (0x61530,350,0x46164c,0x461665,[
            ('mov','eax, dword ptr [edx + 0x14]'),('push','eax'),('push','ecx'),
            ('push','edx'),('call','0x461690'),('add','esp, 0xc'),
            ('lea','edi, [esp + 0x18]'),('mov','esi, ebx'),('mov','ecx, 0x10')]),
        (0x616c0,273,0x461799,0x4617ae,[
            ('mov','eax, dword ptr [ebp + 0x14]'),('push','eax'),('push','edx'),
            ('push','ebp'),('call','0x461690'),('add','esp, 0xc'),
            ('push','8'),('call','0x45f4a0')])]:
        assert [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(r,n),0x400000+r)
            if lo<=i.address<hi]==expected
    packing=list(md.disasm(pe.get_data(0x614d0,61),0x4614d0))
    assert [(i.mnemonic,i.op_str) for i in packing]==[
        ('push','edi'),('xor','edx, edx'),
        ('mov','dword ptr [0x51fc10], edx'),('mov','dword ptr [0x51fc14], edx'),
        ('mov','edi, 0x51ff58'),('mov','dword ptr [0x51fc0c], edx'),
        ('xor','eax, eax'),('mov','ecx, 0x101'),
        ('mov','dword ptr [0x51fc00], edx'),('mov','dword ptr [0x51fc04], edx'),
        ('mov','dword ptr [0x51fc08], edx'),('rep stosd','dword ptr es:[edi], eax'),
        ('pop','edi'),('mov','dword ptr [0x51fe40], edx'),('ret','')]
    assert pe.get_data(0x6150d,3)==b'\xcc'*3
    assert [(r,relocations[r]) for r in sorted(relocations) if 0x614d0<=r<0x6150d]==[
        (0x614d5,0x51fc10),(0x614db,0x51fc14),(0x614e0,0x51ff58),
        (0x614e6,0x51fc0c),(0x614f3,0x51fc00),(0x614f9,0x51fc04),
        (0x614ff,0x51fc08),(0x61508,0x51fe40)]
    assert [i.address for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic=='call' and i.op_str=='0x4614d0']==[0x45e73d]
    assert not any(v==0x4614d0 for v in relocations.values())
    for r,n,digest in [
        (0x61530,350,'028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394'),
        (0x616c0,273,'6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed'),
        (0x617e0,85,'3e028a97ebd2430f4181d83bd55f460261c9506827cddcee83adce2ea65650b9'),
        (0x618b0,419,'7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072')]:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest and fpo[r][0]==n
    page=list(md.disasm(pe.get_data(0x617e0,85),0x4617e0))
    assert [(i.mnemonic,i.op_str) for i in page if i.address in (0x4617e3,0x4617e8,0x4617f6,0x4617fb)]==[
        ('mov','esi, 0x51fc00'),('push','0x18'),('mov','ecx, 6'),
        ('rep movsd','dword ptr es:[edi], dword ptr [esi]')]
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
    for r,n,argc,slot,digest in SPRITE_CONSUMERS:
        assert fpo[r]==(n,0,argc,0)
        raw=pe.get_data(r,n);assert hashlib.sha256(raw).hexdigest()==digest
        instructions=list(md.disasm(raw,0x400000+r))
        assert sum(i.size for i in instructions)==n and instructions[-1].mnemonic in ('ret','jmp')
        assert any(i.mnemonic in ('call','jmp') and i.op_str=='dword ptr [%s]'%hex(0x50eba0+4*slot) for i in instructions)
        if argc:
            assert any(i.mnemonic=='add' and i.op_str=='esp, '+(hex(4*argc) if 4*argc>=10 else str(4*argc)) for i in instructions)
    for r,n,digest in SPRITE_TARGETS:
        raw=pe.get_data(r,n);assert fpo[r][0]==n and hashlib.sha256(raw).hexdigest()==digest
        instructions=list(md.disasm(raw,0x400000+r))
        assert sum(i.size for i in instructions)==n and instructions[-1].mnemonic=='ret'
    assert sprite==[(va,target) for _,va,target,_ in SPRITE_SLOTS]
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
    # Descriptor forwarding supplies pointer/scalar meaning for the typed slots.
    for r,n,argc,digest in [
        (0x61210,143,2,'d1ca418b53f65abb20e0ec43adeb04905cbd9b247a7c5c188818a512b3620dc9'),
        (0x614b0,31,1,'4b278fe6701f014a83c9ddb65ba7d69c2ca3c12814bedc44e9766dcd98408493'),
        (0x612e0,114,2,'75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858')]:
        assert fpo[r][0]==n and fpo[r][2]==argc
        raw=pe.get_data(r,n);assert hashlib.sha256(raw).hexdigest()==digest
        instructions=list(md.disasm(raw,0x400000+r))
        assert sum(i.size for i in instructions)==n and instructions[-1].mnemonic=='ret'
    # A representative caller passes a byte buffer, equal stride/width, height, 8.
    assert pe.get_data(0x18610,51)==bytes.fromhex(
        'c7050030550040010000c705103c5600c80000006a0868c80000006840010000684001000068b03d5600e881e6030083c414c3')
    assert pe.get_data(0xbace4,8)==bytes(8) # Two lazy allocation flags: file zero.
    # Full independently decoded workspace contracts, including every relocated
    # operand. Buffer contents/layout are deliberately outside these bodies.
    for side,r,count,flag,pointers in [('A',0x5d840,0x51aa50,0x4bace8,0x51aa5c),
                                    ('B',0x5c9f0,0x51a9cc,0x4bace4,0x51a9d8)]:
        instructions=list(md.disasm(pe.get_data(r,84),0x400000+r))
        assert len(instructions)==17 and pe.get_data(r+84,12)==b'\xcc'*12
        assert [(i.mnemonic,i.op_str) for i in instructions]==[
            ('mov',f'dword ptr [{hex(count)}], 0'),('cmp',f'dword ptr [{hex(flag)}], 0'),
            ('jne',hex(0x400000+r+83))] + [item for index in range(3) for item in [
                ('push','0x20d8'),('call','0x469400'),('add','esp, 4'),
                ('mov',f'dword ptr [{hex(pointers+4*index)}], eax')]] + [
            ('mov',f'dword ptr [{hex(flag)}], 1'),('ret','')]
        assert [(x,relocations[x]) for x in sorted(relocations) if r<=x<r+84]==[
            (r+2,count),(r+12,flag),(r+33,pointers),(r+51,pointers+4),
            (r+69,pointers+8),(r+75,flag)]
        for va in (count,pointers,pointers+4,pointers+8):
            assert data.VirtualAddress+data.SizeOfRawData<=va-0x400000<data.VirtualAddress+data.Misc_VirtualSize
        assert [x for x in sorted(relocations) if relocations[x]==flag]==[r+12,r+75]
        assert [x for x in sorted(relocations) if relocations[x]==count]==[r+2]
    # Independently recovered consumers establish buffer use, without declaring
    # a complete production buffer layout from allocation size or legacy types.
    consumers=[(0x5d8a0,1805,'78d4b4b04194828bc6b6683a6890568822a230369f947edda82cb6222c4640c4'),
        (0x5dfb0,1561,'739bf60339f6915a8460f79fdaeb474af0426aab014e2b386bd74c2e37ac433e'),
        (0x5ca50,1813,'4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf'),
        (0x5d170,1569,'d1d8d33f6f947f949ae7160d07da0fcc3e66d4a67168ad30c5eb80fa62f175f7')]
    for r,n,digest in consumers:
        raw=pe.get_data(r,n);instructions=list(md.disasm(raw,0x400000+r))
        assert fpo[r][0]==n and fpo[r][2]==1 and hashlib.sha256(raw).hexdigest()==digest
        assert sum(i.size for i in instructions)==n and instructions[-1].mnemonic=='ret'
    for side,r,start,pointers in [('A',0x5d840,0x5d8a0,0x51aa5c),('B',0x5c9f0,0x5ca50,0x51a9d8)]:
        ranges=consumers[:2] if side=='A' else consumers[2:]
        decoded=[i for cr,n,_ in ranges for i in md.disasm(pe.get_data(cr,n),0x400000+cr)]
        for index in range(3):
            refs=[x for x in sorted(relocations) if relocations[x]==pointers+4*index]
            assert len(refs)==(20 if index==0 else 17 if index==1 else 18)
            assert refs[0]==r+33+18*index
            for operand in refs[1:]:
                instruction=next(i for i in decoded if i.address-0x400000<=operand<i.address-0x400000+i.size)
                assert instruction.mnemonic=='mov' and instruction.op_str.split(', ')[1]==f'dword ptr [{hex(pointers+4*index)}]'
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
    # Default initialization and dynamically allocated pointer-table ownership.
    assert fpo[0x611d0]==(53,0,0,0)
    assert hashlib.sha256(pe.get_data(0x611d0,53)).hexdigest()=='bb24fdcef5b8b3fdd31746c899ca6523d467479ebc00098f5be527e9e414d70a'
    default=list(md.disasm(pe.get_data(0x611d0,53),0x4611d0))
    assert len(default)==10 and [(i.mnemonic,i.op_str) for i in default if i.mnemonic!='mov']==[
        ('xor','eax, eax'),('ret','')]
    assert [i.op_str for i in default if i.mnemonic=='mov']==[
        'dword ptr [0x51fb88], 0x4bacf8']+[
        'dword ptr [%s], eax'%hex(0x51fb88+o) for o in range(4,28,4)]+[
        'dword ptr [0x520380], 1']
    assert pe.get_data(0xbacf8,8)==b'default\0'
    assert pe.get_data(0x61205,11)==b'\xcc'*11
    assert [(r,relocations[r]) for r in sorted(relocations) if 0x611d0<=r<0x61205]==[
        (0x611d2,0x51fb88),(0x611d6,0x4bacf8),(0x611dd,0x51fb8c),
        (0x611e2,0x51fb90),(0x611e7,0x51fb94),(0x611ec,0x51fb98),
        (0x611f1,0x51fb9c),(0x611f6,0x51fba0),(0x611fc,0x520380)]
    assert [i.address for r,(n,_,_,_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic=='call' and i.op_str=='0x4611d0']==[0x45e738]
    assert not any(v==0x4611d0 for v in relocations.values())
    image_op=list(md.disasm(pe.get_data(0x61360,329),0x461360))
    assert [(i.address,i.mnemonic,i.op_str) for i in image_op if i.address in
        (0x461378,0x4613c8,0x4613d0,0x4613fb,0x46142a)]==[
        (0x461378,'mov','eax, dword ptr [0x520380]'),
        (0x4613c8,'cmp','dword ptr [0x520380], ebx'),
        (0x4613d0,'mov','dword ptr [0x520380], ebx'),
        (0x4613fb,'mov','ebx, dword ptr [0x520380]'),
        (0x46142a,'mov','dword ptr [0x520380], eax')]
    assert hashlib.sha256(pe.get_data(0x5f560,133)).hexdigest()=='c70cf873d3217422a169e38c5ba292d11bad39e619c1873c813d5830be610d82'
    grow=list(md.disasm(pe.get_data(0x5f560,133),0x45f560))
    assert [i.op_str for i in grow if i.mnemonic=='call']==['0x469400','0x4693b0']
    assert any(i.op_str=='ecx, [eax*4 + 0xfa0]' for i in grow)
    assert any(i.op_str=='dword ptr [esi + 8], 0x3e8' for i in grow)
    # The actual primitive reset writes capacity/table before calling default init.
    primitive=list(md.disasm(pe.get_data(0x5e610,326),0x45e610))
    assert [(i.mnemonic,i.op_str) for i in primitive if i.address in (0x45e728,0x45e72b,0x45e738)]==[
        ('mov','dword ptr [edx + 8], eax'),('mov','dword ptr [edx + 0xc], ecx'),('call','0x4611d0')]
    for _,va,n in SPRITE_STORAGE:
        assert data.VirtualAddress+data.SizeOfRawData<=va-0x400000
        assert va-0x400000+n<=data.VirtualAddress+data.Misc_VirtualSize
    assert [(r,relocations[r]) for r in sorted(relocations) if 0x612e0<=r<0x61352]==[
        (0x612eb,0x51fb88),(0x61301,0x520388),(0x61309,0x52038c),
        (0x61315,0x51fb88),(0x61340,0x51fb88)]
    assert [(r,relocations[r]) for r in sorted(relocations) if 0x5c7f0<=r<0x5c82e]==[
        (0x5c7f1,0x514bd8),(0x5c7f6,0x512c58),(0x5c7fc,0x51a998),
        (0x5c800,0x514b98),(0x5c813,0x514b98),(0x5c81c,0x514b98)]


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


def verify_sprite_consumer_abi(pe):
    """Original wrappers with entry sinks; no target graphics bodies execute."""
    cpu=Uc(UC_ARCH_X86,UC_MODE_32)
    cpu.mem_map(0x400000,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095)
    cpu.mem_write(0x400000,pe.get_memory_mapped_image())
    cpu.mem_map(STACK,0x10000);cpu.mem_map(STOP,0x1000);sp=STACK+0x8000
    def initialize(uc,address,length,unused):
        if address in (0x45d840,0x45c9f0,0x45c7f0):
            current=uc.reg_read(UC_X86_REG_ESP)
            uc.reg_write(UC_X86_REG_EAX,0)
            uc.reg_write(UC_X86_REG_ESP,current+4)
            uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(current,4))[0])
        else:assert 0x456e60<=address<0x456f15
    hook=cpu.hook_add(UC_HOOK_CODE,initialize)
    cpu.mem_write(sp,struct.pack('<I',STOP));cpu.reg_write(UC_X86_REG_ESP,sp)
    cpu.emu_start(0x456e60,STOP,count=100);cpu.hook_del(hook)
    assert cpu.reg_read(UC_X86_REG_EAX)==1
    cases=0;targets={va:target for _,va,target,_ in SPRITE_SLOTS}
    for r,n,argc,slot,_ in SPRITE_CONSUMERS:
        target=targets[0x50eba0+4*slot]
        for seed in (0,0x80000000,0xffffffff):
            args=tuple((seed+0x1020304*(i+1))&0xffffffff for i in range(argc));seen=[]
            surface=0x45b740 if slot==0 else 0x45c060 if slot==1 else None
            if surface:cpu.mem_write(0x50eb6c if slot==0 else 0x50eb74,struct.pack('<I',surface))
            def sink(uc,address,length,unused):
                if address in (target,surface):
                    current=uc.reg_read(UC_X86_REG_ESP)
                    actual=struct.unpack('<'+'I'*argc,uc.mem_read(current+4,argc*4)) if argc else ()
                    assert actual==args;seen.append(address)
                    uc.reg_write(UC_X86_REG_EAX,1 if address==surface else seed)
                    uc.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);uc.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
                    uc.reg_write(UC_X86_REG_ESP,current+4)
                    uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(current,4))[0])
                else:assert 0x400000+r<=address<0x400000+r+n
            hook=cpu.hook_add(UC_HOOK_CODE,sink)
            cpu.mem_write(sp,struct.pack('<'+'I'*(argc+1),STOP,*args));before=bytes(cpu.mem_read(sp,128))
            saved=[0x13572468+i for i in range(4)]
            for register,value in zip(PRESERVED,saved):cpu.reg_write(register,value)
            cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
            cpu.emu_start(0x400000+r,STOP,count=100)
            assert seen==([surface,target] if surface else [target])
            assert cpu.reg_read(UC_X86_REG_ESP)==sp+4 and cpu.reg_read(UC_X86_REG_EAX)==seed
            assert [cpu.reg_read(register) for register in PRESERVED]==saved
            assert bytes(cpu.mem_read(sp,128))==before and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
            cpu.hook_del(hook);cases+=1
    return cases


def verify_sprite_initializer_contracts(pe):
    """Retained original-only boundary checks, including helper-return independence."""
    from unicorn import UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
    cpu=Uc(UC_ARCH_X86,UC_MODE_32);size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
    cpu.mem_map(0x400000,size);cpu.mem_map(STACK,0x10000);cpu.mem_map(STOP,0x1000)
    image=pe.get_memory_mapped_image();sp=STACK+0x8000;cases=0
    for r,count,flag,pointers in [(0x5d840,0x51aa50,0x4bace8,0x51aa5c),(0x5c9f0,0x51a9cc,0x4bace4,0x51a9d8)]:
        for enabled in (0,1,2,0x80000000,0xffffffff):
            for returns in ((0,0,0),(0x7100000,0,0x7104000),(0,0x7102000,0),(0x7100000,0x7102000,0x7104000)):
                cpu.mem_write(0x400000,image);cpu.mem_write(flag,struct.pack('<I',enabled))
                cpu.mem_write(count,struct.pack('<I',0x12345678));cpu.mem_write(pointers,struct.pack('<3I',11,22,33))
                events=[];expected=[('write',count,4,0),('read',flag,4)]
                if enabled==0:
                    for i,value in enumerate(returns):expected += [('malloc',0x20d8),('write',pointers+4*i,4,value)]
                    expected += [('write',flag,4,1)]
                index=0
                def access(uc,kind,address,n,value,unused):
                    if STACK<=address<STACK+0x10000:return
                    events.append(('write',address,n,value) if kind==UC_MEM_WRITE else ('read',address,n))
                def boundary(uc,address,n,unused):
                    nonlocal index
                    if address==0x469400:
                        current=uc.reg_read(UC_X86_REG_ESP)
                        assert struct.unpack('<I',uc.mem_read(current+4,4))[0]==0x20d8
                        events.append(('malloc',0x20d8));uc.reg_write(UC_X86_REG_EAX,returns[index]);index+=1
                        uc.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);uc.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
                        uc.reg_write(UC_X86_REG_EFLAGS,0x43);uc.reg_write(UC_X86_REG_ESP,current+4)
                        uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(current,4))[0])
                    else:assert 0x400000+r<=address<0x400000+r+84
                hooks=[cpu.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,access),cpu.hook_add(UC_HOOK_CODE,boundary)]
                cpu.mem_write(sp,struct.pack('<I',STOP));stack=bytes(cpu.mem_read(sp,128));before=bytes(cpu.mem_read(0x400000,size))
                saved=[0x12340000+i for i in range(4)]
                for register,value in zip(PRESERVED,saved):cpu.reg_write(register,value)
                cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
                cpu.emu_start(0x400000+r,STOP,count=100)
                assert events==expected and index==(3 if enabled==0 else 0)
                assert cpu.reg_read(UC_X86_REG_ESP)==sp+4 and [cpu.reg_read(register) for register in PRESERVED]==saved
                assert bytes(cpu.mem_read(sp,128))==stack and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
                after=bytearray(cpu.mem_read(0x400000,size));oracle=bytearray(before)
                struct.pack_into('<I',oracle,count-0x400000,0)
                if enabled==0:
                    struct.pack_into('<3I',oracle,pointers-0x400000,*returns);struct.pack_into('<I',oracle,flag-0x400000,1)
                assert after==oracle
                for hook in hooks:cpu.hook_del(hook)
                cases+=1
    # Handle freelist: 2000 pointer words, 12-byte entries, only entry word zero cleared.
    for seed in (0,0x80000000,0xffffffff):
        cpu.mem_write(0x400000,image);rng=random.Random(seed)
        cpu.mem_write(0x512c58,rng.randbytes(8000));cpu.mem_write(0x514bd8,rng.randbytes(24000))
        before=bytes(cpu.mem_read(0x400000,size));expected=[];events=[]
        oracle=bytearray(before);struct.pack_into('<I',oracle,0x11a998,0x514b98)
        expected.append(('write',0x51a998,4,0x514b98))
        for i in range(2000):
            expected += [('write',0x512c58+4*i,4,0x514bd8+12*i),('write',0x514bd8+12*i,4,0)]
            struct.pack_into('<I',oracle,0x112c58+4*i,0x514bd8+12*i);struct.pack_into('<I',oracle,0x114bd8+12*i,0)
        def access(uc,kind,address,n,value,unused):
            if STACK<=address<STACK+0x10000:return
            events.append(('write',address,n,value) if kind==UC_MEM_WRITE else ('read',address,n))
        def boundary(uc,address,n,unused):
            if address==0x4612e0:
                current=uc.reg_read(UC_X86_REG_ESP)
                args=struct.unpack('<2I',uc.mem_read(current+4,8));assert args==(0x514b98,0)
                events.append(('descriptor',args))
                assert bytes(uc.mem_read(0x400000,size))==bytes(oracle)
                uc.reg_write(UC_X86_REG_EAX,seed);uc.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);uc.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
                uc.reg_write(UC_X86_REG_EFLAGS,0x43);uc.reg_write(UC_X86_REG_ESP,current+4)
                uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(current,4))[0])
            else:assert 0x45c7f0<=address<0x45c82e
        hooks=[cpu.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,access),cpu.hook_add(UC_HOOK_CODE,boundary)]
        cpu.mem_write(sp,struct.pack('<I',STOP));stack=bytes(cpu.mem_read(sp,128))
        saved=[0x12340000+i for i in range(4)]
        for register,value in zip(PRESERVED,saved):cpu.reg_write(register,value)
        cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
        cpu.emu_start(0x45c7f0,STOP,count=20000)
        assert events==expected+[('descriptor',(0x514b98,0))]
        assert cpu.reg_read(UC_X86_REG_EAX)==1 and cpu.reg_read(UC_X86_REG_ESP)==sp+4
        assert [cpu.reg_read(register) for register in PRESERVED]==saved and bytes(cpu.mem_read(sp,128))==stack
        assert bytes(cpu.mem_read(0x400000,size))==bytes(oracle) and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        for hook in hooks:cpu.hook_del(hook)
        cases+=1
    return cases


def verify_default_initializer(pe):
    """Original-only: prove seven-word initialization and nine-word preservation."""
    from verify_matching import execute
    fields=[('default',0x51fb88,64),('next_image_id',0x520380,4)]
    rng=random.Random(0x611d0)
    for k in range(16):
        initial=[rng.randbytes(64),rng.randbytes(4)]
        expected=bytearray(initial[0]);struct.pack_into('<7I',expected,0,0x4bacf8,0,0,0,0,0,0)
        events=[('write','default',0,4,0x4bacf8)]+[
            ('write','default',o,4,0) for o in range(4,28,4)]+[('write','next_image_id',0,4,1)]
        actual,_=execute(pe,0x4611d0,fields,initial,k,(0x4611d0,0x461205),expected_eax=0,expected_events=events)
        assert actual==[bytes(expected),struct.pack('<I',1)]
    return 16


def boundary_actions(name):
    # Only the independently inspected descriptor/table effects of primitive
    # startup are modeled here. Its other unreconstructed effects remain excluded.
    if name != 'Gfx_InitPrimitiveState':return []
    return [('g_nativeNextImageId',0,4,0),
        ('g_nativeImageCapacity',0,4,0),('g_nativeImageRecords',0,4,0)]


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
        for field,off,n,value in boundary_actions(name)+list(self.actions.get((name,index),())):
            self.state[field][off:off+n]=value.to_bytes(n,'little')
        if name=='Gfx_InitPrimitiveState':
            self.call('Gfx_InitDefaultSpriteDescriptor')
            self.call('Gfx_InitSpritePackingState')
        values=self.returns.get(name,())
        return values[index] if index<len(values) else 0xdeadbeef

    def call(self,name,args=()):
        if name!=self.top:self.events.append(('call',name,args))
        if name in MODELED_STARTUP or (self.isolated and name in ISOLATED_HELPERS):
            return self.boundary(name,args)
        if name=='Gfx_LinkSpritePackingNode':
            previous,current,next_node=args
            # Raw verified dword offsets, independent of production node fields.
            self.write('arena',current-ARENA+20,next_node)
            self.write('arena',current-ARENA+16,previous)
            if previous:self.write('arena',previous-ARENA+20,current)
            if next_node:self.write('arena',next_node-ARENA+16,current)
            return previous
        if name=='Gfx_InitSpritePackingState':
            for off in (16,20,12,0,4,8):self.write('g_spritePackingTemplate',off,0)
            for off in range(0,1028,4):self.write('g_spritePackingBuckets',off,0)
            self.write('g_spritePackingPages',0,0)
            return 0
        if name=='Gfx_InitDefaultSpriteDescriptor':
            self.write('g_nativeSpriteDefault',0,0x4bacf8)
            for off in range(4,28,4):self.write('g_nativeSpriteDefault',off,0)
            self.write('g_nativeNextImageId',0,1)
            return 0
        if name in ('Gfx_InitSpriteWorkspaceA','Gfx_InitSpriteWorkspaceB'):
            prefix='g_spriteWorkspace'+name[-1]
            self.write(prefix+'Count',0,0)
            if self.read(prefix+'Allocated')==0:
                for index in range(3):
                    pointer=self.call('malloc',(0x20d8,))
                    self.write(prefix+'Buffer'+str(index),0,pointer)
                self.write(prefix+'Allocated',0,1)
            return None # Original EAX is incidental; callers consume no result.
        if name=='Gfx_InitSpriteHandles':
            self.write('g_nativeSpriteFreeCursor',0,0x514b98)
            for i in range(2000):
                self.write('g_nativeSpriteFreeList',4*i,0x514bd8+12*i)
                self.write('g_nativeSpriteHandles',12*i,0)
            self.call('Gfx_CopySpriteDescriptor',(0x514b98,0));return 1
        if name=='Gfx_CopySpriteDescriptor':
            pointer,image_id=args
            signed=lambda v:v if v<0x80000000 else v-0x100000000
            source='g_nativeSpriteDefault';source_off=0;live=False
            if signed(image_id)>0 and signed(image_id)<signed(self.read('g_nativeImageCapacity')):
                table=self.read('g_nativeImageRecords')
                record=self.read('arena',table-ARENA+image_id*4)
                if record:source='arena';source_off=record-ARENA;live=True
            destination='g_nativeSpriteScratch' if pointer==0x514b98 else 'arena'
            destination_off=0 if pointer==0x514b98 else pointer-ARENA
            for off in range(0,64,4):
                value=self.read(source,source_off+off)
                self.write(destination,destination_off+off,value)
            if live:
                self.write(destination,destination_off+24,0)
                self.write(destination,destination_off+28,0)
                return 0
            return image_id
        if name=='Gfx_InstallSurfaceDispatch':
            for field,target in SURFACE_SLOTS:self.write(field,0,target)
            return 1
        if name=='Gfx_InstallSpriteDispatch':
            for field,_,target,_ in SPRITE_SLOTS:self.write(field,0,target)
            for dep in ('Gfx_InitSpriteWorkspaceA','Gfx_InitSpriteWorkspaceB','Gfx_InitSpriteHandles'):self.call(dep)
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
        self.symbols=symbols
        self.executed={}
        self.to_original = {} if symbols is None else {symbols['Gfx_Surface'+name[len('g_surface'):]+'Native']:va for name,va in SURFACE_SLOTS} | ({symbols[target]:va for _,_,va,target in SPRITE_SLOTS} if symbols else {})
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

    def canonical_word(self,name,value,off=0):
        if name in ('g_nativeSpriteDefault','g_nativeSpriteScratch') and off==0 and self.symbols and value==self.symbols['g_nativeSpriteDefaultName']:
            return 0x4bacf8
        if name=='g_nativeSpriteFreeList' and self.symbols:
            start=self.symbols['g_nativeSpriteHandles']
            if start<=value<start+24000:return 0x514bd8+value-start
        if name=='g_nativeSpriteFreeCursor' and self.symbols:
            start=self.symbols['g_nativeSpriteFreeList']
            if start<=value<=start+8000:return 0x512c58+value-start
        return self.to_original.get(value,value) if name.startswith(('g_surface','g_sprite')) and not name.startswith(('g_spriteWorkspace','g_spritePacking')) else value

    def translated_bytes(self,name,data,rebuilt=False):
        if name in ('g_nativeSpriteDefault','g_nativeSpriteScratch'):
            value=struct.unpack_from('<I',data)[0]
            if rebuilt and value==0x4bacf8:value=self.symbols['g_nativeSpriteDefaultName']
            elif not rebuilt:value=self.canonical_word(name,value)
            return struct.pack('<I',value)+data[4:]
        if (name.startswith(('g_surface','g_sprite')) and not name.startswith(('g_spriteWorkspace','g_spritePacking'))) or name in ('g_nativeSpriteFreeList','g_nativeSpriteFreeCursor'):
            values=struct.unpack('<'+'I'*(len(data)//4),data)
            if rebuilt:
                if name=='g_nativeSpriteFreeList':
                    values=[self.symbols['g_nativeSpriteHandles']+v-0x514bd8 if 0x514bd8<=v<0x51a998 else v for v in values]
                elif name=='g_nativeSpriteFreeCursor':
                    values=[self.symbols['g_nativeSpriteFreeList']+v-0x512c58 if 0x512c58<=v<=0x514b98 else v for v in values]
                else:values=[self.to_rebuilt.get(v,v) for v in values]
            else:values=[self.canonical_word(name,v) for v in values]
            return struct.pack('<'+'I'*len(values),*values)
        return data

    def state(self):
        return {'table':bytes(self.cpu.mem_read(self.table,1024)),
            'arena':bytes(self.cpu.mem_read(ARENA,ARENA_SIZE)),
            **{name:self.translated_bytes(name,bytes(self.cpu.mem_read(addr,n))) for name,addr,n in self.fields}}

    def reset_state(self,state):
        self.reset(state['table'],state['arena'])
        for name,addr,n in self.fields:
            data=state[name]
            if self.symbols:data=self.translated_bytes(name,data,rebuilt=True)
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
        if region:self.events.append(('write',*region,n,self.canonical_word(region[0],value,region[1])))

    def hook(self,cpu,address,n,unused):
        if address==PRIMITIVE_RETURN:
            target,result=self.primitive_return
            assert cpu.reg_read(UC_X86_REG_EAX)==0, 'Real primitive leaf result'
            if self.primitive_leaf==0:
                self.primitive_leaf=1
                sp=cpu.reg_read(UC_X86_REG_ESP)-4
                cpu.mem_write(sp,struct.pack('<I',PRIMITIVE_RETURN))
                cpu.reg_write(UC_X86_REG_ESP,sp)
                cpu.reg_write(UC_X86_REG_EIP,self.entries['Gfx_InitSpritePackingState'])
                return
            cpu.reg_write(UC_X86_REG_EAX,result);cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1)
            cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2);cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
            cpu.reg_write(UC_X86_REG_EIP,target);return
        name=self.by_address.get(address)
        if name:
            sp=cpu.reg_read(UC_X86_REG_ESP);argc=ARGUMENTS.get(name,0)
            args=struct.unpack('<'+'I'*argc,cpu.mem_read(sp+4,argc*4)) if argc else ()
            if name=='Gfx_CopySpriteDescriptor' and self.symbols and args[0]==self.symbols['g_nativeSpriteScratch']:
                args=(0x514b98,args[1])
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
            if not modeled and name in ROUTINES:self.executed[name]=self.executed.get(name,0)+1
            if modeled:
                assert self.boundary_index<len(self.expected_boundaries),'Unexpected boundary'
                expected_name,expected_args,state=self.expected_boundaries[self.boundary_index]
                assert (name,args)==(expected_name,expected_args)
                assert self.state()==state,'Boundary-entry state mismatch '+name
                self.boundary_index+=1;self.events.append(('boundary',name,args))
                index=self.indices.get(name,0);self.indices[name]=index+1
                addresses={key:addr for key,addr,length in self.fields}|{'table':self.table,'arena':ARENA}
                for field,off,length,value in boundary_actions(name)+list(self.actions.get((name,index),())):
                    cpu.mem_write(addresses[field]+off,value.to_bytes(length,'little'))
                values=self.returns.get(name,());result=values[index] if index<len(values) else 0xdeadbeef
                if name=='Gfx_InitPrimitiveState':
                    # Validation-only driver: model selected primitive reset effects,
                    # execute both real reset leaves in caller order, then model the return.
                    self.primitive_leaf=0
                    self.primitive_return=(struct.unpack('<I',cpu.mem_read(sp,4))[0],result)
                    cpu.mem_write(sp,struct.pack('<I',PRIMITIVE_RETURN))
                    cpu.reg_write(UC_X86_REG_EIP,self.entries['Gfx_InitDefaultSpriteDescriptor'])
                    return
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
        actual=self.state()
        assert actual==state, 'State mismatch '+str([(k,next((i for i,(x,y) in enumerate(zip(actual[k],v)) if x!=y),None)) for k,v in state.items() if actual[k]!=v])
        if eax is not None:assert cpu.reg_read(UC_X86_REG_EAX)==eax
        return state


def verify_mem_lifecycle():
    verify_target();phase='compilation';counts={name:0 for name in ROUTINES}
    try:
        original=pefile.PE(str(TARGET));inspect_original(original)
        consumer_cases=verify_surface_consumer_abi(original)
        sprite_consumer_cases=verify_sprite_consumer_abi(original)
        downstream_cases=verify_sprite_initializer_contracts(original)
        default_cases=verify_default_initializer(original)
        build(dll=DLL)
        for name in ROUTINES:record(name,'compilation','pass',scope='Complete production mem.c plus extracted production banner/input/selector/surface/sprite/handle/descriptor and both workspace bodies and packing link helper, thirty dispatch globals and independently recovered sprite/workspace storage; strict C89; provisional Clang/LLD; default initializer and primitive/CRT fixtures')
        rebuilt=pefile.PE(str(DLL));validate_platform_imports(rebuilt)
        symbols=validation_symbols(rebuilt)
        for name,(r,n,_) in ROUTINES.items():
            equal=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,n)==original.get_data(r,n)
            record(name,'raw_bytes','pass' if equal else 'different',scope='Raw prefix diagnostic only; instruction equality unverified')
        assert rebuilt.get_data(symbols['g_nativeSpriteDefaultName']-rebuilt.OPTIONAL_HEADER.ImageBase,8)==b'default\0'
        a,b=LifecycleSession(original),LifecycleSession(rebuilt,symbols);phase='emulation'
        rng=random.Random(0x5b170);isolated_counts={name:0 for name in ROUTINES};persistent=0
        def fresh(flag=0):
            state={name:rng.randbytes(n) for name,_,n in STATE_FIELDS}
            state.update(table=bytes(1024),arena=bytes(PoolFixture().data))
            state['g_memHandlesInitialized']=struct.pack('<I',flag)
            state['g_memHandleStatus']=bytes(800)
            state['g_nativeImageCapacity']=bytes(4);state['g_nativeImageRecords']=bytes(4)
            # Arbitrary unused pointer words exclude rebuilt-address collisions.
            state['g_nativeSpriteFreeList']=struct.pack('<2000I',*[rng.getrandbits(28) for _ in range(2000)])
            state['g_nativeSpriteFreeCursor']=struct.pack('<I',rng.getrandbits(28))
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
        # Every bucket including zero and 256, raw pointer bits, all template
        # fields and stale page roots. Persistent repeats execute without reset.
        packing_patterns=[bytes(1028),b'\xff'*1028,
            struct.pack('<257I',*([0x80000000]*257)),struct.pack('<257I',*range(257))]
        for index in range(257):
            table=bytearray(1028);struct.pack_into('<I',table,4*index,0xffffffff)
            packing_patterns.append(bytes(table))
        packing_patterns += [rng.randbytes(1028) for _ in range(128)]
        for pattern in packing_patterns:
            state=fresh();state['g_spritePackingBuckets']=pattern
            state=compare('Gfx_InitSpritePackingState',state)
            compare('Gfx_InitSpritePackingState',state,reset=False)
        # Client inserts a real fixture pointer between persistent resets; neither
        # initializer reads/frees/changes the pointed-to arena contents.
        for k in range(16):
            state=fresh();state['g_spritePackingPages']=struct.pack('<I',ARENA+0xe0000)
            state=compare('Gfx_InitSpritePackingState',state)
            state['g_spritePackingPages']=struct.pack('<I',ARENA+0xe1000)
            for session in (a,b):
                address=0x51fe40 if session.symbols is None else session.symbols['g_spritePackingPages']
                session.cpu.mem_write(address,state['g_spritePackingPages'])
            compare('Gfx_InitSpritePackingState',state,reset=False)
        # Arbitrary tails, including pointer-like bits, are preserved in repeated
        # real initialization. Capacity/table/search neighbors stay independent.
        default_patterns=[bytes(64),b'\xff'*64,struct.pack('<16I',*([0x80000000]*16)),
            struct.pack('<16I',*([0x4bacf8]*16))]+[rng.randbytes(64) for _ in range(128)]
        for pattern in default_patterns:
            state=fresh();state['g_nativeSpriteDefault']=pattern
            state['g_nativeImageCapacity']=rng.randbytes(4)
            state['g_nativeImageRecords']=rng.randbytes(4)
            state=compare('Gfx_InitDefaultSpriteDescriptor',state)
            compare('Gfx_InitDefaultSpriteDescriptor',state,reset=False)
        # Independent client edits before reinitialization and consumption by the
        # real copy helper/handle initializer; these are not extra startup calls.
        for k in range(16):
            state=fresh();tail=bytearray(state['g_nativeSpriteDefault'])
            struct.pack_into('<I',tail,28+4*(k%9),[0,1,0x80000000,0xffffffff][k%4])
            state['g_nativeSpriteDefault']=bytes(tail)
            state=compare('Gfx_InitDefaultSpriteDescriptor',state)
            expected=Oracle(state).invoke('Gfx_CopySpriteDescriptor',(0x514b98,0))
            for session in (a,b):session.invoke_case('Gfx_CopySpriteDescriptor',(session.symbols['g_nativeSpriteScratch'] if session.symbols else 0x514b98,0),expected,seed=k)
            state=expected[1]
            expected=Oracle(state).invoke('Gfx_InitSpriteHandles')
            for session in (a,b):session.invoke_case('Gfx_InitSpriteHandles',(),expected,seed=k)
        # Complete helper signed-ID contract. Table/record fixtures occupy unused
        # heap bytes and are not production substitutes or storage declarations.
        helper_specs=[(i,c,live) for c in [0,1,2,4,0x7fffffff,0x80000000,0xffffffff]
            for i in [0,1,2,3,4,0x7fffffff,0x80000000,0xffffffff]
            for live in [False,True]]
        helper_specs += [(rng.randrange(1,64),64,bool(k%2)) for k in range(128)]
        for image_id,capacity,live in helper_specs:
            state=fresh();heap=bytearray(state['arena'])
            state['g_nativeImageCapacity']=struct.pack('<I',capacity)
            valid=0<image_id<capacity<0x80000000
            state['g_nativeImageRecords']=struct.pack('<I',ARENA+0xe0000 if valid else 0xffffffff)
            if valid:
                struct.pack_into('<I',heap,0xe0000+4*image_id,ARENA+0xe1000 if live else 0)
            heap[0xe1000:0xe1040]=rng.randbytes(64)
            heap[0xe2000:0xe2040]=rng.randbytes(64)
            state['arena']=bytes(heap)
            state=compare('Gfx_CopySpriteDescriptor',state,(ARENA+0xe2000,image_id))
            compare('Gfx_CopySpriteDescriptor',state,(ARENA+0xe2000,image_id),reset=False)
        for image_id in [0,1,0xffffffff]:
            state=fresh();heap=bytearray(state['arena'])
            state['g_nativeImageCapacity']=struct.pack('<I',2)
            state['g_nativeImageRecords']=struct.pack('<I',ARENA+0xe0000)
            struct.pack_into('<I',heap,0xe0004,ARENA+0xe1000)
            heap[0xe1000:0xe1040]=rng.randbytes(64);state['arena']=bytes(heap)
            state=compare('Gfx_CopySpriteDescriptor',state,(ARENA+0xe1000,image_id))
            compare('Gfx_CopySpriteDescriptor',state,(ARENA+0xe1000,image_id),reset=False)
        for _ in range(24):
            state=compare('Gfx_InitSpriteHandles',fresh())
            compare('Gfx_InitSpriteHandles',state,reset=False)
        # All eight independent failure combinations, exact-zero/noncanonical
        # flags, stale pointers, persistent skip and forced-zero orphaning.
        workspace_persistent={side:0 for side in ('A','B')}
        for side in ('A','B'):
            routine='Gfx_InitSpriteWorkspace'+side;prefix='g_spriteWorkspace'+side
            patterns=[tuple(ARENA+0x80000+0x3000*i if mask&(1<<i) else 0 for i in range(3)) for mask in range(8)]
            for flag in (0,1,2,0x7fffffff,0x80000000,0xffffffff):
                for pointers in patterns:
                    state=fresh();state[prefix+'Allocated']=struct.pack('<I',flag)
                    for i in range(3):state[prefix+'Buffer'+str(i)]=struct.pack('<I',[0,0x80000000,0xffffffff][i])
                    state=compare(routine,state,returns={'malloc':pointers})
                    compare(routine,state,returns={'malloc':(0,0,0)},reset=False)
                    workspace_persistent[side]+=1
            for _ in range(32):
                state=fresh();state[prefix+'Allocated']=bytes(4)
                pointers=tuple(rng.getrandbits(32) for i in range(3))
                # The allocator may mutate a flag/pointer/count between calls;
                # the already-taken branch is not rechecked or rolled back.
                actions={('malloc',0):[(prefix+'Allocated',0,4,0xffffffff),
                    (prefix+'Count',0,4,rng.getrandbits(32))],
                    ('malloc',1):[(prefix+'Buffer0',0,4,rng.getrandbits(32))]}
                state=compare(routine,state,actions=actions,returns={'malloc':pointers})
                compare(routine,state,reset=False);workspace_persistent[side]+=1
            state=fresh();state[prefix+'Allocated']=bytes(4)
            state=compare(routine,state,returns={'malloc':patterns[-1]})
            for i in range(3):
                # Explicit client clears flag; old allocations are overwritten
                # without free. Contents of every arena byte stay unchanged.
                state[prefix+'Allocated']=bytes(4)
                state=compare(routine,state,returns={'malloc':patterns[i]})
                compare(routine,state,reset=False);workspace_persistent[side]+=1
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
            state=fresh();state['g_spriteWorkspaceBAllocated']=bytes(4)
            actions={('malloc',2):[
                ('g_surfaceOpen',0,4,value),('validation_gfxDispatchWords',12,4,value),
                ('g_inputPollCallback',0,4,value)]}
            state=compare('Gfx_SelectBackend',state,(0,),actions=actions,
                returns={'malloc':[0,value,0]})
            compare('Gfx_SelectBackend',state,(0,),reset=False)
        sprite_pairs=[(x,y) for x in [0,1,2,0x80000000,0xffffffff] for y in [0,1,2,0x80000000,0xffffffff]]
        sprite_pairs += [(rng.getrandbits(32),rng.getrandbits(32)) for _ in range(128)]
        for x,y in sprite_pairs:
            state=fresh()
            state['g_spriteWorkspaceAAllocated']=bytes(4);state['g_spriteWorkspaceBAllocated']=bytes(4)
            for field,_,_,_ in SPRITE_SLOTS:state[field]=struct.pack('<I',x)
            actions={('malloc',0):[('g_spriteOpen',0,4,x),('g_surfaceOpen',0,4,y)],
                ('malloc',3):[('g_spriteSetState',0,4,y),('validation_gfxDispatchWords',0,4,x),('g_inputPollCallback',0,4,y)]}
            returns={'malloc':[x,0,y,y,0,x]}
            state=compare('Gfx_InstallSpriteDispatch',state,actions=actions,returns=returns)
            compare('Gfx_InstallSpriteDispatch',state,reset=False)
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
                state['g_spriteWorkspaceAAllocated']=struct.pack('<I',flag)
                state['g_spriteWorkspaceBAllocated']=struct.pack('<I',[0xffffffff,0,0x80000000,2,1][[0,1,2,0x80000000,0xffffffff].index(flag)])
                compare('Mem_InitSystem',state,returns={'malloc':[pointer,0,ARENA+0x80000,0,ARENA+0x83000,0,ARENA+0x86000]})
        # Changes made by early/late startup boundaries are observed by real helpers.
        for flag in [0,1,2,0xffffffff]:
            actions={
                ('printf',0):[('g_memHandlesInitialized',0,4,flag),
                    ('g_inputKeyEventCallback',0,4,0xffffffff),('g_inputPollCallback',0,4,0x12345678)],
                ('printf',1):[('g_inputPollCallback',0,4,0x87654321)],
                ('Gfx_InitPrimitiveState',0):[('g_memPendingCallback',0,4,CALLBACK),
                    ('g_spriteWorkspaceAAllocated',0,4,0),('g_spriteWorkspaceBAllocated',0,4,0)],
                ('malloc',4):[('g_memHandlesInitialized',0,4,0)]}
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
        state['g_spriteWorkspaceAAllocated']=bytes(4);state['g_spriteWorkspaceBAllocated']=bytes(4)
        for s in (a,b):s.reset_state(state)
        def step(name,args=(),actions=None,returns=None):
            nonlocal state,persistent
            state=compare(name,state,args,actions=actions,returns=returns,reset=False);persistent+=1
        step('Mem_InitSystem',returns={'malloc':[ARENA,ARENA+0x80000,0,ARENA+0x83000,0,ARENA+0x86000,0]})
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
        # Three adjacent 24-byte nodes; all 48 argument combinations include
        # null neighbors, distinct nodes, every pair alias and the triple alias.
        # Arbitrary link words are never followed/read. Full arena checks retain
        # range/pixel/child words, absent-neighbor fields and surrounding bytes.
        link_rng=random.Random(0x461690)
        nodes=tuple(ARENA+0x90000+24*i for i in range(3))
        link_patterns=[bytes(72),b'\xff'*72,struct.pack('<18I',*([0x80000000]*18)),
            struct.pack('<18I',*range(18))]+[link_rng.randbytes(72) for _ in range(4)]
        for pattern in link_patterns:
            for current in nodes:
                for previous in (0,)+nodes:
                    for next_node in (0,)+nodes:
                        state=fresh();arena=bytearray(state['arena'])
                        arena[0x90000:0x90000+72]=pattern;state['arena']=bytes(arena)
                        args=(previous,current,next_node)
                        state=compare('Gfx_LinkSpritePackingNode',state,args)
                        compare('Gfx_LinkSpritePackingNode',state,args,reset=False)
        # Persistent client-driven relinking is explicit, without invented
        # packing caller/reset integration or a reset between the six calls.
        for _ in range(16):
            state=fresh();arena=bytearray(state['arena'])
            arena[0x90000:0x90000+72]=link_rng.randbytes(72);state['arena']=bytes(arena)
            x,y,z=nodes
            for index,args in enumerate([(0,x,y),(x,y,z),(y,z,0),(z,x,z),(x,x,x),(0,y,0)]):
                state=compare('Gfx_LinkSpritePackingNode',state,args,reset=index==0)
        assert a.executed==b.executed
        for name in ROUTINES:
            record(name,'emulation','pass',cases=counts[name],isolated_cases=isolated_counts[name],
                real_body_invocations=a.executed.get(name,0),
                modeled_primitive_driver_invocations=112 if name in ('Gfx_InitDefaultSpriteDescriptor','Gfx_InitSpritePackingState') else None,
                real_default_consumption_chains=16 if name=='Gfx_InitDefaultSpriteDescriptor' else None,
                original_consumer_abi_cases=consumer_cases if name=='Gfx_InstallSurfaceDispatch' else sprite_consumer_cases if name=='Gfx_InstallSpriteDispatch' else None,
                original_default_initializer_cases=default_cases if name in ('Gfx_InitSpriteHandles','Gfx_CopySpriteDescriptor') else None,
                original_downstream_contract_cases=downstream_cases if name=='Gfx_InstallSpriteDispatch' else None,
                persistent_invocations=(464 if name=='Gfx_LinkSpritePackingNode' else 405 if name=='Gfx_InitSpritePackingState' else 132 if name=='Gfx_InitDefaultSpriteDescriptor' else workspace_persistent[name[-1]] if name.startswith('Gfx_InitSpriteWorkspace') else 153 if name=='Gfx_InstallSpriteDispatch' else 133 if name=='Gfx_InstallSurfaceDispatch' else 266 if name=='Gfx_SelectBackend' else 153 if name=='Lisa_PrintVersion' else
                    persistent if name in ('Mem_InitSystem','Mem_ShutdownSystem') else 24 if name=='Gfx_InitSpriteHandles' else 243 if name=='Gfx_CopySpriteDescriptor' else 0),
                persistent_integrated_startups=0 if name=='Gfx_LinkSpritePackingNode' else 4 if name not in ('Mem_InitSystem','Mem_ShutdownSystem') else None,
                integrated_startups=0 if name=='Gfx_LinkSpritePackingNode' else 18 if name not in ('Mem_InitSystem','Mem_ShutdownSystem') else None,
                scope=('864 standalone link invocations: 384 initial cases, 384 persistent repeats and 16 six-call persistent relinking sequences (80 later calls); all 48 null/distinct/exact-alias configurations, eight node patterns, exact ordered two-to-four dword stores, no node reads/calls, unrelated node fields/complete arena and state preservation, previous-pointer EAX, stack/nonvolatile/DF; no packing caller integration' if name=='Gfx_LinkSpritePackingNode' else '810 standalone resets, 405 persistent repeats, all 257 buckets and raw pointer states, exact 264-store order/no reads, template layout/page/bucket independence, persistent root reinsertion without free, EAX=0, complete state/arena/ABI; 112 startups through modeled primitive driver executing real reset leaves in caller order; primitive body unexecuted' if name=='Gfx_InitSpritePackingState' else '280 standalone real initializations, 132 persistent repeats, arbitrary nine-word tails, exact eight ordered stores, no reads, EAX=0; sixteen real helper/handle consumption chains; modeled primitive driver executes real default body in 112 startups; primitive body is not executed' if name=='Gfx_InitDefaultSpriteDescriptor' else '167 standalone workspace invocations including 83 persistent repeats, every independent allocation failure, exact-zero/noncanonical flags, immediate stores, skipped pointer preservation, allocator mutations and forced-zero orphaning; both real workspaces through installer/selector/18 startups; full state/access/ABI checks; void EAX excluded' if name.startswith('Gfx_InitSpriteWorkspace') else 'Sixteen-word copies with every signed ID/default/null/live branch, conditional ownership-slot clearing, source preservation and EAX; real helper through handles/installers/selector/startup, complete state/access/ABI checks' if name=='Gfx_CopySpriteDescriptor' else '2000 interleaved freelist stores/first-dword clears, tail preservation, real descriptor helper, persistent reinitialization and real installer/selector/startup integration' if name=='Gfx_InitSpriteHandles' else '306 standalone sixteen-store sprite installers, exact nonmonotonic store and downstream call order; independent return pairs and boundary mutations with persistent repeats; real surface/selector/18 startup integration; full-state/ABI/image checks; original-only consumer and downstream contracts' if name=='Gfx_InstallSpriteDispatch' else '266 standalone fourteen-store installers with arbitrary initial words and persistent repetition; real selector and 18 integrated startups; relocated function identity, exact ordered stores, untouched sprite words/holes during surface-only calls and full-state/ABI/image checks' if name=='Gfx_InstallSurfaceDispatch' else '532 standalone selectors: 266 nonzero calls without effects, 266 zero calls with real ordered surface installation and real sprite installation and ignored downstream returns/mutations; 18 integrated startups; ordered calls, full tracked state and boundary snapshots, EAX/stack/nonvolatile/DF/unrelated-image checks' if name=='Gfx_SelectBackend' else '306 standalone calls, exact NUL-terminated printf format/vararg bytes and order; independent return pairs, CRT boundary mutations, repeated persistent calls; 18 integrated startups; EAX=0, full tracked state, stack/nonvolatile/DF/unrelated-image checks' if name=='Lisa_PrintVersion' else '266 standalone ordered two-dword clearing calls, repeated persistent clearing; 18 integrated startups; complete tracked state and boundary snapshots; nonvolatile registers/stack/DF/unrelated-image checks; void EAX excluded' if name=='Input_ResetCallbacks' else 'Exact ordered calls/arguments and accesses; complete handles, roots, one-MiB heap and boundary-entry snapshots; ABI/stack/unrelated-image checks; real pool/handle initialization and teardown; 400 callback slot/pass cases, cross-phase mutation and persistent real allocation/registration'),
                workspace_contract=('Exact-zero guard, unconditional count reset, three ordered 0x20d8 allocations/immediate stores including all independent failures, noncanonical flags, skipped pointer preservation, persistent repetition/forced-zero orphaning and allocator mutations; no EAX result contract' if name.startswith('Gfx_InitSpriteWorkspace') else None),
                limitations='Primitive initializer, CRT printf/heap and callbacks modeled; workspace buffers opaque, consumers unreconstructed; no instruction equality, original linked layout, native startup/graphics/heap/game parity, invalid/unmapped storage, partial overlap, concurrency or general reentry validation; exact live source/destination alias tested')
        print('PASS: %d Gfx_SelectBackend; %d Lisa_PrintVersion; %d Input_ResetCallbacks; %d Mem_InitSystem and %d Mem_ShutdownSystem differential invocations; isolated %s; %d persistent invocations.'%(counts['Gfx_SelectBackend'],counts['Lisa_PrintVersion'],counts['Input_ResetCallbacks'],counts['Mem_InitSystem'],counts['Mem_ShutdownSystem'],isolated_counts,persistent))
        print('PASS: %d packing links; 464 persistent invocations; no packing caller integration.'%counts['Gfx_LinkSpritePackingNode'])
        print('PASS: %d sprite packing resets; 405 persistent repeats; real primitive-driver integration.'%counts['Gfx_InitSpritePackingState'])
        print('PASS: %d default initializer comparisons; 132 persistent repeats; real modeled-driver integration.'%counts['Gfx_InitDefaultSpriteDescriptor'])
        print('PASS: %d descriptor helpers and %d handle initializers with real integration.'%(counts['Gfx_CopySpriteDescriptor'],counts['Gfx_InitSpriteHandles']))
        print('PASS: workspace standalone counts %s; real body counts %s.'%(
            {n:counts[n] for n in ROUTINES if n.startswith('Gfx_InitSpriteWorkspace')},a.executed))
        print('PASS: %d standalone production surface installers; real selector/startup integration.'%counts['Gfx_InstallSurfaceDispatch'])
        print('PASS: %d standalone production sprite installers; %d original-only sprite consumer ABI and %d downstream contract cases.'%(counts['Gfx_InstallSpriteDispatch'],sprite_consumer_cases,downstream_cases))
        return counts
    except Exception as exc:
        for name in ROUTINES:record(name,phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    verify_mem_lifecycle()
