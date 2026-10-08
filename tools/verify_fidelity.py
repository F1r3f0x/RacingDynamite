"""Audit active Windows extents, reconstructed-source provenance, and validation freshness."""
import sys
from windows_tracking import audit


def main():
    try:
        data = audit()
    except (OSError,ValueError,KeyError) as exc:
        print(f'[FAIL] {exc}',file=sys.stderr)
        return 1
    summary = data['summary']
    print(f"PASS: {summary['functions']} Windows candidates; {summary['stages']['reconstructed']} reconstructed routines.")
    print('Current evidence:',summary['current_passes'])
    print('Scope: active Windows records. Other unmigrated source modules are not certified; native game parity is separate.')
    return 0


if __name__=='__main__':
    sys.exit(main())
