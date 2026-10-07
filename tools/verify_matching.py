# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Execute original Windows instructions versus compiled C in x86 emulation.

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
    UC_X86_REG_EIP)
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
PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

def execute(pe, entry, fields, values, seed, extent, expected_eax=1, extra_words=(), expected_read=None,
            stack_args=(), expected_events=None):
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
        assert events == expected_events, (events,expected_events)
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
    assert not any(i.mnemonic == 'call' for i in generated), 'Helper is no longer a leaf'
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

if __name__ == '__main__':
    try:
        verify()
    except Exception as exc:
        try:
            record_result('emulation', 'fail', details={'error': str(exc), 'scope': 'Verification did not complete'})
        except Exception as tracking_error:
            print(f'Could not persist verification failure: {tracking_error}')
        raise
