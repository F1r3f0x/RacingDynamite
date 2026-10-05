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
        
        self.fixup_page_tbl_off = struct.unpack('<I', self.data[e_lfanew+0x68:e_lfanew+0x6C])[0] + e_lfanew
        self.fixup_rec_tbl_off = struct.unpack('<I', self.data[e_lfanew+0x6C:e_lfanew+0x70])[0] + e_lfanew

        self.objects = []
        page_to_obj = {}  # page_idx -> (obj_index, offset_within_obj, vaddr)

        for i in range(self.num_objects):
            o = self.obj_table_offset + i * 24
            vsize, reloc_base, flags, pidx, pcount, _ = struct.unpack('<IIIIII', self.data[o:o+24])
            
            # Read pages
            obj_data = bytearray()
            for p in range(pcount):
                global_p = (pidx - 1) + p
                page_to_obj[global_p] = (i, p * self.page_size, reloc_base + p * self.page_size)

                pmap_entry = self.page_map_offset + global_p * 4
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
                    obj_data.extend(b'\x00' * self.page_size)
                    
            self.objects.append({
                'vsize': vsize,
                'reloc_base': reloc_base,
                'flags': flags,
                'data': obj_data
            })

        # Apply LE fixups directly in memory
        obj_bases = {idx + 1: obj['reloc_base'] for idx, obj in enumerate(self.objects)}
        raw = self.data

        for p in range(self.num_pages):
            if p not in page_to_obj:
                continue
            rec_start = self.fixup_rec_tbl_off + struct.unpack_from("<I", raw, self.fixup_page_tbl_off + p * 4)[0]
            rec_end = self.fixup_rec_tbl_off + struct.unpack_from("<I", raw, self.fixup_page_tbl_off + (p + 1) * 4)[0]

            obj_idx, page_off_in_obj, page_vaddr = page_to_obj[p]
            target_data = self.objects[obj_idx]['data']

            pos = rec_start
            while pos < rec_end:
                stype = raw[pos]
                tflags = raw[pos + 1]
                soff = struct.unpack_from("<h", raw, pos + 2)[0]
                pos += 4

                if tflags & 0x40:
                    tobj = struct.unpack_from("<H", raw, pos)[0]
                    pos += 2
                else:
                    tobj = raw[pos]
                    pos += 1

                if tflags & 0x10:
                    toff = struct.unpack_from("<I", raw, pos)[0]
                    pos += 4
                else:
                    toff = struct.unpack_from("<H", raw, pos)[0]
                    pos += 2

                target_linear = obj_bases[tobj] + toff
                source_offset = page_off_in_obj + soff
                source_vaddr = page_vaddr + soff

                if 0 <= source_offset + 4 <= len(target_data):
                    if stype == 0x7: # 32-bit linear address
                        curr_val = struct.unpack_from("<I", target_data, source_offset)[0]
                        new_val = (obj_bases[tobj] + curr_val) & 0xFFFFFFFF
                        struct.pack_into("<I", target_data, source_offset, new_val)
                    elif stype == 0x8: # 32-bit relative offset
                        rel_val = (target_linear - (source_vaddr + 4)) & 0xFFFFFFFF
                        struct.pack_into("<I", target_data, source_offset, rel_val)

        # Convert back to read-only bytes truncated to vsize
        for obj in self.objects:
            obj['data'] = bytes(obj['data'][:obj['vsize']])

    def read_vaddr(self, vaddr, size=4):
        for obj in self.objects:
            base = obj['reloc_base']
            vsize = obj['vsize']
            if base <= vaddr < base + vsize:
                off = vaddr - base
                return obj['data'][off:off+size]
        return None

    def read_dword(self, vaddr):
        b = self.read_vaddr(vaddr, 4)
        return struct.unpack('<I', b)[0] if b and len(b) == 4 else None

    def read_string(self, vaddr, max_len=256):
        data = self.read_vaddr(vaddr, max_len)
        if not data:
            return ""
        end = data.find(b'\x00')
        if end != -1:
            data = data[:end]
        return data.decode('latin1', errors='replace')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: le_parser.py <file>")
        sys.exit(1)
    le = LEFile(sys.argv[1])
    print(f"Parsed {le.num_objects} objects directly from {sys.argv[1]}.")
    for i, obj in enumerate(le.objects):
        print(f"Object {i+1}: VSize={hex(obj['vsize'])}, RelocBase={hex(obj['reloc_base'])}, Flags={hex(obj['flags'])}, DataLen={hex(len(obj['data']))}")
