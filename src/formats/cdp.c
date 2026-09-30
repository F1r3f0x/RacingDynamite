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

#include "ignition/formats.h"
#include <string.h>

/**
 * @original Cdp_OpenFile (MAINDOS_32BIT.EXE @ 0x00412580, lisa3d.c)
 * @fidelity EXACT
 */
bool Cdp_OpenFile(CdpStream *cdp, const uint8_t *data, size_t size) {
    if (!cdp || !data || size < 0x310) {
        return false;
    }

    cdp->is_open = 0;
    if (data[0] == 'C' && data[1] == 'D' && data[2] == 'P' && data[3] == '\0') {
        if (*(const int16_t *)(data + 4) != 100) {
            return false;
        }

        cdp->file_data = (uint8_t *)data;
        cdp->frame_count = *(const int16_t *)(data + 6);
        cdp->loop_flag = *(const int16_t *)(data + 8);
        cdp->width = *(const int16_t *)(data + 10);
        cdp->height = *(const int16_t *)(data + 12);
        cdp->palette = (uint8_t *)(data + 0x10);
        cdp->current_frame = 0;
        cdp->is_open = 1;
        cdp->frame_data_start = (uint8_t *)(data + 0x310);
        cdp->cur_frame_ptr = (uint8_t *)(data + 0x310);
        return true;
    }
    return false;
}

/**
 * @original Cdp_DecompressRLE (MAINDOS_32BIT.EXE @ 0x00499abc, lisa3d.c)
 * @fidelity EXACT
 */
int Cdp_DecompressRLE(CdpStream *cdp) {
    uint8_t *dst;
    uint8_t *src;

    if (!cdp || !cdp->pixel_buffer || !cdp->cur_frame_ptr) {
        return 0;
    }

    dst = cdp->pixel_buffer;
    src = cdp->cur_frame_ptr;

    while (1) {
        uint8_t op = *src++;
        switch (op) {
            case 0xF6:
                *dst++ = *src++;
                break;
            case 0xF7:
                dst += 2;
                break;
            case 0xF8:
                dst += 3;
                break;
            case 0xF9:
                dst += 4;
                break;
            case 0xFA:
                dst += 5;
                break;
            case 0xFB:
                dst += *src++;
                break;
            case 0xFC: {
                uint16_t skip = *(const uint16_t *)src;
                src += 2;
                dst += skip;
                break;
            }
            case 0xFD: {
                uint8_t count = *src++;
                uint8_t val = *src++;
                memset(dst, val, count);
                dst += count;
                break;
            }
            case 0xFE: {
                uint16_t count = *(const uint16_t *)src;
                src += 2;
                uint8_t val = *src++;
                memset(dst, val, count);
                dst += count;
                break;
            }
            case 0xFF:
                cdp->cur_frame_ptr = src;
                cdp->current_frame++;
                if (cdp->current_frame >= cdp->frame_count) {
                    cdp->cur_frame_ptr = cdp->frame_data_start;
                    cdp->current_frame = 0;
                    if (cdp->loop_flag != 1) {
                        return 0;
                    }
                }
                return 1;
            default:
                *dst++ = op;
                break;
        }
    }
}

/**
 * @original Cdp_DecodeFrame (MAINDOS_32BIT.EXE @ 0x00412610, lisa3d.c)
 * @fidelity EXACT
 */
int Cdp_DecodeFrame(CdpStream *cdp) {
    if (!cdp || !cdp->is_open) {
        return -1;
    }
    return Cdp_DecompressRLE(cdp);
}
