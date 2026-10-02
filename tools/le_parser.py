import struct
import sys
import os

class LEFile:
    def __init__(self, path):
        with open(path, 'rb') as f:
            self.data = f.read()
            
        e_lfanew = struct.unpack('<I', self.data[0x3C:0x40])[0]
        self.le_offset = e_lfanew
        
        self.cpu = struct.unpack('<H', self.data[e_lfanew+8:e_lfanew+10])[0]
        self.num_pages = struct.unpack('<I', self.data[e_lfanew+0x14:e_lfanew+0x18])[0]
        self.eip_obj = struct.unpack('<I', self.data[e_lfanew+0x18:e_lfanew+0x1C])[0]
        self.eip = struct.unpack('<I', self.data[e_lfanew+0x1C:e_lfanew+0x20])[0]
        self.page_size = struct.unpack('<I', self.data[e_lfanew+0x28:e_lfanew+0x2C])[0]
        
        self.obj_table_offset = struct.unpack('<I', self.data[e_lfanew+0x40:e_lfanew+0x44])[0] + e_lfanew
        self.num_objects = struct.unpack('<I', self.data[e_lfanew+0x44:e_lfanew+0x48])[0]
        self.page_map_offset = struct.unpack('<I', self.data[e_lfanew+0x48:e_lfanew+0x4C])[0] + e_lfanew
        self.data_pages_offset = struct.unpack('<I', self.data[e_lfanew+0x80:e_lfanew+0x84])[0]
        
        self.objects = []
        for i in range(self.num_objects):
            o = self.obj_table_offset + i * 24
            vsize, reloc_base, flags, page_idx, num_pages, _ = struct.unpack('<IIIIII', self.data[o:o+24])
            
            # Read pages
            obj_data = bytearray()
            for p in range(num_pages):
                pmap_entry = self.page_map_offset + (page_idx - 1 + p) * 4
                b = self.data[pmap_entry:pmap_entry+4]
                
                idx = (b[0] << 16) | (b[1] << 8) | b[2]
                pflags = b[3]
                
                # type 0=valid, 1=Iterated Data, 2=Invalid, 3=Zeroed, 4=Iterated Data 2, 5=compressed
                if pflags == 0:
                    d_off = self.data_pages_offset + (idx - 1) * self.page_size
                    obj_data.extend(self.data[d_off:d_off+self.page_size])
                elif pflags == 3:
                    obj_data.extend(b'\x00' * self.page_size)
                else:
                    # just append zeroes for unhandled page types
                    obj_data.extend(b'\x00' * self.page_size)
                    
            self.objects.append({
                'vsize': vsize,
                'reloc_base': reloc_base,
                'flags': flags,
                'data': bytes(obj_data[:vsize])
            })

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: le_parser.py <file>")
        sys.exit(1)
    le = LEFile(sys.argv[1])
    print(f"Parsed {le.num_objects} objects.")
    for i, obj in enumerate(le.objects):
        print(f"Object {i+1}: VSize={hex(obj['vsize'])}, RelocBase={hex(obj['reloc_base'])}, Flags={hex(obj['flags'])}, DataLen={hex(len(obj['data']))}")
