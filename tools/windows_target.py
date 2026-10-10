"""Fingerprint gate for the bounded Windows reconstruction. No DOS fallback."""
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TARGET = ROOT / 'Ignition/Ignition/IGN_WIN.EXE'
SHA256 = '7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782'
ROUTINE_SHA256 = 'bed1560d41e6ae198b393b26af425505bdae1eb3fb6f4169f6bb89267019cf8c'
BUILD = ROOT / 'build/decomp/windows'
DLL = BUILD / 'mem_validation.dll'
INVENTORY = ROOT / 'docs/tracking/windows_inventory.json'

def verify_target(path=TARGET):
    manifest = json.loads((ROOT/'decomp/target.json').read_text(encoding='utf-8-sig'))
    expected = {'binary':'Ignition/Ignition/IGN_WIN.EXE', 'size':915968,
                'machine':'0x014c', 'optional_magic':'0x010b',
                'image_base':'0x00400000', 'entry_rva':'0x00069950'}
    if any(manifest.get(k) != v for k,v in expected.items()) or manifest['sha256'].lower() != SHA256:
        raise ValueError('Target manifest differs from this verified milestone')
    data = path.read_bytes()
    if len(data) != 915968 or hashlib.sha256(data).hexdigest() != SHA256:
        raise ValueError('Authentic IGN_WIN.EXE fingerprint mismatch')
    nt = struct.unpack_from('<I', data, 0x3c)[0]
    if data[nt:nt+4] != b'PE\0\0':
        raise ValueError('Expected PE')
    machine = struct.unpack_from('<H', data, nt+4)[0]
    magic = struct.unpack_from('<H', data, nt+24)[0]
    entry = struct.unpack_from('<I', data, nt+24+16)[0]
    base = struct.unpack_from('<I', data, nt+24+28)[0]
    if (machine, magic, entry, base) != (0x14c, 0x10b, 0x69950, 0x400000):
        raise ValueError('Unexpected Windows PE headers')
    return data


def validation_symbols(pe):
    """Exported PE addresses plus verified image-control member bindings.

    The aliases are harness addresses inside the single production control
    object, not extra globals or linker-contiguity assumptions. Keep legacy
    standalone validation images readable when they export the three fields.
    """
    base = pe.OPTIONAL_HEADER.ImageBase
    symbols = {e.name.decode():base+e.address
        for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    control = symbols.get('g_nativeImageControl')
    if control is not None:
        for name,offset in (('g_nativeNextImageId',0),
                ('g_nativeImageCapacity',8),('g_nativeImageRecords',12)):
            address = control+offset
            if name in symbols and symbols[name] != address:
                raise ValueError('Conflicting image-control export: '+name)
            symbols[name] = address
    return symbols


def validate_platform_imports(pe):
    """Require exactly the authenticated constructor's SDK dependencies.

    Validation DLLs containing the real surface constructor now import APIs.
    This replaces the obsolete no-import assumption, not the dependency audit.
    """
    expected={('DDRAW.dll','DirectDrawCreate'),('GDI32.dll','GetDeviceCaps')}
    expected.update(('USER32.dll',name) for name in (
        'GetSystemMetrics','CreateWindowExA','UpdateWindow','SetFocus','GetDC',
        'ReleaseDC','GetWindowLongA','SetWindowLongA','SetRect','GetMenu',
        'AdjustWindowRectEx','SetWindowPos','SystemParametersInfoA','GetWindowRect',
        'MessageBoxA','ShowWindow'))
    actual={(group.dll.decode(),entry.name.decode() if entry.name else '')
        for group in getattr(pe,'DIRECTORY_ENTRY_IMPORT',()) for entry in group.imports}
    if actual!=expected:
        raise ValueError('Unexpected platform dependencies: '+repr(actual.symmetric_difference(expected)))
    import pefile
    original=pefile.PE(data=verify_target())
    authentic={(group.dll.decode(),entry.name.decode() if entry.name else '')
        for group in original.DIRECTORY_ENTRY_IMPORT for entry in group.imports}
    if not actual.issubset(authentic):
        raise ValueError('Dependencies absent from authentic standard Windows target')
    return actual
