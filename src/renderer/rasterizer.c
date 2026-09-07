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

#include "ignition/renderer.h"
#include "ignition/physics.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define DEG2RAD (3.14159265358979323846f / 180.0f)

static inline Vec3 Vec3_Sub(Vec3 a, Vec3 b) {
    Vec3 r = { a.x - b.x, a.y - b.y, a.z - b.z };
    return r;
}

static inline float Vec3_Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
    Vec3 r = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return r;
}

static inline Vec3 Vec3_Normalize(Vec3 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 1e-6f) {
        float inv = 1.0f / len;
        v.x *= inv;
        v.y *= inv;
        v.z *= inv;
    }
    return v;
}

void Renderer_GetDefaultOptions(RendererOptions *opts) {
    if (!opts) return;
    opts->backend_type = RENDERER_BACKEND_LISA3D_SOFTWARE;
    opts->authentic_fixed_point_uv = true;
    opts->authentic_depth_buckets = true; // Authentic 1997 6,000 depth buckets by default
    opts->authentic_dithering = false;
    opts->force_8bit_paletted = true;
    opts->render_panoramas = true;
    opts->internal_width = 640;
    opts->internal_height = 480;
}

void Renderer_SetOptions(Renderer3D *r, const RendererOptions *opts) {
    if (!r || !opts) return;
    r->options = *opts;
}

/**
 * @brief Initializes 3D renderer state, framebuffers, depth buffer, and default options.
 * @original FUN_0043e2a0 (IGN_WIN.EXE @ 0x0043e2a0, lisa3d.c)
 * @fidelity ADAPTED
 */
bool Renderer_Init(Renderer3D *r, int width, int height) {
    if (!r || width <= 0 || height <= 0) return false;
    memset(r, 0, sizeof(Renderer3D));

    r->width = width;
    r->height = height;
    r->zbuffer = (float *)calloc((size_t)width * (size_t)height, sizeof(float));
    if (!r->zbuffer) return false;

    Renderer_GetDefaultOptions(&r->options);
    r->options.internal_width = width;
    r->options.internal_height = height;

    return true;
}

/**
 * @brief Releases renderer framebuffers and allocated depth buffers.
 * @original FUN_00412530 (IGN_WIN.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Renderer_Shutdown(Renderer3D *r) {
    if (!r) return;
    if (r->zbuffer) {
        free(r->zbuffer);
        r->zbuffer = NULL;
    }
}

/**
 * @brief Clears virtual framebuffer and depth buffer.
 * @original FUN_004468d0 (IGN_WIN.EXE @ 0x004468d0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Renderer_Clear(Renderer3D *r, uint8_t clear_color, float clear_depth) {
    (void)clear_depth;
    if (!r) return;
    if (r->framebuffer) {
        memset(r->framebuffer, clear_color, (size_t)r->width * (size_t)r->height);
    }
    if (r->zbuffer) {
        // Clear 1/Z depth buffer to 0.0 (infinitely far)
        memset(r->zbuffer, 0, (size_t)r->width * (size_t)r->height * sizeof(float));
    }
}

void Renderer_SetTexture(Renderer3D *r, const TexData *tex) {
    if (r) r->active_texture = tex;
}

void Renderer_SetShading(Renderer3D *r, const TabData *tab) {
    if (r) r->active_shading = tab;
}

void Renderer_SetShadow(Renderer3D *r, const ShdData *shd) {
    if (r) r->active_shadow = shd;
}

typedef struct {
    float x, y;             // 2D Screen coordinates
    float inv_z;            // 1 / Z_camera (linear in screen space)
    float u_over_z;         // U / Z_camera (perspective correct)
    float v_over_z;         // V / Z_camera (perspective correct)
    float light_over_z;     // Light / Z_camera
    uint32_t page_offset;   // Byte offset of 64KB page
    bool is_transparent;    // Alpha test (color key 0)
    bool is_shadow_blend;   // Shadow / alpha remapping via SHD table
    uint8_t color;
} ProjectedVertex;

/**
 * @brief Transforms 3D vertex to camera space, applies near/far clipping and perspective projection.
 * @original FUN_00449e70 (IGN_WIN.EXE @ 0x00449e70, lisa3d.c)
 * @fidelity ADAPTED
 */
static bool ProjectVertex(const Renderer3D *r, const Camera3D *cam,
                          Vec3 forward, Vec3 right, Vec3 up, float fov_scale,
                          const Vertex3D *in_v, ProjectedVertex *out_pv) {
    Vec3 d = Vec3_Sub(in_v->pos, cam->position);

    float z_cam = Vec3_Dot(d, forward);
    if (z_cam < cam->near_z || z_cam > cam->far_z) {
        return false;
    }

    float x_cam = Vec3_Dot(d, right);
    float y_cam = Vec3_Dot(d, up);

    float inv_z = 1.0f / z_cam;
    out_pv->x = (x_cam * fov_scale * inv_z) + ((float)r->width * 0.5f);
    out_pv->y = (-y_cam * fov_scale * inv_z) + ((float)r->height * 0.5f);
    out_pv->inv_z = inv_z;

    out_pv->u_over_z = in_v->uv.u * inv_z;
    out_pv->v_over_z = in_v->uv.v * inv_z;
    out_pv->light_over_z = in_v->light * inv_z;
    out_pv->page_offset = in_v->page_offset;
    out_pv->is_transparent = in_v->is_transparent;
    out_pv->is_shadow_blend = in_v->is_shadow_blend;
    out_pv->color = in_v->color;
    return true;
}

/**
 * @brief Rasterizes single 3D triangle with perspective texture mapping, shading, and alpha/shadow blend.
 * @original FUN_0044d550 (IGN_WIN.EXE @ 0x0044d550, lisa3d.c)
 * @original FUN_00452800 (IGN_WIN.EXE @ 0x00452800, lisa3d.c)
 * @original FUN_004537dc (IGN_WIN.EXE @ 0x004537dc, lisa3d.c)
 * @original FUN_0044f0e9 (IGN_WIN.EXE @ 0x0044f0e9, lisa3d.c)
 * @fidelity ADAPTED
 * @deviation DEV-005 (1/Z depth buffer precision vs authentic 6,000 depth buckets)
 * @fix_category FIX_CAT_RENDERER
 */
void Renderer_DrawTriangle(Renderer3D *r, const Camera3D *cam,
                           const Vertex3D *v0, const Vertex3D *v1, const Vertex3D *v2) {
    if (!r || !cam || !r->framebuffer || !r->zbuffer) return;

    Vec3 forward = Vec3_Normalize(Vec3_Sub(cam->target, cam->position));
    Vec3 world_up = { 0.0f, 1.0f, 0.0f };
    Vec3 right = Vec3_Normalize(Vec3_Cross(forward, world_up));
    Vec3 up = Vec3_Cross(right, forward);

    float fov_scale = ((float)r->height * 0.5f) / tanf((cam->fov * 0.5f) * DEG2RAD);

    ProjectedVertex p0, p1, p2;
    if (!ProjectVertex(r, cam, forward, right, up, fov_scale, v0, &p0) ||
        !ProjectVertex(r, cam, forward, right, up, fov_scale, v1, &p1) ||
        !ProjectVertex(r, cam, forward, right, up, fov_scale, v2, &p2)) {
        return;
    }

    // 2D Backface culling & signed area
    // In screen coordinates (+Y down), front-facing triangles have negative signed area.
    // Discard back-facing and degenerate triangles (area >= -0.001f).
    float area = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
    if (area >= -0.001f) return;

    // Swap p1 and p2 to establish counter-clockwise screen winding (positive area)
    // for standard barycentric coordinate evaluation.
    ProjectedVertex tmp = p1;
    p1 = p2;
    p2 = tmp;
    area = -area;

    float inv_area = 1.0f / area;

    // Bounding box
    int min_x = (int)floorf(fminf(p0.x, fminf(p1.x, p2.x)));
    int max_x = (int)ceilf(fmaxf(p0.x, fmaxf(p1.x, p2.x)));
    int min_y = (int)floorf(fminf(p0.y, fminf(p1.y, p2.y)));
    int max_y = (int)ceilf(fmaxf(p0.y, fmaxf(p1.y, p2.y)));

    if (min_x < 0) min_x = 0;
    if (max_x >= r->width) max_x = r->width - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= r->height) max_y = r->height - 1;
    if (min_x > max_x || min_y > max_y) return;

    const uint8_t *tex_pixels = r->active_texture ? r->active_texture->pixels : NULL;
    const uint8_t *shading_tab = r->active_shading ? r->active_shading->table : NULL;
    const uint8_t *shadow_tab = r->active_shadow ? r->active_shadow->table : NULL;

    for (int y = min_y; y <= max_y; ++y) {
        float py = (float)y + 0.5f;
        int row_idx = y * r->width;

        for (int x = min_x; x <= max_x; ++x) {
            float px = (float)x + 0.5f;

            float w0 = ((p1.x - px) * (p2.y - py) - (p1.y - py) * (p2.x - px)) * inv_area;
            float w1 = ((p2.x - px) * (p0.y - py) - (p2.y - py) * (p0.x - px)) * inv_area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                // Perspective-correct 1/Z depth test (near objects have larger 1/Z)
                float inv_z = w0 * p0.inv_z + w1 * p1.inv_z + w2 * p2.inv_z;
                int pixel_idx = row_idx + x;

                // DEV-005: 1/Z depth buffer precision vs authentic 6,000 depth buckets
                float test_inv_z = inv_z;
                if (r->options.authentic_depth_buckets) {
                    test_inv_z = floorf(inv_z * 6000.0f) / 6000.0f;
                }

                // For shadow blend triangles, use slight depth bias (-0.00005f) so shadows on road don't z-fight
                bool depth_pass = p0.is_shadow_blend ? (test_inv_z >= r->zbuffer[pixel_idx] - 0.00005f)
                                                     : (test_inv_z > r->zbuffer[pixel_idx]);
                if (depth_pass) {
                    float z_real = 1.0f / inv_z;
                    uint8_t col = p0.color;
                    if (tex_pixels) {
                        int tu, tv;
                        if (r->options.authentic_fixed_point_uv) {
                            // Authentic 1997 Lisa3D 16.16 fixed-point UV interpolation
                            int32_t u_fp = (int32_t)((w0 * p0.u_over_z + w1 * p1.u_over_z + w2 * p2.u_over_z) * z_real * 65536.0f);
                            int32_t v_fp = (int32_t)((w0 * p0.v_over_z + w1 * p1.v_over_z + w2 * p2.v_over_z) * z_real * 65536.0f);
                            tu = (u_fp >> 8) & 255;
                            tv = (v_fp >> 8) & 255;
                        } else {
                            float u = (w0 * p0.u_over_z + w1 * p1.u_over_z + w2 * p2.u_over_z) * z_real;
                            float v = (w0 * p0.v_over_z + w1 * p1.v_over_z + w2 * p2.v_over_z) * z_real;
                            tu = ((int)floorf(u * 256.0f)) & 255;
                            tv = ((int)floorf(v * 256.0f)) & 255;
                        }
                        col = tex_pixels[p0.page_offset + tv * 256 + tu];

                        // Transparent / color key 0 discard for cutout and shadow polygons
                        if ((p0.is_transparent || p0.is_shadow_blend) && col == 0) {
                            continue;
                        }

                        // Authentic Shadow / Alpha blending via SHD table
                        if (p0.is_shadow_blend && shadow_tab) {
                            uint8_t bg = r->framebuffer[pixel_idx];
                            col = shadow_tab[((uint32_t)col << 8) | bg];
                            if (col == bg) {
                                continue;
                            }
                        }
                    }

                    // Authentic Lisa3D behavior: Only opaque and cutout geometry writes to the Z-buffer.
                    // Shadow / translucent blend polygons never write to the Z-buffer.
                    if (!p0.is_shadow_blend) {
                        r->zbuffer[pixel_idx] = test_inv_z;
                    }

                    float light = (w0 * p0.light_over_z + w1 * p1.light_over_z + w2 * p2.light_over_z) * z_real;
                    if (light < 0.0f) light = 0.0f;
                    if (light > 1.0f) light = 1.0f;

                    // Authentic TAB lookup: Row is texel color, Column is light level (0 = bright, 63 = dark)
                    // Opcode 0x11 polygons (unshaded track geometry) run with light >= 0.98f and bypass TAB lookup
                    if (shading_tab && light < 0.98f && !p0.is_shadow_blend) {
                        uint8_t light_level = (uint8_t)((1.0f - light) * 63.0f);
                        if (light_level > 63) light_level = 63;
                        col = shading_tab[((uint32_t)col << 8) | light_level];
                    }

                    r->framebuffer[pixel_idx] = col;
                }
            }
        }
    }
}

void Renderer_DrawTrackSurface(Renderer3D *r, const Camera3D *cam, const SrfData *srf) {
    (void)r; (void)cam; (void)srf;
}

void Renderer_DrawPlacedObjects(Renderer3D *r, const Camera3D *cam, const PlcData *plc) {
    (void)r; (void)cam; (void)plc;
}

/**
 * @brief Renders all scenery and track meshes placed by .PLC file across two passes.
 * @original FUN_00416250 (IGN_WIN.EXE @ 0x00416250, main.c)
 * @original FUN_004466d0 (IGN_WIN.EXE @ 0x004466d0, lisa3d.c)
 * @original FUN_00448e70 (IGN_WIN.EXE @ 0x00448e70, lisa3d.c)
 * @original FUN_0044b480 (IGN_WIN.EXE @ 0x0044b480, lisa3d.c)
 * @original FUN_0044c1f0 (IGN_WIN.EXE @ 0x0044c1f0, lisa3d.c)
 * @original LAB_0044caa0 (IGN_WIN.EXE @ 0x0044caa0, lisa3d.c)
 * @original LAB_0044cac0 (IGN_WIN.EXE @ 0x0044cac0, lisa3d.c)
 * @original LAB_0044cae0 (IGN_WIN.EXE @ 0x0044cae0, lisa3d.c)
 * @original LAB_0044cb00 (IGN_WIN.EXE @ 0x0044cb00, lisa3d.c)
 * @original FUN_0044cb20 (IGN_WIN.EXE @ 0x0044cb20, lisa3d.c)
 * @fidelity ADAPTED
 */
void Renderer_DrawTrackMesh(Renderer3D *r, const Camera3D *cam, const MshData *msh, const PlcData *plc) {
    if (!r || !cam || !msh || !plc || !plc->objects) return;

    // Authentic Lisa3D two-pass rendering:
    // Pass 0: Opaque & cutout geometry (depth test with Z-write).
    // Pass 1: Translucent & shadow blend geometry (depth test with bias, blends via SHD, no Z-write).
    for (int pass = 0; pass < 2; ++pass) {
        for (uint32_t i = 0; i < plc->count; ++i) {
            const PlcObject *obj = &plc->objects[i];

            // Authentic Lisa3D 12-bit archetype mask from Track_PreprocessPlacements (FUN_0041b360)
            uint32_t raw_type = (uint32_t)obj->model_type & 0xFFF;
            // Archetype check from Lisa_DrawTriangle_OpcodeHelper (0x0044cb20):
            // Only objects in ranges [0, 99], 200, and [300, 302] are shadow/alpha blend models.
            bool is_shadow_type = (raw_type < 100 || raw_type == 200 || (raw_type >= 300 && raw_type <= 302));

            // Pass 1 only renders shadow blend objects; skip all non-shadow objects immediately.
            if (pass == 1 && !is_shadow_type) {
                continue;
            }

            int32_t v_count = 0, p_count = 0;
            const int32_t *vertices = NULL;
            const MshPolygon *polygons = NULL;

            if (!Msh_GetSubmeshAt(msh, obj->submesh_offset, &v_count, &p_count, &vertices, &polygons)) {
                continue;
            }

            float obj_x = (float)obj->pos_x;
            float obj_y = (float)obj->pos_y;
            float obj_z = (float)obj->pos_z;

            float cdx = obj_x - cam->position.x;
            float cdy = obj_y - cam->position.y;
            float cdz = obj_z - cam->position.z;
            float dist_sq = cdx * cdx + cdy * cdy + cdz * cdz;
            if (dist_sq > 40000.0f * 40000.0f) {
                continue;
            }

            for (int32_t p = 0; p < p_count; ++p) {
                const MshPolygon *poly = &polygons[p];
                if (poly->idx0 >= (uint32_t)v_count ||
                    poly->idx1 >= (uint32_t)v_count ||
                    poly->idx2 >= (uint32_t)v_count) {
                    continue;
                }

                uint8_t opcode = (uint8_t)(poly->header & 0xFF);

                // Authentic Lisa3D opcode routing:
                // Opcodes 0x13 and 0x17 only act as shadow blends (0x15 in OT) if model type is a shadow archetype.
                // Otherwise, they route to 0x12 (1-bit transparency cutout with color key 0).
                bool is_shadow_blend = (opcode == 0x13 || opcode == 0x17) && is_shadow_type;
                bool is_transparent = (opcode == 0x12 || opcode == 0x16) || ((opcode == 0x13 || opcode == 0x17) && !is_shadow_type);

                if (pass == 0 && is_shadow_blend) {
                    continue; // Defer to Pass 1
                }
                if (pass == 1 && !is_shadow_blend) {
                    continue; // Rendered in Pass 0
                }

                // Authentic Lisa3D formula: world Y = obj_y - vertex.y
                Vec3 p0 = {
                    (float)vertices[poly->idx0 * 3] + obj_x,
                    obj_y - (float)vertices[poly->idx0 * 3 + 1],
                    (float)vertices[poly->idx0 * 3 + 2] + obj_z
                };
                Vec3 p1 = {
                    (float)vertices[poly->idx1 * 3] + obj_x,
                    obj_y - (float)vertices[poly->idx1 * 3 + 1],
                    (float)vertices[poly->idx1 * 3 + 2] + obj_z
                };
                Vec3 p2 = {
                    (float)vertices[poly->idx2 * 3] + obj_x,
                    obj_y - (float)vertices[poly->idx2 * 3 + 1],
                    (float)vertices[poly->idx2 * 3 + 2] + obj_z
                };

                // Authentic Lisa3D texture page offset & opcode
                uint32_t page_offset = poly->extra;

                // Authentic UV mapping: 0..65536 maps to 0..256 texels within page
                float u0 = (float)poly->tu0 / 65536.0f;
                float v0 = (float)poly->tv0 / 65536.0f;
                float u1 = (float)poly->tu1 / 65536.0f;
                float v1 = (float)poly->tv1 / 65536.0f;
                float u2 = (float)poly->tu2 / 65536.0f;
                float v2 = (float)poly->tv2 / 65536.0f;

                // Compute face normal for 3D diffuse facet lighting
                float e1x = p1.x - p0.x, e1y = p1.y - p0.y, e1z = p1.z - p0.z;
                float e2x = p2.x - p0.x, e2y = p2.y - p0.y, e2z = p2.z - p0.z;
                float nx = e1y * e2z - e1z * e2y;
                float ny = e1z * e2x - e1x * e2z;
                float nz = e1x * e2y - e1y * e2x;
                float nlen = sqrtf(nx * nx + ny * ny + nz * nz);
                float diffuse = 1.0f;
                if (nlen > 1e-4f) {
                    nx /= nlen; ny /= nlen; nz /= nlen;
                    float dot = nx * 0.408f + ny * 0.816f + nz * 0.408f;
                    if (dot < 0.25f) dot = 0.25f;
                    if (dot > 1.0f) dot = 1.0f;
                    diffuse = dot;
                }

                // When untextured, use 3D normal diffuse lighting; when textured, use 1.0f (unshaded)
                float light = (!r->active_texture) ? diffuse : 1.0f;
                uint8_t base_color = (uint8_t)(32 + ((raw_type * 37 + (uint32_t)i * 13) % 200));

                Vertex3D vert0 = { p0, { u0, v0 }, light, page_offset, is_transparent, is_shadow_blend, base_color };
                Vertex3D vert1 = { p1, { u1, v1 }, light, page_offset, is_transparent, is_shadow_blend, base_color };
                Vertex3D vert2 = { p2, { u2, v2 }, light, page_offset, is_transparent, is_shadow_blend, base_color };

                Renderer_DrawTriangle(r, cam, &vert0, &vert1, &vert2);
            }
        }
    }
}

void Renderer_DrawLine3D(Renderer3D *r, const Camera3D *cam, Vec3 p0, Vec3 p1, uint8_t color) {
    if (!r || !cam || !r->framebuffer || !r->zbuffer) return;

    Vec3 forward = Vec3_Normalize(Vec3_Sub(cam->target, cam->position));
    Vec3 world_up = { 0.0f, 1.0f, 0.0f };
    Vec3 right = Vec3_Normalize(Vec3_Cross(forward, world_up));
    Vec3 up = Vec3_Cross(right, forward);

    float fov_scale = ((float)r->height * 0.5f) / tanf((cam->fov * 0.5f) * DEG2RAD);

    Vec3 d0 = Vec3_Sub(p0, cam->position);
    Vec3 d1 = Vec3_Sub(p1, cam->position);

    float z0 = Vec3_Dot(d0, forward);
    float z1 = Vec3_Dot(d1, forward);

    float near_z = cam->near_z > 10.0f ? cam->near_z : 10.0f;
    if (z0 < near_z && z1 < near_z) return;

    float x0 = Vec3_Dot(d0, right);
    float y0 = Vec3_Dot(d0, up);
    float x1 = Vec3_Dot(d1, right);
    float y1 = Vec3_Dot(d1, up);

    if (z0 < near_z) {
        float t = (near_z - z0) / (z1 - z0);
        x0 = x0 + t * (x1 - x0);
        y0 = y0 + t * (y1 - y0);
        z0 = near_z;
    } else if (z1 < near_z) {
        float t = (near_z - z1) / (z0 - z1);
        x1 = x1 + t * (x0 - x1);
        y1 = y1 + t * (y0 - y1);
        z1 = near_z;
    }

    if (z0 > cam->far_z && z1 > cam->far_z) return;

    float inv_z0 = 1.0f / z0;
    float inv_z1 = 1.0f / z1;

    float sx0 = (x0 * fov_scale * inv_z0) + ((float)r->width * 0.5f);
    float sy0 = (-y0 * fov_scale * inv_z0) + ((float)r->height * 0.5f);
    float sx1 = (x1 * fov_scale * inv_z1) + ((float)r->width * 0.5f);
    float sy1 = (-y1 * fov_scale * inv_z1) + ((float)r->height * 0.5f);

    int ix0 = (int)roundf(sx0);
    int iy0 = (int)roundf(sy0);
    int ix1 = (int)roundf(sx1);
    int iy1 = (int)roundf(sy1);

    int dx = abs(ix1 - ix0);
    int dy = abs(iy1 - iy0);
    int sx = ix0 < ix1 ? 1 : -1;
    int sy = iy0 < iy1 ? 1 : -1;
    int err = dx - dy;

    int steps = dx > dy ? dx : dy;
    if (steps == 0) steps = 1;
    float inv_steps = 1.0f / (float)steps;
    int cur_step = 0;

    while (1) {
        float t = (float)cur_step * inv_steps;
        float cur_inv_z = inv_z0 + t * (inv_z1 - inv_z0);

        // 2x2 stamp for line thickness and clarity on 640x480 display
        for (int oy = 0; oy <= 1; ++oy) {
            int py = iy0 + oy;
            if (py < 0 || py >= r->height) continue;
            for (int ox = 0; ox <= 1; ++ox) {
                int px = ix0 + ox;
                if (px < 0 || px >= r->width) continue;
                int idx = py * r->width + px;
                // Bias depth slightly (-0.00005f) so lines on top of track road surface don't z-fight
                if (cur_inv_z >= r->zbuffer[idx] - 0.00005f) {
                    r->zbuffer[idx] = cur_inv_z;
                    r->framebuffer[idx] = color;
                }
            }
        }

        if (ix0 == ix1 && iy0 == iy1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; ix0 += sx; }
        if (e2 < dx)  { err += dx; iy0 += sy; }
        cur_step++;
    }
}

void Renderer_DrawWaypoints(Renderer3D *r, const Camera3D *cam, const TrackWaypoints *wp) {
    if (!r || !cam || !wp || !wp->waypoints || wp->count == 0) return;

    // Palette colors in Ignition:
    // 215: Bright yellow (circuit trajectory line)
    // 160: Cyan (lateral boundary road crossbars)
    // 251: Bright white (waypoint center node pins)
    // 240: Bright red/orange (start / finish line gate)
    const uint8_t color_center = 215;
    const uint8_t color_crossbar = 160;
    const uint8_t color_pin = 251;
    const uint8_t color_start = 240;

    const float lift = 40.0f; // Lift above asphalt surface to prevent z-fighting

    for (uint32_t i = 0; i < wp->count; ++i) {
        const TrackWaypoint *curr = &wp->waypoints[i];
        uint32_t next_idx = (i + 1 < wp->count) ? i + 1 : 0;
        const TrackWaypoint *next = &wp->waypoints[next_idx];

        Vec3 p0 = { curr->center_x, curr->center_y + lift, curr->center_z };
        Vec3 p1 = { next->center_x, next->center_y + lift, next->center_z };

        // 1. Centerline trajectory connecting consecutive waypoints
        uint8_t line_col = (i == 0) ? color_start : color_center;
        Renderer_DrawLine3D(r, cam, p0, p1, line_col);

        // 2. Lateral road boundary crossbar (left to right rail)
        Vec3 left = { curr->left_x, curr->left_y + lift, curr->left_z };
        Vec3 right = { curr->right_x, curr->right_y + lift, curr->right_z };
        Renderer_DrawLine3D(r, cam, left, right, (i == 0) ? color_start : color_crossbar);

        // 3. Vertical indicator pin at waypoint center
        Vec3 pin_tip = { curr->center_x, curr->center_y + lift + 80.0f, curr->center_z };
        Renderer_DrawLine3D(r, cam, p0, pin_tip, (i == 0) ? color_start : color_pin);
    }
}

static const int32_t s_car_body_submesh[CAR_ARCHETYPE_COUNT] = {
    5464,  // 0: Cooper (type 0)
    3672,  // 1: Evor / Porsche (type 20)
    2790,  // 2: Buggy / Jeep (type 30)
    1032,  // 3: Enforcer / Police Cop (type 90)
    10839, // 4: Red Devil / Mustang (type 40)
    14383, // 5: School Bus (type 70)
    16815, // 6: Smoke / Van (type 50)
    8875,  // 7: Bug / VW Beetle (type 10)
    7078,  // 8: Monster / Truck (type 60)
    18585, // 9: Vegas / Dodge (type 80)
    12547  // 10: Ignition / NASCAR (type 100)
};

static const uint8_t s_car_fallback_colors[CAR_ARCHETYPE_COUNT] = {
    160, // 0: Cooper: cyan
    15,  // 1: Evor: yellow / white
    120, // 2: Buggy: green
    31,  // 3: Enforcer: dark blue
    224, // 4: Red Devil: bright red
    208, // 5: School Bus: amber
    48,  // 6: Smoke: maroon
    200, // 7: Bug: yellow
    48,  // 8: Monster: red/black
    192, // 9: Vegas: purple
    250  // 10: Ignition: flame red
};

/**
 * @brief Renders vehicle 3D model with chassis roll/pitch/yaw transforms and ground alignment.
 * @original FUN_0041d190 (IGN_WIN.EXE @ 0x0041d190, main.c / lisa3d.c)
 * @fidelity EXTENDED
 * @deviation DEV-002 (Tire contact vertex alignment)
 * @fix_category FIX_CAT_ELEVATION
 */
void Renderer_DrawCar(Renderer3D *r, const Camera3D *cam, const MshData *cars_msh, const TexData *cars_tex, const VehicleState *veh) {
    if (!r || !cam || !veh) return;

    int car_idx = veh->car_index;
    if (car_idx < 0 || car_idx >= CAR_ARCHETYPE_COUNT) car_idx = 0;

    float r_cos = cosf((float)veh->roll);
    float r_sin = sinf((float)veh->roll);
    float p_cos = cosf((float)veh->pitch);
    float p_sin = sinf((float)veh->pitch);
    float y_cos = cosf((float)veh->yaw);
    float y_sin = sinf((float)veh->yaw);

    // If CARS.MSH is loaded, render the 3D car body mesh
    if (cars_msh && cars_msh->raw_data) {
        int32_t sub_offset = s_car_body_submesh[car_idx];
        int32_t v_count = 0, p_count = 0;
        const int32_t *vertices = NULL;
        const MshPolygon *polygons = NULL;

        if (Msh_GetSubmeshAt(cars_msh, sub_offset, &v_count, &p_count, &vertices, &polygons)) {
            // Find bottom-most Y (maximum vy in CARS.MSH coords) to align wheels with ground (FUN_0041d190)
            int32_t max_vy = -10000;
            for (int32_t i = 0; i < v_count; ++i) {
                int32_t vy_val = vertices[i * 3 + 1];
                if (vy_val > max_vy) max_vy = vy_val;
            }

            float scale = (float)veh->params.chassis_scale;

            const TexData *prev_tex = r->active_texture;
            if (cars_tex) r->active_texture = cars_tex;
            for (int32_t p = 0; p < p_count; ++p) {
                const MshPolygon *poly = &polygons[p];
                if (poly->idx0 >= (uint32_t)v_count ||
                    poly->idx1 >= (uint32_t)v_count ||
                    poly->idx2 >= (uint32_t)v_count) {
                    continue;
                }

                uint32_t indices[3] = { poly->idx0, poly->idx1, poly->idx2 };
                Vec3 p_world[3];

                for (int v = 0; v < 3; v++) {
                    float vx = (float)vertices[indices[v] * 3 + 0]; // Front (+X) / Rear (-X)
                    float vy = (float)vertices[indices[v] * 3 + 1]; // Roof (-Y) / Ground (+Y)
                    float vz = (float)vertices[indices[v] * 3 + 2]; // Right (+Z) / Left (-Z)

                    // Map CARS.MSH axes to local chassis space:
                    // In CARS.MSH: +X=Front, -X=Rear, -Y=Roof, +Y=Ground, +Z=Left, -Z=Right
                    // Authentic alignment (FUN_0041d190): subtract max_vy so bottom of tires is at Y = 0.
                    // Chassis Right (+X) = -vz * scale
                    // Chassis Up (+Y) = (max_vy - vy) * scale - PHYSICS_RIDE_HEIGHT (sits on ground contact)
                    // Chassis Front (+Z) = vx * scale
                    float lx = -vz * scale;
                    float ly = (float)(max_vy - vy) * scale - 5.0f;
                    float lz = vx * scale;


                    // 1. Roll around chassis longitudinal axis (Z)
                    float x1 = lx * r_cos - ly * r_sin;
                    float y1 = lx * r_sin + ly * r_cos;
                    float z1 = lz;

                    // 2. Pitch around chassis lateral axis (X)
                    float x2 = x1;
                    float y2 = y1 * p_cos - z1 * p_sin;
                    float z2 = y1 * p_sin + z1 * p_cos;

                    // 3. Yaw around vertical axis (Y)
                    float rx =  x2 * y_cos + z2 * y_sin;
                    float ry =  y2;
                    float rz = -x2 * y_sin + z2 * y_cos;

                    p_world[v].x = (float)veh->x + rx;
                    p_world[v].y = (float)veh->y + ry;
                    p_world[v].z = (float)veh->z + rz;
                }

                uint32_t page_offset = poly->extra;
                float u0 = (float)poly->tu0 / 65536.0f;
                float v0 = (float)poly->tv0 / 65536.0f;
                float u1 = (float)poly->tu1 / 65536.0f;
                float v1 = (float)poly->tv1 / 65536.0f;
                float u2 = (float)poly->tu2 / 65536.0f;
                float v2 = (float)poly->tv2 / 65536.0f;

                uint8_t base_col = s_car_fallback_colors[car_idx];
                Vertex3D vert0 = { p_world[0], { u0, v0 }, 1.0f, page_offset, false, false, base_col };
                Vertex3D vert1 = { p_world[1], { u1, v1 }, 1.0f, page_offset, false, false, base_col };
                Vertex3D vert2 = { p_world[2], { u2, v2 }, 1.0f, page_offset, false, false, base_col };

                Renderer_DrawTriangle(r, cam, &vert0, &vert1, &vert2);
            }
            r->active_texture = prev_tex;
        }
    } else {
        // Fallback procedural chassis block if CARS.MSH is absent
        float hw = 16.0f, hl = 28.0f, hh = 10.0f;
        Vec3 corners[8] = {
            { -hw, -hh,  hl }, {  hw, -hh,  hl }, {  hw,  hh,  hl }, { -hw,  hh,  hl },
            { -hw, -hh, -hl }, {  hw, -hh, -hl }, {  hw,  hh, -hl }, { -hw,  hh, -hl }
        };
        Vec3 xformed[8];
        for (int i = 0; i < 8; i++) {
            float vx = corners[i].x, vy = corners[i].y, vz = corners[i].z;
            float x1 = vx * r_cos - vy * r_sin, y1 = vx * r_sin + vy * r_cos, z1 = vz;
            float x2 = x1, y2 = y1 * p_cos - z1 * p_sin, z2 = y1 * p_sin + z1 * p_cos;
            float rx =  x2 * y_cos + z2 * y_sin;
            float ry =  y2;
            float rz = -x2 * y_sin + z2 * y_cos;
            xformed[i].x = (float)veh->x + rx;
            xformed[i].y = (float)veh->y + ry;
            xformed[i].z = (float)veh->z + rz;
        }

        uint8_t col = s_car_fallback_colors[car_idx];
        // 12 lines for chassis box outline
        Renderer_DrawLine3D(r, cam, xformed[0], xformed[1], col);
        Renderer_DrawLine3D(r, cam, xformed[1], xformed[2], col);
        Renderer_DrawLine3D(r, cam, xformed[2], xformed[3], col);
        Renderer_DrawLine3D(r, cam, xformed[3], xformed[0], col);

        Renderer_DrawLine3D(r, cam, xformed[4], xformed[5], col);
        Renderer_DrawLine3D(r, cam, xformed[5], xformed[6], col);
        Renderer_DrawLine3D(r, cam, xformed[6], xformed[7], col);
        Renderer_DrawLine3D(r, cam, xformed[7], xformed[4], col);

        Renderer_DrawLine3D(r, cam, xformed[0], xformed[4], col);
        Renderer_DrawLine3D(r, cam, xformed[1], xformed[5], col);
        Renderer_DrawLine3D(r, cam, xformed[2], xformed[6], col);
        Renderer_DrawLine3D(r, cam, xformed[3], xformed[7], col);
    }

    // Render 4 wheel markers at wheel contact positions
    for (int w = 0; w < 4; w++) {
        Vec3 wp = { (float)veh->wheels[w].world_x, (float)veh->wheels[w].world_y, (float)veh->wheels[w].world_z };
        Vec3 wt = { wp.x, wp.y + 12.0f, wp.z };
        Renderer_DrawLine3D(r, cam, wp, wt, 255); // White contact strut
    }
}


