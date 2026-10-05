"""Annotated LE disassembly: tools/disasm_annot.py <start_hex> <end_hex> [--out file]

Annotates call targets with names from database/decomp.db (dos_address) and
immediate/memory operands that point at printable strings in MAINDOS.EXE.
"""
import os
import sqlite3
import sys

sys.path.insert(0, os.path.dirname(__file__))
import capstone
from le_parser import LEFile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_names():
    names = {}
    try:
        con = sqlite3.connect(os.path.join(ROOT, "database", "decomp.db"))
        for addr, name in con.execute("SELECT dos_address, symbol_name FROM functions WHERE dos_address IS NOT NULL"):
            try:
                names[int(addr, 16)] = name
            except ValueError:
                pass
    except sqlite3.Error:
        pass
    return names


def try_string(le, addr):
    try:
        s = le.read_string(addr)
    except Exception:
        return None
    if 3 <= len(s) < 80 and all(32 <= ord(c) < 127 for c in s):
        return s
    return None


def main():
    start = int(sys.argv[1], 16)
    end = int(sys.argv[2], 16)
    out = None
    if "--out" in sys.argv:
        out = open(sys.argv[sys.argv.index("--out") + 1], "w")
    le = LEFile(os.path.join(ROOT, "Ignition", "Ignition", "MAINDOS.EXE"))
    names = load_names()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    code = bytes(le.read_vaddr(start, end - start))
    for insn in md.disasm(code, start):
        note = ""
        if insn.mnemonic in ("call", "jmp") and insn.op_str.startswith("0x"):
            t = int(insn.op_str, 16)
            if t in names:
                note = "; " + names[t]
            elif insn.mnemonic == "call":
                note = "; sub_%x" % t
        else:
            for op in insn.operands:
                v = None
                if op.type == capstone.x86.X86_OP_IMM:
                    v = op.imm
                elif op.type == capstone.x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                    v = op.mem.disp
                if v and 0xA0000 <= v < 0xC0000:
                    s = try_string(le, v)
                    if s:
                        note = '; "%s"' % s
        line = "0x%08x:  %-7s %-34s %s" % (insn.address, insn.mnemonic, insn.op_str, note)
        (out.write(line + "\n") if out else print(line))
    if out:
        out.close()


if __name__ == "__main__":
    main()
