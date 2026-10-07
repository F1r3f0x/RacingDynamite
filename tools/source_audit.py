"""Conservative C source inventory; not a compiler or proof of behavior."""
import re
from pathlib import Path

def masked(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text, flags=re.S)

def definitions(text):
    clean = masked(text)
    pattern = r'(?m)^[ \t]*(?:[A-Za-z_]\w*[ \t\n*]+)+([A-Za-z_]\w*)[ \t]*\([^;{}]*\)[ \t\n]*\{'
    result = []
    for m in re.finditer(pattern, clean):
        name = m[1]
        if name in {'if', 'while', 'for', 'switch'}:
            continue
        depth, end = 1, m.end()
        while depth and end < len(clean):
            depth += (clean[end] == '{') - (clean[end] == '}')
            end += 1
        body = clean[m.end():end-1]
        reduced = re.sub(r'\(void\)\s*\w+\s*;', '', body).strip()
        stub = not reduced or bool(re.fullmatch(r'return\s+(?:[-+]?\d+(?:\.\d+)?|NULL)\s*;', reduced))
        result.append({'symbol': name, 'line': text.count('\n', 0, m.start())+1,
                       'body': body, 'stub_candidate': stub})
    return result

def source_inventory(root):
    files = {}
    for directory in ('src', 'include', 'decomp/src', 'decomp/include'):
        for p in sorted((root / directory).rglob('*')):
            if p.suffix in ('.c', '.h'):
                text = p.read_text(encoding='utf-8', errors='replace')
                files[p.relative_to(root).as_posix()] = (text, definitions(text))
    return files

def annotations(text):
    result = {}
    # Accept existing file-level named provenance blocks; require named records.
    for m in re.finditer(r'@original\s+(\w+)\s*([^\n]*)', text):
        address = re.search(r'0x[0-9a-fA-F]+', m[2])
        end = text.find('@original', m.end())
        region = text[m.end():end if end >= 0 else len(text)]
        fidelity = re.search(r'@fidelity\s+(\w+)', region)
        result[m[1]] = (address[0].lower() if address else None, fidelity[1] if fidelity else None)
    return result
