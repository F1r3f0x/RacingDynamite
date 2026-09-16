#include <math.h>
#define fsin sin
#define fcos cos
#define __ftol() (int)Math_RandomFloat()
#define CONCAT44(high, low) ((((long long)(high)) << 32) | (low))
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

extern int _DAT_0047a578;
extern int DAT_0047a5f8;
extern int DAT_00563c48;
extern int DAT_0054f968;
extern int DAT_00498544;
extern int DAT_00498548;
extern int DAT_0047a578;
extern int DAT_0047a5f0;
extern int DAT_0049874c;
extern int DAT_0047a5e8;
extern int DAT_00563d20;
extern int _DAT_0047a588;
extern int _DAT_0047a5c8;
extern int DAT_0047a5a8;
extern int _DAT_0047a598;
extern int DAT_00527f28;
extern int DAT_00601664;
extern int DAT_005530a0;
extern int _DAT_00601664;
extern int _DAT_00619318;
extern int DAT_0047a5b0;
extern int DAT_0054f96c;
extern int DAT_00494990;
extern int DAT_0047a5e0;
extern int DAT_00498740;
extern int DAT_005532d0;
extern int _DAT_0054f980;
extern int DAT_0049853c;
extern int DAT_0047a588;
extern int DAT_00552ffc;
extern int _DAT_0047a5b8;
extern int _DAT_0054f9e0;
extern int DAT_0047a5d0;
extern int DAT_0054f964;
extern int _DAT_0047a5c0;
extern int DAT_00552e40;
extern int _DAT_0047a5b0;
extern int DAT_004949a4;
extern int DAT_005530a8;
extern int _DAT_00525e2c;
extern int _DAT_006192f8;
extern int DAT_00563c10;
extern int _DAT_0047a580;
extern int DAT_0054f9e0;
extern int DAT_00552fc4;
extern int DAT_00497ed8;
extern int DAT_00553098;
extern int DAT_0047a5c8;
extern int DAT_0055309c;
extern int DAT_00563c60;
extern int DAT_00498730;
extern int _DAT_0047a5f0;
extern int DAT_00563c40;
extern int _DAT_00563c48;
extern int DAT_0047a5c0;
extern int DAT_0047a580;
extern int _DAT_0054f9dc;
extern int DAT_00552e48;
extern int DAT_005daffc;
extern int DAT_00563db0;
extern int _DAT_00498738;
extern int _DAT_0047a5a8;
extern int _DAT_0047a5e8;
extern int DAT_00563c5c;
extern int _DAT_00563c40;
extern int _DAT_0047a5f8;
extern int _DAT_00498730;
extern int DAT_005daff4;
extern int _DAT_005285d8;
extern int DAT_00563c14;
extern int DAT_0054f934;
extern int DAT_006192f0;
extern int _DAT_0049873c;
extern int DAT_0054f960;
extern int DAT_00527f40;
extern int DAT_00553788;
extern int _DAT_00552e48;
extern int DAT_0047a5a0;
extern int DAT_00498540;
extern int DAT_0054f9d8;
extern int _DAT_0047a5d0;
extern int DAT_00552f4c;
extern int DAT_00563c50;
extern int DAT_00498538;
extern int DAT_00563c58;
extern int _DAT_0054f9d8;
extern int _DAT_0047a5d8;
extern int _DAT_0047a5a0;
extern int DAT_00552f60;
extern int _DAT_0047a5e0;
extern int DAT_0054f9dc;
extern int DAT_00552f58;
extern int DAT_00563bec;
extern int DAT_005285d8;
extern int DAT_00498744;
extern int _DAT_00498734;
extern int DAT_0047a5b8;
extern int DAT_00498734;
extern int DAT_00498748;
extern int DAT_00563be4;
extern int DAT_0054f92c;
extern int DAT_00527f44;
extern int DAT_00525e2c;
extern int DAT_00563d88;
extern int DAT_0049873c;
extern int DAT_0063c5d8;
extern int DAT_00498738;
extern int DAT_0047a598;
extern int _DAT_00563c50;
extern int DAT_0047a5d8;
extern int DAT_00498480;
extern int DAT_005db040;
extern int DAT_00563d78;
extern int DAT_00563d64;
extern int DAT_0047a430;
extern int DAT_00527f6c;
extern int DAT_0054f954;
extern int DAT_00552f78;
extern int DAT_005dfe60;
extern int DAT_00527f24;
extern int DAT_00563cf0;
extern int _DAT_0047a430;
extern int _DAT_00563d64;
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

/**
 * @original Race_RenderViewport (IGN_WIN.EXE @ 0x00436990, fx.c)
 * @fidelity ADAPTED
 */
void Race_RenderViewport(double param_1)
{
  double *pdVar1;
  char uVar2;
  int uVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  unsigned int uVar7;
  int iVar8;
  int *piVar9;
  double *pdVar10;
  int extraout_ECX;
  int extraout_ECX_00;
  unsigned int uVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int uVar15;
  unsigned int extraout_EDX;
  unsigned int extraout_EDX_00;
  char *puVar16;
  int *puVar17;
  double fVar18;
  double fVar19;
  long long lVar20;
  long long local_7c;
  long long local_74;
  long long local_6c;
  long long local_64;
  long long local_5c;
  long long local_54;
  long long local_4c;
  long long local_44;
  long long local_3c;
  int local_34;
  int local_30;
  long long local_28;
  double local_20;
  double local_18;
  double local_10;
  double local_8;
  if (DAT_0054f92c == 1) {
    fVar18 = (double)_DAT_00525e2c;
    _DAT_00525e2c = (float)(fVar18 + (double)_DAT_0047a578);
    fVar18 = (double)fsin(fVar18 + (double)_DAT_0047a578);
    DAT_0063c5f0[6] = (double)fVar18;
  }
  if ((DAT_00494990 == 1) && (uVar11 = DAT_00553000 * DAT_00563c10, 0 < (int)uVar11)) {
    puVar17 = &DAT_00563db0;
    for (uVar7 = uVar11 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *puVar17 = 0;
      puVar17 = puVar17 + 1;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *(char *)puVar17 = 0;
      puVar17 = (int *)((int)puVar17 + 1);
    }
  }
  if (DAT_00552fc4 == 5) {
    iVar14 = *(int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
    if ((0x1b < iVar14) && (iVar14 < 0x46)) {
      *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else if (DAT_00552fc4 == 3) {
    iVar14 = *(int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
    if ((0 < iVar14) && (iVar14 < 0x1e)) {
      *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x19;
      goto LAB_00436c9c;
    }
    if ((0xbb < iVar14) && (iVar14 < 0xc5)) {
      *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else {
    if (DAT_00552fc4 == 2) {
      iVar8 = *(int *)(DAT_00552ffc + 0x30 + DAT_004949a4 * 0x4c);
      iVar14 = DAT_00552ffc + DAT_004949a4 * 0x4c;
      iVar4 = (DAT_00563c10 / 200) * 0x19;
      if (iVar8 < iVar4 + iVar8) {
        iVar13 = DAT_00553000 * iVar8;
        do {
          iVar12 = *(int *)(iVar14 + 0x2c);
          if (iVar12 < *(int *)(iVar14 + 0x34)) {
            do {
              *(char *)((int)&DAT_00563db0 + iVar12 + iVar13) = 0x72;
              iVar12 = iVar12 + 1;
            } while (iVar12 < *(int *)(iVar14 + 0x34));
          }
          iVar8 = iVar8 + 1;
          iVar13 = iVar13 + DAT_00553000;
        } while (iVar8 < *(int *)(iVar14 + 0x30) + iVar4);
      }
      if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
        iVar14 = *(int *)(DAT_00552ffc + 0x7c);
        piVar6 = (int *)(DAT_00552ffc + 0x7c);
        if (iVar14 < iVar14 + iVar4) {
          piVar9 = (int *)(DAT_00552ffc + 0x78);
          piVar5 = (int *)(DAT_00552ffc + 0x80);
          iVar8 = DAT_00553000 * iVar14;
          do {
            iVar13 = *piVar9;
            if (iVar13 < *piVar5) {
              do {
                *(char *)((int)&DAT_00563db0 + iVar13 + iVar8) = 0x72;
                iVar13 = iVar13 + 1;
              } while (iVar13 < *piVar5);
            }
            iVar14 = iVar14 + 1;
            iVar8 = iVar8 + DAT_00553000;
          } while (iVar14 < *piVar6 + iVar4);
        }
      }
      goto LAB_00436c9c;
    }
    if (DAT_00552fc4 == 1) {
      iVar14 = *(int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
      if ((-0x7c < iVar14) && (iVar14 < -0x71)) {
        *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
    }
    else if (DAT_00552fc4 == 0) {
      iVar14 = *(int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
      if ((0x24 < iVar14) && (iVar14 < 0x2e)) {
        *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
      if ((0x7c < iVar14) && (iVar14 < 0x8c)) {
LAB_00436c7c:
        *(int *)((int)DAT_0063c5f0 + 0xa4) = 0x23;
        goto LAB_00436c9c;
      }
    }
    else {
      if (DAT_00552fc4 != 4) goto LAB_00436c9c;
      iVar14 = *(int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
      if ((6 < iVar14) && (iVar14 < 0xe)) goto LAB_00436c7c;
    }
  }
  *(int *)((int)DAT_0063c5f0 + 0xa4) = 0;
LAB_00436c9c:
  if (DAT_0054f934 == 1) {
    iVar8 = *(int *)(DAT_00552ffc + 0x30 + DAT_004949a4 * 0x4c);
    iVar14 = DAT_00552ffc + DAT_004949a4 * 0x4c;
    if (iVar8 < *(int *)(DAT_00552ffc + 0x38 + DAT_004949a4 * 0x4c)) {
      iVar4 = DAT_00553000 * iVar8;
      do {
        iVar13 = *(int *)(iVar14 + 0x2c);
        if (iVar13 < *(int *)(iVar14 + 0x34)) {
          uVar2 = (&DAT_00497ed8)[DAT_00552fc4 * 4];
          do {
            *(char *)((int)&DAT_00563db0 + iVar13 + iVar4) = uVar2;
            iVar13 = iVar13 + 1;
          } while (iVar13 < *(int *)(iVar14 + 0x34));
        }
        iVar8 = iVar8 + 1;
        iVar4 = iVar4 + DAT_00553000;
      } while (iVar8 < *(int *)(iVar14 + 0x38));
    }
    if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
      iVar14 = *(int *)(DAT_00552ffc + 0x7c);
      piVar6 = (int *)(DAT_00552ffc + 0x84);
      if (iVar14 < *(int *)(DAT_00552ffc + 0x84)) {
        piVar5 = (int *)(DAT_00552ffc + 0x78);
        iVar8 = DAT_00553000 * iVar14;
        piVar9 = (int *)(DAT_00552ffc + 0x80);
        do {
          iVar4 = *piVar5;
          if (iVar4 < *piVar9) {
            uVar2 = (&DAT_00497ed8)[DAT_00552fc4 * 4];
            do {
              *(char *)((int)&DAT_00563db0 + iVar4 + iVar8) = uVar2;
              iVar4 = iVar4 + 1;
            } while (iVar4 < *piVar9);
          }
          iVar14 = iVar14 + 1;
          iVar8 = iVar8 + DAT_00553000;
        } while (iVar14 < *piVar6);
      }
    }
  }
  pdVar10 = (double *)(DAT_005daffc + DAT_004949a4 * 0x484c);
  pdVar1 = (double *)(DAT_005daff4 + DAT_004949a4 * 200);
  if (*(int *)(DAT_005daffc + 0x5a4 + DAT_004949a4 * 0x484c) == 0) {
    local_5c = pdVar1[7] + *pdVar1;
    local_6c = pdVar1[0xf];
    local_54 = pdVar1[9] + pdVar1[8] + pdVar1[6];
    local_64 = pdVar1[0x11];
    uVar3 = *(int *)((int)pdVar1 + 0x6c);
    uVar15 = *(int *)(pdVar1 + 0xd);
  }
  else {
    local_5c = pdVar1[7] + *pdVar1;
    local_6c = pdVar1[0x12];
    local_54 = pdVar1[9] + pdVar1[8] + pdVar1[6];
    local_64 = pdVar1[0x14];
    uVar3 = *(int *)((int)pdVar1 + 0x74);
    uVar15 = *(int *)(pdVar1 + 0xe);
  }
  local_7c = (double)CONCAT44(uVar3,uVar15);
  if (DAT_0054f934 == 1) {
    fVar18 = (double)fcos((double)pdVar10[0x1f]);
    local_5c = pdVar10[1] + _DAT_0047a588;
    fVar19 = (double)fsin((double)pdVar10[0x1f]);
    local_54 = 150.0;
    local_6c = (double)(fVar18 * (double)_DAT_0047a580 + (double)*pdVar10);
    local_64 = (double)(fVar19 * (double)_DAT_0047a580 + (double)pdVar10[2]);
    for (local_7c = pdVar10[0x1f] * _DAT_0047a598 + _DAT_0047a5a0; local_7c < _DAT_0047a5a8;
        local_7c = local_7c + _DAT_0047a5b0) {
    }
    for (; _DAT_0047a5b8 < local_7c; local_7c = local_7c - _DAT_0047a5b0) {
    }
  }
  if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
    if (*(int *)(DAT_005daffc + 0x4df0) == 0) {
      local_28 = *(double *)(DAT_005daff4 + 200) + *(double *)(DAT_005daff4 + 0x100);
      local_4c = *(double *)(DAT_005daff4 + 0x140);
      local_3c = *(double *)(DAT_005daff4 + 0x108) + *(double *)(DAT_005daff4 + 0xf8) +
                 *(double *)(DAT_005daff4 + 0x110);
      local_44 = *(double *)(DAT_005daff4 + 0x150);
      local_74 = *(double *)(DAT_005daff4 + 0x130);
    }
    else {
      local_28 = *(double *)(DAT_005daff4 + 200) + *(double *)(DAT_005daff4 + 0x100);
      local_4c = *(double *)(DAT_005daff4 + 0x158);
      local_44 = *(double *)(DAT_005daff4 + 0x168);
      local_74 = *(double *)(DAT_005daff4 + 0x138);
      local_3c = *(double *)(DAT_005daff4 + 0x108) + *(double *)(DAT_005daff4 + 0xf8) +
                 *(double *)(DAT_005daff4 + 0x110);
    }
    if (DAT_0054f934 == 1) {
      pdVar10 = (double *)(DAT_005daffc + 0x4944);
      fVar18 = (double)fcos((double)*pdVar10);
      local_28 = *(double *)(DAT_005daffc + 0x4854) + _DAT_0047a588;
      fVar19 = (double)fsin((double)*pdVar10);
      local_3c = 150.0;
      local_4c = (double)(fVar18 * (double)_DAT_0047a580 +
                         (double)*(double *)(DAT_005daffc + 0x484c));
      local_44 = (double)(fVar19 * (double)_DAT_0047a580 +
                         (double)*(double *)(DAT_005daffc + 0x485c));
      for (local_74 = *pdVar10 * _DAT_0047a598 + _DAT_0047a5a0; local_74 < _DAT_0047a5a8;
          local_74 = local_74 + _DAT_0047a5b0) {
      }
      for (; _DAT_0047a5b8 < local_74; local_74 = local_74 - _DAT_0047a5b0) {
      }
    }
  }
  if ((-1 < DAT_00527f40) && (DAT_00527f40 < 0x78)) {
    lVar20 = __ftol();
    DAT_00527f40 = (int)lVar20;
  }
  if ((-1 < DAT_00527f44) && (DAT_00527f44 < 0x78)) {
    lVar20 = __ftol();
    DAT_00527f44 = (int)lVar20;
  }
  if (((0.0 <= _DAT_0054f980) && (_DAT_0054f980 < _DAT_0047a5c0)) || (DAT_005530a8 == 7)) {
    _DAT_0054f980 = _DAT_0054f980 + param_1;
    if ((*(int *)(DAT_005daffc + 0x528) == 1) &&
       (puVar17 = (int *)(DAT_005daffc + 0x5d4), *(int *)(DAT_005daffc + 0x5d4) < 0x19)) {
      lVar20 = __ftol();
      *puVar17 = (int)lVar20;
    }
    if ((*(int *)(DAT_005daffc + 0x4d74) == 1) &&
       (puVar17 = (int *)(DAT_005daffc + 20000), *(int *)(DAT_005daffc + 20000) < 0x19)) {
      lVar20 = __ftol();
      *puVar17 = (int)lVar20;
    }
  }
  else if ((_DAT_0047a5c0 <= _DAT_0054f980) &&
          (((DAT_0055306c == 0 && (*(int *)(DAT_005daffc + 0x528) == 1)) ||
           ((DAT_0055306c == 1 &&
            ((*(int *)(DAT_005daffc + 0x528) == 1 && (*(int *)(DAT_005daffc + 0x4d74) == 1)))))))) {
    if (DAT_00563d20 == 3) {
      _DAT_00601664 = FUN_00438050();
      if (DAT_006192f0 - _DAT_00601664 == -1) {
        DAT_00563d20 = 0;
        DAT_004949a4 = 0;
        iVar14 = DAT_00552fc4 * 0x3c;
        local_6c = (double)*(float *)(&DAT_00498538 + DAT_00552fc4 * 0x3c);
        fVar18 = (double)fsin((double)_DAT_005285d8 * (double)_DAT_0047a5c8);
        local_64 = (double)*(float *)(&DAT_00498540 + iVar14);
        local_5c = (double)(fVar18 * (double)_DAT_0047a5d0 +
                           (double)*(float *)(&DAT_0049853c + iVar14));
        local_54 = (double)(*(float *)(&DAT_00498544 + iVar14) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        local_7c = (double)(*(float *)(&DAT_00498548 + iVar14) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        local_20 = local_5c;
        local_18 = local_64;
        local_10 = local_54;
        local_8 = local_7c;
        if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
          local_74 = local_7c;
          local_4c = local_6c;
          local_44 = local_64;
          local_3c = local_54;
          local_28 = local_5c;
        }
      }
      else {
        local_5c = *(double *)(DAT_005daff4 + 0x38 + DAT_004949a4 * 200) +
                   *(double *)(DAT_005daff4 + DAT_004949a4 * 200);
        iVar14 = DAT_005daff4 + DAT_004949a4 * 200;
        local_6c = (double)CONCAT44(*(int *)(DAT_005daff4 + 0x94 + DAT_004949a4 * 200),
                                    *(int *)(iVar14 + 0x90));
        local_64 = (double)CONCAT44(*(int *)(DAT_005daff4 + 0xa4 + DAT_004949a4 * 200),
                                    *(int *)(iVar14 + 0xa0));
        local_54 = *(double *)(iVar14 + 0x48) + *(double *)(iVar14 + 0x40) +
                   *(double *)(iVar14 + 0x30);
        local_7c = *(double *)(iVar14 + 0x70);
      }
    }
    else {
      iVar14 = DAT_00552fc4 * 3 + DAT_00563d20;
      local_6c = (double)*(float *)(&DAT_00498538 + iVar14 * 0x14);
      fVar18 = (double)fsin((double)_DAT_005285d8 * (double)_DAT_0047a5c8);
      iVar14 = iVar14 * 0x14;
      local_64 = (double)*(float *)(&DAT_00498540 + iVar14);
      local_54 = (double)(*(float *)(&DAT_00498544 + iVar14) * _DAT_0047a5d8 * (float)_DAT_0047a5e0)
      ;
      local_5c = (double)(fVar18 * (double)_DAT_0047a5d0 +
                         (double)*(float *)(&DAT_0049853c + iVar14));
      local_7c = (double)(*(float *)(&DAT_00498548 + iVar14) * _DAT_0047a5d8 * (float)_DAT_0047a5e0)
      ;
      if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
        local_4c = (double)*(float *)(&DAT_00498538 + DAT_00552fc4 * 0x3c);
        local_44 = (double)*(float *)(&DAT_00498540 + DAT_00552fc4 * 0x3c);
        local_28 = (double)(*(float *)(&DAT_0049853c + DAT_00552fc4 * 0x3c) +
                           (float)(fVar18 * (double)_DAT_0047a5d0));
        local_3c = (double)(*(float *)(&DAT_00498544 + DAT_00552fc4 * 0x3c) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        local_74 = (double)(*(float *)(&DAT_00498548 + DAT_00552fc4 * 0x3c) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
      }
    }
  }
  if (DAT_005532b0 == 1) {
    local_6c = DAT_005532c0;
    local_54 = _DAT_006192f8 * _DAT_0047a5e8;
    local_7c = _DAT_00619318 * _DAT_0047a5e8;
    local_5c = DAT_005532b8;
    local_64 = DAT_00553288;
  }
  *(int *)(DAT_0063c5f0 + 7) = 0;
  pdVar10 = DAT_0063c5f0;
  *(int *)((int)DAT_0063c5f0 + 4) = ((int*)&local_6c)[1];
  *(int *)pdVar10 = (int)local_6c;
  pdVar10 = DAT_0063c5f0;
  *(int *)((int)DAT_0063c5f0 + 0xc) = ((int*)&local_5c)[1];
  *(int *)(pdVar10 + 1) = (int)local_5c;
  pdVar10 = DAT_0063c5f0;
  *(int *)((int)DAT_0063c5f0 + 0x14) = ((int*)&local_64)[1];
  *(int *)(pdVar10 + 2) = (int)local_64;
  pdVar10 = DAT_0063c5f0;
  *(int *)((int)DAT_0063c5f0 + 0x1c) = ((int*)&local_54)[1];
  *(int *)(pdVar10 + 3) = (int)local_54;
  pdVar10 = DAT_0063c5f0;
  *(int *)((int)DAT_0063c5f0 + 0x24) = ((int*)&local_7c)[1];
  *(int *)(pdVar10 + 4) = (int)local_7c;
  pdVar10 = DAT_0063c5f0;
  *(int *)(DAT_0063c5f0 + 5) = 0;
  *(int *)((int)pdVar10 + 0x2c) = 0;
  *(int *)((int)DAT_0063c5f0 + 0x7c) = 0;
  *(int *)(DAT_0063c5f0 + 0x10) = *(int *)(DAT_005daff4 + 0x58 + DAT_004949a4 * 200);
  *(int *)((int)DAT_0063c5f0 + 0x84) =
       *(int *)(DAT_005daff4 + 0x5c + DAT_004949a4 * 200);
  lVar20 = __ftol();
  *(int *)(DAT_0063c5f0 + 0x11) = (int)lVar20;
  if (DAT_0055306c == 0) {
    *(int *)((int)DAT_0063c5f0 + 0x9c) = DAT_00553000 / 2;
  }
  else if (DAT_0055306c == 1) {
    *(int *)((int)DAT_0063c5f0 + 0x9c) =
         ((int)(DAT_00553000 + (DAT_00553000 >> 0x1f & 3U)) >> 2) * 3;
  }
  if (DAT_00552f58 == 1) {
    *(int *)((int)DAT_0063c5f0 + 0x9c) = DAT_00553000 / 2;
  }
  if (DAT_005532b0 == 1) {
    *(int *)((int)DAT_0063c5f0 + 0x7c) = DAT_00563bec;
    if (DAT_00563c14 == 1) {
      *(int *)(DAT_0063c5f0 + 7) = 1;
      lVar20 = __ftol();
      *(int *)(DAT_0063c5f0 + 0xe) = (int)lVar20;
      lVar20 = __ftol();
      *(int *)((int)DAT_0063c5f0 + 0x74) = (int)lVar20;
      lVar20 = __ftol();
      *(int *)(DAT_0063c5f0 + 0xf) = (int)lVar20;
    }
    if (DAT_00563c14 == 2) {
      iVar14 = DAT_005daffc + DAT_00563c5c * 0x484c;
      if (((*(int *)(DAT_005daffc + 0x354 + DAT_00563c5c * 0x484c) == 0) &&
          (*(int *)(iVar14 + 0x358) == 0)) && (*(int *)(iVar14 + 0x35c) == 0)) {
        *(int *)(DAT_0063c5f0 + 7) = 1;
        *DAT_0063c5f0 = *(double *)(DAT_005daffc + DAT_00563c5c * 0x484c) + _DAT_00563c48;
        DAT_005532c4 = *(int *)((int)DAT_0063c5f0 + 4);
        DAT_005532c0 = *(int *)DAT_0063c5f0;
        DAT_0063c5f0[1] = *(double *)(DAT_005daffc + 8 + DAT_00563c5c * 0x484c) + _DAT_00563c40;
        DAT_005532bc = *(int *)((int)DAT_0063c5f0 + 0xc);
        DAT_005532b8 = *(int *)(DAT_0063c5f0 + 1);
        DAT_0063c5f0[2] = *(double *)(DAT_005daffc + 0x10 + DAT_00563c5c * 0x484c) + _DAT_00563c50;
        DAT_0055328c = *(int *)((int)DAT_0063c5f0 + 0x14);
        DAT_00553288 = *(int *)(DAT_0063c5f0 + 2);
        lVar20 = __ftol();
        *(int *)(DAT_0063c5f0 + 0xe) = (int)lVar20;
        lVar20 = __ftol();
        *(int *)((int)DAT_0063c5f0 + 0x74) = (int)lVar20;
        lVar20 = __ftol();
        *(int *)(DAT_0063c5f0 + 0xf) = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9d8 = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9dc = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9e0 = (int)lVar20;
        DAT_00553098 = *(int *)(DAT_0063c5f0 + 0xe);
        DAT_005530a0 = *(int *)(DAT_0063c5f0 + 0xe);
        DAT_0055309c = *(int *)(DAT_0063c5f0 + 0xe);
      }
      else {
        *(int *)(DAT_0063c5f0 + 7) = 1;
        *DAT_0063c5f0 = (double)_DAT_0054f9d8;
        DAT_0063c5f0[1] = (double)_DAT_0054f9d8;
        DAT_0063c5f0[2] = (double)_DAT_0054f9d8;
        *(int *)(DAT_0063c5f0 + 0xe) = DAT_00553098;
        *(int *)((int)DAT_0063c5f0 + 0x74) = DAT_005530a0;
        *(int *)(DAT_0063c5f0 + 0xf) = DAT_0055309c;
      }
    }
  }
  if ((_DAT_0047a5f0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a5f8)) {
    lVar20 = __ftol();
    uVar11 = (int)(unsigned int)lVar20 >> 0x1f;
    if (((((unsigned int)lVar20 ^ uVar11) - uVar11 & 1 ^ uVar11) == uVar11) &&
       (piVar6 = (int *)(DAT_006192f0 * 0x20 + DAT_00553788), piVar6[7] == 0)) {
      FUN_00446eb0(piVar6);
    }
  }
  _DAT_00498730 = DAT_0063c5bc;
  _DAT_00498738 = &DAT_00563db0;
  _DAT_0049873c = DAT_00563be4;
  _DAT_00498734 = &DAT_00563db0;
  DAT_00498740 = *(int *)(DAT_00552ffc + 0x2c + DAT_004949a4 * 0x4c);
  iVar14 = DAT_00552ffc + DAT_004949a4 * 0x4c;
  DAT_00498744 = *(int *)(iVar14 + 0x30);
  DAT_00498748 = *(int *)(iVar14 + 0x34) + -1;
  DAT_0049874c = *(int *)(iVar14 + 0x38) + -1;
  uVar11 = *(unsigned int *)(DAT_005daffc + 0x364 + DAT_004949a4 * 0x484c);
  if ((int)uVar11 < 0) {
    iVar14 = *(int *)(DAT_00552f60 + 0xc +
                     ((uVar11 ^ (int)uVar11 >> 0x1f) - ((int)uVar11 >> 0x1f)) * 0x18);
  }
  else {
    iVar14 = *(int *)(DAT_00552f60 + uVar11 * 0x18);
  }
  if (_DAT_0047a5c0 <= _DAT_0054f980) {
    DAT_0063c5d8 = 0;
  }
  else {
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + iVar14 * 0xc);
  }
  Lisa_RenderScene();
  FUN_00438030(extraout_ECX,extraout_EDX);
  piVar6 = (int *)(DAT_006192f0 * 0x20 + DAT_00553788);
  if (piVar6[7] == 1) {
    FUN_00447150(piVar6);
  }
  if (((*(int *)(DAT_005daffc + 0x528) == 0) || (_DAT_0054f980 <= _DAT_0047a5c0)) &&
     (DAT_005532b0 == 0)) {
    FUN_0043ef30(DAT_004949a4);
  }
  if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
    *(int *)(DAT_0063c5f0 + 7) = 0;
    pdVar10 = DAT_0063c5f0;
    *(int *)((int)DAT_0063c5f0 + 4) = ((int*)&local_4c)[1];
    *(int *)pdVar10 = (int)local_4c;
    pdVar10 = DAT_0063c5f0;
    *(int *)((int)DAT_0063c5f0 + 0xc) = ((int*)&local_28)[1];
    *(int *)(pdVar10 + 1) = (int)local_28;
    pdVar10 = DAT_0063c5f0;
    *(int *)((int)DAT_0063c5f0 + 0x14) = ((int*)&local_44)[1];
    *(int *)(pdVar10 + 2) = (int)local_44;
    pdVar10 = DAT_0063c5f0;
    *(int *)((int)DAT_0063c5f0 + 0x1c) = ((int*)&local_3c)[1];
    *(int *)(pdVar10 + 3) = (int)local_3c;
    pdVar10 = DAT_0063c5f0;
    *(int *)((int)DAT_0063c5f0 + 0x24) = ((int*)&local_74)[1];
    *(int *)(pdVar10 + 4) = (int)local_74;
    pdVar10 = DAT_0063c5f0;
    *(int *)(DAT_0063c5f0 + 5) = 0;
    *(int *)((int)pdVar10 + 0x2c) = 0;
    *(int *)((int)DAT_0063c5f0 + 0x7c) = 0;
    *(int *)(DAT_0063c5f0 + 0x10) = *(int *)(DAT_005daff4 + 0x120);
    *(int *)((int)DAT_0063c5f0 + 0x84) = *(int *)(DAT_005daff4 + 0x124);
    lVar20 = __ftol();
    *(int *)(DAT_0063c5f0 + 0x11) = (int)lVar20;
    *(int *)((int)DAT_0063c5f0 + 0x9c) = DAT_00553000 / (DAT_0055306c * 2 + 2);
    _DAT_00498730 = DAT_0063c5bc;
    _DAT_00498734 = &DAT_00563db0;
    _DAT_00498738 = &DAT_00563db0;
    _DAT_0049873c = DAT_00563be4;
    DAT_00498740 = *(int *)(DAT_00552ffc + 0x78);
    DAT_00498744 = *(int *)(DAT_00552ffc + 0x7c);
    DAT_00498748 = *(int *)(DAT_00552ffc + 0x80) + -1;
    DAT_0049874c = *(int *)(DAT_00552ffc + 0x84) + -1;
    if ((_DAT_0047a5f0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a5f8)) {
      lVar20 = __ftol();
      uVar11 = (int)(unsigned int)lVar20 >> 0x1f;
      if (((((unsigned int)lVar20 ^ uVar11) - uVar11 & 1 ^ uVar11) == uVar11) &&
         (iVar14 = DAT_006192f0 * 0x20 + DAT_00553788, *(int *)(iVar14 + 0x3c) == 0)) {
        FUN_00446eb0((int *)(iVar14 + 0x20));
      }
    }
    uVar11 = *(unsigned int *)(DAT_005daffc + 0x4bb0);
    if ((int)uVar11 < 0) {
      iVar14 = *(int *)(DAT_00552f60 + 0xc +
                       ((uVar11 ^ (int)uVar11 >> 0x1f) - ((int)uVar11 >> 0x1f)) * 0x18);
    }
    else {
      iVar14 = *(int *)(DAT_00552f60 + uVar11 * 0x18);
    }
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + iVar14 * 0xc);
    Lisa_RenderScene();
    FUN_00438030(extraout_ECX_00,extraout_EDX_00);
    iVar14 = DAT_006192f0 * 0x20 + DAT_00553788;
    if (*(int *)(iVar14 + 0x3c) == 1) {
      FUN_00447150((int *)(iVar14 + 0x20));
    }
    if ((*(int *)(DAT_005daffc + 0x4d74) == 0) || (_DAT_0054f980 <= _DAT_0047a5c0)) {
      FUN_0043ef30(1);
    }
  }
  FUN_0043ec70();
  if ((0.0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a588)) {
    __ftol();
    Lisa_RenderPanorama();
  }
  if (((((DAT_0055306c == 0) && (*(int *)(DAT_005daffc + 0x528) == 1)) ||
       ((DAT_0055306c == 1 &&
        ((*(int *)(DAT_005daffc + 0x528) == 1 && (*(int *)(DAT_005daffc + 0x4d74) == 1)))))) &&
      (_DAT_0047a5c0 <= _DAT_0054f980)) && ((DAT_00563c60 == 0 && (DAT_005532b0 == 0)))) {
    FUN_00438a60();
  }
  iVar14 = *(int *)(DAT_00552ffc + 0x38);
  iVar8 = *(int *)(DAT_00552ffc + 0x34);
  if ((DAT_00527f28 == 0) && ((DAT_0055306c == 0 || (DAT_00552f58 == 1)))) {
    local_34 = 0;
    local_30 = 0;
    FUN_00456d20(DAT_0054f960,&local_34,0);
    local_30 = 0;
    local_34 = (iVar8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    FUN_00456d20(DAT_0054f964,&local_34,0);
    local_34 = 0;
    local_30 = (iVar14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    FUN_00456d20(DAT_0054f968,&local_34,0);
    local_34 = (iVar8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    local_30 = (iVar14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    FUN_00456d20(DAT_0054f96c,&local_34,0);
  }
  if (DAT_00563c60 == 1) {
    lVar20 = __ftol();
    DAT_00552f4c = (int)lVar20;
    if (DAT_00552f4c < 1) {
      FUN_004222b0();
      DAT_00563d88 = 2;
    }
    Lisa_RenderPanorama();
  }
  if ((DAT_0055306c == 1) && (DAT_00552f58 == 0)) {
    iVar4 = DAT_00553000 / 2;
    iVar8 = DAT_00563c10;
    for (iVar14 = iVar4 + -1; DAT_00563c10 = iVar8, iVar14 < iVar4 + 1; iVar14 = iVar14 + 1) {
      if (0 < iVar8) {
        puVar16 = (char *)((int)&DAT_00563db0 + iVar14);
        do {
          iVar13 = DAT_00553000;
          *puVar16 = 0;
          puVar16 = puVar16 + iVar13;
          iVar8 = iVar8 + -1;
        } while (iVar8 != 0);
      }
      iVar8 = DAT_00563c10;
    }
  }
  return;
}
/**
 * @original FX_UpdateAllParticles (IGN_WIN.EXE @ 0x00434190, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateAllParticles(void)
{
  float fVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  piVar3 = &DAT_005db040;
  iVar5 = 0;
  do {
    if (*piVar3 != 0) {
      switch(*piVar3) {
      case 1:
        FX_UpdateTransparentSpriteObject((SceneryParticle *)piVar3,iVar5);
        break;
      case 2:
        FX_UpdateHandlePlotObject((SceneryParticle *)piVar3,iVar5);
        break;
      case 3:
        Obstacle_SimulateDynamics((SceneryParticle *)piVar3);
        break;
      case 4:
        FX_UpdateTransparentSpriteObject2((SceneryParticle *)piVar3,iVar5);
        break;
      case 5:
        FX_UpdateFlyingParticles((SceneryParticle *)piVar3,iVar5);
        break;
      case 6:
        FX_UpdateExplosionNode((SceneryParticle *)piVar3,iVar5);
        break;
      case 7:
        FUN_004307b0(piVar3);
        break;
      case 8:
        FX_UpdateSuperPlotObject((SceneryParticle *)piVar3,iVar5);
        break;
      case 9:
        FUN_00431d00(piVar3,iVar5);
        break;
      case 10:
        FUN_00432040(piVar3);
        break;
      case 0xb:
        FUN_00432bb0(piVar3,iVar5);
      }
    }
    piVar3 = piVar3 + 0x19;
    iVar5 = iVar5 + 1;
  } while (piVar3 < &DAT_005dfe60);
  _DAT_00563d64 = _DAT_00563d64 + _DAT_0047a430;
  if ((float)*(int *)(&DAT_00498480 + DAT_00552fc4 * 4) <= _DAT_00563d64) {
    FUN_00401f00();
    fVar1 = (float)*(int *)(&DAT_00498480 + DAT_00552fc4 * 4);
    if ((float)*(int *)(&DAT_00498480 + DAT_00552fc4 * 4) <= _DAT_00563d64) {
      do {
        _DAT_00563d64 = _DAT_00563d64 - fVar1;
      } while (fVar1 <= _DAT_00563d64);
    }
  }
  if (0 < DAT_0054f954) {
    FUN_00429a40();
  }
  if (0 < DAT_00563cf0) {
    FUN_0042aa00();
  }
  if (0 < DAT_00563d78) {
    FUN_0042acb0();
  }
  if (0 < DAT_00552f78) {
    FUN_0042b5d0();
  }
  if ((DAT_00527f6c == 3) && (iVar5 = 0, 0 < DAT_006192f0)) {
    iVar4 = 0;
    do {
      iVar2 = DAT_005daffc + iVar4;
      if ((((*(int *)(iVar2 + 0x39c) == DAT_00527f24) && (_DAT_00552e48 < 0.0)) &&
          (*(int *)(iVar2 + 0x354) == 0)) &&
         (((*(int *)(iVar2 + 0x358) == 0 && (*(int *)(iVar2 + 0x35c) == 0)) &&
          (*(int *)(iVar2 + 0x528) == 0)))) {
        FUN_00444890();
      }
      iVar4 = iVar4 + 0x484c;
      iVar5 = iVar5 + 1;
    } while (iVar5 < DAT_006192f0);
  }
  FUN_00441210();
  return;
}