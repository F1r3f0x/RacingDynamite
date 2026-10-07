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
