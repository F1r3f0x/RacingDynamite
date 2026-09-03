# Ignition (1997) .COL Palette Format

## Overview
`.COL` files contain global and level-specific 256-color palettes (e.g. `SYS.COL`, `LEVELS/AUSTRIA/AUSTRIA.COL`).

## File Structure

| Offset | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32_t` | 4 bytes | **Total File Size**: 776 bytes (`0x00000308`). |
| `0x04` | `uint32_t` | 4 bytes | **Signature / Flags**: `0x0000B123`. |
| `0x08` | `uint8_t[768]` | 768 bytes | **256 Color Entries**: Triplet array `[R, G, B]` (0–255). |

## Memory Mapping in Engine
In the original executable:
```c
// File_LoadToMemory loads the file:
void *pCol = File_LoadToMemory("SYS.COL");
// The engine passes (pCol + 8) to Set_Palette:
Set_Palette(pCol + 8);
```
