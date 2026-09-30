import capstone

with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

start_va = 0x0001ae29
start_off = start_va - 0x10000 + 0x400

print(f"=== Disassembly from 0x{start_va:08x} ===")
for insn in md.disasm(data[start_off:start_off+0x180], start_va):
    print(f"0x{insn.address:08x}:  {insn.mnemonic:<8} {insn.op_str}")
