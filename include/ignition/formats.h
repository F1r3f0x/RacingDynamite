/*
 * Racing Dynamite - Modern open-source source port of Ignition (1997)
 * Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IGNITION_FORMATS_H
#define IGNITION_FORMATS_H

#include "ignition/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

// Ignition .PIC Header (846 bytes / 0x34E)
typedef struct {
    uint32_t file_size;       // Total file size in bytes (offset 0..4)
    uint16_t format_id;       // 0x9500 (offset 4..6)
    uint16_t width;           // Image width in pixels (offset 6..8)
    uint16_t height;          // Image height in pixels (offset 8..10)
    uint8_t  reserved1[54];   // Padding up to offset 64
    uint32_t col_size;        // Embedded palette size (0x0308 = 776, offset 64..68)
    uint32_t col_flags;       // Flags (offset 68..72)
    ColorRGB palette[256];    // 256 RGB entries (768 bytes, offset 72..840)
    uint8_t  reserved2[6];    // Padding up to byte 846 (0x34E)
} PicHeader;

// Ignition .COL Header (776 bytes)
typedef struct {
    uint32_t file_size;       // 776 bytes
    uint32_t signature;       // 0x0000B123
    ColorRGB palette[256];    // 256 RGB entries (768 bytes)
} ColHeader;

#pragma pack(pop)

// In-memory decoded image representation
typedef struct {
    uint32_t   width;
    uint32_t   height;
    Palette256 palette;
    uint8_t   *pixels;        // 8-bit paletted pixel buffer (width * height)
} Image8bpp;

// PIC functions
Image8bpp *Pic_LoadFromFile(const char *filepath);
Image8bpp *Pic_LoadFromMemory(const uint8_t *data, size_t size);
void       Pic_Free(Image8bpp *image);

// Convert 8-bit paletted buffer to 32-bit RGBA/BGRA for SDL texture upload
void       Image8bpp_ToRGBA32(const Image8bpp *src, uint32_t *dst_pixels);
void       Image8bpp_ToBGRA32(const Image8bpp *src, uint32_t *dst_pixels);

// COL functions
bool       Col_LoadFromFile(const char *filepath, Palette256 *out_palette);
bool       Col_LoadFromMemory(const uint8_t *data, size_t size, Palette256 *out_palette);

#pragma pack(push, 1)

// Ignition .SRF Header (36 bytes / 9 int32s)
typedef struct {
    int32_t grid_cells_x;     // Number of cells in X (usually 200)
    int32_t grid_cells_z;     // Number of cells in Z (usually 200)
    int32_t cell_size_z;      // Size of one cell in Z (512 units)
    int32_t cell_size_x;      // Size of one cell in X (512 units)
    int32_t grid_stride_x;    // Stride dimension X (101)
    int32_t grid_stride_z;    // Stride dimension Z (101)
    int32_t triangle_count;   // Total surface collision triangles
    int32_t table1_count;     // Secondary index table count
    int32_t table2_count;     // Tertiary index table count
} SrfHeader;

// Spatial grid cell entry (12 bytes)
// @original DAT_004c53d4 grid entries (IGN_WIN.EXE @ 0x00412fc0, getsurf.c)
// @notes Confirmed by FUN_00412fc0 decompile: *(ushort*)(cell+8) = table1_count,
//        *(ushort*)(cell+10) = table2_count. NOT a packed bitfield.
typedef struct {
    int32_t  table2_offset; // +0x00: Byte offset from table2 base to this cell's table2 entries
    int32_t  table1_offset; // +0x04: Byte offset from table1 base to this cell's table1 entries
    uint16_t table1_count;  // +0x08: Number of table1 entries (triangles with dz < 0)
    uint16_t table2_count;  // +0x0A: Number of table2 entries (triangles with dz >= 0)
} SrfCell;

// Surface collision triangle (24 bytes)
typedef struct {
    int32_t v0_y;             // Vertex 0 Y elevation / coordinate
    int32_t v1_y;             // Vertex 1 Y elevation / coordinate
    int32_t v2_y;             // Vertex 2 Y elevation / coordinate
    int32_t material_flags;   // Surface friction / material type
    int32_t normal_x;         // Triangle normal vector X
    int32_t normal_z;         // Triangle normal vector Z
} SrfTriangle;

// In-memory decoded SRF data
typedef struct {
    SrfHeader    header;
    SrfCell     *grid;        // grid_stride_x * grid_stride_z cells
    SrfTriangle *triangles;   // triangle_count triangles
    int32_t     *table1;
    int32_t     *table2;
} SrfData;

// Ignition .PLC Placed Object (20 bytes)
typedef struct {
    int32_t submesh_offset;   // Offset in 4-byte dwords into .MSH geometry
    int32_t model_type;       // Scenery / collision model archetype
    int32_t pos_x;            // World position X
    int32_t pos_y;            // World position Y
    int32_t pos_z;            // World position Z
} PlcObject;

// In-memory PLC data
typedef struct PlcData {
    uint32_t   count;
    PlcObject *objects;
} PlcData;

// Ignition .MSH Polygon Record (44 bytes / 11 x int32)
// @original FUN_00452800 (IGN_WIN.EXE @ 0x00452800, lisa3d.c)
typedef struct {
    uint32_t header;          // Opcode (low byte: 0x11, 0x12, 0x13, 0x15, 0x17) and flags
    uint32_t idx0;            // Index of vertex 0 in submesh vertex array
    uint32_t idx1;            // Index of vertex 1 in submesh vertex array
    uint32_t idx2;            // Index of vertex 2 in submesh vertex array
    int32_t  tu0;             // Vertex 0 U texture coord (8.8 fixed-point: divide by 256.0f for texel [0,255])
    int32_t  tv0;             // Vertex 0 V texture coord (8.8 fixed-point)
    int32_t  tu1;             // Vertex 1 U texture coord (8.8 fixed-point)
    int32_t  tv1;             // Vertex 1 V texture coord (8.8 fixed-point)
    int32_t  tu2;             // Vertex 2 U texture coord (8.8 fixed-point)
    int32_t  tv2;             // Vertex 2 V texture coord (8.8 fixed-point)
    uint32_t extra;           // Texture page byte offset within .TEX file (page_index * 65536)
} MshPolygon;

// In-memory MSH data
typedef struct MshData {
    uint8_t *raw_data;
    size_t   raw_size;
} MshData;

#pragma pack(pop)

// SRF functions
SrfData *Srf_LoadFromFile(const char *filepath);
void     Srf_Free(SrfData *srf);
int32_t  Srf_GetElevationAt(const SrfData *srf, int32_t world_x, int32_t world_z);

// PLC functions
PlcData *Plc_LoadFromFile(const char *filepath);
void     Plc_Free(PlcData *plc);

// MSH functions
MshData *Msh_LoadFromFile(const char *filepath);
void     Msh_Free(MshData *msh);
bool     Msh_GetSubmeshAt(const MshData *msh, int32_t dword_offset,
                          int32_t *out_v_count, int32_t *out_p_count,
                          const int32_t **out_vertices, const MshPolygon **out_polygons);

// TAB (Shading table) - 64 KB (256x256)
typedef struct {
    uint8_t table[256 * 256];
} TabData;

TabData *Tab_LoadFromFile(const char *filepath);
void     Tab_Free(TabData *tab);

// SHD (Shadow / Alpha Blend table) - 64 KB (256x256)
typedef struct {
    uint8_t table[256 * 256];
} ShdData;

ShdData *Shd_LoadFromFile(const char *filepath);
void     Shd_Free(ShdData *shd);

// TEX (Texture page) - 1024x1024 8bpp
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *pixels;
} TexData;

TexData *Tex_LoadFromFile(const char *filepath);
void     Tex_Free(TexData *tex);

#pragma pack(push, 1)

// Ignition .LFT Font Header (12 bytes)
typedef struct {
    char     magic[4];        // "LFT\0"
    uint16_t version;         // 100
    uint16_t ascii_base;      // Starting ASCII character index
    uint16_t max_width;       // Default character spacing/width
    uint16_t height;          // Glyph height in pixels
} LftHeader;

#pragma pack(pop)

// In-memory Font (supports IGNITION.FNT and .LFT)
typedef struct {
    LftHeader header;
    bool      is_fnt;          // True for IGNITION.FNT format
    int16_t   spacing;
    uint8_t   ascii_map[256];  // ASCII char to glyph index
    uint8_t   widths[256];     // Glyph widths
    int32_t   offsets[256];    // Glyph pixel offsets
    uint8_t  *glyph_pixels;    // Raw glyph pixel buffer
    size_t    glyph_data_size;
} LftFont;

LftFont *Lft_LoadFromFile(const char *filepath);
void     Lft_Free(LftFont *font);
void     Font_DrawText(uint8_t *framebuffer, int stride, const LftFont *font, int x, int y, const char *text, uint8_t color_offset);
int      Font_GetTextWidth(const LftFont *font, const char *text);

#pragma pack(push, 1)

// Ignition .POS Keyframe Record (24 bytes)
typedef struct {
    int32_t pos_x;            // 0x00: X position coordinate
    int32_t pos_y;            // 0x04: Y elevation coordinate
    int32_t pos_z;            // 0x08: Z position coordinate
    int32_t rot_x;            // 0x0C: Pitch angle (0..3599 tenths of degree)
    int32_t rot_y;            // 0x10: Yaw angle (0..3599 tenths of degree)
    int32_t rot_z;            // 0x14: Roll angle (0..3599 tenths of degree)
} PosKeyframe;

// Ignition .POS Track Stream Header (8 bytes)
typedef struct {
    int32_t frame_count;      // Total animation keyframes in track
    int32_t current_frame;    // Playhead index (0 during loading)
} PosTrackHeader;

#pragma pack(pop)

// Single animated scenery object track bound to a PLC object
typedef struct {
    uint32_t     object_index; // Index into PlcData objects array
    int32_t      frame_count;  // Total keyframe count
    int32_t      current_frame;// Current playback frame index
    PosKeyframe *keyframes;    // Array of keyframes (frame_count elements)
} PosTrack;

// In-memory decoded .POS animated scenery container
typedef struct {
    uint32_t  track_count;     // Number of animated object tracks
    PosTrack *tracks;          // Array of animated tracks
} PosData;

// POS functions
PosData *Pos_LoadFromFile(const char *filepath, uint32_t plc_count);
void     Pos_Free(PosData *pos);
void     Pos_Update(PosData *pos, PlcData *plc);

#include "ignition/tri.h"


#ifdef __cplusplus
}
#endif

#endif // IGNITION_FORMATS_H
