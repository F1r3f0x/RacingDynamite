"""Audit the temporary Windows inventory and sole migrated module.

Legacy database, other modules and dashboards do not certify Windows progress.
"""
import json
import re
from windows_target import ROOT, SHA256, INVENTORY, verify_target

if not __debug__:
    raise RuntimeError('Audit requires assertions; disable -O/PYTHONOPTIMIZE')

def audit():
    verify_target()
    inventory = json.loads(INVENTORY.read_text(encoding='utf-8'))
    assert inventory['target']['sha256'].lower() == SHA256
    functions = inventory['functions']
    assert len(functions) == 1
    fn = functions[0]
    assert (fn['symbol'], fn['va'], fn['rva'], fn['size']) == ('Mem_InitHandles', '0x0045B1F0', '0x0005B1F0', 76)
    assert fn['instruction_match'] == 'not claimed'
    source = (ROOT/'decomp/src/mem.c').read_text(encoding='utf-8')
    assert '@original Mem_InitHandles (IGN_WIN.EXE @ 0x0045B1F0,' in source
    assert '@fidelity EXACT' in source
    assert not re.search(r'__asm|MAINDOS|Watcom', source)
    definitions = re.findall(r'^int\s+(\w+)\(void\)\s*\{', source, re.M)
    assert definitions == ['Mem_InitHandles']
    print('PASS: Windows fingerprint, address, annotation and bounded inventory consistent.')
    print('Scope: mem.c only. Remaining decomp modules, database/decomp.db and dashboards are LEGACY/unmigrated.')

if __name__ == '__main__':
    audit()
