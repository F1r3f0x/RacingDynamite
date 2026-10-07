"""List retained tools that must be migrated before execution or import."""
import json
from pathlib import Path

MANIFEST = Path(__file__).with_name('migration_required.json')


def require_migration(script):
    name = Path(script).name
    entries = json.loads(MANIFEST.read_text(encoding='utf-8'))
    reason = entries['scripts'][name]
    raise SystemExit(
        f'MIGRATION REQUIRED: {name}: {reason}\n'
        'Retained for Windows adaptation; execution/import is disabled. '
        'See tools/MIGRATION.md. No override is supported.'
    )


if __name__ == '__main__':
    print(MANIFEST.read_text(encoding='utf-8'))
