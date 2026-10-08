# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Execute original Windows instructions versus compiled C in x86 emulation.

Includes font initialization/cleanup with actual handle allocation, registration
and release, plus persistent lifecycle differential checks.
Instruction equality and native game runtime are reported separately.
"""
import hashlib
import random
import struct
import subprocess
import shutil
import unicorn
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
    UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS,
    UC_X86_REG_EIP, UC_X86_REG_ECX, UC_X86_REG_EDX)
from build_decomp import build
from windows_target import TARGET, DLL, ROUTINE_SHA256, verify_target, ROOT
from windows_tracking import record_run

TRACKING_INPUTS = ['decomp/src/mem.c', 'decomp/include/mem.h', 'decomp/target.json',
                   'tools/build_decomp.py', 'tools/verify_matching.py',
                   'tools/windows_target.py', 'tools/windows_tracking.py']

VALIDATING_RVA = 0x5b1f0

def record_result(kind, outcome, cases=0, details=None):
    record_run(VALIDATING_RVA, kind, outcome, inputs=TRACKING_INPUTS,
               artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
               cases=cases, command='uv run tools/verify_matching.py', details=details or {})

if not __debug__:
    raise RuntimeError('Verification requires assertions; disable -O/PYTHONOPTIMIZE')

FIELDS = [('g_memHandlesInitialized', 0x4bab38, 4),
          ('g_memHandleStatus', 0x5116e0, 800),
          ('g_memHandleIds', 0x511230, 400),
          ('g_memHandleCursor', 0x512040, 2)]
BOOKKEEPING_FIELDS = FIELDS + [
    ('g_memHandleContexts',0x510bf0,800),
    ('g_memHandleParameters',0x510f10,800),
    ('g_memRegisteredHandleIds',0x5113c0,800),
    ('g_memHandleCallbacks',0x511a00,800),
    ('g_memHandleFlags',0x511d20,800),
    ('g_memPendingContext',0x63c690,4),
    ('g_memPendingCallback',0x63c694,4),
    ('g_memPendingParameter',0x63c698,4)]
STACK, STOP = 0x7000000, 0x7100000
CALLBACK, CALLBACK_RETURN = 0x7200000, 0x7200100
PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

def execute(pe, entry, fields, values, seed, extent, expected_eax=1, extra_words=(), expected_read=None,
            stack_args=(), expected_events=None, callback_code=None, callback_handler=None):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, size)
    uc.mem_write(base, pe.get_memory_mapped_image())
    # Negative cursors need mapped signed words before the compiled table.
    # This is harness memory, not an invented reconstruction data structure.
    uc.mem_map(base-0x10000, 0x10000)
    # Surrounding writable bytes expose overruns; neither leaf uses import slots.
    for section in pe.sections:
        if section.Characteristics & 0x80000000:
            uc.mem_write(base+section.VirtualAddress,
                         bytes([0xa5]) * section.Misc_VirtualSize)
    for (_, addr, length), value in zip(fields, values):
        assert len(value) == length
        uc.mem_write(addr, value)
    for address, value in extra_words:
        uc.mem_write(address, struct.pack('<h', value))
    uc.mem_map(STACK, 0x10000)
    uc.mem_map(STOP, 0x1000)
    if callback_handler is not None:
        # Hook entry bytes are copied from an authenticated original callback,
        # solely to provide decodable mapped addresses. No fixture instruction
        # is executed: callback semantics are explicit Python test models.
        uc.mem_map(CALLBACK,0x1000)
        uc.mem_write(CALLBACK,callback_code)
        uc.mem_write(CALLBACK_RETURN,callback_code)
    sp = STACK + 0x8000
    uc.mem_write(sp, struct.pack('<I', STOP))
    if stack_args:
        uc.mem_write(sp+4,struct.pack('<'+'I'*len(stack_args),*stack_args))
    caller_stack = bytes(uc.mem_read(sp,STACK+0x10000-sp))
    rng = random.Random(seed)
    registers = [rng.getrandbits(32) for _ in PRESERVED]
    for reg, value in zip(PRESERVED, registers):
        uc.reg_write(reg, value)
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_EFLAGS, 2)  # Windows x86 ABI: direction flag clear.
    before = bytes(uc.mem_read(base, size))
    prefix_before = bytes(uc.mem_read(base-0x10000,0x10000))
    writes = []
    reads = []
    events = []
    callback_frames = []
    def event(kind,address,length,value):
        if STACK <= address < STACK+0x10000:
            return
        for name,addr,n in fields:
            if addr <= address and address+length <= addr+n:
                events.append((kind,name,address-addr,length,value & ((1 << (8*length))-1)))
                return
        if expected_events is not None:
            raise AssertionError(f'Unexpected data {kind} at {address:#x}')
    def code_hook(cpu, address, length, unused):
        def return_from_callback():
            callback_sp = cpu.reg_read(UC_X86_REG_ESP)
            target = struct.unpack('<I',cpu.mem_read(callback_sp,4))[0]
            assert extent[0] <= target < extent[1], 'Callback did not originate in routine'
            cpu.reg_write(UC_X86_REG_EAX,0xdeadbeef)
            cpu.reg_write(UC_X86_REG_ECX,0xa1b2c3d4)
            cpu.reg_write(UC_X86_REG_EDX,0x87654321)
            cpu.reg_write(UC_X86_REG_ESP,callback_sp+4)
            cpu.reg_write(UC_X86_REG_EIP,target)
        if callback_handler is not None and address == CALLBACK:
            callback_sp = cpu.reg_read(UC_X86_REG_ESP)
            parameter = struct.unpack('<I',cpu.mem_read(callback_sp+4,4))[0]
            events.append(('callback','Mem_ShutdownCallback',0,4,parameter))
            reenter = callback_handler(cpu,fields,parameter,events)
            if reenter:
                callback_frames.append(callback_sp)
                cpu.mem_write(callback_sp-4,struct.pack('<I',CALLBACK_RETURN))
                cpu.reg_write(UC_X86_REG_ESP,callback_sp-4)
                cpu.reg_write(UC_X86_REG_EIP,entry)
            else:
                return_from_callback()
            return
        if callback_handler is not None and address == CALLBACK_RETURN:
            assert cpu.reg_read(UC_X86_REG_EAX)==1, 'Reentrant shutdown return'
            assert callback_frames and cpu.reg_read(UC_X86_REG_ESP)==callback_frames.pop()
            return_from_callback()
            return
        if not (extent[0] <= address < extent[1]):
            raise AssertionError('Execution escaped bounded routine')
    def write_hook(cpu, access, address, length, value, unused):
        if STACK <= address and address+length <= STACK+0x10000:
            return
        if not any(addr <= address and address+length <= addr+n for _,addr,n in fields):
            raise AssertionError(f'Unexpected write at {address:#x}, length {length}')
        writes.append((address, length, value))
        event('write',address,length,value)
    def read_hook(cpu,access,address,length,value,unused):
        reads.append((address,length))
        if expected_events is not None:
            event('read',address,length,int.from_bytes(cpu.mem_read(address,length),'little'))
    uc.hook_add(UC_HOOK_CODE, code_hook)
    uc.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    uc.hook_add(UC_HOOK_MEM_READ, read_hook)
    uc.emu_start(entry, STOP, count=10000)
    assert uc.reg_read(UC_X86_REG_EIP) == STOP, 'Did not return within instruction limit'
    assert not callback_frames, 'Reentrant invocation did not finish'
    assert uc.reg_read(UC_X86_REG_EAX) == expected_eax & 0xffffffff
    assert uc.reg_read(UC_X86_REG_ESP) == sp+4
    assert bytes(uc.mem_read(sp,STACK+0x10000-sp)) == caller_stack
    assert [uc.reg_read(r) for r in PRESERVED] == registers
    assert not (uc.reg_read(UC_X86_REG_EFLAGS) & 0x400)
    after = bytearray(uc.mem_read(base, size))
    result = [bytes(uc.mem_read(addr,n)) for _,addr,n in fields]
    for _,addr,n in fields:
        after[addr-base:addr-base+n] = before[addr-base:addr-base+n]
    assert bytes(after) == before, 'Changed unrelated image memory'
    assert bytes(uc.mem_read(base-0x10000,0x10000)) == prefix_before
    if expected_read is not None:
        # Refused consumption must not read an ID; enabled consumption reads
        # exactly one signed word at the original effective address.
        word_reads = [(a,n) for a,n in reads if n==2 and a!=fields[3][1]]
        assert word_reads == expected_read, word_reads
        expected_data_reads = [(fields[0][1],4)]
        if struct.unpack('<I',values[0])[0] != 0:
            expected_data_reads.append((fields[3][1],2))
        expected_data_reads += expected_read
        assert [(a,n) for a,n in reads if not STACK <= a < STACK+0x10000] == expected_data_reads
    for address, value in extra_words:
        assert bytes(uc.mem_read(address,2)) == struct.pack('<h',value)
    if expected_events is not None:
        if events != expected_events:
            mismatch = next((i for i,(a,b) in enumerate(zip(events,expected_events)) if a!=b),
                            min(len(events),len(expected_events)))
            raise AssertionError(f'Data-event mismatch at {mismatch}: '
                f'actual {events[mismatch:mismatch+2]}, expected {expected_events[mismatch:mismatch+2]}; '
                f'event counts {len(events)}/{len(expected_events)}')
    return result, writes

def verify():
    global VALIDATING_RVA
    VALIDATING_RVA = 0x5b1f0
    verify_target()
    build()  # Always fresh; no --skip-build and no stale DOS inputs.
    original, rebuilt = pefile.PE(str(TARGET)), pefile.PE(str(DLL))
    assert rebuilt.FILE_HEADER.Machine == 0x14c
    assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT'), 'Unexpected dependencies'
    symbols = {e.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+e.address
               for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    fields = [(name, symbols[name], n) for name,_,n in FIELDS]
    print('Validation globals:', fields)
    original_code = original.get_data(0x5b1f0, 76)
    assert hashlib.sha256(original_code).hexdigest() == ROUTINE_SHA256
    text = next(s for s in rebuilt.sections if s.Name.rstrip(b'\0') == b'.text')
    lo = rebuilt.OPTIONAL_HEADER.ImageBase+text.VirtualAddress
    extent = (lo, lo+text.Misc_VirtualSize)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(original_code, 0x45b1f0))
    assert sum(i.size for i in instructions) == 76
    generated_code = text.get_data()[:text.Misc_VirtualSize]
    generated = list(md.disasm(generated_code, lo))
    direct_calls = [i for i in generated if i.mnemonic == 'call' and i.op_str.startswith('0x')]
    code_entries = {name: symbols[name] for name in [
        'Mem_DestroyPool', 'Mem_ShutdownPools', 'Mem_InitPools', 'Mem_CreatePool', 'Mem_Free', 'Mem_Alloc', 'Mem_ReleaseHandleId', 'Mem_ShutdownHandles',
        'Mem_RegisterHandle', 'Mem_NextHandleId', 'Mem_InitHandles', 'free', 'malloc']}
    assert len(direct_calls) == 13, 'Unexpected direct helper dependency count'
    owners = []
    for call in direct_calls:
        owner = max((name for name, address in code_entries.items() if address <= call.address),
                    key=lambda name: code_entries[name])
        expected = {'Mem_Free': 'free', 'Mem_Alloc': 'malloc', 'Mem_CreatePool': 'malloc',
                    'Mem_InitPools': 'Mem_CreatePool', 'Mem_DestroyPool': 'free',
                    'Mem_ShutdownPools': 'Mem_DestroyPool'}.get(owner)
        assert expected is not None and int(call.op_str, 16) == symbols[expected], 'Unexpected direct helper dependency'
        owners.append(owner)
    assert {name: owners.count(name) for name in set(owners)} == {
        'Mem_Free': 1, 'Mem_Alloc': 5, 'Mem_CreatePool': 1, 'Mem_InitPools': 1,
        'Mem_DestroyPool': 4, 'Mem_ShutdownPools': 1}
    print(f'Original: 76 bytes, {len(instructions)} instructions; compiled .text: {text.Misc_VirtualSize} bytes.')
    print(f'Raw code-byte equality: {original_code == generated_code}; relocation-aware equality not evaluated.')
    print('Instruction equality: not claimed (modern provisional Clang code generation).')
    rng = random.Random(0x45b1f0)
    cases = 0
    for flag in [0, 1, 2, 0xffffffff, 0x80000000]:
        for pattern in [0, 0xff, None]:
            def data(n):
                return bytes([pattern])*n if pattern is not None else rng.randbytes(n)
            values = [struct.pack('<I',flag), data(800), data(400), data(2)]
            for repeat in range(2):
                old = list(values)
                values, owrites = execute(original, 0x45b1f0, FIELDS, old, cases,
                                         (0x45b1f0,0x45b23c))
                actual, cwrites = execute(rebuilt, symbols['Mem_InitHandles'], fields,
                                         old, cases, extent)
                assert actual == values, f'Original/C divergence: flag={flag:#x}, pattern={pattern}, repeat={repeat}'
                if struct.unpack('<I',old[0])[0] == 1:
                    assert values == old and not owrites and not cwrites
                else:
                    # Derived from REP STOSD count and 16-bit stores in original assembly.
                    assert values == [struct.pack('<I',1), bytes(800),
                                      struct.pack('<200H',*range(1,201)), bytes(2)]
                    assert owrites[0] == (0x4bab38,4,1)
                    assert owrites[-1] == (0x512040,2,0)
                cases += 1
    versions = {name: subprocess.check_output([shutil.which(name), '--version'], text=True).splitlines()[0]
                for name in ('clang', 'lld-link')}
    details = {'toolchain': versions, 'pefile': pefile.__version__, 'unicorn': unicorn.__version__,
               'scope': 'Mem_InitHandles only; focused PE32 DLL; not native game execution',
               'compiler_flags': '--target=i686-pc-windows-msvc -std=c89 -pedantic-errors -Wall -Wextra -Werror -O2 -ffreestanding -fno-builtin -fno-vectorize -fno-slp-vectorize -mno-sse -mno-sse2',
               'linker_flags': '/dll /noentry /nodefaultlib /machine:x86 /base:0x10000000',
               'checks': 'state regions, boundary guards, skip/repeat, writes, EAX/ESP, saved registers, DF'}
    record_result('compilation', 'pass', details=details)
    record_result('raw_bytes', 'pass' if original_code == generated_code else 'different', details={'original_bytes': len(original_code), 'compiled_text_bytes': len(generated_code), 'scope': 'whole focused DLL .text versus routine; relocation-aware equality not evaluated'})
    record_result('emulation', 'pass', cases=cases, details=details)
    print(f'PASS: {cases} original-vs-C executions; full state, boundaries, skip/repeat, writes and ABI checked.')
    VALIDATING_RVA = 0x5b1b0
    consumer_details = {**details,
        'scope': 'Mem_NextHandleId; focused PE32 DLL; no native/layout parity claim',
        'checks': 'signed EAX, full state, exact reads/writes, caller stack, saved registers, DF; initializer/exhaustion integration',
        'negative_contract': 'readable nonaliasing signed-word storage; -4/-32768 differential; -1/-2 original-only'}
    record_result('compilation','pass',details=consumer_details)
    consumer_cases, original_only = verify_consumer(original, rebuilt, symbols, fields, extent)
    record_result('emulation','pass',cases=consumer_cases,
                  details={**consumer_details,'original_only_negative_cases':original_only})
    VALIDATING_RVA = 0x5b360
    register_details = {**details,
        'scope':'Mem_RegisterHandle; single-threaded nonaliasing mapped globals; no native/layout parity',
        'checks':'all 200 indices, first-zero selection, full/disabled tables, raw argument/dispatch words, exact read/write order, full image guards, EAX/ESP/caller arguments, saved registers, DF; init/consumer integration'}
    record_result('compilation','pass',details=register_details)
    register_cases = verify_registration(original,rebuilt,symbols,extent)
    record_result('emulation','pass',cases=register_cases,details=register_details)
    VALIDATING_RVA = 0x5b240
    shutdown_details = {**details,
        'scope':'Mem_ShutdownHandles; callbacks modeled at verified ABI boundary; no native callback/game parity',
        'checks':'two live 200-slot scans, exact status/flag eligibility and pass order, flag clear and status clear before callback, raw arguments, callback mutations/reentry, scratch-register clobbering, EAX/ESP/saved registers/DF, full image guards'}
    record_result('compilation','pass',details=shutdown_details)
    shutdown_cases = verify_shutdown(original,rebuilt,symbols,extent)
    record_result('emulation','pass',cases=shutdown_cases,details=shutdown_details)
    VALIDATING_RVA = 0x5b410
    release_details = {**details,
        'scope':'Mem_ReleaseHandleId; single-threaded nonaliasing mapped globals; no native/layout parity',
        'checks':'all 200 slots, duplicates, exact status equality, raw ID equality, no-match/disabled/repeat states, exact ordered global reads/writes, full image guards, EAX/ESP/argument/saved registers/DF; five-routine integration'}
    record_result('compilation','pass',details=release_details)
    release_cases = verify_release(original,rebuilt,symbols,extent)
    record_result('emulation','pass',cases=release_cases,details=release_details)
    print('Native DLL/game execution and startup/gameplay parity: unverified by this harness.')

def verify_consumer(original, rebuilt, symbols, fields, extent):
    code = original.get_data(0x5b1b0,58)
    assert hashlib.sha256(code).hexdigest() == '696ce4a075f495b91bd79ce9fe531b4c474b66540d3935dcc8e5a45c74d6d139'
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    decoded = list(md.disasm(code,0x45b1b0))
    assert sum(i.size for i in decoded) == 58 and not any(i.mnemonic=='call' for i in decoded)
    # This is only a raw diagnostic, not a relocation-aware match score.
    rebuilt_code = rebuilt.get_data(symbols['Mem_NextHandleId']-rebuilt.OPTIONAL_HEADER.ImageBase,58)
    print(f'Consumer: 58 original bytes, {len(decoded)} instructions; raw 58-byte prefix equal: {code == rebuilt_code}.')
    record_result('raw_bytes','pass' if code == rebuilt_code else 'different',
        details={'original_bytes':58,'compiled_prefix_bytes':58,
                 'scope':'prefix diagnostic only, not verified compiled extent or relocation-aware equality'})
    cases = 0
    rng = random.Random(0x45b1b0)
    def compare(old, expected, result, cursor, word=None):
        nonlocal cases
        extras = [] if word is None else [(FIELDS[2][1]+cursor*2,word)]
        cextras = [] if word is None else [(fields[2][1]+cursor*2,word)]
        for address,_ in cextras:
            assert not any(address < a+n and a < address+2 for _,a,n in fields), 'Sentinel aliases compiled global'
        enabled = struct.unpack('<I',old[0])[0] != 0 and cursor < 199
        actual, owrites = execute(original,0x45b1b0,FIELDS,old,cases,
                                 (0x45b1b0,0x45b1ea),result,extras,
                                 [(FIELDS[2][1]+cursor*2,2)] if enabled else [])
        rebuilt_result, cwrites = execute(rebuilt,symbols['Mem_NextHandleId'],fields,
                                         old,cases,extent,result,cextras,
                                         [(fields[2][1]+cursor*2,2)] if enabled else [])
        assert actual == rebuilt_result == expected, (cursor, result,
            [(i,a.hex(),b.hex(),e.hex()) for i,(a,b,e) in enumerate(zip(actual,rebuilt_result,expected)) if a!=b or a!=e])
        changed = expected[3] != old[3]
        assert owrites == ([(FIELDS[3][1],2,(cursor+1)&0xffff)] if changed else [])
        assert cwrites == ([(fields[3][1],2,(cursor+1)&0xffff)] if changed else [])
        cases += 1
        return actual
    # Independent expected results derive from MOVSX, signed JGE, INC AX and
    # the original effective address, rather than calling the reconstruction.
    for flag in [0,1,2,0xffffffff,0x80000000]:
        for cursor in [0,198,199,200,32767,-4,-32768]:
            for id_value in [0,1,32767,-1,-32768]:
                ids = bytearray(rng.randbytes(400))
                if 0 <= cursor < 200:
                    struct.pack_into('<h',ids,cursor*2,id_value)
                old = [struct.pack('<I',flag),rng.randbytes(800),bytes(ids),struct.pack('<h',cursor)]
                enabled = flag != 0 and cursor < 199
                expected = list(old)
                if enabled:
                    expected[3] = struct.pack('<h',cursor+1)
                compare(old,expected,id_value if enabled else -1,cursor,
                        id_value if cursor < 0 else None)
    # Linked validation globals have a different layout: at -1/-2 the compiled
    # address aliases cursor/flag. Independently execute the original at those
    # offsets without pretending the surrounding storage is layout-equivalent.
    original_only = 0
    for cursor in [-1,-2]:
        for word in [0,32767,-1,-32768]:
            old = [struct.pack('<I',2),bytes(800),bytes(400),struct.pack('<h',cursor)]
            expected = list(old)
            expected[3] = struct.pack('<h',cursor+1)
            actual,writes = execute(original,0x45b1b0,FIELDS,old,800+original_only,
                (0x45b1b0,0x45b1ea),word,[(FIELDS[2][1]+cursor*2,word)],
                [(FIELDS[2][1]+cursor*2,2)])
            assert actual == expected and writes == [(FIELDS[3][1],2,(cursor+1)&0xffff)]
            original_only += 1
    # Integrate actual original/C initializer outputs and consume to exhaustion.
    seed = [bytes(4),rng.randbytes(800),rng.randbytes(400),b'\xff\x7f']
    values,_ = execute(original,0x45b1f0,FIELDS,seed,900,(0x45b1f0,0x45b23c))
    cvalues,_ = execute(rebuilt,symbols['Mem_InitHandles'],fields,seed,900,extent)
    assert values == cvalues
    for index in range(202):
        cursor = min(index,199)
        expected = list(values)
        if index < 199:
            expected[3] = struct.pack('<h',index+1)
        values = compare(values,expected,index+1 if index < 199 else -1,cursor)
    print(f'PASS: {cases} consumer original-vs-C executions; flags, signed IDs/cursors, exact writes, ABI and initializer/exhaustion integration.')
    print(f'PASS: {original_only} original-only -1/-2 cursor executions; compiled surrounding data layout differs, no layout parity claim.')
    return cases, original_only


def verify_registration(original,rebuilt,symbols,extent):
    code = original.get_data(0x5b360,120)
    assert hashlib.sha256(code).hexdigest() == '2dc672ad67179fa73bca4d901a607b72986ba7138a1d00ecb954b3b9280e5e34'
    decoded = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,0x45b360))
    assert sum(i.size for i in decoded) == 120 and not any(i.mnemonic=='call' for i in decoded)
    prefix = rebuilt.get_data(symbols['Mem_RegisterHandle']-rebuilt.OPTIONAL_HEADER.ImageBase,120)
    record_result('raw_bytes','pass' if code == prefix else 'different',details={
        'original_bytes':120,'compiled_prefix_bytes':120,
        'scope':'prefix diagnostic only; compiled extent/relocation-aware equality not established'})
    print(f'Registration: 120 original bytes / {len(decoded)} instructions; raw prefix equal: {code == prefix}.')
    fields = [(name,symbols[name],n) for name,_,n in BOOKKEEPING_FIELDS]
    for i,(_,a,n) in enumerate(fields):
        assert not any(a < b+m and b < a+n for _,b,m in fields[i+1:]), 'Aliasing validation fields'
    rng = random.Random(0x45b360)
    cases = 0
    def compare(old,handle):
        nonlocal cases
        # Expectations derived from CMP/JZ loop and six MOV stores in the PE.
        flag = struct.unpack('<I',old[0])[0]
        statuses = struct.unpack('<200I',old[1])
        index = next((i for i,s in enumerate(statuses) if s==0),None) if flag else None
        result = 0 if not flag else -1 if index is None else index
        expected = list(old)
        events = [('read',BOOKKEEPING_FIELDS[0][0],0,4,flag)]
        def append(kind,field,offset,value):
            events.append((kind,BOOKKEEPING_FIELDS[field][0],offset,4,value))
        if flag:
            for i in range(200 if index is None else index+1):
                append('read',1,4*i,statuses[i])
        if index is not None:
            context,callback,parameter = [struct.unpack('<I',old[i])[0] for i in [9,10,11]]
            append('read',9,0,context)
            # Order: status, flags, context, ID, parameter read/store, callback read/store.
            for field,value in [(1,1),(8,0x10000),(4,context),(6,handle),
                                (5,parameter),(7,callback)]:
                if field==5:
                    append('read',11,0,parameter)
                if field==7:
                    append('read',10,0,callback)
                append('write',field,4*index,value)
                data = bytearray(expected[field]);struct.pack_into('<I',data,4*index,value)
                expected[field] = bytes(data)
        actual,owrites = execute(original,0x45b360,BOOKKEEPING_FIELDS,old,cases,
            (0x45b360,0x45b3d8),result,stack_args=(handle,),expected_events=events)
        rebuilt_result,cwrites = execute(rebuilt,symbols['Mem_RegisterHandle'],fields,old,cases,
            extent,result,stack_args=(handle,),expected_events=events)
        assert actual == rebuilt_result == expected, (flag,index,handle)
        assert len(owrites) == len(cwrites) == (0 if index is None else 6)
        cases += 1
        return actual
    def values(flag,statuses):
        old = [rng.randbytes(n) for _,_,n in BOOKKEEPING_FIELDS]
        old[0] = struct.pack('<I',flag)
        old[1] = struct.pack('<200I',*statuses)
        return old
    handles = [0,1,0x7fffffff,0x80000000,0xffffffff]
    # Every slot must be reached without touching index 200. Later holes remain free.
    for index in range(200):
        statuses = [rng.choice([1,2,0x80000000,0xffffffff]) for _ in range(index)] + [0]*(200-index)
        compare(values(1,statuses),handles[index%len(handles)])
    for flag in [0,1,2,0x80000000,0xffffffff]:
        for index in [0,1,198,199,None]:
            statuses = [rng.choice([1,2,0x80000000,0xffffffff]) for _ in range(200)]
            if index is not None:
                statuses[index] = 0
            for handle in handles:
                old = values(flag,statuses)
                # Include zero and all-one dispatch words, including null callback.
                for field in [9,10,11]:
                    old[field] = struct.pack('<I',handle)
                compare(old,handle)
    for _ in range(64):
        compare(values(rng.getrandbits(32),[rng.choice([0,1,2,0xffffffff]) for _ in range(200)]),rng.getrandbits(32))
    # Run all three actual routines: ID exhaustion at 199 does not stop registration
    # from accepting 0xFFFFFFFF into slot 199; next registration fails without writes.
    old = values(0,[0xffffffff]*200)
    state,_ = execute(original,0x45b1f0,BOOKKEEPING_FIELDS,old,1000,(0x45b1f0,0x45b23c))
    cstate,_ = execute(rebuilt,symbols['Mem_InitHandles'],fields,old,1000,extent)
    assert state == cstate and state[4:] == old[4:]
    for index in range(201):
        handle = index+1 if index < 199 else 0xffffffff
        state,_ = execute(original,0x45b1b0,BOOKKEEPING_FIELDS,state,1001+index,
            (0x45b1b0,0x45b1ea),handle)
        cstate,_ = execute(rebuilt,symbols['Mem_NextHandleId'],fields,cstate,1001+index,extent,handle)
        assert state == cstate
        state = compare(state,handle)
        cstate = list(state)
    print(f'PASS: {cases} registration original-vs-C executions; all indices, exhaustion/disabled, ordered effects and ABI; three-routine integration.')
    return cases

def verify_shutdown(original,rebuilt,symbols,extent):
    code = original.get_data(0x5b240,155)
    assert hashlib.sha256(code).hexdigest() == 'edf69cfb1fd2d860913575999d39e05e37e615c11fb1e4280e4c166836e3b59f'
    prefix = rebuilt.get_data(symbols['Mem_ShutdownHandles']-rebuilt.OPTIONAL_HEADER.ImageBase,155)
    record_result('raw_bytes','pass' if code==prefix else 'different',details={
        'original_bytes':155,'compiled_prefix_bytes':155,
        'scope':'prefix diagnostic only; compiled extent/relocation-aware equality not established'})
    fields = [(name,symbols[name],n) for name,_,n in BOOKKEEPING_FIELDS]
    callback_code = original.get_data(0x56210,83)
    rng = random.Random(0x45b240)
    cases = 0
    def fresh(flag,statuses,flags):
        state = [rng.randbytes(n) for _,_,n in BOOKKEEPING_FIELDS]
        state[0] = struct.pack('<I',flag)
        state[1] = struct.pack('<200I',*statuses)
        state[8] = struct.pack('<200I',*flags)
        state[7] = struct.pack('<200I',*[CALLBACK]*200)
        return state
    def compare(old,actions=None):
        nonlocal cases
        actions = actions or {}
        expected = [bytearray(value) for value in old]
        events, calls = [], []
        def word(field,index=0):
            return struct.unpack_from('<I',expected[field],4*index)[0]
        def event(kind,field,index,value):
            events.append((kind,BOOKKEEPING_FIELDS[field][0],4*index,4,value))
        def read(field,index=0):
            value = word(field,index);event('read',field,index,value)
            return value
        def write(field,index,value,kind='write'):
            event(kind,field,index,value)
            struct.pack_into('<I',expected[field],4*index,value)
        # Independent instruction-derived model: equality (not mask/nonzero),
        # two ascending passes, status cleared BEFORE argument/pointer read.
        if read(0):
            write(0,0,0)
            for required_flag in [0x10000,0x20000]:
                for index in range(200):
                    if read(1,index)!=1:
                        continue
                    if read(8,index)!=required_flag:
                        continue
                    write(1,index,0)
                    parameter = read(5,index)
                    assert read(7,index)==CALLBACK
                    events.append(('callback','Mem_ShutdownCallback',0,4,parameter))
                    action = actions.get(len(calls),{})
                    calls.append((parameter,[bytes(value) for value in expected],action))
                    for field,slot,value in action.get('writes',[]):
                        write(field,slot,value,'callback_write')
                    if action.get('reenter'):
                        assert word(0)==0, 'This reentry contract uses the disabled nested path'
                        read(0)
        expected = [bytes(value) for value in expected]
        def run(pe,entry,run_fields,run_extent):
            call_index = 0
            def callback(cpu,mapped_fields,parameter,actual_events):
                nonlocal call_index
                assert call_index < len(calls), 'Unexpected callback'
                expected_parameter,snapshot,action = calls[call_index]
                assert parameter==expected_parameter, 'Callback stack argument'
                assert [bytes(cpu.mem_read(a,n)) for _,a,n in mapped_fields]==snapshot, 'Callback observes wrong lifecycle state'
                for field,slot,value in action.get('writes',[]):
                    name,address,_ = mapped_fields[field]
                    cpu.mem_write(address+4*slot,struct.pack('<I',value))
                    actual_events.append(('callback_write',name,4*slot,4,value))
                call_index += 1
                return action.get('reenter',False)
            actual,writes = execute(pe,entry,run_fields,old,cases,run_extent,
                expected_events=events,callback_code=callback_code,callback_handler=callback)
            assert call_index==len(calls), 'Missing callback'
            assert actual==expected, 'Shutdown state differs from original-instruction expectations'
            return actual,writes
        actual,owrites = run(original,0x45b240,BOOKKEEPING_FIELDS,(0x45b240,0x45b2db))
        rebuilt_state,cwrites = run(rebuilt,symbols['Mem_ShutdownHandles'],fields,extent)
        assert actual==rebuilt_state and len(owrites)==len(cwrites)
        cases += 1
        return actual
    # Disabled, inactive, both all-active passes, unsupported flags, mixed states.
    for flag in [0,1,2,0x80000000,0xffffffff]:
        for pattern in range(6):
            statuses = [0]*200 if pattern==0 else [1]*200
            flags = [0x10000 if pattern in (0,1) else 0x20000 if pattern==2 else 0x30000]*200
            if pattern==4:
                statuses = [i%3 for i in range(200)]
                flags = [0x10000 if i%2 else 0x20000 for i in range(200)]
            if pattern==5:
                statuses = [rng.choice([0,1,2,0xffffffff]) for _ in range(200)]
                flags = [rng.choice([0,0x10000,0x20000,0x30000]) for _ in range(200)]
            compare(fresh(flag,statuses,flags))
    # Individually reach every slot in each pass; no index-200 access permitted.
    for selected_flag in [0x10000,0x20000]:
        for index in range(200):
            statuses = [0]*200;statuses[index]=1
            flags = [0]*200;flags[index]=selected_flag
            compare(fresh(1,statuses,flags))
    for index in [0,199]:
        for status in [0,1,2,0x80000000,0xffffffff]:
            for flag in [0,0x10000,0x20000,0x30000,0x10001,0xffffffff]:
                statuses = [0]*200;statuses[index]=status
                flags = [0]*200;flags[index]=flag
                state = fresh(2,statuses,flags)
                state[5] = struct.pack('<200I',*[0xffffffff]*200)
                compare(state)
    for _ in range(32):
        compare(fresh(rng.getrandbits(32),[rng.choice([0,1,2,0xffffffff]) for _ in range(200)],
            [rng.choice([0,0x10000,0x20000,0x30000,0x10001]) for _ in range(200)]))
    # Mutation and reentry plans are explicit test callbacks, not game substitutes.
    plans = [
        ([0,1],[0x10000,0x10000],{0:{'writes':[(1,1,0),(1,199,1),(8,199,0x10000)]}}),
        ([199],[0x10000],{0:{'writes':[(1,0,1),(8,0,0x20000)]}}),
        ([199],[0x20000],{0:{'writes':[(1,0,1),(8,0,0x20000)]}}),
        ([199],[0x10000],{0:{'writes':[(1,0,1),(8,0,0x10000)]}}),
        ([0,1],[0x10000,0x10000],{0:{'writes':[(5,1,0xffffffff)]}}),
        ([0,199],[0x10000,0x20000],{0:{'writes':[(0,0,2)]}}),
        ([0,199],[0x10000,0x20000],{0:{'reenter':True}}),
        ([0],[0x10000],{0:{'writes':[(1,0,1),(8,0,0x20000)]}})]
    for indices,selected_flags,actions in plans:
        statuses,flags = [0]*200,[0]*200
        for index,flag in zip(indices,selected_flags):
            statuses[index]=1;flags[index]=flag
        compare(fresh(1,statuses,flags),actions)
    # Registration -> shutdown integration uses actual original and compiled code.
    old = fresh(0,[0xffffffff]*200,[0xffffffff]*200)
    state,_ = execute(original,0x45b1f0,BOOKKEEPING_FIELDS,old,2000,(0x45b1f0,0x45b23c))
    cstate,_ = execute(rebuilt,symbols['Mem_InitHandles'],fields,old,2000,extent)
    assert state==cstate
    for index in range(3):
        state[10] = cstate[10] = struct.pack('<I',CALLBACK)
        state[11] = cstate[11] = struct.pack('<I',[0,0x80000000,0xffffffff][index])
        state,_ = execute(original,0x45b1b0,BOOKKEEPING_FIELDS,state,2001+index,(0x45b1b0,0x45b1ea),index+1)
        cstate,_ = execute(rebuilt,symbols['Mem_NextHandleId'],fields,cstate,2001+index,extent,index+1)
        assert state==cstate
        state,_ = execute(original,0x45b360,BOOKKEEPING_FIELDS,state,2004+index,(0x45b360,0x45b3d8),index,stack_args=(index+1,))
        cstate,_ = execute(rebuilt,symbols['Mem_RegisterHandle'],fields,cstate,2004+index,extent,index,stack_args=(index+1,))
        assert state==cstate
    compare(state)
    print(f'PASS: {cases} shutdown original-vs-C executions; two passes, all indices, ordered lifecycle, callback ABI/model mutations/reentry and four-routine integration.')
    return cases


def verify_release(original,rebuilt,symbols,extent):
    code = original.get_data(0x5b410,62)
    assert hashlib.sha256(code).hexdigest()=='d0fee3704a333538872dacd400ee150f38a7a7dcfe33e1770f357d09ffb8d1cb'
    decoded = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,0x45b410))
    assert len(decoded)==16 and sum(i.size for i in decoded)==62 and not any(i.mnemonic=='call' for i in decoded)
    prefix = rebuilt.get_data(symbols['Mem_ReleaseHandleId']-rebuilt.OPTIONAL_HEADER.ImageBase,62)
    record_result('raw_bytes','pass' if code==prefix else 'different',details={
        'original_bytes':62,'compiled_prefix_bytes':62,
        'scope':'prefix diagnostic only; compiled extent/relocation-aware equality not established'})
    fields = [(name,symbols[name],n) for name,_,n in BOOKKEEPING_FIELDS]
    rng = random.Random(0x45b410)
    cases = 0
    handles = [0,1,0x7fffffff,0x80000000,0xffffffff]
    def fresh(flag,statuses,ids):
        old = [rng.randbytes(n) for _,_,n in BOOKKEEPING_FIELDS]
        old[0] = struct.pack('<I',flag)
        old[1] = struct.pack('<200I',*statuses)
        old[6] = struct.pack('<200I',*ids)
        return old
    def compare(old,handle):
        nonlocal cases
        flag = struct.unpack('<I',old[0])[0]
        statuses = struct.unpack('<200I',old[1])
        ids = struct.unpack('<200I',old[6])
        expected = list(old)
        status_bytes = bytearray(old[1])
        events = [('read',BOOKKEEPING_FIELDS[0][0],0,4,flag)]
        writes = []
        # Derived from CMP status,1; CMP registered ID,EAX; conditional store,
        # then unconditional ascending continuation through all 200 slots.
        if flag:
            for index,status in enumerate(statuses):
                events.append(('read',BOOKKEEPING_FIELDS[1][0],4*index,4,status))
                if status!=1:
                    continue
                events.append(('read',BOOKKEEPING_FIELDS[6][0],4*index,4,ids[index]))
                if ids[index]==handle:
                    struct.pack_into('<I',status_bytes,4*index,0)
                    events.append(('write',BOOKKEEPING_FIELDS[1][0],4*index,4,0))
                    writes.append(index)
        expected[1] = bytes(status_bytes)
        actual,owrites = execute(original,0x45b410,BOOKKEEPING_FIELDS,old,cases,
            (0x45b410,0x45b44e),int(flag!=0),stack_args=(handle,),expected_events=events)
        rebuilt_state,cwrites = execute(rebuilt,symbols['Mem_ReleaseHandleId'],fields,old,cases,
            extent,int(flag!=0),stack_args=(handle,),expected_events=events)
        assert actual==rebuilt_state==expected
        assert owrites==[(BOOKKEEPING_FIELDS[1][1]+4*i,4,0) for i in writes]
        assert cwrites==[(fields[1][1]+4*i,4,0) for i in writes]
        cases += 1
        return actual
    for index in range(200):
        statuses = [0]*200;statuses[index]=1
        handle = handles[index%len(handles)]
        ids = [handle^0x01010101]*200;ids[index]=handle
        compare(fresh(1,statuses,ids),handle)
    for flag in [0,1,2,0x80000000,0xffffffff]:
        for handle in handles:
            for pattern in range(6):
                statuses = [0]*200 if pattern==0 else [1]*200
                ids = [handle^0x01010101]*200 if pattern==1 else [handle]*200
                if pattern==3:
                    statuses = [i%3 for i in range(200)]
                    ids = [handle if i%2 else handle^0x01010101 for i in range(200)]
                if pattern==4:
                    statuses = [rng.choice([0,2,0x80000000,0xffffffff]) for _ in range(200)]
                if pattern==5:
                    statuses = [0]*200;statuses[0]=statuses[199]=1
                compare(fresh(flag,statuses,ids),handle)
    for _ in range(64):
        handle = rng.getrandbits(32)
        compare(fresh(rng.getrandbits(32),[rng.choice([0,1,2,0x80000000,0xffffffff]) for _ in range(200)],
            [handle if rng.randrange(3) else rng.getrandbits(32) for _ in range(200)]),handle)
    # Actual initializer/consumer/registration produce three duplicate IDs.
    # Release clears all, repeat and no-match still succeed; shutdown then leaves
    # release disabled. No callback is needed or manufactured for this sequence.
    old = fresh(0,[0xffffffff]*200,[0xffffffff]*200)
    state,_ = execute(original,0x45b1f0,BOOKKEEPING_FIELDS,old,3000,(0x45b1f0,0x45b23c))
    cstate,_ = execute(rebuilt,symbols['Mem_InitHandles'],fields,old,3000,extent)
    assert state==cstate
    for index in range(3):
        state,_ = execute(original,0x45b1b0,BOOKKEEPING_FIELDS,state,3001+index,(0x45b1b0,0x45b1ea),index+1)
        cstate,_ = execute(rebuilt,symbols['Mem_NextHandleId'],fields,cstate,3001+index,extent,index+1)
        assert state==cstate
        state,_ = execute(original,0x45b360,BOOKKEEPING_FIELDS,state,3004+index,(0x45b360,0x45b3d8),index,stack_args=(42,))
        cstate,_ = execute(rebuilt,symbols['Mem_RegisterHandle'],fields,cstate,3004+index,extent,index,stack_args=(42,))
        assert state==cstate
    state = compare(state,0)
    state = compare(state,42)
    state = compare(state,42)
    cstate = list(state)
    state,_ = execute(original,0x45b240,BOOKKEEPING_FIELDS,state,3007,(0x45b240,0x45b2db))
    cstate,_ = execute(rebuilt,symbols['Mem_ShutdownHandles'],fields,cstate,3007,extent)
    assert state==cstate
    compare(state,42)
    print(f'PASS: {cases} ID-release original-vs-C executions; all indices, duplicates, eligibility, ordered effects, ABI and five-routine integration.')
    return cases


if __name__ == '__main__':
    try:
        verify()
    except Exception as exc:
        try:
            record_result('emulation', 'fail', details={'error': str(exc), 'scope': 'Verification did not complete'})
        except Exception as tracking_error:
            print(f'Could not persist verification failure: {tracking_error}')
        raise
    from verify_font_width import verify_font_width
    verify_font_width()
    from verify_font_draw import verify_font_draw
    verify_font_draw()
    from verify_sprite_backend import verify_sprite_backend
    verify_sprite_backend()
    from verify_font_cleanup import verify_font_cleanup
    verify_font_cleanup()
    from verify_font_parse import verify_font_parse
    verify_font_parse()

    from verify_font_load import verify_font_load
    verify_font_load()

    from verify_mem_free import verify_mem_free
    verify_mem_free()

    from verify_mem_alloc import verify_mem_alloc
    verify_mem_alloc()

    from verify_file_helpers import verify_file_helpers
    verify_file_helpers()

    from verify_file_load import verify_file_load
    verify_file_load()

    from verify_mem_pools import verify_mem_pools
    verify_mem_pools()

    from verify_mem_destroy import verify_mem_destroy
    verify_mem_destroy()
