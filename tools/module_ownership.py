"""Source ownership policy; reconstruction paths are not original filenames."""
import json
from pathlib import Path

MANIFEST = 'decomp/modules.json'


def load(root):
    path = root / MANIFEST
    if not path.is_file():
        raise ValueError('Missing module ownership manifest: ' + MANIFEST)
    data = json.loads(path.read_text(encoding='utf-8-sig'))
    if data.get('schema_version') != 1:
        raise ValueError('Unsupported module ownership schema')
    modules = data['modules']
    sources, prefixes = set(), set()
    for name, module in modules.items():
        for key in ('source', 'header'):
            relative = Path(module[key])
            if relative.is_absolute() or not (root / relative).resolve().is_relative_to(root.resolve()):
                raise ValueError('Module path escapes repository: ' + module[key])
        if module['source'] in sources:
            raise ValueError('Multiple modules own ' + module['source'])
        sources.add(module['source'])
        for key in ('responsibility', 'state', 'dependencies', 'placement_reason', 'build_mode'):
            if not module.get(key):
                raise ValueError(name + ': missing ' + key)
        if 'original_source' not in module:
            raise ValueError(name + ': original_source must be explicit (null when unknown)')
        if module['build_mode'] not in ('direct', 'extracted'):
            raise ValueError(name + ': build_mode must be direct or extracted')
        if module['build_mode'] == 'extracted' and not module.get('approved_symbols'):
            raise ValueError(name + ': extracted modules need an explicit approved_symbols map')
        if module['build_mode'] == 'extracted':
            approved = module['approved_symbols']
            if not isinstance(approved, dict) or not all(isinstance(reason, str) and reason.strip()
                                                       for reason in approved.values()):
                raise ValueError(name + ': every approved legacy symbol needs a placement reason')
        for prefix in module['symbol_prefixes']:
            if not prefix or prefix in prefixes:
                raise ValueError('Empty or duplicate module symbol prefix')
            prefixes.add(prefix)
    return modules


def owner(modules, symbol):
    matches = [(len(prefix), name) for name, module in modules.items()
               for prefix in module['symbol_prefixes'] if symbol.startswith(prefix)]
    if not matches:
        raise ValueError(symbol + ': no module ownership rule; document placement first')
    return max(matches)[1]


def validate(root, rows, parse_functions):
    modules = load(root)
    definitions = {}
    for path in (root / 'decomp/src').rglob('*.c'):
        for symbol in parse_functions(path.read_text(encoding='utf-8-sig')):
            definitions.setdefault(symbol, []).append(path.relative_to(root).as_posix())
    for row in rows:
        if row['analysis_stage'] != 'reconstructed':
            continue
        symbol = row['symbol_name']
        module = modules[owner(modules, symbol)]
        expected = module['source']
        if module['build_mode'] == 'extracted' and symbol not in module['approved_symbols']:
            raise ValueError(symbol + ': legacy module additions need explicit ownership review; prefer a direct module')
        if row['source_path'] != expected:
            raise ValueError(symbol + ': module owns ' + expected + '; tracking says ' + str(row['source_path']))
        if definitions.get(symbol) != [expected]:
            raise ValueError(symbol + ': requires one definition in ' + expected + '; found ' + str(definitions.get(symbol, [])))
    return modules


def placement(root, rows, rvas, files, selected, reason):
    modules = load(root)
    if rvas and (not selected or not reason.strip()):
        raise ValueError('Declare --module and --placement-reason before a Windows feature')
    if set(selected) - modules.keys():
        raise ValueError('Unknown selected module')
    indexed = {row['rva']: row for row in rows}
    for rva in rvas:
        if rva not in indexed:
            raise ValueError('Unknown affected RVA: ' + hex(rva))
        symbol = indexed[rva]['symbol_name']
        # Unidentified candidates need an explicit proposed owner; the policy
        # validates their actual symbol/path once reconstruction is recorded.
        if not symbol.startswith('sub_'):
            name = owner(modules, symbol)
            if name not in selected:
                raise ValueError(symbol + ': owning module is absent from --module')
            if modules[name]['build_mode'] == 'extracted' and symbol not in modules[name]['approved_symbols']:
                raise ValueError(symbol + ': legacy module additions need explicit ownership review')
    if rvas and not all(modules[name]['source'] in files for name in selected):
        raise ValueError('Declare each selected module source with --file')
    return {'modules': {name: modules[name] for name in selected}, 'placement_reason': reason}
