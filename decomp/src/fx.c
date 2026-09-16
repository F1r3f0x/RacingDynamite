#include "main.h"

// External references
extern int Lisa_DeleteDynamicObject(void *obj);
extern int Lisa_MoveDynamicObject(void *obj);
extern void FatalError(const char *msg);
extern void *FUN_00445f20(int a, int b, int c);
extern void FUN_00456c40(int ptr);
extern void FUN_00447150(void *obj);
extern void Lisa_InitDynamicObjectNode(int a, int b, void *c, void *d, int e, int f, int g, int h, int i);

extern int g_AudioEventParam1; // DAT_005532d0
extern int g_AudioEventParam2;
extern int g_AudioEventParam3;
extern int g_AudioEventParam4;
extern int g_AudioEventParam5;
extern int g_AudioEventParam6;
extern int g_AudioEventParam7;
extern void FUN_004579b0(int *ptr);
extern void FUN_00456b70(int a, int b, char *c, void *d, int e, int f, int g);

extern long long FUN_0044f070(int a, unsigned int b);
extern int *DAT_0063c5bc;
extern int *DAT_0063c600;
extern unsigned long long FUN_00438030(int a, unsigned int b);

extern int DAT_004949bc;
extern double DAT_005532c0;
extern double DAT_005532c4;
extern double DAT_005532b8;
extern double DAT_005532bc;
extern double DAT_00553288;
extern double DAT_0055328c;
extern double DAT_006192f8;
extern double DAT_006192fc;
extern double DAT_00619318;
extern double DAT_0061931c;
extern int DAT_00553000;
extern int DAT_00525e58;

extern double DAT_0063c5f0[];
extern double DAT_0063c64c[];
extern int DAT_005532b0;
extern int DAT_0055306c;
extern int DAT_0054f980;
extern int DAT_0047a440;
extern int DAT_004986e0[];

/**
 * @original Audio_PlaySampleVol (IGN_WIN.EXE @ 0x0043e710, fx.c)
 * @fidelity ADAPTED
 */
void Audio_PlaySampleVol(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7) {
    g_AudioEventParam1 = param_1;
    g_AudioEventParam2 = param_2;
    g_AudioEventParam3 = param_3;
    g_AudioEventParam4 = param_4;
    g_AudioEventParam5 = (int)Math_RandomFloat();
    g_AudioEventParam6 = param_6;
    g_AudioEventParam7 = param_7;
    FUN_004579b0(&g_AudioEventParam1);
}

/**
 * @original FX_UpdateExplosionNode (IGN_WIN.EXE @ 0x00430300, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateExplosionNode(SceneryParticle *p, int instance_idx) {
    int count, timer, pos_x, pos_y, pos_z, i;
    DynamicObject *obj;
    SceneryParticle sp;

    pos_y = p->pos_y;
    pos_z = p->pos_z;
    pos_x = p->pos_x;
    timer = p->vel_y;
    count = p->vel_z;

    if (timer == 0) {
        Lisa_InitDynamicObjectNode(g_pDynamicObjects[0].field_0, count, &g_pDynamicObjects[instance_idx], (void *)0x00493cf0, 0, 0x12d, 0, 100, 0);

        for (i = 8; i != 0; i--) {
            sp.type = 2;
            sp.pos_x = pos_x;
            sp.pos_y = pos_y;
            sp.pos_z = pos_z;
            sp.vel_x = (int)Math_RandomFloat();
            sp.vel_y = (int)Math_RandomFloat();
            sp.vel_z = (int)Math_RandomFloat();
            sp.drag = 0x3e1;
            sp.gravity = -153;
            sp.field_24 = (int)Math_RandomFloat();
            sp.field_28 = -614;
            sp.field_2c = 1;
            sp.rot_x = 0;
            sp.rot_y = 0xf;
            FX_SpawnParticle(&sp);
        }

        for (i = 4; i != 0; i--) {
            sp.vel_x = 0;
            sp.type = 1;
            sp.pos_x = pos_x;
            sp.pos_y = pos_y;
            sp.pos_z = pos_z;
            sp.vel_y = (int)Math_RandomFloat();
            sp.vel_z = 0;
            sp.gravity = 0;
            sp.field_28 = 0;
            sp.rot_z = 0;
            sp.field_3c = 0;
            sp.field_40 = 0;
            sp.field_44 = 0;
            sp.field_48 = 0;
            sp.field_4c = 0;
            sp.life = 0;
            sp.drag = 0x400;
            sp.field_24 = 5;
            sp.field_2c = 0x99;
            sp.rot_x = 0x19000;
            sp.rot_y = 0x1000;
            sp.rot_z = 0x00498118;
            sp.field_3c = 0x32;
            FX_SpawnParticle(&sp);
        }
        g_EliminationFlag = 1;
    }

    obj = &g_pDynamicObjects[instance_idx];
    obj->pos_x = pos_x / 1024;
    obj->pos_y = pos_y / 1024;
    obj->pos_z = pos_z / 1024;
    obj->rot_x = (int)Math_RandomFloat();
    obj->rot_y = (int)Math_RandomFloat();
    obj->rot_z = 0;

    if (obj->field_1c == 0) {
        Lisa_MoveDynamicObject(obj);
    } else {
        if (Lisa_DeleteDynamicObject(obj) != 0) {
        }
    }

    if (count <= timer + 1) {
        FUN_00447150(obj);
        p->type = 0;
        g_EliminationFlag = 0;
    }
    p->vel_y = timer + 1;
}

/**
 * @original Audio_LoadAssets (IGN_WIN.EXE @ 0x0043e6a0, fx.c)
 * @fidelity ADAPTED
 */
long long Audio_LoadAssets(int param_1, unsigned int param_2) {
    long long lVar1;
    lVar1 = FUN_0044f070(param_1, param_2);
    return (((long long)(lVar1 >> 32)) << 32) | 1;
}

/**
 * @original Audio_StopSample (IGN_WIN.EXE @ 0x0043e6d0, fx.c)
 * @fidelity ADAPTED
 */
unsigned long long Audio_StopSample(void) {
    int *puVar1;
    unsigned long long uVar2;
    DAT_0063c5bc[0] = (int)DAT_0063c600;
    puVar1 = DAT_0063c5bc;
    DAT_0063c5bc[1] = 0;
    DAT_0063c600[0] = 0;
    DAT_0063c600[1] = 0;
    DAT_0063c600[2] = 0;
    uVar2 = FUN_00438030(0, (unsigned int)puVar1);
    return uVar2;
}

/**
 * @original FX_SpawnWeather (IGN_WIN.EXE @ 0x0043ec70, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWeather(void) {
    char local_20[32];
    if (DAT_004949bc != 0) {
        sprintf(local_20, "X: %f", DAT_005532c0, DAT_005532c4);
        FUN_00456b70(0, 0, local_20, (void*)0x563db0, DAT_00553000, DAT_00525e58, 20);
        sprintf(local_20, "Y: %f", DAT_005532b8, DAT_005532bc);
        FUN_00456b70(0, 10, local_20, (void*)0x563db0, DAT_00553000, DAT_00525e58, 20);
        sprintf(local_20, "Z: %f", DAT_00553288, DAT_0055328c);
        FUN_00456b70(0, 20, local_20, (void*)0x563db0, DAT_00553000, DAT_00525e58, 20);
        sprintf(local_20, "X VIN: %f", DAT_006192f8, DAT_006192fc);
        FUN_00456b70(0, 30, local_20, (void*)0x563db0, DAT_00553000, DAT_00525e58, 20);
        sprintf(local_20, "Y VIN: %f", DAT_00619318, DAT_0061931c);
        FUN_00456b70(0, 40, local_20, (void*)0x563db0, DAT_00553000, DAT_00525e58, 20);
    }
}

/**
 * @original FX_SpawnParticle (IGN_WIN.EXE @ 0x00434380, fx.c)
 * @fidelity ADAPTED
 */
unsigned int FX_SpawnParticle(SceneryParticle *p) {
    int should_spawn = 1;
    int type = p->type;
    int pos_x, pos_z;
    int diff_x, diff_z;
    SceneryParticle *slot;
    unsigned int index = 0;
    int min_prio;
    unsigned int best_idx;
    unsigned int i;

    if (type == 1 || type == 2 || type == 8) {
        should_spawn = 0;
        pos_x = p->pos_x / 1024;
        diff_x = pos_x - (int)DAT_0063c5f0[0];
        if (abs(diff_x) < 3500) {
            pos_z = p->pos_z / 1024;
            diff_z = pos_z - (int)DAT_0063c5f0[2];
            if (abs(diff_z) <= 3500) {
                should_spawn = 1;
            } else if (DAT_0047a440 <= DAT_0054f980 || DAT_005532b0 == 1) {
                should_spawn = 1;
            }
        } else if (DAT_0047a440 <= DAT_0054f980 || DAT_005532b0 == 1) {
            should_spawn = 1;
        }

        if (DAT_0055306c == 1) {
            diff_x = pos_x - (int)DAT_0063c64c[0];
            if (abs(diff_x) < 2500) {
                pos_z = p->pos_z / 1024;
                diff_z = pos_z - (int)DAT_0063c64c[2];
                if (abs(diff_z) <= 2500) {
                    should_spawn = 1;
                } else if (DAT_0054f980 >= DAT_0047a440) {
                    should_spawn = 1;
                }
            } else if (DAT_0054f980 >= DAT_0047a440) {
                should_spawn = 1;
            }
        }
    }

    if (!should_spawn) return 0xffffffff;

    slot = (SceneryParticle *)0x5db040;
    index = 0;
    while (slot < (SceneryParticle *)0x5dfe60) {
        if (slot->type == 0) break;
        slot++;
        index++;
    }

    if (index > 199) {
        min_prio = 100;
        best_idx = 0;
        slot = (SceneryParticle *)0x5db040;
        for (i = 0; i < 200; i++) {
            if (DAT_004986e0[slot->type] < min_prio) {
                best_idx = i;
                min_prio = DAT_004986e0[slot->type];
            }
            slot++;
        }
        if (min_prio < DAT_004986e0[p->type]) {
            index = best_idx;
        }
        if (index > 199) return 0xffffffff;
    }

    slot = (SceneryParticle *)(0x5db040 + index * 0x64);
    *slot = *p;
    return index;
}
