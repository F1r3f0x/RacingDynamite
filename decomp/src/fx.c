extern char s_SIE_SIND_LETZTER__004996fc[];
extern char s_ATTENTION__VOUS_ETES_DERNIER__00499690[];
extern char s_ATTENZIONE__SEI_ULTIMO_004996e4[];
extern char s__d__d_00499728[];
extern char s_AVISO__ERES_EL_ULTIMO_004996cc[];
extern char s_WARNING__YOU_RE_LAST__00499710[];
extern char s_VARNING__DU_LIGGER_SIST__004996b0[];
extern char s__________00499730[];
extern char s_TYPE2__d_00499608[];
extern char s_SPEED___3f_004995ec[];
extern char s_FPS__d_00499658[];
extern char s_TYPE1__d_00499614[];
extern char s_SKILLNAD___3f_004995f8[];
extern char s_Error_while_initializing_lisaGM_004995c8[];
extern char s_ROLL___1f_00499640[];
extern char s_Cannot_use_this_graphics_mode__004989dc[];
extern char s_RECORDING__0049964c[];
extern char s_OLDROADNR__d_00499630[];
extern char s_SQUASHED2__d_00499620[];
extern char s_FEL_VID_LI_MOVEOBJECT_HANDLE_WHE_00499580[];
extern char s_FEL_VID_LI_MOVEOBJECT_HANDLE_SHA_00499558[];
extern char s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_00499530[];
extern char s__d_PTS_00496948[];
extern char s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4[];
extern char s_RETRY_00494dd0[];
extern char s_FEL_VID_LI_PLACEOBJECT_HANDLE_CA_00499508[];
extern char s_PLACE_OR_BETTER_TO_PROCEED__00496452[];
extern char s_SORRY__YOU_MUST_REACH_THIRD_00496420[];
extern char s_GOLD_STATUE_TO_ADVANCE__0049707b[];
extern char s_YOU_MUST_ACHIEVE_THE_00497058[];
extern char s_NOW_TRY_THE__s_LEAGUE__00496f68[];
extern char s_A_SCORE_OF__d_PTS__00496a7a[];
extern char s_AT_THIRD_PLACE_WITH_00496a57[];
extern char s_AT_SECOND_PLACE_WITH_00496a34[];
extern char s_AT_FIRST_PLACE_WITH_00496a11[];
extern char s__s_CHAMPIONSHIP_004969ee[];
extern char s_YOU_HAVE_COMPLETED_THE_004969cb[];
extern char s_CONGRATULATIONS__004969a8[];
extern char s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8[];
extern char s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70[];
extern char s_TOTAL_SCORE_004955e0[];
extern char s_TRACK_SCORE_00495528[];
extern char s_PLAY_TRACK_AGAIN___Y_N__00495698[];
extern char s_PRESS_RETURN_TO_CONTINUE_00495860[];
extern char s_WAITING_FOR_HOST_004957a8[];
extern char s_TRACK_RESULTS_00495470[];
extern char s_QUIT___Y_N__00495248[];
extern char s_RESTART___Y_N__00495300[];
extern char s_RANDOM_00494e0c[];
extern char s_DEFAULT_00494dee[];
extern char s_CD_TRACK_00494db2[];
extern char s_RESTART_00494d76[];
extern char s_CONTINUE_00494d58[];
extern char s_Error_while_changing_car_mesh_00499248[];
extern char s_FEL_VID_LI_MOVEOBJECT_ANIM_OBJ_004994ac[];
extern char s_FEL_VID_LI_MOVEOBJECT_WEATHER_0049948c[];
extern char s_FEL_VID_LI_MOVEOBJECT_TRAN_SPRIT_004993b4[];
extern char s_FEL_VID_LI_MOVEOBJECT_WATER_SPLA_00499468[];
#include <math.h>
#define fsin sin
#define fcos cos
#define __ftol() (int)Math_RandomFloat()
#define CONCAT44(high, low) ((((long long)(high)) << 32) | (low))
#include <math.h>
#define fsin sin
#define fcos cos
#define __ftol() (int)Math_RandomFloat()
#define CONCAT44(high, low) ((((long long)(high)) << 32) | (low))
#include "main.h"
#include "fx.h"

// External references
extern int Lisa_DeleteDynamicObject(void *obj);
extern int Lisa_MoveDynamicObject(void *obj);
extern void FatalError(const char *msg);
extern void Lisa_InitDynamicObjectNode(int a, int b, void *c, void *d, int e, int f, int g, int h, int i);

extern int g_AudioEventParam1; // DAT_005532d0
extern int g_AudioEventParam2;
extern int g_AudioEventParam3;
extern int g_AudioEventParam4;
extern int g_AudioEventParam5;
extern int g_AudioEventParam6;
extern int g_AudioEventParam7;
extern long long Lisa_InitRasterizerTables(int a, unsigned int b);
extern int *g_pLisaDrawCommandQueue;
extern int *g_LisaDrawCommandBuffer;
extern unsigned long long Lisa_FlushRasterizerCommands(int a, unsigned int b);
void FX_UpdateVehicleWreck(int *wreck);
void FX_UpdateDetachedWheel(int *wheel, int wheel_index);
void FX_UpdateVehicleCrashSequence(int *crash_seq);
void FX_UpdateCarDebris(int *debris, int debris_idx);
void FX_SpawnWaterSplashes(void);
void FX_SpawnTireDirtDebris(void);
void FX_SpawnLandingDustPuffs(void);
void FX_SpawnTireSkidSmoke(void);
void FX_UpdateWeatherGeometry(void);
void FX_FrameTick(void);
double Math_LookupTrigAngle(double angle);
void Camera_UpdateOverview(void);
int Race_FindFocusedVehicle(void);
void HUD_RenderPauseMenu(void);
void HUD_RenderConfirmationPrompt(void);
int HUD_RenderTrackResults(void);
void HUD_RenderPlayAgainPrompt(void);
void Car_UpdateDynamicObjects(int car_idx);
int Video_SetGraphicsMode(void);
void Video_FlipScreen(void);
void HUD_RenderTelemetryOverlay(void);
void HUD_CheckWrongWayHeading(int player_idx);
void HUD_RenderPlayerElements(int player_idx);

extern int DAT_004944e0;
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
extern int _DAT_0047a578;
extern int DAT_0047a5f8;
extern int DAT_00563c48;
extern int DAT_0054f968;
extern int DAT_00498544;
extern int DAT_00498548;
extern int DAT_0047a578;
extern int DAT_0047a5f0;
extern int g_ViewportMaxY;
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
extern int DAT_0047a5e0;
extern int g_ViewportMinX;
extern int DAT_005532d0;
extern int _DAT_0054f980;
extern int DAT_0049853c;
extern int DAT_0047a588;
extern int _DAT_0047a5b8;
extern int _DAT_0054f9e0;
extern int DAT_0047a5d0;
extern int DAT_0054f964;
extern int _DAT_0047a5c0;
extern int DAT_00552e40;
extern int _DAT_0047a5b0;
extern int _DAT_00525e2c;
extern int _DAT_006192f8;
extern int _DAT_0047a580;
extern int DAT_0054f9e0;
extern int g_CurrentTrackIndex;
extern int DAT_00497ed8;
extern int DAT_00553098;
extern int DAT_0047a5c8;
extern int DAT_0055309c;
extern int DAT_00563c60;
extern int _DAT_0047a5f0;
extern int DAT_00563c40;
extern int _DAT_00563c48;
extern int DAT_0047a5c0;
extern int DAT_0047a580;
extern int _DAT_0054f9dc;
extern int DAT_00552e48;
extern int _DAT_00498738;
extern int _DAT_0047a5a8;
extern int _DAT_0047a5e8;
extern int DAT_00563c5c;
extern int _DAT_00563c40;
extern int _DAT_0047a5f8;
extern int _DAT_00498730;
extern int _DAT_005285d8;
extern int DAT_00563c14;
extern int DAT_0054f934;
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
extern int g_pTrackRoadSequence[];
extern int _DAT_0047a5e0;
extern int DAT_0054f9dc;
extern int DAT_00552f58;
extern int DAT_00563bec;
extern int DAT_005285d8;
extern int g_ViewportMinY;
extern int _DAT_00498734;
extern int DAT_0047a5b8;
extern int g_ViewportMaxX;
extern int g_pActiveTAB;
extern int DAT_0054f92c;
extern int DAT_00527f44;
extern int DAT_00525e2c;
extern int DAT_00563d88;
extern int DAT_0063c5d8;
extern int DAT_0047a598;
extern int _DAT_00563c50;
extern int DAT_0047a5d8;
extern int DAT_00498480;
extern int g_SceneryParticles;
extern int DAT_00563d78;
extern int DAT_00563d64;
extern int DAT_0047a430;
extern int DAT_00552f78;
extern int DAT_005dfe60;
extern int DAT_00563cf0;
extern int _DAT_0047a430;
extern int _DAT_00563d64;
extern int DAT_00552f30;
extern int _DAT_00552f34;
extern int DAT_00552f34;
extern int DAT_0047a548;
extern int _DAT_0047a3f8;
extern int DAT_00563d74;
extern int _DAT_0047a548;
extern int DAT_00528624;
extern int DAT_00528808;
extern int DAT_0047a798;
extern int DAT_00639378;
extern int DAT_0047a480;
extern int DAT_005287f0;
extern int _DAT_0047a888;
extern int DAT_0047a518;
extern int _DAT_0047a768;
extern int _DAT_0047a8a0;
extern int DAT_005286f4;
extern int DAT_0052863c;
extern int DAT_0047a3e8;
extern int DAT_00552df0;
extern int DAT_00499504;
extern int _DAT_0047a458;
extern int DAT_0047a488;
extern int DAT_0063f2d8;
extern int _DAT_00553090;
extern int _DAT_0047a778;
extern int _DAT_0047a8f8;
extern int _DAT_0047a700;
extern int DAT_0047a8f8;
extern int DAT_00563d04;
extern int _DAT_00552f68;
extern int _DAT_0047a708;
extern int DAT_004994cc;
extern int DAT_0047a8a0;
extern int DAT_006192c8;
extern int DAT_00553090;
extern int DAT_0047a2c8;
extern int DAT_00528804;
extern int DAT_00528654;
extern int DAT_00528834;
extern int _DAT_0047a738;
extern int DAT_00525e64;
extern int DAT_004ba6e4;
extern int _DAT_0047a770;
extern int DAT_0063956b;
extern int DAT_0063956f;
extern int DAT_006395ab;
extern int DAT_0047a770;
extern int DAT_005531c0;
extern int DAT_004949dc;
extern int DAT_0047a3f0;
extern int _DAT_0047a550;
extern int DAT_0047a4f0;
extern int DAT_00563c38;
extern int DAT_00528818;
extern int DAT_00639340;
extern int DAT_0049847c;
extern int DAT_0047a898;
extern int _DAT_0047a2a0;
extern int DAT_006192a0;
extern int DAT_0047a6f0;
extern int DAT_006035f0;
extern int _DAT_0047a900;
extern int DAT_0060373c;
extern int DAT_0047a448;
extern int DAT_0047a2e8;
extern int DAT_0047a320;
extern int DAT_00639368;
extern int DAT_0047a8c8;
extern int DAT_00498478;
extern int DAT_00639380;
extern int DAT_0047a6f8;
extern int DAT_0052888c;
extern int _DAT_0047a8d0;
extern int _DAT_0047a8b8;
extern int _DAT_0047a758;
extern int DAT_0054f930;
extern int DAT_00525e50;
extern int DAT_005287ec;
extern int DAT_0054f908;
extern int DAT_0047a318;
extern int _DAT_0047a6e8;
extern int DAT_006192b8;
extern int DAT_00639334;
extern int DAT_00563da0;
extern int _DAT_0047a788;
extern int DAT_005287f8;
extern int DAT_00639358;
extern int DAT_00639597;
extern int DAT_00639348;
extern int DAT_006395cf;
extern int DAT_00528860;
extern int DAT_0052887c;
extern int DAT_00493c78;
extern int DAT_0047a910;
extern int DAT_00553084;
extern int DAT_00563c04;
extern int DAT_0047a560;
extern int DAT_0047a8b8;
extern int _DAT_0047a2b0;
extern int _DAT_0047a320;
extern int DAT_00528810;
extern int DAT_005db038;
extern int _DAT_0047a2d8;
extern int DAT_00552d90;
extern int DAT_00527f74;
extern int DAT_005db000;
extern int DAT_00553784;
extern int DAT_00497438;
extern int DAT_0047a330;
extern int DAT_0047a778;
extern int DAT_005285e4;
extern int DAT_00563d68;
extern int DAT_0047a458;
extern int DAT_00528840;
extern int DAT_0047a750;
extern int _DAT_0047a8b0;
extern int _DAT_0047a790;
extern int _DAT_0047a918;
extern int DAT_00563d80;
extern int DAT_0047a900;
extern int DAT_00528738;
extern int _DAT_0054f970;
extern int _DAT_0047a448;
extern int _DAT_0047a6f0;
extern int _DAT_0047a780;
extern int DAT_0047a4f8;
extern int DAT_00525e74;
extern int g_RaceFinished;
extern int DAT_00528620;
extern int _DAT_0047a710;
extern int DAT_0047a2b0;
extern int _DAT_0047a910;
extern int _DAT_0047a568;
extern int _DAT_0047a6f8;
extern int DAT_0052884c;
extern int DAT_006395a7;
extern int DAT_00553280;
extern int DAT_0054f978;
extern int DAT_00528618;
extern int _DAT_0047a2b8;
extern int DAT_0047a730;
extern int DAT_0063935c;
extern int DAT_0047a288;
extern int DAT_0047a6e8;
extern int DAT_00563ce8;
extern int _DAT_0047a340;
extern int DAT_005537d8;
extern int DAT_00525e78;
extern int DAT_0047a568;
extern int DAT_0047a4e8;
extern int DAT_0063934c;
extern int DAT_00552f68;
extern int DAT_006395d3;
extern int DAT_00528824;
extern int DAT_0047a380;
extern int DAT_0047a738;
extern int DAT_0063992b;
extern int DAT_0047a908;
extern int _DAT_0047a318;
extern int _DAT_00563d74;
extern int DAT_00563ce0;
extern int DAT_00528850;
extern int DAT_00552f20;
extern int DAT_00552fc0;
extern int DAT_0052870c[];
extern int DAT_00525e84;
extern int DAT_0047a4e0;
extern int DAT_005287bc;
extern int _DAT_0047a718;
extern int DAT_00525e6c;
extern int DAT_0047a300;
extern int DAT_0052873c;
extern int DAT_00525e60;
extern int _DAT_0047a3c0;
extern int _DAT_0047a300;
extern int DAT_0047a2e0;
extern int DAT_00639927;
extern int DAT_004ba6ec;
extern int DAT_00552f70;
extern int DAT_0047a500;
extern int _DAT_0047a630;
extern int DAT_00528830;
extern int DAT_00601674;
extern int DAT_005287f4;
extern int DAT_005286a0;
extern int DAT_0047a780;
extern int _DAT_004949e0;
extern int DAT_00603744;
extern int _DAT_0047a288;
extern int DAT_0047a658;
extern int _DAT_0047a488;
extern int DAT_0052861c;
extern int _DAT_0047a520;
extern int _DAT_0047a540;
extern int DAT_005db02c;
extern int DAT_0063959b;
extern int DAT_00552d80;
extern int _DAT_0047a760;
extern int DAT_0047a2d8;
extern int DAT_00563c34[];
extern int _DAT_0047a748;
extern int DAT_0047a310;
extern int DAT_00528878;
extern int DAT_0054f9cc[];
extern int _DAT_0047a380;
extern int DAT_00528848;
extern int DAT_00563c30[];
extern int DAT_004949b0;
extern int _DAT_0047a2e0;
extern int DAT_0052882c;
extern int DAT_0052881c;
extern int DAT_00525e80;
extern int DAT_005285c4;
extern int DAT_0047a6e0;
extern int DAT_00525e30;
extern int DAT_0047a2a8;
extern int DAT_0052886c;
extern int _DAT_0047a7e0;
extern int DAT_00528664;
extern int DAT_005285e8;
extern int _DAT_0047a510;
extern int DAT_00639354;
extern int DAT_0047a570;
extern int _DAT_0047a658;
extern int DAT_0063933c;
extern int DAT_0047a708;
extern int DAT_00603730;
extern int DAT_0047a710;
extern int _DAT_0047a3f0;
extern int DAT_00525e44;
extern int DAT_0047a760;
extern int DAT_0047a8b0;
extern int _DAT_0047a330;
extern int _DAT_0047a920;
extern int DAT_005daff0;
extern int _DAT_0047a518;
extern int DAT_00553068;
extern int DAT_00639344;
extern int DAT_00639577;
extern int DAT_0047a790;
extern int DAT_0047a748;
extern int DAT_0047a3f8;
extern int DAT_00528814;
extern int DAT_0047a718;
extern int DAT_00619310;
extern int DAT_00528800;
extern int DAT_00528870;
extern int DAT_0047a700;
extern int DAT_0047a630;
extern int DAT_00553300;
extern int DAT_004920e8;
extern int DAT_00563d28;
extern int DAT_0063936c;
extern int DAT_00528868;
extern int DAT_00553040;
extern int _DAT_0047a528;
extern int DAT_0063955b;
extern int DAT_0047a540;
extern int DAT_0052885c;
extern int _DAT_0047a6e0;
extern int DAT_00603720;
extern int DAT_0047a510;
extern int DAT_0052880c;
extern int DAT_004949a8;
extern int DAT_0047a720;
extern int DAT_00563d00;
extern int DAT_0052883c;
extern int _DAT_0047a908;
extern int DAT_00494d94;
extern int DAT_00563d38;
extern int _DAT_0047a740;
extern int _DAT_0047a2e8;
extern int DAT_00639384;
extern int DAT_005285c8;
extern int DAT_00525e70;
extern int DAT_00528658;
extern int _DAT_0047a798;
extern int _DAT_00563d80;
extern int DAT_00492a24;
extern int DAT_0052865c;
extern int DAT_00603738;
extern int DAT_00528858;
extern int DAT_005287e8;
extern int DAT_0047a298;
extern int DAT_00563da4;
extern int DAT_00528880;
extern int DAT_00553030;
extern int DAT_00553070;
extern int DAT_005287c0;
extern int DAT_0047a528;
extern int DAT_00563cf4;
extern int DAT_004ba6e0;
extern int DAT_0060372c;
extern int _DAT_0047a8c8;
extern int DAT_0047a8d0;
extern int DAT_00528888;
extern int DAT_00552e60;
extern int DAT_006395af;
extern int _DAT_0047a480;
extern int DAT_00552f10;
extern int g_pLisaDrawCommandWritePtr;
extern int _DAT_0047a750;
extern int DAT_00552f28;
extern int DAT_00528660;
extern int DAT_0047a758;
extern int DAT_00639360;
extern int _DAT_0047a3e8;
extern int DAT_00639593;
extern int _DAT_0047a570;
extern int _DAT_0047a898;
extern int DAT_00528884;
extern int DAT_00639350;
extern int DAT_00552f50;
extern int DAT_0047a2d0;
extern int _DAT_0047a290;
extern int DAT_004949e0;
extern int DAT_0047a308;
extern int DAT_0047a4d8;
extern int DAT_00639388;
extern int DAT_00563d70;
extern int DAT_004ba6e8;
extern int DAT_00603718;
extern int DAT_00552f5c;
extern int DAT_00552fdc;
extern int DAT_005285e0;
extern int DAT_00639374;
extern int DAT_0047a920;
extern int DAT_005533b0;
extern int DAT_006395b3;
extern int _DAT_0047a2a8;
extern int DAT_00497d74;
extern int DAT_0054f994;
extern int _DAT_0063992b;
extern int DAT_0063955f;
extern int _DAT_0047a308;
extern int DAT_0047a768;
extern int _DAT_0047a730;
extern int DAT_00528820;
extern int DAT_00553010;
extern int DAT_00528838;
extern int DAT_00528854;
extern int DAT_00639364;
extern int DAT_005287b8;
extern int DAT_00639338;
extern int DAT_0047a2b8;
extern int DAT_00553064;
extern int DAT_00528794;
extern int DAT_006393ac;
extern int _DAT_0047a560;
extern int DAT_0047a7e0;
extern int _DAT_0047a328;
extern int _DAT_0047a720;
extern int DAT_0047a888;
extern int _DAT_0047a310;
extern int _DAT_0047a2d0;
extern int DAT_00639497;
extern int _DAT_0047a298;
extern int DAT_0047a520;
extern int DAT_0047a328;
extern int DAT_0054f970;
extern int DAT_00497f48;
extern int DAT_0063937c;
extern int _DAT_0049847c;
extern int DAT_00563d54;
extern int DAT_0054f98c;
extern int DAT_0054f9d0;
extern int DAT_00528828;
extern int DAT_004949b4;
extern int _DAT_0047a728;
extern int _DAT_0047a2c8;
extern int DAT_0047a728;
extern int DAT_00528844;
extern int DAT_0047a8f0;
extern int _DAT_0047a8f0;
extern int DAT_0047a550;
extern int DAT_0047a918;
extern int DAT_00553ff8;
extern int DAT_00528874;
extern int _DAT_00498478;
extern int DAT_0047a290;
extern int DAT_00639370;
extern int _DAT_00563d70;
extern int DAT_00552e54;
extern int DAT_0047a2a0;
extern int DAT_00639573;
extern int DAT_0047a340;
extern int DAT_00528764;
extern int DAT_00525e7c;
extern int DAT_0054f988;
extern int DAT_0047a788;
extern int DAT_0047a740;
extern int DAT_00497ff8;
extern int DAT_00603724;
extern int DAT_00528864;
extern int DAT_00563d8c;
extern int DAT_005285f4;
extern int DAT_0047a3c0;
extern double DAT_0063c64c[];
extern int DAT_005532b0;
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
    Audio_PlaySound(&g_AudioEventParam1);
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
        Lisa_DeleteDynamicObject(obj);
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
    lVar1 = Lisa_InitRasterizerTables(param_1, param_2);
    return (((long long)(lVar1 >> 32)) << 32) | 1;
}

/**
 * @original Audio_StopSample (IGN_WIN.EXE @ 0x0043e6d0, fx.c)
 * @fidelity ADAPTED
 */
unsigned long long Audio_StopSample(void) {
    int *puVar1;
    unsigned long long uVar2;
    g_pLisaDrawCommandQueue[0] = (int)g_LisaDrawCommandBuffer;
    puVar1 = g_pLisaDrawCommandQueue;
    g_pLisaDrawCommandQueue[1] = 0;
    g_LisaDrawCommandBuffer[0] = 0;
    g_LisaDrawCommandBuffer[1] = 0;
    g_LisaDrawCommandBuffer[2] = 0;
    uVar2 = Lisa_FlushRasterizerCommands(0, (unsigned int)puVar1);
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
        Font_PrintDirect(0, 0, local_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(local_20, "Y: %f", DAT_005532b8, DAT_005532bc);
        Font_PrintDirect(0, 10, local_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(local_20, "Z: %f", DAT_00553288, DAT_0055328c);
        Font_PrintDirect(0, 20, local_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(local_20, "X VIN: %f", DAT_006192f8, DAT_006192fc);
        Font_PrintDirect(0, 30, local_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(local_20, "Y VIN: %f", DAT_00619318, DAT_0061931c);
        Font_PrintDirect(0, 40, local_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
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
        diff_x = pos_x - (int)g_pActiveCamera[0];
        if (abs(diff_x) < 3500) {
            pos_z = p->pos_z / 1024;
            diff_z = pos_z - (int)g_pActiveCamera[2];
            if (abs(diff_z) <= 3500) {
                should_spawn = 1;
            } else if (DAT_0047a440 <= DAT_0054f980 || DAT_005532b0 == 1) {
                should_spawn = 1;
            }
        } else if (DAT_0047a440 <= DAT_0054f980 || DAT_005532b0 == 1) {
            should_spawn = 1;
        }

        if (g_IsSplitScreen == 1) {
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
void Race_RenderViewport(double param_1) {
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
    g_pActiveCamera[6] = (double)fVar18;
  }
  if ((g_IsDemoMode == 1) && (uVar11 = g_ScreenWidth * g_ScreenHeight, 0 < (int)uVar11)) {
    puVar17 = &g_VirtualFramebuffer;
    for (uVar7 = uVar11 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *puVar17 = 0;
      puVar17 = puVar17 + 1;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *(char *)puVar17 = 0;
      puVar17 = (int *)((int)puVar17 + 1);
    }
  }
  if (g_CurrentTrackIndex == 5) {
    iVar14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
    if ((0x1b < iVar14) && (iVar14 < 0x46)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else if (g_CurrentTrackIndex == 3) {
    iVar14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
    if ((0 < iVar14) && (iVar14 < 0x1e)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x19;
      goto LAB_00436c9c;
    }
    if ((0xbb < iVar14) && (iVar14 < 0xc5)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else {
    if (g_CurrentTrackIndex == 2) {
      iVar8 = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
      iVar14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
      iVar4 = (g_ScreenHeight / 200) * 0x19;
      if (iVar8 < iVar4 + iVar8) {
        iVar13 = g_ScreenWidth * iVar8;
        do {
          iVar12 = *(int *)(iVar14 + 0x2c);
          if (iVar12 < *(int *)(iVar14 + 0x34)) {
            do {
              *(char *)((int)&g_VirtualFramebuffer + iVar12 + iVar13) = 0x72;
              iVar12 = iVar12 + 1;
            } while (iVar12 < *(int *)(iVar14 + 0x34));
          }
          iVar8 = iVar8 + 1;
          iVar13 = iVar13 + g_ScreenWidth;
        } while (iVar8 < *(int *)(iVar14 + 0x30) + iVar4);
      }
      if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
        iVar14 = *(int *)(g_PlayerHUDState + 0x7c);
        piVar6 = (int *)(g_PlayerHUDState + 0x7c);
        if (iVar14 < iVar14 + iVar4) {
          piVar9 = (int *)(g_PlayerHUDState + 0x78);
          piVar5 = (int *)(g_PlayerHUDState + 0x80);
          iVar8 = g_ScreenWidth * iVar14;
          do {
            iVar13 = *piVar9;
            if (iVar13 < *piVar5) {
              do {
                *(char *)((int)&g_VirtualFramebuffer + iVar13 + iVar8) = 0x72;
                iVar13 = iVar13 + 1;
              } while (iVar13 < *piVar5);
            }
            iVar14 = iVar14 + 1;
            iVar8 = iVar8 + g_ScreenWidth;
          } while (iVar14 < *piVar6 + iVar4);
        }
      }
      goto LAB_00436c9c;
    }
    if (g_CurrentTrackIndex == 1) {
      iVar14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((-0x7c < iVar14) && (iVar14 < -0x71)) {
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
    }
    else if (g_CurrentTrackIndex == 0) {
      iVar14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((0x24 < iVar14) && (iVar14 < 0x2e)) {
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
      if ((0x7c < iVar14) && (iVar14 < 0x8c)) {
LAB_00436c7c:
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x23;
        goto LAB_00436c9c;
      }
    }
    else {
      if (g_CurrentTrackIndex != 4) goto LAB_00436c9c;
      iVar14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((6 < iVar14) && (iVar14 < 0xe)) goto LAB_00436c7c;
    }
  }
  *(int *)((int)g_pActiveCamera + 0xa4) = 0;
LAB_00436c9c:
  if (DAT_0054f934 == 1) {
    iVar8 = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
    iVar14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
    if (iVar8 < *(int *)(g_PlayerHUDState + 0x38 + g_MenuCursorPos * 0x4c)) {
      iVar4 = g_ScreenWidth * iVar8;
      do {
        iVar13 = *(int *)(iVar14 + 0x2c);
        if (iVar13 < *(int *)(iVar14 + 0x34)) {
          uVar2 = ((int*)&(DAT_00497ed8))[g_CurrentTrackIndex * 4];
          do {
            *(char *)((int)&g_VirtualFramebuffer + iVar13 + iVar4) = uVar2;
            iVar13 = iVar13 + 1;
          } while (iVar13 < *(int *)(iVar14 + 0x34));
        }
        iVar8 = iVar8 + 1;
        iVar4 = iVar4 + g_ScreenWidth;
      } while (iVar8 < *(int *)(iVar14 + 0x38));
    }
    if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
      iVar14 = *(int *)(g_PlayerHUDState + 0x7c);
      piVar6 = (int *)(g_PlayerHUDState + 0x84);
      if (iVar14 < *(int *)(g_PlayerHUDState + 0x84)) {
        piVar5 = (int *)(g_PlayerHUDState + 0x78);
        iVar8 = g_ScreenWidth * iVar14;
        piVar9 = (int *)(g_PlayerHUDState + 0x80);
        do {
          iVar4 = *piVar5;
          if (iVar4 < *piVar9) {
            uVar2 = ((int*)&(DAT_00497ed8))[g_CurrentTrackIndex * 4];
            do {
              *(char *)((int)&g_VirtualFramebuffer + iVar4 + iVar8) = uVar2;
              iVar4 = iVar4 + 1;
            } while (iVar4 < *piVar9);
          }
          iVar14 = iVar14 + 1;
          iVar8 = iVar8 + g_ScreenWidth;
        } while (iVar14 < *piVar6);
      }
    }
  }
  pdVar10 = (double *)(g_Vehicles + g_MenuCursorPos * 0x484c);
  pdVar1 = (double *)(g_VehicleConfigs + g_MenuCursorPos * 200);
  if (*(int *)(g_Vehicles + 0x5a4 + g_MenuCursorPos * 0x484c) == 0) {
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
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    if (*(int *)(g_Vehicles + 0x4df0) == 0) {
      local_28 = *(double *)(g_VehicleConfigs + 200) + *(double *)(g_VehicleConfigs + 0x100);
      local_4c = *(double *)(g_VehicleConfigs + 0x140);
      local_3c = *(double *)(g_VehicleConfigs + 0x108) + *(double *)(g_VehicleConfigs + 0xf8) +
                 *(double *)(g_VehicleConfigs + 0x110);
      local_44 = *(double *)(g_VehicleConfigs + 0x150);
      local_74 = *(double *)(g_VehicleConfigs + 0x130);
    }
    else {
      local_28 = *(double *)(g_VehicleConfigs + 200) + *(double *)(g_VehicleConfigs + 0x100);
      local_4c = *(double *)(g_VehicleConfigs + 0x158);
      local_44 = *(double *)(g_VehicleConfigs + 0x168);
      local_74 = *(double *)(g_VehicleConfigs + 0x138);
      local_3c = *(double *)(g_VehicleConfigs + 0x108) + *(double *)(g_VehicleConfigs + 0xf8) +
                 *(double *)(g_VehicleConfigs + 0x110);
    }
    if (DAT_0054f934 == 1) {
      pdVar10 = (double *)(g_Vehicles + 0x4944);
      fVar18 = (double)fcos((double)*pdVar10);
      local_28 = *(double *)(g_Vehicles + 0x4854) + _DAT_0047a588;
      fVar19 = (double)fsin((double)*pdVar10);
      local_3c = 150.0;
      local_4c = (double)(fVar18 * (double)_DAT_0047a580 +
                         (double)*(double *)(g_Vehicles + 0x484c));
      local_44 = (double)(fVar19 * (double)_DAT_0047a580 +
                         (double)*(double *)(g_Vehicles + 0x485c));
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
  if (((0.0 <= _DAT_0054f980) && (_DAT_0054f980 < _DAT_0047a5c0)) || (g_PlayerCarChoice == 7)) {
    _DAT_0054f980 = _DAT_0054f980 + param_1;
    if ((*(int *)(g_Vehicles + 0x528) == 1) &&
       (puVar17 = (int *)(g_Vehicles + 0x5d4), *(int *)(g_Vehicles + 0x5d4) < 0x19)) {
      lVar20 = __ftol();
      *puVar17 = (int)lVar20;
    }
    if ((*(int *)(g_Vehicles + 0x4d74) == 1) &&
       (puVar17 = (int *)(g_Vehicles + 20000), *(int *)(g_Vehicles + 20000) < 0x19)) {
      lVar20 = __ftol();
      *puVar17 = (int)lVar20;
    }
  }
  else if ((_DAT_0047a5c0 <= _DAT_0054f980) &&
          (((g_IsSplitScreen == 0 && (*(int *)(g_Vehicles + 0x528) == 1)) ||
           ((g_IsSplitScreen == 1 &&
            ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))))) {
    if (DAT_00563d20 == 3) {
      _DAT_00601664 = Race_FindFocusedVehicle();
      if (g_NumRacers - _DAT_00601664 == -1) {
        DAT_00563d20 = 0;
        g_MenuCursorPos = 0;
        iVar14 = g_CurrentTrackIndex * 0x3c;
        local_6c = (double)*(float *)(&DAT_00498538 + g_CurrentTrackIndex * 0x3c);
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
        if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
          local_74 = local_7c;
          local_4c = local_6c;
          local_44 = local_64;
          local_3c = local_54;
          local_28 = local_5c;
        }
      }
      else {
        local_5c = *(double *)(g_VehicleConfigs + 0x38 + g_MenuCursorPos * 200) +
                   *(double *)(g_VehicleConfigs + g_MenuCursorPos * 200);
        iVar14 = g_VehicleConfigs + g_MenuCursorPos * 200;
        local_6c = (double)CONCAT44(*(int *)(g_VehicleConfigs + 0x94 + g_MenuCursorPos * 200),
                                    *(int *)(iVar14 + 0x90));
        local_64 = (double)CONCAT44(*(int *)(g_VehicleConfigs + 0xa4 + g_MenuCursorPos * 200),
                                    *(int *)(iVar14 + 0xa0));
        local_54 = *(double *)(iVar14 + 0x48) + *(double *)(iVar14 + 0x40) +
                   *(double *)(iVar14 + 0x30);
        local_7c = *(double *)(iVar14 + 0x70);
      }
    }
    else {
      iVar14 = g_CurrentTrackIndex * 3 + DAT_00563d20;
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
      if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
        local_4c = (double)*(float *)(&DAT_00498538 + g_CurrentTrackIndex * 0x3c);
        local_44 = (double)*(float *)(&DAT_00498540 + g_CurrentTrackIndex * 0x3c);
        local_28 = (double)(*(float *)(&DAT_0049853c + g_CurrentTrackIndex * 0x3c) +
                           (float)(fVar18 * (double)_DAT_0047a5d0));
        local_3c = (double)(*(float *)(&DAT_00498544 + g_CurrentTrackIndex * 0x3c) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        local_74 = (double)(*(float *)(&DAT_00498548 + g_CurrentTrackIndex * 0x3c) * _DAT_0047a5d8 *
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
  *(int *)(g_pActiveCamera + 7) = 0;
  pdVar10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 4) = ((int*)&local_6c)[1];
  *(int *)pdVar10 = (int)local_6c;
  pdVar10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0xc) = ((int*)&local_5c)[1];
  *(int *)(pdVar10 + 1) = (int)local_5c;
  pdVar10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x14) = ((int*)&local_64)[1];
  *(int *)(pdVar10 + 2) = (int)local_64;
  pdVar10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x1c) = ((int*)&local_54)[1];
  *(int *)(pdVar10 + 3) = (int)local_54;
  pdVar10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x24) = ((int*)&local_7c)[1];
  *(int *)(pdVar10 + 4) = (int)local_7c;
  pdVar10 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 5) = 0;
  *(int *)((int)pdVar10 + 0x2c) = 0;
  *(int *)((int)g_pActiveCamera + 0x7c) = 0;
  *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x58 + g_MenuCursorPos * 200);
  *(int *)((int)g_pActiveCamera + 0x84) =
       *(int *)(g_VehicleConfigs + 0x5c + g_MenuCursorPos * 200);
  lVar20 = __ftol();
  *(int *)(g_pActiveCamera + 0x11) = (int)lVar20;
  if (g_IsSplitScreen == 0) {
    *(int *)((int)g_pActiveCamera + 0x9c) = g_ScreenWidth / 2;
  }
  else if (g_IsSplitScreen == 1) {
    *(int *)((int)g_pActiveCamera + 0x9c) =
         ((int)(g_ScreenWidth + (g_ScreenWidth >> 0x1f & 3U)) >> 2) * 3;
  }
  if (DAT_00552f58 == 1) {
    *(int *)((int)g_pActiveCamera + 0x9c) = g_ScreenWidth / 2;
  }
  if (DAT_005532b0 == 1) {
    *(int *)((int)g_pActiveCamera + 0x7c) = DAT_00563bec;
    if (DAT_00563c14 == 1) {
      *(int *)(g_pActiveCamera + 7) = 1;
      lVar20 = __ftol();
      *(int *)(g_pActiveCamera + 0xe) = (int)lVar20;
      lVar20 = __ftol();
      *(int *)((int)g_pActiveCamera + 0x74) = (int)lVar20;
      lVar20 = __ftol();
      *(int *)(g_pActiveCamera + 0xf) = (int)lVar20;
    }
    if (DAT_00563c14 == 2) {
      iVar14 = g_Vehicles + DAT_00563c5c * 0x484c;
      if (((*(int *)(g_Vehicles + 0x354 + DAT_00563c5c * 0x484c) == 0) &&
          (*(int *)(iVar14 + 0x358) == 0)) && (*(int *)(iVar14 + 0x35c) == 0)) {
        *(int *)(g_pActiveCamera + 7) = 1;
        *g_pActiveCamera = *(double *)(g_Vehicles + DAT_00563c5c * 0x484c) + _DAT_00563c48;
        DAT_005532c4 = *(int *)((int)g_pActiveCamera + 4);
        DAT_005532c0 = *(int *)g_pActiveCamera;
        g_pActiveCamera[1] = *(double *)(g_Vehicles + 8 + DAT_00563c5c * 0x484c) + _DAT_00563c40;
        DAT_005532bc = *(int *)((int)g_pActiveCamera + 0xc);
        DAT_005532b8 = *(int *)(g_pActiveCamera + 1);
        g_pActiveCamera[2] = *(double *)(g_Vehicles + 0x10 + DAT_00563c5c * 0x484c) + _DAT_00563c50;
        DAT_0055328c = *(int *)((int)g_pActiveCamera + 0x14);
        DAT_00553288 = *(int *)(g_pActiveCamera + 2);
        lVar20 = __ftol();
        *(int *)(g_pActiveCamera + 0xe) = (int)lVar20;
        lVar20 = __ftol();
        *(int *)((int)g_pActiveCamera + 0x74) = (int)lVar20;
        lVar20 = __ftol();
        *(int *)(g_pActiveCamera + 0xf) = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9d8 = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9dc = (int)lVar20;
        lVar20 = __ftol();
        _DAT_0054f9e0 = (int)lVar20;
        DAT_00553098 = *(int *)(g_pActiveCamera + 0xe);
        DAT_005530a0 = *(int *)(g_pActiveCamera + 0xe);
        DAT_0055309c = *(int *)(g_pActiveCamera + 0xe);
      }
      else {
        *(int *)(g_pActiveCamera + 7) = 1;
        *g_pActiveCamera = (double)_DAT_0054f9d8;
        g_pActiveCamera[1] = (double)_DAT_0054f9d8;
        g_pActiveCamera[2] = (double)_DAT_0054f9d8;
        *(int *)(g_pActiveCamera + 0xe) = DAT_00553098;
        *(int *)((int)g_pActiveCamera + 0x74) = DAT_005530a0;
        *(int *)(g_pActiveCamera + 0xf) = DAT_0055309c;
      }
    }
  }
  if ((_DAT_0047a5f0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a5f8)) {
    lVar20 = __ftol();
    uVar11 = (int)(unsigned int)lVar20 >> 0x1f;
    if (((((unsigned int)lVar20 ^ uVar11) - uVar11 & 1 ^ uVar11) == uVar11) &&
       (piVar6 = (int *)(g_NumRacers * 0x20 + DAT_00553788), piVar6[7] == 0)) {
      Lisa_MoveDynamicObject(piVar6);
    }
  }
  _DAT_00498730 = g_pLisaDrawCommandQueue;
  _DAT_00498738 = &g_VirtualFramebuffer;
  _DAT_0049873c = g_pActiveTAB;
  _DAT_00498734 = &g_VirtualFramebuffer;
  g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x2c + g_MenuCursorPos * 0x4c);
  iVar14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
  g_ViewportMinY = *(int *)(iVar14 + 0x30);
  g_ViewportMaxX = *(int *)(iVar14 + 0x34) + -1;
  g_ViewportMaxY = *(int *)(iVar14 + 0x38) + -1;
  uVar11 = *(unsigned int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
  if ((int)uVar11 < 0) {
    iVar14 = *(int *)(g_pTrackRoadSequence + 0xc +
                     ((uVar11 ^ (int)uVar11 >> 0x1f) - ((int)uVar11 >> 0x1f)) * 0x18);
  }
  else {
    iVar14 = *(int *)(g_pTrackRoadSequence + uVar11 * 0x18);
  }
  if (_DAT_0047a5c0 <= _DAT_0054f980) {
    DAT_0063c5d8 = 0;
  }
  else {
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + iVar14 * 0xc);
  }
  Lisa_RenderScene();
  Lisa_FlushRasterizerCommands(extraout_ECX,extraout_EDX);
  piVar6 = (int *)(g_NumRacers * 0x20 + DAT_00553788);
  if (piVar6[7] == 1) {
    Lisa_DeleteDynamicObject(piVar6);
  }
  if (((*(int *)(g_Vehicles + 0x528) == 0) || (_DAT_0054f980 <= _DAT_0047a5c0)) &&
     (DAT_005532b0 == 0)) {
    HUD_RenderPlayerElements(g_MenuCursorPos);
  }
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    *(int *)(g_pActiveCamera + 7) = 0;
    pdVar10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 4) = ((int*)&local_4c)[1];
    *(int *)pdVar10 = (int)local_4c;
    pdVar10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0xc) = ((int*)&local_28)[1];
    *(int *)(pdVar10 + 1) = (int)local_28;
    pdVar10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x14) = ((int*)&local_44)[1];
    *(int *)(pdVar10 + 2) = (int)local_44;
    pdVar10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x1c) = ((int*)&local_3c)[1];
    *(int *)(pdVar10 + 3) = (int)local_3c;
    pdVar10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x24) = ((int*)&local_74)[1];
    *(int *)(pdVar10 + 4) = (int)local_74;
    pdVar10 = g_pActiveCamera;
    *(int *)(g_pActiveCamera + 5) = 0;
    *(int *)((int)pdVar10 + 0x2c) = 0;
    *(int *)((int)g_pActiveCamera + 0x7c) = 0;
    *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x120);
    *(int *)((int)g_pActiveCamera + 0x84) = *(int *)(g_VehicleConfigs + 0x124);
    lVar20 = __ftol();
    *(int *)(g_pActiveCamera + 0x11) = (int)lVar20;
    *(int *)((int)g_pActiveCamera + 0x9c) = g_ScreenWidth / (g_IsSplitScreen * 2 + 2);
    _DAT_00498730 = g_pLisaDrawCommandQueue;
    _DAT_00498734 = &g_VirtualFramebuffer;
    _DAT_00498738 = &g_VirtualFramebuffer;
    _DAT_0049873c = g_pActiveTAB;
    g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x78);
    g_ViewportMinY = *(int *)(g_PlayerHUDState + 0x7c);
    g_ViewportMaxX = *(int *)(g_PlayerHUDState + 0x80) + -1;
    g_ViewportMaxY = *(int *)(g_PlayerHUDState + 0x84) + -1;
    if ((_DAT_0047a5f0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a5f8)) {
      lVar20 = __ftol();
      uVar11 = (int)(unsigned int)lVar20 >> 0x1f;
      if (((((unsigned int)lVar20 ^ uVar11) - uVar11 & 1 ^ uVar11) == uVar11) &&
         (iVar14 = g_NumRacers * 0x20 + DAT_00553788, *(int *)(iVar14 + 0x3c) == 0)) {
        Lisa_MoveDynamicObject((int *)(iVar14 + 0x20));
      }
    }
    uVar11 = *(unsigned int *)(g_Vehicles + 0x4bb0);
    if ((int)uVar11 < 0) {
      iVar14 = *(int *)(g_pTrackRoadSequence + 0xc +
                       ((uVar11 ^ (int)uVar11 >> 0x1f) - ((int)uVar11 >> 0x1f)) * 0x18);
    }
    else {
      iVar14 = *(int *)(g_pTrackRoadSequence + uVar11 * 0x18);
    }
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + iVar14 * 0xc);
    Lisa_RenderScene();
    Lisa_FlushRasterizerCommands(extraout_ECX_00,extraout_EDX_00);
    iVar14 = g_NumRacers * 0x20 + DAT_00553788;
    if (*(int *)(iVar14 + 0x3c) == 1) {
      Lisa_DeleteDynamicObject((int *)(iVar14 + 0x20));
    }
    if ((*(int *)(g_Vehicles + 0x4d74) == 0) || (_DAT_0054f980 <= _DAT_0047a5c0)) {
      HUD_RenderPlayerElements(1);
    }
  }
  FX_SpawnWeather();
  if ((0.0 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a588)) {
    __ftol();
    Lisa_RenderPanorama();
  }
  if (((((g_IsSplitScreen == 0) && (*(int *)(g_Vehicles + 0x528) == 1)) ||
       ((g_IsSplitScreen == 1 &&
        ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))) &&
      (_DAT_0047a5c0 <= _DAT_0054f980)) && ((DAT_00563c60 == 0 && (DAT_005532b0 == 0)))) {
    HUD_RenderTrackResults();
  }
  iVar14 = *(int *)(g_PlayerHUDState + 0x38);
  iVar8 = *(int *)(g_PlayerHUDState + 0x34);
  if ((DAT_00527f28 == 0) && ((g_IsSplitScreen == 0 || (DAT_00552f58 == 1)))) {
    local_34 = 0;
    local_30 = 0;
    Gfx_DrawSprite(DAT_0054f960,&local_34,0);
    local_30 = 0;
    local_34 = (iVar8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    Gfx_DrawSprite(DAT_0054f964,&local_34,0);
    local_34 = 0;
    local_30 = (iVar14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    Gfx_DrawSprite(DAT_0054f968,&local_34,0);
    local_34 = (iVar8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    local_30 = (iVar14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    Gfx_DrawSprite(DAT_0054f96c,&local_34,0);
  }
  if (DAT_00563c60 == 1) {
    lVar20 = __ftol();
    DAT_00552f4c = (int)lVar20;
    if (DAT_00552f4c < 1) {
      Ghost_SaveGhostData();
      DAT_00563d88 = 2;
    }
    Lisa_RenderPanorama();
  }
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    iVar4 = g_ScreenWidth / 2;
    iVar8 = g_ScreenHeight;
    for (iVar14 = iVar4 + -1; g_ScreenHeight = iVar8, iVar14 < iVar4 + 1; iVar14 = iVar14 + 1) {
      if (0 < iVar8) {
        puVar16 = (char *)((int)&g_VirtualFramebuffer + iVar14);
        do {
          iVar13 = g_ScreenWidth;
          *puVar16 = 0;
          puVar16 = puVar16 + iVar13;
          iVar8 = iVar8 + -1;
        } while (iVar8 != 0);
      }
      iVar8 = g_ScreenHeight;
    }
  }
  return;
}
/**
 * @original FX_UpdateAllParticles (IGN_WIN.EXE @ 0x00434190, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateAllParticles(void) {
  float fVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  piVar3 = &g_SceneryParticles;
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
        FX_UpdateVehicleWreck(piVar3);
        break;
      case 8:
        FX_UpdateSuperPlotObject((SceneryParticle *)piVar3,iVar5);
        break;
      case 9:
        FX_UpdateDetachedWheel(piVar3,iVar5);
        break;
      case 10:
        FX_UpdateVehicleCrashSequence(piVar3);
        break;
      case 0xb:
        FX_UpdateCarDebris(piVar3,iVar5);
      }
    }
    piVar3 = piVar3 + 0x19;
    iVar5 = iVar5 + 1;
  } while (piVar3 < &DAT_005dfe60);
  _DAT_00563d64 = _DAT_00563d64 + _DAT_0047a430;
  if ((float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4) <= _DAT_00563d64) {
    Track_UpdateMovingPathNodes();
    fVar1 = (float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4);
    if ((float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4) <= _DAT_00563d64) {
      do {
        _DAT_00563d64 = _DAT_00563d64 - fVar1;
      } while (fVar1 <= _DAT_00563d64);
    }
  }
  if (0 < g_CheckpointCount) {
    Track_UpdateDynamicObjects_Type1();
  }
  if (0 < DAT_00563cf0) {
    Track_UpdateDynamicObjects_Type2();
  }
  if (0 < DAT_00563d78) {
    Track_UpdateDynamicObjects_Type3();
  }
  if (0 < DAT_00552f78) {
    Track_UpdateDynamicObjects_Type4();
  }
  if ((g_GameMode == 3) && (iVar5 = 0, 0 < g_NumRacers)) {
    iVar4 = 0;
    do {
      iVar2 = g_Vehicles + iVar4;
      if ((((*(int *)(iVar2 + 0x39c) == g_PlayerCarModel) && (_DAT_00552e48 < 0.0)) &&
          (*(int *)(iVar2 + 0x354) == 0)) &&
         (((*(int *)(iVar2 + 0x358) == 0 && (*(int *)(iVar2 + 0x35c) == 0)) &&
          (*(int *)(iVar2 + 0x528) == 0)))) {
        FX_SpawnAmbientTrackParticles();
      }
      iVar4 = iVar4 + 0x484c;
      iVar5 = iVar5 + 1;
    } while (iVar5 < g_NumRacers);
  }
  HUD_UpdateFloatingMessages();
  return;
}
/**
 * @original FX_UpdateWeatherBounds (IGN_WIN.EXE @ 0x00434580, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateWeatherBounds(void) {
  unsigned int uVar1;
  int iVar2;
  unsigned int uVar3;
  if (g_HudEnabled == 1) {
    if (((g_CurrentTrackIndex == 0) &&
        (iVar2 = *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), 0x2d < iVar2)) &&
       (iVar2 < 0x78)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 0) &&
       ((iVar2 = *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), iVar2 < 0x2e ||
        (0x77 < iVar2)))) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 0;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 0;
      }
    }
  }
  if (g_HudEnabled == 3) {
    if (((g_CurrentTrackIndex == 1) &&
        (uVar1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), uVar3 = (int)uVar1 >> 0x1f
        , iVar2 = (uVar1 ^ uVar3) - uVar3, 0 < iVar2)) && (iVar2 < 0x1e)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 1) &&
       ((uVar1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), uVar3 = (int)uVar1 >> 0x1f
        , iVar2 = (uVar1 ^ uVar3) - uVar3, iVar2 < 1 || (0x1d < iVar2)))) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 0;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 0;
      }
    }
  }
  if (g_HudEnabled == 2) {
    if (((g_CurrentTrackIndex == 4) &&
        (uVar1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), uVar3 = (int)uVar1 >> 0x1f
        , iVar2 = (uVar1 ^ uVar3) - uVar3, 0x1c < iVar2)) && (iVar2 < 100)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 4) &&
       ((uVar1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), uVar3 = (int)uVar1 >> 0x1f
        , iVar2 = (uVar1 ^ uVar3) - uVar3, iVar2 < 0x1d || (99 < iVar2)))) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 0;
        return;
      }
      if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 0;
      }
    }
  }
  return;
}
/**
 * @original FX_UpdateVehicleWreck (IGN_WIN.EXE @ 0x004307b0, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateVehicleWreck(int *wreck) {
  int *piVar1;
  double dVar2;
  int uVar3;
  int uVar4;
  int uVar5;
  int iVar6;
  unsigned int uVar7;
  unsigned int uVar8;
  unsigned int uVar9;
  double dVar10;
  int bVar11;
  char *puVar12;
  double *pdVar13;
  int iVar14;
  int *puVar15;
  int iVar16;
  double *pdVar17;
  int iVar18;
  int *piVar19;
  int iVar20;
  unsigned int *puVar21;
  int iVar22;
  int iVar23;
  int iVar24;
  double fVar25;
  long long lVar26;
  double local_48;
  int local_3c;
  int local_34;
  int local_30;
  double local_24;
  int local_1c;
  int local_18;
  pdVar13 = g_Vehicles;
  local_18 = 0;
  local_34 = 0;
  uVar3 = wreck[1];
  uVar4 = wreck[2];
  local_30 = 0;
  local_1c = 0;
  uVar5 = wreck[3];
  iVar6 = wreck[4];
  iVar23 = wreck[5];
  iVar22 = wreck[6];
  iVar18 = iVar6 * 0x484c;
  *(int *)((int)g_Vehicles + iVar18 + 0x534) = 0;
  *(int *)((int)pdVar13 + iVar18 + 0x538) = 0x40590000;
  *(int *)((int)g_Vehicles + iVar18 + 0x53c) = 0;
  if (iVar23 == 0) {
    iVar20 = iVar6 * 0x20;
    if ((((int *)(DAT_005db02c + iVar20))[7] != 0) && (DAT_0054f930 == 0)) {
      Lisa_DeleteDynamicObject((int *)(DAT_005db02c + iVar20));
    }
    iVar24 = iVar6 * 0x80;
    if (((int *)(g_RaceFinished + iVar24))[7] != 0) {
      Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar24));
    }
    if (*(int *)(g_RaceFinished + iVar24 + 0x3c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar24 + 0x20));
    }
    if (*(int *)(g_RaceFinished + iVar24 + 0x5c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar24 + 0x40));
    }
    if (*(int *)(g_RaceFinished + iVar24 + 0x7c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar24 + 0x60));
    }
    if ((((int *)(DAT_00552f20 + iVar20))[7] != 0) && (DAT_0054f930 == 0)) {
      Lisa_DeleteDynamicObject((int *)(DAT_00552f20 + iVar20));
    }
    if (((*(int *)(g_PlayerHUDState + iVar6 * 0x4c) == 7) && (DAT_0054f930 == 0)) &&
       (((int *)(iVar20 + DAT_00552f70))[7] != 0)) {
      Lisa_DeleteDynamicObject((int *)(iVar20 + DAT_00552f70));
    }
    puVar15 = (int *)((int)g_Vehicles + iVar18);
    puVar15[0xe2] = puVar15[1];
    puVar15[0xe1] = *puVar15;
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar18 + 0x390) =
         *(int *)((int)g_Vehicles + iVar18 + 0xc);
    *(int *)((int)pdVar13 + iVar18 + 0x38c) = *(int *)((int)pdVar13 + iVar18 + 8);
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar18 + 0x398) =
         *(int *)((int)g_Vehicles + iVar18 + 0x14);
    *(int *)((int)pdVar13 + iVar18 + 0x394) = *(int *)((int)pdVar13 + iVar18 + 0x10);
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar18 + 0x380) =
         *(int *)((int)g_Vehicles + iVar18 + 0xfc);
    *(int *)((int)pdVar13 + iVar18 + 0x37c) = *(int *)((int)pdVar13 + iVar18 + 0xf8);
    *(int *)((int)g_Vehicles + iVar18 + 0x270) = 1;
    if (*(int *)((int)g_Vehicles + iVar18 + 0x298) == 1) {
      puVar12 = Audio_GetVoice(*(int *)((int)g_Vehicles + iVar18 + 0x99c));
      if (puVar12 != (char *)0x0) {
        *(int *)(puVar12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
    }
    if (*(int *)((int)g_Vehicles + iVar18 + 0x29c) == 1) {
      puVar12 = Audio_GetVoice(*(int *)((int)g_Vehicles + iVar18 + 0x9a0));
      if (puVar12 != (char *)0x0) {
        *(int *)(puVar12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x29c) = 0;
    }
    g_ActiveParticle.pos_x = 0xffffffc4;
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.vel_x = 0xffffffc4;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.drag = 0x3c;
    g_ActiveParticle.field_28 = 0x3c;
    g_ActiveParticle.pos_z = 0xffffffd8;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.rot_x = 0xffffffd8;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0x3c;
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.vel_x = 0x3c;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.drag = 0xffffffc4;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.field_28 = 0xffffffc4;
    g_ActiveParticle.pos_z = 0xffffffd8;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.rot_x = 0xffffffd8;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.pos_z = 0x3c;
    g_ActiveParticle.vel_x = 0;
    g_ActiveParticle.vel_z = 0x3c;
    g_ActiveParticle.drag = 0;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_x = 0xffffffd8;
    g_ActiveParticle.field_24 = 0xffffffc4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.rot_x = 0xffffffc4;
    g_ActiveParticle.field_28 = 0xffffffd8;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.pos_z = 0xffffffc4;
    g_ActiveParticle.vel_x = 0;
    g_ActiveParticle.vel_z = 0xffffffc4;
    g_ActiveParticle.drag = 0;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_x = 0xffffffd8;
    g_ActiveParticle.field_24 = 0x3c;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.field_28 = 0xffffffd8;
    g_ActiveParticle.rot_x = 0x3c;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0xffffffc4;
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.vel_x = 0xffffffc4;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.drag = 0x3c;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.field_28 = 0x3c;
    g_ActiveParticle.pos_z = 0x28;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.rot_x = 0x28;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0x3c;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.vel_x = 0x3c;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.drag = 0xffffffc4;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.field_28 = 0xffffffc4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.pos_z = 0x28;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.rot_x = 0x28;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.pos_z = 0x3c;
    g_ActiveParticle.vel_x = 0;
    g_ActiveParticle.vel_z = 0x3c;
    g_ActiveParticle.drag = 0;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_x = 0x28;
    g_ActiveParticle.field_24 = 0xffffffc4;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.rot_x = 0xffffffc4;
    g_ActiveParticle.field_28 = 0x28;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    g_ActiveParticle.life = 0;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_y = 0;
    g_ActiveParticle.pos_z = 0xffffffc4;
    g_ActiveParticle.vel_x = 0;
    g_ActiveParticle.vel_z = 0xffffffc4;
    g_ActiveParticle.drag = 0;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_x = 0x28;
    g_ActiveParticle.field_24 = 0x3c;
    g_ActiveParticle.field_2c = 0;
    g_ActiveParticle.vel_y = 0xfffffed4;
    g_ActiveParticle.gravity = 0xfffffed4;
    g_ActiveParticle.rot_x = 0x3c;
    g_ActiveParticle.field_28 = 0x28;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0xffffffc4;
    g_ActiveParticle.pos_z = 0;
    g_ActiveParticle.vel_x = 0xffffffc4;
    g_ActiveParticle.vel_z = 0x3c;
    g_ActiveParticle.drag = 0x3c;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0x3c;
    g_ActiveParticle.field_28 = 0x3c;
    g_ActiveParticle.pos_y = 10;
    g_ActiveParticle.vel_y = 10;
    g_ActiveParticle.gravity = 10;
    g_ActiveParticle.rot_x = 0;
    g_ActiveParticle.field_2c = 10;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0x3c;
    g_ActiveParticle.pos_z = 0;
    g_ActiveParticle.vel_x = 0x3c;
    g_ActiveParticle.vel_z = 0x3c;
    g_ActiveParticle.drag = 0xffffffc4;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0x3c;
    g_ActiveParticle.field_28 = 0xffffffc4;
    g_ActiveParticle.pos_y = 10;
    g_ActiveParticle.vel_y = 10;
    g_ActiveParticle.gravity = 10;
    g_ActiveParticle.rot_x = 0;
    g_ActiveParticle.field_2c = 10;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0xffffffc4;
    g_ActiveParticle.pos_z = 0xffffffc4;
    g_ActiveParticle.vel_x = 0xffffffc4;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.drag = 0x3c;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.pos_y = 10;
    g_ActiveParticle.vel_y = 10;
    g_ActiveParticle.gravity = 10;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.field_28 = 0x3c;
    g_ActiveParticle.rot_x = 0xffffffc4;
    g_ActiveParticle.field_2c = 10;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    g_ActiveParticle.pos_x = 0x3c;
    g_ActiveParticle.pos_z = 0xffffffc4;
    g_ActiveParticle.vel_x = 0x3c;
    g_ActiveParticle.vel_z = 0;
    g_ActiveParticle.drag = 0xffffffc4;
    g_ActiveParticle.type = 9;
    g_ActiveParticle.field_24 = 0;
    g_ActiveParticle.field_28 = 0xffffffc4;
    g_ActiveParticle.pos_y = 10;
    g_ActiveParticle.vel_y = 10;
    g_ActiveParticle.gravity = 10;
    g_ActiveParticle.field_2c = 10;
    g_ActiveParticle.rot_y = &DAT_00497f48;
    g_ActiveParticle.rot_z = 3;
    g_ActiveParticle.rot_x = 0xffffffc4;
    lVar26 = __ftol();
    g_ActiveParticle.field_3c = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_40 = (int)lVar26;
    lVar26 = __ftol();
    g_ActiveParticle.field_44 = (int)lVar26;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    Audio_PlaySampleVol(0,2,5,0,0x10000,22000,0);
  }
  if (iVar22 - iVar23 == 0x14) {
    iVar20 = 10;
    do {
      g_ActiveParticle.type = 0xb;
      g_ActiveParticle.pos_y = 0;
      g_ActiveParticle.pos_x = iVar6;
      Math_RandomFloat0To1();
      lVar26 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar26;
      Math_RandomFloat0To1();
      lVar26 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar26;
      g_ActiveParticle.vel_y = 0;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x2800;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.field_24 = 0x3c;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      iVar20 = iVar20 + -1;
    } while (iVar20 != 0);
    __ftol();
    __ftol();
    Audio_PlaySampleVol(0,4,3,0,0xe000,0,0);
  }
  if (iVar23 <= iVar22 + -0x28) goto LAB_00431ab6;
  pdVar13 = (double *)((int)g_Vehicles + iVar18);
  uVar7 = *(unsigned int *)((int)pdVar13 + 0x364);
  iVar20 = (uVar7 ^ (int)uVar7 >> 0x1f) - ((int)uVar7 >> 0x1f);
  if (iVar20 + 2 < DAT_005287f8) {
    if (-1 < (int)uVar7) {
      local_48 = *(double *)(g_pTrackRoadSequence + uVar7 * 6 + 0xd);
      iVar24 = g_pTrackRoadSequence[uVar7 * 6 + 0xc];
      goto joined_r0x004317bd;
    }
    local_48 = *(double *)(g_pTrackRoadSequence + iVar20 * 6 + 0x10) - _DAT_0047a288;
    piVar19 = g_pTrackRoadSequence + iVar20 * 6 + 0xf;
    iVar24 = *piVar19;
    if (local_48 < _DAT_0047a290) {
      local_48 = local_48 + _DAT_0047a298;
    }
    if (iVar24 == 10000) {
      local_48 = *(double *)(g_pTrackRoadSequence + uVar7 * 6 + 0xd) - _DAT_0047a288;
      iVar24 = g_pTrackRoadSequence[uVar7 * 6 + 0xc];
      if (local_48 < _DAT_0047a290) {
        local_48 = local_48 + _DAT_0047a298;
      }
    }
    if (iVar24 == -2) {
      iVar24 = 0;
      do {
        piVar1 = piVar19 + 6;
        piVar19 = piVar19 + 6;
        iVar24 = iVar24 + 1;
      } while (*piVar1 == -2);
      local_48 = *(double *)(g_pTrackRoadSequence + (iVar24 + iVar20) * 6 + 0xd);
      iVar24 = g_pTrackRoadSequence[(iVar24 + iVar20) * 6 + 0xc];
      goto joined_r0x004317bd;
    }
  }
  else {
    local_48 = *(double *)(g_pTrackRoadSequence + 1);
    iVar24 = *g_pTrackRoadSequence;
joined_r0x004317bd:
    local_48 = local_48 - _DAT_0047a288;
    if (local_48 < _DAT_0047a290) {
      local_48 = local_48 + _DAT_0047a298;
    }
  }
  iVar16 = iVar24 * 0x14 + DAT_0054f9cc;
  iVar24 = *(int *)(iVar16 + 0x10);
  iVar20 = DAT_00525e60 + *(int *)(iVar16 + 4) * 4;
  iVar14 = *(int *)(iVar20 + 4);
  puVar21 = (unsigned int *)(iVar20 + 8 + *(int *)(DAT_00525e60 + *(int *)(iVar16 + 4) * 4) * 0xc);
  if (0 < iVar14) {
    do {
      uVar7 = puVar21[1];
      uVar8 = puVar21[2];
      uVar9 = puVar21[3];
      if ((*puVar21 & 0xffff0000) < 0x280000) {
        local_18 = local_18 +
                   *(int *)(iVar20 + 8 + uVar7 * 0xc) + (*(int *)(iVar16 + 0xc) + 0x6400) * 3 +
                   *(int *)(iVar20 + 8 + uVar8 * 0xc) + *(int *)(iVar20 + 8 + uVar9 * 0xc);
        local_34 = (((((local_34 - *(int *)(iVar20 + 0xc + uVar7 * 0xc)) + iVar24) -
                     *(int *)(iVar20 + 0xc + uVar8 * 0xc)) + iVar24) -
                   *(int *)(iVar20 + 0xc + uVar9 * 0xc)) + iVar24;
        local_1c = local_1c + 3;
        local_30 = local_30 +
                   *(int *)(iVar20 + 0x10 + uVar7 * 0xc) + (*(int *)(iVar16 + 0x14) + 0x6400) * 3 +
                   *(int *)(iVar20 + 0x10 + uVar8 * 0xc) + *(int *)(iVar20 + 0x10 + uVar9 * 0xc);
      }
      puVar21 = puVar21 + 0xb;
      iVar14 = iVar14 + -1;
    } while (iVar14 != 0);
  }
  local_24 = local_48;
  if (local_48 < 0.0) {
    local_24 = local_48 + _DAT_0047a298;
  }
  dVar2 = (double)((iVar23 - iVar22) + 0x28);
  fVar25 = Math_LookupTrigAngle(dVar2);
  *pdVar13 = (double)(((double)(local_18 / local_1c) - (double)*(double *)((int)pdVar13 + 900)) *
                      fVar25 + (double)*(double *)((int)pdVar13 + 900));
  *(double *)((int)g_Vehicles + iVar18 + 8) =
       ((double)(local_34 / local_1c + 0xfa) - *(double *)((int)g_Vehicles + iVar18 + 0x38c)) *
       dVar2 * _DAT_0047a2a0 + *(double *)((int)g_Vehicles + iVar18 + 0x38c);
  pdVar13 = g_Vehicles;
  fVar25 = Math_LookupTrigAngle(dVar2);
  dVar10 = local_24 - _DAT_0047a298;
  *(double *)((int)pdVar13 + iVar18 + 0x10) =
       (double)(((double)(local_30 / local_1c) -
                (double)*(double *)((int)pdVar13 + iVar18 + 0x394)) * fVar25 +
               (double)*(double *)((int)pdVar13 + iVar18 + 0x394));
  if (ABS(dVar10 - *(double *)((int)g_Vehicles + iVar18 + 0x37c)) <
      ABS(local_24 - *(double *)((int)g_Vehicles + iVar18 + 0x37c))) {
    local_24 = dVar10;
  }
  *(double *)((int)g_Vehicles + iVar18 + 0xf8) =
       (local_24 - *(double *)((int)g_Vehicles + iVar18 + 0x37c)) * dVar2 * _DAT_0047a2a0 +
       *(double *)((int)g_Vehicles + iVar18 + 0x37c);
  pdVar13 = (double *)((int)g_Vehicles + iVar18 + 0xf8);
  if (_DAT_0047a2a8 < *(double *)((int)g_Vehicles + iVar18 + 0xf8)) {
    *pdVar13 = *pdVar13 - _DAT_0047a298;
  }
  pdVar13 = (double *)((int)g_Vehicles + iVar18 + 0xf8);
  if (*(double *)((int)g_Vehicles + iVar18 + 0xf8) < _DAT_0047a2b0) {
    *pdVar13 = *pdVar13 + _DAT_0047a298;
  }
LAB_00431ab6:
  iVar20 = iVar23 + 1;
  local_3c = iVar20;
  if (iVar22 <= iVar20) {
    bVar11 = 0;
    iVar22 = 0;
    pdVar13 = g_Vehicles;
    if (0 < g_NumRacers) {
      do {
        if (((((iVar6 != iVar22) &&
              (pdVar17 = (double *)((int)g_Vehicles + iVar18),
              ABS(*pdVar13 - *pdVar17) < _DAT_0047a2b8)) &&
             ((ABS(pdVar13[1] - pdVar17[1]) < _DAT_0047a2b8 &&
              ((ABS(pdVar13[2] - pdVar17[2]) < _DAT_0047a2b8 &&
               (*(int *)((int)pdVar13 + 0x354) == 0)))))) && (*(int *)(pdVar13 + 0x6b) == 0)) &&
           (*(int *)((int)pdVar13 + 0x35c) == 0)) {
          bVar11 = 1;
        }
        iVar22 = iVar22 + 1;
        pdVar13 = (double *)((int)pdVar13 + 0x484c);
      } while (iVar22 < g_NumRacers);
    }
    local_3c = iVar23;
    if ((!bVar11) &&
       ((*(int *)((int)g_Vehicles + iVar18 + 0x528) == 0 ||
        (*(int *)((int)g_Vehicles + iVar18 + 0x604) == 1)))) {
      *wreck = 0;
      *(int *)((int)g_Vehicles + iVar18 + 0x350) = 1;
      iVar23 = iVar6 * 0x20;
      *(int *)((int)g_Vehicles + iVar18 + 0x358) = 0;
      pdVar13 = g_Vehicles;
      *(int *)((int)g_Vehicles + iVar18 + 0x534) = 0;
      *(int *)((int)pdVar13 + iVar18 + 0x538) = 0x40590000;
      if ((((int *)(DAT_005db02c + iVar23))[7] == 0) && (DAT_0054f930 == 0)) {
        Lisa_MoveDynamicObject((int *)(DAT_005db02c + iVar23));
      }
      iVar22 = iVar6 * 0x80;
      if (((int *)(g_RaceFinished + iVar22))[7] == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar22));
      }
      if (*(int *)(g_RaceFinished + iVar22 + 0x3c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar22 + 0x20));
      }
      if (*(int *)(g_RaceFinished + iVar22 + 0x5c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar22 + 0x40));
      }
      if (*(int *)(g_RaceFinished + iVar22 + 0x7c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar22 + 0x60));
      }
      if ((((int *)(DAT_00552f20 + iVar23))[7] == 0) && (DAT_0054f930 == 0)) {
        Lisa_MoveDynamicObject((int *)(DAT_00552f20 + iVar23));
      }
      if (((*(int *)(g_PlayerHUDState + iVar6 * 0x4c) == 7) && (DAT_0054f930 == 0)) &&
         (((int *)(iVar23 + DAT_00552f70))[7] == 0)) {
        Lisa_MoveDynamicObject((int *)(iVar23 + DAT_00552f70));
      }
      Car_UpdateDynamicObjects(iVar6);
      local_3c = iVar20;
    }
  }
  wreck[2] = uVar4;
  wreck[1] = uVar3;
  wreck[3] = uVar5;
  wreck[5] = local_3c;
  return;
}

/**
 * @original FX_UpdateDetachedWheel (IGN_WIN.EXE @ 0x00431d00, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateDetachedWheel(int *wheel, int wheel_index) {
  int uVar1;
  int uVar2;
  int uVar3;
  int uVar4;
  int uVar5;
  int uVar6;
  int uVar7;
  int uVar8;
  int uVar9;
  int uVar10;
  int uVar11;
  int iVar12;
  int uVar13;
  int iVar14;
  int iVar15;
  int uVar16;
  int iVar17;
  int iVar18;
  int *piVar19;
  int iVar20;
  int iVar21;
  int iVar22;
  int iVar23;
  uVar1 = wheel[2];
  uVar2 = wheel[1];
  uVar3 = wheel[3];
  uVar4 = wheel[4];
  uVar5 = wheel[5];
  uVar6 = wheel[6];
  uVar7 = wheel[7];
  uVar8 = wheel[8];
  uVar9 = wheel[9];
  uVar10 = wheel[10];
  uVar11 = wheel[0xb];
  iVar12 = wheel[0xd];
  uVar13 = wheel[0xc];
  iVar21 = wheel[0xf];
  iVar14 = wheel[0x10];
  iVar15 = wheel[0x11];
  uVar16 = wheel[0xe];
  iVar20 = wheel[0x12];
  iVar17 = wheel[0x13];
  iVar18 = wheel[0x15];
  iVar22 = wheel[0x14];
  if (iVar22 == 0) {
    iVar23 = wheel_index * 800;
    *(int *)(&DAT_00528800 + iVar23) = 4;
    *(int *)(&DAT_00528804 + iVar23) = 2;
    *(int *)(&DAT_00528808 + iVar23) = uVar2;
    *(int *)(&DAT_0052880c + iVar23) = uVar1;
    *(int *)(&DAT_00528810 + iVar23) = uVar3;
    *(int *)(&DAT_00528814 + iVar23) = uVar4;
    *(int *)(&DAT_00528818 + iVar23) = uVar5;
    *(int *)(&DAT_0052881c + iVar23) = uVar6;
    *(int *)(&DAT_00528820 + iVar23) = uVar7;
    *(int *)(&DAT_00528824 + iVar23) = uVar8;
    *(int *)(&DAT_00528828 + iVar23) = uVar9;
    *(int *)(&DAT_0052882c + iVar23) = uVar10;
    *(int *)(&DAT_00528830 + iVar23) = uVar11;
    *(int *)(&DAT_00528834 + iVar23) = uVar13;
    *(int *)(&DAT_00528838 + iVar23) = 0x13;
    *(int *)(&DAT_0052883c + iVar23) = 0;
    *(int *)(&DAT_00528840 + iVar23) = 1;
    *(int *)(&DAT_00528844 + iVar23) = 2;
    *(int *)(&DAT_00528860 + iVar23) = uVar16;
    *(int *)(&DAT_00528864 + iVar23) = 0x13;
    *(int *)(&DAT_00528868 + iVar23) = 0;
    *(int *)(&DAT_0052886c + iVar23) = 2;
    *(int *)(&DAT_00528870 + iVar23) = 3;
    *(int *)(&DAT_0052888c + iVar23) = uVar16;
    Lisa_SetDynamicObjectMesh(DAT_0054f994,uVar13,(int *)(wheel_index * 0x20 + DAT_0054f994),
                 (int *)(&DAT_00528800 + iVar23),uVar16,200,0,0x14,0);
  }
  iVar23 = wheel_index * 0x20;
  *(int *)(iVar23 + 4 + DAT_0054f994) = (int)(iVar21 + (iVar21 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(iVar23 + 8 + DAT_0054f994) = (int)(iVar14 + (iVar14 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(iVar23 + 0xc + DAT_0054f994) = (int)(iVar15 + (iVar15 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(iVar23 + 0x10 + DAT_0054f994) = 0;
  *(int *)(iVar23 + 0x14 + DAT_0054f994) = 0;
  *(int *)(iVar23 + 0x18 + DAT_0054f994) = 0;
  piVar19 = (int *)(DAT_0054f994 + iVar23);
  if (piVar19[7] == 0) {
    Lisa_MoveDynamicObject(piVar19);
  }
  else {
    iVar21 = Lisa_UpdateObjectSpatialGrid(piVar19);
    if (iVar21 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_WATER_SPLA_00499468);
    }
  }
  iVar20 = iVar20 + iVar17;
  iVar21 = wheel_index * 800;
  piVar19 = (int *)(iVar12 + ((int)(iVar20 + (iVar20 >> 0x1f & 0x3ffU)) >> 10) * 0x10);
  *(int *)(&DAT_00528848 + iVar21) = *piVar19 << 8;
  *(int *)(&DAT_0052884c + iVar21) = piVar19[3] << 8;
  *(int *)(&DAT_00528850 + iVar21) = *piVar19 << 8;
  *(int *)(&DAT_00528854 + iVar21) = piVar19[1] << 8;
  *(int *)(&DAT_00528858 + iVar21) = piVar19[2] << 8;
  *(int *)(&DAT_0052885c + iVar21) = piVar19[1] << 8;
  *(int *)(&DAT_00528874 + iVar21) = *piVar19 << 8;
  *(int *)(&DAT_00528878 + iVar21) = piVar19[3] << 8;
  *(int *)(&DAT_0052887c + iVar21) = piVar19[2] << 8;
  *(int *)(&DAT_00528880 + iVar21) = piVar19[1] << 8;
  iVar22 = iVar22 + 1;
  *(int *)(&DAT_00528884 + iVar21) = piVar19[2] << 8;
  *(int *)(&DAT_00528888 + iVar21) = piVar19[3] << 8;
  if (iVar18 <= iVar22) {
    Lisa_DeleteDynamicObject((int *)(iVar23 + DAT_0054f994));
    *wheel = 0;
  }
  wheel[0x14] = iVar22;
  wheel[0x12] = iVar20;
  return;
}

/**
 * @original FX_UpdateVehicleCrashSequence (IGN_WIN.EXE @ 0x00432040, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateVehicleCrashSequence(int *crash_seq) {
  int *piVar1;
  double dVar2;
  int uVar3;
  int uVar4;
  int uVar5;
  int iVar6;
  unsigned int uVar7;
  unsigned int uVar8;
  unsigned int uVar9;
  double dVar10;
  int bVar11;
  char *puVar12;
  double *pdVar13;
  int iVar14;
  int iVar15;
  int *puVar16;
  int iVar17;
  double *pdVar18;
  int iVar19;
  unsigned int *puVar20;
  int iVar21;
  int *piVar22;
  int iVar23;
  int iVar24;
  int iVar25;
  double fVar26;
  long long lVar27;
  double local_48;
  int local_3c;
  int local_34;
  double local_28;
  int local_20;
  int local_1c;
  pdVar13 = g_Vehicles;
  iVar24 = 0;
  local_1c = 0;
  local_34 = 0;
  uVar3 = crash_seq[1];
  uVar4 = crash_seq[2];
  local_20 = 0;
  uVar5 = crash_seq[3];
  iVar6 = crash_seq[4];
  iVar25 = crash_seq[5];
  iVar23 = crash_seq[6];
  iVar15 = iVar6 * 0x484c;
  *(int *)((int)g_Vehicles + iVar15 + 0x534) = 0;
  *(int *)((int)pdVar13 + iVar15 + 0x538) = 0x40590000;
  *(int *)((int)g_Vehicles + iVar15 + 0x53c) = 0;
  if (iVar25 == 0) {
    puVar16 = (int *)(iVar15 + (int)g_Vehicles);
    puVar16[0xe2] = puVar16[1];
    puVar16[0xe1] = *puVar16;
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar15 + 0x390) =
         *(int *)((int)g_Vehicles + iVar15 + 0xc);
    *(int *)((int)pdVar13 + iVar15 + 0x38c) = *(int *)((int)pdVar13 + iVar15 + 8);
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar15 + 0x398) =
         *(int *)((int)g_Vehicles + iVar15 + 0x14);
    *(int *)((int)pdVar13 + iVar15 + 0x394) = *(int *)((int)pdVar13 + iVar15 + 0x10);
    pdVar13 = g_Vehicles;
    *(int *)((int)g_Vehicles + iVar15 + 0x380) =
         *(int *)((int)g_Vehicles + iVar15 + 0xfc);
    *(int *)((int)pdVar13 + iVar15 + 0x37c) = *(int *)((int)pdVar13 + iVar15 + 0xf8);
    *(int *)((int)g_Vehicles + iVar15 + 0x270) = 1;
    if (*(int *)((int)g_Vehicles + iVar15 + 0x298) == 1) {
      puVar12 = Audio_GetVoice(*(int *)((int)g_Vehicles + iVar15 + 0x99c));
      if (puVar12 != (char *)0x0) {
        *(int *)(puVar12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
    }
    if (*(int *)((int)g_Vehicles + iVar15 + 0x29c) == 1) {
      puVar12 = Audio_GetVoice(*(int *)((int)g_Vehicles + iVar15 + 0x9a0));
      if (puVar12 != (char *)0x0) {
        *(int *)(puVar12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x29c) = 0;
    }
  }
  if (iVar25 < 0x1f) {
    iVar19 = 0x1e - iVar25;
    if (10 < iVar19) {
      iVar19 = 10;
    }
    dVar2 = (double)iVar19;
    pdVar13 = (double *)((int)g_Vehicles + iVar15 + 900);
    *pdVar13 = dVar2 + *pdVar13;
    *(double *)((int)g_Vehicles + iVar15 + 0x394) =
         *(double *)((int)g_Vehicles + iVar15 + 0x394) + dVar2;
    *(double *)((int)g_Vehicles + iVar15) = *(double *)((int)g_Vehicles + iVar15) + dVar2;
    if (iVar25 < 10) {
      dVar10 = *(double *)((int)g_Vehicles + iVar15 + 8) + _DAT_0047a2c8;
    }
    else {
      dVar10 = *(double *)((int)g_Vehicles + iVar15 + 8) - _DAT_0047a2d0;
    }
    *(double *)((int)g_Vehicles + iVar15 + 8) = dVar10;
    *(double *)((int)g_Vehicles + iVar15 + 0x10) =
         *(double *)((int)g_Vehicles + iVar15 + 0x10) + dVar2;
    *(double *)((int)g_Vehicles + iVar15 + 0x108) =
         *(double *)((int)g_Vehicles + iVar15 + 0x108) + _DAT_0047a2d8;
    *(double *)((int)g_Vehicles + iVar15 + 0x110) =
         *(double *)((int)g_Vehicles + iVar15 + 0x110) + _DAT_0047a2e0;
    fVar26 = Math_RandomFloat0To1();
    if (fVar26 == (double)_DAT_0047a2e8) {
      __ftol();
      __ftol();
      Audio_PlaySampleVol(0,2,1,0,0x10000,22000,0);
    }
    if (iVar25 == 0x1e) {
      iVar19 = iVar6 * 0x20;
      if ((((int *)(DAT_005db02c + iVar19))[7] != 0) && (DAT_0054f930 == 0)) {
        Lisa_DeleteDynamicObject((int *)(DAT_005db02c + iVar19));
      }
      iVar21 = iVar6 * 0x80;
      if (((int *)(g_RaceFinished + iVar21))[7] != 0) {
        Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar21));
      }
      if (*(int *)(g_RaceFinished + iVar21 + 0x3c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar21 + 0x20));
      }
      if (*(int *)(g_RaceFinished + iVar21 + 0x5c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar21 + 0x40));
      }
      if (*(int *)(g_RaceFinished + iVar21 + 0x7c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_RaceFinished + iVar21 + 0x60));
      }
      if ((((int *)(DAT_00552f20 + iVar19))[7] != 0) && (DAT_0054f930 == 0)) {
        Lisa_DeleteDynamicObject((int *)(DAT_00552f20 + iVar19));
      }
      if (((*(int *)(g_PlayerHUDState + iVar6 * 0x4c) == 7) && (DAT_0054f930 == 0)) &&
         (((int *)(iVar19 + DAT_00552f70))[7] != 0)) {
        Lisa_DeleteDynamicObject((int *)(iVar19 + DAT_00552f70));
      }
    }
  }
  if (iVar23 - iVar25 == 0x14) {
    iVar19 = 10;
    do {
      g_ActiveParticle.type = 0xb;
      g_ActiveParticle.pos_y = 0;
      g_ActiveParticle.pos_x = iVar6;
      Math_RandomFloat0To1();
      lVar27 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar27;
      Math_RandomFloat0To1();
      lVar27 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar27;
      g_ActiveParticle.vel_y = 0;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x2800;
      g_ActiveParticle.field_24 = 0x3c;
      g_ActiveParticle.gravity = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      iVar19 = iVar19 + -1;
    } while (iVar19 != 0);
    __ftol();
    __ftol();
    Audio_PlaySampleVol(0,4,3,0,0xe000,0,0);
  }
  if (iVar25 <= iVar23 + -0x28) goto LAB_00432970;
  pdVar13 = (double *)((int)g_Vehicles + iVar15);
  uVar7 = *(unsigned int *)((int)pdVar13 + 0x364);
  iVar19 = (uVar7 ^ (int)uVar7 >> 0x1f) - ((int)uVar7 >> 0x1f);
  if (iVar19 + 2 < DAT_005287f8) {
    if (-1 < (int)uVar7) {
      local_48 = *(double *)(g_pTrackRoadSequence + uVar7 * 6 + 0xd);
      iVar21 = g_pTrackRoadSequence[uVar7 * 6 + 0xc];
      goto joined_r0x00432683;
    }
    local_48 = *(double *)(g_pTrackRoadSequence + iVar19 * 6 + 0x10) - _DAT_0047a300;
    piVar22 = g_pTrackRoadSequence + iVar19 * 6 + 0xf;
    iVar21 = *piVar22;
    if (local_48 < _DAT_0047a308) {
      local_48 = local_48 + _DAT_0047a310;
    }
    if (iVar21 == 10000) {
      local_48 = *(double *)(g_pTrackRoadSequence + uVar7 * 6 + 0xd) - _DAT_0047a300;
      iVar21 = g_pTrackRoadSequence[uVar7 * 6 + 0xc];
      if (local_48 < _DAT_0047a308) {
        local_48 = local_48 + _DAT_0047a310;
      }
    }
    if (iVar21 == -2) {
      iVar21 = 0;
      do {
        piVar1 = piVar22 + 6;
        piVar22 = piVar22 + 6;
        iVar21 = iVar21 + 1;
      } while (*piVar1 == -2);
      local_48 = *(double *)(g_pTrackRoadSequence + (iVar21 + iVar19) * 6 + 0xd);
      iVar21 = g_pTrackRoadSequence[(iVar21 + iVar19) * 6 + 0xc];
      goto joined_r0x00432683;
    }
  }
  else {
    local_48 = *(double *)(g_pTrackRoadSequence + 1);
    iVar21 = *g_pTrackRoadSequence;
joined_r0x00432683:
    local_48 = local_48 - _DAT_0047a300;
    if (local_48 < _DAT_0047a308) {
      local_48 = local_48 + _DAT_0047a310;
    }
  }
  iVar17 = iVar21 * 0x14 + DAT_0054f9cc;
  iVar21 = *(int *)(iVar17 + 0x10);
  iVar19 = DAT_00525e60 + *(int *)(iVar17 + 4) * 4;
  iVar14 = *(int *)(iVar19 + 4);
  puVar20 = (unsigned int *)(iVar19 + 8 + *(int *)(DAT_00525e60 + *(int *)(iVar17 + 4) * 4) * 0xc);
  if (0 < iVar14) {
    do {
      uVar7 = puVar20[1];
      uVar8 = puVar20[2];
      uVar9 = puVar20[3];
      if ((*puVar20 & 0xffff0000) < 0x280000) {
        local_1c = local_1c +
                   *(int *)(iVar19 + 8 + uVar7 * 0xc) + (*(int *)(iVar17 + 0xc) + 0x6400) * 3 +
                   *(int *)(iVar19 + 8 + uVar8 * 0xc) + *(int *)(iVar19 + 8 + uVar9 * 0xc);
        iVar24 = (((((iVar24 - *(int *)(iVar19 + 0xc + uVar7 * 0xc)) + iVar21) -
                   *(int *)(iVar19 + 0xc + uVar8 * 0xc)) + iVar21) -
                 *(int *)(iVar19 + 0xc + uVar9 * 0xc)) + iVar21;
        local_34 = local_34 +
                   *(int *)(iVar19 + 0x10 + uVar7 * 0xc) + (*(int *)(iVar17 + 0x14) + 0x6400) * 3 +
                   *(int *)(iVar19 + 0x10 + uVar8 * 0xc) + *(int *)(iVar19 + 0x10 + uVar9 * 0xc);
        local_20 = local_20 + 3;
      }
      puVar20 = puVar20 + 0xb;
      iVar14 = iVar14 + -1;
    } while (iVar14 != 0);
  }
  local_28 = local_48;
  if (local_48 < 0.0) {
    local_28 = local_48 + _DAT_0047a310;
  }
  dVar2 = (double)((iVar25 - iVar23) + 0x28);
  fVar26 = Math_LookupTrigAngle(dVar2);
  *pdVar13 = (double)(((double)(local_1c / local_20) - (double)*(double *)((int)pdVar13 + 900)) *
                      fVar26 + (double)*(double *)((int)pdVar13 + 900));
  *(double *)((int)g_Vehicles + iVar15 + 8) =
       ((double)(iVar24 / local_20 + 0xfa) - *(double *)((int)g_Vehicles + iVar15 + 0x38c)) *
       dVar2 * _DAT_0047a318 + *(double *)((int)g_Vehicles + iVar15 + 0x38c);
  pdVar13 = g_Vehicles;
  fVar26 = Math_LookupTrigAngle(dVar2);
  dVar10 = local_28 - _DAT_0047a310;
  *(double *)((int)pdVar13 + iVar15 + 0x10) =
       (double)(((double)(local_34 / local_20) -
                (double)*(double *)((int)pdVar13 + iVar15 + 0x394)) * fVar26 +
               (double)*(double *)((int)pdVar13 + iVar15 + 0x394));
  if (ABS(dVar10 - *(double *)((int)g_Vehicles + iVar15 + 0x37c)) <
      ABS(local_28 - *(double *)((int)g_Vehicles + iVar15 + 0x37c))) {
    local_28 = dVar10;
  }
  *(double *)((int)g_Vehicles + iVar15 + 0xf8) =
       (local_28 - *(double *)((int)g_Vehicles + iVar15 + 0x37c)) * dVar2 * _DAT_0047a318 +
       *(double *)((int)g_Vehicles + iVar15 + 0x37c);
  pdVar13 = (double *)((int)g_Vehicles + iVar15 + 0xf8);
  if (_DAT_0047a320 < *(double *)((int)g_Vehicles + iVar15 + 0xf8)) {
    *pdVar13 = *pdVar13 - _DAT_0047a310;
  }
  pdVar13 = (double *)((int)g_Vehicles + iVar15 + 0xf8);
  if (*(double *)((int)g_Vehicles + iVar15 + 0xf8) < _DAT_0047a328) {
    *pdVar13 = *pdVar13 + _DAT_0047a310;
  }
LAB_00432970:
  iVar24 = iVar25 + 1;
  local_3c = iVar24;
  if (iVar23 <= iVar24) {
    bVar11 = 0;
    iVar23 = 0;
    pdVar13 = g_Vehicles;
    if (0 < g_NumRacers) {
      do {
        if (((((iVar6 != iVar23) &&
              (pdVar18 = (double *)(iVar15 + (int)g_Vehicles),
              ABS(*pdVar13 - *pdVar18) < _DAT_0047a330)) &&
             ((ABS(pdVar13[1] - pdVar18[1]) < _DAT_0047a330 &&
              ((ABS(pdVar13[2] - pdVar18[2]) < _DAT_0047a330 &&
               (*(int *)((int)pdVar13 + 0x354) == 0)))))) && (*(int *)(pdVar13 + 0x6b) == 0)) &&
           (*(int *)((int)pdVar13 + 0x35c) == 0)) {
          bVar11 = 1;
        }
        iVar23 = iVar23 + 1;
        pdVar13 = (double *)((int)pdVar13 + 0x484c);
      } while (iVar23 < g_NumRacers);
    }
    local_3c = iVar25;
    if ((!bVar11) &&
       ((*(int *)((int)g_Vehicles + iVar15 + 0x528) == 0 ||
        (*(int *)((int)g_Vehicles + iVar15 + 0x604) == 1)))) {
      *crash_seq = 0;
      *(int *)((int)g_Vehicles + iVar15 + 0x350) = 1;
      iVar25 = iVar6 * 0x20;
      *(int *)((int)g_Vehicles + iVar15 + 0x35c) = 0;
      pdVar13 = g_Vehicles;
      *(int *)((int)g_Vehicles + iVar15 + 0x534) = 0;
      *(int *)((int)pdVar13 + iVar15 + 0x538) = 0x40590000;
      if ((((int *)(DAT_005db02c + iVar25))[7] == 0) && (DAT_0054f930 == 0)) {
        Lisa_MoveDynamicObject((int *)(DAT_005db02c + iVar25));
      }
      iVar23 = iVar6 * 0x80;
      if (((int *)(g_RaceFinished + iVar23))[7] == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar23));
      }
      if (*(int *)(g_RaceFinished + iVar23 + 0x3c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar23 + 0x20));
      }
      if (*(int *)(g_RaceFinished + iVar23 + 0x5c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar23 + 0x40));
      }
      if (*(int *)(g_RaceFinished + iVar23 + 0x7c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_RaceFinished + iVar23 + 0x60));
      }
      if ((((int *)(DAT_00552f20 + iVar25))[7] == 0) && (DAT_0054f930 == 0)) {
        Lisa_MoveDynamicObject((int *)(DAT_00552f20 + iVar25));
      }
      if (((*(int *)(g_PlayerHUDState + iVar6 * 0x4c) == 7) && (DAT_0054f930 == 0)) &&
         (((int *)(iVar25 + DAT_00552f70))[7] == 0)) {
        Lisa_MoveDynamicObject((int *)(iVar25 + DAT_00552f70));
      }
      Car_UpdateDynamicObjects(iVar6);
      local_3c = iVar24;
    }
  }
  crash_seq[2] = uVar4;
  crash_seq[1] = uVar3;
  crash_seq[3] = uVar5;
  crash_seq[5] = local_3c;
  return;
}

/**
 * @original FX_UpdateCarDebris (IGN_WIN.EXE @ 0x00432bb0, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateCarDebris(int *debris, int debris_idx) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  long long lVar12;
  int local_1c;
  local_1c = debris[2];
  iVar11 = debris[3];
  iVar10 = debris[4];
  iVar1 = debris[7];
  iVar2 = debris[5];
  iVar3 = debris[6];
  iVar4 = debris[8];
  iVar5 = debris[9];
  if (iVar4 == 0) {
    iVar6 = debris_idx * 800;
    *(int *)(&DAT_00528800 + iVar6) = 1;
    *(int *)(&DAT_00528804 + iVar6) = 1;
    *(int *)(&DAT_00528808 + iVar6) = 0;
    *(int *)(&DAT_0052880c + iVar6) = 0;
    *(int *)(&DAT_00528810 + iVar6) = 0;
    *(int *)(&DAT_00528814 + iVar6) = 7;
    iVar7 = DAT_00552d80;
    *(int *)(&DAT_00528818 + iVar6) = 0;
    *(int *)(&DAT_0052881c + iVar6) = 0x2100;
    *(int *)(&DAT_00528820 + iVar6) = 0x5400;
    *(int *)(&DAT_00528824 + iVar6) = 0x4200;
    *(int *)(&DAT_00528828 + iVar6) = 0x7500;
    *(int *)(&DAT_0052882c + iVar6) = 0;
    *(int *)(&DAT_00528830 + iVar6) = iVar7 * 0x96;
    iVar9 = DAT_0054f994;
    iVar7 = DAT_00552f5c * 0x96;
    *(int *)(&DAT_00528834 + iVar6) = iVar7;
    Lisa_SetDynamicObjectMesh(iVar9,iVar7,(int *)(debris_idx * 0x20 + iVar9),(int *)(&DAT_00528800 + iVar6),2,200,0,
                 0x14,0);
  }
  iVar7 = iVar4 * -8 + 0xfa;
  *(int *)(&DAT_00528808 + debris_idx * 800) = iVar7;
  if (iVar7 < 0x32) {
    *(int *)(&DAT_00528808 + debris_idx * 800) = 0x32;
  }
  if (0x59c00 < local_1c) {
    local_1c = local_1c + ((local_1c + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (0x59c00 < iVar11) {
    iVar11 = iVar11 + ((iVar11 + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (0x59c00 < iVar10) {
    iVar10 = iVar10 + ((iVar10 + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (local_1c < 0) {
    local_1c = local_1c + ((0x59fffU - local_1c) / 0x5a000) * 0x5a000;
  }
  if (iVar11 < 0) {
    iVar11 = iVar11 + ((0x59fffU - iVar11) / 0x5a000) * 0x5a000;
  }
  if (iVar10 < 0) {
    iVar10 = iVar10 + ((0x59fffU - iVar10) / 0x5a000) * 0x5a000;
  }
  iVar7 = debris_idx * 0x20;
  lVar12 = __ftol();
  *(int *)(iVar7 + 4 + DAT_0054f994) = (int)lVar12;
  lVar12 = __ftol();
  *(int *)(iVar7 + 8 + DAT_0054f994) = (int)lVar12;
  lVar12 = __ftol();
  *(int *)(iVar7 + 0xc + DAT_0054f994) = (int)lVar12;
  *(int *)(iVar7 + 0x10 + DAT_0054f994) =
       (int)(local_1c * 10 + (local_1c * 10 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(iVar7 + 0x14 + DAT_0054f994) = (int)(iVar11 * 10 + (iVar11 * 10 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(iVar7 + 0x18 + DAT_0054f994) = (int)(iVar10 * 10 + (iVar10 * 10 >> 0x1f & 0x3ffU)) >> 10;
  piVar8 = (int *)(DAT_0054f994 + iVar7);
  if (piVar8[7] == 0) {
    Lisa_MoveDynamicObject(piVar8);
  }
  else {
    iVar9 = Lisa_UpdateObjectSpatialGrid(piVar8);
    if (iVar9 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_TRAN_SPRIT_004993b4);
    }
  }
  if (iVar5 <= iVar4 + 1) {
    Lisa_DeleteDynamicObject((int *)(iVar7 + DAT_0054f994));
    *debris = 0;
  }
  debris[2] = local_1c + iVar2;
  debris[3] = iVar11 + iVar3;
  debris[4] = iVar10 + iVar1;
  debris[8] = iVar4 + 1;
  return;
}

/**
 * @original FX_SpawnWaterSplashes (IGN_WIN.EXE @ 0x00432f20, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWaterSplashes(void) {
  int iVar1;
  long long lVar2;
  long long lVar3;
  long long lVar4;
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (((((*(int *)(g_Vehicles + 0x5d8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) ||
        (*(int *)(iVar1 + 0x5dc) == 0x5a)) || (*(int *)(iVar1 + 0x5e0) == 0x5a)) ||
      (*(int *)(iVar1 + 0x5e4) == 0x5a)) &&
     ((_DAT_0047a340 < *(double *)(iVar1 + 0x118) && (*(int *)(iVar1 + 0x270) == 0)))) {
    *(int *)(iVar1 + 0x33c) = 6;
    *(int *)(g_Vehicles + 0x5e8 + g_ActiveVehicleIndex * 0x484c) =
         *(int *)(g_Vehicles + 0x5d8 + g_ActiveVehicleIndex * 0x484c);
    *(int *)(g_Vehicles + 0x5ec + g_ActiveVehicleIndex * 0x484c) =
         *(int *)(g_Vehicles + 0x5dc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(g_Vehicles + 0x5f0 + g_ActiveVehicleIndex * 0x484c) =
         *(int *)(g_Vehicles + 0x5e0 + g_ActiveVehicleIndex * 0x484c);
    *(int *)(g_Vehicles + 0x5f4 + g_ActiveVehicleIndex * 0x484c) =
         *(int *)(g_Vehicles + 0x5e4 + g_ActiveVehicleIndex * 0x484c);
  }
  if (0 < *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c)) {
    Math_RandomFloat0To1();
    __ftol();
    iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x5e8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) {
      fcos((double)*(double *)(iVar1 + 0xf8));
      fsin((double)*(double *)(iVar1 + 0xf8));
    }
    if (*(int *)(iVar1 + 0x5ec) == 0x5a) {
      fcos((double)*(double *)(iVar1 + 0xf8));
      fsin((double)*(double *)(iVar1 + 0xf8));
    }
    if (*(int *)(iVar1 + 0x5f0) == 0x5a) {
      fcos((double)*(double *)(iVar1 + 0xf8));
      fsin((double)*(double *)(iVar1 + 0xf8));
    }
    if (*(int *)(iVar1 + 0x5f4) == 0x5a) {
      fcos((double)*(double *)(iVar1 + 0xf8));
      fsin((double)*(double *)(iVar1 + 0xf8));
    }
    iVar1 = (*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) == 0) + 1;
    if (iVar1 != 0) {
      lVar2 = __ftol();
      lVar3 = __ftol();
      do {
        Math_RandomFloat0To1();
        Math_RandomFloat0To1();
        fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
        fsin((double)*(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0xf8));
        g_ActiveParticle.type = 2;
        g_ActiveParticle.pos_x = (int)lVar2;
        lVar4 = __ftol();
        g_ActiveParticle.pos_y = (int)lVar4;
        g_ActiveParticle.pos_z = (int)lVar3;
        lVar4 = __ftol();
        g_ActiveParticle.vel_x = (int)lVar4;
        g_ActiveParticle.vel_y = 0x4cc;
        lVar4 = __ftol();
        g_ActiveParticle.vel_z = (int)lVar4;
        g_ActiveParticle.drag = 0x3e1;
        g_ActiveParticle.gravity = 0xffffff67;
        Math_RandomFloat0To1();
        lVar4 = __ftol();
        g_ActiveParticle.field_24 = (int)lVar4;
        g_ActiveParticle.field_28 = 0xfffffd9a;
        g_ActiveParticle.field_2c = 1;
        g_ActiveParticle.rot_x = 0;
        g_ActiveParticle.rot_y = 0xf;
        FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
        iVar1 = iVar1 + -1;
      } while (iVar1 != 0);
    }
  }
  iVar1 = *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c);
  if (0 < iVar1) {
    *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c) = iVar1 + -1;
  }
  return;
}

/**
 * @original FX_SpawnTireDirtDebris (IGN_WIN.EXE @ 0x004333e0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnTireDirtDebris(void) {
  int iVar1;
  int iVar2;
  long long lVar3;
  long long lVar4;
  int local_24;
  if ((_DAT_0047a380 < *(double *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c)) &&
     (*(int *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x270) == 0)) {
    local_24 = 0;
    do {
      *(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) = 0;
      iVar2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      iVar1 = *(int *)(g_Vehicles + 0x150 + g_ActiveVehicleIndex * 0x484c);
      if (((((iVar1 == 6) || (iVar1 == 0x10)) || (iVar1 == 0x1a)) || (DAT_004949b0 == 1)) &&
         (local_24 == 0)) {
        fcos((double)*(double *)(iVar2 + 0xf8));
        fsin((double)*(double *)(iVar2 + 0xf8));
        *(int *)(iVar2 + 0x360) = 1;
      }
      iVar2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      iVar1 = *(int *)(iVar2 + 0x154);
      if ((((iVar1 == 6) || (iVar1 == 0x10)) || ((iVar1 == 0x1a || (DAT_004949b0 == 1)))) &&
         (local_24 == 1)) {
        fcos((double)*(double *)(iVar2 + 0xf8));
        fsin((double)*(double *)(iVar2 + 0xf8));
        *(int *)(iVar2 + 0x360) = 1;
      }
      iVar2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      iVar1 = *(int *)(iVar2 + 0x158);
      if ((((iVar1 == 6) || (iVar1 == 0x10)) || ((iVar1 == 0x1a || (DAT_004949b0 == 1)))) &&
         (local_24 == 2)) {
        fcos((double)*(double *)(iVar2 + 0xf8));
        fsin((double)*(double *)(iVar2 + 0xf8));
        *(int *)(iVar2 + 0x360) = 1;
      }
      iVar2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      iVar1 = *(int *)(iVar2 + 0x15c);
      if (((((iVar1 == 6) || (iVar1 == 0x10)) || (iVar1 == 0x1a)) || (DAT_004949b0 == 1)) &&
         (local_24 == 3)) {
        fcos((double)*(double *)(iVar2 + 0xf8));
        fsin((double)*(double *)(iVar2 + 0xf8));
        *(int *)(iVar2 + 0x360) = 1;
      }
      if (*(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) == 1) {
        iVar2 = 2;
        lVar3 = __ftol();
        do {
          Math_RandomFloat0To1();
          Math_RandomFloat0To1();
          g_ActiveParticle.type = 8;
          lVar4 = __ftol();
          g_ActiveParticle.pos_x = (int)lVar4;
          lVar4 = __ftol();
          g_ActiveParticle.pos_y = (int)lVar4;
          lVar4 = __ftol();
          g_ActiveParticle.pos_z = (int)lVar4;
          lVar4 = __ftol();
          g_ActiveParticle.vel_x = (int)lVar4;
          g_ActiveParticle.vel_y = (int)lVar3;
          lVar4 = __ftol();
          g_ActiveParticle.vel_z = (int)lVar4;
          g_ActiveParticle.rot_x = 0;
          g_ActiveParticle.rot_y = 0;
          g_ActiveParticle.rot_z = 0;
          g_ActiveParticle.drag = 0x3d7;
          g_ActiveParticle.gravity = 0xffffff67;
          g_ActiveParticle.field_24 = 0x1ec00;
          g_ActiveParticle.field_28 = 0xffffffe0;
          g_ActiveParticle.field_2c = 1;
          Math_RandomFloat0To1();
          lVar4 = __ftol();
          g_ActiveParticle.field_3c = (int)lVar4;
          Math_RandomFloat0To1();
          lVar4 = __ftol();
          g_ActiveParticle.field_40 = (int)lVar4;
          Math_RandomFloat0To1();
          lVar4 = __ftol();
          g_ActiveParticle.field_44 = (int)lVar4;
          g_ActiveParticle.field_48 = 0;
          g_ActiveParticle.field_4c = 0x3c;
          FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
          iVar2 = iVar2 + -1;
        } while (iVar2 != 0);
      }
      local_24 = local_24 + 1;
    } while (local_24 < 4);
  }
  return;
}

/**
 * @original FX_SpawnLandingDustPuffs (IGN_WIN.EXE @ 0x004338d0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnLandingDustPuffs(void) {
  int iVar1;
  int iVar2;
  double fVar3;
  long long lVar4;
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (*(int *)(g_Vehicles + 0x278 + g_ActiveVehicleIndex * 0x484c) == 1) {
    iVar2 = *(int *)(iVar1 + 0x158);
    if (((iVar2 < 0) || (0x27 < iVar2)) &&
       ((iVar2 = *(int *)(iVar1 + 0x15c), iVar2 < 0 || (0x27 < iVar2)))) {
      iVar2 = 0;
    }
    if (((int*)&(DAT_00497438))[iVar2] == '\x01') {
      fVar3 = (double)*(double *)(iVar1 + 0xf8) + (double)_DAT_0047a3c0;
      fcos(fVar3);
      fsin(fVar3);
      g_ActiveParticle.type = 1;
      lVar4 = __ftol();
      g_ActiveParticle.pos_x = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_y = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_z = (int)lVar4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_x = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_y = (int)lVar4;
      g_ActiveParticle.field_3c = 0;
      g_ActiveParticle.field_40 = 0;
      g_ActiveParticle.field_44 = 0;
      g_ActiveParticle.field_48 = 0;
      g_ActiveParticle.field_4c = 0;
      g_ActiveParticle.life = 0;
      g_ActiveParticle.field_54 = 0;
      g_ActiveParticle.rot_z = &DAT_00497ff8;
      g_ActiveParticle.field_58 = 0x28;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      fVar3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(fVar3);
      fsin(fVar3);
      g_ActiveParticle.type = 1;
      lVar4 = __ftol();
      g_ActiveParticle.pos_x = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_y = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_z = (int)lVar4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.field_28 = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_x = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_y = (int)lVar4;
      g_ActiveParticle.field_3c = 0;
      g_ActiveParticle.field_40 = 0;
      g_ActiveParticle.field_44 = 0;
      g_ActiveParticle.field_48 = 0;
      g_ActiveParticle.field_4c = 0;
      g_ActiveParticle.life = 0;
      g_ActiveParticle.rot_z = &DAT_00497ff8;
      g_ActiveParticle.field_58 = 0x28;
      g_ActiveParticle.field_54 = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      fVar3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(fVar3);
      fsin(fVar3);
      g_ActiveParticle.type = 1;
      lVar4 = __ftol();
      g_ActiveParticle.pos_x = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_y = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_z = (int)lVar4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_x = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_y = (int)lVar4;
      g_ActiveParticle.field_3c = 0;
      g_ActiveParticle.field_40 = 0;
      g_ActiveParticle.field_44 = 0;
      g_ActiveParticle.field_48 = 0;
      g_ActiveParticle.field_4c = 0;
      g_ActiveParticle.life = 0;
      g_ActiveParticle.rot_z = &DAT_00497ff8;
      g_ActiveParticle.field_58 = 0x28;
      g_ActiveParticle.field_54 = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      fVar3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(fVar3);
      fsin(fVar3);
      g_ActiveParticle.type = 1;
      lVar4 = __ftol();
      g_ActiveParticle.pos_x = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_y = (int)lVar4;
      lVar4 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_x = (int)lVar4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.vel_z = (int)lVar4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_x = (int)lVar4;
      Math_RandomFloat0To1();
      lVar4 = __ftol();
      g_ActiveParticle.rot_y = (int)lVar4;
      g_ActiveParticle.field_3c = 0;
      g_ActiveParticle.field_40 = 0;
      g_ActiveParticle.field_44 = 0;
      g_ActiveParticle.field_48 = 0;
      g_ActiveParticle.field_4c = 0;
      g_ActiveParticle.life = 0;
      g_ActiveParticle.rot_z = &DAT_00497ff8;
      g_ActiveParticle.field_58 = 0x28;
      g_ActiveParticle.field_54 = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    }
  }
  return;
}

/**
 * @original FX_SpawnTireSkidSmoke (IGN_WIN.EXE @ 0x00433f90, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnTireSkidSmoke(void) {
  int iVar1;
  double dVar2;
  int iVar3;
  double fVar4;
  long long lVar5;
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  iVar3 = *(int *)(iVar1 + 0x158);
  if (((iVar3 < 0) || (0x27 < iVar3)) &&
     ((iVar3 = *(int *)(iVar1 + 0x15c), iVar3 < 0 || (0x27 < iVar3)))) {
    iVar3 = 0;
  }
  if ((((iVar3 == 4) || (iVar3 == 0xe)) || (iVar3 == 0x18)) &&
     ((_DAT_0047a3e8 < *(double *)(iVar1 + 0x118) && (*(int *)(iVar1 + 0x270) == 0)))) {
    dVar2 = *(double *)(iVar1 + 0x118) * *(double *)(iVar1 + 0x118) * _DAT_0047a3f0;
    fVar4 = Math_RandomFloat0To1();
    if (fVar4 * (double)_DAT_0047a3f8 < (double)dVar2) {
      g_ActiveParticle.type = 1;
      lVar5 = __ftol();
      g_ActiveParticle.pos_x = (int)lVar5;
      lVar5 = __ftol();
      g_ActiveParticle.pos_y = (int)lVar5;
      lVar5 = __ftol();
      g_ActiveParticle.pos_z = (int)lVar5;
      g_ActiveParticle.vel_x = 0;
      Math_RandomFloat0To1();
      lVar5 = __ftol();
      g_ActiveParticle.vel_y = (int)lVar5;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x3f5;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_28 = 0x800;
      g_ActiveParticle.field_2c = 0x3d;
      g_ActiveParticle.gravity = 0;
      Math_RandomFloat0To1();
      lVar5 = __ftol();
      g_ActiveParticle.rot_x = (int)lVar5;
      Math_RandomFloat0To1();
      lVar5 = __ftol();
      g_ActiveParticle.rot_y = (int)lVar5;
      g_ActiveParticle.field_3c = 0;
      g_ActiveParticle.field_40 = 0;
      g_ActiveParticle.field_44 = 0;
      g_ActiveParticle.field_48 = 0;
      g_ActiveParticle.field_4c = 0;
      g_ActiveParticle.life = 0;
      g_ActiveParticle.rot_z = &DAT_00497ff8;
      g_ActiveParticle.field_58 = 100;
      g_ActiveParticle.field_54 = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    }
  }
  return;
}

/**
 * @original FX_UpdateWeatherGeometry (IGN_WIN.EXE @ 0x00434840, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateWeatherGeometry(void) {
  unsigned int uVar1;
  int iVar2;
  char *puVar3;
  int *piVar4;
  int *piVar5;
  int iVar6;
  unsigned int uVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  long long uVar11;
  long long lVar12;
  long long lVar13;
  long long lVar14;
  int local_18;
  int local_10;
  int local_c;
  if (((*(int *)(g_Vehicles + 0x528) != 1) || (_DAT_0054f980 < _DAT_0047a448)) ||
     ((g_HudEnabled != 1 && (g_HudEnabled != 2)))) {
    if ((g_HudEnabled == 1) || (g_HudEnabled == 2)) {
      local_18 = 0;
      if (0 < DAT_0054f98c) {
        do {
          if (local_18 == 0) {
            local_10 = g_MenuCursorPos;
            piVar4 = DAT_00563c34;
          }
          else {
            local_10 = 1;
            piVar4 = DAT_00563c30;
          }
          if ((double)(DAT_00563cf4 + 2) < _DAT_005285d8) {
            if (((((int*)&(DAT_00552f30))[local_18] == 1) && (_DAT_0054f980 < 0.0)) &&
               (iVar9 = piVar4[1], (double)iVar9 < _DAT_0047a458)) {
              iVar10 = iVar9 / 2;
              if ((DAT_0054f98c == 1) && (g_HudEnabled == 1)) {
                uVar11 = Palette_AdjustRGB(iVar9,iVar9 >> 0x1f,g_ActiveTrackPalette + 8);
                Gfx_FreeSurface((int)uVar11);
              }
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              iVar9 = (int)lVar12;
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              iVar6 = (int)lVar12;
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              iVar2 = (int)lVar12;
              if (g_HudEnabled == 1) {
                *piVar4 = *piVar4 + 4;
                piVar4[1] = piVar4[1] + 2;
                piVar4[iVar10 * 0xc + 2] = iVar9;
                piVar4[iVar10 * 0xc + 3] = -iVar6;
                piVar4[iVar10 * 0xc + 4] = iVar2;
                piVar4[iVar10 * 0xc + 5] = iVar9 + 4;
                piVar4[iVar10 * 0xc + 6] = 0x3c - iVar6;
                piVar4[iVar10 * 0xc + 7] = iVar2;
                piVar4[iVar10 * 0xc + 8] = iVar9;
                piVar4[iVar10 * 0xc + 9] = -800 - iVar6;
                piVar4[iVar10 * 0xc + 10] = iVar2;
                piVar4[iVar10 * 0xc + 0xb] = iVar9 + 4;
                piVar4[iVar10 * 0xc + 0xc] = -0x2e4 - iVar6;
                piVar4[iVar10 * 0xc + 0xd] = iVar2;
              }
              else if (g_HudEnabled == 2) {
                *piVar4 = *piVar4 + 2;
                piVar4[1] = piVar4[1] + 2;
                piVar4[iVar10 * 6 + 2] = iVar9;
                piVar4[iVar10 * 6 + 3] = -iVar6;
                piVar4[iVar10 * 6 + 4] = iVar2;
                piVar4[iVar10 * 6 + 5] = iVar9;
                piVar4[iVar10 * 6 + 6] = -800 - iVar6;
                piVar4[iVar10 * 6 + 7] = iVar2;
              }
              iVar9 = iVar10 * 2 + 2;
              iVar6 = 0;
              if (0 < iVar9) {
                piVar8 = piVar4 + (iVar10 + 1) * 6 + 2;
                piVar5 = piVar4 + (iVar10 + 1) * 0xc + 2;
                iVar10 = 0;
                do {
                  if (g_HudEnabled == 1) {
                    *piVar5 = 0xd;
                    piVar5[1] = iVar10;
                    piVar5[2] = iVar10 + 1;
                    piVar5[3] = 0x7700;
                    piVar5[4] = 0x7b00;
                    piVar5[5] = DAT_00527f74;
                  }
                  else if (g_HudEnabled == 2) {
                    *piVar8 = 7;
                    piVar8[1] = iVar6;
                    piVar8[2] = 0;
                    piVar8[3] = 0x7500;
                    piVar8[4] = 0x1f00;
                    piVar8[5] = 0x9400;
                    piVar8[6] = 0;
                    piVar8[7] = DAT_00552d80 * 100;
                    piVar8[8] = DAT_00552f5c * 100;
                  }
                  piVar8 = piVar8 + 9;
                  piVar5 = piVar5 + 6;
                  iVar10 = iVar10 + 2;
                  iVar6 = iVar6 + 1;
                } while (iVar6 < iVar9);
              }
            }
            if (((((int*)&(DAT_00552f30))[local_18] == 0) || (0.0 <= _DAT_0054f980)) && (0 < piVar4[1])) {
              if (g_HudEnabled == 1) {
                iVar9 = *piVar4 + -4;
LAB_00434c38:
                *piVar4 = iVar9;
                piVar4[1] = piVar4[1] + -2;
              }
              else if (g_HudEnabled == 2) {
                iVar9 = *piVar4 + -2;
                goto LAB_00434c38;
              }
              iVar9 = piVar4[1] / 2;
              if ((DAT_0054f98c == 1) && (g_HudEnabled == 1)) {
                uVar11 = Palette_AdjustRGB(piVar4,piVar4[1] >> 0x1f,g_ActiveTrackPalette + 8);
                Gfx_FreeSurface((int)uVar11);
              }
              iVar10 = 0;
              if (0 < iVar9 * 2) {
                piVar8 = piVar4 + iVar9 * 6 + 2;
                piVar5 = piVar4 + iVar9 * 0xc + 2;
                iVar6 = 0;
                do {
                  if (g_HudEnabled == 1) {
                    *piVar5 = 0xd;
                    piVar5[1] = iVar6;
                    piVar5[2] = iVar6 + 1;
                    piVar5[3] = 0x7700;
                    piVar5[4] = 0x7b00;
                    piVar5[5] = DAT_00527f74;
                  }
                  else if (g_HudEnabled == 2) {
                    *piVar8 = 7;
                    piVar8[1] = iVar10;
                    piVar8[2] = 0;
                    piVar8[3] = 0x7500;
                    piVar8[4] = 0x1f00;
                    piVar8[5] = 0x9400;
                    piVar8[6] = 0;
                    piVar8[7] = DAT_00552d80 * 100;
                    piVar8[8] = DAT_00552f5c * 100;
                  }
                  piVar8 = piVar8 + 9;
                  piVar5 = piVar5 + 6;
                  iVar6 = iVar6 + 2;
                  iVar10 = iVar10 + 1;
                } while (iVar10 < iVar9 * 2);
              }
            }
          }
          if ((g_HudEnabled == 1) && (0 < *piVar4)) {
            *(int *)(&DAT_00563d28 + local_18 * 4) = *(int *)(&DAT_00563d28 + local_18 * 4) + 5;
            *(int *)(&DAT_00563d38 + local_18 * 4) = *(int *)(&DAT_00563d38 + local_18 * 4) + -0x19;
            Math_RandomFloat0To1();
            lVar12 = __ftol();
            if (((int)lVar12 == 0) && (_DAT_0054f980 < 0.0)) {
              __ftol();
              __ftol();
              Audio_PlaySampleVol(1,0,1,0,0x10000,0,0);
            }
          }
          else if ((g_HudEnabled == 2) && (0 < *piVar4)) {
            *(double *)(&DAT_00525e30 + local_18 * 8) =
                 *(double *)(&DAT_00525e30 + local_18 * 8) + _DAT_0047a480;
            if (_DAT_0047a488 < *(double *)(&DAT_00525e30 + local_18 * 8)) {
              *(double *)(&DAT_00525e30 + local_18 * 8) =
                   *(double *)(&DAT_00525e30 + local_18 * 8) - _DAT_0047a488;
            }
            fsin((double)*(double *)(&DAT_00525e30 + local_18 * 8));
            lVar12 = __ftol();
            iVar9 = g_Vehicles;
            *(int *)(&DAT_00563d28 + local_18 * 4) = (int)lVar12;
            *(int *)(&DAT_00563d38 + local_18 * 4) = *(int *)(&DAT_00563d38 + local_18 * 4) + -5;
            fcos((double)*(double *)(iVar9 + 0x100 + local_10 * 0x484c));
            lVar12 = __ftol();
            fsin((double)*(double *)(iVar9 + local_10 * 0x484c + 0x100));
            lVar13 = __ftol();
            iVar9 = piVar4[1] / 2;
            if (0 < iVar9) {
              piVar4 = piVar4 + 2;
              do {
                iVar6 = *piVar4 + (int)lVar12;
                iVar10 = (int)lVar13 + piVar4[2];
                iVar9 = iVar9 + -1;
                piVar4[3] = iVar6;
                piVar4[5] = iVar10;
                piVar4[9] = iVar6;
                piVar4[0xb] = iVar10;
                piVar4 = piVar4 + 6;
              } while (iVar9 != 0);
            }
          }
          piVar4 = (int *)(&DAT_00563d38 + local_18 * 4);
          if (*piVar4 < -800) {
            *(int *)(&DAT_00563d28 + local_18 * 4) = 0;
            *piVar4 = *piVar4 + 800;
          }
          local_c = 0;
          iVar9 = local_18 * 0x120;
          do {
            iVar10 = 0;
            do {
              lVar12 = __ftol();
              lVar13 = __ftol();
              lVar14 = __ftol();
              *(int *)(DAT_00553030 + 4 + iVar9) =
                   *(int *)(&DAT_00563d28 + local_18 * 4) + (((int)lVar14 + iVar10) * 5 + -5) * 200;
              *(int *)(DAT_00553030 + 8 + iVar9) = *piVar4 + (int)lVar12;
              *(int *)(DAT_00553030 + 0xc + iVar9) = ((local_c + (int)lVar13) * 5 + -5) * 200;
              *(int *)(DAT_00553030 + 0x10 + iVar9) = 0;
              *(int *)(DAT_00553030 + 0x14 + iVar9) = 0;
              *(int *)(DAT_00553030 + 0x18 + iVar9) = 0;
              iVar6 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_00553030 + iVar9));
              if (iVar6 != 0) {
                Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_WEATHER_0049948c);
              }
              iVar9 = iVar9 + 0x20;
              iVar10 = iVar10 + 1;
            } while (iVar10 < 3);
            local_c = local_c + 1;
          } while (local_c < 3);
          puVar3 = Audio_GetVoice(((int*)&(DAT_00525e50))[local_18]);
          if (puVar3 != (char *)0x0) {
            lVar12 = __ftol();
            *(int *)(puVar3 + 0xc) = (int)lVar12;
          }
          if (((g_HudEnabled == 1) && (((int*)&(DAT_00552f30))[local_18] == 1)) && (DAT_00553084 == 1)) {
            Math_RandomFloat0To1();
            lVar12 = __ftol();
            if ((int)lVar12 == 0) {
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              local_18 = (int)lVar12;
              uVar1 = *(unsigned int *)(g_Vehicles + 0x364 + local_10 * 0x484c);
              uVar7 = (int)uVar1 >> 0x1f;
              iVar9 = ((uVar1 ^ uVar7) - uVar7) + local_18;
              if (iVar9 < DAT_005287f8) {
                iVar9 = *(int *)(g_pTrackRoadSequence + iVar9 * 0x18);
              }
              else {
                iVar9 = *(int *)(g_pTrackRoadSequence + local_18 * 0x18);
              }
              iVar9 = *(int *)(iVar9 * 0x14 + DAT_0054f9cc + 0x10);
              g_ActiveParticle.type = 6;
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              g_ActiveParticle.pos_x = (int)lVar12;
              g_ActiveParticle.pos_y = iVar9 * -0x400;
              Math_RandomFloat0To1();
              lVar12 = __ftol();
              g_ActiveParticle.pos_z = (int)lVar12;
              g_ActiveParticle.vel_z = 6;
              g_ActiveParticle.vel_y = 0;
              g_ActiveParticle.vel_x = local_10;
              FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
              __ftol();
              __ftol();
              Audio_PlaySampleVol(1,0,2,0,0x10000,0,0);
            }
          }
          local_18 = local_18 + 1;
        } while (local_18 < DAT_0054f98c);
      }
      if ((double)(DAT_00563cf4 + 2) < _DAT_005285d8) {
        lVar12 = __ftol();
        DAT_00563cf4 = (int)lVar12;
      }
    }
  }
  else if (((0 < DAT_00563c34[1]) || (0 < DAT_00563c30[1])) &&
          (((g_IsSplitScreen == 0 && (_DAT_0047a448 <= _DAT_0054f980)) ||
           (((g_IsSplitScreen == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)) &&
            (_DAT_0047a448 <= _DAT_0054f980)))))) {
    Gfx_FreeSurface(g_ActiveTrackPalette + 8);
    DAT_00563c34[1] = 0;
    DAT_00563c30[1] = 0;
  }
  if ((g_HudEnabled == 1) || (g_HudEnabled == 3)) {
    iVar9 = 0;
    iVar10 = 0;
    if (0 < DAT_0054f98c) {
      do {
        puVar3 = Audio_GetVoice(*(int *)((int)&DAT_00525e50 + iVar9));
        if (puVar3 != (char *)0x0) {
          if ((*(int *)((int)&DAT_00552f30 + iVar9) == 1) && (*(int *)(puVar3 + 0xc) < 30000)) {
            *(int *)(puVar3 + 0xc) = *(int *)(puVar3 + 0xc) + 0x9c4;
          }
          else if ((*(int *)((int)&DAT_00552f30 + iVar9) == 0) &&
                  ((0 < *(int *)(puVar3 + 0xc) &&
                   (iVar6 = *(int *)(puVar3 + 0xc) + -0x9c4, *(int *)(puVar3 + 0xc) = iVar6,
                   iVar6 < 0)))) {
            *(int *)(puVar3 + 0xc) = 0;
          }
        }
        iVar9 = iVar9 + 4;
        iVar10 = iVar10 + 1;
      } while (iVar10 < DAT_0054f98c);
    }
  }
  return;
}

/**
 * @original FX_FrameTick (IGN_WIN.EXE @ 0x00435350, fx.c)
 * @fidelity ADAPTED
 */
void FX_FrameTick(void) {
  int iVar1;
  double *pdVar2;
  double dVar3;
  int *piVar4;
  int *piVar5;
  double fVar6;
  g_ActiveVehicleIndex = 0;
  if (0 < g_NumRacers) {
    do {
      dVar3 = *(double *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) * g_Const_DegToRad;
      iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
        *(double *)(iVar1 + 0x584) =
             ((dVar3 + *(double *)(iVar1 + 0xb8)) * g_Const_0_05 - *(double *)(iVar1 + 0x584)) *
             g_Const_0_1 + *(double *)(iVar1 + 0x584);
        iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
        fVar6 = (double)Math_AngleMod(g_ActiveVehicleIndex * 0x909);
        dVar3 = (double)fVar6;
        if ((double)g_Const_Pi < fVar6) {
          do {
            dVar3 = dVar3 - g_Const_TwoPi;
          } while (g_Const_Pi < dVar3);
        }
        for (; dVar3 < g_Const_NegPi; dVar3 = dVar3 + g_Const_TwoPi) {
        }
        *(double *)(iVar1 + 0x58c) = dVar3 * g_Const_0_1 + *(double *)(iVar1 + 0x58c);
      }
      else {
        *(double *)(iVar1 + 0x584) = dVar3 * g_Const_0_05;
        *(double *)(g_Vehicles + 0x58c + g_ActiveVehicleIndex * 0x484c) =
             *(double *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) * g_Const_DegToRad *
             g_Const_0_05;
      }
      FX_SpawnWaterSplashes();
      pdVar2 = (double *)(g_Vehicles + 0x60c + g_ActiveVehicleIndex * 0x484c);
      if ((0.0 < *(double *)(g_Vehicles + 0x60c + g_ActiveVehicleIndex * 0x484c)) &&
         (*pdVar2 < g_Const_0_05)) {
        *pdVar2 = *pdVar2 + _DAT_0047a510;
      }
      g_ActiveVehicleIndex = g_ActiveVehicleIndex + 1;
    } while (g_ActiveVehicleIndex < g_NumRacers);
  }
  FX_UpdateWeatherGeometry();
  if ((((g_IsSplitScreen == 0) && (*(int *)(g_Vehicles + 0x528) == 1)) ||
      ((g_IsSplitScreen == 1 &&
       ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))) &&
     ((_DAT_0047a518 <= _DAT_0054f980 &&
      ((DAT_00563c60 == 0 && (DAT_005db038 = DAT_005db038 + 10, 0x13f < DAT_005db038)))))) {
    DAT_005db038 = 0x13f;
    if (g_GameMode == 3) {
      if (DAT_00553280 + -1 <= 5 - DAT_0054f9d0) goto LAB_00435673;
      DAT_0054f9d0 = DAT_0054f9d0 + -1;
    }
    else {
      if (DAT_0054f9d0 < DAT_00553280 + -1) {
        DAT_005db038 = 0;
        DAT_0054f9d0 = DAT_0054f9d0 + 1;
      }
      if (((_DAT_00553090 <= (double)(DAT_00525e44 / 100)) || (1 < DAT_0054f9d0)) ||
         (((*(int *)(g_Vehicles + 0x528) != 0 || (DAT_00553280 != 0)) &&
          ((*(int *)(g_Vehicles + 0x528) != 1 || (DAT_00553280 != 1)))))) goto LAB_00435673;
      DAT_0054f9d0 = DAT_0054f9d0 + 1;
    }
    DAT_005db038 = 0;
  }
LAB_00435673:
  piVar4 = &DAT_006192a0;
  do {
    piVar5 = piVar4 + 1;
    *piVar4 = *piVar4 + 1;
    piVar4[10] = piVar4[10] + 1;
    piVar4 = piVar5;
  } while (piVar5 < &DAT_006192c8);
  if ((g_PlayerCarChoice == 7) && (_DAT_00552f68 < _DAT_0047a520)) {
    _DAT_00552f68 = _DAT_00552f68 + _DAT_0047a528;
  }
  return;
}

/**
 * @original Pos_InitAnimatedObjects (IGN_WIN.EXE @ 0x004356d0, fx.c)
 * @fidelity ADAPTED
 */
void Pos_InitAnimatedObjects(void) {
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  iVar7 = 0;
  if (0 < *DAT_0054f9cc) {
    do {
      iVar3 = *(int *)(DAT_00553064 + iVar7 * 4);
      if (iVar3 != -1) {
        piVar2 = (int *)(DAT_00553064 + (iVar3 + iVar7) * 4);
        iVar3 = piVar2[2];
        iVar4 = piVar2[3];
        iVar5 = piVar2[4];
        piVar1 = piVar2 + 2;
        iVar8 = 0;
        if (*piVar2 != 1 && -1 < *piVar2 + -1) {
          piVar6 = piVar1;
          piVar9 = piVar2 + 8;
          do {
            iVar8 = iVar8 + 1;
            *piVar6 = *piVar6 - *piVar9;
            piVar6[1] = piVar6[1] - piVar9[1];
            piVar6[2] = piVar6[2] - piVar9[2];
            piVar6 = piVar6 + 6;
            piVar9 = piVar9 + 6;
          } while (iVar8 < *piVar2 + -1);
        }
        piVar2 = piVar1 + iVar8 * 6;
        *piVar2 = piVar1[iVar8 * 6] - iVar3;
        piVar2[1] = piVar2[1] - iVar4;
        piVar2[2] = piVar2[2] - iVar5;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 < *DAT_0054f9cc);
  }
  return;
}

/**
 * @original Pos_UpdateAnimatedObjects (IGN_WIN.EXE @ 0x004357a0, fx.c)
 * @fidelity ADAPTED
 */
void Pos_UpdateAnimatedObjects(void) {
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  iVar5 = 0;
  iVar6 = 0;
  if (0 < *DAT_0054f9cc) {
    iVar4 = 0;
    do {
      if ((*(int *)(DAT_00553064 + iVar5) != -1) && (*(int *)(DAT_0054f908 + 0x1c + iVar4) == 1)) {
        iVar3 = *(int *)(DAT_00553064 + iVar5) + iVar6;
        iVar1 = DAT_00553064 + iVar3 * 4;
        if (*(int *)(DAT_00553064 + 4 + iVar3 * 4) == *(int *)(DAT_00553064 + iVar3 * 4)) {
          *(int *)(iVar1 + 4) = 0;
        }
        iVar3 = iVar1 + 8 + *(int *)(iVar1 + 4) * 0x18;
        piVar2 = (int *)(DAT_0054f908 + 4 + iVar4);
        *piVar2 = *piVar2 - *(int *)(iVar1 + 8 + *(int *)(iVar1 + 4) * 0x18);
        piVar2 = (int *)(DAT_0054f908 + 8 + iVar4);
        *piVar2 = *piVar2 - *(int *)(iVar3 + 4);
        piVar2 = (int *)(DAT_0054f908 + 0xc + iVar4);
        *piVar2 = *piVar2 - *(int *)(iVar3 + 8);
        *(int *)(DAT_0054f908 + 0x10 + iVar4) = *(int *)(iVar3 + 0xc);
        *(int *)(DAT_0054f908 + 0x14 + iVar4) = *(int *)(iVar3 + 0x10);
        *(int *)(DAT_0054f908 + 0x18 + iVar4) = *(int *)(iVar3 + 0x14);
        iVar3 = *(int *)(DAT_0054f908 + 0x10 + iVar4);
        if (0xe0f < iVar3) {
          *(int *)(DAT_0054f908 + 0x10 + iVar4) = iVar3 + -0xe10;
        }
        iVar3 = *(int *)(DAT_0054f908 + 0x14 + iVar4);
        if (0xe0f < iVar3) {
          *(int *)(DAT_0054f908 + 0x14 + iVar4) = iVar3 + -0xe10;
        }
        iVar3 = *(int *)(DAT_0054f908 + 0x18 + iVar4);
        if (0xe0f < iVar3) {
          *(int *)(DAT_0054f908 + 0x18 + iVar4) = iVar3 + -0xe10;
        }
        iVar3 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_0054f908 + iVar4));
        if (iVar3 != 0) {
          Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_ANIM_OBJ_004994ac);
        }
        *(int *)(iVar1 + 4) = *(int *)(iVar1 + 4) + 1;
      }
      iVar6 = iVar6 + 1;
      iVar4 = iVar4 + 0x20;
      iVar5 = iVar5 + 4;
    } while (iVar6 < *DAT_0054f9cc);
  }
  return;
}

/**
 * @original Math_LookupTrigAngle (IGN_WIN.EXE @ 0x004358e0, fx.c)
 * @fidelity ADAPTED
 */
double Math_LookupTrigAngle(double angle) {
    int index = (int)(angle * g_Const_200_0);
    return (double)*(int *)(&DAT_004944e0 + index * 4) * g_Const_0_005;
}

/**
 * @original Camera_UpdateOverview (IGN_WIN.EXE @ 0x00435910, fx.c)
 * @fidelity ADAPTED
 */
void Camera_UpdateOverview(void) {
  int iVar1;
  double *pdVar2;
  int iVar3;
  unsigned int uVar4;
  unsigned int uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  double dVar12;
  int iVar13;
  unsigned int uVar14;
  int *piVar15;
  int iVar16;
  int iVar17;
  int *piVar18;
  int iVar19;
  unsigned int *puVar20;
  int iVar21;
  int *piVar22;
  double fVar23;
  long long lVar24;
  int local_34;
  unsigned int local_2c;
  int *local_28;
  int local_24;
  int local_20;
  int uStack_1c;
  long long local_18;
  int local_10;
  int local_4;
  iVar16 = 0;
  local_2c = 0;
  local_10 = 0;
  local_4 = 0;
  uVar14 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c);
  iVar13 = (uVar14 ^ (int)uVar14 >> 0x1f) - ((int)uVar14 >> 0x1f);
  if (iVar13 + 2 < DAT_005287f8) {
    if ((int)uVar14 < 0) {
      local_18 = *(double *)(g_pTrackRoadSequence + iVar13 * 6 + 0x10) - _DAT_0047a540;
      piVar15 = g_pTrackRoadSequence + iVar13 * 6 + 0xf;
      local_24 = *piVar15;
      if (local_18 < _DAT_0047a548) {
        local_18 = local_18 + _DAT_0047a550;
      }
      if (local_24 == 10000) {
        local_18 = *(double *)(g_pTrackRoadSequence + uVar14 * 6 + 0xd) - _DAT_0047a540;
        local_24 = g_pTrackRoadSequence[uVar14 * 6 + 0xc];
        if (local_18 < _DAT_0047a548) {
          local_18 = local_18 + _DAT_0047a550;
        }
      }
      if (local_24 != -2) goto LAB_00435a9e;
      iVar19 = 0;
      do {
        piVar18 = piVar15 + 6;
        piVar15 = piVar15 + 6;
        iVar19 = iVar19 + 1;
      } while (*piVar18 == -2);
      local_18 = *(double *)(g_pTrackRoadSequence + (iVar19 + iVar13) * 6 + 0xd);
      local_24 = g_pTrackRoadSequence[(iVar19 + iVar13) * 6 + 0xc];
    }
    else {
      local_18 = *(double *)(g_pTrackRoadSequence + uVar14 * 6 + 0xd);
      local_24 = g_pTrackRoadSequence[uVar14 * 6 + 0xc];
    }
  }
  else {
    local_18 = *(double *)(g_pTrackRoadSequence + 1);
    local_24 = *g_pTrackRoadSequence;
  }
  local_18 = local_18 - _DAT_0047a540;
  if (local_18 < _DAT_0047a548) {
    local_18 = local_18 + _DAT_0047a550;
  }
LAB_00435a9e:
  iVar17 = local_24 * 0x14 + DAT_0054f9cc;
  iVar19 = *(int *)(iVar17 + 0x10);
  iVar13 = DAT_00525e60 + *(int *)(iVar17 + 4) * 4;
  local_34 = *(int *)(iVar13 + 4);
  puVar20 = (unsigned int *)(iVar13 + 8 + *(int *)(DAT_00525e60 + *(int *)(iVar17 + 4) * 4) * 0xc);
  if (0 < local_34) {
    do {
      uVar14 = puVar20[1];
      uVar4 = puVar20[2];
      uVar5 = puVar20[3];
      if ((*puVar20 & 0xffff0000) < 0x280000) {
        local_2c = local_2c +
                   *(int *)(iVar13 + 8 + uVar14 * 0xc) + (*(int *)(iVar17 + 0xc) + 0x6400) * 3 +
                   *(int *)(iVar13 + 8 + uVar4 * 0xc) + *(int *)(iVar13 + 8 + uVar5 * 0xc);
        iVar16 = (((((iVar16 - *(int *)(iVar13 + 0xc + uVar14 * 0xc)) + iVar19) -
                   *(int *)(iVar13 + 0xc + uVar4 * 0xc)) + iVar19) -
                 *(int *)(iVar13 + 0xc + uVar5 * 0xc)) + iVar19;
        local_10 = local_10 +
                   *(int *)(iVar13 + 0x10 + uVar14 * 0xc) + (*(int *)(iVar17 + 0x14) + 0x6400) * 3 +
                   *(int *)(iVar13 + 0x10 + uVar4 * 0xc) + *(int *)(iVar13 + 0x10 + uVar5 * 0xc);
        local_4 = local_4 + 3;
      }
      puVar20 = puVar20 + 0xb;
      local_34 = local_34 + -1;
    } while (local_34 != 0);
  }
  local_2c = (int)local_2c / local_4;
  local_10 = local_10 / local_4;
  uVar14 = iVar16 / local_4 + 0xfa;
  DAT_0063c5d8 = *(int *)(DAT_00552e40 + local_24 * 0xc);
  *(int *)(g_pActiveCamera + 7) = 0;
  *g_pActiveCamera = (double)(int)local_2c;
  dVar12 = (double)local_10;
  g_pActiveCamera[1] = (double)(iVar16 / local_4 + 0x2ee);
  uStack_1c = (int)((unsigned long long)dVar12 >> 0x20);
  local_20 = SUB84(dVar12,0);
  g_pActiveCamera[2] = dVar12;
  pdVar2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 3) = 0;
  *(int *)((int)pdVar2 + 0x1c) = 0x408a9000;
  pdVar2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 4) = 0;
  *(int *)((int)pdVar2 + 0x24) = 0;
  pdVar2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 5) = 0;
  *(int *)((int)pdVar2 + 0x2c) = 0;
  *(int *)((int)g_pActiveCamera + 0x7c) = 0;
  *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x58 + g_ActiveVehicleIndex * 200);
  *(int *)((int)g_pActiveCamera + 0x84) =
       *(int *)(g_VehicleConfigs + 0x5c + g_ActiveVehicleIndex * 200);
  lVar24 = __ftol();
  *(int *)(g_pActiveCamera + 0x11) = (int)lVar24;
  Lisa_CullObjectsOrthographic();
  piVar15 = Track_FindSurfaceHeight(local_2c,uVar14,local_10,-1,0x32);
  if (*piVar15 == -1) {
    local_2c = local_2c - 0x14;
    piVar15 = Track_FindSurfaceHeight(local_2c,uVar14,local_10,-1,0x32);
    if (*piVar15 == -1) {
      Log_DebugPrintf((const char *)&DAT_004994cc);
    }
  }
  iVar13 = 0;
  do {
    iVar16 = 0;
    do {
      piVar18 = piVar15 + iVar16;
      iVar19 = g_ActiveVehicleIndex * 0x1213 + iVar16;
      iVar16 = iVar16 + 1;
      *(int *)(g_Vehicles + 0x170 + (iVar19 + iVar13) * 4) = *piVar18;
    } while (iVar16 < 0x10);
    iVar13 = iVar13 + 0x10;
  } while (iVar13 < 0x40);
  iVar16 = piVar15[2];
  iVar6 = piVar15[6];
  iVar7 = piVar15[4];
  iVar8 = piVar15[9];
  iVar19 = piVar15[3];
  iVar9 = piVar15[7];
  iVar17 = piVar15[1];
  iVar10 = piVar15[8];
  iVar11 = piVar15[5];
  iVar3 = piVar15[5];
  iVar13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (*(int *)(iVar13 + 0x558) != 0) {
    *(int *)(iVar13 + 0x55c) = 0;
    if ((*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) != 2) || (g_ActiveVehicleIndex == 0)) {
      *(int *)(g_Vehicles + 0x560 + g_ActiveVehicleIndex * 0x484c) = 0;
    }
    lVar24 = Lisa_SetDynamicObjectMesh(*(int *)
                           (&DAT_005db000 + *(int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c) * 4),
                          g_PlayerHUDState,(int *)(g_ActiveVehicleIndex * 0x20 + DAT_005db02c),
                          (int *)*(int *)
                                  (&DAT_005db000 + *(int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c) * 4)
                          ,1,(short)g_ActiveVehicleIndex + 100,0,-1,0);
    if ((int)lVar24 != 0) {
      Log_DebugPrintf(s_Error_while_changing_car_mesh_00499248);
    }
    iVar13 = 0;
    local_28 = &DAT_005537d8 + g_ActiveVehicleIndex * 0x820;
    piVar15 = (int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c);
    do {
      iVar21 = 0;
      iVar1 = iVar13 + *piVar15 * 4;
      if (0 < *(int *)(DAT_00563da4 + iVar1 * 0xc)) {
        piVar18 = local_28;
        piVar22 = (int *)(*(int *)(&DAT_00552e60 + iVar1 * 4) + 8);
        do {
          iVar21 = iVar21 + 1;
          *piVar18 = *(int *)(&DAT_005533b0 + (iVar13 + *piVar15 * 4) * 4) + *piVar22;
          piVar18[1] = piVar22[1];
          piVar18[2] = *(int *)(&DAT_00553300 + (iVar13 + *piVar15 * 4) * 4) + piVar22[2];
          piVar18 = piVar18 + 3;
          piVar22 = piVar22 + 3;
        } while (iVar21 < *(int *)(DAT_00563da4 + (iVar13 + *piVar15 * 4) * 0xc));
      }
      iVar13 = iVar13 + 1;
      local_28 = local_28 + 0x208;
    } while (iVar13 < 4);
    *(int *)(g_Vehicles + 0x558 + g_ActiveVehicleIndex * 0x484c) = 0;
  }
  *(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c) = (double)(int)local_2c;
  *(double *)(g_Vehicles + 8 + g_ActiveVehicleIndex * 0x484c) = (double)(int)uVar14;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x14 + g_ActiveVehicleIndex * 0x484c) = uStack_1c;
  *(int *)(iVar1 + 0x10 + iVar13 * 0x484c) = local_20;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x18 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x1c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x20 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x24 + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x28 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x2c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x30 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x34 + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x38 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x3c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x40 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x44 + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x10c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c) = ((int*)&local_18)[1];
  *(int *)(iVar1 + 0xf8 + iVar13 * 0x484c) = (int)local_18;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x100 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x104 + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x114 + iVar13 * 0x484c) = 0;
  fVar23 = (double)fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  pdVar2 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  pdVar2[9] = (double)((double)*pdVar2 -
                      fVar23 * (double)*(int *)(g_Vehicles + 0x5b0 + g_ActiveVehicleIndex * 0x484c));
  fVar23 = (double)fsin((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  iVar13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(iVar13 + 0x50) =
       (double)((double)*(double *)(iVar13 + 0x10) -
               fVar23 * (double)*(int *)(g_Vehicles + 0x5b0 + g_ActiveVehicleIndex * 0x484c));
  fVar23 = (double)fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  pdVar2 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  pdVar2[0xd] = (double)(fVar23 * (double)*(int *)(g_Vehicles + 0x5ac + g_ActiveVehicleIndex * 0x484c) +
                        (double)*pdVar2);
  fVar23 = (double)fsin((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  iVar13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(iVar13 + 0x70) =
       (double)(fVar23 * (double)*(int *)(g_Vehicles + 0x5ac + g_ActiveVehicleIndex * 0x484c) +
               (double)*(double *)(iVar13 + 0x10));
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x58 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x5c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x60 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 100 + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x78 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x7c + iVar13 * 0x484c) = 0;
  iVar1 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x80 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar1 + 0x84 + iVar13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x150 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(int *)(g_Vehicles + 0x154 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(int *)(g_Vehicles + 0x158 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  dVar12 = -((double)iVar3 +
            ((double)(iVar10 - iVar11) * (double)iVar16 +
            (double)iVar17 * ((double)(iVar9 - iVar7) - (double)(int)(local_2c - iVar7)) +
            (double)iVar19 * ((double)(iVar8 - iVar6) - (double)(local_10 - iVar6))) /
            (double)iVar16);
  *(int *)(g_Vehicles + 0x15c + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(double *)(g_Vehicles + 0x120 + g_ActiveVehicleIndex * 0x484c) = dVar12;
  *(double *)(g_Vehicles + 0x128 + g_ActiveVehicleIndex * 0x484c) = dVar12;
  *(int *)(g_Vehicles + 0x270 + g_ActiveVehicleIndex * 0x484c) = 1;
  *(int *)(g_Vehicles + 0x344 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x348 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x34c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x27c + g_ActiveVehicleIndex * 0x484c) = 1;
  *(int *)(g_PlayerHUDState + 0x10 + g_ActiveVehicleIndex * 0x4c) = 0;
  *(int *)(g_Vehicles + 0x350 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x298 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x29c + g_ActiveVehicleIndex * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0x11c + iVar13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x274 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x278 + g_ActiveVehicleIndex * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x288 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0x28c + iVar13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x2f8 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x344 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x348 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x34c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c) =
       *(int *)(DAT_00553784 + local_24 * 0xc);
  *(int *)(g_Vehicles + 0x368 + g_ActiveVehicleIndex * 0x484c) =
       *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c);
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x88 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0x8c + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x90 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0x94 + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x98 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0x9c + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xa0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0xa4 + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xb0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0xb4 + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xb8 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0xbc + iVar13 * 0x484c) = 0;
  iVar16 = g_Vehicles;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xc0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(iVar16 + 0xc4 + iVar13 * 0x484c) = 0;
  iVar13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(iVar13 + 0x584) =
       (*(double *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a560 +
       *(double *)(iVar13 + 0xb8)) * _DAT_0047a568;
  *(double *)(g_Vehicles + 0x58c + g_ActiveVehicleIndex * 0x484c) =
       (*(double *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a560 +
       *(double *)(g_Vehicles + 0xc0 + g_ActiveVehicleIndex * 0x484c)) * _DAT_0047a568;
  iVar16 = g_VehicleConfigs;
  iVar13 = g_ActiveVehicleIndex;
  *(int *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) = 0;
  *(int *)(iVar16 + 0xc4 + iVar13 * 200) = 0;
  iVar19 = g_Vehicles;
  iVar16 = DAT_00563d54;
  iVar13 = g_ActiveVehicleIndex;
  iVar17 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar17 + 0x2c + DAT_00563d54) =
       *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
  *(int *)(iVar17 + 0x28 + iVar16) = *(int *)(iVar19 + 0xf8 + iVar13 * 0x484c);
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x30 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x34 + iVar13) = 0;
  *(double *)(g_ActiveVehicleIndex * 0x1d0 + 0x68 + DAT_00563d54) =
       *(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a570;
  *(double *)(g_ActiveVehicleIndex * 0x1d0 + 0x70 + DAT_00563d54) =
       *(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a570;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x78 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x7c + iVar13) = 0;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x80 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x84 + iVar13) = 0;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x88 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x8c + iVar13) = 0;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x90 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x94 + iVar13) = 0;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0x98 + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0x9c + iVar13) = 0;
  iVar13 = DAT_00563d54;
  iVar16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(iVar16 + 0xac + DAT_00563d54) = 0;
  *(int *)(iVar16 + 0xb0 + iVar13) = 0;
  *(int *)(g_ActiveVehicleIndex * 0x1d0 + 0xd4 + DAT_00563d54) = 0;
  Car_UpdateDynamicObjects(g_ActiveVehicleIndex);
  return;
}

/**
 * @original Lisa_FlushRasterizerCommands (IGN_WIN.EXE @ 0x00438030, fx.c)
 * @fidelity ADAPTED
 */

/**
 * @original Race_FindFocusedVehicle (IGN_WIN.EXE @ 0x00438050, fx.c)
 * @fidelity ADAPTED
 */
int Race_FindFocusedVehicle(void) {
  int iVar1;
  int *piVar2;
  int iVar3;
  int local_4;
  if (g_GameMode == 3) {
    iVar1 = 1;
    iVar3 = 0;
    if (0 < g_NumRacers) {
      piVar2 = (int *)(g_Vehicles + 0x39c);
      do {
        if ((piVar2[99] == 0) && (iVar1 < *piVar2)) {
          iVar1 = *piVar2;
          local_4 = iVar3;
        }
        piVar2 = piVar2 + 0x1213;
        iVar3 = iVar3 + 1;
      } while (iVar3 < g_NumRacers);
    }
    if (iVar1 == 1) {
      iVar1 = g_NumRacers + 1;
    }
    if (local_4 == g_MenuCursorPos) {
      DAT_004949a8 = 0;
    }
    else {
      DAT_004949a8 = DAT_004949a8 + 1;
      if (0x14 < DAT_004949a8) {
        g_MenuCursorPos = local_4;
        return iVar1;
      }
    }
  }
  else {
    iVar3 = 0;
    iVar1 = g_NumRacers + 1;
    if (0 < g_NumRacers) {
      piVar2 = (int *)(g_Vehicles + 0x39c);
      do {
        if ((piVar2[99] == 0) && (*piVar2 < iVar1)) {
          iVar1 = *piVar2;
          local_4 = iVar3;
        }
        piVar2 = piVar2 + 0x1213;
        iVar3 = iVar3 + 1;
      } while (iVar3 < g_NumRacers);
    }
    if (local_4 == g_MenuCursorPos) {
      DAT_004949a8 = 0;
      return iVar1;
    }
    DAT_004949a8 = DAT_004949a8 + 1;
    if (0x14 < DAT_004949a8) {
      g_MenuCursorPos = local_4;
      return iVar1;
    }
  }
  return iVar1;
}

/**
 * @original Lisa_RenderPanorama (IGN_WIN.EXE @ 0x00438210, fx.c)
 * @fidelity ADAPTED
 */

/**
 * @original HUD_RenderPauseMenu (IGN_WIN.EXE @ 0x004383b0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderPauseMenu(void) {
  int iVar1;
  int iVar2;
  char *extraout_ECX;
  char *pcVar3;
  char *extraout_EDX;
  int iVar4;
  int *puVar5;
  int *puVar6;
  int bVar7;
  long long uVar8;
  int iVar9;
  char *pcVar10;
  char *pcVar11;
  char local_18 [12];
  int local_c [3];
  puVar5 = (int *)(g_RenderTargetSurface + 0x25800);
  Gfx_SetRenderTarget(puVar5,0x140,0x140,0x1e0,8);
  DAT_0063f2d8 = DAT_00525e64;
  puVar6 = puVar5;
  for (iVar2 = 8000; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  if ((((DAT_00552f10 == 2) || (DAT_00552f10 == 3)) && (DAT_00563da0 != 1)) &&
     (((DAT_005daff0 != 1 && (0 < DAT_00552f28)) && (g_PlayerCarChoice < 6)))) {
    iVar4 = 1;
    iVar2 = 0;
  }
  else {
    iVar4 = 4;
    iVar2 = 4;
  }
  uVar8 = Font_GetTextWidth((const char *)(s_CONTINUE_00494d58 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,0,(int)uVar8,0,0);
  uVar8 = Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,1,(int)uVar8,0,0);
  uVar8 = Font_GetTextWidth((const char *)(&DAT_00494d94 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,2,(int)uVar8,0,0);
  uVar8 = Font_GetTextWidth((const char *)(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2), g_FontId_Medium);
  iVar1 = (int)uVar8;
  Menu_AddLayoutItem(0x39,3,iVar1,0,0);
  Menu_LayoutItems(puVar5,0x140);
  Menu_ClearLayout();
  if (DAT_00639497 == 0) {
    pcVar11 = (char *)(g_LanguageId * 0x69);
    pcVar3 = s_DEFAULT_00494dee + g_LanguageId * 0xd2;
    pcVar10 = pcVar3;
  }
  else if (DAT_00639497 == -1) {
    pcVar3 = (char *)(g_LanguageId * 0x15);
    pcVar11 = s_RANDOM_00494e0c + g_LanguageId * 0xd2;
    pcVar10 = pcVar11;
  }
  else {
    pcVar10 = &DAT_00499504;
    pcVar3 = extraout_ECX;
    pcVar11 = extraout_EDX;
  }
  uVar8 = Font_GetTextWidth((const char *)(pcVar10), DAT_00552fdc);
  Menu_AddLayoutItem(iVar1 + 0x57,3,(int)uVar8,0,1);
  Menu_LayoutItems(puVar5,0x140);
  Menu_ClearLayout();
  iVar9 = g_FontId_Large;
  if (DAT_00563c04 == 0) {
    iVar9 = g_FontId_Medium;
  }
  Font_DrawText(s_CONTINUE_00494d58 + g_LanguageId * 0xd2,iVar9,0x46,0xc);
  if (DAT_00563c04 == 1) {
    iVar2 = ((int*)&(g_FontId_Large))[iVar4];
  }
  else {
    iVar2 = ((int*)&(g_FontId_Large))[iVar2];
  }
  Font_DrawText(s_RESTART_00494d76 + g_LanguageId * 0xd2,iVar2,0x46,0x1f);
  iVar2 = g_FontId_Large;
  if (DAT_00563c04 == 2) {
    iVar2 = g_FontId_Medium;
  }
  Font_DrawText((const char *)(&DAT_00494d94 + g_LanguageId * 0xd2),iVar2,0x46,0x32);
  iVar2 = g_FontId_Large;
  if (DAT_00563c04 == 3) {
    iVar2 = g_FontId_Medium;
  }
  Font_DrawText(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2,iVar2,0x46,0x45);
  if (DAT_00639497 == 0) {
    pcVar11 = &DAT_004920e8;
    pcVar3 = s_DEFAULT_00494dee + g_LanguageId * 0xd2;
  }
  else if (DAT_00639497 == -1) {
    pcVar11 = &DAT_004920e8;
    pcVar3 = s_RANDOM_00494e0c + g_LanguageId * 0xd2;
  }
  else {
    pcVar11 = &DAT_00493c78;
    pcVar3 = DAT_00552e54;
  }
  _sprintf(local_18,pcVar11,pcVar3);
  ((int*)&(g_FontAlignMode))[DAT_00552fdc * 400] = 1;
  Font_DrawText(local_18,DAT_00552fdc,(int)uVar8 / 2 + iVar1 + 100,0x45);
  iVar2 = 0;
  bVar7 = g_GameMode == 0;
  ((int*)&(g_FontAlignMode))[DAT_00552fdc * 400] = 0;
  if (bVar7) {
    uVar8 = Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
    if (0 < DAT_00552f28) {
      iVar4 = ((int)uVar8 + 0x67) * 0x100;
      do {
        iVar2 = iVar2 + 1;
        local_c[1] = 0x1b00;
        local_c[0] = iVar4;
        Gfx_DrawSprite(DAT_005287e8,local_c,0);
        iVar4 = iVar4 + 0x1a00;
      } while (iVar2 < DAT_00552f28);
    }
  }
  Gfx_BlitTransparentLUT((int)puVar5,0,0,0x140,100,0x563db0,g_ScreenWidth / 2 + -0xa0,g_ScreenHeight / 2 + -0x32,
               g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
  return;
}

/**
 * @original HUD_RenderConfirmationPrompt (IGN_WIN.EXE @ 0x00438880, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderConfirmationPrompt(void) {
    int *prompt_surface;
    int *clear_ptr;
    const char *prompt_text;
    int font_id;
    int text_width;
    int screen_w;
    int clear_count;

    prompt_surface = (int *)(g_RenderTargetSurface + 0x25800);
    Gfx_SetRenderTarget(prompt_surface, 0x140, 0x140, 0x1e0, 8);
    DAT_0063f2d8 = DAT_00525e64;

    clear_ptr = prompt_surface;
    for (clear_count = 8000; clear_count != 0; clear_count--) {
        *clear_ptr++ = 0;
    }

    if (DAT_00601674 == 2) {
        text_width = (int)Font_GetTextWidth((const char *)(s_RESTART___Y_N__00495300 + g_LanguageId * 0x1e), g_FontId_Medium);
    } else if (DAT_00601674 == 1) {
        text_width = (int)Font_GetTextWidth((const char *)(s_QUIT___Y_N__00495248 + g_LanguageId * 0x1e), g_FontId_Medium);
    } else {
        text_width = 0;
    }

    Menu_AddLayoutItem(0x94 - text_width / 2, 0, text_width, 0, 0);
    Menu_LayoutItems(prompt_surface, 0x140);
    Menu_ClearLayout();

    font_id = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    if (DAT_00601674 == 2) {
        prompt_text = s_RESTART___Y_N__00495300 + g_LanguageId * 0x1e;
        Font_DrawText(prompt_text, font_id, 0xa0, 0xc);
    } else if (DAT_00601674 == 1) {
        prompt_text = s_QUIT___Y_N__00495248 + g_LanguageId * 0x1e;
        Font_DrawText(prompt_text, font_id, 0xa0, 0xc);
    }

    screen_w = g_ScreenWidth;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    Gfx_BlitTransparentLUT((int)prompt_surface, 0, 0, 0x140, 0x22, 0x563db0,
                           screen_w / 2 - 0xa0, g_ScreenHeight / 2 - 0xf,
                           g_pLisaDrawCommandWritePtr, 0x140, screen_w);
    Gfx_SetRenderTarget(&g_VirtualFramebuffer, g_ScreenWidth, g_ScreenWidth, g_ScreenHeight, 8);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
}

/**
 * @original HUD_RenderTrackResults (IGN_WIN.EXE @ 0x00438a60, fx.c)
 * @fidelity ADAPTED
 */
int HUD_RenderTrackResults(void) {
  int uVar1;
  int iVar2;
  int *piVar3;
  char *pcVar4;
  int *puVar5;
  int uVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int extraout_EDX;
  int extraout_EDX_00;
  int extraout_EDX_01;
  int extraout_EDX_02;
  int extraout_EDX_03;
  unsigned int uVar10;
  int iVar11;
  int iVar12;
  int *puVar13;
  unsigned int uVar14;
  int bVar15;
  long long uVar16;
  long long lVar17;
  int iVar18;
  int iVar19;
  int iVar20;
  int iVar21;
  char *puVar22;
  int local_dc;
  int local_d8;
  int local_d0 [9];
  char local_ac [4];
  int local_a8;
  int local_a4 [12];
  int aiStack_74 [9];
  char local_50 [40];
  char local_28 [40];
  if (DAT_00527f28 != 0) {
    DAT_00527f28 = 0;
    Video_SetGraphicsMode();
  }
  iVar2 = 0;
  if (0 < g_NumRacers) {
    piVar3 = g_PlayerHUDState + 1;
    piVar9 = (int *)(g_Vehicles + 0x528);
    do {
      if ((*piVar9 == 0) && (*piVar3 == 0)) {
        return 0;
      }
      piVar3 = piVar3 + 0x13;
      piVar9 = piVar9 + 0x1213;
      iVar2 = iVar2 + 1;
    } while (iVar2 < g_NumRacers);
  }
  piVar3 = local_d0;
  for (iVar2 = 8; piVar3 = piVar3 + 1, iVar2 != 0; iVar2 = iVar2 + -1) {
    *piVar3 = 100;
  }
  iVar12 = 0;
  Gfx_SetClipRect(0,0,g_ScreenWidth,g_ScreenHeight);
  iVar2 = extraout_EDX;
  if (g_PlayerCarChoice == 0) {
    g_PlayerCarChoice = 1;
    if (0 < DAT_00553280) {
      do {
        iVar2 = 0;
        if (0 < g_NumRacers) {
          piVar3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*piVar3 - iVar12 == 1) break;
            piVar3 = piVar3 + 0x1213;
            iVar2 = iVar2 + 1;
          } while (iVar2 < g_NumRacers);
          if (iVar2 < g_NumRacers) {
            local_d0[iVar12 + 1] = iVar2;
          }
        }
        iVar12 = iVar12 + 1;
      } while (iVar12 < DAT_00553280);
    }
    uVar16 = Font_GetTextWidth((const char *)(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e), g_FontId_Medium);
    iVar2 = 0;
    do {
      iVar2 = iVar2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + iVar2) = 0;
    } while (iVar2 < 0x25800);
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    DAT_0063f2d8 = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)uVar16 / 2,0,(int)uVar16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    iVar2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e,iVar2,0xa0,0xd);
    bVar15 = DAT_00552f10 == 4;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    iVar2 = g_FontId_Small;
    if (bVar15) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,iVar2,0xa0,0x19c);
      iVar2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
      iVar2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      pcVar4 = s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e;
LAB_00438edb:
      Font_DrawText(pcVar4,iVar2,0x140,0xd3);
      iVar2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(iVar2,0x140,0x140,0x1e0,8);
    }
    else {
      if (g_GameMode == 0) {
        Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32), g_FontId_Small);
        iVar2 = g_FontId_Small;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,iVar2,0xa0,0x19c);
        iVar2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
        iVar12 = g_LanguageId;
        iVar2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        pcVar4 = s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar12 * 0x32;
        goto LAB_00438edb;
      }
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d,iVar2,0xa0,0x19c);
      iVar2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
      iVar12 = g_LanguageId;
      iVar2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + iVar12 * 0x2d,iVar2,0x140,0xd3);
      iVar2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(iVar2,0x140,0x140,0x1e0,8);
      DAT_005285c8 = 1;
    }
    iVar2 = 0;
    if (0 < g_NumRacers) {
      iVar11 = 0x28;
      puVar13 = &DAT_005286f4;
      iVar12 = 0x3000;
      do {
        local_d8 = iVar12;
        if (g_GameMode == 3) {
          local_dc = 0x7000;
          Gfx_DrawSprite(*puVar13,&local_dc,0);
          iVar21 = 0x28;
          iVar20 = 0x8b;
          iVar8 = iVar11;
LAB_00439056:
          Menu_AddLayoutItem(iVar20,0,iVar21,iVar8,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
        }
        else {
          local_dc = 0x3400;
          Gfx_DrawSprite(*puVar13,&local_dc,0);
          Menu_AddLayoutItem(0x4f,0,0x28,iVar11,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          Menu_AddLayoutItem(0x92,0,0x5f,iVar11,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          if (DAT_00553068 == 1) {
            local_d8 = iVar12 + 0x3c00;
            local_dc = 0x3400;
            Gfx_DrawSprite(puVar13[1],&local_dc,0);
            Menu_AddLayoutItem(0x4f,0,0x28,iVar11 + 0x3c,0);
            Menu_LayoutItems(g_RenderTargetSurface,0x140);
            Menu_ClearLayout();
            iVar21 = 0x5f;
            iVar20 = 0x92;
            iVar8 = iVar11 + 0x3c;
            goto LAB_00439056;
          }
        }
        iVar11 = iVar11 + 0x3c;
        puVar13 = puVar13 + 1;
        iVar12 = iVar12 + 0x3c00;
        iVar2 = iVar2 + 1;
      } while (iVar2 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
    iVar2 = extraout_EDX_00;
  }
  if (g_PlayerCarChoice == 2) {
    iVar12 = 0;
    g_PlayerCarChoice = 3;
    if (0 < DAT_00553280) {
      do {
        iVar2 = 0;
        if (0 < g_NumRacers) {
          piVar3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*piVar3 - iVar12 == 1) break;
            piVar3 = piVar3 + 0x1213;
            iVar2 = iVar2 + 1;
          } while (iVar2 < g_NumRacers);
          if (iVar2 < g_NumRacers) {
            local_d0[iVar12 + 1] = iVar2;
          }
        }
        iVar12 = iVar12 + 1;
        iVar2 = DAT_00553280;
      } while (iVar12 < DAT_00553280);
    }
    uVar16 = Font_GetTextWidth((const char *)(s_TRACK_SCORE_00495528 + g_LanguageId * 0x1e), g_FontId_Medium);
    iVar2 = 0;
    do {
      iVar2 = iVar2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + iVar2) = 0;
    } while (iVar2 < 0x25800);
    iVar11 = 0;
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    DAT_0063f2d8 = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)uVar16 / 2,0,(int)uVar16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    iVar2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TRACK_SCORE_00495528 + g_LanguageId * 0x1e,iVar2,0xa0,0xd);
    iVar12 = g_LanguageId;
    iVar2 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar12 * 0x32), iVar2);
    iVar12 = g_LanguageId;
    iVar2 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar12 * 0x32,iVar2,0xa0,0x19c);
    iVar2 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
    iVar12 = g_LanguageId;
    iVar2 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar12 * 0x32,iVar2,0x140,0xd3);
    iVar2 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
    Gfx_SetRenderTarget(iVar2,0x140,0x140,0x1e0,8);
    if (0 < g_NumRacers) {
      puVar13 = &DAT_005286f4;
      iVar2 = 0x28;
      iVar12 = 0x3000;
      do {
        uVar6 = *puVar13;
        puVar13 = puVar13 + 1;
        iVar11 = iVar11 + 1;
        local_dc = 0x5600;
        local_d8 = iVar12;
        Gfx_DrawSprite(uVar6,&local_dc,0);
        Menu_AddLayoutItem(0x73,0,0x28,iVar2,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0xb7,0,0x16,iVar2,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        iVar2 = iVar2 + 0x3c;
        iVar12 = iVar12 + 0x3c00;
      } while (iVar11 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
    iVar2 = extraout_EDX_01;
  }
  if (g_PlayerCarChoice == 4) {
    iVar12 = 0;
    g_PlayerCarChoice = 5;
    if (0 < DAT_00553280) {
      iVar2 = 0;
      do {
        iVar11 = 0;
        if (0 < g_NumRacers) {
          piVar3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*piVar3 - iVar12 == 1) break;
            piVar3 = piVar3 + 0x1213;
            iVar11 = iVar11 + 1;
          } while (iVar11 < g_NumRacers);
          if (iVar11 < g_NumRacers) {
            local_d0[iVar12 + 1] = iVar11;
          }
        }
        iVar12 = iVar12 + 1;
      } while (iVar12 < DAT_00553280);
    }
    uVar16 = Font_GetTextWidth((const char *)(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e), g_FontId_Medium);
    iVar2 = 0;
    do {
      iVar2 = iVar2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + iVar2) = 0;
    } while (iVar2 < 0x25800);
    bVar15 = 0;
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    DAT_0063f2d8 = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)uVar16 / 2,0,(int)uVar16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    iVar2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e,iVar2,0xa0,0xd);
    iVar12 = g_NumRacers;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    iVar2 = g_FontId_Small;
    if (0 < iVar12) {
      do {
        if ((*(int *)(g_Vehicles + 0x3a0) < 4) ||
           ((*(int *)(g_Vehicles + 0x4bec) < 4 && (g_IsSplitScreen == 1)))) {
          bVar15 = 1;
        }
        iVar12 = iVar12 + -1;
      } while (iVar12 != 0);
    }
    if (bVar15) {
      if ((((g_DifficultyLevel == 4) && (DAT_006393ac == 0)) ||
          ((g_DifficultyLevel == 5 && (DAT_006393ac == 1)))) ||
         ((g_DifficultyLevel == 6 && ((DAT_006393ac == 2 || (DAT_006393ac == 3)))))) {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70 + g_LanguageId * 100,iVar2,0xa0,
                     0x19c);
        iVar2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
        iVar2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70 + g_LanguageId * 100,iVar2,0x140,
                     0xd3);
        iVar2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
        Gfx_SetRenderTarget(iVar2,0x140,0x140,0x1e0,8);
        DAT_00563d68 = 0;
      }
      else {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8 + g_LanguageId * 100,iVar2,0xa0,
                     0x19c);
        iVar2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
        iVar2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8 + g_LanguageId * 100,iVar2,0x140,
                     0xd3);
        iVar2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
        Gfx_SetRenderTarget(iVar2,0x140,0x140,0x1e0,8);
        DAT_00619310 = g_CurrentTrackIndex + 1;
        DAT_00563d68 = 1;
      }
    }
    else {
      _sprintf((char *)local_a4,s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32);
      Font_GetTextWidth((const char *)local_a4, g_FontId_Small);
      iVar2 = g_FontId_Small;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText((char *)local_a4,iVar2,0xa0,0x19c);
      iVar2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(iVar2,0x280,0x280,0xf0,8);
      iVar2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText((char *)local_a4,iVar2,0x140,0xd3);
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
      DAT_00563d68 = 0;
      DAT_005285c8 = 0;
    }
    iVar2 = 0;
    if (0 < g_NumRacers) {
      puVar13 = &DAT_005286f4;
      iVar12 = 0x3000;
      iVar11 = 0x28;
      do {
        uVar6 = *puVar13;
        puVar13 = puVar13 + 1;
        iVar2 = iVar2 + 1;
        local_dc = 0x5600;
        local_d8 = iVar12;
        Gfx_DrawSprite(uVar6,&local_dc,0);
        Menu_AddLayoutItem(0x73,0,0x28,iVar11,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0xb7,0,0x16,iVar11,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        iVar12 = iVar12 + 0x3c00;
        iVar11 = iVar11 + 0x3c;
      } while (iVar2 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
  }
  if (g_PlayerCarChoice == 6) {
    iVar2 = 0;
    do {
      iVar2 = iVar2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + iVar2) = 0;
    } while (iVar2 < 0x25800);
    bVar15 = 0;
    DAT_00563d68 = 1;
    g_PlayerCarChoice = 7;
    if (0 < g_NumRacers) {
      iVar2 = g_NumRacers;
      do {
        if ((*(int *)(g_Vehicles + 0x3a0) < 4) ||
           ((*(int *)(g_Vehicles + 0x4bec) < 4 && (g_IsSplitScreen == 1)))) {
          bVar15 = 1;
        }
        iVar2 = iVar2 + -1;
      } while (iVar2 != 0);
    }
    if (bVar15) {
      *(int *)(g_Vehicles + 0x5d4) = 0;
      *(int *)(g_Vehicles + 20000) = 0;
      DAT_00563d68 = 0;
      _DAT_00601664 = 0;
      g_MenuCursorPos = 0;
      *(int *)(g_Vehicles + 0x604) = 1;
      if (g_IsSplitScreen == 1) {
        *(int *)(g_Vehicles + 0x4e50) = 1;
      }
      puVar13 = &DAT_006192a0;
      do {
        *puVar13 = 100;
        puVar5 = puVar13 + 1;
        puVar13[10] = 100;
        puVar13 = puVar5;
      } while (puVar5 < &DAT_006192b8);
      iVar12 = 0;
      iVar2 = g_NumRacers;
      if (0 < g_NumRacers) {
        piVar3 = (int *)(g_Vehicles + 0x3a0);
        iVar11 = 0;
        do {
          iVar2 = *piVar3;
          *(int *)((int)&DAT_00525e70 + iVar11) = iVar12;
          iVar8 = g_NumRacers;
          piVar3 = piVar3 + 0x1213;
          iVar12 = iVar12 + 1;
          *(int *)((int)&DAT_00553040 + iVar11) =
               *(int *)(&DAT_00492a24 + iVar2 * 4) + *(int *)((int)&DAT_00563d00 + iVar11);
          iVar11 = iVar11 + 4;
          iVar2 = g_NumRacers;
        } while (iVar12 < iVar8);
      }
      for (; -1 < iVar2; iVar2 = iVar2 + -1) {
        if (1 < g_NumRacers) {
          iVar11 = 4;
          iVar12 = g_NumRacers + -1;
          do {
            iVar8 = *(int *)((int)&DAT_00553040 + iVar11);
            if (*(int *)(iVar11 + 0x55303c) < iVar8) {
              uVar6 = *(int *)((int)&DAT_00525e70 + iVar11);
              *(int *)((int)&DAT_00553040 + iVar11) = *(int *)(iVar11 + 0x55303c);
              uVar1 = *(int *)((int)&DAT_00525e6c + iVar11);
              *(int *)(iVar11 + 0x55303c) = iVar8;
              *(int *)((int)&DAT_00525e70 + iVar11) = uVar1;
              *(int *)((int)&DAT_00525e6c + iVar11) = uVar6;
            }
            iVar11 = iVar11 + 4;
            iVar12 = iVar12 + -1;
          } while (iVar12 != 0);
        }
      }
      if ((DAT_00525e70 == 0) && (g_IsSplitScreen == 0)) {
        if (g_DifficultyLevel == 6) {
          if (DAT_006393ac == 3) {
            _DAT_0063992b = 1;
          }
          if (DAT_006393ac < 3) {
            if (DAT_00639927 < 3) {
              DAT_00639927 = 3;
            }
            DAT_006393ac = 3;
          }
        }
        if ((g_DifficultyLevel == 5) && (DAT_006393ac < 2)) {
          if (DAT_00639927 < 2) {
            DAT_00639927 = 2;
          }
          DAT_006393ac = 2;
        }
        if ((g_DifficultyLevel == 4) && (DAT_006393ac == 0)) {
          if (DAT_00639927 < 1) {
            DAT_00639927 = 1;
          }
          DAT_006393ac = 1;
        }
      }
    }
  }
  if ((0 < g_PlayerCarChoice) && (DAT_00553070 < DAT_00553280)) {
    iVar2 = 0;
    if (0 < g_NumRacers) {
      piVar3 = (int *)(g_Vehicles + 0x3a0);
      iVar12 = 0;
      do {
        iVar11 = *piVar3;
        piVar3 = piVar3 + 0x1213;
        *(int *)((int)aiStack_74 + iVar12 + 4) =
             *(int *)(&DAT_00492a24 + iVar11 * 4) + *(int *)((int)&DAT_00563d00 + iVar12);
        *(int *)((int)local_d0 + iVar12 + 4) = iVar2;
        iVar2 = iVar2 + 1;
        iVar12 = iVar12 + 4;
      } while (iVar2 < g_NumRacers);
    }
    if ((DAT_00553070 < DAT_00553280) && (iVar2 = g_NumRacers, g_PlayerCarChoice == 5)) {
      for (; -1 < iVar2; iVar2 = iVar2 + -1) {
        if (1 < g_NumRacers) {
          iVar12 = 4;
          iVar11 = g_NumRacers + -1;
          do {
            iVar8 = *(int *)((int)aiStack_74 + iVar12 + 4);
            if (*(int *)((int)aiStack_74 + iVar12) < iVar8) {
              uVar6 = *(int *)((int)local_d0 + iVar12 + 4);
              *(int *)((int)aiStack_74 + iVar12 + 4) = *(int *)((int)aiStack_74 + iVar12);
              *(int *)((int)local_d0 + iVar12 + 4) = *(int *)((int)local_d0 + iVar12);
              *(int *)((int)aiStack_74 + iVar12) = iVar8;
              *(int *)((int)local_d0 + iVar12) = uVar6;
            }
            iVar12 = iVar12 + 4;
            iVar11 = iVar11 + -1;
          } while (iVar11 != 0);
        }
      }
    }
    uVar10 = 0;
    if (0 < g_NumRacers) {
      do {
        if (g_GameMode == 3) {
          iVar2 = 0;
          if (0 < g_NumRacers) {
            piVar3 = (int *)(g_Vehicles + 0x3a0);
            do {
              if (*piVar3 + uVar10 == 6) break;
              piVar3 = piVar3 + 0x1213;
              iVar2 = iVar2 + 1;
            } while (iVar2 < g_NumRacers);
            if (iVar2 < g_NumRacers) {
              Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
              local_dc = 0x3b00;
              DAT_0063f2d8 = DAT_00525e64;
              if ((uVar10 & 1) == 0) {
                local_dc = 0xd300;
              }
              local_d8 = uVar10 * -0x3c00 + 0x15100;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[iVar2 * 0x13]],&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              iVar2 = uVar10 * -0x3c + 0x160;
              iVar11 = 0xad;
              goto LAB_0043a494;
            }
          }
        }
        else {
          iVar2 = 0;
          if (g_NumRacers < 1) {
LAB_00439d5b:
            if (DAT_00553068 != 1) goto LAB_0043a4e1;
          }
          else {
            piVar3 = (int *)(g_Vehicles + 0x3a0);
            do {
              if (*piVar3 - uVar10 == 1) break;
              piVar3 = piVar3 + 0x1213;
              iVar2 = iVar2 + 1;
            } while (iVar2 < g_NumRacers);
            if (g_NumRacers <= iVar2) goto LAB_00439d5b;
          }
          Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
          DAT_0063f2d8 = DAT_00525e64;
          if (g_PlayerCarChoice == 1) {
            if (DAT_00553068 == 0) {
              local_dc = 0x100;
              if ((uVar10 & 1) != 0) {
                local_dc = 0x10d00;
              }
              local_d8 = uVar10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[iVar2 * 0x13]],&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar2 = uVar10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,g_FontId_Medium,0x71,iVar2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            else {
              uVar10 = (unsigned int)((double)DAT_00525e44 * _DAT_0047a630 <=
                             *(double *)(g_Vehicles + 0x3a4));
              local_dc = 0x100;
              if ((double)DAT_00525e44 * _DAT_0047a630 <= *(double *)(g_Vehicles + 0x3a4)) {
                local_dc = 0x10d00;
              }
              local_d8 = uVar10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[*g_PlayerHUDState],&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar2 = uVar10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,g_FontId_Medium,0x71,iVar2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,iVar12,0xce,iVar2);
              uVar10 = uVar10 ^ 1;
              local_dc = 0x100;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              if ((char)uVar10 != '\0') {
                local_dc = 0x10d00;
              }
              local_d8 = uVar10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(DAT_00528738,&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar2 = uVar10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,g_FontId_Medium,0x71,iVar2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            Font_DrawText((char *)local_a4,iVar12,0xce,iVar2);
            iVar2 = g_ScreenWidth;
            ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
            iVar12 = g_ScreenHeight;
          }
          else {
            if (g_PlayerCarChoice == 3) {
              local_dc = 0x2200;
              if ((uVar10 & 1) != 0) {
                local_dc = 0xeb00;
              }
              local_d8 = uVar10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[iVar2 * 0x13]],&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar2 = uVar10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,g_FontId_Medium,0x93,iVar2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              _sprintf((char *)local_a4,&DAT_00493c78);
              iVar12 = g_FontId_Medium;
              iVar11 = 0xcf;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            else {
              if (g_PlayerCarChoice != 5) goto LAB_0043a4e1;
              local_dc = 0x2200;
              if ((uVar10 & 1) != 0) {
                local_dc = 0xeb00;
              }
              local_d8 = uVar10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[local_d0[uVar10 + 1] * 0x13]],&local_dc,0);
              _sprintf((char *)local_a4,&DAT_004920e8);
              iVar2 = uVar10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)local_a4,g_FontId_Medium,0x93,iVar2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              _sprintf((char *)local_a4,&DAT_00493c78);
              iVar12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              iVar11 = 0xcf;
            }
LAB_0043a494:
            Font_DrawText((char *)local_a4,iVar12,iVar11,iVar2);
            iVar12 = g_ScreenHeight;
            iVar2 = g_ScreenWidth;
            ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
          }
          Gfx_SetRenderTarget(&g_VirtualFramebuffer,iVar2,iVar2,iVar12,8);
          DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
        }
LAB_0043a4e1:
        uVar10 = uVar10 + 1;
      } while ((int)uVar10 < g_NumRacers);
    }
    DAT_00553070 = DAT_00553280;
  }
  aiStack_74[1] = 0;
  if (0 < g_NumRacers) {
    piVar3 = &DAT_00553010;
    iVar2 = 0;
    do {
      iVar12 = g_CurrentTrackIndex;
      iVar11 = g_Vehicles + iVar2;
      if ((((*(int *)(iVar11 + 0x528) == 1) && (*piVar3 == 0)) && (g_PlayerHUDState[1] == 0)) &&
         ((iVar2 == 0 || ((iVar2 == 0x484c && (g_IsSplitScreen == 1)))))) {
        *piVar3 = 1;
        iVar8 = iVar12 * 0x78;
        if (*(double *)(iVar11 + 0x3a4) < *(double *)(&DAT_00639593 + iVar12 * 0x78)) {
          *(int *)(&DAT_00639597 + iVar8) = *(int *)(iVar11 + 0x3a8);
          iVar12 = 4;
          *(int *)(&DAT_00639593 + iVar8) = *(int *)(iVar11 + 0x3a4);
          _sprintf(&DAT_0063956f + iVar8,&DAT_004920e8);
          do {
            iVar11 = g_CurrentTrackIndex * 0xf + iVar12;
            if (*(double *)(&DAT_00639573 + iVar11 * 8) < *(double *)(&DAT_0063956b + iVar11 * 8)) {
              local_d0[2] = *(int *)(&DAT_0063956f + iVar11 * 8);
              local_d0[1] = *(int *)(&DAT_0063956b + iVar11 * 8);
              _sprintf(local_ac,&DAT_004920e8);
              iVar11 = g_CurrentTrackIndex * 0xf + iVar12;
              *(int *)(&DAT_0063956f + iVar11 * 8) =
                   *(int *)(&DAT_00639577 + iVar11 * 8);
              *(int *)(&DAT_0063956b + iVar11 * 8) =
                   *(int *)(&DAT_00639573 + iVar11 * 8);
              _sprintf(&DAT_0063955b + (g_CurrentTrackIndex * 0x1e + iVar12) * 4,&DAT_004920e8);
              iVar8 = g_CurrentTrackIndex * 0xf + iVar12;
              *(int *)(&DAT_00639577 + iVar8 * 8) = local_d0[2];
              iVar11 = g_CurrentTrackIndex;
              *(int *)(&DAT_00639573 + iVar8 * 8) = local_d0[1];
              _sprintf(&DAT_0063955f + (iVar11 * 0x1e + iVar12) * 4,&DAT_004920e8);
            }
            iVar12 = iVar12 + -1;
          } while (iVar12 != 0);
        }
        puVar13 = (int *)(g_Vehicles + 0x3bc + iVar2);
        iVar12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3bc + iVar2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + iVar12) = puVar13[1];
          iVar11 = 4;
          *(int *)(&DAT_006395cf + iVar12) = *puVar13;
          _sprintf(&DAT_006395ab + iVar12,&DAT_004920e8);
          do {
            iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
            if (*(double *)(&DAT_006395af + iVar12 * 8) < *(double *)(&DAT_006395a7 + iVar12 * 8)) {
              local_d0[2] = *(int *)(&DAT_006395ab + iVar12 * 8);
              local_d0[1] = *(int *)(&DAT_006395a7 + iVar12 * 8);
              _sprintf(local_ac,&DAT_004920e8);
              iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395ab + iVar12 * 8) =
                   *(int *)(&DAT_006395b3 + iVar12 * 8);
              *(int *)(&DAT_006395a7 + iVar12 * 8) =
                   *(int *)(&DAT_006395af + iVar12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + iVar11) * 4,&DAT_004920e8);
              iVar8 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395b3 + iVar8 * 8) = local_d0[2];
              iVar12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + iVar8 * 8) = local_d0[1];
              _sprintf(&DAT_0063959b + (iVar12 * 0x1e + iVar11) * 4,&DAT_004920e8);
            }
            iVar11 = iVar11 + -1;
          } while (iVar11 != 0);
        }
        puVar13 = (int *)(g_Vehicles + 0x3c4 + iVar2);
        iVar12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3c4 + iVar2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + iVar12) = puVar13[1];
          iVar11 = 4;
          *(int *)(&DAT_006395cf + iVar12) = *puVar13;
          _sprintf(&DAT_006395ab + iVar12,&DAT_004920e8);
          do {
            iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
            if (*(double *)(&DAT_006395af + iVar12 * 8) < *(double *)(&DAT_006395a7 + iVar12 * 8)) {
              local_d0[2] = *(int *)(&DAT_006395ab + iVar12 * 8);
              local_d0[1] = *(int *)(&DAT_006395a7 + iVar12 * 8);
              _sprintf(local_ac,&DAT_004920e8);
              iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395ab + iVar12 * 8) =
                   *(int *)(&DAT_006395b3 + iVar12 * 8);
              *(int *)(&DAT_006395a7 + iVar12 * 8) =
                   *(int *)(&DAT_006395af + iVar12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + iVar11) * 4,&DAT_004920e8);
              iVar8 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395b3 + iVar8 * 8) = local_d0[2];
              iVar12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + iVar8 * 8) = local_d0[1];
              _sprintf(&DAT_0063959b + (iVar12 * 0x1e + iVar11) * 4,&DAT_004920e8);
            }
            iVar11 = iVar11 + -1;
          } while (iVar11 != 0);
        }
        puVar13 = (int *)(g_Vehicles + 0x3cc + iVar2);
        iVar12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3cc + iVar2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + iVar12) = puVar13[1];
          iVar11 = 4;
          *(int *)(&DAT_006395cf + iVar12) = *puVar13;
          _sprintf(&DAT_006395ab + iVar12,&DAT_004920e8);
          do {
            iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
            if (*(double *)(&DAT_006395af + iVar12 * 8) < *(double *)(&DAT_006395a7 + iVar12 * 8)) {
              local_d0[2] = *(int *)(&DAT_006395ab + iVar12 * 8);
              local_d0[1] = *(int *)(&DAT_006395a7 + iVar12 * 8);
              _sprintf(local_ac,&DAT_004920e8);
              iVar12 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395ab + iVar12 * 8) =
                   *(int *)(&DAT_006395b3 + iVar12 * 8);
              *(int *)(&DAT_006395a7 + iVar12 * 8) =
                   *(int *)(&DAT_006395af + iVar12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + iVar11) * 4,&DAT_004920e8);
              iVar8 = g_CurrentTrackIndex * 0xf + iVar11;
              *(int *)(&DAT_006395b3 + iVar8 * 8) = local_d0[2];
              iVar12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + iVar8 * 8) = local_d0[1];
              _sprintf(&DAT_0063959b + (iVar12 * 0x1e + iVar11) * 4,&DAT_004920e8);
            }
            iVar11 = iVar11 + -1;
          } while (iVar11 != 0);
        }
      }
      piVar3 = piVar3 + 1;
      iVar2 = iVar2 + 0x484c;
      aiStack_74[1] = aiStack_74[1] + 1;
    } while (aiStack_74[1] < g_NumRacers);
  }
  if (DAT_00552fc0 == 0) {
    aiStack_74[1] = 0x19;
    iVar2 = 0x2d;
  }
  else {
    aiStack_74[1] = 0x28;
    iVar2 = 0x50;
  }
  if (g_PlayerCarChoice < 1) goto LAB_0043c014;
  if ((g_PlayerCarChoice < 6) && (DAT_0054f978 == 1)) {
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,8,0x140,0x25,0x563db0,g_ScreenWidth / 2 + -0xa0,0,g_pLisaDrawCommandWritePtr,0x140,
                 g_ScreenWidth);
    if (DAT_00552fc0 == 0) {
      iVar19 = 0x140;
      iVar12 = g_ScreenHeight + -8;
      iVar11 = g_ScreenWidth / 2 + -0xa0;
      iVar21 = 0x1a4;
      iVar20 = 0x140;
      iVar8 = 0x19c;
    }
    else {
      iVar19 = 0x280;
      iVar12 = g_ScreenHeight + -0x10;
      iVar11 = g_ScreenWidth / 2 + -0x140;
      iVar21 = 0xe3;
      iVar20 = 0x280;
      iVar8 = 0xd3;
    }
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,iVar8,iVar20,iVar21,0x563db0,iVar11,iVar12,g_pLisaDrawCommandWritePtr,iVar19,
                 g_ScreenWidth);
  }
  if ((5 < g_PlayerCarChoice) && (g_PlayerCarChoice < 8)) {
    if (*(int *)(g_Vehicles + 0x3a0) < 4) {
      local_dc = (g_ScreenWidth / 2 + -0x6c) * 0x100;
      lVar17 = __ftol();
      local_d8 = (int)lVar17;
      local_d0[1] = *(int *)(g_Vehicles + 0x5d4);
      _DAT_00563d74 = 0;
      _DAT_00563d70 = (float)local_d0[1] * (float)_DAT_0047a658;
      if (DAT_00563ce0 == 0) {
        Audio_StopSound(8);
        DAT_00563ce0 = 1;
      }
      if ((DAT_00525e70 == 0) || ((DAT_00525e70 == 1 && (g_IsSplitScreen == 1)))) {
        if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
          puVar22 = &DAT_00563d70;
        }
        else {
          puVar22 = (char *)0x0;
        }
        uVar6 = ((int*)&(DAT_005287b8))[DAT_00563ce8 * 3];
LAB_0043b1a7:
        Gfx_DrawSprite(uVar6,&local_dc,puVar22);
      }
      else {
        if ((DAT_00525e74 == 0) || ((DAT_00525e74 == 1 && (g_IsSplitScreen == 1)))) {
          if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
            puVar22 = &DAT_00563d70;
            uVar6 = ((int*)&(DAT_005287bc))[DAT_00563ce8 * 3];
          }
          else {
            puVar22 = (char *)0x0;
            uVar6 = ((int*)&(DAT_005287bc))[DAT_00563ce8 * 3];
          }
          goto LAB_0043b1a7;
        }
        if ((DAT_00525e78 == 0) || ((DAT_00525e78 == 1 && (g_IsSplitScreen == 1)))) {
          if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
            puVar22 = &DAT_00563d70;
            uVar6 = *(int *)(&DAT_005287c0 + DAT_00563ce8 * 0xc);
          }
          else {
            puVar22 = (char *)0x0;
            uVar6 = *(int *)(&DAT_005287c0 + DAT_00563ce8 * 0xc);
          }
          goto LAB_0043b1a7;
        }
      }
      _sprintf((char *)local_a4,s_CONGRATULATIONS__004969a8 + g_LanguageId * 0xf5);
      lVar17 = __ftol();
      Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)lVar17);
      _sprintf((char *)local_a4,s_YOU_HAVE_COMPLETED_THE_004969cb + g_LanguageId * 0xf5);
      lVar17 = __ftol();
      Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)lVar17);
      _sprintf((char *)local_a4,s__s_CHAMPIONSHIP_004969ee + g_LanguageId * 0xf5);
      lVar17 = __ftol();
      Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)lVar17);
      if (DAT_00525e70 == 0) {
        pcVar4 = s_AT_FIRST_PLACE_WITH_00496a11 + g_LanguageId * 0xf5;
LAB_0043b3a9:
        _sprintf((char *)local_a4,pcVar4);
      }
      else {
        if (DAT_00525e74 == 0) {
          pcVar4 = s_AT_SECOND_PLACE_WITH_00496a34 + g_LanguageId * 0xf5;
          goto LAB_0043b3a9;
        }
        if (DAT_00525e78 == 0) {
          pcVar4 = s_AT_THIRD_PLACE_WITH_00496a57 + g_LanguageId * 0xf5;
          goto LAB_0043b3a9;
        }
      }
      lVar17 = __ftol();
      Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)lVar17);
      _sprintf((char *)local_a4,s_A_SCORE_OF__d_PTS__00496a7a + g_LanguageId * 0xf5);
      lVar17 = __ftol();
      Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)lVar17);
      iVar12 = g_LanguageId;
      if (g_IsSplitScreen == 0) {
        if (DAT_00525e70 == 0) {
          if (2 < DAT_00563ce8) goto LAB_0043b5f5;
          _sprintf((char *)local_a4,s_NOW_TRY_THE__s_LEAGUE__00496f68 + g_LanguageId * 0x28);
          ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        }
        else {
          ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
          _sprintf((char *)local_a4,s_YOU_MUST_ACHIEVE_THE_00497058 + iVar12 * 0x46);
          lVar17 = __ftol();
          Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2,(int)lVar17);
          _sprintf((char *)local_a4,s_GOLD_STATUE_TO_ADVANCE__0049707b + g_LanguageId * 0x46);
        }
        lVar17 = __ftol();
        Font_DrawText((char *)local_a4,g_FontId_Menu,g_ScreenWidth / 2,(int)lVar17);
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      }
LAB_0043b5f5:
      if (DAT_00552fc0 == 0) {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,g_FontId_Small,
                     g_ScreenWidth / 2,g_ScreenHeight + -10);
        iVar12 = g_FontId_Small;
      }
      else {
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,g_FontId_Menu,
                     g_ScreenWidth / 2,g_ScreenHeight + -0x14);
        iVar12 = g_FontId_Menu;
      }
      iVar11 = 0;
      ((int*)&(g_FontAlignMode))[iVar12 * 400] = 0;
      do {
        if ((0xc < *(int *)((int)&DAT_006192a0 + iVar11)) && (*(int *)(g_Vehicles + 0x5d4) < 0x14)
           ) {
          iVar12 = g_PlayerHUDState[0xb];
          Math_RandomFloat0To1();
          local_d0[1] = (g_PlayerHUDState[0xd] - iVar12) / 2 + iVar12;
          lVar17 = __ftol();
          *(int *)((int)&DAT_00552d90 + iVar11) = (int)lVar17;
          Math_RandomFloat0To1();
          local_d0[1] = (g_PlayerHUDState[0xe] - g_PlayerHUDState[0xc]) / 2;
          lVar17 = __ftol();
          *(int *)((int)&DAT_00552df0 + iVar11) = (int)lVar17;
          if (*(int *)((int)&DAT_006192a0 + iVar11) < 100) {
            *(int *)((int)&DAT_006192a0 + iVar11) = 0;
          }
          else {
            Math_RandomFloat0To1();
            lVar17 = __ftol();
            *(int *)((int)&DAT_006192a0 + iVar11) = (int)lVar17;
          }
        }
        if (*(int *)((int)&DAT_006192a0 + iVar11) < 0xd) {
          _DAT_00563d74 = 0;
          _DAT_00563d70 = 0.5;
          local_dc = *(int *)((int)&DAT_00552d90 + iVar11) << 8;
          local_d8 = *(int *)((int)&DAT_00552df0 + iVar11) << 8;
        }
        iVar11 = iVar11 + 4;
      } while (iVar11 < 0x18);
    }
    else if (((DAT_00552f28 < 1) && (DAT_00563c60 == 0)) && (DAT_00563da0 == 0)) {
      fsin((double)_DAT_00552f68);
      local_dc = (g_ScreenWidth / 2 + -0x4c) * 0x100;
      lVar17 = __ftol();
      local_d8 = (int)lVar17;
      Gfx_DrawSprite(DAT_00528794,&local_dc,0);
    }
    else {
      iVar12 = 0x2580;
      do {
        iVar11 = 0;
        do {
          iVar8 = g_RenderTargetSurface + iVar11;
          iVar11 = iVar11 + 1;
          *(char *)(iVar8 + iVar12) = 0;
        } while (iVar11 < 0x140);
        iVar12 = iVar12 + 0x140;
      } while (iVar12 < 0xaf00);
      Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,g_ScreenHeight,8);
      DAT_0063f2d8 = DAT_00525e64;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      _sprintf((char *)local_a4,s_SORRY__YOU_MUST_REACH_THIRD_00496420 + g_LanguageId * 100);
      Font_DrawText((char *)local_a4,g_FontId_Menu,0xa0,0x28);
      _sprintf((char *)local_a4,s_PLACE_OR_BETTER_TO_PROCEED__00496452 + g_LanguageId * 100);
      Font_DrawText((char *)local_a4,g_FontId_Menu,0xa0,0x37);
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      _sprintf(local_50,s_RETRY_00494dd0 + g_LanguageId * 0xd2);
      uVar16 = Font_GetTextWidth((const char *)(local_50), g_FontId_Medium);
      Menu_AddLayoutItem(0x32,0,(int)uVar16,0x50,0);
      _sprintf(local_28,&DAT_00494d94 + g_LanguageId * 0xd2);
      uVar16 = Font_GetTextWidth((const char *)(local_28), g_FontId_Medium);
      Menu_AddLayoutItem(0x32,1,(int)uVar16,0x50,0);
      Menu_LayoutItems(g_RenderTargetSurface,0x140);
      Menu_ClearLayout();
      iVar12 = g_FontId_Large;
      if (DAT_0054f988 == 0) {
        iVar12 = g_FontId_Medium;
      }
      Font_DrawText(local_50,iVar12,0x40,0x5c);
      iVar12 = g_FontId_Large;
      if (DAT_0054f988 == 1) {
        iVar12 = g_FontId_Medium;
      }
      Font_DrawText(local_28,iVar12,0x40,0x6f);
      iVar12 = 0;
      uVar16 = Font_GetTextWidth((const char *)(local_50), g_FontId_Medium);
      if (0 < DAT_00552f28) {
        iVar11 = ((int)uVar16 + 0x5a) * 0x100;
        do {
          iVar12 = iVar12 + 1;
          local_d8 = 0x5800;
          local_dc = iVar11;
          Gfx_DrawSprite(DAT_005287e8,&local_dc,0);
          iVar11 = iVar11 + 0x1900;
        } while (iVar12 < DAT_00552f28);
      }
      DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
      Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
      Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,0x1e,0x13f,0x8c,0x563db0,g_ScreenWidth / 2 + -0xa0,
                   g_ScreenHeight / 2 + -0x46,g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
    }
  }
  if (g_PlayerCarChoice == 8) {
    iVar12 = 0;
    do {
      iVar12 = iVar12 + 1;
      *(char *)(g_RenderTargetSurface + -1 + iVar12) = 0;
    } while (iVar12 < 0x25800);
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    DAT_0063f2d8 = DAT_00525e64;
    uVar16 = Font_GetTextWidth((const char *)(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e), g_FontId_Medium);
    Menu_AddLayoutItem(0x93 - (int)uVar16 / 2,0,(int)uVar16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    iVar12 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e,iVar12,0xa0,0xd);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    iVar11 = 0;
    iVar12 = g_NumRacers;
    if (0 < g_NumRacers) {
      piVar3 = (int *)(g_Vehicles + 0x3a0);
      iVar8 = 0;
      do {
        iVar12 = *piVar3;
        *(int *)((int)&DAT_00525e70 + iVar8) = iVar11;
        iVar20 = g_NumRacers;
        piVar3 = piVar3 + 0x1213;
        iVar11 = iVar11 + 1;
        *(int *)((int)&DAT_00553040 + iVar8) =
             *(int *)(&DAT_00492a24 + iVar12 * 4) + *(int *)((int)&DAT_00563d00 + iVar8);
        iVar8 = iVar8 + 4;
        iVar12 = g_NumRacers;
      } while (iVar11 < iVar20);
    }
    for (; -1 < iVar12; iVar12 = iVar12 + -1) {
      if (1 < g_NumRacers) {
        iVar8 = 4;
        iVar11 = g_NumRacers + -1;
        do {
          iVar20 = *(int *)((int)&DAT_00553040 + iVar8);
          if (*(int *)(iVar8 + 0x55303c) < iVar20) {
            uVar6 = *(int *)((int)&DAT_00525e70 + iVar8);
            *(int *)((int)&DAT_00553040 + iVar8) = *(int *)(iVar8 + 0x55303c);
            uVar1 = *(int *)((int)&DAT_00525e6c + iVar8);
            *(int *)(iVar8 + 0x55303c) = iVar20;
            *(int *)((int)&DAT_00525e70 + iVar8) = uVar1;
            *(int *)((int)&DAT_00525e6c + iVar8) = uVar6;
          }
          iVar8 = iVar8 + 4;
          iVar11 = iVar11 + -1;
        } while (iVar11 != 0);
      }
    }
    local_d0[0] = g_ScreenHeight / 2;
    local_a8 = (int)(g_ScreenHeight + (g_ScreenHeight >> 0x1f & 7U)) >> 3;
    uVar14 = (unsigned int)(*(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x3a0) * 4) + DAT_00563d00 <
                   *(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x4bec) * 4) + DAT_00563d04);
    uVar10 = (unsigned int)(*(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x4bec) * 4) + DAT_00563d04 <=
                   *(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x3a0) * 4) + DAT_00563d00);
    if (DAT_00525e70 == uVar14) {
      iVar12 = 0;
    }
    else if (DAT_00525e74 == uVar14) {
      iVar12 = 1;
    }
    else if (DAT_00525e78 == uVar14) {
      iVar12 = 2;
    }
    else if (DAT_00525e7c == uVar14) {
      iVar12 = 3;
    }
    else if (DAT_00525e80 == uVar14) {
      iVar12 = 4;
    }
    else {
      iVar12 = 5;
      if (DAT_00525e84 != uVar14) {
        iVar12 = local_a4[0];
      }
    }
    local_d0[1] = local_d0[0] - local_a8;
    local_dc = 0x2800;
    _DAT_00563d70 = 0.5;
    local_d8 = (local_d0[1] + -0xd) * 0x100;
    if (DAT_00552fc0 != 0) {
      _DAT_00563d70 = 1.0;
    }
    _DAT_00563d74 = 0;
    if (iVar12 < 3) {
      Gfx_DrawSprite(((int*)&(DAT_005287b8))[DAT_00563ce8 * 3 + iVar12],&local_dc,&DAT_00563d70);
    }
    local_dc = 0x4600;
    local_d8 = (local_d0[1] + -0xf) * 0x100;
    Gfx_DrawSprite(((int*)&(DAT_005286f4))[iVar12],&local_dc,0);
    Menu_AddLayoutItem(100,0,0x2d,local_d0[1] + -0x17,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    _sprintf((char *)local_a4,&DAT_004920e8);
    iVar11 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)local_a4,iVar11,0x87,local_d0[1] + -0xb);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    _sprintf((char *)local_a4,s__d_PTS_00496948 + g_LanguageId * 0xf);
    uVar16 = Font_GetTextWidth((const char *)local_a4, g_FontId_Medium);
    Menu_AddLayoutItem(0xaf,0,(int)uVar16,local_d0[1] + -0x17,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    iVar11 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)local_a4,iVar11,(int)uVar16 / 2 + 0xbc,local_d0[1] + -0xb);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    if (DAT_00525e70 == uVar10) {
      iVar12 = 0;
    }
    else if (DAT_00525e74 == uVar10) {
      iVar12 = 1;
    }
    else if (DAT_00525e78 == uVar10) {
      iVar12 = 2;
    }
    else if (DAT_00525e7c == uVar10) {
      iVar12 = 3;
    }
    else if (DAT_00525e80 == uVar10) {
      iVar12 = 4;
    }
    else if (DAT_00525e84 == uVar10) {
      iVar12 = 5;
    }
    local_dc = 0x2800;
    _DAT_00563d70 = 0.5;
    local_d8 = (local_a8 + 0x19 + local_d0[0]) * 0x100;
    if (DAT_00552fc0 != 0) {
      _DAT_00563d70 = 1.0;
    }
    _DAT_00563d74 = 0;
    if (iVar12 < 3) {
      Gfx_DrawSprite(((int*)&(DAT_005287b8))[DAT_00563ce8 * 3 + iVar12],&local_dc,&DAT_00563d70);
    }
    local_dc = 0x4600;
    local_d8 = (local_a8 + 0xf + local_d0[0]) * 0x100;
    Gfx_DrawSprite(((int*)&(DAT_005286f4))[iVar12],&local_dc,0);
    local_d0[1] = local_a8 + 7 + local_d0[0];
    Menu_AddLayoutItem(100,0,0x2d,local_d0[1],0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    _sprintf((char *)local_a4,&DAT_004920e8);
    iVar12 = local_a8 + 0x13 + local_d0[0];
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)local_a4,g_FontId_Medium,0x87,iVar12);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    _sprintf((char *)local_a4,s__d_PTS_00496948 + g_LanguageId * 0xf);
    uVar16 = Font_GetTextWidth((const char *)local_a4, g_FontId_Medium);
    Menu_AddLayoutItem(0xaf,0,(int)uVar16,local_d0[1],0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)local_a4,g_FontId_Medium,(int)uVar16 / 2 + 0xbc,iVar12);
    iVar11 = g_ScreenHeight;
    iVar12 = g_ScreenWidth;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,iVar12,iVar12,iVar11,8);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,0,0x140,g_ScreenHeight,0x563db0,g_ScreenWidth / 2 + -0xa0,0,
                 g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
    iVar11 = g_LanguageId;
    iVar12 = g_FontId_Small;
    if (DAT_00552fc0 == 0) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar11 * 0x32,iVar12,0xa0,
                   g_ScreenHeight + -10);
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    }
    else {
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,g_FontId_Menu,
                   g_ScreenWidth / 2,g_ScreenHeight + -0x10);
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
    }
  }
LAB_0043c014:
  iVar12 = g_NumRacers;
  if ((DAT_00553068 == 1) &&
     (local_d0[1] = DAT_00525e44 / 100, (double)local_d0[1] <= _DAT_00553090)) {
    iVar12 = g_NumRacers + 1;
  }
  uVar10 = 0;
  if (0 < iVar12) {
    iVar11 = 0x57;
    iVar2 = iVar2 + -0x1a;
    do {
      iVar8 = 0;
      if (0 < g_NumRacers) {
        piVar3 = (int *)(g_Vehicles + 0x3a0);
        do {
          if (*piVar3 - uVar10 == 1) break;
          piVar3 = piVar3 + 0x1213;
          iVar8 = iVar8 + 1;
        } while (iVar8 < g_NumRacers);
      }
      if (DAT_00553068 == 1) {
        bVar15 = 0;
        if (g_NumRacers == iVar8) {
          iVar8 = 0;
          goto LAB_0043c0a0;
        }
      }
      else {
LAB_0043c0a0:
        bVar15 = g_NumRacers == iVar8;
      }
      if (((bVar15 || g_NumRacers < iVar8) || (g_PlayerCarChoice < 1)) ||
         ((5 < g_PlayerCarChoice || (DAT_0054f978 != 1)))) goto LAB_0043c34d;
      iVar21 = DAT_005db038;
      iVar20 = iVar11;
      iVar8 = iVar2;
      if (g_GameMode == 3) {
        if (DAT_0054f9d0 == uVar10) {
          iVar7 = g_ScreenWidth / 2 + -0xa0;
          iVar19 = iVar11 + -0x36;
          iVar18 = 0;
        }
        else {
          if ((int)uVar10 <= (int)DAT_0054f9d0) goto LAB_0043c34d;
          if ((uVar10 & 1) == 0) {
            iVar8 = g_ScreenWidth / 2 + -0x69;
            iVar21 = 0x73;
            iVar20 = 0x37;
          }
          else {
            iVar8 = g_ScreenWidth / 2 + 0x2d;
            iVar21 = 0x109;
            iVar20 = 0xcd;
          }
          Gfx_BlitTransparentLUT(g_RenderTargetSurface,iVar20,iVar11 + -0x36,iVar21,iVar11,0x563db0,iVar8,iVar2,
                       g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
          iVar8 = iVar2 + 0xf;
          iVar20 = iVar11 + -0xf;
          iVar19 = iVar11 + -0x27;
          iVar7 = g_ScreenWidth / 2 + -0x37;
          iVar21 = 0xd7;
          iVar18 = 0x69;
        }
LAB_0043c33f:
        Gfx_BlitTransparentLUT(g_RenderTargetSurface,iVar18,iVar19,iVar21,iVar20,0x563db0,iVar7,iVar8,g_pLisaDrawCommandWritePtr,
                     0x140,g_ScreenWidth);
      }
      else {
        if (DAT_0054f9d0 == uVar10) {
          iVar7 = g_ScreenWidth / 2 + -0xa0;
          iVar19 = iVar11 + -0x36;
          iVar18 = 0;
          goto LAB_0043c33f;
        }
        if ((int)uVar10 < (int)DAT_0054f9d0) {
          if (g_PlayerCarChoice == 1) {
            if ((uVar10 & 1) == 0) {
              iVar8 = g_ScreenWidth / 2 + -0xa0;
              iVar21 = 0x32;
              iVar20 = 0;
            }
            else {
              iVar8 = g_ScreenWidth / 2 + 0x6e;
              iVar21 = 0x140;
              iVar20 = 0x10e;
            }
            Gfx_BlitTransparentLUT(g_RenderTargetSurface,iVar20,iVar11 + -0x36,iVar21,iVar11,0x563db0,iVar8,iVar2,
                         g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
            iVar19 = iVar11 + -0x27;
            iVar7 = g_ScreenWidth / 2 + -0x6e;
            iVar18 = 0x32;
            iVar21 = 0x10e;
            iVar20 = iVar11 + -0xf;
            iVar8 = iVar2 + 0xf;
          }
          else {
            if ((g_PlayerCarChoice != 3) && (g_PlayerCarChoice != 5)) goto LAB_0043c34d;
            if ((uVar10 & 1) == 0) {
              iVar8 = g_ScreenWidth / 2 + -0x82;
              iVar21 = 0x50;
              iVar20 = 0x1e;
            }
            else {
              iVar8 = g_ScreenWidth / 2 + 0x4d;
              iVar21 = 0x122;
              iVar20 = 0xed;
            }
            Gfx_BlitTransparentLUT(g_RenderTargetSurface,iVar20,iVar11 + -0x36,iVar21,iVar11,0x563db0,iVar8,iVar2,
                         g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
            iVar19 = iVar11 + -0x27;
            iVar7 = g_ScreenWidth / 2 + -0x50;
            iVar18 = 0x50;
            iVar21 = 0xed;
            iVar20 = iVar11 + -0xf;
            iVar8 = iVar2 + 0xf;
          }
          goto LAB_0043c33f;
        }
      }
LAB_0043c34d:
      iVar11 = iVar11 + 0x3c;
      iVar2 = iVar2 + aiStack_74[1];
      uVar10 = uVar10 + 1;
    } while ((int)uVar10 < iVar12);
  }
  if (DAT_00552f10 != 2) {
    HUD_RenderPlayAgainPrompt();
  }
  _DAT_00498730 = g_pLisaDrawCommandQueue;
  _DAT_0049873c = g_pActiveTAB;
  _DAT_00498734 = &g_VirtualFramebuffer;
  _DAT_00498738 = &g_VirtualFramebuffer;
  g_ViewportMinX = 0;
  g_ViewportMinY = 0;
  g_ViewportMaxX = g_ScreenWidth + -1;
  g_ViewportMaxY = g_ScreenHeight + -1;
  return g_ScreenHeight + -1;
}

/**
 * @original HUD_RenderPlayAgainPrompt (IGN_WIN.EXE @ 0x0043c3c0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderPlayAgainPrompt(void) {
  int uVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *puVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int local_2c;
  int local_28;
  int aiStack_20 [8];
  iVar4 = 0;
  if (0 < DAT_00553280) {
    do {
      iVar3 = 0;
      if (0 < g_NumRacers) {
        piVar2 = (int *)(g_Vehicles + 0x3a0);
        do {
          if (*piVar2 - iVar4 == 1) break;
          piVar2 = piVar2 + 0x1213;
          iVar3 = iVar3 + 1;
        } while (iVar3 < g_NumRacers);
        if (iVar3 < g_NumRacers) {
          aiStack_20[iVar4] = iVar3;
        }
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < DAT_00553280);
  }
  Font_GetTextWidth((const char *)(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e), g_FontId_Medium);
  Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
  iVar4 = g_FontId_Small;
  DAT_0063f2d8 = DAT_00525e64;
  if (DAT_00552f10 == 4) {
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,iVar4,0xa0,0x19c);
    uVar1 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(uVar1,0x280,0x280,0xf0,8);
    iVar4 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,iVar4,0x140,0xd3);
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
  }
  else {
    if (g_GameMode != 0) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d,iVar4,0xa0,0x19c);
      uVar1 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(uVar1,0x280,0x280,0xf0,8);
      iVar3 = g_LanguageId;
      iVar4 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + iVar3 * 0x2d,iVar4,0x140,0xd3);
      uVar1 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(uVar1,0x140,0x140,0x1e0,8);
      DAT_005285c8 = 1;
      goto LAB_0043c75a;
    }
    Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32), g_FontId_Small);
    iVar4 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,iVar4,0xa0,0x19c);
    uVar1 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(uVar1,0x280,0x280,0xf0,8);
    iVar3 = g_LanguageId;
    iVar4 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + iVar3 * 0x32,iVar4,0x140,0xd3);
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
  }
  Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
LAB_0043c75a:
  iVar4 = 0;
  if (0 < g_NumRacers) {
    iVar3 = 0x28;
    puVar6 = &DAT_005286f4;
    iVar5 = 0x3000;
    do {
      local_28 = iVar5;
      if (g_GameMode == 3) {
        local_2c = 0x7000;
        Gfx_DrawSprite(*puVar6,&local_2c,0);
        iVar8 = 0x28;
        iVar7 = 0x8b;
        iVar9 = iVar3;
LAB_0043c897:
        Menu_AddLayoutItem(iVar7,0,iVar8,iVar9,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
      }
      else {
        local_2c = 0x3400;
        Gfx_DrawSprite(*puVar6,&local_2c,0);
        Menu_AddLayoutItem(0x4f,0,0x28,iVar3,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0x92,0,0x5f,iVar3,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        if (DAT_00553068 == 1) {
          local_28 = iVar5 + 0x3c00;
          local_2c = 0x3400;
          Gfx_DrawSprite(puVar6[1],&local_2c,0);
          Menu_AddLayoutItem(0x4f,0,0x28,iVar3 + 0x3c,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          iVar8 = 0x5f;
          iVar7 = 0x92;
          iVar9 = iVar3 + 0x3c;
          goto LAB_0043c897;
        }
      }
      iVar3 = iVar3 + 0x3c;
      puVar6 = puVar6 + 1;
      iVar5 = iVar5 + 0x3c00;
      iVar4 = iVar4 + 1;
    } while (iVar4 < g_NumRacers);
  }
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
  return;
}

/**
 * @original Camera_UpdateChase (IGN_WIN.EXE @ 0x0043c910, fx.c)
 * @fidelity ADAPTED
 */
void Camera_UpdateChase(void) {
  int iVar1;
  unsigned int uVar2;
  double dVar3;
  int iVar4;
  double *pdVar5;
  int iVar6;
  unsigned int uVar7;
  double fVar8;
  long long lVar9;
  long long local_20;
  unsigned int uStack_14;
  unsigned int uStack_c;
  double local_8;
  if (0.0 <= _DAT_00552e48) {
    uStack_c = 0x40390000;
  }
  else {
    uStack_c = 0x40240000;
  }
  pdVar5 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  if (*(int *)(g_Vehicles + 0x528 + g_ActiveVehicleIndex * 0x484c) == 0) {
    fVar8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x78) =
         (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 + (double)*pdVar5);
    iVar4 = g_Vehicles;
    iVar6 = g_VehicleConfigs;
    iVar1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0x84 + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(iVar6 + 0x80 + iVar1 * 200) = *(int *)(iVar4 + 8 + iVar1 * 0x484c);
    fVar8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x88) =
         (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 +
                 (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
    iVar4 = g_Vehicles;
    iVar6 = g_VehicleConfigs;
    iVar1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0xac + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(iVar6 + 0xa8 + iVar1 * 200) = *(int *)(iVar4 + 0xf8 + iVar1 * 0x484c);
    iVar4 = g_VehicleConfigs;
    iVar6 = g_ActiveVehicleIndex;
    iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
      *(int *)(g_VehicleConfigs + 0xb4 + g_ActiveVehicleIndex * 200) = *(int *)(iVar1 + 0x114);
      *(int *)(iVar4 + 0xb0 + iVar6 * 200) = *(int *)(iVar1 + 0x110);
    }
  }
  else {
    if (*(int *)((int)pdVar5 + 0x604) != 1) goto LAB_0043cd2c;
    fVar8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x78) =
         (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 + (double)*pdVar5);
    iVar4 = g_Vehicles;
    iVar6 = g_VehicleConfigs;
    iVar1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0x84 + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(iVar6 + 0x80 + iVar1 * 200) = *(int *)(iVar4 + 8 + iVar1 * 0x484c);
    fVar8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x88) =
         (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 +
                 (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
    iVar4 = g_Vehicles;
    iVar6 = g_VehicleConfigs;
    iVar1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0xac + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(iVar6 + 0xa8 + iVar1 * 200) = *(int *)(iVar4 + 0xf8 + iVar1 * 0x484c);
    iVar4 = g_VehicleConfigs;
    iVar6 = g_ActiveVehicleIndex;
    iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
      *(int *)(g_VehicleConfigs + 0xb4 + g_ActiveVehicleIndex * 200) = *(int *)(iVar1 + 0x114);
      *(int *)(iVar4 + 0xb0 + iVar6 * 200) = *(int *)(iVar1 + 0x110);
    }
  }
  fVar8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x90) =
       (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                (double)_DAT_0047a6e8 + (double)*(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c))
  ;
  iVar4 = g_Vehicles;
  iVar6 = g_VehicleConfigs;
  iVar1 = g_ActiveVehicleIndex;
  *(int *)(g_VehicleConfigs + 0x9c + g_ActiveVehicleIndex * 200) =
       *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
  *(int *)(iVar6 + 0x98 + iVar1 * 200) = *(int *)(iVar4 + 8 + iVar1 * 0x484c);
  fVar8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0xa0) =
       (double)(fVar8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                (double)_DAT_0047a6e8 +
               (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
  *(double *)(g_VehicleConfigs + 0xb8 + g_ActiveVehicleIndex * 200) =
       *(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) + _DAT_0047a6f0;
LAB_0043cd2c:
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (((*(int *)(g_Vehicles + 0x354 + g_ActiveVehicleIndex * 0x484c) == 0) &&
      (*(int *)(iVar1 + 0x358) == 0)) && (*(int *)(iVar1 + 0x35c) == 0)) {
    dVar3 = *(double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200) * _DAT_0047a6f8;
    pdVar5 = (double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200);
  }
  else {
    pdVar5 = (double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200);
    dVar3 = (_DAT_0047a700 - *(double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200)) * _DAT_0047a708
            + *pdVar5;
  }
  *pdVar5 = dVar3;
  local_20 = *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) -
             *(double *)(g_VehicleConfigs + 0xa8 + g_ActiveVehicleIndex * 200);
  iVar1 = g_VehicleConfigs + g_ActiveVehicleIndex * 200;
  if (_DAT_0047a6f0 < local_20) {
    local_20 = local_20 - _DAT_0047a710;
  }
  if (local_20 < _DAT_0047a718) {
    local_20 = local_20 + _DAT_0047a710;
  }
  *(double *)(iVar1 + 8) = local_20 * _DAT_0047a720 + *(double *)(iVar1 + 8);
  pdVar5 = (double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a710 <= *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200)) {
    *pdVar5 = *pdVar5 - _DAT_0047a710;
  }
  pdVar5 = (double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) <= 0.0) {
    *pdVar5 = *pdVar5 + _DAT_0047a710;
  }
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x68) =
       *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) * _DAT_0047a728 + _DAT_0047a730;
  pdVar5 = (double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200) < 0.0) {
    *pdVar5 = *pdVar5 + _DAT_0047a738;
  }
  pdVar5 = (double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a740 < *(double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200)) {
    *pdVar5 = *pdVar5 - _DAT_0047a738;
  }
  local_20 = *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) -
             *(double *)(g_VehicleConfigs + 0xb8 + g_ActiveVehicleIndex * 200);
  iVar1 = g_VehicleConfigs + g_ActiveVehicleIndex * 200;
  if (_DAT_0047a6f0 < local_20) {
    local_20 = local_20 - _DAT_0047a710;
  }
  if (local_20 < _DAT_0047a718) {
    local_20 = local_20 + _DAT_0047a710;
  }
  *(double *)(iVar1 + 0x28) = local_20 * _DAT_0047a720 + *(double *)(iVar1 + 0x28);
  pdVar5 = (double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a710 <= *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200)) {
    *pdVar5 = *pdVar5 - _DAT_0047a710;
  }
  pdVar5 = (double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) <= 0.0) {
    *pdVar5 = *pdVar5 + _DAT_0047a710;
  }
  *(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200) =
       *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) * _DAT_0047a728 + _DAT_0047a730;
  pdVar5 = (double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200) < 0.0) {
    *pdVar5 = *pdVar5 + _DAT_0047a738;
  }
  pdVar5 = (double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a740 < *(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200)) {
    *pdVar5 = *pdVar5 - _DAT_0047a738;
  }
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  uVar2 = *(unsigned int *)(iVar1 + 0x364);
  uVar7 = (int)uVar2 >> 0x1f;
  if (*(int *)(g_Vehicles + 0x36c + g_ActiveVehicleIndex * 0x484c) == 3) {
    iVar6 = (uVar2 ^ uVar7) - uVar7;
    local_20 = (*(double *)(g_pTrackRoadSequence + 4 + iVar6 * 0x18) +
               *(double *)(g_pTrackRoadSequence + 0x10 + iVar6 * 0x18)) * _DAT_0047a6e8;
  }
  else if ((int)uVar2 < 0) {
    iVar6 = (uVar2 ^ uVar7) - uVar7;
    local_20 = (double)CONCAT44(*(int *)(g_pTrackRoadSequence + 0x14 + iVar6 * 0x18),
                                *(int *)(g_pTrackRoadSequence + 0x10 + iVar6 * 0x18));
  }
  else {
    local_20 = (double)CONCAT44(*(int *)(g_pTrackRoadSequence + 8 + uVar2 * 0x18),
                                *(int *)(g_pTrackRoadSequence + 4 + uVar2 * 0x18));
  }
  if (*(int *)(iVar1 + 0x528) == 0) {
    fVar8 = (double)fcos((((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) -
                           (double)local_20) - (double)_DAT_0047a748) * (double)_DAT_0047a750 +
                          (double)_DAT_0047a6f0);
    local_8 = (double)((fVar8 + (double)_DAT_0047a758) * (double)_DAT_0047a760);
  }
  else {
    local_8 = 0.0;
  }
  if (*(int *)(iVar1 + 0x35c) == 0) {
    local_20 = *(double *)(g_VehicleConfigs + 0xb0 + g_ActiveVehicleIndex * 200) * _DAT_0047a768;
  }
  else {
    local_20 = 0.0;
  }
  pdVar5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  if ((_DAT_0047a738 < *(double *)(g_VehicleConfigs + 0x48 + g_ActiveVehicleIndex * 200)) ||
     (pdVar5[9] < _DAT_0047a770)) {
    _DAT_004949e0 = 0;
  }
  pdVar5[9] = ((((*pdVar5 - (double)*(int *)(pdVar5 + 10)) - pdVar5[0x10]) * _DAT_0047a6e8 +
                local_8 * _DAT_0047a778 + local_20) - pdVar5[9]) /
              (double)((unsigned long long)uStack_c << 0x20) + pdVar5[9];
  iVar1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  uStack_14 = 0x40240000;
  if (0.0 <= _DAT_00552e48) {
    uStack_14 = uStack_c;
  }
  uVar2 = *(unsigned int *)(iVar1 + 0x364);
  if ((int)uVar2 < 0) {
    iVar6 = *(int *)(g_pTrackRoadSequence + 0xc +
                    ((uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f)) * 0x18);
  }
  else {
    iVar6 = *(int *)(g_pTrackRoadSequence + uVar2 * 0x18);
  }
  iVar6 = *(int *)(DAT_00552e40 + 4 + iVar6 * 0xc);
  if (iVar6 < 1) {
    local_20 = 0.0;
  }
  else if (iVar6 < 0xb) {
    local_20 = (double)(iVar6 * -0x32);
  }
  else if ((10 < iVar6) && (iVar6 < 0x15)) {
    local_20 = (double)((iVar6 * 5 + -0x32) * 10);
  }
  pdVar5 = (double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) < local_20) {
    *pdVar5 = *(double *)(iVar1 + 0x118) * _DAT_0047a780 + *pdVar5;
  }
  pdVar5 = (double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200);
  if (local_20 < *(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200)) {
    *pdVar5 = *(double *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a788 + *pdVar5;
  }
  pdVar5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  *pdVar5 = ((double)*(int *)(g_VehicleConfigs + 0x50 + g_ActiveVehicleIndex * 200) -
            ((local_8 * _DAT_0047a790 - *(double *)(g_VehicleConfigs + 0x80 + g_ActiveVehicleIndex * 200)) +
             *(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) + *pdVar5)) /
            (double)((unsigned long long)uStack_14 << 0x20) + *pdVar5;
  dVar3 = *(double *)(g_VehicleConfigs + 0x80 + g_ActiveVehicleIndex * 200) + _DAT_0047a798;
  pdVar5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  if (*pdVar5 < dVar3) {
    *pdVar5 = dVar3;
  }
  iVar1 = g_VehicleConfigs + 0x54;
  iVar6 = g_ActiveVehicleIndex * 200;
  lVar9 = __ftol();
  *(int *)(iVar1 + iVar6) = (int)lVar9;
  return;
}

/**
 * @original Car_UpdateDynamicObjects (IGN_WIN.EXE @ 0x0043d540, fx.c)
 * @fidelity ADAPTED
 */
void Car_UpdateDynamicObjects(int car_idx) {
  int iVar1;
  int iVar2;
  int iVar3;
  int *puVar4;
  int iVar5;
  int *piVar6;
  double fVar7;
  long long lVar8;
  int *local_20;
  int local_c;
  iVar3 = car_idx * 0x20;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 4 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 8 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 0xc + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 0x10 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 0x14 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_005db02c + 0x18 + iVar3) = (int)lVar8;
  if ((DAT_0054f930 == 0) && (iVar1 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_005db02c + iVar3)), iVar1 != 0)) {
    Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
  }
  iVar1 = g_Vehicles + car_idx * 0x484c;
  fVar7 = (double)*(double *)(iVar1 + 0xb0) * (double)_DAT_0047a7e0;
  iVar5 = 0;
  fcos(fVar7);
  fsin(fVar7);
  if (*(int *)(iVar1 + 0x558) == 0) {
    piVar6 = (int *)(g_PlayerHUDState + car_idx * 0x4c);
    if (0 < *(int *)(*piVar6 * 0x30 + DAT_00563da4)) {
      puVar4 = &DAT_005537d8 + car_idx * 0x820;
      do {
        iVar5 = iVar5 + 1;
        lVar8 = __ftol();
        *puVar4 = (int)lVar8;
        lVar8 = __ftol();
        iVar1 = DAT_00563da4;
        puVar4[2] = (int)lVar8;
        puVar4 = puVar4 + 3;
      } while (iVar5 < *(int *)(*piVar6 * 0x30 + iVar1));
    }
    iVar1 = 0;
    if (0 < *(int *)(*piVar6 * 0x30 + 0xc + DAT_00563da4)) {
      puVar4 = (int *)(&DAT_00553ff8 + car_idx * 0x2080);
      do {
        iVar1 = iVar1 + 1;
        lVar8 = __ftol();
        *puVar4 = (int)lVar8;
        lVar8 = __ftol();
        iVar5 = DAT_00563da4;
        puVar4[2] = (int)lVar8;
        puVar4 = puVar4 + 3;
      } while (iVar1 < *(int *)(*piVar6 * 0x30 + 0xc + iVar5));
    }
  }
  else {
    iVar1 = 0;
    local_20 = &DAT_005537d8 + car_idx * 0x820;
    piVar6 = (int *)(g_PlayerHUDState + car_idx * 0x4c);
    do {
      iVar5 = 0;
      puVar4 = local_20;
      if (0 < *(int *)(DAT_00563da4 + (iVar1 + *piVar6 * 4) * 0xc)) {
        do {
          lVar8 = __ftol();
          *puVar4 = (int)lVar8;
          lVar8 = __ftol();
          puVar4[1] = (int)lVar8;
          lVar8 = __ftol();
          puVar4[2] = (int)lVar8;
          iVar5 = iVar5 + 1;
          puVar4 = puVar4 + 3;
        } while (iVar5 < *(int *)(DAT_00563da4 + (iVar1 + *piVar6 * 4) * 0xc));
      }
      iVar1 = iVar1 + 1;
      local_20 = local_20 + 0x208;
    } while (iVar1 < 4);
  }
  local_c = car_idx * 0x4c;
  iVar1 = 4;
  iVar5 = car_idx << 7;
  do {
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 4 + iVar5) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 8 + iVar5) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 0xc + iVar5) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 0x10 + iVar5) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 0x14 + iVar5) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(g_RaceFinished + 0x18 + iVar5) = (int)lVar8;
    iVar2 = Lisa_UpdateObjectSpatialGrid((int *)(g_RaceFinished + iVar5));
    if (iVar2 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_WHE_00499580);
    }
    iVar5 = iVar5 + 0x20;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if ((*(int *)(g_PlayerHUDState + local_c) == 7) && (DAT_0054f930 == 0)) {
    lVar8 = __ftol();
    *(int *)(iVar3 + 4 + DAT_00552f70) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(iVar3 + 8 + DAT_00552f70) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(iVar3 + 0xc + DAT_00552f70) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(iVar3 + 0x10 + DAT_00552f70) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(iVar3 + 0x14 + DAT_00552f70) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(iVar3 + 0x18 + DAT_00552f70) = (int)lVar8;
    if ((DAT_0054f930 == 0) && (iVar1 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_00552f70 + iVar3)), iVar1 != 0)) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
    }
  }
  lVar8 = __ftol();
  ((int*)&(DAT_00603718))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_00603720))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_00603724))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_0060372c))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_00603730))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_00603738))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_0060373c))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  ((int*)&(DAT_00603744))[car_idx * 0x1e] = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 4 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 8 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 0xc + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 0x10 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 0x14 + iVar3) = (int)lVar8;
  lVar8 = __ftol();
  *(int *)(DAT_00552f20 + 0x18 + iVar3) = (int)lVar8;
  if ((DAT_0054f930 == 0) && (iVar1 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_00552f20 + iVar3)), iVar1 != 0)) {
    Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_SHA_00499558);
  }
  if ((*(int *)(g_PlayerHUDState + 4 + local_c) == 2) && (car_idx != 0)) {
    lVar8 = __ftol();
    *(int *)(DAT_00553788 + 4 + iVar3) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(DAT_00553788 + 8 + iVar3) = (int)lVar8;
    lVar8 = __ftol();
    *(int *)(DAT_00553788 + 0xc + iVar3) = (int)lVar8;
    *(int *)(DAT_00553788 + 0x10 + iVar3) = 0;
    *(int *)(DAT_00553788 + 0x14 + iVar3) = 0;
    *(int *)(DAT_00553788 + 0x18 + iVar3) = 0;
    piVar6 = (int *)(DAT_00553788 + iVar3);
    if (piVar6[7] == 1) {
      iVar3 = Lisa_UpdateObjectSpatialGrid(piVar6);
      if (iVar3 != 0) {
        Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_00499530);
        return;
      }
    }
    else {
      iVar3 = Lisa_MoveDynamicObject(piVar6);
      if (iVar3 != 0) {
        Log_DebugPrintf(s_FEL_VID_LI_PLACEOBJECT_HANDLE_CA_00499508);
      }
    }
  }
  return;
}

/**
 * @original Video_SetGraphicsMode (IGN_WIN.EXE @ 0x0043dea0, fx.c)
 * @fidelity ADAPTED
 */
int Video_SetGraphicsMode(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  unsigned int uVar4;
  unsigned int uVar5;
  int *puVar6;
  long long uVar7;
  if (DAT_00552fc0 == 0) {
    DAT_004ba6e0 = 0x140;
    DAT_004ba6e4 = 200;
    DAT_004ba6e8 = 8;
    DAT_004ba6ec = 1;
    iVar2 = Gfx_RestoreSurface();
    if (iVar2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    iVar2 = 0;
    g_ScreenHeight = 200;
    g_ScreenWidth = 0x140;
    if (0 < g_NumRacers) {
      iVar3 = 0;
      do {
        iVar2 = iVar2 + 1;
        *(int *)(g_VehicleConfigs + 0x58 + iVar3) = 200;
        *(int *)(g_VehicleConfigs + 0x5c + iVar3) = 0xa4;
        iVar1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + 0x60 + iVar3) = 0;
        *(int *)(iVar1 + 100 + iVar3) = 0x40390000;
        iVar3 = iVar3 + 200;
      } while (iVar2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if ((((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && (DAT_0054f98c == 1)) && (g_HudEnabled == 1)
       ) {
      uVar7 = Palette_AdjustRGB(extraout_ECX,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)uVar7);
    }
  }
  if (DAT_00552fc0 == 1) {
    DAT_004ba6e0 = 0x280;
    DAT_004ba6e4 = 0x1e0;
    DAT_004ba6e8 = 8;
    DAT_004ba6ec = 1;
    iVar2 = Gfx_RestoreSurface();
    if (iVar2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    iVar2 = 0;
    g_ScreenWidth = 0x280;
    g_ScreenHeight = 0x1e0;
    if (0 < g_NumRacers) {
      iVar3 = 0;
      do {
        iVar3 = iVar3 + 200;
        *(int *)(g_VehicleConfigs + -0x70 + iVar3) = 0x1a9;
        iVar2 = iVar2 + 1;
        *(int *)(g_VehicleConfigs + -0x6c + iVar3) = 0x1a9;
        iVar1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + -0x68 + iVar3) = 0;
        *(int *)(iVar1 + -100 + iVar3) = 0x40240000;
      } while (iVar2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if (((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && ((DAT_0054f98c == 1 && (g_HudEnabled == 1)))
       ) {
      uVar7 = Palette_AdjustRGB(extraout_ECX_00,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)uVar7);
    }
  }
  if (DAT_00552fc0 == 2) {
    DAT_004ba6e0 = 800;
    DAT_004ba6e4 = 600;
    DAT_004ba6e8 = 8;
    DAT_004ba6ec = 1;
    iVar2 = Gfx_RestoreSurface();
    if (iVar2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    iVar2 = 0;
    g_ScreenWidth = 800;
    g_ScreenHeight = 600;
    if (0 < g_NumRacers) {
      iVar3 = 0;
      do {
        iVar3 = iVar3 + 200;
        *(int *)(g_VehicleConfigs + -0x70 + iVar3) = 0x226;
        iVar2 = iVar2 + 1;
        *(int *)(g_VehicleConfigs + -0x6c + iVar3) = 0x226;
        iVar1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + -0x68 + iVar3) = 0;
        *(int *)(iVar1 + -100 + iVar3) = 0x40240000;
      } while (iVar2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if (((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && ((DAT_0054f98c == 1 && (g_HudEnabled == 1)))
       ) {
      uVar7 = Palette_AdjustRGB(extraout_ECX_01,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)uVar7);
    }
  }
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  uVar5 = g_ScreenWidth * g_ScreenHeight;
  if (0 < (int)uVar5) {
    puVar6 = &g_VirtualFramebuffer;
    for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar6 = 0;
      puVar6 = puVar6 + 1;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(char *)puVar6 = 0;
      puVar6 = (int *)((int)puVar6 + 1);
    }
  }
  Lisa_Init();
  Timer_GetDeltaTime();
  return 0;
}

/**
 * @original Lisa_Init (IGN_WIN.EXE @ 0x0043e2a0, fx.c)
 * @fidelity ADAPTED
 */
void Lisa_Init(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  long long lVar5;
  unsigned long long uVar6;
  long long uVar7;
  lVar5 = __ftol();
  DAT_005285c4 = (int)lVar5;
  lVar5 = __ftol();
  DAT_00552f50 = (int)lVar5;
  lVar5 = __ftol();
  DAT_00552d80 = (int)lVar5;
  lVar5 = __ftol();
  DAT_00552f5c = (int)lVar5;
  iVar2 = 0;
  if ((g_IsSplitScreen == 0) || (DAT_00552f58 == 1)) {
    iVar2 = 0;
    iVar4 = 0;
    if (0 < g_NumRacers) {
      do {
        iVar2 = iVar2 + 0x4c;
        iVar4 = iVar4 + 1;
        *(int *)(g_PlayerHUDState + -0x20 + iVar2) = (int)g_ScreenWidth / 2 - DAT_005285c4 / 2;
        *(int *)(g_PlayerHUDState + -0x1c + iVar2) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
        *(int *)(g_PlayerHUDState + -0x18 + iVar2) = DAT_005285c4 / 2 + (int)g_ScreenWidth / 2;
        *(int *)(g_PlayerHUDState + -0x14 + iVar2) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
      } while (iVar4 < g_NumRacers);
    }
  }
  else {
    iVar4 = 0;
    if (0 < g_NumRacers) {
      iVar3 = 0;
      do {
        *(int *)(g_PlayerHUDState + 0x2c + iVar2) =
             (((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) * 3 - DAT_005285c4 / 2)
             + 1;
        *(int *)(g_PlayerHUDState + 0x30 + iVar2) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
        *(int *)(g_PlayerHUDState + 0x34 + iVar2) =
             ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) * 3 + DAT_005285c4 / 2;
        *(int *)(g_PlayerHUDState + 0x38 + iVar2) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
        *(int *)(g_VehicleConfigs + 0x58 + iVar3) = *(int *)(g_VehicleConfigs + 0x58);
        *(int *)(g_VehicleConfigs + 0x5c + iVar3) = *(int *)(g_VehicleConfigs + 0x5c);
        iVar1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + 100 + iVar3) = *(int *)(g_VehicleConfigs + 100);
        iVar3 = iVar3 + 200;
        iVar2 = iVar2 + 0x4c;
        iVar4 = iVar4 + 1;
        *(int *)(iVar1 + -0x68 + iVar3) = *(int *)(iVar1 + 0x60);
      } while (iVar4 < g_NumRacers);
    }
    *(int *)(g_PlayerHUDState + 0x78) =
         ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) - DAT_005285c4 / 2;
    *(int *)(g_PlayerHUDState + 0x7c) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
    *(int *)(g_PlayerHUDState + 0x80) =
         DAT_005285c4 / 2 + ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) + -1;
    *(int *)(g_PlayerHUDState + 0x84) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
  }
  uVar6 = Lisa_SetCameraViewport();
  g_ViewportMinX = 0;
  g_ViewportMaxX = g_ScreenWidth - 1;
  g_ViewportMinY = 0;
  g_ViewportMaxY = g_ScreenHeight + -1;
  uVar7 = Audio_LoadAssets(g_ScreenWidth,(unsigned int)(uVar6 >> 0x20));
  if (-1 < (int)uVar7) {
    _DAT_00498730 = g_pLisaDrawCommandQueue;
    _DAT_0049873c = g_pActiveTAB;
    _DAT_00498734 = &g_VirtualFramebuffer;
    _DAT_00498738 = &g_VirtualFramebuffer;
    Audio_StopSample();
    uVar7 = Audio_LoadAssets(g_ScreenHeight,g_ScreenWidth);
    if (-1 < (int)uVar7) {
      _DAT_00498478 = DAT_00552d80 * 500;
      _DAT_0049847c = DAT_00552f5c * 500;
      return;
    }
    Log_DebugPrintf(s_Error_while_initializing_lisaGM_004995c8);
                    /* WARNING: Subroutine does not return */
    _exit(1);
  }
  Log_DebugPrintf(s_Error_while_initializing_lisaGM_004995c8);
                    /* WARNING: Subroutine does not return */
  _exit(1);
}

/**
 * @original Video_FlipScreen (IGN_WIN.EXE @ 0x0043e6c0, fx.c)
 * @fidelity ADAPTED
 */

/**
 * @original HUD_RenderTelemetryOverlay (IGN_WIN.EXE @ 0x0043e9c0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderTelemetryOverlay(void) {
  char local_24 [32];
  int local_4;
  if (DAT_004949b4 != 0) {
    _sprintf(local_24,s_FPS__d_00499658, 0);
    Font_PrintDirect(0x10e,0,local_24,(void*)0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  }
  if (DAT_004949dc == 1) {
    _sprintf(local_24,s_RECORDING__0049964c);
    Font_PrintDirect(0x96,10,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  }
  if (DAT_005532b0 == 1) {
    local_4 = DAT_00563bec / 10;
    _sprintf(local_24,s_ROLL___1f_00499640,(double)local_4);
    Font_PrintDirect(5,0x1e,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
    return;
  }
  _sprintf(local_24,s_OLDROADNR__d_00499630);
  Font_PrintDirect(5,10,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(local_24,s_SQUASHED2__d_00499620);
  Font_PrintDirect(5,0x14,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(local_24,s_TYPE1__d_00499614);
  Font_PrintDirect(0,0x28,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(local_24,s_TYPE2__d_00499608);
  Font_PrintDirect(0,0x32,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(local_24,s_SKILLNAD___3f_004995f8,
           ABS(*(double *)(g_Vehicles + 0x128) - *(double *)(g_Vehicles + 0x120)));
  Font_PrintDirect(0,0x3c,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(local_24,s_SPEED___3f_004995ec,ABS(*(double *)(g_Vehicles + 0x118)));
  Font_PrintDirect(0,0x46,local_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  return;
}

/**
 * @original HUD_CheckWrongWayHeading (IGN_WIN.EXE @ 0x0043edf0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_CheckWrongWayHeading(int player_idx) {
  int iVar1;
  int iVar2;
  int *piVar3;
  double dVar4;
  unsigned int uVar5;
  double fVar6;
  double fVar7;
  long long lVar8;
  iVar1 = player_idx * 0x484c;
  uVar5 = *(unsigned int *)(g_Vehicles + 0x364 + player_idx * 0x484c);
  if ((int)uVar5 < 0) {
    dVar4 = *(double *)
             (g_pTrackRoadSequence + 0x10 + ((uVar5 ^ (int)uVar5 >> 0x1f) - ((int)uVar5 >> 0x1f)) * 0x18);
  }
  else {
    dVar4 = *(double *)(g_pTrackRoadSequence + 4 + uVar5 * 0x18);
  }
  fVar6 = Math_WrapAngle(dVar4 - _DAT_0047a888,6.2831853071796);
  fVar7 = Math_WrapAngle((double)CONCAT44(*(int *)(g_Vehicles + 0xfc + iVar1),
                                        *(int *)(g_Vehicles + 0xf8 + iVar1)),
                       6.2831853071796);
  if (((double)ABS((double)(double)fVar6 - fVar7) <= _DAT_0047a898) ||
     (_DAT_0047a8a0 <= (double)ABS((double)(double)fVar6 - fVar7))) {
    piVar3 = (int *)(g_Vehicles + 0x580 + iVar1);
    if (0 < *piVar3) {
      lVar8 = __ftol();
      *piVar3 = (int)lVar8;
    }
  }
  else {
    iVar2 = g_Vehicles + 0x580;
    lVar8 = __ftol();
    *(int *)(iVar2 + iVar1) = (int)lVar8;
    if (0x78 < *(int *)(g_Vehicles + 0x580 + iVar1)) {
      *(int *)(g_Vehicles + 0x580 + iVar1) = 0x78;
      return;
    }
  }
  return;
}

/**
 * @original HUD_RenderPlayerElements (IGN_WIN.EXE @ 0x0043ef30, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderPlayerElements(int player_idx) {
  int *puVar1;
  int iVar2;
  char *pcVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  unsigned int uVar7;
  int iVar8;
  double *pdVar9;
  double fVar10;
  long long lVar11;
  int *piVar12;
  int iVar13;
  int uVar14;
  char *puVar15;
  int local_80;
  int local_7c;
  int local_74;
  int local_70;
  int local_6c;
  int local_68;
  int local_64;
  double local_60 [8];
  char local_20 [32];
  local_68 = player_idx * 0x4c;
  iVar4 = g_PlayerHUDState + local_68;
  Gfx_SetClipRect(*(int *)(iVar4 + 0x2c),*(int *)(iVar4 + 0x30),
               *(int *)(iVar4 + 0x34),*(int *)(iVar4 + 0x38));
  if (((*(unsigned int *)(g_VehicleConfigs + 0x3c + player_idx * 200) & 0x7fffffff) == 0 &&
       *(int *)(g_VehicleConfigs + 0x38 + player_idx * 200) == 0) || (g_IsDemoMode == 1)) {
    iVar8 = g_PlayerHUDState + local_68;
    iVar4 = *(int *)(iVar8 + 0x2c);
    local_74 = *(int *)(iVar8 + 0x30);
    iVar6 = *(int *)(iVar8 + 0x34);
    local_6c = *(int *)(iVar8 + 0x38);
  }
  else {
    fVar10 = Math_RandomFloat0To1();
    if (fVar10 < (double)_DAT_0047a8b0) {
      iVar8 = g_PlayerHUDState + local_68;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x2c);
      lVar11 = __ftol();
      iVar4 = (int)lVar11;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x30);
      lVar11 = __ftol();
      local_74 = (int)lVar11;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x34);
      lVar11 = __ftol();
      iVar6 = (int)lVar11;
      local_60[0] = (double)CONCAT44(((int*)&(local_60[0]))[1],*(int *)(iVar8 + 0x38));
      lVar11 = __ftol();
      local_6c = (int)lVar11;
    }
    else {
      iVar8 = g_PlayerHUDState + local_68;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x2c);
      lVar11 = __ftol();
      iVar4 = (int)lVar11;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x30);
      lVar11 = __ftol();
      local_74 = (int)lVar11;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar8 + 0x34);
      lVar11 = __ftol();
      iVar6 = (int)lVar11;
      local_60[0] = (double)CONCAT44(((int*)&(local_60[0]))[1],*(int *)(iVar8 + 0x38));
      lVar11 = __ftol();
      local_6c = (int)lVar11;
    }
  }
  if (g_GameMode != 3) {
    iVar8 = iVar4 + 7;
    local_7c = (local_74 + 7) * 0x100;
    local_80 = iVar8 * 0x100;
    Gfx_DrawSprite(DAT_005285e0,&local_80,0);
    DAT_0063f2d8 = g_pLisaDrawCommandWritePtr;
    iVar5 = iVar8;
    if (_DAT_00552e48 == _DAT_0047a8c8) {
      iVar2 = *g_SpeedoPosition + local_74;
      iVar13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      pcVar3 = HUD_FormatLapTime();
    }
    else {
      iVar2 = *g_SpeedoPosition + local_74;
      iVar13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      pcVar3 = s__________00499730;
    }
    Font_DrawText(pcVar3,iVar13,iVar5,iVar2);
    local_7c = (*g_SpeedoConfig + local_74) * 0x100;
    local_80 = iVar8 * 0x100;
    Gfx_DrawSprite(DAT_005285e4,&local_80,0);
    iVar5 = iVar8;
    if (_DAT_00552e48 == _DAT_0047a8c8) {
      if (*(int *)(g_Vehicles + 0x374 + player_idx * 0x484c) == 0) {
        iVar2 = g_SpeedoPosition[1] + local_74;
        iVar13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
        pcVar3 = HUD_FormatLapTime();
      }
      else {
        iVar2 = g_SpeedoPosition[1] + local_74;
        iVar13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
        pcVar3 = HUD_FormatLapTime();
      }
    }
    else {
      iVar2 = g_SpeedoPosition[1] + local_74;
      iVar13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      pcVar3 = s__________00499730;
    }
    Font_DrawText(pcVar3,iVar13,iVar5,iVar2);
    iVar5 = *(int *)(g_Vehicles + player_idx * 0x484c + 0x374);
    if (iVar5 == 1) {
      iVar13 = g_SpeedoPosition[2];
      iVar5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
LAB_0043f316:
      iVar13 = iVar13 + local_74;
      iVar2 = iVar8;
      pcVar3 = HUD_FormatLapTime();
      Font_DrawText(pcVar3,iVar5,iVar2,iVar13);
    }
    else if (1 < iVar5) {
      iVar13 = g_SpeedoPosition[2];
      iVar5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      goto LAB_0043f316;
    }
    iVar5 = *(int *)(g_Vehicles + player_idx * 0x484c + 0x374);
    if (iVar5 == 2) {
      iVar13 = g_SpeedoPosition[3];
      iVar5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
    }
    else {
      if (iVar5 < 3) goto LAB_0043f3a7;
      iVar13 = g_SpeedoPosition[3];
      iVar5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
    }
    iVar13 = iVar13 + local_74;
    pcVar3 = HUD_FormatLapTime();
    Font_DrawText(pcVar3,iVar5,iVar8,iVar13);
  }
LAB_0043f3a7:
  if ((1 < g_NumRacers) || (DAT_00553068 == 1)) {
    if (g_GameMode == 3) {
      iVar8 = g_Vehicles + player_idx * 0x484c;
      if (*(int *)(g_Vehicles + 0x528 + player_idx * 0x484c) != 0) {
        uVar14 = *(int *)(iVar8 + 0x39c);
        goto LAB_0043f498;
      }
      _sprintf(local_20,s__d__d_00499728,*(int *)(iVar8 + 0x39c),g_PlayerCarModel);
    }
    else {
      local_80 = (g_SpeedoConfig[1] + iVar4) * 0x100;
      local_7c = (g_SpeedoConfig[2] + local_6c) * 0x100;
      Gfx_DrawSprite(DAT_005285e8,&local_80,0);
      iVar8 = g_Vehicles + player_idx * 0x484c;
      if (*(int *)(g_Vehicles + 0x528 + player_idx * 0x484c) == 0) {
        uVar14 = *(int *)(iVar8 + 0x39c);
      }
      else {
        uVar14 = *(int *)(iVar8 + 0x3a0);
      }
LAB_0043f498:
      _sprintf(local_20,&DAT_00493c78,uVar14);
    }
    Font_DrawText(local_20,((int*)&(DAT_0052863c))[g_ActiveFontColor],g_SpeedoPosition[4] + iVar4,
                 g_SpeedoPosition[5] + local_6c);
  }
  HUD_RenderSpeedometerGauge(player_idx);
  iVar4 = player_idx * 0x484c;
  iVar5 = g_Vehicles + iVar4;
  iVar8 = *(int *)(iVar5 + 0x27c) << 2;
  local_60[0] = (double)CONCAT44(((int*)&(local_60[0]))[1],iVar8);
  if (*(double *)(iVar5 + 0x280) < (double)iVar8) {
    *(double *)(iVar5 + 0x280) = _DAT_00563d80 * _DAT_0047a8b0 + *(double *)(iVar5 + 0x280);
    iVar8 = iVar4 + g_Vehicles;
    local_60[0] = (double)(*(int *)(iVar8 + 0x27c) << 2);
    if (local_60[0] < *(double *)(iVar8 + 0x280)) {
      *(double *)(iVar8 + 0x280) = local_60[0];
    }
  }
  iVar5 = iVar4 + g_Vehicles;
  iVar8 = *(int *)(iVar5 + 0x27c) << 2;
  local_60[0] = (double)CONCAT44(((int*)&(local_60[0]))[1],iVar8);
  if ((double)iVar8 < *(double *)(iVar5 + 0x280)) {
    *(double *)(iVar5 + 0x280) = _DAT_00563d80 * _DAT_0047a8d0 + *(double *)(iVar5 + 0x280);
    iVar8 = iVar4 + g_Vehicles;
    local_60[0] = (double)(*(int *)(iVar8 + 0x27c) << 2);
    if (*(double *)(iVar8 + 0x280) < local_60[0]) {
      *(double *)(iVar8 + 0x280) = local_60[0];
    }
  }
  uVar14 = 0;
  local_80 = (g_SpeedoConfig[3] + iVar6) * 0x100;
  piVar12 = &local_80;
  local_7c = (g_SpeedoConfig[4] + local_6c) * 0x100;
  lVar11 = __ftol();
  Gfx_DrawSprite(((int*)&(DAT_005285f4))[(int)lVar11],piVar12,uVar14);
  iVar8 = local_6c;
  if (g_ActiveFontColor == 0) {
    local_60[0] = 4.2;
  }
  if (g_ActiveFontColor == 1) {
    local_60[0] = 2.1;
  }
  if (g_ActiveFontColor == 2) {
    local_60[0] = 1.0;
  }
  *g_pLisaDrawCommandQueue = g_LisaDrawCommandBuffer;
  g_pLisaDrawCommandQueue[1] = g_LisaDrawCommandBuffer + 0xc;
  g_pLisaDrawCommandQueue[2] = 0;
  *g_LisaDrawCommandBuffer = 0xf;
  g_LisaDrawCommandBuffer[1] = (g_SpeedoConfig[0x11] + iVar6) * 0x100;
  g_LisaDrawCommandBuffer[2] = (g_SpeedoConfig[0x12] + local_6c) * 0x100;
  g_LisaDrawCommandBuffer[3] = 0;
  g_LisaDrawCommandBuffer[4] = (g_SpeedoConfig[0x11] + iVar6) * 0x100;
  local_70 = g_SpeedoConfig[0x12] + local_6c;
  lVar11 = __ftol();
  iVar5 = local_6c;
  g_LisaDrawCommandBuffer[5] = (int)lVar11 << 8;
  g_LisaDrawCommandBuffer[6] = 0;
  g_LisaDrawCommandBuffer[7] = (g_SpeedoConfig[0x13] + iVar6) * 0x100;
  g_LisaDrawCommandBuffer[8] = (g_SpeedoConfig[0x12] + iVar8) * 0x100;
  g_LisaDrawCommandBuffer[9] = 0;
  g_LisaDrawCommandBuffer[10] = 0xd7;
  g_LisaDrawCommandBuffer[0xb] = 0;
  g_LisaDrawCommandBuffer[0xc] = 0xf;
  g_LisaDrawCommandBuffer[0xd] = (g_SpeedoConfig[0x13] + iVar6) * 0x100;
  g_LisaDrawCommandBuffer[0xe] = (g_SpeedoConfig[0x12] + local_6c) * 0x100;
  g_LisaDrawCommandBuffer[0xf] = 0;
  g_LisaDrawCommandBuffer[0x10] = (g_SpeedoConfig[0x11] + iVar6) * 0x100;
  local_70 = g_SpeedoConfig[0x12] + local_6c;
  lVar11 = __ftol();
  g_LisaDrawCommandBuffer[0x11] = (int)lVar11 << 8;
  g_LisaDrawCommandBuffer[0x12] = 0;
  g_LisaDrawCommandBuffer[0x13] = (g_SpeedoConfig[0x13] + iVar6) * 0x100;
  ((int*)&(local_60[0]))[0] = g_SpeedoConfig[0x12] + iVar5;
  lVar11 = __ftol();
  g_LisaDrawCommandBuffer[0x14] = (int)lVar11 << 8;
  g_LisaDrawCommandBuffer[0x15] = 0;
  g_LisaDrawCommandBuffer[0x16] = 0xd7;
  puVar1 = g_LisaDrawCommandBuffer;
  g_LisaDrawCommandBuffer[0x17] = 0;
  Lisa_FlushRasterizerCommands(puVar1,(unsigned int)((unsigned long long)lVar11 >> 0x20));
  local_80 = (g_SpeedoConfig[0xd] + iVar6) * 0x100;
  local_7c = (g_SpeedoConfig[0xe] + local_6c) * 0x100;
  Gfx_DrawSprite(DAT_00528618,&local_80,0);
  iVar8 = g_Vehicles + iVar4;
  if (*(int *)(iVar8 + 0x53c) == 0) {
    if ((*(double *)(iVar8 + 0x534) < _DAT_0047a8f0) ||
       (_DAT_0047a8f8 <= *(double *)(iVar8 + 0x534))) {
      if (*(double *)(iVar8 + 0x534) < _DAT_0047a8f8) goto LAB_0043fa85;
      if ((g_IsDemoMode == 0) || (DAT_00552f10 != 2)) {
        *(int *)(iVar8 + 0x540) = *(int *)(iVar8 + 0x540) + 1;
        if (0xe < *(int *)(g_Vehicles + 0x540 + iVar4)) {
          *(int *)(g_Vehicles + 0x540 + iVar4) = 0;
        }
      }
      if (*(int *)(g_Vehicles + 0x540 + iVar4) < 7) {
        local_80 = (g_SpeedoConfig[0xf] + iVar6) * 0x100;
        local_7c = (g_SpeedoConfig[0x10] + local_6c) * 0x100;
        uVar14 = DAT_00528620;
      }
      else {
        local_80 = (g_SpeedoConfig[0xf] + iVar6) * 0x100;
        local_7c = (g_SpeedoConfig[0x10] + local_6c) * 0x100;
        uVar14 = DAT_0052861c;
      }
    }
    else {
      local_80 = (g_SpeedoConfig[0xf] + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x10] + local_6c) * 0x100;
      uVar14 = DAT_0052861c;
    }
LAB_0043fb28:
    Gfx_DrawSprite(uVar14,&local_80,0);
  }
  else {
LAB_0043fa85:
    if ((0 < *(int *)(iVar8 + 0x53c)) || (*(double *)(iVar8 + 0x534) < _DAT_0047a8f0)) {
      if ((g_IsDemoMode == 0) || (DAT_00552f10 != 2)) {
        *(int *)(iVar8 + 0x540) = *(int *)(iVar8 + 0x540) + 1;
        if (1 < *(int *)(g_Vehicles + 0x540 + iVar4)) {
          *(int *)(g_Vehicles + 0x540 + iVar4) = 0;
        }
      }
      local_80 = (g_SpeedoConfig[0xf] + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x10] + local_6c) * 0x100;
      uVar14 = ((int*)&(DAT_0052861c))[*(int *)(g_Vehicles + 0x540 + iVar4)];
      goto LAB_0043fb28;
    }
  }
  iVar6 = iVar4 + g_Vehicles;
  if ((*(int *)(iVar6 + 0x528) == 0) && (_DAT_00552e48 == _DAT_0047a8c8)) {
    uVar7 = *(unsigned int *)(iVar6 + 0x364);
    if ((int)uVar7 < 0) {
      iVar8 = *(int *)(g_pTrackRoadSequence + 0xc +
                      ((uVar7 ^ (int)uVar7 >> 0x1f) - ((int)uVar7 >> 0x1f)) * 0x18);
    }
    else {
      iVar8 = *(int *)(g_pTrackRoadSequence + uVar7 * 0x18);
    }
    if ((*(int *)(iVar6 + 0x580) < 0x1a) || (*(int *)(iVar6 + 0x368) <= (int)uVar7)) {
      if (*(int *)(DAT_00552e40 + 8 + iVar8 * 0xc) < 1) {
        *(int *)(iVar6 + 0x5f8) = 0;
      }
      else {
        *(int *)(iVar6 + 0x5f8) = *(int *)(iVar6 + 0x5f8) + 1;
        iVar6 = *(int *)(g_Vehicles + 0x5f8 + iVar4);
        iVar5 = iVar6 + (iVar6 >> 0x1f & 3U);
        uVar7 = iVar5 >> 0x1f;
        if ((((iVar5 >> 2 ^ uVar7) - uVar7 & 1 ^ uVar7) == uVar7) || (0x17 < iVar6)) {
          iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
          local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 -
                      *(int *)(DAT_00563d8c + 0x228) / 2) + iVar6) * 0x100;
          local_7c = (g_SpeedoConfig[0x1b] + local_74) * 0x100;
          iVar6 = *(int *)(DAT_00552e40 + 8 + iVar8 * 0xc);
          if (0x10 < iVar6) {
            iVar6 = 0x10;
          }
          Gfx_DrawSprite(((int*)&(DAT_00528664))[*(int *)(&DAT_00497d74 + (DAT_00563ce8 * 0x14 + iVar6) * 4)],
                       &local_80,0);
        }
      }
    }
    else {
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 -
                  *(int *)(DAT_00563d8c + 0x228) / 2) + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x1b] + local_74) * 0x100;
      Gfx_DrawSprite(DAT_005286a0,&local_80,0);
    }
  }
  if (g_GameMode == 3) {
    if (((*(int *)(g_Vehicles + iVar4 + 0x39c) == g_PlayerCarModel) &&
        (*(int *)(g_Vehicles + iVar4 + 0x528) == 0)) &&
       ((g_IsDemoMode == 0 || (DAT_00552f10 != 2)))) {
      if (g_LanguageId == 0) {
        pcVar3 = s_WARNING__YOU_RE_LAST__00499710;
LAB_0043fda0:
        _sprintf(local_20,pcVar3);
      }
      else {
        if (g_LanguageId == 1) {
          pcVar3 = s_SIE_SIND_LETZTER__004996fc;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 2) {
          pcVar3 = s_ATTENZIONE__SEI_ULTIMO_004996e4;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 3) {
          pcVar3 = s_AVISO__ERES_EL_ULTIMO_004996cc;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 4) {
          pcVar3 = s_VARNING__DU_LIGGER_SIST__004996b0;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 5) {
          pcVar3 = s_ATTENTION__VOUS_ETES_DERNIER__00499690;
          goto LAB_0043fda0;
        }
      }
      if (player_idx == 0) {
        if (g_IsSplitScreen == 0) {
          iVar6 = -1;
        }
        else {
          if (g_IsSplitScreen != 1) goto LAB_0043fdde;
          iVar6 = 0;
        }
      }
      else {
LAB_0043fdde:
        if ((player_idx != 1) || (g_IsSplitScreen != 1)) goto LAB_0043fe07;
        iVar6 = 1;
      }
      HUD_AddFloatingMessage(local_20,0,1,iVar6);
    }
LAB_0043fe07:
    if (DAT_00552fc0 == 0) {
      iVar8 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x30);
      uVar14 = DAT_005287ec;
    }
    else {
      iVar8 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x30);
      uVar14 = DAT_005287f0;
    }
    local_7c = (iVar6 + 8) * 0x100;
    local_80 = (iVar8 + 8) * 0x100;
    Gfx_DrawSprite(uVar14,&local_80,0);
    local_64 = (-(unsigned int)(DAT_00552fc0 == 0) & 0xffffff4c) + 300;
    iVar6 = g_NumRacers + -1;
    if (-1 < iVar6) {
      pdVar9 = local_60 + iVar6;
      iVar8 = iVar6 * 0x4c;
      iVar6 = iVar6 * 0x484c;
      do {
        if (_DAT_005285d8 < _DAT_0047a900) {
          *(int *)pdVar9 = 0;
          *(int *)((int)pdVar9 + 4) = 0;
        }
        else {
          *pdVar9 = ABS((double)*(int *)(g_Vehicles + 0x364 + iVar6)) / (double)DAT_005287f8;
        }
        if (*(int *)(g_Vehicles + 0x528 + iVar6) == 0) {
          iVar5 = g_PlayerHUDState + local_68;
          local_80 = (*(int *)(iVar5 + 0x2c) + 5) * 0x100;
          lVar11 = __ftol();
          local_70 = (int)lVar11;
          local_7c = ((*(int *)(iVar5 + 0x30) - local_70) + local_64 + 5) * 0x100;
          Gfx_DrawSprite(((int*)&(DAT_00528764))[*(int *)(g_PlayerHUDState + iVar8)],&local_80,0);
          if (*(int *)(g_Vehicles + 0x39c + iVar6) == 1) {
            local_80 = (*(int *)(local_68 + g_PlayerHUDState + 0x2c) + 0x13) * 0x100;
            local_7c = ((*(int *)(local_68 + g_PlayerHUDState + 0x30) - local_70) + local_64 + 8) *
                       0x100;
            Gfx_DrawSprite(DAT_005287f4,&local_80,0);
          }
        }
        iVar8 = iVar8 + -0x4c;
        iVar6 = iVar6 + -0x484c;
        pdVar9 = pdVar9 + -1;
      } while (local_60 <= pdVar9);
    }
    if (-1 < g_NumRacers + -1) {
      iVar6 = (g_NumRacers + -1) * 0x484c;
      do {
        iVar8 = g_Vehicles + iVar6;
        if (((*(int *)(iVar8 + 0x528) == 1) && (*(double *)(iVar8 + 0x60c) < _DAT_0047a908)) &&
           (*(int *)(iVar8 + 0x3a0) != 1)) {
          local_80 = *(int *)(local_68 + g_PlayerHUDState + 0x2c) << 8;
          iVar8 = *(int *)(local_68 + g_PlayerHUDState + 0x30);
          lVar11 = __ftol();
          uVar14 = 0;
          piVar12 = &local_80;
          local_7c = ((iVar8 - (int)lVar11) + local_64) * 0x100;
          lVar11 = __ftol();
          Gfx_DrawSprite(((int*)&(DAT_0052873c))[(int)lVar11],piVar12,uVar14);
        }
        iVar6 = iVar6 + -0x484c;
      } while (-1 < iVar6);
    }
    if ((player_idx == 1) && (*(int *)(g_Vehicles + 0x4d74) == 0)) {
      if (_DAT_005285d8 < _DAT_0047a900) {
        local_60[1] = 0.0;
      }
      else {
        local_60[1] = ABS((double)*(int *)(g_Vehicles + 0x4bb0)) / (double)DAT_005287f8;
      }
      local_80 = (*(int *)(local_68 + g_PlayerHUDState + 0x2c) + 5) * 0x100;
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x30);
      lVar11 = __ftol();
      local_7c = ((iVar6 - (int)lVar11) + local_64 + 5) * 0x100;
      Gfx_DrawSprite(((int*)&(DAT_00528764))[*(int *)(g_PlayerHUDState + 0x4c)],&local_80,0);
    }
  }
  if ((_DAT_00552e48 < _DAT_0047a910) || (_DAT_0047a8f8 <= _DAT_00552e48)) {
    if ((_DAT_0047a8f8 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a918)) {
      if (DAT_00563c38 == 0) {
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        Audio_PlaySampleVol(0,4,0,0,0x10000,22000,0);
        DAT_00563c38 = 1;
      }
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x19] + local_74) * 0x100;
      uVar14 = DAT_00528658;
      goto LAB_00440490;
    }
    if ((_DAT_0047a918 <= _DAT_00552e48) && (_DAT_00552e48 < _DAT_0047a920)) {
      if (DAT_00563c38 == 1) {
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        Audio_PlaySampleVol(0,4,0,0,0x10000,22000,0);
        DAT_00563c38 = 2;
      }
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x19] + local_74) * 0x100;
      uVar14 = DAT_0052865c;
      goto LAB_00440490;
    }
    if ((_DAT_00552e48 == _DAT_0047a8c8) && (_DAT_005285d8 < _DAT_0047a900)) {
      if (DAT_00563c38 == 2) {
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        Audio_PlaySampleVol(0,4,1,0,0x10000,22000,0);
        DAT_00563c38 = 3;
      }
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + iVar6) * 0x100;
      local_7c = (g_SpeedoConfig[0x19] + local_74) * 0x100;
      uVar14 = DAT_00528660;
      goto LAB_00440490;
    }
  }
  else {
    iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
    local_80 = (((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 - g_SpeedoConfig[0x18] / 2) +
               iVar6) * 0x100;
    local_7c = (g_SpeedoConfig[0x19] + local_74) * 0x100;
    uVar14 = DAT_00528654;
LAB_00440490:
    Gfx_DrawSprite(uVar14,&local_80,0);
  }
  if (((_DAT_0054f980 < 0.0) || (0x77 < (int)((int*)&(DAT_00527f40))[player_idx])) ||
     ((g_GameMode == 2 && (g_NumRacers < 2)))) goto LAB_004407ac;
  if (g_GameMode == 3) {
LAB_00440508:
    if (1 < *(int *)(g_Vehicles + 0x3a0 + iVar4)) goto LAB_004407ac;
  }
  else if (3 < *(int *)(g_Vehicles + 0x3a0 + iVar4)) {
    if (g_GameMode != 3) goto LAB_004407ac;
    goto LAB_00440508;
  }
  iVar4 = g_Vehicles + iVar4;
  if (*(int *)(iVar4 + 0x528) == 1) {
    if (((0 < *(int *)(iVar4 + 0x5d4)) && (*(int *)(iVar4 + 0x3a0) < 4)) &&
       ((g_GameMode != 2 || (1 < g_NumRacers)))) {
      iVar6 = *(int *)(local_68 + g_PlayerHUDState + 0x2c);
      local_80 = ((*(int *)(local_68 + g_PlayerHUDState + 0x34) - iVar6) / 2 + iVar6) * 0x100;
      local_7c = (local_6c - local_74) / 2 << 8;
      ((int*)&(local_60[0]))[0] = *(int *)(iVar4 + 0x5d4);
      _DAT_00563d74 = 0;
      _DAT_00563d70 = (float)((int*)&(local_60[0]))[0] * (float)_DAT_0047a8b8;
      if (*(int *)(iVar4 + 0x5d4) < 0x19) {
        puVar15 = &DAT_00563d70;
      }
      else {
        puVar15 = (char *)0x0;
      }
      Gfx_DrawSprite(((int*)&(DAT_00528794))[*(int *)(iVar4 + 0x3a0)],&local_80,puVar15);
      if ((99 < (int)((int*)&(DAT_006192a0))[player_idx * 10]) && (*(int *)(&DAT_005531c0 + player_idx * 4) == 0))
      {
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        lVar11 = __ftol();
        ((int*)&(local_60[0]))[0] = (int)lVar11;
        Audio_PlaySampleVol(0,4,5,0,0x10000,22000,0);
        *(int *)(&DAT_005531c0 + player_idx * 4) = 1;
      }
    }
    iVar6 = 6;
    iVar4 = player_idx * 0x28;
    do {
      if ((0xc < *(int *)((int)&DAT_006192a0 + iVar4)) && ((int)((int*)&(DAT_00527f40))[player_idx] < 0x28)) {
        iVar5 = g_PlayerHUDState + local_68;
        iVar8 = *(int *)(iVar5 + 0x2c);
        Math_RandomFloat0To1();
        ((int*)&(local_60[0]))[0] = (*(int *)(iVar5 + 0x34) - iVar8) / 2 + iVar8;
        lVar11 = __ftol();
        *(int *)((int)&DAT_00552d90 + iVar4) = (int)lVar11;
        Math_RandomFloat0To1();
        ((int*)&(local_60[0]))[0] = (local_6c - local_74) / 2;
        lVar11 = __ftol();
        *(int *)((int)&DAT_00552df0 + iVar4) = (int)lVar11;
        if (*(int *)((int)&DAT_006192a0 + iVar4) < 100) {
          *(int *)((int)&DAT_006192a0 + iVar4) = 0;
        }
        else {
          Math_RandomFloat0To1();
          lVar11 = __ftol();
          *(int *)((int)&DAT_006192a0 + iVar4) = (int)lVar11;
        }
      }
      if (*(int *)((int)&DAT_006192a0 + iVar4) < 0xd) {
        _DAT_00563d70 = 0.5;
        _DAT_00563d74 = 0;
        local_80 = *(int *)((int)&DAT_00552d90 + iVar4) << 8;
        local_7c = *(int *)((int)&DAT_00552df0 + iVar4) << 8;
      }
      iVar4 = iVar4 + 4;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
  }
LAB_004407ac:
  HUD_RenderFloatingMessages();
  return;
}
