"""Stage and launch equivalent disposable DOSBox environments; never write Ignition/."""

# MIGRATION REQUIRED: retained for Windows adaptation; do not use yet.
if __package__:
    from .tool_migration import require_migration
else:
    from tool_migration import require_migration
require_migration(__file__)

from pathlib import Path
import shutil
import hashlib
import json
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
game = root/'Ignition/Ignition'
stage = root/'build/runtime'
stage.mkdir(parents=True, exist_ok=True)
if len(sys.argv) != 2 or sys.argv[1] not in ('stage', 'check', 'original', 'rebuilt'):
    sys.exit('Usage: runtime_baseline.py stage|check|original|rebuilt')
if sys.argv[1] == 'stage':
    hashes = {p.relative_to(game).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in game.rglob('*') if p.is_file()}
    (stage/'original_hashes.json').write_text(json.dumps(hashes, indent=2))
    for target in ('original', 'rebuilt'):
        dest = stage/target
        shutil.copytree(game, dest, dirs_exist_ok=True, ignore=shutil.ignore_patterns('DOSBOX', '__support', 'unins*', 'MREBUILT.EXE', 'MAINDOS_REBUILT.EXE'))
        if target == 'rebuilt':
            shutil.copy2(root/'build/decomp/MAINDOS_REBUILT.EXE', dest/'MREBUILT.EXE')
        exe = 'MAINDOS.EXE' if target == 'original' else 'MREBUILT.EXE'
        conf = f'''[sdl]
fullscreen=false
windowresolution=640x480
output=surface
autolock=false
[dosbox]
machine=svga_s3
memsize=32
[cpu]
core=normal
cputype=auto
cycles=fixed 50000
[mixer]
nosound=false
[autoexec]
@echo off
mount c "{dest}"
imgmount d "{dest / 'game.ins'}" -t iso -fs iso
c:
{exe}
'''
        (stage/f'{target}.conf').write_text(conf)
    print('Staged original and rebuilt with equivalent DOSBox configuration; original asset hashes saved.')
elif sys.argv[1] == 'check':
    hashes = json.loads((stage/'original_hashes.json').read_text())
    generated = {'MAINDOS_REBUILT.EXE', 'MREBUILT.EXE'}
    changes = [name for name, digest in hashes.items() if name not in generated and (not (game/name).is_file() or hashlib.sha256((game/name).read_bytes()).hexdigest() != digest)]
    generated_changes = [name for name, digest in hashes.items() if name in generated and (not (game/name).is_file() or hashlib.sha256((game/name).read_bytes()).hexdigest() != digest)]
    print('Preexisting generated executable changes (outside this staging tool):', generated_changes)
    print('Original asset changes:', changes)
    sys.exit(bool(changes))
else:
    target = sys.argv[1]
    p = subprocess.Popen([str(game/'DOSBOX/DOSBox.exe'), '-conf', str(stage/f'{target}.conf')], cwd=stage/target)
    (stage/f'{target}.pid').write_text(str(p.pid))
    print('Launched', target, 'PID', p.pid)
