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
#define CONCAT44(high, low) ((((__int64)(high)) << 32) | (low))
#include <math.h>
#define fsin sin
#define fcos cos
#define __ftol() (int)Math_RandomFloat()
#define CONCAT44(high, low) ((((__int64)(high)) << 32) | (low))
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
extern __int64 Lisa_InitRasterizerTables(int a, unsigned int b);
extern int *g_pLisaDrawCommandQueue;
extern int *g_LisaDrawCommandBuffer;
extern unsigned __int64 Lisa_FlushRasterizerCommands(int a, unsigned int b);
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
extern int g_RaceTimer;
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
extern int g_RaceTimer_P2;
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
extern int g_pCarGhostTransforms;
extern int g_RaceTimer_P2;
extern int DAT_0047a5a0;
extern int DAT_00498540;
extern int DAT_0054f9d8;
extern int _DAT_0047a5d0;
extern int g_MasterMusicVolume;
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
extern int g_pActiveDrawBuffer;
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
extern int g_ScreenHeight;
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
extern int g_DynamicObjectsPaused;
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
extern int g_Format_Str_d;
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
extern int g_pWheelTransforms;
extern int DAT_00528620;
extern int _DAT_0047a710;
extern int DAT_0047a2b0;
extern int _DAT_0047a910;
extern int _DAT_0047a568;
extern int _DAT_0047a6f8;
extern int DAT_0052884c;
extern int DAT_006395a7;
extern int g_LapsTotal;
extern int DAT_0054f978;
extern int DAT_00528618;
extern int _DAT_0047a2b8;
extern int DAT_0047a730;
extern int DAT_0063935c;
extern int DAT_0047a288;
extern int DAT_0047a6e8;
extern int DAT_00563ce8;
extern int _DAT_0047a340;
extern int g_ParticleArray1;
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
extern int g_pCarShadowTransforms;
extern int DAT_00552fc0;
extern int DAT_0052870c[];
extern int DAT_00525e84;
extern int DAT_0047a4e0;
extern int DAT_005287bc;
extern int _DAT_0047a718;
extern int DAT_00525e6c;
extern int DAT_0047a300;
extern int DAT_0052873c;
extern int g_pActiveMSH;
extern int _DAT_0047a3c0;
extern int _DAT_0047a300;
extern int DAT_0047a2e0;
extern int DAT_00639927;
extern int DAT_004ba6ec;
extern int g_pCarReflectionTransforms;
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
extern int g_pCarTransforms;
extern int DAT_0063959b;
extern int DAT_00552d80;
extern int _DAT_0047a760;
extern int DAT_0047a2d8;
extern int DAT_00563c34[];
extern int _DAT_0047a748;
extern int DAT_0047a310;
extern int DAT_00528878;
extern int g_pActivePLC[];
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
extern int g_Format_Str_s;
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
extern int g_ParticleCount1;
extern int DAT_00528880;
extern int DAT_00553030;
extern int DAT_00553070;
extern int DAT_005287c0;
extern int DAT_0047a528;
extern int DAT_00563cf4;
extern int g_ScreenWidth;
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
extern int g_ColorDepth;
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
extern int g_pFXTransforms;
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
extern int g_pActivePOS;
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
extern int g_VehicleCameraStates;
extern int DAT_0054f98c;
extern int g_RacePosition;
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
extern int g_ParticleArray2;
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
extern int g_RaceTimer;
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
__int64 Audio_LoadAssets(int param_1, unsigned int param_2) {
    __int64 var_l1;
    var_l1 = Lisa_InitRasterizerTables(param_1, param_2);
    return (((__int64)(var_l1 >> 32)) << 32) | 1;
}

/**
 * @original Audio_StopSample (IGN_WIN.EXE @ 0x0043e6d0, fx.c)
 * @fidelity ADAPTED
 */
unsigned __int64 Audio_StopSample(void) {
    int *var_pu1;
    unsigned __int64 var_u2;
    g_pLisaDrawCommandQueue[0] = (int)g_LisaDrawCommandBuffer;
    var_pu1 = g_pLisaDrawCommandQueue;
    g_pLisaDrawCommandQueue[1] = 0;
    g_LisaDrawCommandBuffer[0] = 0;
    g_LisaDrawCommandBuffer[1] = 0;
    g_LisaDrawCommandBuffer[2] = 0;
    var_u2 = Lisa_FlushRasterizerCommands(0, (unsigned int)var_pu1);
    return var_u2;
}

/**
 * @original FX_SpawnWeather (IGN_WIN.EXE @ 0x0043ec70, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWeather(void) {
    char var_20[32];
    if (DAT_004949bc != 0) {
        sprintf(var_20, "X: %f", DAT_005532c0, DAT_005532c4);
        Font_PrintDirect(0, 0, var_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(var_20, "Y: %f", DAT_005532b8, DAT_005532bc);
        Font_PrintDirect(0, 10, var_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(var_20, "Z: %f", DAT_00553288, DAT_0055328c);
        Font_PrintDirect(0, 20, var_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(var_20, "X VIN: %f", DAT_006192f8, DAT_006192fc);
        Font_PrintDirect(0, 30, var_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(var_20, "Y VIN: %f", DAT_00619318, DAT_0061931c);
        Font_PrintDirect(0, 40, var_20, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, 20);
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
            } else if (DAT_0047a440 <= g_RaceTimer || DAT_005532b0 == 1) {
                should_spawn = 1;
            }
        } else if (DAT_0047a440 <= g_RaceTimer || DAT_005532b0 == 1) {
            should_spawn = 1;
        }

        if (g_IsSplitScreen == 1) {
            diff_x = pos_x - (int)DAT_0063c64c[0];
            if (abs(diff_x) < 2500) {
                pos_z = p->pos_z / 1024;
                diff_z = pos_z - (int)DAT_0063c64c[2];
                if (abs(diff_z) <= 2500) {
                    should_spawn = 1;
                } else if (g_RaceTimer >= DAT_0047a440) {
                    should_spawn = 1;
                }
            } else if (g_RaceTimer >= DAT_0047a440) {
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
  double *var_pd1;
  char var_u2;
  int var_u3;
  int var_i4;
  int *var_pi5;
  int *var_pi6;
  unsigned int var_u7;
  int var_i8;
  int *var_pi9;
  double *var_pd10;
  int out_ECX;
  int extraout_ECX_00;
  unsigned int var_u11;
  int var_i12;
  int var_i13;
  int var_i14;
  int var_u15;
  unsigned int out_EDX;
  unsigned int extraout_EDX_00;
  char *var_pu16;
  int *var_pu17;
  double var_f18;
  double var_f19;
  __int64 var_l20;
  __int64 var_7c;
  __int64 var_74;
  __int64 var_6c;
  __int64 var_64;
  __int64 var_5c;
  __int64 var_54;
  __int64 var_4c;
  __int64 var_44;
  __int64 var_3c;
  int var_34;
  int var_30;
  __int64 var_28;
  double var_20;
  double var_18;
  double var_10;
  double var_8;
  if (DAT_0054f92c == 1) {
    var_f18 = (double)_DAT_00525e2c;
    _DAT_00525e2c = (float)(var_f18 + (double)_DAT_0047a578);
    var_f18 = (double)fsin(var_f18 + (double)_DAT_0047a578);
    g_pActiveCamera[6] = (double)var_f18;
  }
  if ((g_IsDemoMode == 1) && (var_u11 = g_ScreenWidth * g_ScreenHeight, 0 < (int)var_u11)) {
    var_pu17 = &g_VirtualFramebuffer;
    for (var_u7 = var_u11 >> 2; var_u7 != 0; var_u7 = var_u7 - 1) {
      *var_pu17 = 0;
      var_pu17 = var_pu17 + 1;
    }
    for (var_u11 = var_u11 & 3; var_u11 != 0; var_u11 = var_u11 - 1) {
      *(char *)var_pu17 = 0;
      var_pu17 = (int *)((int)var_pu17 + 1);
    }
  }
  if (g_CurrentTrackIndex == 5) {
    var_i14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
    if ((0x1b < var_i14) && (var_i14 < 0x46)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else if (g_CurrentTrackIndex == 3) {
    var_i14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
    if ((0 < var_i14) && (var_i14 < 0x1e)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x19;
      goto LAB_00436c9c;
    }
    if ((0xbb < var_i14) && (var_i14 < 0xc5)) {
      *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
      goto LAB_00436c9c;
    }
  }
  else {
    if (g_CurrentTrackIndex == 2) {
      var_i8 = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
      var_i14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
      var_i4 = (g_ScreenHeight / 200) * 0x19;
      if (var_i8 < var_i4 + var_i8) {
        var_i13 = g_ScreenWidth * var_i8;
        do {
          var_i12 = *(int *)(var_i14 + 0x2c);
          if (var_i12 < *(int *)(var_i14 + 0x34)) {
            do {
              *(char *)((int)&g_VirtualFramebuffer + var_i12 + var_i13) = 0x72;
              var_i12 = var_i12 + 1;
            } while (var_i12 < *(int *)(var_i14 + 0x34));
          }
          var_i8 = var_i8 + 1;
          var_i13 = var_i13 + g_ScreenWidth;
        } while (var_i8 < *(int *)(var_i14 + 0x30) + var_i4);
      }
      if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
        var_i14 = *(int *)(g_PlayerHUDState + 0x7c);
        var_pi6 = (int *)(g_PlayerHUDState + 0x7c);
        if (var_i14 < var_i14 + var_i4) {
          var_pi9 = (int *)(g_PlayerHUDState + 0x78);
          var_pi5 = (int *)(g_PlayerHUDState + 0x80);
          var_i8 = g_ScreenWidth * var_i14;
          do {
            var_i13 = *var_pi9;
            if (var_i13 < *var_pi5) {
              do {
                *(char *)((int)&g_VirtualFramebuffer + var_i13 + var_i8) = 0x72;
                var_i13 = var_i13 + 1;
              } while (var_i13 < *var_pi5);
            }
            var_i14 = var_i14 + 1;
            var_i8 = var_i8 + g_ScreenWidth;
          } while (var_i14 < *var_pi6 + var_i4);
        }
      }
      goto LAB_00436c9c;
    }
    if (g_CurrentTrackIndex == 1) {
      var_i14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((-0x7c < var_i14) && (var_i14 < -0x71)) {
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
    }
    else if (g_CurrentTrackIndex == 0) {
      var_i14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((0x24 < var_i14) && (var_i14 < 0x2e)) {
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x1e;
        goto LAB_00436c9c;
      }
      if ((0x7c < var_i14) && (var_i14 < 0x8c)) {
LAB_00436c7c:
        *(int *)((int)g_pActiveCamera + 0xa4) = 0x23;
        goto LAB_00436c9c;
      }
    }
    else {
      if (g_CurrentTrackIndex != 4) goto LAB_00436c9c;
      var_i14 = *(int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
      if ((6 < var_i14) && (var_i14 < 0xe)) goto LAB_00436c7c;
    }
  }
  *(int *)((int)g_pActiveCamera + 0xa4) = 0;
LAB_00436c9c:
  if (DAT_0054f934 == 1) {
    var_i8 = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
    var_i14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
    if (var_i8 < *(int *)(g_PlayerHUDState + 0x38 + g_MenuCursorPos * 0x4c)) {
      var_i4 = g_ScreenWidth * var_i8;
      do {
        var_i13 = *(int *)(var_i14 + 0x2c);
        if (var_i13 < *(int *)(var_i14 + 0x34)) {
          var_u2 = ((int*)&(DAT_00497ed8))[g_CurrentTrackIndex * 4];
          do {
            *(char *)((int)&g_VirtualFramebuffer + var_i13 + var_i4) = var_u2;
            var_i13 = var_i13 + 1;
          } while (var_i13 < *(int *)(var_i14 + 0x34));
        }
        var_i8 = var_i8 + 1;
        var_i4 = var_i4 + g_ScreenWidth;
      } while (var_i8 < *(int *)(var_i14 + 0x38));
    }
    if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
      var_i14 = *(int *)(g_PlayerHUDState + 0x7c);
      var_pi6 = (int *)(g_PlayerHUDState + 0x84);
      if (var_i14 < *(int *)(g_PlayerHUDState + 0x84)) {
        var_pi5 = (int *)(g_PlayerHUDState + 0x78);
        var_i8 = g_ScreenWidth * var_i14;
        var_pi9 = (int *)(g_PlayerHUDState + 0x80);
        do {
          var_i4 = *var_pi5;
          if (var_i4 < *var_pi9) {
            var_u2 = ((int*)&(DAT_00497ed8))[g_CurrentTrackIndex * 4];
            do {
              *(char *)((int)&g_VirtualFramebuffer + var_i4 + var_i8) = var_u2;
              var_i4 = var_i4 + 1;
            } while (var_i4 < *var_pi9);
          }
          var_i14 = var_i14 + 1;
          var_i8 = var_i8 + g_ScreenWidth;
        } while (var_i14 < *var_pi6);
      }
    }
  }
  var_pd10 = (double *)(g_Vehicles + g_MenuCursorPos * 0x484c);
  var_pd1 = (double *)(g_VehicleConfigs + g_MenuCursorPos * 200);
  if (*(int *)(g_Vehicles + 0x5a4 + g_MenuCursorPos * 0x484c) == 0) {
    var_5c = var_pd1[7] + *var_pd1;
    var_6c = var_pd1[0xf];
    var_54 = var_pd1[9] + var_pd1[8] + var_pd1[6];
    var_64 = var_pd1[0x11];
    var_u3 = *(int *)((int)var_pd1 + 0x6c);
    var_u15 = *(int *)(var_pd1 + 0xd);
  }
  else {
    var_5c = var_pd1[7] + *var_pd1;
    var_6c = var_pd1[0x12];
    var_54 = var_pd1[9] + var_pd1[8] + var_pd1[6];
    var_64 = var_pd1[0x14];
    var_u3 = *(int *)((int)var_pd1 + 0x74);
    var_u15 = *(int *)(var_pd1 + 0xe);
  }
  var_7c = (double)CONCAT44(var_u3,var_u15);
  if (DAT_0054f934 == 1) {
    var_f18 = (double)fcos((double)var_pd10[0x1f]);
    var_5c = var_pd10[1] + _DAT_0047a588;
    var_f19 = (double)fsin((double)var_pd10[0x1f]);
    var_54 = 150.0;
    var_6c = (double)(var_f18 * (double)_DAT_0047a580 + (double)*var_pd10);
    var_64 = (double)(var_f19 * (double)_DAT_0047a580 + (double)var_pd10[2]);
    for (var_7c = var_pd10[0x1f] * _DAT_0047a598 + _DAT_0047a5a0; var_7c < _DAT_0047a5a8;
        var_7c = var_7c + _DAT_0047a5b0) {
    }
    for (; _DAT_0047a5b8 < var_7c; var_7c = var_7c - _DAT_0047a5b0) {
    }
  }
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    if (*(int *)(g_Vehicles + 0x4df0) == 0) {
      var_28 = *(double *)(g_VehicleConfigs + 200) + *(double *)(g_VehicleConfigs + 0x100);
      var_4c = *(double *)(g_VehicleConfigs + 0x140);
      var_3c = *(double *)(g_VehicleConfigs + 0x108) + *(double *)(g_VehicleConfigs + 0xf8) +
                 *(double *)(g_VehicleConfigs + 0x110);
      var_44 = *(double *)(g_VehicleConfigs + 0x150);
      var_74 = *(double *)(g_VehicleConfigs + 0x130);
    }
    else {
      var_28 = *(double *)(g_VehicleConfigs + 200) + *(double *)(g_VehicleConfigs + 0x100);
      var_4c = *(double *)(g_VehicleConfigs + 0x158);
      var_44 = *(double *)(g_VehicleConfigs + 0x168);
      var_74 = *(double *)(g_VehicleConfigs + 0x138);
      var_3c = *(double *)(g_VehicleConfigs + 0x108) + *(double *)(g_VehicleConfigs + 0xf8) +
                 *(double *)(g_VehicleConfigs + 0x110);
    }
    if (DAT_0054f934 == 1) {
      var_pd10 = (double *)(g_Vehicles + 0x4944);
      var_f18 = (double)fcos((double)*var_pd10);
      var_28 = *(double *)(g_Vehicles + 0x4854) + _DAT_0047a588;
      var_f19 = (double)fsin((double)*var_pd10);
      var_3c = 150.0;
      var_4c = (double)(var_f18 * (double)_DAT_0047a580 +
                         (double)*(double *)(g_Vehicles + 0x484c));
      var_44 = (double)(var_f19 * (double)_DAT_0047a580 +
                         (double)*(double *)(g_Vehicles + 0x485c));
      for (var_74 = *var_pd10 * _DAT_0047a598 + _DAT_0047a5a0; var_74 < _DAT_0047a5a8;
          var_74 = var_74 + _DAT_0047a5b0) {
      }
      for (; _DAT_0047a5b8 < var_74; var_74 = var_74 - _DAT_0047a5b0) {
      }
    }
  }
  if ((-1 < DAT_00527f40) && (DAT_00527f40 < 0x78)) {
    var_l20 = __ftol();
    DAT_00527f40 = (int)var_l20;
  }
  if ((-1 < DAT_00527f44) && (DAT_00527f44 < 0x78)) {
    var_l20 = __ftol();
    DAT_00527f44 = (int)var_l20;
  }
  if (((0.0 <= g_RaceTimer) && (g_RaceTimer < _DAT_0047a5c0)) || (g_PlayerCarChoice == 7)) {
    g_RaceTimer = g_RaceTimer + param_1;
    if ((*(int *)(g_Vehicles + 0x528) == 1) &&
       (var_pu17 = (int *)(g_Vehicles + 0x5d4), *(int *)(g_Vehicles + 0x5d4) < 0x19)) {
      var_l20 = __ftol();
      *var_pu17 = (int)var_l20;
    }
    if ((*(int *)(g_Vehicles + 0x4d74) == 1) &&
       (var_pu17 = (int *)(g_Vehicles + 20000), *(int *)(g_Vehicles + 20000) < 0x19)) {
      var_l20 = __ftol();
      *var_pu17 = (int)var_l20;
    }
  }
  else if ((_DAT_0047a5c0 <= g_RaceTimer) &&
          (((g_IsSplitScreen == 0 && (*(int *)(g_Vehicles + 0x528) == 1)) ||
           ((g_IsSplitScreen == 1 &&
            ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))))) {
    if (DAT_00563d20 == 3) {
      _DAT_00601664 = Race_FindFocusedVehicle();
      if (g_NumRacers - _DAT_00601664 == -1) {
        DAT_00563d20 = 0;
        g_MenuCursorPos = 0;
        var_i14 = g_CurrentTrackIndex * 0x3c;
        var_6c = (double)*(float *)(&DAT_00498538 + g_CurrentTrackIndex * 0x3c);
        var_f18 = (double)fsin((double)_DAT_005285d8 * (double)_DAT_0047a5c8);
        var_64 = (double)*(float *)(&DAT_00498540 + var_i14);
        var_5c = (double)(var_f18 * (double)_DAT_0047a5d0 +
                           (double)*(float *)(&DAT_0049853c + var_i14));
        var_54 = (double)(*(float *)(&DAT_00498544 + var_i14) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        var_7c = (double)(*(float *)(&DAT_00498548 + var_i14) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        var_20 = var_5c;
        var_18 = var_64;
        var_10 = var_54;
        var_8 = var_7c;
        if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
          var_74 = var_7c;
          var_4c = var_6c;
          var_44 = var_64;
          var_3c = var_54;
          var_28 = var_5c;
        }
      }
      else {
        var_5c = *(double *)(g_VehicleConfigs + 0x38 + g_MenuCursorPos * 200) +
                   *(double *)(g_VehicleConfigs + g_MenuCursorPos * 200);
        var_i14 = g_VehicleConfigs + g_MenuCursorPos * 200;
        var_6c = (double)CONCAT44(*(int *)(g_VehicleConfigs + 0x94 + g_MenuCursorPos * 200),
                                    *(int *)(var_i14 + 0x90));
        var_64 = (double)CONCAT44(*(int *)(g_VehicleConfigs + 0xa4 + g_MenuCursorPos * 200),
                                    *(int *)(var_i14 + 0xa0));
        var_54 = *(double *)(var_i14 + 0x48) + *(double *)(var_i14 + 0x40) +
                   *(double *)(var_i14 + 0x30);
        var_7c = *(double *)(var_i14 + 0x70);
      }
    }
    else {
      var_i14 = g_CurrentTrackIndex * 3 + DAT_00563d20;
      var_6c = (double)*(float *)(&DAT_00498538 + var_i14 * 0x14);
      var_f18 = (double)fsin((double)_DAT_005285d8 * (double)_DAT_0047a5c8);
      var_i14 = var_i14 * 0x14;
      var_64 = (double)*(float *)(&DAT_00498540 + var_i14);
      var_54 = (double)(*(float *)(&DAT_00498544 + var_i14) * _DAT_0047a5d8 * (float)_DAT_0047a5e0)
      ;
      var_5c = (double)(var_f18 * (double)_DAT_0047a5d0 +
                         (double)*(float *)(&DAT_0049853c + var_i14));
      var_7c = (double)(*(float *)(&DAT_00498548 + var_i14) * _DAT_0047a5d8 * (float)_DAT_0047a5e0)
      ;
      if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
        var_4c = (double)*(float *)(&DAT_00498538 + g_CurrentTrackIndex * 0x3c);
        var_44 = (double)*(float *)(&DAT_00498540 + g_CurrentTrackIndex * 0x3c);
        var_28 = (double)(*(float *)(&DAT_0049853c + g_CurrentTrackIndex * 0x3c) +
                           (float)(var_f18 * (double)_DAT_0047a5d0));
        var_3c = (double)(*(float *)(&DAT_00498544 + g_CurrentTrackIndex * 0x3c) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
        var_74 = (double)(*(float *)(&DAT_00498548 + g_CurrentTrackIndex * 0x3c) * _DAT_0047a5d8 *
                           (float)_DAT_0047a5e0);
      }
    }
  }
  if (DAT_005532b0 == 1) {
    var_6c = DAT_005532c0;
    var_54 = _DAT_006192f8 * _DAT_0047a5e8;
    var_7c = _DAT_00619318 * _DAT_0047a5e8;
    var_5c = DAT_005532b8;
    var_64 = DAT_00553288;
  }
  *(int *)(g_pActiveCamera + 7) = 0;
  var_pd10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 4) = ((int*)&var_6c)[1];
  *(int *)var_pd10 = (int)var_6c;
  var_pd10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0xc) = ((int*)&var_5c)[1];
  *(int *)(var_pd10 + 1) = (int)var_5c;
  var_pd10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x14) = ((int*)&var_64)[1];
  *(int *)(var_pd10 + 2) = (int)var_64;
  var_pd10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x1c) = ((int*)&var_54)[1];
  *(int *)(var_pd10 + 3) = (int)var_54;
  var_pd10 = g_pActiveCamera;
  *(int *)((int)g_pActiveCamera + 0x24) = ((int*)&var_7c)[1];
  *(int *)(var_pd10 + 4) = (int)var_7c;
  var_pd10 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 5) = 0;
  *(int *)((int)var_pd10 + 0x2c) = 0;
  *(int *)((int)g_pActiveCamera + 0x7c) = 0;
  *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x58 + g_MenuCursorPos * 200);
  *(int *)((int)g_pActiveCamera + 0x84) =
       *(int *)(g_VehicleConfigs + 0x5c + g_MenuCursorPos * 200);
  var_l20 = __ftol();
  *(int *)(g_pActiveCamera + 0x11) = (int)var_l20;
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
      var_l20 = __ftol();
      *(int *)(g_pActiveCamera + 0xe) = (int)var_l20;
      var_l20 = __ftol();
      *(int *)((int)g_pActiveCamera + 0x74) = (int)var_l20;
      var_l20 = __ftol();
      *(int *)(g_pActiveCamera + 0xf) = (int)var_l20;
    }
    if (DAT_00563c14 == 2) {
      var_i14 = g_Vehicles + DAT_00563c5c * 0x484c;
      if (((*(int *)(g_Vehicles + 0x354 + DAT_00563c5c * 0x484c) == 0) &&
          (*(int *)(var_i14 + 0x358) == 0)) && (*(int *)(var_i14 + 0x35c) == 0)) {
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
        var_l20 = __ftol();
        *(int *)(g_pActiveCamera + 0xe) = (int)var_l20;
        var_l20 = __ftol();
        *(int *)((int)g_pActiveCamera + 0x74) = (int)var_l20;
        var_l20 = __ftol();
        *(int *)(g_pActiveCamera + 0xf) = (int)var_l20;
        var_l20 = __ftol();
        _DAT_0054f9d8 = (int)var_l20;
        var_l20 = __ftol();
        _DAT_0054f9dc = (int)var_l20;
        var_l20 = __ftol();
        _DAT_0054f9e0 = (int)var_l20;
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
  if ((_DAT_0047a5f0 <= g_RaceTimer_P2) && (g_RaceTimer_P2 < _DAT_0047a5f8)) {
    var_l20 = __ftol();
    var_u11 = (int)(unsigned int)var_l20 >> 0x1f;
    if (((((unsigned int)var_l20 ^ var_u11) - var_u11 & 1 ^ var_u11) == var_u11) &&
       (var_pi6 = (int *)(g_NumRacers * 0x20 + g_pCarGhostTransforms), var_pi6[7] == 0)) {
      Lisa_MoveDynamicObject(var_pi6);
    }
  }
  _DAT_00498730 = g_pLisaDrawCommandQueue;
  _DAT_00498738 = &g_VirtualFramebuffer;
  _DAT_0049873c = g_pActiveTAB;
  _DAT_00498734 = &g_VirtualFramebuffer;
  g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x2c + g_MenuCursorPos * 0x4c);
  var_i14 = g_PlayerHUDState + g_MenuCursorPos * 0x4c;
  g_ViewportMinY = *(int *)(var_i14 + 0x30);
  g_ViewportMaxX = *(int *)(var_i14 + 0x34) + -1;
  g_ViewportMaxY = *(int *)(var_i14 + 0x38) + -1;
  var_u11 = *(unsigned int *)(g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);
  if ((int)var_u11 < 0) {
    var_i14 = *(int *)(g_pTrackRoadSequence + 0xc +
                     ((var_u11 ^ (int)var_u11 >> 0x1f) - ((int)var_u11 >> 0x1f)) * 0x18);
  }
  else {
    var_i14 = *(int *)(g_pTrackRoadSequence + var_u11 * 0x18);
  }
  if (_DAT_0047a5c0 <= g_RaceTimer) {
    DAT_0063c5d8 = 0;
  }
  else {
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + var_i14 * 0xc);
  }
  Lisa_RenderScene();
  Lisa_FlushRasterizerCommands(out_ECX,out_EDX);
  var_pi6 = (int *)(g_NumRacers * 0x20 + g_pCarGhostTransforms);
  if (var_pi6[7] == 1) {
    Lisa_DeleteDynamicObject(var_pi6);
  }
  if (((*(int *)(g_Vehicles + 0x528) == 0) || (g_RaceTimer <= _DAT_0047a5c0)) &&
     (DAT_005532b0 == 0)) {
    HUD_RenderPlayerElements(g_MenuCursorPos);
  }
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    *(int *)(g_pActiveCamera + 7) = 0;
    var_pd10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 4) = ((int*)&var_4c)[1];
    *(int *)var_pd10 = (int)var_4c;
    var_pd10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0xc) = ((int*)&var_28)[1];
    *(int *)(var_pd10 + 1) = (int)var_28;
    var_pd10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x14) = ((int*)&var_44)[1];
    *(int *)(var_pd10 + 2) = (int)var_44;
    var_pd10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x1c) = ((int*)&var_3c)[1];
    *(int *)(var_pd10 + 3) = (int)var_3c;
    var_pd10 = g_pActiveCamera;
    *(int *)((int)g_pActiveCamera + 0x24) = ((int*)&var_74)[1];
    *(int *)(var_pd10 + 4) = (int)var_74;
    var_pd10 = g_pActiveCamera;
    *(int *)(g_pActiveCamera + 5) = 0;
    *(int *)((int)var_pd10 + 0x2c) = 0;
    *(int *)((int)g_pActiveCamera + 0x7c) = 0;
    *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x120);
    *(int *)((int)g_pActiveCamera + 0x84) = *(int *)(g_VehicleConfigs + 0x124);
    var_l20 = __ftol();
    *(int *)(g_pActiveCamera + 0x11) = (int)var_l20;
    *(int *)((int)g_pActiveCamera + 0x9c) = g_ScreenWidth / (g_IsSplitScreen * 2 + 2);
    _DAT_00498730 = g_pLisaDrawCommandQueue;
    _DAT_00498734 = &g_VirtualFramebuffer;
    _DAT_00498738 = &g_VirtualFramebuffer;
    _DAT_0049873c = g_pActiveTAB;
    g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x78);
    g_ViewportMinY = *(int *)(g_PlayerHUDState + 0x7c);
    g_ViewportMaxX = *(int *)(g_PlayerHUDState + 0x80) + -1;
    g_ViewportMaxY = *(int *)(g_PlayerHUDState + 0x84) + -1;
    if ((_DAT_0047a5f0 <= g_RaceTimer_P2) && (g_RaceTimer_P2 < _DAT_0047a5f8)) {
      var_l20 = __ftol();
      var_u11 = (int)(unsigned int)var_l20 >> 0x1f;
      if (((((unsigned int)var_l20 ^ var_u11) - var_u11 & 1 ^ var_u11) == var_u11) &&
         (var_i14 = g_NumRacers * 0x20 + g_pCarGhostTransforms, *(int *)(var_i14 + 0x3c) == 0)) {
        Lisa_MoveDynamicObject((int *)(var_i14 + 0x20));
      }
    }
    var_u11 = *(unsigned int *)(g_Vehicles + 0x4bb0);
    if ((int)var_u11 < 0) {
      var_i14 = *(int *)(g_pTrackRoadSequence + 0xc +
                       ((var_u11 ^ (int)var_u11 >> 0x1f) - ((int)var_u11 >> 0x1f)) * 0x18);
    }
    else {
      var_i14 = *(int *)(g_pTrackRoadSequence + var_u11 * 0x18);
    }
    DAT_0063c5d8 = *(int *)(DAT_00552e40 + var_i14 * 0xc);
    Lisa_RenderScene();
    Lisa_FlushRasterizerCommands(extraout_ECX_00,extraout_EDX_00);
    var_i14 = g_NumRacers * 0x20 + g_pCarGhostTransforms;
    if (*(int *)(var_i14 + 0x3c) == 1) {
      Lisa_DeleteDynamicObject((int *)(var_i14 + 0x20));
    }
    if ((*(int *)(g_Vehicles + 0x4d74) == 0) || (g_RaceTimer <= _DAT_0047a5c0)) {
      HUD_RenderPlayerElements(1);
    }
  }
  FX_SpawnWeather();
  if ((0.0 <= g_RaceTimer_P2) && (g_RaceTimer_P2 < _DAT_0047a588)) {
    __ftol();
    Lisa_RenderPanorama();
  }
  if (((((g_IsSplitScreen == 0) && (*(int *)(g_Vehicles + 0x528) == 1)) ||
       ((g_IsSplitScreen == 1 &&
        ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))) &&
      (_DAT_0047a5c0 <= g_RaceTimer)) && ((DAT_00563c60 == 0 && (DAT_005532b0 == 0)))) {
    HUD_RenderTrackResults();
  }
  var_i14 = *(int *)(g_PlayerHUDState + 0x38);
  var_i8 = *(int *)(g_PlayerHUDState + 0x34);
  if ((DAT_00527f28 == 0) && ((g_IsSplitScreen == 0 || (DAT_00552f58 == 1)))) {
    var_34 = 0;
    var_30 = 0;
    Gfx_DrawSprite(DAT_0054f960,&var_34,0);
    var_30 = 0;
    var_34 = (var_i8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    Gfx_DrawSprite(DAT_0054f964,&var_34,0);
    var_34 = 0;
    var_30 = (var_i14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    Gfx_DrawSprite(DAT_0054f968,&var_34,0);
    var_34 = (var_i8 - *(int *)(DAT_00563c58 + 8)) * 0x100;
    var_30 = (var_i14 - *(int *)(DAT_00563c58 + 0xc)) * 0x100;
    Gfx_DrawSprite(DAT_0054f96c,&var_34,0);
  }
  if (DAT_00563c60 == 1) {
    var_l20 = __ftol();
    g_MasterMusicVolume = (int)var_l20;
    if (g_MasterMusicVolume < 1) {
      Ghost_SaveGhostData();
      DAT_00563d88 = 2;
    }
    Lisa_RenderPanorama();
  }
  if ((g_IsSplitScreen == 1) && (DAT_00552f58 == 0)) {
    var_i4 = g_ScreenWidth / 2;
    var_i8 = g_ScreenHeight;
    for (var_i14 = var_i4 + -1; g_ScreenHeight = var_i8, var_i14 < var_i4 + 1; var_i14 = var_i14 + 1) {
      if (0 < var_i8) {
        var_pu16 = (char *)((int)&g_VirtualFramebuffer + var_i14);
        do {
          var_i13 = g_ScreenWidth;
          *var_pu16 = 0;
          var_pu16 = var_pu16 + var_i13;
          var_i8 = var_i8 + -1;
        } while (var_i8 != 0);
      }
      var_i8 = g_ScreenHeight;
    }
  }
  return;
}
/**
 * @original FX_UpdateAllParticles (IGN_WIN.EXE @ 0x00434190, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateAllParticles(void) {
  float var_f1;
  int var_i2;
  int *var_pi3;
  int var_i4;
  int var_i5;
  var_pi3 = &g_SceneryParticles;
  var_i5 = 0;
  do {
    if (*var_pi3 != 0) {
      switch(*var_pi3) {
      case 1:
        FX_UpdateTransparentSpriteObject((SceneryParticle *)var_pi3,var_i5);
        break;
      case 2:
        FX_UpdateHandlePlotObject((SceneryParticle *)var_pi3,var_i5);
        break;
      case 3:
        Obstacle_SimulateDynamics((SceneryParticle *)var_pi3);
        break;
      case 4:
        FX_UpdateTransparentSpriteObject2((SceneryParticle *)var_pi3,var_i5);
        break;
      case 5:
        FX_UpdateFlyingParticles((SceneryParticle *)var_pi3,var_i5);
        break;
      case 6:
        FX_UpdateExplosionNode((SceneryParticle *)var_pi3,var_i5);
        break;
      case 7:
        FX_UpdateVehicleWreck(var_pi3);
        break;
      case 8:
        FX_UpdateSuperPlotObject((SceneryParticle *)var_pi3,var_i5);
        break;
      case 9:
        FX_UpdateDetachedWheel(var_pi3,var_i5);
        break;
      case 10:
        FX_UpdateVehicleCrashSequence(var_pi3);
        break;
      case 0xb:
        FX_UpdateCarDebris(var_pi3,var_i5);
      }
    }
    var_pi3 = var_pi3 + 0x19;
    var_i5 = var_i5 + 1;
  } while (var_pi3 < &DAT_005dfe60);
  _DAT_00563d64 = _DAT_00563d64 + _DAT_0047a430;
  if ((float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4) <= _DAT_00563d64) {
    Track_UpdateMovingPathNodes();
    var_f1 = (float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4);
    if ((float)*(int *)(&DAT_00498480 + g_CurrentTrackIndex * 4) <= _DAT_00563d64) {
      do {
        _DAT_00563d64 = _DAT_00563d64 - var_f1;
      } while (var_f1 <= _DAT_00563d64);
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
  if ((g_GameMode == 3) && (var_i5 = 0, 0 < g_NumRacers)) {
    var_i4 = 0;
    do {
      var_i2 = g_Vehicles + var_i4;
      if ((((*(int *)(var_i2 + 0x39c) == g_PlayerCarModel) && (g_RaceTimer_P2 < 0.0)) &&
          (*(int *)(var_i2 + 0x354) == 0)) &&
         (((*(int *)(var_i2 + 0x358) == 0 && (*(int *)(var_i2 + 0x35c) == 0)) &&
          (*(int *)(var_i2 + 0x528) == 0)))) {
        FX_SpawnAmbientTrackParticles();
      }
      var_i4 = var_i4 + 0x484c;
      var_i5 = var_i5 + 1;
    } while (var_i5 < g_NumRacers);
  }
  HUD_UpdateFloatingMessages();
  return;
}
/**
 * @original FX_UpdateWeatherBounds (IGN_WIN.EXE @ 0x00434580, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateWeatherBounds(void) {
  unsigned int var_u1;
  int var_i2;
  unsigned int var_u3;
  if (g_HudEnabled == 1) {
    if (((g_CurrentTrackIndex == 0) &&
        (var_i2 = *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), 0x2d < var_i2)) &&
       (var_i2 < 0x78)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 0) &&
       ((var_i2 = *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), var_i2 < 0x2e ||
        (0x77 < var_i2)))) {
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
        (var_u1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), var_u3 = (int)var_u1 >> 0x1f
        , var_i2 = (var_u1 ^ var_u3) - var_u3, 0 < var_i2)) && (var_i2 < 0x1e)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 1) &&
       ((var_u1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), var_u3 = (int)var_u1 >> 0x1f
        , var_i2 = (var_u1 ^ var_u3) - var_u3, var_i2 < 1 || (0x1d < var_i2)))) {
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
        (var_u1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), var_u3 = (int)var_u1 >> 0x1f
        , var_i2 = (var_u1 ^ var_u3) - var_u3, 0x1c < var_i2)) && (var_i2 < 100)) {
      if (g_ActiveVehicleIndex == 0) {
        DAT_00552f30 = 1;
      }
      else if ((g_ActiveVehicleIndex == 1) && (g_IsSplitScreen == 1)) {
        _DAT_00552f34 = 1;
      }
    }
    if ((g_CurrentTrackIndex == 4) &&
       ((var_u1 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c), var_u3 = (int)var_u1 >> 0x1f
        , var_i2 = (var_u1 ^ var_u3) - var_u3, var_i2 < 0x1d || (99 < var_i2)))) {
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
  int *dyn_obj;
  double velocity;
  int pos_x;
  int pos_y;
  int pos_z;
  int car_index;
  unsigned int rot_x;
  unsigned int rot_y;
  unsigned int rot_z;
  double speed;
  int is_active;
  char *model_ptr;
  double *veh_ptr;
  int temp_val1;
  int *mesh_ptr;
  int temp_val2;
  double *veh_state_ptr;
  int veh_offset;
  int *transform_ptr;
  int obj_offset;
  unsigned int *model_data;
  int timer2;
  int timer1;
  int race_fin_offset;
  double float_val;
  __int64 long_val;
  double temp_f1;
  int temp_i1;
  int temp_i2;
  int temp_i3;
  double temp_f2;
  int temp_i4;
  int temp_i5;
  veh_ptr = g_Vehicles;
  temp_i5 = 0;
  temp_i2 = 0;
  pos_x = wreck[1];
  pos_y = wreck[2];
  temp_i3 = 0;
  temp_i4 = 0;
  pos_z = wreck[3];
  car_index = wreck[4];
  timer1 = wreck[5];
  timer2 = wreck[6];
  veh_offset = car_index * 0x484c;
  *(int *)((int)g_Vehicles + veh_offset + 0x534) = 0;
  *(int *)((int)veh_ptr + veh_offset + 0x538) = 0x40590000;
  *(int *)((int)g_Vehicles + veh_offset + 0x53c) = 0;
  if (timer1 == 0) {
    obj_offset = car_index * 0x20;
    if ((((int *)(g_pCarTransforms + obj_offset))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
      Lisa_DeleteDynamicObject((int *)(g_pCarTransforms + obj_offset));
    }
    race_fin_offset = car_index * 0x80;
    if (((int *)(g_pWheelTransforms + race_fin_offset))[7] != 0) {
      Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + race_fin_offset));
    }
    if (*(int *)(g_pWheelTransforms + race_fin_offset + 0x3c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + race_fin_offset + 0x20));
    }
    if (*(int *)(g_pWheelTransforms + race_fin_offset + 0x5c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + race_fin_offset + 0x40));
    }
    if (*(int *)(g_pWheelTransforms + race_fin_offset + 0x7c) != 0) {
      Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + race_fin_offset + 0x60));
    }
    if ((((int *)(g_pCarShadowTransforms + obj_offset))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
      Lisa_DeleteDynamicObject((int *)(g_pCarShadowTransforms + obj_offset));
    }
    if (((*(int *)(g_PlayerHUDState + car_index * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
       (((int *)(obj_offset + g_pCarReflectionTransforms))[7] != 0)) {
      Lisa_DeleteDynamicObject((int *)(obj_offset + g_pCarReflectionTransforms));
    }
    mesh_ptr = (int *)((int)g_Vehicles + veh_offset);
    mesh_ptr[0xe2] = mesh_ptr[1];
    mesh_ptr[0xe1] = *mesh_ptr;
    veh_ptr = g_Vehicles;
    *(int *)((int)g_Vehicles + veh_offset + 0x390) =
         *(int *)((int)g_Vehicles + veh_offset + 0xc);
    *(int *)((int)veh_ptr + veh_offset + 0x38c) = *(int *)((int)veh_ptr + veh_offset + 8);
    veh_ptr = g_Vehicles;
    *(int *)((int)g_Vehicles + veh_offset + 0x398) =
         *(int *)((int)g_Vehicles + veh_offset + 0x14);
    *(int *)((int)veh_ptr + veh_offset + 0x394) = *(int *)((int)veh_ptr + veh_offset + 0x10);
    veh_ptr = g_Vehicles;
    *(int *)((int)g_Vehicles + veh_offset + 0x380) =
         *(int *)((int)g_Vehicles + veh_offset + 0xfc);
    *(int *)((int)veh_ptr + veh_offset + 0x37c) = *(int *)((int)veh_ptr + veh_offset + 0xf8);
    *(int *)((int)g_Vehicles + veh_offset + 0x270) = 1;
    if (*(int *)((int)g_Vehicles + veh_offset + 0x298) == 1) {
      model_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + veh_offset + 0x99c));
      if (model_ptr != (char *)0x0) {
        *(int *)(model_ptr + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
    }
    if (*(int *)((int)g_Vehicles + veh_offset + 0x29c) == 1) {
      model_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + veh_offset + 0x9a0));
      if (model_ptr != (char *)0x0) {
        *(int *)(model_ptr + 0x10) = 0;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
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
    long_val = __ftol();
    g_ActiveParticle.field_3c = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_40 = (int)long_val;
    long_val = __ftol();
    g_ActiveParticle.field_44 = (int)long_val;
    g_ActiveParticle.field_48 = 0;
    g_ActiveParticle.life = 0;
    g_ActiveParticle.field_4c = 0x166;
    g_ActiveParticle.field_54 = 0x1f;
    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
    Audio_PlaySampleVol(0,2,5,0,0x10000,22000,0);
  }
  if (timer2 - timer1 == 0x14) {
    obj_offset = 10;
    do {
      g_ActiveParticle.type = 0xb;
      g_ActiveParticle.pos_y = 0;
      g_ActiveParticle.pos_x = car_index;
      Math_RandomFloat0To1();
      long_val = __ftol();
      g_ActiveParticle.pos_z = (int)long_val;
      Math_RandomFloat0To1();
      long_val = __ftol();
      g_ActiveParticle.vel_x = (int)long_val;
      g_ActiveParticle.vel_y = 0;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x2800;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.field_24 = 0x3c;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      obj_offset = obj_offset + -1;
    } while (obj_offset != 0);
    __ftol();
    __ftol();
    Audio_PlaySampleVol(0,4,3,0,0xe000,0,0);
  }
  if (timer1 <= timer2 + -0x28) goto LAB_00431ab6;
  veh_ptr = (double *)((int)g_Vehicles + veh_offset);
  rot_x = *(unsigned int *)((int)veh_ptr + 0x364);
  obj_offset = (rot_x ^ (int)rot_x >> 0x1f) - ((int)rot_x >> 0x1f);
  if (obj_offset + 2 < DAT_005287f8) {
    if (-1 < (int)rot_x) {
      temp_f1 = *(double *)(g_pTrackRoadSequence + rot_x * 6 + 0xd);
      race_fin_offset = g_pTrackRoadSequence[rot_x * 6 + 0xc];
      goto joined_r0x004317bd;
    }
    temp_f1 = *(double *)(g_pTrackRoadSequence + obj_offset * 6 + 0x10) - _DAT_0047a288;
    transform_ptr = g_pTrackRoadSequence + obj_offset * 6 + 0xf;
    race_fin_offset = *transform_ptr;
    if (temp_f1 < _DAT_0047a290) {
      temp_f1 = temp_f1 + _DAT_0047a298;
    }
    if (race_fin_offset == 10000) {
      temp_f1 = *(double *)(g_pTrackRoadSequence + rot_x * 6 + 0xd) - _DAT_0047a288;
      race_fin_offset = g_pTrackRoadSequence[rot_x * 6 + 0xc];
      if (temp_f1 < _DAT_0047a290) {
        temp_f1 = temp_f1 + _DAT_0047a298;
      }
    }
    if (race_fin_offset == -2) {
      race_fin_offset = 0;
      do {
        dyn_obj = transform_ptr + 6;
        transform_ptr = transform_ptr + 6;
        race_fin_offset = race_fin_offset + 1;
      } while (*dyn_obj == -2);
      temp_f1 = *(double *)(g_pTrackRoadSequence + (race_fin_offset + obj_offset) * 6 + 0xd);
      race_fin_offset = g_pTrackRoadSequence[(race_fin_offset + obj_offset) * 6 + 0xc];
      goto joined_r0x004317bd;
    }
  }
  else {
    temp_f1 = *(double *)(g_pTrackRoadSequence + 1);
    race_fin_offset = *g_pTrackRoadSequence;
joined_r0x004317bd:
    temp_f1 = temp_f1 - _DAT_0047a288;
    if (temp_f1 < _DAT_0047a290) {
      temp_f1 = temp_f1 + _DAT_0047a298;
    }
  }
  temp_val2 = race_fin_offset * 0x14 + g_pActivePLC;
  race_fin_offset = *(int *)(temp_val2 + 0x10);
  obj_offset = g_pActiveMSH + *(int *)(temp_val2 + 4) * 4;
  temp_val1 = *(int *)(obj_offset + 4);
  model_data = (unsigned int *)(obj_offset + 8 + *(int *)(g_pActiveMSH + *(int *)(temp_val2 + 4) * 4) * 0xc);
  if (0 < temp_val1) {
    do {
      rot_x = model_data[1];
      rot_y = model_data[2];
      rot_z = model_data[3];
      if ((*model_data & 0xffff0000) < 0x280000) {
        temp_i5 = temp_i5 +
                   *(int *)(obj_offset + 8 + rot_x * 0xc) + (*(int *)(temp_val2 + 0xc) + 0x6400) * 3 +
                   *(int *)(obj_offset + 8 + rot_y * 0xc) + *(int *)(obj_offset + 8 + rot_z * 0xc);
        temp_i2 = (((((temp_i2 - *(int *)(obj_offset + 0xc + rot_x * 0xc)) + race_fin_offset) -
                     *(int *)(obj_offset + 0xc + rot_y * 0xc)) + race_fin_offset) -
                   *(int *)(obj_offset + 0xc + rot_z * 0xc)) + race_fin_offset;
        temp_i4 = temp_i4 + 3;
        temp_i3 = temp_i3 +
                   *(int *)(obj_offset + 0x10 + rot_x * 0xc) + (*(int *)(temp_val2 + 0x14) + 0x6400) * 3 +
                   *(int *)(obj_offset + 0x10 + rot_y * 0xc) + *(int *)(obj_offset + 0x10 + rot_z * 0xc);
      }
      model_data = model_data + 0xb;
      temp_val1 = temp_val1 + -1;
    } while (temp_val1 != 0);
  }
  temp_f2 = temp_f1;
  if (temp_f1 < 0.0) {
    temp_f2 = temp_f1 + _DAT_0047a298;
  }
  velocity = (double)((timer1 - timer2) + 0x28);
  float_val = Math_LookupTrigAngle(velocity);
  *veh_ptr = (double)(((double)(temp_i5 / temp_i4) - (double)*(double *)((int)veh_ptr + 900)) *
                      float_val + (double)*(double *)((int)veh_ptr + 900));
  *(double *)((int)g_Vehicles + veh_offset + 8) =
       ((double)(temp_i2 / temp_i4 + 0xfa) - *(double *)((int)g_Vehicles + veh_offset + 0x38c)) *
       velocity * _DAT_0047a2a0 + *(double *)((int)g_Vehicles + veh_offset + 0x38c);
  veh_ptr = g_Vehicles;
  float_val = Math_LookupTrigAngle(velocity);
  speed = temp_f2 - _DAT_0047a298;
  *(double *)((int)veh_ptr + veh_offset + 0x10) =
       (double)(((double)(temp_i3 / temp_i4) -
                (double)*(double *)((int)veh_ptr + veh_offset + 0x394)) * float_val +
               (double)*(double *)((int)veh_ptr + veh_offset + 0x394));
  if (ABS(speed - *(double *)((int)g_Vehicles + veh_offset + 0x37c)) <
      ABS(temp_f2 - *(double *)((int)g_Vehicles + veh_offset + 0x37c))) {
    temp_f2 = speed;
  }
  *(double *)((int)g_Vehicles + veh_offset + 0xf8) =
       (temp_f2 - *(double *)((int)g_Vehicles + veh_offset + 0x37c)) * velocity * _DAT_0047a2a0 +
       *(double *)((int)g_Vehicles + veh_offset + 0x37c);
  veh_ptr = (double *)((int)g_Vehicles + veh_offset + 0xf8);
  if (_DAT_0047a2a8 < *(double *)((int)g_Vehicles + veh_offset + 0xf8)) {
    *veh_ptr = *veh_ptr - _DAT_0047a298;
  }
  veh_ptr = (double *)((int)g_Vehicles + veh_offset + 0xf8);
  if (*(double *)((int)g_Vehicles + veh_offset + 0xf8) < _DAT_0047a2b0) {
    *veh_ptr = *veh_ptr + _DAT_0047a298;
  }
LAB_00431ab6:
  obj_offset = timer1 + 1;
  temp_i1 = obj_offset;
  if (timer2 <= obj_offset) {
    is_active = 0;
    timer2 = 0;
    veh_ptr = g_Vehicles;
    if (0 < g_NumRacers) {
      do {
        if (((((car_index != timer2) &&
              (veh_state_ptr = (double *)((int)g_Vehicles + veh_offset),
              ABS(*veh_ptr - *veh_state_ptr) < _DAT_0047a2b8)) &&
             ((ABS(veh_ptr[1] - veh_state_ptr[1]) < _DAT_0047a2b8 &&
              ((ABS(veh_ptr[2] - veh_state_ptr[2]) < _DAT_0047a2b8 &&
               (*(int *)((int)veh_ptr + 0x354) == 0)))))) && (*(int *)(veh_ptr + 0x6b) == 0)) &&
           (*(int *)((int)veh_ptr + 0x35c) == 0)) {
          is_active = 1;
        }
        timer2 = timer2 + 1;
        veh_ptr = (double *)((int)veh_ptr + 0x484c);
      } while (timer2 < g_NumRacers);
    }
    temp_i1 = timer1;
    if ((!is_active) &&
       ((*(int *)((int)g_Vehicles + veh_offset + 0x528) == 0 ||
        (*(int *)((int)g_Vehicles + veh_offset + 0x604) == 1)))) {
      *wreck = 0;
      *(int *)((int)g_Vehicles + veh_offset + 0x350) = 1;
      timer1 = car_index * 0x20;
      *(int *)((int)g_Vehicles + veh_offset + 0x358) = 0;
      veh_ptr = g_Vehicles;
      *(int *)((int)g_Vehicles + veh_offset + 0x534) = 0;
      *(int *)((int)veh_ptr + veh_offset + 0x538) = 0x40590000;
      if ((((int *)(g_pCarTransforms + timer1))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_MoveDynamicObject((int *)(g_pCarTransforms + timer1));
      }
      timer2 = car_index * 0x80;
      if (((int *)(g_pWheelTransforms + timer2))[7] == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + timer2));
      }
      if (*(int *)(g_pWheelTransforms + timer2 + 0x3c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + timer2 + 0x20));
      }
      if (*(int *)(g_pWheelTransforms + timer2 + 0x5c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + timer2 + 0x40));
      }
      if (*(int *)(g_pWheelTransforms + timer2 + 0x7c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + timer2 + 0x60));
      }
      if ((((int *)(g_pCarShadowTransforms + timer1))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_MoveDynamicObject((int *)(g_pCarShadowTransforms + timer1));
      }
      if (((*(int *)(g_PlayerHUDState + car_index * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
         (((int *)(timer1 + g_pCarReflectionTransforms))[7] == 0)) {
        Lisa_MoveDynamicObject((int *)(timer1 + g_pCarReflectionTransforms));
      }
      Car_UpdateDynamicObjects(car_index);
      temp_i1 = obj_offset;
    }
  }
  wreck[2] = pos_y;
  wreck[1] = pos_x;
  wreck[3] = pos_z;
  wreck[5] = temp_i1;
  return;
}

/**
 * @original FX_UpdateDetachedWheel (IGN_WIN.EXE @ 0x00431d00, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateDetachedWheel(int *wheel, int wheel_index) {
  int var_u1;
  int var_u2;
  int var_u3;
  int var_u4;
  int var_u5;
  int var_u6;
  int var_u7;
  int var_u8;
  int var_u9;
  int var_u10;
  int var_u11;
  int var_i12;
  int var_u13;
  int var_i14;
  int var_i15;
  int var_u16;
  int var_i17;
  int var_i18;
  int *var_pi19;
  int var_i20;
  int var_i21;
  int var_i22;
  int var_i23;
  var_u1 = wheel[2];
  var_u2 = wheel[1];
  var_u3 = wheel[3];
  var_u4 = wheel[4];
  var_u5 = wheel[5];
  var_u6 = wheel[6];
  var_u7 = wheel[7];
  var_u8 = wheel[8];
  var_u9 = wheel[9];
  var_u10 = wheel[10];
  var_u11 = wheel[0xb];
  var_i12 = wheel[0xd];
  var_u13 = wheel[0xc];
  var_i21 = wheel[0xf];
  var_i14 = wheel[0x10];
  var_i15 = wheel[0x11];
  var_u16 = wheel[0xe];
  var_i20 = wheel[0x12];
  var_i17 = wheel[0x13];
  var_i18 = wheel[0x15];
  var_i22 = wheel[0x14];
  if (var_i22 == 0) {
    var_i23 = wheel_index * 800;
    *(int *)(&DAT_00528800 + var_i23) = 4;
    *(int *)(&DAT_00528804 + var_i23) = 2;
    *(int *)(&DAT_00528808 + var_i23) = var_u2;
    *(int *)(&DAT_0052880c + var_i23) = var_u1;
    *(int *)(&DAT_00528810 + var_i23) = var_u3;
    *(int *)(&DAT_00528814 + var_i23) = var_u4;
    *(int *)(&DAT_00528818 + var_i23) = var_u5;
    *(int *)(&DAT_0052881c + var_i23) = var_u6;
    *(int *)(&DAT_00528820 + var_i23) = var_u7;
    *(int *)(&DAT_00528824 + var_i23) = var_u8;
    *(int *)(&DAT_00528828 + var_i23) = var_u9;
    *(int *)(&DAT_0052882c + var_i23) = var_u10;
    *(int *)(&DAT_00528830 + var_i23) = var_u11;
    *(int *)(&DAT_00528834 + var_i23) = var_u13;
    *(int *)(&DAT_00528838 + var_i23) = 0x13;
    *(int *)(&DAT_0052883c + var_i23) = 0;
    *(int *)(&DAT_00528840 + var_i23) = 1;
    *(int *)(&DAT_00528844 + var_i23) = 2;
    *(int *)(&DAT_00528860 + var_i23) = var_u16;
    *(int *)(&DAT_00528864 + var_i23) = 0x13;
    *(int *)(&DAT_00528868 + var_i23) = 0;
    *(int *)(&DAT_0052886c + var_i23) = 2;
    *(int *)(&DAT_00528870 + var_i23) = 3;
    *(int *)(&DAT_0052888c + var_i23) = var_u16;
    Lisa_SetDynamicObjectMesh(g_pFXTransforms,var_u13,(int *)(wheel_index * 0x20 + g_pFXTransforms),
                 (int *)(&DAT_00528800 + var_i23),var_u16,200,0,0x14,0);
  }
  var_i23 = wheel_index * 0x20;
  *(int *)(var_i23 + 4 + g_pFXTransforms) = (int)(var_i21 + (var_i21 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(var_i23 + 8 + g_pFXTransforms) = (int)(var_i14 + (var_i14 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(var_i23 + 0xc + g_pFXTransforms) = (int)(var_i15 + (var_i15 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(var_i23 + 0x10 + g_pFXTransforms) = 0;
  *(int *)(var_i23 + 0x14 + g_pFXTransforms) = 0;
  *(int *)(var_i23 + 0x18 + g_pFXTransforms) = 0;
  var_pi19 = (int *)(g_pFXTransforms + var_i23);
  if (var_pi19[7] == 0) {
    Lisa_MoveDynamicObject(var_pi19);
  }
  else {
    var_i21 = Lisa_UpdateObjectSpatialGrid(var_pi19);
    if (var_i21 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_WATER_SPLA_00499468);
    }
  }
  var_i20 = var_i20 + var_i17;
  var_i21 = wheel_index * 800;
  var_pi19 = (int *)(var_i12 + ((int)(var_i20 + (var_i20 >> 0x1f & 0x3ffU)) >> 10) * 0x10);
  *(int *)(&DAT_00528848 + var_i21) = *var_pi19 << 8;
  *(int *)(&DAT_0052884c + var_i21) = var_pi19[3] << 8;
  *(int *)(&DAT_00528850 + var_i21) = *var_pi19 << 8;
  *(int *)(&DAT_00528854 + var_i21) = var_pi19[1] << 8;
  *(int *)(&DAT_00528858 + var_i21) = var_pi19[2] << 8;
  *(int *)(&DAT_0052885c + var_i21) = var_pi19[1] << 8;
  *(int *)(&DAT_00528874 + var_i21) = *var_pi19 << 8;
  *(int *)(&DAT_00528878 + var_i21) = var_pi19[3] << 8;
  *(int *)(&DAT_0052887c + var_i21) = var_pi19[2] << 8;
  *(int *)(&DAT_00528880 + var_i21) = var_pi19[1] << 8;
  var_i22 = var_i22 + 1;
  *(int *)(&DAT_00528884 + var_i21) = var_pi19[2] << 8;
  *(int *)(&DAT_00528888 + var_i21) = var_pi19[3] << 8;
  if (var_i18 <= var_i22) {
    Lisa_DeleteDynamicObject((int *)(var_i23 + g_pFXTransforms));
    *wheel = 0;
  }
  wheel[0x14] = var_i22;
  wheel[0x12] = var_i20;
  return;
}

/**
 * @original FX_UpdateVehicleCrashSequence (IGN_WIN.EXE @ 0x00432040, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateVehicleCrashSequence(int *crash_seq) {
  int *var_pi1;
  double var_d2;
  int var_u3;
  int var_u4;
  int var_u5;
  int var_i6;
  unsigned int var_u7;
  unsigned int var_u8;
  unsigned int var_u9;
  double var_d10;
  int var_b11;
  char *var_pu12;
  double *var_pd13;
  int var_i14;
  int var_i15;
  int *var_pu16;
  int var_i17;
  double *var_pd18;
  int var_i19;
  unsigned int *var_pu20;
  int var_i21;
  int *var_pi22;
  int var_i23;
  int var_i24;
  int var_i25;
  double var_f26;
  __int64 var_l27;
  double var_48;
  int var_3c;
  int var_34;
  double var_28;
  int var_20;
  int var_1c;
  var_pd13 = g_Vehicles;
  var_i24 = 0;
  var_1c = 0;
  var_34 = 0;
  var_u3 = crash_seq[1];
  var_u4 = crash_seq[2];
  var_20 = 0;
  var_u5 = crash_seq[3];
  var_i6 = crash_seq[4];
  var_i25 = crash_seq[5];
  var_i23 = crash_seq[6];
  var_i15 = var_i6 * 0x484c;
  *(int *)((int)g_Vehicles + var_i15 + 0x534) = 0;
  *(int *)((int)var_pd13 + var_i15 + 0x538) = 0x40590000;
  *(int *)((int)g_Vehicles + var_i15 + 0x53c) = 0;
  if (var_i25 == 0) {
    var_pu16 = (int *)(var_i15 + (int)g_Vehicles);
    var_pu16[0xe2] = var_pu16[1];
    var_pu16[0xe1] = *var_pu16;
    var_pd13 = g_Vehicles;
    *(int *)((int)g_Vehicles + var_i15 + 0x390) =
         *(int *)((int)g_Vehicles + var_i15 + 0xc);
    *(int *)((int)var_pd13 + var_i15 + 0x38c) = *(int *)((int)var_pd13 + var_i15 + 8);
    var_pd13 = g_Vehicles;
    *(int *)((int)g_Vehicles + var_i15 + 0x398) =
         *(int *)((int)g_Vehicles + var_i15 + 0x14);
    *(int *)((int)var_pd13 + var_i15 + 0x394) = *(int *)((int)var_pd13 + var_i15 + 0x10);
    var_pd13 = g_Vehicles;
    *(int *)((int)g_Vehicles + var_i15 + 0x380) =
         *(int *)((int)g_Vehicles + var_i15 + 0xfc);
    *(int *)((int)var_pd13 + var_i15 + 0x37c) = *(int *)((int)var_pd13 + var_i15 + 0xf8);
    *(int *)((int)g_Vehicles + var_i15 + 0x270) = 1;
    if (*(int *)((int)g_Vehicles + var_i15 + 0x298) == 1) {
      var_pu12 = Audio_GetVoice(*(int *)((int)g_Vehicles + var_i15 + 0x99c));
      if (var_pu12 != (char *)0x0) {
        *(int *)(var_pu12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
    }
    if (*(int *)((int)g_Vehicles + var_i15 + 0x29c) == 1) {
      var_pu12 = Audio_GetVoice(*(int *)((int)g_Vehicles + var_i15 + 0x9a0));
      if (var_pu12 != (char *)0x0) {
        *(int *)(var_pu12 + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x29c) = 0;
    }
  }
  if (var_i25 < 0x1f) {
    var_i19 = 0x1e - var_i25;
    if (10 < var_i19) {
      var_i19 = 10;
    }
    var_d2 = (double)var_i19;
    var_pd13 = (double *)((int)g_Vehicles + var_i15 + 900);
    *var_pd13 = var_d2 + *var_pd13;
    *(double *)((int)g_Vehicles + var_i15 + 0x394) =
         *(double *)((int)g_Vehicles + var_i15 + 0x394) + var_d2;
    *(double *)((int)g_Vehicles + var_i15) = *(double *)((int)g_Vehicles + var_i15) + var_d2;
    if (var_i25 < 10) {
      var_d10 = *(double *)((int)g_Vehicles + var_i15 + 8) + _DAT_0047a2c8;
    }
    else {
      var_d10 = *(double *)((int)g_Vehicles + var_i15 + 8) - _DAT_0047a2d0;
    }
    *(double *)((int)g_Vehicles + var_i15 + 8) = var_d10;
    *(double *)((int)g_Vehicles + var_i15 + 0x10) =
         *(double *)((int)g_Vehicles + var_i15 + 0x10) + var_d2;
    *(double *)((int)g_Vehicles + var_i15 + 0x108) =
         *(double *)((int)g_Vehicles + var_i15 + 0x108) + _DAT_0047a2d8;
    *(double *)((int)g_Vehicles + var_i15 + 0x110) =
         *(double *)((int)g_Vehicles + var_i15 + 0x110) + _DAT_0047a2e0;
    var_f26 = Math_RandomFloat0To1();
    if (var_f26 == (double)_DAT_0047a2e8) {
      __ftol();
      __ftol();
      Audio_PlaySampleVol(0,2,1,0,0x10000,22000,0);
    }
    if (var_i25 == 0x1e) {
      var_i19 = var_i6 * 0x20;
      if ((((int *)(g_pCarTransforms + var_i19))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_DeleteDynamicObject((int *)(g_pCarTransforms + var_i19));
      }
      var_i21 = var_i6 * 0x80;
      if (((int *)(g_pWheelTransforms + var_i21))[7] != 0) {
        Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + var_i21));
      }
      if (*(int *)(g_pWheelTransforms + var_i21 + 0x3c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + var_i21 + 0x20));
      }
      if (*(int *)(g_pWheelTransforms + var_i21 + 0x5c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + var_i21 + 0x40));
      }
      if (*(int *)(g_pWheelTransforms + var_i21 + 0x7c) != 0) {
        Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + var_i21 + 0x60));
      }
      if ((((int *)(g_pCarShadowTransforms + var_i19))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_DeleteDynamicObject((int *)(g_pCarShadowTransforms + var_i19));
      }
      if (((*(int *)(g_PlayerHUDState + var_i6 * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
         (((int *)(var_i19 + g_pCarReflectionTransforms))[7] != 0)) {
        Lisa_DeleteDynamicObject((int *)(var_i19 + g_pCarReflectionTransforms));
      }
    }
  }
  if (var_i23 - var_i25 == 0x14) {
    var_i19 = 10;
    do {
      g_ActiveParticle.type = 0xb;
      g_ActiveParticle.pos_y = 0;
      g_ActiveParticle.pos_x = var_i6;
      Math_RandomFloat0To1();
      var_l27 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l27;
      Math_RandomFloat0To1();
      var_l27 = __ftol();
      g_ActiveParticle.vel_x = (int)var_l27;
      g_ActiveParticle.vel_y = 0;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x2800;
      g_ActiveParticle.field_24 = 0x3c;
      g_ActiveParticle.gravity = 0;
      FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
      var_i19 = var_i19 + -1;
    } while (var_i19 != 0);
    __ftol();
    __ftol();
    Audio_PlaySampleVol(0,4,3,0,0xe000,0,0);
  }
  if (var_i25 <= var_i23 + -0x28) goto LAB_00432970;
  var_pd13 = (double *)((int)g_Vehicles + var_i15);
  var_u7 = *(unsigned int *)((int)var_pd13 + 0x364);
  var_i19 = (var_u7 ^ (int)var_u7 >> 0x1f) - ((int)var_u7 >> 0x1f);
  if (var_i19 + 2 < DAT_005287f8) {
    if (-1 < (int)var_u7) {
      var_48 = *(double *)(g_pTrackRoadSequence + var_u7 * 6 + 0xd);
      var_i21 = g_pTrackRoadSequence[var_u7 * 6 + 0xc];
      goto joined_r0x00432683;
    }
    var_48 = *(double *)(g_pTrackRoadSequence + var_i19 * 6 + 0x10) - _DAT_0047a300;
    var_pi22 = g_pTrackRoadSequence + var_i19 * 6 + 0xf;
    var_i21 = *var_pi22;
    if (var_48 < _DAT_0047a308) {
      var_48 = var_48 + _DAT_0047a310;
    }
    if (var_i21 == 10000) {
      var_48 = *(double *)(g_pTrackRoadSequence + var_u7 * 6 + 0xd) - _DAT_0047a300;
      var_i21 = g_pTrackRoadSequence[var_u7 * 6 + 0xc];
      if (var_48 < _DAT_0047a308) {
        var_48 = var_48 + _DAT_0047a310;
      }
    }
    if (var_i21 == -2) {
      var_i21 = 0;
      do {
        var_pi1 = var_pi22 + 6;
        var_pi22 = var_pi22 + 6;
        var_i21 = var_i21 + 1;
      } while (*var_pi1 == -2);
      var_48 = *(double *)(g_pTrackRoadSequence + (var_i21 + var_i19) * 6 + 0xd);
      var_i21 = g_pTrackRoadSequence[(var_i21 + var_i19) * 6 + 0xc];
      goto joined_r0x00432683;
    }
  }
  else {
    var_48 = *(double *)(g_pTrackRoadSequence + 1);
    var_i21 = *g_pTrackRoadSequence;
joined_r0x00432683:
    var_48 = var_48 - _DAT_0047a300;
    if (var_48 < _DAT_0047a308) {
      var_48 = var_48 + _DAT_0047a310;
    }
  }
  var_i17 = var_i21 * 0x14 + g_pActivePLC;
  var_i21 = *(int *)(var_i17 + 0x10);
  var_i19 = g_pActiveMSH + *(int *)(var_i17 + 4) * 4;
  var_i14 = *(int *)(var_i19 + 4);
  var_pu20 = (unsigned int *)(var_i19 + 8 + *(int *)(g_pActiveMSH + *(int *)(var_i17 + 4) * 4) * 0xc);
  if (0 < var_i14) {
    do {
      var_u7 = var_pu20[1];
      var_u8 = var_pu20[2];
      var_u9 = var_pu20[3];
      if ((*var_pu20 & 0xffff0000) < 0x280000) {
        var_1c = var_1c +
                   *(int *)(var_i19 + 8 + var_u7 * 0xc) + (*(int *)(var_i17 + 0xc) + 0x6400) * 3 +
                   *(int *)(var_i19 + 8 + var_u8 * 0xc) + *(int *)(var_i19 + 8 + var_u9 * 0xc);
        var_i24 = (((((var_i24 - *(int *)(var_i19 + 0xc + var_u7 * 0xc)) + var_i21) -
                   *(int *)(var_i19 + 0xc + var_u8 * 0xc)) + var_i21) -
                 *(int *)(var_i19 + 0xc + var_u9 * 0xc)) + var_i21;
        var_34 = var_34 +
                   *(int *)(var_i19 + 0x10 + var_u7 * 0xc) + (*(int *)(var_i17 + 0x14) + 0x6400) * 3 +
                   *(int *)(var_i19 + 0x10 + var_u8 * 0xc) + *(int *)(var_i19 + 0x10 + var_u9 * 0xc);
        var_20 = var_20 + 3;
      }
      var_pu20 = var_pu20 + 0xb;
      var_i14 = var_i14 + -1;
    } while (var_i14 != 0);
  }
  var_28 = var_48;
  if (var_48 < 0.0) {
    var_28 = var_48 + _DAT_0047a310;
  }
  var_d2 = (double)((var_i25 - var_i23) + 0x28);
  var_f26 = Math_LookupTrigAngle(var_d2);
  *var_pd13 = (double)(((double)(var_1c / var_20) - (double)*(double *)((int)var_pd13 + 900)) *
                      var_f26 + (double)*(double *)((int)var_pd13 + 900));
  *(double *)((int)g_Vehicles + var_i15 + 8) =
       ((double)(var_i24 / var_20 + 0xfa) - *(double *)((int)g_Vehicles + var_i15 + 0x38c)) *
       var_d2 * _DAT_0047a318 + *(double *)((int)g_Vehicles + var_i15 + 0x38c);
  var_pd13 = g_Vehicles;
  var_f26 = Math_LookupTrigAngle(var_d2);
  var_d10 = var_28 - _DAT_0047a310;
  *(double *)((int)var_pd13 + var_i15 + 0x10) =
       (double)(((double)(var_34 / var_20) -
                (double)*(double *)((int)var_pd13 + var_i15 + 0x394)) * var_f26 +
               (double)*(double *)((int)var_pd13 + var_i15 + 0x394));
  if (ABS(var_d10 - *(double *)((int)g_Vehicles + var_i15 + 0x37c)) <
      ABS(var_28 - *(double *)((int)g_Vehicles + var_i15 + 0x37c))) {
    var_28 = var_d10;
  }
  *(double *)((int)g_Vehicles + var_i15 + 0xf8) =
       (var_28 - *(double *)((int)g_Vehicles + var_i15 + 0x37c)) * var_d2 * _DAT_0047a318 +
       *(double *)((int)g_Vehicles + var_i15 + 0x37c);
  var_pd13 = (double *)((int)g_Vehicles + var_i15 + 0xf8);
  if (_DAT_0047a320 < *(double *)((int)g_Vehicles + var_i15 + 0xf8)) {
    *var_pd13 = *var_pd13 - _DAT_0047a310;
  }
  var_pd13 = (double *)((int)g_Vehicles + var_i15 + 0xf8);
  if (*(double *)((int)g_Vehicles + var_i15 + 0xf8) < _DAT_0047a328) {
    *var_pd13 = *var_pd13 + _DAT_0047a310;
  }
LAB_00432970:
  var_i24 = var_i25 + 1;
  var_3c = var_i24;
  if (var_i23 <= var_i24) {
    var_b11 = 0;
    var_i23 = 0;
    var_pd13 = g_Vehicles;
    if (0 < g_NumRacers) {
      do {
        if (((((var_i6 != var_i23) &&
              (var_pd18 = (double *)(var_i15 + (int)g_Vehicles),
              ABS(*var_pd13 - *var_pd18) < _DAT_0047a330)) &&
             ((ABS(var_pd13[1] - var_pd18[1]) < _DAT_0047a330 &&
              ((ABS(var_pd13[2] - var_pd18[2]) < _DAT_0047a330 &&
               (*(int *)((int)var_pd13 + 0x354) == 0)))))) && (*(int *)(var_pd13 + 0x6b) == 0)) &&
           (*(int *)((int)var_pd13 + 0x35c) == 0)) {
          var_b11 = 1;
        }
        var_i23 = var_i23 + 1;
        var_pd13 = (double *)((int)var_pd13 + 0x484c);
      } while (var_i23 < g_NumRacers);
    }
    var_3c = var_i25;
    if ((!var_b11) &&
       ((*(int *)((int)g_Vehicles + var_i15 + 0x528) == 0 ||
        (*(int *)((int)g_Vehicles + var_i15 + 0x604) == 1)))) {
      *crash_seq = 0;
      *(int *)((int)g_Vehicles + var_i15 + 0x350) = 1;
      var_i25 = var_i6 * 0x20;
      *(int *)((int)g_Vehicles + var_i15 + 0x35c) = 0;
      var_pd13 = g_Vehicles;
      *(int *)((int)g_Vehicles + var_i15 + 0x534) = 0;
      *(int *)((int)var_pd13 + var_i15 + 0x538) = 0x40590000;
      if ((((int *)(g_pCarTransforms + var_i25))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_MoveDynamicObject((int *)(g_pCarTransforms + var_i25));
      }
      var_i23 = var_i6 * 0x80;
      if (((int *)(g_pWheelTransforms + var_i23))[7] == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + var_i23));
      }
      if (*(int *)(g_pWheelTransforms + var_i23 + 0x3c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + var_i23 + 0x20));
      }
      if (*(int *)(g_pWheelTransforms + var_i23 + 0x5c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + var_i23 + 0x40));
      }
      if (*(int *)(g_pWheelTransforms + var_i23 + 0x7c) == 0) {
        Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + var_i23 + 0x60));
      }
      if ((((int *)(g_pCarShadowTransforms + var_i25))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
        Lisa_MoveDynamicObject((int *)(g_pCarShadowTransforms + var_i25));
      }
      if (((*(int *)(g_PlayerHUDState + var_i6 * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
         (((int *)(var_i25 + g_pCarReflectionTransforms))[7] == 0)) {
        Lisa_MoveDynamicObject((int *)(var_i25 + g_pCarReflectionTransforms));
      }
      Car_UpdateDynamicObjects(var_i6);
      var_3c = var_i24;
    }
  }
  crash_seq[2] = var_u4;
  crash_seq[1] = var_u3;
  crash_seq[3] = var_u5;
  crash_seq[5] = var_3c;
  return;
}

/**
 * @original FX_UpdateCarDebris (IGN_WIN.EXE @ 0x00432bb0, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateCarDebris(int *debris, int debris_idx) {
  int var_i1;
  int var_i2;
  int var_i3;
  int var_i4;
  int var_i5;
  int var_i6;
  int var_i7;
  int *var_pi8;
  int var_i9;
  int var_i10;
  int var_i11;
  __int64 var_l12;
  int var_1c;
  var_1c = debris[2];
  var_i11 = debris[3];
  var_i10 = debris[4];
  var_i1 = debris[7];
  var_i2 = debris[5];
  var_i3 = debris[6];
  var_i4 = debris[8];
  var_i5 = debris[9];
  if (var_i4 == 0) {
    var_i6 = debris_idx * 800;
    *(int *)(&DAT_00528800 + var_i6) = 1;
    *(int *)(&DAT_00528804 + var_i6) = 1;
    *(int *)(&DAT_00528808 + var_i6) = 0;
    *(int *)(&DAT_0052880c + var_i6) = 0;
    *(int *)(&DAT_00528810 + var_i6) = 0;
    *(int *)(&DAT_00528814 + var_i6) = 7;
    var_i7 = DAT_00552d80;
    *(int *)(&DAT_00528818 + var_i6) = 0;
    *(int *)(&DAT_0052881c + var_i6) = 0x2100;
    *(int *)(&DAT_00528820 + var_i6) = 0x5400;
    *(int *)(&DAT_00528824 + var_i6) = 0x4200;
    *(int *)(&DAT_00528828 + var_i6) = 0x7500;
    *(int *)(&DAT_0052882c + var_i6) = 0;
    *(int *)(&DAT_00528830 + var_i6) = var_i7 * 0x96;
    var_i9 = g_pFXTransforms;
    var_i7 = DAT_00552f5c * 0x96;
    *(int *)(&DAT_00528834 + var_i6) = var_i7;
    Lisa_SetDynamicObjectMesh(var_i9,var_i7,(int *)(debris_idx * 0x20 + var_i9),(int *)(&DAT_00528800 + var_i6),2,200,0,
                 0x14,0);
  }
  var_i7 = var_i4 * -8 + 0xfa;
  *(int *)(&DAT_00528808 + debris_idx * 800) = var_i7;
  if (var_i7 < 0x32) {
    *(int *)(&DAT_00528808 + debris_idx * 800) = 0x32;
  }
  if (0x59c00 < var_1c) {
    var_1c = var_1c + ((var_1c + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (0x59c00 < var_i11) {
    var_i11 = var_i11 + ((var_i11 + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (0x59c00 < var_i10) {
    var_i10 = var_i10 + ((var_i10 + 0x3ffU) / 0x5a000) * -0x5a000;
  }
  if (var_1c < 0) {
    var_1c = var_1c + ((0x59fffU - var_1c) / 0x5a000) * 0x5a000;
  }
  if (var_i11 < 0) {
    var_i11 = var_i11 + ((0x59fffU - var_i11) / 0x5a000) * 0x5a000;
  }
  if (var_i10 < 0) {
    var_i10 = var_i10 + ((0x59fffU - var_i10) / 0x5a000) * 0x5a000;
  }
  var_i7 = debris_idx * 0x20;
  var_l12 = __ftol();
  *(int *)(var_i7 + 4 + g_pFXTransforms) = (int)var_l12;
  var_l12 = __ftol();
  *(int *)(var_i7 + 8 + g_pFXTransforms) = (int)var_l12;
  var_l12 = __ftol();
  *(int *)(var_i7 + 0xc + g_pFXTransforms) = (int)var_l12;
  *(int *)(var_i7 + 0x10 + g_pFXTransforms) =
       (int)(var_1c * 10 + (var_1c * 10 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(var_i7 + 0x14 + g_pFXTransforms) = (int)(var_i11 * 10 + (var_i11 * 10 >> 0x1f & 0x3ffU)) >> 10;
  *(int *)(var_i7 + 0x18 + g_pFXTransforms) = (int)(var_i10 * 10 + (var_i10 * 10 >> 0x1f & 0x3ffU)) >> 10;
  var_pi8 = (int *)(g_pFXTransforms + var_i7);
  if (var_pi8[7] == 0) {
    Lisa_MoveDynamicObject(var_pi8);
  }
  else {
    var_i9 = Lisa_UpdateObjectSpatialGrid(var_pi8);
    if (var_i9 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_TRAN_SPRIT_004993b4);
    }
  }
  if (var_i5 <= var_i4 + 1) {
    Lisa_DeleteDynamicObject((int *)(var_i7 + g_pFXTransforms));
    *debris = 0;
  }
  debris[2] = var_1c + var_i2;
  debris[3] = var_i11 + var_i3;
  debris[4] = var_i10 + var_i1;
  debris[8] = var_i4 + 1;
  return;
}

/**
 * @original FX_SpawnWaterSplashes (IGN_WIN.EXE @ 0x00432f20, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWaterSplashes(void) {
  int var_i1;
  __int64 var_l2;
  __int64 var_l3;
  __int64 var_l4;
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (((((*(int *)(g_Vehicles + 0x5d8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) ||
        (*(int *)(var_i1 + 0x5dc) == 0x5a)) || (*(int *)(var_i1 + 0x5e0) == 0x5a)) ||
      (*(int *)(var_i1 + 0x5e4) == 0x5a)) &&
     ((_DAT_0047a340 < *(double *)(var_i1 + 0x118) && (*(int *)(var_i1 + 0x270) == 0)))) {
    *(int *)(var_i1 + 0x33c) = 6;
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
    var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x5e8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) {
      fcos((double)*(double *)(var_i1 + 0xf8));
      fsin((double)*(double *)(var_i1 + 0xf8));
    }
    if (*(int *)(var_i1 + 0x5ec) == 0x5a) {
      fcos((double)*(double *)(var_i1 + 0xf8));
      fsin((double)*(double *)(var_i1 + 0xf8));
    }
    if (*(int *)(var_i1 + 0x5f0) == 0x5a) {
      fcos((double)*(double *)(var_i1 + 0xf8));
      fsin((double)*(double *)(var_i1 + 0xf8));
    }
    if (*(int *)(var_i1 + 0x5f4) == 0x5a) {
      fcos((double)*(double *)(var_i1 + 0xf8));
      fsin((double)*(double *)(var_i1 + 0xf8));
    }
    var_i1 = (*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) == 0) + 1;
    if (var_i1 != 0) {
      var_l2 = __ftol();
      var_l3 = __ftol();
      do {
        Math_RandomFloat0To1();
        Math_RandomFloat0To1();
        fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
        fsin((double)*(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0xf8));
        g_ActiveParticle.type = 2;
        g_ActiveParticle.pos_x = (int)var_l2;
        var_l4 = __ftol();
        g_ActiveParticle.pos_y = (int)var_l4;
        g_ActiveParticle.pos_z = (int)var_l3;
        var_l4 = __ftol();
        g_ActiveParticle.vel_x = (int)var_l4;
        g_ActiveParticle.vel_y = 0x4cc;
        var_l4 = __ftol();
        g_ActiveParticle.vel_z = (int)var_l4;
        g_ActiveParticle.drag = 0x3e1;
        g_ActiveParticle.gravity = 0xffffff67;
        Math_RandomFloat0To1();
        var_l4 = __ftol();
        g_ActiveParticle.field_24 = (int)var_l4;
        g_ActiveParticle.field_28 = 0xfffffd9a;
        g_ActiveParticle.field_2c = 1;
        g_ActiveParticle.rot_x = 0;
        g_ActiveParticle.rot_y = 0xf;
        FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
        var_i1 = var_i1 + -1;
      } while (var_i1 != 0);
    }
  }
  var_i1 = *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c);
  if (0 < var_i1) {
    *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c) = var_i1 + -1;
  }
  return;
}

/**
 * @original FX_SpawnTireDirtDebris (IGN_WIN.EXE @ 0x004333e0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnTireDirtDebris(void) {
  int var_i1;
  int var_i2;
  __int64 var_l3;
  __int64 var_l4;
  int var_24;
  if ((_DAT_0047a380 < *(double *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c)) &&
     (*(int *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x270) == 0)) {
    var_24 = 0;
    do {
      *(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) = 0;
      var_i2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      var_i1 = *(int *)(g_Vehicles + 0x150 + g_ActiveVehicleIndex * 0x484c);
      if (((((var_i1 == 6) || (var_i1 == 0x10)) || (var_i1 == 0x1a)) || (DAT_004949b0 == 1)) &&
         (var_24 == 0)) {
        fcos((double)*(double *)(var_i2 + 0xf8));
        fsin((double)*(double *)(var_i2 + 0xf8));
        *(int *)(var_i2 + 0x360) = 1;
      }
      var_i2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      var_i1 = *(int *)(var_i2 + 0x154);
      if ((((var_i1 == 6) || (var_i1 == 0x10)) || ((var_i1 == 0x1a || (DAT_004949b0 == 1)))) &&
         (var_24 == 1)) {
        fcos((double)*(double *)(var_i2 + 0xf8));
        fsin((double)*(double *)(var_i2 + 0xf8));
        *(int *)(var_i2 + 0x360) = 1;
      }
      var_i2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      var_i1 = *(int *)(var_i2 + 0x158);
      if ((((var_i1 == 6) || (var_i1 == 0x10)) || ((var_i1 == 0x1a || (DAT_004949b0 == 1)))) &&
         (var_24 == 2)) {
        fcos((double)*(double *)(var_i2 + 0xf8));
        fsin((double)*(double *)(var_i2 + 0xf8));
        *(int *)(var_i2 + 0x360) = 1;
      }
      var_i2 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      var_i1 = *(int *)(var_i2 + 0x15c);
      if (((((var_i1 == 6) || (var_i1 == 0x10)) || (var_i1 == 0x1a)) || (DAT_004949b0 == 1)) &&
         (var_24 == 3)) {
        fcos((double)*(double *)(var_i2 + 0xf8));
        fsin((double)*(double *)(var_i2 + 0xf8));
        *(int *)(var_i2 + 0x360) = 1;
      }
      if (*(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) == 1) {
        var_i2 = 2;
        var_l3 = __ftol();
        do {
          Math_RandomFloat0To1();
          Math_RandomFloat0To1();
          g_ActiveParticle.type = 8;
          var_l4 = __ftol();
          g_ActiveParticle.pos_x = (int)var_l4;
          var_l4 = __ftol();
          g_ActiveParticle.pos_y = (int)var_l4;
          var_l4 = __ftol();
          g_ActiveParticle.pos_z = (int)var_l4;
          var_l4 = __ftol();
          g_ActiveParticle.vel_x = (int)var_l4;
          g_ActiveParticle.vel_y = (int)var_l3;
          var_l4 = __ftol();
          g_ActiveParticle.vel_z = (int)var_l4;
          g_ActiveParticle.rot_x = 0;
          g_ActiveParticle.rot_y = 0;
          g_ActiveParticle.rot_z = 0;
          g_ActiveParticle.drag = 0x3d7;
          g_ActiveParticle.gravity = 0xffffff67;
          g_ActiveParticle.field_24 = 0x1ec00;
          g_ActiveParticle.field_28 = 0xffffffe0;
          g_ActiveParticle.field_2c = 1;
          Math_RandomFloat0To1();
          var_l4 = __ftol();
          g_ActiveParticle.field_3c = (int)var_l4;
          Math_RandomFloat0To1();
          var_l4 = __ftol();
          g_ActiveParticle.field_40 = (int)var_l4;
          Math_RandomFloat0To1();
          var_l4 = __ftol();
          g_ActiveParticle.field_44 = (int)var_l4;
          g_ActiveParticle.field_48 = 0;
          g_ActiveParticle.field_4c = 0x3c;
          FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
          var_i2 = var_i2 + -1;
        } while (var_i2 != 0);
      }
      var_24 = var_24 + 1;
    } while (var_24 < 4);
  }
  return;
}

/**
 * @original FX_SpawnLandingDustPuffs (IGN_WIN.EXE @ 0x004338d0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnLandingDustPuffs(void) {
  int var_i1;
  int var_i2;
  double var_f3;
  __int64 var_l4;
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (*(int *)(g_Vehicles + 0x278 + g_ActiveVehicleIndex * 0x484c) == 1) {
    var_i2 = *(int *)(var_i1 + 0x158);
    if (((var_i2 < 0) || (0x27 < var_i2)) &&
       ((var_i2 = *(int *)(var_i1 + 0x15c), var_i2 < 0 || (0x27 < var_i2)))) {
      var_i2 = 0;
    }
    if (((int*)&(DAT_00497438))[var_i2] == '\x01') {
      var_f3 = (double)*(double *)(var_i1 + 0xf8) + (double)_DAT_0047a3c0;
      fcos(var_f3);
      fsin(var_f3);
      g_ActiveParticle.type = 1;
      var_l4 = __ftol();
      g_ActiveParticle.pos_x = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_y = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_x = (int)var_l4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_z = (int)var_l4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_x = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_y = (int)var_l4;
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
      var_f3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(var_f3);
      fsin(var_f3);
      g_ActiveParticle.type = 1;
      var_l4 = __ftol();
      g_ActiveParticle.pos_x = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_y = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_x = (int)var_l4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_z = (int)var_l4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.field_28 = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_x = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_y = (int)var_l4;
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
      var_f3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(var_f3);
      fsin(var_f3);
      g_ActiveParticle.type = 1;
      var_l4 = __ftol();
      g_ActiveParticle.pos_x = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_y = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_x = (int)var_l4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_z = (int)var_l4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_x = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_y = (int)var_l4;
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
      var_f3 = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
              (double)_DAT_0047a3c0;
      fcos(var_f3);
      fsin(var_f3);
      g_ActiveParticle.type = 1;
      var_l4 = __ftol();
      g_ActiveParticle.pos_x = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_y = (int)var_l4;
      var_l4 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_x = (int)var_l4;
      g_ActiveParticle.vel_y = 0xfffffc00;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.vel_z = (int)var_l4;
      g_ActiveParticle.gravity = 0;
      g_ActiveParticle.drag = 0x400;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_2c = 0xcc;
      g_ActiveParticle.field_28 = 0;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_x = (int)var_l4;
      Math_RandomFloat0To1();
      var_l4 = __ftol();
      g_ActiveParticle.rot_y = (int)var_l4;
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
  int var_i1;
  double var_d2;
  int var_i3;
  double var_f4;
  __int64 var_l5;
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  var_i3 = *(int *)(var_i1 + 0x158);
  if (((var_i3 < 0) || (0x27 < var_i3)) &&
     ((var_i3 = *(int *)(var_i1 + 0x15c), var_i3 < 0 || (0x27 < var_i3)))) {
    var_i3 = 0;
  }
  if ((((var_i3 == 4) || (var_i3 == 0xe)) || (var_i3 == 0x18)) &&
     ((_DAT_0047a3e8 < *(double *)(var_i1 + 0x118) && (*(int *)(var_i1 + 0x270) == 0)))) {
    var_d2 = *(double *)(var_i1 + 0x118) * *(double *)(var_i1 + 0x118) * _DAT_0047a3f0;
    var_f4 = Math_RandomFloat0To1();
    if (var_f4 * (double)_DAT_0047a3f8 < (double)var_d2) {
      g_ActiveParticle.type = 1;
      var_l5 = __ftol();
      g_ActiveParticle.pos_x = (int)var_l5;
      var_l5 = __ftol();
      g_ActiveParticle.pos_y = (int)var_l5;
      var_l5 = __ftol();
      g_ActiveParticle.pos_z = (int)var_l5;
      g_ActiveParticle.vel_x = 0;
      Math_RandomFloat0To1();
      var_l5 = __ftol();
      g_ActiveParticle.vel_y = (int)var_l5;
      g_ActiveParticle.vel_z = 0;
      g_ActiveParticle.drag = 0x3f5;
      g_ActiveParticle.field_24 = 4;
      g_ActiveParticle.field_28 = 0x800;
      g_ActiveParticle.field_2c = 0x3d;
      g_ActiveParticle.gravity = 0;
      Math_RandomFloat0To1();
      var_l5 = __ftol();
      g_ActiveParticle.rot_x = (int)var_l5;
      Math_RandomFloat0To1();
      var_l5 = __ftol();
      g_ActiveParticle.rot_y = (int)var_l5;
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
  unsigned int var_u1;
  int var_i2;
  char *var_pu3;
  int *var_pi4;
  int *var_pi5;
  int var_i6;
  unsigned int var_u7;
  int *var_pi8;
  int var_i9;
  int var_i10;
  __int64 var_u11;
  __int64 var_l12;
  __int64 var_l13;
  __int64 var_l14;
  int var_18;
  int var_10;
  int var_c;
  if (((*(int *)(g_Vehicles + 0x528) != 1) || (g_RaceTimer < _DAT_0047a448)) ||
     ((g_HudEnabled != 1 && (g_HudEnabled != 2)))) {
    if ((g_HudEnabled == 1) || (g_HudEnabled == 2)) {
      var_18 = 0;
      if (0 < DAT_0054f98c) {
        do {
          if (var_18 == 0) {
            var_10 = g_MenuCursorPos;
            var_pi4 = DAT_00563c34;
          }
          else {
            var_10 = 1;
            var_pi4 = DAT_00563c30;
          }
          if ((double)(DAT_00563cf4 + 2) < _DAT_005285d8) {
            if (((((int*)&(DAT_00552f30))[var_18] == 1) && (g_RaceTimer < 0.0)) &&
               (var_i9 = var_pi4[1], (double)var_i9 < _DAT_0047a458)) {
              var_i10 = var_i9 / 2;
              if ((DAT_0054f98c == 1) && (g_HudEnabled == 1)) {
                var_u11 = Palette_AdjustRGB(var_i9,var_i9 >> 0x1f,g_ActiveTrackPalette + 8);
                Gfx_FreeSurface((int)var_u11);
              }
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              var_i9 = (int)var_l12;
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              var_i6 = (int)var_l12;
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              var_i2 = (int)var_l12;
              if (g_HudEnabled == 1) {
                *var_pi4 = *var_pi4 + 4;
                var_pi4[1] = var_pi4[1] + 2;
                var_pi4[var_i10 * 0xc + 2] = var_i9;
                var_pi4[var_i10 * 0xc + 3] = -var_i6;
                var_pi4[var_i10 * 0xc + 4] = var_i2;
                var_pi4[var_i10 * 0xc + 5] = var_i9 + 4;
                var_pi4[var_i10 * 0xc + 6] = 0x3c - var_i6;
                var_pi4[var_i10 * 0xc + 7] = var_i2;
                var_pi4[var_i10 * 0xc + 8] = var_i9;
                var_pi4[var_i10 * 0xc + 9] = -800 - var_i6;
                var_pi4[var_i10 * 0xc + 10] = var_i2;
                var_pi4[var_i10 * 0xc + 0xb] = var_i9 + 4;
                var_pi4[var_i10 * 0xc + 0xc] = -0x2e4 - var_i6;
                var_pi4[var_i10 * 0xc + 0xd] = var_i2;
              }
              else if (g_HudEnabled == 2) {
                *var_pi4 = *var_pi4 + 2;
                var_pi4[1] = var_pi4[1] + 2;
                var_pi4[var_i10 * 6 + 2] = var_i9;
                var_pi4[var_i10 * 6 + 3] = -var_i6;
                var_pi4[var_i10 * 6 + 4] = var_i2;
                var_pi4[var_i10 * 6 + 5] = var_i9;
                var_pi4[var_i10 * 6 + 6] = -800 - var_i6;
                var_pi4[var_i10 * 6 + 7] = var_i2;
              }
              var_i9 = var_i10 * 2 + 2;
              var_i6 = 0;
              if (0 < var_i9) {
                var_pi8 = var_pi4 + (var_i10 + 1) * 6 + 2;
                var_pi5 = var_pi4 + (var_i10 + 1) * 0xc + 2;
                var_i10 = 0;
                do {
                  if (g_HudEnabled == 1) {
                    *var_pi5 = 0xd;
                    var_pi5[1] = var_i10;
                    var_pi5[2] = var_i10 + 1;
                    var_pi5[3] = 0x7700;
                    var_pi5[4] = 0x7b00;
                    var_pi5[5] = DAT_00527f74;
                  }
                  else if (g_HudEnabled == 2) {
                    *var_pi8 = 7;
                    var_pi8[1] = var_i6;
                    var_pi8[2] = 0;
                    var_pi8[3] = 0x7500;
                    var_pi8[4] = 0x1f00;
                    var_pi8[5] = 0x9400;
                    var_pi8[6] = 0;
                    var_pi8[7] = DAT_00552d80 * 100;
                    var_pi8[8] = DAT_00552f5c * 100;
                  }
                  var_pi8 = var_pi8 + 9;
                  var_pi5 = var_pi5 + 6;
                  var_i10 = var_i10 + 2;
                  var_i6 = var_i6 + 1;
                } while (var_i6 < var_i9);
              }
            }
            if (((((int*)&(DAT_00552f30))[var_18] == 0) || (0.0 <= g_RaceTimer)) && (0 < var_pi4[1])) {
              if (g_HudEnabled == 1) {
                var_i9 = *var_pi4 + -4;
LAB_00434c38:
                *var_pi4 = var_i9;
                var_pi4[1] = var_pi4[1] + -2;
              }
              else if (g_HudEnabled == 2) {
                var_i9 = *var_pi4 + -2;
                goto LAB_00434c38;
              }
              var_i9 = var_pi4[1] / 2;
              if ((DAT_0054f98c == 1) && (g_HudEnabled == 1)) {
                var_u11 = Palette_AdjustRGB(var_pi4,var_pi4[1] >> 0x1f,g_ActiveTrackPalette + 8);
                Gfx_FreeSurface((int)var_u11);
              }
              var_i10 = 0;
              if (0 < var_i9 * 2) {
                var_pi8 = var_pi4 + var_i9 * 6 + 2;
                var_pi5 = var_pi4 + var_i9 * 0xc + 2;
                var_i6 = 0;
                do {
                  if (g_HudEnabled == 1) {
                    *var_pi5 = 0xd;
                    var_pi5[1] = var_i6;
                    var_pi5[2] = var_i6 + 1;
                    var_pi5[3] = 0x7700;
                    var_pi5[4] = 0x7b00;
                    var_pi5[5] = DAT_00527f74;
                  }
                  else if (g_HudEnabled == 2) {
                    *var_pi8 = 7;
                    var_pi8[1] = var_i10;
                    var_pi8[2] = 0;
                    var_pi8[3] = 0x7500;
                    var_pi8[4] = 0x1f00;
                    var_pi8[5] = 0x9400;
                    var_pi8[6] = 0;
                    var_pi8[7] = DAT_00552d80 * 100;
                    var_pi8[8] = DAT_00552f5c * 100;
                  }
                  var_pi8 = var_pi8 + 9;
                  var_pi5 = var_pi5 + 6;
                  var_i6 = var_i6 + 2;
                  var_i10 = var_i10 + 1;
                } while (var_i10 < var_i9 * 2);
              }
            }
          }
          if ((g_HudEnabled == 1) && (0 < *var_pi4)) {
            *(int *)(&DAT_00563d28 + var_18 * 4) = *(int *)(&DAT_00563d28 + var_18 * 4) + 5;
            *(int *)(&DAT_00563d38 + var_18 * 4) = *(int *)(&DAT_00563d38 + var_18 * 4) + -0x19;
            Math_RandomFloat0To1();
            var_l12 = __ftol();
            if (((int)var_l12 == 0) && (g_RaceTimer < 0.0)) {
              __ftol();
              __ftol();
              Audio_PlaySampleVol(1,0,1,0,0x10000,0,0);
            }
          }
          else if ((g_HudEnabled == 2) && (0 < *var_pi4)) {
            *(double *)(&DAT_00525e30 + var_18 * 8) =
                 *(double *)(&DAT_00525e30 + var_18 * 8) + _DAT_0047a480;
            if (_DAT_0047a488 < *(double *)(&DAT_00525e30 + var_18 * 8)) {
              *(double *)(&DAT_00525e30 + var_18 * 8) =
                   *(double *)(&DAT_00525e30 + var_18 * 8) - _DAT_0047a488;
            }
            fsin((double)*(double *)(&DAT_00525e30 + var_18 * 8));
            var_l12 = __ftol();
            var_i9 = g_Vehicles;
            *(int *)(&DAT_00563d28 + var_18 * 4) = (int)var_l12;
            *(int *)(&DAT_00563d38 + var_18 * 4) = *(int *)(&DAT_00563d38 + var_18 * 4) + -5;
            fcos((double)*(double *)(var_i9 + 0x100 + var_10 * 0x484c));
            var_l12 = __ftol();
            fsin((double)*(double *)(var_i9 + var_10 * 0x484c + 0x100));
            var_l13 = __ftol();
            var_i9 = var_pi4[1] / 2;
            if (0 < var_i9) {
              var_pi4 = var_pi4 + 2;
              do {
                var_i6 = *var_pi4 + (int)var_l12;
                var_i10 = (int)var_l13 + var_pi4[2];
                var_i9 = var_i9 + -1;
                var_pi4[3] = var_i6;
                var_pi4[5] = var_i10;
                var_pi4[9] = var_i6;
                var_pi4[0xb] = var_i10;
                var_pi4 = var_pi4 + 6;
              } while (var_i9 != 0);
            }
          }
          var_pi4 = (int *)(&DAT_00563d38 + var_18 * 4);
          if (*var_pi4 < -800) {
            *(int *)(&DAT_00563d28 + var_18 * 4) = 0;
            *var_pi4 = *var_pi4 + 800;
          }
          var_c = 0;
          var_i9 = var_18 * 0x120;
          do {
            var_i10 = 0;
            do {
              var_l12 = __ftol();
              var_l13 = __ftol();
              var_l14 = __ftol();
              *(int *)(DAT_00553030 + 4 + var_i9) =
                   *(int *)(&DAT_00563d28 + var_18 * 4) + (((int)var_l14 + var_i10) * 5 + -5) * 200;
              *(int *)(DAT_00553030 + 8 + var_i9) = *var_pi4 + (int)var_l12;
              *(int *)(DAT_00553030 + 0xc + var_i9) = ((var_c + (int)var_l13) * 5 + -5) * 200;
              *(int *)(DAT_00553030 + 0x10 + var_i9) = 0;
              *(int *)(DAT_00553030 + 0x14 + var_i9) = 0;
              *(int *)(DAT_00553030 + 0x18 + var_i9) = 0;
              var_i6 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_00553030 + var_i9));
              if (var_i6 != 0) {
                Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_WEATHER_0049948c);
              }
              var_i9 = var_i9 + 0x20;
              var_i10 = var_i10 + 1;
            } while (var_i10 < 3);
            var_c = var_c + 1;
          } while (var_c < 3);
          var_pu3 = Audio_GetVoice(((int*)&(DAT_00525e50))[var_18]);
          if (var_pu3 != (char *)0x0) {
            var_l12 = __ftol();
            *(int *)(var_pu3 + 0xc) = (int)var_l12;
          }
          if (((g_HudEnabled == 1) && (((int*)&(DAT_00552f30))[var_18] == 1)) && (DAT_00553084 == 1)) {
            Math_RandomFloat0To1();
            var_l12 = __ftol();
            if ((int)var_l12 == 0) {
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              var_18 = (int)var_l12;
              var_u1 = *(unsigned int *)(g_Vehicles + 0x364 + var_10 * 0x484c);
              var_u7 = (int)var_u1 >> 0x1f;
              var_i9 = ((var_u1 ^ var_u7) - var_u7) + var_18;
              if (var_i9 < DAT_005287f8) {
                var_i9 = *(int *)(g_pTrackRoadSequence + var_i9 * 0x18);
              }
              else {
                var_i9 = *(int *)(g_pTrackRoadSequence + var_18 * 0x18);
              }
              var_i9 = *(int *)(var_i9 * 0x14 + g_pActivePLC + 0x10);
              g_ActiveParticle.type = 6;
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              g_ActiveParticle.pos_x = (int)var_l12;
              g_ActiveParticle.pos_y = var_i9 * -0x400;
              Math_RandomFloat0To1();
              var_l12 = __ftol();
              g_ActiveParticle.pos_z = (int)var_l12;
              g_ActiveParticle.vel_z = 6;
              g_ActiveParticle.vel_y = 0;
              g_ActiveParticle.vel_x = var_10;
              FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
              __ftol();
              __ftol();
              Audio_PlaySampleVol(1,0,2,0,0x10000,0,0);
            }
          }
          var_18 = var_18 + 1;
        } while (var_18 < DAT_0054f98c);
      }
      if ((double)(DAT_00563cf4 + 2) < _DAT_005285d8) {
        var_l12 = __ftol();
        DAT_00563cf4 = (int)var_l12;
      }
    }
  }
  else if (((0 < DAT_00563c34[1]) || (0 < DAT_00563c30[1])) &&
          (((g_IsSplitScreen == 0 && (_DAT_0047a448 <= g_RaceTimer)) ||
           (((g_IsSplitScreen == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)) &&
            (_DAT_0047a448 <= g_RaceTimer)))))) {
    Gfx_FreeSurface(g_ActiveTrackPalette + 8);
    DAT_00563c34[1] = 0;
    DAT_00563c30[1] = 0;
  }
  if ((g_HudEnabled == 1) || (g_HudEnabled == 3)) {
    var_i9 = 0;
    var_i10 = 0;
    if (0 < DAT_0054f98c) {
      do {
        var_pu3 = Audio_GetVoice(*(int *)((int)&DAT_00525e50 + var_i9));
        if (var_pu3 != (char *)0x0) {
          if ((*(int *)((int)&DAT_00552f30 + var_i9) == 1) && (*(int *)(var_pu3 + 0xc) < 30000)) {
            *(int *)(var_pu3 + 0xc) = *(int *)(var_pu3 + 0xc) + 0x9c4;
          }
          else if ((*(int *)((int)&DAT_00552f30 + var_i9) == 0) &&
                  ((0 < *(int *)(var_pu3 + 0xc) &&
                   (var_i6 = *(int *)(var_pu3 + 0xc) + -0x9c4, *(int *)(var_pu3 + 0xc) = var_i6,
                   var_i6 < 0)))) {
            *(int *)(var_pu3 + 0xc) = 0;
          }
        }
        var_i9 = var_i9 + 4;
        var_i10 = var_i10 + 1;
      } while (var_i10 < DAT_0054f98c);
    }
  }
  return;
}

/**
 * @original FX_FrameTick (IGN_WIN.EXE @ 0x00435350, fx.c)
 * @fidelity ADAPTED
 */
void FX_FrameTick(void) {
  int var_i1;
  double *var_pd2;
  double var_d3;
  int *var_pi4;
  int *var_pi5;
  double var_f6;
  g_ActiveVehicleIndex = 0;
  if (0 < g_NumRacers) {
    do {
      var_d3 = *(double *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) * g_Const_DegToRad;
      var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
      if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
        *(double *)(var_i1 + 0x584) =
             ((var_d3 + *(double *)(var_i1 + 0xb8)) * g_Const_0_05 - *(double *)(var_i1 + 0x584)) *
             g_Const_0_1 + *(double *)(var_i1 + 0x584);
        var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
        var_f6 = (double)Math_AngleMod(g_ActiveVehicleIndex * 0x909);
        var_d3 = (double)var_f6;
        if ((double)g_Const_Pi < var_f6) {
          do {
            var_d3 = var_d3 - g_Const_TwoPi;
          } while (g_Const_Pi < var_d3);
        }
        for (; var_d3 < g_Const_NegPi; var_d3 = var_d3 + g_Const_TwoPi) {
        }
        *(double *)(var_i1 + 0x58c) = var_d3 * g_Const_0_1 + *(double *)(var_i1 + 0x58c);
      }
      else {
        *(double *)(var_i1 + 0x584) = var_d3 * g_Const_0_05;
        *(double *)(g_Vehicles + 0x58c + g_ActiveVehicleIndex * 0x484c) =
             *(double *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) * g_Const_DegToRad *
             g_Const_0_05;
      }
      FX_SpawnWaterSplashes();
      var_pd2 = (double *)(g_Vehicles + 0x60c + g_ActiveVehicleIndex * 0x484c);
      if ((0.0 < *(double *)(g_Vehicles + 0x60c + g_ActiveVehicleIndex * 0x484c)) &&
         (*var_pd2 < g_Const_0_05)) {
        *var_pd2 = *var_pd2 + _DAT_0047a510;
      }
      g_ActiveVehicleIndex = g_ActiveVehicleIndex + 1;
    } while (g_ActiveVehicleIndex < g_NumRacers);
  }
  FX_UpdateWeatherGeometry();
  if ((((g_IsSplitScreen == 0) && (*(int *)(g_Vehicles + 0x528) == 1)) ||
      ((g_IsSplitScreen == 1 &&
       ((*(int *)(g_Vehicles + 0x528) == 1 && (*(int *)(g_Vehicles + 0x4d74) == 1)))))) &&
     ((_DAT_0047a518 <= g_RaceTimer &&
      ((DAT_00563c60 == 0 && (DAT_005db038 = DAT_005db038 + 10, 0x13f < DAT_005db038)))))) {
    DAT_005db038 = 0x13f;
    if (g_GameMode == 3) {
      if (g_LapsTotal + -1 <= 5 - g_RacePosition) goto LAB_00435673;
      g_RacePosition = g_RacePosition + -1;
    }
    else {
      if (g_RacePosition < g_LapsTotal + -1) {
        DAT_005db038 = 0;
        g_RacePosition = g_RacePosition + 1;
      }
      if (((_DAT_00553090 <= (double)(DAT_00525e44 / 100)) || (1 < g_RacePosition)) ||
         (((*(int *)(g_Vehicles + 0x528) != 0 || (g_LapsTotal != 0)) &&
          ((*(int *)(g_Vehicles + 0x528) != 1 || (g_LapsTotal != 1)))))) goto LAB_00435673;
      g_RacePosition = g_RacePosition + 1;
    }
    DAT_005db038 = 0;
  }
LAB_00435673:
  var_pi4 = &DAT_006192a0;
  do {
    var_pi5 = var_pi4 + 1;
    *var_pi4 = *var_pi4 + 1;
    var_pi4[10] = var_pi4[10] + 1;
    var_pi4 = var_pi5;
  } while (var_pi5 < &DAT_006192c8);
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
  int *var_pi1;
  int *var_pi2;
  int var_i3;
  int var_i4;
  int var_i5;
  int *var_pi6;
  int var_i7;
  int var_i8;
  int *var_pi9;
  var_i7 = 0;
  if (0 < *g_pActivePLC) {
    do {
      var_i3 = *(int *)(g_pActivePOS + var_i7 * 4);
      if (var_i3 != -1) {
        var_pi2 = (int *)(g_pActivePOS + (var_i3 + var_i7) * 4);
        var_i3 = var_pi2[2];
        var_i4 = var_pi2[3];
        var_i5 = var_pi2[4];
        var_pi1 = var_pi2 + 2;
        var_i8 = 0;
        if (*var_pi2 != 1 && -1 < *var_pi2 + -1) {
          var_pi6 = var_pi1;
          var_pi9 = var_pi2 + 8;
          do {
            var_i8 = var_i8 + 1;
            *var_pi6 = *var_pi6 - *var_pi9;
            var_pi6[1] = var_pi6[1] - var_pi9[1];
            var_pi6[2] = var_pi6[2] - var_pi9[2];
            var_pi6 = var_pi6 + 6;
            var_pi9 = var_pi9 + 6;
          } while (var_i8 < *var_pi2 + -1);
        }
        var_pi2 = var_pi1 + var_i8 * 6;
        *var_pi2 = var_pi1[var_i8 * 6] - var_i3;
        var_pi2[1] = var_pi2[1] - var_i4;
        var_pi2[2] = var_pi2[2] - var_i5;
      }
      var_i7 = var_i7 + 1;
    } while (var_i7 < *g_pActivePLC);
  }
  return;
}

/**
 * @original Pos_UpdateAnimatedObjects (IGN_WIN.EXE @ 0x004357a0, fx.c)
 * @fidelity ADAPTED
 */
void Pos_UpdateAnimatedObjects(void) {
  int var_i1;
  int *var_pi2;
  int var_i3;
  int var_i4;
  int var_i5;
  int var_i6;
  var_i5 = 0;
  var_i6 = 0;
  if (0 < *g_pActivePLC) {
    var_i4 = 0;
    do {
      if ((*(int *)(g_pActivePOS + var_i5) != -1) && (*(int *)(DAT_0054f908 + 0x1c + var_i4) == 1)) {
        var_i3 = *(int *)(g_pActivePOS + var_i5) + var_i6;
        var_i1 = g_pActivePOS + var_i3 * 4;
        if (*(int *)(g_pActivePOS + 4 + var_i3 * 4) == *(int *)(g_pActivePOS + var_i3 * 4)) {
          *(int *)(var_i1 + 4) = 0;
        }
        var_i3 = var_i1 + 8 + *(int *)(var_i1 + 4) * 0x18;
        var_pi2 = (int *)(DAT_0054f908 + 4 + var_i4);
        *var_pi2 = *var_pi2 - *(int *)(var_i1 + 8 + *(int *)(var_i1 + 4) * 0x18);
        var_pi2 = (int *)(DAT_0054f908 + 8 + var_i4);
        *var_pi2 = *var_pi2 - *(int *)(var_i3 + 4);
        var_pi2 = (int *)(DAT_0054f908 + 0xc + var_i4);
        *var_pi2 = *var_pi2 - *(int *)(var_i3 + 8);
        *(int *)(DAT_0054f908 + 0x10 + var_i4) = *(int *)(var_i3 + 0xc);
        *(int *)(DAT_0054f908 + 0x14 + var_i4) = *(int *)(var_i3 + 0x10);
        *(int *)(DAT_0054f908 + 0x18 + var_i4) = *(int *)(var_i3 + 0x14);
        var_i3 = *(int *)(DAT_0054f908 + 0x10 + var_i4);
        if (0xe0f < var_i3) {
          *(int *)(DAT_0054f908 + 0x10 + var_i4) = var_i3 + -0xe10;
        }
        var_i3 = *(int *)(DAT_0054f908 + 0x14 + var_i4);
        if (0xe0f < var_i3) {
          *(int *)(DAT_0054f908 + 0x14 + var_i4) = var_i3 + -0xe10;
        }
        var_i3 = *(int *)(DAT_0054f908 + 0x18 + var_i4);
        if (0xe0f < var_i3) {
          *(int *)(DAT_0054f908 + 0x18 + var_i4) = var_i3 + -0xe10;
        }
        var_i3 = Lisa_UpdateObjectSpatialGrid((int *)(DAT_0054f908 + var_i4));
        if (var_i3 != 0) {
          Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_ANIM_OBJ_004994ac);
        }
        *(int *)(var_i1 + 4) = *(int *)(var_i1 + 4) + 1;
      }
      var_i6 = var_i6 + 1;
      var_i4 = var_i4 + 0x20;
      var_i5 = var_i5 + 4;
    } while (var_i6 < *g_pActivePLC);
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
  int var_i1;
  double *var_pd2;
  int var_i3;
  unsigned int var_u4;
  unsigned int var_u5;
  int var_i6;
  int var_i7;
  int var_i8;
  int var_i9;
  int var_i10;
  int var_i11;
  double var_d12;
  int var_i13;
  unsigned int var_u14;
  int *var_pi15;
  int var_i16;
  int var_i17;
  int *var_pi18;
  int var_i19;
  unsigned int *var_pu20;
  int var_i21;
  int *var_pi22;
  double var_f23;
  __int64 var_l24;
  int var_34;
  unsigned int var_2c;
  int *var_28;
  int var_24;
  int var_20;
  int uStack_1c;
  __int64 var_18;
  int var_10;
  int var_4;
  var_i16 = 0;
  var_2c = 0;
  var_10 = 0;
  var_4 = 0;
  var_u14 = *(unsigned int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c);
  var_i13 = (var_u14 ^ (int)var_u14 >> 0x1f) - ((int)var_u14 >> 0x1f);
  if (var_i13 + 2 < DAT_005287f8) {
    if ((int)var_u14 < 0) {
      var_18 = *(double *)(g_pTrackRoadSequence + var_i13 * 6 + 0x10) - _DAT_0047a540;
      var_pi15 = g_pTrackRoadSequence + var_i13 * 6 + 0xf;
      var_24 = *var_pi15;
      if (var_18 < _DAT_0047a548) {
        var_18 = var_18 + _DAT_0047a550;
      }
      if (var_24 == 10000) {
        var_18 = *(double *)(g_pTrackRoadSequence + var_u14 * 6 + 0xd) - _DAT_0047a540;
        var_24 = g_pTrackRoadSequence[var_u14 * 6 + 0xc];
        if (var_18 < _DAT_0047a548) {
          var_18 = var_18 + _DAT_0047a550;
        }
      }
      if (var_24 != -2) goto LAB_00435a9e;
      var_i19 = 0;
      do {
        var_pi18 = var_pi15 + 6;
        var_pi15 = var_pi15 + 6;
        var_i19 = var_i19 + 1;
      } while (*var_pi18 == -2);
      var_18 = *(double *)(g_pTrackRoadSequence + (var_i19 + var_i13) * 6 + 0xd);
      var_24 = g_pTrackRoadSequence[(var_i19 + var_i13) * 6 + 0xc];
    }
    else {
      var_18 = *(double *)(g_pTrackRoadSequence + var_u14 * 6 + 0xd);
      var_24 = g_pTrackRoadSequence[var_u14 * 6 + 0xc];
    }
  }
  else {
    var_18 = *(double *)(g_pTrackRoadSequence + 1);
    var_24 = *g_pTrackRoadSequence;
  }
  var_18 = var_18 - _DAT_0047a540;
  if (var_18 < _DAT_0047a548) {
    var_18 = var_18 + _DAT_0047a550;
  }
LAB_00435a9e:
  var_i17 = var_24 * 0x14 + g_pActivePLC;
  var_i19 = *(int *)(var_i17 + 0x10);
  var_i13 = g_pActiveMSH + *(int *)(var_i17 + 4) * 4;
  var_34 = *(int *)(var_i13 + 4);
  var_pu20 = (unsigned int *)(var_i13 + 8 + *(int *)(g_pActiveMSH + *(int *)(var_i17 + 4) * 4) * 0xc);
  if (0 < var_34) {
    do {
      var_u14 = var_pu20[1];
      var_u4 = var_pu20[2];
      var_u5 = var_pu20[3];
      if ((*var_pu20 & 0xffff0000) < 0x280000) {
        var_2c = var_2c +
                   *(int *)(var_i13 + 8 + var_u14 * 0xc) + (*(int *)(var_i17 + 0xc) + 0x6400) * 3 +
                   *(int *)(var_i13 + 8 + var_u4 * 0xc) + *(int *)(var_i13 + 8 + var_u5 * 0xc);
        var_i16 = (((((var_i16 - *(int *)(var_i13 + 0xc + var_u14 * 0xc)) + var_i19) -
                   *(int *)(var_i13 + 0xc + var_u4 * 0xc)) + var_i19) -
                 *(int *)(var_i13 + 0xc + var_u5 * 0xc)) + var_i19;
        var_10 = var_10 +
                   *(int *)(var_i13 + 0x10 + var_u14 * 0xc) + (*(int *)(var_i17 + 0x14) + 0x6400) * 3 +
                   *(int *)(var_i13 + 0x10 + var_u4 * 0xc) + *(int *)(var_i13 + 0x10 + var_u5 * 0xc);
        var_4 = var_4 + 3;
      }
      var_pu20 = var_pu20 + 0xb;
      var_34 = var_34 + -1;
    } while (var_34 != 0);
  }
  var_2c = (int)var_2c / var_4;
  var_10 = var_10 / var_4;
  var_u14 = var_i16 / var_4 + 0xfa;
  DAT_0063c5d8 = *(int *)(DAT_00552e40 + var_24 * 0xc);
  *(int *)(g_pActiveCamera + 7) = 0;
  *g_pActiveCamera = (double)(int)var_2c;
  var_d12 = (double)var_10;
  g_pActiveCamera[1] = (double)(var_i16 / var_4 + 0x2ee);
  uStack_1c = (int)((unsigned __int64)var_d12 >> 0x20);
  var_20 = SUB84(var_d12,0);
  g_pActiveCamera[2] = var_d12;
  var_pd2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 3) = 0;
  *(int *)((int)var_pd2 + 0x1c) = 0x408a9000;
  var_pd2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 4) = 0;
  *(int *)((int)var_pd2 + 0x24) = 0;
  var_pd2 = g_pActiveCamera;
  *(int *)(g_pActiveCamera + 5) = 0;
  *(int *)((int)var_pd2 + 0x2c) = 0;
  *(int *)((int)g_pActiveCamera + 0x7c) = 0;
  *(int *)(g_pActiveCamera + 0x10) = *(int *)(g_VehicleConfigs + 0x58 + g_ActiveVehicleIndex * 200);
  *(int *)((int)g_pActiveCamera + 0x84) =
       *(int *)(g_VehicleConfigs + 0x5c + g_ActiveVehicleIndex * 200);
  var_l24 = __ftol();
  *(int *)(g_pActiveCamera + 0x11) = (int)var_l24;
  Lisa_CullObjectsOrthographic();
  var_pi15 = Track_FindSurfaceHeight(var_2c,var_u14,var_10,-1,0x32);
  if (*var_pi15 == -1) {
    var_2c = var_2c - 0x14;
    var_pi15 = Track_FindSurfaceHeight(var_2c,var_u14,var_10,-1,0x32);
    if (*var_pi15 == -1) {
      Log_DebugPrintf((const char *)&DAT_004994cc);
    }
  }
  var_i13 = 0;
  do {
    var_i16 = 0;
    do {
      var_pi18 = var_pi15 + var_i16;
      var_i19 = g_ActiveVehicleIndex * 0x1213 + var_i16;
      var_i16 = var_i16 + 1;
      *(int *)(g_Vehicles + 0x170 + (var_i19 + var_i13) * 4) = *var_pi18;
    } while (var_i16 < 0x10);
    var_i13 = var_i13 + 0x10;
  } while (var_i13 < 0x40);
  var_i16 = var_pi15[2];
  var_i6 = var_pi15[6];
  var_i7 = var_pi15[4];
  var_i8 = var_pi15[9];
  var_i19 = var_pi15[3];
  var_i9 = var_pi15[7];
  var_i17 = var_pi15[1];
  var_i10 = var_pi15[8];
  var_i11 = var_pi15[5];
  var_i3 = var_pi15[5];
  var_i13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (*(int *)(var_i13 + 0x558) != 0) {
    *(int *)(var_i13 + 0x55c) = 0;
    if ((*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) != 2) || (g_ActiveVehicleIndex == 0)) {
      *(int *)(g_Vehicles + 0x560 + g_ActiveVehicleIndex * 0x484c) = 0;
    }
    var_l24 = Lisa_SetDynamicObjectMesh(*(int *)
                           (&DAT_005db000 + *(int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c) * 4),
                          g_PlayerHUDState,(int *)(g_ActiveVehicleIndex * 0x20 + g_pCarTransforms),
                          (int *)*(int *)
                                  (&DAT_005db000 + *(int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c) * 4)
                          ,1,(short)g_ActiveVehicleIndex + 100,0,-1,0);
    if ((int)var_l24 != 0) {
      Log_DebugPrintf(s_Error_while_changing_car_mesh_00499248);
    }
    var_i13 = 0;
    var_28 = &g_ParticleArray1 + g_ActiveVehicleIndex * 0x820;
    var_pi15 = (int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c);
    do {
      var_i21 = 0;
      var_i1 = var_i13 + *var_pi15 * 4;
      if (0 < *(int *)(g_ParticleCount1 + var_i1 * 0xc)) {
        var_pi18 = var_28;
        var_pi22 = (int *)(*(int *)(&DAT_00552e60 + var_i1 * 4) + 8);
        do {
          var_i21 = var_i21 + 1;
          *var_pi18 = *(int *)(&DAT_005533b0 + (var_i13 + *var_pi15 * 4) * 4) + *var_pi22;
          var_pi18[1] = var_pi22[1];
          var_pi18[2] = *(int *)(&DAT_00553300 + (var_i13 + *var_pi15 * 4) * 4) + var_pi22[2];
          var_pi18 = var_pi18 + 3;
          var_pi22 = var_pi22 + 3;
        } while (var_i21 < *(int *)(g_ParticleCount1 + (var_i13 + *var_pi15 * 4) * 0xc));
      }
      var_i13 = var_i13 + 1;
      var_28 = var_28 + 0x208;
    } while (var_i13 < 4);
    *(int *)(g_Vehicles + 0x558 + g_ActiveVehicleIndex * 0x484c) = 0;
  }
  *(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c) = (double)(int)var_2c;
  *(double *)(g_Vehicles + 8 + g_ActiveVehicleIndex * 0x484c) = (double)(int)var_u14;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x14 + g_ActiveVehicleIndex * 0x484c) = uStack_1c;
  *(int *)(var_i1 + 0x10 + var_i13 * 0x484c) = var_20;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x18 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x1c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x20 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x24 + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x28 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x2c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x30 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x34 + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x38 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x3c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x40 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x44 + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x10c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c) = ((int*)&var_18)[1];
  *(int *)(var_i1 + 0xf8 + var_i13 * 0x484c) = (int)var_18;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x100 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x104 + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x114 + var_i13 * 0x484c) = 0;
  var_f23 = (double)fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  var_pd2 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  var_pd2[9] = (double)((double)*var_pd2 -
                      var_f23 * (double)*(int *)(g_Vehicles + 0x5b0 + g_ActiveVehicleIndex * 0x484c));
  var_f23 = (double)fsin((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  var_i13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(var_i13 + 0x50) =
       (double)((double)*(double *)(var_i13 + 0x10) -
               var_f23 * (double)*(int *)(g_Vehicles + 0x5b0 + g_ActiveVehicleIndex * 0x484c));
  var_f23 = (double)fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  var_pd2 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  var_pd2[0xd] = (double)(var_f23 * (double)*(int *)(g_Vehicles + 0x5ac + g_ActiveVehicleIndex * 0x484c) +
                        (double)*var_pd2);
  var_f23 = (double)fsin((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
  var_i13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(var_i13 + 0x70) =
       (double)(var_f23 * (double)*(int *)(g_Vehicles + 0x5ac + g_ActiveVehicleIndex * 0x484c) +
               (double)*(double *)(var_i13 + 0x10));
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x58 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x5c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x60 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 100 + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x78 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x7c + var_i13 * 0x484c) = 0;
  var_i1 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x80 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i1 + 0x84 + var_i13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x150 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(int *)(g_Vehicles + 0x154 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(int *)(g_Vehicles + 0x158 + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  var_d12 = -((double)var_i3 +
            ((double)(var_i10 - var_i11) * (double)var_i16 +
            (double)var_i17 * ((double)(var_i9 - var_i7) - (double)(int)(var_2c - var_i7)) +
            (double)var_i19 * ((double)(var_i8 - var_i6) - (double)(var_10 - var_i6))) /
            (double)var_i16);
  *(int *)(g_Vehicles + 0x15c + g_ActiveVehicleIndex * 0x484c) = 0xffffffff;
  *(double *)(g_Vehicles + 0x120 + g_ActiveVehicleIndex * 0x484c) = var_d12;
  *(double *)(g_Vehicles + 0x128 + g_ActiveVehicleIndex * 0x484c) = var_d12;
  *(int *)(g_Vehicles + 0x270 + g_ActiveVehicleIndex * 0x484c) = 1;
  *(int *)(g_Vehicles + 0x344 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x348 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x34c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x27c + g_ActiveVehicleIndex * 0x484c) = 1;
  *(int *)(g_PlayerHUDState + 0x10 + g_ActiveVehicleIndex * 0x4c) = 0;
  *(int *)(g_Vehicles + 0x350 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x298 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x29c + g_ActiveVehicleIndex * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0x11c + var_i13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x274 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x278 + g_ActiveVehicleIndex * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x288 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0x28c + var_i13 * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x2f8 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x344 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x348 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x34c + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c) =
       *(int *)(DAT_00553784 + var_24 * 0xc);
  *(int *)(g_Vehicles + 0x368 + g_ActiveVehicleIndex * 0x484c) =
       *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c);
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x88 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0x8c + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x90 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0x94 + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0x98 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0x9c + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xa0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0xa4 + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xb0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0xb4 + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xb8 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0xbc + var_i13 * 0x484c) = 0;
  var_i16 = g_Vehicles;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_Vehicles + 0xc0 + g_ActiveVehicleIndex * 0x484c) = 0;
  *(int *)(var_i16 + 0xc4 + var_i13 * 0x484c) = 0;
  var_i13 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  *(double *)(var_i13 + 0x584) =
       (*(double *)(g_Vehicles + 0x108 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a560 +
       *(double *)(var_i13 + 0xb8)) * _DAT_0047a568;
  *(double *)(g_Vehicles + 0x58c + g_ActiveVehicleIndex * 0x484c) =
       (*(double *)(g_Vehicles + 0x110 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a560 +
       *(double *)(g_Vehicles + 0xc0 + g_ActiveVehicleIndex * 0x484c)) * _DAT_0047a568;
  var_i16 = g_VehicleConfigs;
  var_i13 = g_ActiveVehicleIndex;
  *(int *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) = 0;
  *(int *)(var_i16 + 0xc4 + var_i13 * 200) = 0;
  var_i19 = g_Vehicles;
  var_i16 = g_VehicleCameraStates;
  var_i13 = g_ActiveVehicleIndex;
  var_i17 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i17 + 0x2c + g_VehicleCameraStates) =
       *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
  *(int *)(var_i17 + 0x28 + var_i16) = *(int *)(var_i19 + 0xf8 + var_i13 * 0x484c);
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x30 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x34 + var_i13) = 0;
  *(double *)(g_ActiveVehicleIndex * 0x1d0 + 0x68 + g_VehicleCameraStates) =
       *(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a570;
  *(double *)(g_ActiveVehicleIndex * 0x1d0 + 0x70 + g_VehicleCameraStates) =
       *(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a570;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x78 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x7c + var_i13) = 0;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x80 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x84 + var_i13) = 0;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x88 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x8c + var_i13) = 0;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x90 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x94 + var_i13) = 0;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0x98 + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0x9c + var_i13) = 0;
  var_i13 = g_VehicleCameraStates;
  var_i16 = g_ActiveVehicleIndex * 0x1d0;
  *(int *)(var_i16 + 0xac + g_VehicleCameraStates) = 0;
  *(int *)(var_i16 + 0xb0 + var_i13) = 0;
  *(int *)(g_ActiveVehicleIndex * 0x1d0 + 0xd4 + g_VehicleCameraStates) = 0;
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
  int var_i1;
  int *var_pi2;
  int var_i3;
  int var_4;
  if (g_GameMode == 3) {
    var_i1 = 1;
    var_i3 = 0;
    if (0 < g_NumRacers) {
      var_pi2 = (int *)(g_Vehicles + 0x39c);
      do {
        if ((var_pi2[99] == 0) && (var_i1 < *var_pi2)) {
          var_i1 = *var_pi2;
          var_4 = var_i3;
        }
        var_pi2 = var_pi2 + 0x1213;
        var_i3 = var_i3 + 1;
      } while (var_i3 < g_NumRacers);
    }
    if (var_i1 == 1) {
      var_i1 = g_NumRacers + 1;
    }
    if (var_4 == g_MenuCursorPos) {
      DAT_004949a8 = 0;
    }
    else {
      DAT_004949a8 = DAT_004949a8 + 1;
      if (0x14 < DAT_004949a8) {
        g_MenuCursorPos = var_4;
        return var_i1;
      }
    }
  }
  else {
    var_i3 = 0;
    var_i1 = g_NumRacers + 1;
    if (0 < g_NumRacers) {
      var_pi2 = (int *)(g_Vehicles + 0x39c);
      do {
        if ((var_pi2[99] == 0) && (*var_pi2 < var_i1)) {
          var_i1 = *var_pi2;
          var_4 = var_i3;
        }
        var_pi2 = var_pi2 + 0x1213;
        var_i3 = var_i3 + 1;
      } while (var_i3 < g_NumRacers);
    }
    if (var_4 == g_MenuCursorPos) {
      DAT_004949a8 = 0;
      return var_i1;
    }
    DAT_004949a8 = DAT_004949a8 + 1;
    if (0x14 < DAT_004949a8) {
      g_MenuCursorPos = var_4;
      return var_i1;
    }
  }
  return var_i1;
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
  int var_i1;
  int var_i2;
  char *out_ECX;
  char *var_pc3;
  char *out_EDX;
  int var_i4;
  int *var_pu5;
  int *var_pu6;
  int var_b7;
  __int64 var_u8;
  int var_i9;
  char *var_pc10;
  char *var_pc11;
  char var_18 [12];
  int var_c [3];
  var_pu5 = (int *)(g_RenderTargetSurface + 0x25800);
  Gfx_SetRenderTarget(var_pu5,0x140,0x140,0x1e0,8);
  g_pActiveDrawBuffer = DAT_00525e64;
  var_pu6 = var_pu5;
  for (var_i2 = 8000; var_i2 != 0; var_i2 = var_i2 + -1) {
    *var_pu6 = 0;
    var_pu6 = var_pu6 + 1;
  }
  if ((((DAT_00552f10 == 2) || (DAT_00552f10 == 3)) && (DAT_00563da0 != 1)) &&
     (((DAT_005daff0 != 1 && (0 < DAT_00552f28)) && (g_PlayerCarChoice < 6)))) {
    var_i4 = 1;
    var_i2 = 0;
  }
  else {
    var_i4 = 4;
    var_i2 = 4;
  }
  var_u8 = Font_GetTextWidth((const char *)(s_CONTINUE_00494d58 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,0,(int)var_u8,0,0);
  var_u8 = Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,1,(int)var_u8,0,0);
  var_u8 = Font_GetTextWidth((const char *)(&DAT_00494d94 + g_LanguageId * 0xd2), g_FontId_Medium);
  Menu_AddLayoutItem(0x39,2,(int)var_u8,0,0);
  var_u8 = Font_GetTextWidth((const char *)(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2), g_FontId_Medium);
  var_i1 = (int)var_u8;
  Menu_AddLayoutItem(0x39,3,var_i1,0,0);
  Menu_LayoutItems(var_pu5,0x140);
  Menu_ClearLayout();
  if (DAT_00639497 == 0) {
    var_pc11 = (char *)(g_LanguageId * 0x69);
    var_pc3 = s_DEFAULT_00494dee + g_LanguageId * 0xd2;
    var_pc10 = var_pc3;
  }
  else if (DAT_00639497 == -1) {
    var_pc3 = (char *)(g_LanguageId * 0x15);
    var_pc11 = s_RANDOM_00494e0c + g_LanguageId * 0xd2;
    var_pc10 = var_pc11;
  }
  else {
    var_pc10 = &DAT_00499504;
    var_pc3 = out_ECX;
    var_pc11 = out_EDX;
  }
  var_u8 = Font_GetTextWidth((const char *)(var_pc10), DAT_00552fdc);
  Menu_AddLayoutItem(var_i1 + 0x57,3,(int)var_u8,0,1);
  Menu_LayoutItems(var_pu5,0x140);
  Menu_ClearLayout();
  var_i9 = g_FontId_Large;
  if (DAT_00563c04 == 0) {
    var_i9 = g_FontId_Medium;
  }
  Font_DrawText(s_CONTINUE_00494d58 + g_LanguageId * 0xd2,var_i9,0x46,0xc);
  if (DAT_00563c04 == 1) {
    var_i2 = ((int*)&(g_FontId_Large))[var_i4];
  }
  else {
    var_i2 = ((int*)&(g_FontId_Large))[var_i2];
  }
  Font_DrawText(s_RESTART_00494d76 + g_LanguageId * 0xd2,var_i2,0x46,0x1f);
  var_i2 = g_FontId_Large;
  if (DAT_00563c04 == 2) {
    var_i2 = g_FontId_Medium;
  }
  Font_DrawText((const char *)(&DAT_00494d94 + g_LanguageId * 0xd2),var_i2,0x46,0x32);
  var_i2 = g_FontId_Large;
  if (DAT_00563c04 == 3) {
    var_i2 = g_FontId_Medium;
  }
  Font_DrawText(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2,var_i2,0x46,0x45);
  if (DAT_00639497 == 0) {
    var_pc11 = &g_Format_Str_s;
    var_pc3 = s_DEFAULT_00494dee + g_LanguageId * 0xd2;
  }
  else if (DAT_00639497 == -1) {
    var_pc11 = &g_Format_Str_s;
    var_pc3 = s_RANDOM_00494e0c + g_LanguageId * 0xd2;
  }
  else {
    var_pc11 = &g_Format_Str_d;
    var_pc3 = DAT_00552e54;
  }
  _sprintf(var_18,var_pc11,var_pc3);
  ((int*)&(g_FontAlignMode))[DAT_00552fdc * 400] = 1;
  Font_DrawText(var_18,DAT_00552fdc,(int)var_u8 / 2 + var_i1 + 100,0x45);
  var_i2 = 0;
  var_b7 = g_GameMode == 0;
  ((int*)&(g_FontAlignMode))[DAT_00552fdc * 400] = 0;
  if (var_b7) {
    var_u8 = Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
    if (0 < DAT_00552f28) {
      var_i4 = ((int)var_u8 + 0x67) * 0x100;
      do {
        var_i2 = var_i2 + 1;
        var_c[1] = 0x1b00;
        var_c[0] = var_i4;
        Gfx_DrawSprite(DAT_005287e8,var_c,0);
        var_i4 = var_i4 + 0x1a00;
      } while (var_i2 < DAT_00552f28);
    }
  }
  Gfx_BlitTransparentLUT((int)var_pu5,0,0,0x140,100,0x563db0,g_ScreenWidth / 2 + -0xa0,g_ScreenHeight / 2 + -0x32,
               g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
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
    g_pActiveDrawBuffer = DAT_00525e64;

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
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
}

/**
 * @original HUD_RenderTrackResults (IGN_WIN.EXE @ 0x00438a60, fx.c)
 * @fidelity ADAPTED
 */
int HUD_RenderTrackResults(void) {
  int var_u1;
  int var_i2;
  int *var_pi3;
  char *var_pc4;
  int *var_pu5;
  int var_u6;
  int var_i7;
  int var_i8;
  int *var_pi9;
  int out_EDX;
  int extraout_EDX_00;
  int extraout_EDX_01;
  int extraout_EDX_02;
  int extraout_EDX_03;
  unsigned int var_u10;
  int var_i11;
  int var_i12;
  int *var_pu13;
  unsigned int var_u14;
  int var_b15;
  __int64 var_u16;
  __int64 var_l17;
  int var_i18;
  int var_i19;
  int var_i20;
  int var_i21;
  char *var_pu22;
  int var_dc;
  int var_d8;
  int var_d0 [9];
  char var_ac [4];
  int var_a8;
  int var_a4 [12];
  int aiStack_74 [9];
  char var_50 [40];
  char var_28 [40];
  if (DAT_00527f28 != 0) {
    DAT_00527f28 = 0;
    Video_SetGraphicsMode();
  }
  var_i2 = 0;
  if (0 < g_NumRacers) {
    var_pi3 = g_PlayerHUDState + 1;
    var_pi9 = (int *)(g_Vehicles + 0x528);
    do {
      if ((*var_pi9 == 0) && (*var_pi3 == 0)) {
        return 0;
      }
      var_pi3 = var_pi3 + 0x13;
      var_pi9 = var_pi9 + 0x1213;
      var_i2 = var_i2 + 1;
    } while (var_i2 < g_NumRacers);
  }
  var_pi3 = var_d0;
  for (var_i2 = 8; var_pi3 = var_pi3 + 1, var_i2 != 0; var_i2 = var_i2 + -1) {
    *var_pi3 = 100;
  }
  var_i12 = 0;
  Gfx_SetClipRect(0,0,g_ScreenWidth,g_ScreenHeight);
  var_i2 = out_EDX;
  if (g_PlayerCarChoice == 0) {
    g_PlayerCarChoice = 1;
    if (0 < g_LapsTotal) {
      do {
        var_i2 = 0;
        if (0 < g_NumRacers) {
          var_pi3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*var_pi3 - var_i12 == 1) break;
            var_pi3 = var_pi3 + 0x1213;
            var_i2 = var_i2 + 1;
          } while (var_i2 < g_NumRacers);
          if (var_i2 < g_NumRacers) {
            var_d0[var_i12 + 1] = var_i2;
          }
        }
        var_i12 = var_i12 + 1;
      } while (var_i12 < g_LapsTotal);
    }
    var_u16 = Font_GetTextWidth((const char *)(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e), g_FontId_Medium);
    var_i2 = 0;
    do {
      var_i2 = var_i2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + var_i2) = 0;
    } while (var_i2 < 0x25800);
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    g_pActiveDrawBuffer = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)var_u16 / 2,0,(int)var_u16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    var_i2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e,var_i2,0xa0,0xd);
    var_b15 = DAT_00552f10 == 4;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    var_i2 = g_FontId_Small;
    if (var_b15) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,var_i2,0xa0,0x19c);
      var_i2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
      var_i2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      var_pc4 = s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e;
LAB_00438edb:
      Font_DrawText(var_pc4,var_i2,0x140,0xd3);
      var_i2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(var_i2,0x140,0x140,0x1e0,8);
    }
    else {
      if (g_GameMode == 0) {
        Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32), g_FontId_Small);
        var_i2 = g_FontId_Small;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,var_i2,0xa0,0x19c);
        var_i2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
        var_i12 = g_LanguageId;
        var_i2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        var_pc4 = s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i12 * 0x32;
        goto LAB_00438edb;
      }
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d,var_i2,0xa0,0x19c);
      var_i2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
      var_i12 = g_LanguageId;
      var_i2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + var_i12 * 0x2d,var_i2,0x140,0xd3);
      var_i2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(var_i2,0x140,0x140,0x1e0,8);
      DAT_005285c8 = 1;
    }
    var_i2 = 0;
    if (0 < g_NumRacers) {
      var_i11 = 0x28;
      var_pu13 = &DAT_005286f4;
      var_i12 = 0x3000;
      do {
        var_d8 = var_i12;
        if (g_GameMode == 3) {
          var_dc = 0x7000;
          Gfx_DrawSprite(*var_pu13,&var_dc,0);
          var_i21 = 0x28;
          var_i20 = 0x8b;
          var_i8 = var_i11;
LAB_00439056:
          Menu_AddLayoutItem(var_i20,0,var_i21,var_i8,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
        }
        else {
          var_dc = 0x3400;
          Gfx_DrawSprite(*var_pu13,&var_dc,0);
          Menu_AddLayoutItem(0x4f,0,0x28,var_i11,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          Menu_AddLayoutItem(0x92,0,0x5f,var_i11,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          if (DAT_00553068 == 1) {
            var_d8 = var_i12 + 0x3c00;
            var_dc = 0x3400;
            Gfx_DrawSprite(var_pu13[1],&var_dc,0);
            Menu_AddLayoutItem(0x4f,0,0x28,var_i11 + 0x3c,0);
            Menu_LayoutItems(g_RenderTargetSurface,0x140);
            Menu_ClearLayout();
            var_i21 = 0x5f;
            var_i20 = 0x92;
            var_i8 = var_i11 + 0x3c;
            goto LAB_00439056;
          }
        }
        var_i11 = var_i11 + 0x3c;
        var_pu13 = var_pu13 + 1;
        var_i12 = var_i12 + 0x3c00;
        var_i2 = var_i2 + 1;
      } while (var_i2 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
    var_i2 = extraout_EDX_00;
  }
  if (g_PlayerCarChoice == 2) {
    var_i12 = 0;
    g_PlayerCarChoice = 3;
    if (0 < g_LapsTotal) {
      do {
        var_i2 = 0;
        if (0 < g_NumRacers) {
          var_pi3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*var_pi3 - var_i12 == 1) break;
            var_pi3 = var_pi3 + 0x1213;
            var_i2 = var_i2 + 1;
          } while (var_i2 < g_NumRacers);
          if (var_i2 < g_NumRacers) {
            var_d0[var_i12 + 1] = var_i2;
          }
        }
        var_i12 = var_i12 + 1;
        var_i2 = g_LapsTotal;
      } while (var_i12 < g_LapsTotal);
    }
    var_u16 = Font_GetTextWidth((const char *)(s_TRACK_SCORE_00495528 + g_LanguageId * 0x1e), g_FontId_Medium);
    var_i2 = 0;
    do {
      var_i2 = var_i2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + var_i2) = 0;
    } while (var_i2 < 0x25800);
    var_i11 = 0;
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    g_pActiveDrawBuffer = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)var_u16 / 2,0,(int)var_u16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    var_i2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TRACK_SCORE_00495528 + g_LanguageId * 0x1e,var_i2,0xa0,0xd);
    var_i12 = g_LanguageId;
    var_i2 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i12 * 0x32), var_i2);
    var_i12 = g_LanguageId;
    var_i2 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i12 * 0x32,var_i2,0xa0,0x19c);
    var_i2 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
    var_i12 = g_LanguageId;
    var_i2 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i12 * 0x32,var_i2,0x140,0xd3);
    var_i2 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
    Gfx_SetRenderTarget(var_i2,0x140,0x140,0x1e0,8);
    if (0 < g_NumRacers) {
      var_pu13 = &DAT_005286f4;
      var_i2 = 0x28;
      var_i12 = 0x3000;
      do {
        var_u6 = *var_pu13;
        var_pu13 = var_pu13 + 1;
        var_i11 = var_i11 + 1;
        var_dc = 0x5600;
        var_d8 = var_i12;
        Gfx_DrawSprite(var_u6,&var_dc,0);
        Menu_AddLayoutItem(0x73,0,0x28,var_i2,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0xb7,0,0x16,var_i2,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        var_i2 = var_i2 + 0x3c;
        var_i12 = var_i12 + 0x3c00;
      } while (var_i11 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
    var_i2 = extraout_EDX_01;
  }
  if (g_PlayerCarChoice == 4) {
    var_i12 = 0;
    g_PlayerCarChoice = 5;
    if (0 < g_LapsTotal) {
      var_i2 = 0;
      do {
        var_i11 = 0;
        if (0 < g_NumRacers) {
          var_pi3 = (int *)(g_Vehicles + 0x3a0);
          do {
            if (*var_pi3 - var_i12 == 1) break;
            var_pi3 = var_pi3 + 0x1213;
            var_i11 = var_i11 + 1;
          } while (var_i11 < g_NumRacers);
          if (var_i11 < g_NumRacers) {
            var_d0[var_i12 + 1] = var_i11;
          }
        }
        var_i12 = var_i12 + 1;
      } while (var_i12 < g_LapsTotal);
    }
    var_u16 = Font_GetTextWidth((const char *)(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e), g_FontId_Medium);
    var_i2 = 0;
    do {
      var_i2 = var_i2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + var_i2) = 0;
    } while (var_i2 < 0x25800);
    var_b15 = 0;
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    g_pActiveDrawBuffer = DAT_00525e64;
    Menu_AddLayoutItem(0x93 - (int)var_u16 / 2,0,(int)var_u16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    var_i2 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e,var_i2,0xa0,0xd);
    var_i12 = g_NumRacers;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    var_i2 = g_FontId_Small;
    if (0 < var_i12) {
      do {
        if ((*(int *)(g_Vehicles + 0x3a0) < 4) ||
           ((*(int *)(g_Vehicles + 0x4bec) < 4 && (g_IsSplitScreen == 1)))) {
          var_b15 = 1;
        }
        var_i12 = var_i12 + -1;
      } while (var_i12 != 0);
    }
    if (var_b15) {
      if ((((g_DifficultyLevel == 4) && (DAT_006393ac == 0)) ||
          ((g_DifficultyLevel == 5 && (DAT_006393ac == 1)))) ||
         ((g_DifficultyLevel == 6 && ((DAT_006393ac == 2 || (DAT_006393ac == 3)))))) {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70 + g_LanguageId * 100,var_i2,0xa0,
                     0x19c);
        var_i2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
        var_i2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70 + g_LanguageId * 100,var_i2,0x140,
                     0xd3);
        var_i2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
        Gfx_SetRenderTarget(var_i2,0x140,0x140,0x1e0,8);
        DAT_00563d68 = 0;
      }
      else {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8 + g_LanguageId * 100,var_i2,0xa0,
                     0x19c);
        var_i2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
        Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
        var_i2 = g_FontId_Menu;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8 + g_LanguageId * 100,var_i2,0x140,
                     0xd3);
        var_i2 = g_RenderTargetSurface;
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
        Gfx_SetRenderTarget(var_i2,0x140,0x140,0x1e0,8);
        DAT_00619310 = g_CurrentTrackIndex + 1;
        DAT_00563d68 = 1;
      }
    }
    else {
      _sprintf((char *)var_a4,s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32);
      Font_GetTextWidth((const char *)var_a4, g_FontId_Small);
      var_i2 = g_FontId_Small;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText((char *)var_a4,var_i2,0xa0,0x19c);
      var_i2 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(var_i2,0x280,0x280,0xf0,8);
      var_i2 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText((char *)var_a4,var_i2,0x140,0xd3);
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
      DAT_00563d68 = 0;
      DAT_005285c8 = 0;
    }
    var_i2 = 0;
    if (0 < g_NumRacers) {
      var_pu13 = &DAT_005286f4;
      var_i12 = 0x3000;
      var_i11 = 0x28;
      do {
        var_u6 = *var_pu13;
        var_pu13 = var_pu13 + 1;
        var_i2 = var_i2 + 1;
        var_dc = 0x5600;
        var_d8 = var_i12;
        Gfx_DrawSprite(var_u6,&var_dc,0);
        Menu_AddLayoutItem(0x73,0,0x28,var_i11,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0xb7,0,0x16,var_i11,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        var_i12 = var_i12 + 0x3c00;
        var_i11 = var_i11 + 0x3c;
      } while (var_i2 < g_NumRacers);
    }
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
  }
  if (g_PlayerCarChoice == 6) {
    var_i2 = 0;
    do {
      var_i2 = var_i2 + 1;
      *(char *)(g_RenderTargetSurface + -1 + var_i2) = 0;
    } while (var_i2 < 0x25800);
    var_b15 = 0;
    DAT_00563d68 = 1;
    g_PlayerCarChoice = 7;
    if (0 < g_NumRacers) {
      var_i2 = g_NumRacers;
      do {
        if ((*(int *)(g_Vehicles + 0x3a0) < 4) ||
           ((*(int *)(g_Vehicles + 0x4bec) < 4 && (g_IsSplitScreen == 1)))) {
          var_b15 = 1;
        }
        var_i2 = var_i2 + -1;
      } while (var_i2 != 0);
    }
    if (var_b15) {
      *(int *)(g_Vehicles + 0x5d4) = 0;
      *(int *)(g_Vehicles + 20000) = 0;
      DAT_00563d68 = 0;
      _DAT_00601664 = 0;
      g_MenuCursorPos = 0;
      *(int *)(g_Vehicles + 0x604) = 1;
      if (g_IsSplitScreen == 1) {
        *(int *)(g_Vehicles + 0x4e50) = 1;
      }
      var_pu13 = &DAT_006192a0;
      do {
        *var_pu13 = 100;
        var_pu5 = var_pu13 + 1;
        var_pu13[10] = 100;
        var_pu13 = var_pu5;
      } while (var_pu5 < &DAT_006192b8);
      var_i12 = 0;
      var_i2 = g_NumRacers;
      if (0 < g_NumRacers) {
        var_pi3 = (int *)(g_Vehicles + 0x3a0);
        var_i11 = 0;
        do {
          var_i2 = *var_pi3;
          *(int *)((int)&DAT_00525e70 + var_i11) = var_i12;
          var_i8 = g_NumRacers;
          var_pi3 = var_pi3 + 0x1213;
          var_i12 = var_i12 + 1;
          *(int *)((int)&DAT_00553040 + var_i11) =
               *(int *)(&DAT_00492a24 + var_i2 * 4) + *(int *)((int)&DAT_00563d00 + var_i11);
          var_i11 = var_i11 + 4;
          var_i2 = g_NumRacers;
        } while (var_i12 < var_i8);
      }
      for (; -1 < var_i2; var_i2 = var_i2 + -1) {
        if (1 < g_NumRacers) {
          var_i11 = 4;
          var_i12 = g_NumRacers + -1;
          do {
            var_i8 = *(int *)((int)&DAT_00553040 + var_i11);
            if (*(int *)(var_i11 + 0x55303c) < var_i8) {
              var_u6 = *(int *)((int)&DAT_00525e70 + var_i11);
              *(int *)((int)&DAT_00553040 + var_i11) = *(int *)(var_i11 + 0x55303c);
              var_u1 = *(int *)((int)&DAT_00525e6c + var_i11);
              *(int *)(var_i11 + 0x55303c) = var_i8;
              *(int *)((int)&DAT_00525e70 + var_i11) = var_u1;
              *(int *)((int)&DAT_00525e6c + var_i11) = var_u6;
            }
            var_i11 = var_i11 + 4;
            var_i12 = var_i12 + -1;
          } while (var_i12 != 0);
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
  if ((0 < g_PlayerCarChoice) && (DAT_00553070 < g_LapsTotal)) {
    var_i2 = 0;
    if (0 < g_NumRacers) {
      var_pi3 = (int *)(g_Vehicles + 0x3a0);
      var_i12 = 0;
      do {
        var_i11 = *var_pi3;
        var_pi3 = var_pi3 + 0x1213;
        *(int *)((int)aiStack_74 + var_i12 + 4) =
             *(int *)(&DAT_00492a24 + var_i11 * 4) + *(int *)((int)&DAT_00563d00 + var_i12);
        *(int *)((int)var_d0 + var_i12 + 4) = var_i2;
        var_i2 = var_i2 + 1;
        var_i12 = var_i12 + 4;
      } while (var_i2 < g_NumRacers);
    }
    if ((DAT_00553070 < g_LapsTotal) && (var_i2 = g_NumRacers, g_PlayerCarChoice == 5)) {
      for (; -1 < var_i2; var_i2 = var_i2 + -1) {
        if (1 < g_NumRacers) {
          var_i12 = 4;
          var_i11 = g_NumRacers + -1;
          do {
            var_i8 = *(int *)((int)aiStack_74 + var_i12 + 4);
            if (*(int *)((int)aiStack_74 + var_i12) < var_i8) {
              var_u6 = *(int *)((int)var_d0 + var_i12 + 4);
              *(int *)((int)aiStack_74 + var_i12 + 4) = *(int *)((int)aiStack_74 + var_i12);
              *(int *)((int)var_d0 + var_i12 + 4) = *(int *)((int)var_d0 + var_i12);
              *(int *)((int)aiStack_74 + var_i12) = var_i8;
              *(int *)((int)var_d0 + var_i12) = var_u6;
            }
            var_i12 = var_i12 + 4;
            var_i11 = var_i11 + -1;
          } while (var_i11 != 0);
        }
      }
    }
    var_u10 = 0;
    if (0 < g_NumRacers) {
      do {
        if (g_GameMode == 3) {
          var_i2 = 0;
          if (0 < g_NumRacers) {
            var_pi3 = (int *)(g_Vehicles + 0x3a0);
            do {
              if (*var_pi3 + var_u10 == 6) break;
              var_pi3 = var_pi3 + 0x1213;
              var_i2 = var_i2 + 1;
            } while (var_i2 < g_NumRacers);
            if (var_i2 < g_NumRacers) {
              Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
              var_dc = 0x3b00;
              g_pActiveDrawBuffer = DAT_00525e64;
              if ((var_u10 & 1) == 0) {
                var_dc = 0xd300;
              }
              var_d8 = var_u10 * -0x3c00 + 0x15100;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[var_i2 * 0x13]],&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              var_i2 = var_u10 * -0x3c + 0x160;
              var_i11 = 0xad;
              goto LAB_0043a494;
            }
          }
        }
        else {
          var_i2 = 0;
          if (g_NumRacers < 1) {
LAB_00439d5b:
            if (DAT_00553068 != 1) goto LAB_0043a4e1;
          }
          else {
            var_pi3 = (int *)(g_Vehicles + 0x3a0);
            do {
              if (*var_pi3 - var_u10 == 1) break;
              var_pi3 = var_pi3 + 0x1213;
              var_i2 = var_i2 + 1;
            } while (var_i2 < g_NumRacers);
            if (g_NumRacers <= var_i2) goto LAB_00439d5b;
          }
          Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
          g_pActiveDrawBuffer = DAT_00525e64;
          if (g_PlayerCarChoice == 1) {
            if (DAT_00553068 == 0) {
              var_dc = 0x100;
              if ((var_u10 & 1) != 0) {
                var_dc = 0x10d00;
              }
              var_d8 = var_u10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[var_i2 * 0x13]],&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i2 = var_u10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,g_FontId_Medium,0x71,var_i2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            else {
              var_u10 = (unsigned int)((double)DAT_00525e44 * _DAT_0047a630 <=
                             *(double *)(g_Vehicles + 0x3a4));
              var_dc = 0x100;
              if ((double)DAT_00525e44 * _DAT_0047a630 <= *(double *)(g_Vehicles + 0x3a4)) {
                var_dc = 0x10d00;
              }
              var_d8 = var_u10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[*g_PlayerHUDState],&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i2 = var_u10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,g_FontId_Medium,0x71,var_i2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,var_i12,0xce,var_i2);
              var_u10 = var_u10 ^ 1;
              var_dc = 0x100;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              if ((char)var_u10 != '\0') {
                var_dc = 0x10d00;
              }
              var_d8 = var_u10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(DAT_00528738,&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i2 = var_u10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,g_FontId_Medium,0x71,var_i2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              HUD_FormatLapTime();
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            Font_DrawText((char *)var_a4,var_i12,0xce,var_i2);
            var_i2 = g_ScreenWidth;
            ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
            var_i12 = g_ScreenHeight;
          }
          else {
            if (g_PlayerCarChoice == 3) {
              var_dc = 0x2200;
              if ((var_u10 & 1) != 0) {
                var_dc = 0xeb00;
              }
              var_d8 = var_u10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[var_i2 * 0x13]],&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i2 = var_u10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,g_FontId_Medium,0x93,var_i2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              _sprintf((char *)var_a4,&g_Format_Str_d);
              var_i12 = g_FontId_Medium;
              var_i11 = 0xcf;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
            }
            else {
              if (g_PlayerCarChoice != 5) goto LAB_0043a4e1;
              var_dc = 0x2200;
              if ((var_u10 & 1) != 0) {
                var_dc = 0xeb00;
              }
              var_d8 = var_u10 * 0x3c00 + 0x2500;
              Gfx_DrawSprite(((int*)&(DAT_0052870c))[g_PlayerHUDState[var_d0[var_u10 + 1] * 0x13]],&var_dc,0);
              _sprintf((char *)var_a4,&g_Format_Str_s);
              var_i2 = var_u10 * 0x3c + 0x34;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              Font_DrawText((char *)var_a4,g_FontId_Medium,0x93,var_i2);
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
              _sprintf((char *)var_a4,&g_Format_Str_d);
              var_i12 = g_FontId_Medium;
              ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
              var_i11 = 0xcf;
            }
LAB_0043a494:
            Font_DrawText((char *)var_a4,var_i12,var_i11,var_i2);
            var_i12 = g_ScreenHeight;
            var_i2 = g_ScreenWidth;
            ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
          }
          Gfx_SetRenderTarget(&g_VirtualFramebuffer,var_i2,var_i2,var_i12,8);
          g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
        }
LAB_0043a4e1:
        var_u10 = var_u10 + 1;
      } while ((int)var_u10 < g_NumRacers);
    }
    DAT_00553070 = g_LapsTotal;
  }
  aiStack_74[1] = 0;
  if (0 < g_NumRacers) {
    var_pi3 = &DAT_00553010;
    var_i2 = 0;
    do {
      var_i12 = g_CurrentTrackIndex;
      var_i11 = g_Vehicles + var_i2;
      if ((((*(int *)(var_i11 + 0x528) == 1) && (*var_pi3 == 0)) && (g_PlayerHUDState[1] == 0)) &&
         ((var_i2 == 0 || ((var_i2 == 0x484c && (g_IsSplitScreen == 1)))))) {
        *var_pi3 = 1;
        var_i8 = var_i12 * 0x78;
        if (*(double *)(var_i11 + 0x3a4) < *(double *)(&DAT_00639593 + var_i12 * 0x78)) {
          *(int *)(&DAT_00639597 + var_i8) = *(int *)(var_i11 + 0x3a8);
          var_i12 = 4;
          *(int *)(&DAT_00639593 + var_i8) = *(int *)(var_i11 + 0x3a4);
          _sprintf(&DAT_0063956f + var_i8,&g_Format_Str_s);
          do {
            var_i11 = g_CurrentTrackIndex * 0xf + var_i12;
            if (*(double *)(&DAT_00639573 + var_i11 * 8) < *(double *)(&DAT_0063956b + var_i11 * 8)) {
              var_d0[2] = *(int *)(&DAT_0063956f + var_i11 * 8);
              var_d0[1] = *(int *)(&DAT_0063956b + var_i11 * 8);
              _sprintf(var_ac,&g_Format_Str_s);
              var_i11 = g_CurrentTrackIndex * 0xf + var_i12;
              *(int *)(&DAT_0063956f + var_i11 * 8) =
                   *(int *)(&DAT_00639577 + var_i11 * 8);
              *(int *)(&DAT_0063956b + var_i11 * 8) =
                   *(int *)(&DAT_00639573 + var_i11 * 8);
              _sprintf(&DAT_0063955b + (g_CurrentTrackIndex * 0x1e + var_i12) * 4,&g_Format_Str_s);
              var_i8 = g_CurrentTrackIndex * 0xf + var_i12;
              *(int *)(&DAT_00639577 + var_i8 * 8) = var_d0[2];
              var_i11 = g_CurrentTrackIndex;
              *(int *)(&DAT_00639573 + var_i8 * 8) = var_d0[1];
              _sprintf(&DAT_0063955f + (var_i11 * 0x1e + var_i12) * 4,&g_Format_Str_s);
            }
            var_i12 = var_i12 + -1;
          } while (var_i12 != 0);
        }
        var_pu13 = (int *)(g_Vehicles + 0x3bc + var_i2);
        var_i12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3bc + var_i2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + var_i12) = var_pu13[1];
          var_i11 = 4;
          *(int *)(&DAT_006395cf + var_i12) = *var_pu13;
          _sprintf(&DAT_006395ab + var_i12,&g_Format_Str_s);
          do {
            var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
            if (*(double *)(&DAT_006395af + var_i12 * 8) < *(double *)(&DAT_006395a7 + var_i12 * 8)) {
              var_d0[2] = *(int *)(&DAT_006395ab + var_i12 * 8);
              var_d0[1] = *(int *)(&DAT_006395a7 + var_i12 * 8);
              _sprintf(var_ac,&g_Format_Str_s);
              var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395ab + var_i12 * 8) =
                   *(int *)(&DAT_006395b3 + var_i12 * 8);
              *(int *)(&DAT_006395a7 + var_i12 * 8) =
                   *(int *)(&DAT_006395af + var_i12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + var_i11) * 4,&g_Format_Str_s);
              var_i8 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395b3 + var_i8 * 8) = var_d0[2];
              var_i12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + var_i8 * 8) = var_d0[1];
              _sprintf(&DAT_0063959b + (var_i12 * 0x1e + var_i11) * 4,&g_Format_Str_s);
            }
            var_i11 = var_i11 + -1;
          } while (var_i11 != 0);
        }
        var_pu13 = (int *)(g_Vehicles + 0x3c4 + var_i2);
        var_i12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3c4 + var_i2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + var_i12) = var_pu13[1];
          var_i11 = 4;
          *(int *)(&DAT_006395cf + var_i12) = *var_pu13;
          _sprintf(&DAT_006395ab + var_i12,&g_Format_Str_s);
          do {
            var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
            if (*(double *)(&DAT_006395af + var_i12 * 8) < *(double *)(&DAT_006395a7 + var_i12 * 8)) {
              var_d0[2] = *(int *)(&DAT_006395ab + var_i12 * 8);
              var_d0[1] = *(int *)(&DAT_006395a7 + var_i12 * 8);
              _sprintf(var_ac,&g_Format_Str_s);
              var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395ab + var_i12 * 8) =
                   *(int *)(&DAT_006395b3 + var_i12 * 8);
              *(int *)(&DAT_006395a7 + var_i12 * 8) =
                   *(int *)(&DAT_006395af + var_i12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + var_i11) * 4,&g_Format_Str_s);
              var_i8 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395b3 + var_i8 * 8) = var_d0[2];
              var_i12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + var_i8 * 8) = var_d0[1];
              _sprintf(&DAT_0063959b + (var_i12 * 0x1e + var_i11) * 4,&g_Format_Str_s);
            }
            var_i11 = var_i11 + -1;
          } while (var_i11 != 0);
        }
        var_pu13 = (int *)(g_Vehicles + 0x3cc + var_i2);
        var_i12 = g_CurrentTrackIndex * 0x78;
        if (*(double *)(g_Vehicles + 0x3cc + var_i2) <
            *(double *)(&DAT_006395cf + g_CurrentTrackIndex * 0x78)) {
          *(int *)(&DAT_006395d3 + var_i12) = var_pu13[1];
          var_i11 = 4;
          *(int *)(&DAT_006395cf + var_i12) = *var_pu13;
          _sprintf(&DAT_006395ab + var_i12,&g_Format_Str_s);
          do {
            var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
            if (*(double *)(&DAT_006395af + var_i12 * 8) < *(double *)(&DAT_006395a7 + var_i12 * 8)) {
              var_d0[2] = *(int *)(&DAT_006395ab + var_i12 * 8);
              var_d0[1] = *(int *)(&DAT_006395a7 + var_i12 * 8);
              _sprintf(var_ac,&g_Format_Str_s);
              var_i12 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395ab + var_i12 * 8) =
                   *(int *)(&DAT_006395b3 + var_i12 * 8);
              *(int *)(&DAT_006395a7 + var_i12 * 8) =
                   *(int *)(&DAT_006395af + var_i12 * 8);
              _sprintf(&DAT_00639597 + (g_CurrentTrackIndex * 0x1e + var_i11) * 4,&g_Format_Str_s);
              var_i8 = g_CurrentTrackIndex * 0xf + var_i11;
              *(int *)(&DAT_006395b3 + var_i8 * 8) = var_d0[2];
              var_i12 = g_CurrentTrackIndex;
              *(int *)(&DAT_006395af + var_i8 * 8) = var_d0[1];
              _sprintf(&DAT_0063959b + (var_i12 * 0x1e + var_i11) * 4,&g_Format_Str_s);
            }
            var_i11 = var_i11 + -1;
          } while (var_i11 != 0);
        }
      }
      var_pi3 = var_pi3 + 1;
      var_i2 = var_i2 + 0x484c;
      aiStack_74[1] = aiStack_74[1] + 1;
    } while (aiStack_74[1] < g_NumRacers);
  }
  if (DAT_00552fc0 == 0) {
    aiStack_74[1] = 0x19;
    var_i2 = 0x2d;
  }
  else {
    aiStack_74[1] = 0x28;
    var_i2 = 0x50;
  }
  if (g_PlayerCarChoice < 1) goto LAB_0043c014;
  if ((g_PlayerCarChoice < 6) && (DAT_0054f978 == 1)) {
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,8,0x140,0x25,0x563db0,g_ScreenWidth / 2 + -0xa0,0,g_pLisaDrawCommandWritePtr,0x140,
                 g_ScreenWidth);
    if (DAT_00552fc0 == 0) {
      var_i19 = 0x140;
      var_i12 = g_ScreenHeight + -8;
      var_i11 = g_ScreenWidth / 2 + -0xa0;
      var_i21 = 0x1a4;
      var_i20 = 0x140;
      var_i8 = 0x19c;
    }
    else {
      var_i19 = 0x280;
      var_i12 = g_ScreenHeight + -0x10;
      var_i11 = g_ScreenWidth / 2 + -0x140;
      var_i21 = 0xe3;
      var_i20 = 0x280;
      var_i8 = 0xd3;
    }
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,var_i8,var_i20,var_i21,0x563db0,var_i11,var_i12,g_pLisaDrawCommandWritePtr,var_i19,
                 g_ScreenWidth);
  }
  if ((5 < g_PlayerCarChoice) && (g_PlayerCarChoice < 8)) {
    if (*(int *)(g_Vehicles + 0x3a0) < 4) {
      var_dc = (g_ScreenWidth / 2 + -0x6c) * 0x100;
      var_l17 = __ftol();
      var_d8 = (int)var_l17;
      var_d0[1] = *(int *)(g_Vehicles + 0x5d4);
      _DAT_00563d74 = 0;
      _DAT_00563d70 = (float)var_d0[1] * (float)_DAT_0047a658;
      if (DAT_00563ce0 == 0) {
        Audio_StopSound(8);
        DAT_00563ce0 = 1;
      }
      if ((DAT_00525e70 == 0) || ((DAT_00525e70 == 1 && (g_IsSplitScreen == 1)))) {
        if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
          var_pu22 = &DAT_00563d70;
        }
        else {
          var_pu22 = (char *)0x0;
        }
        var_u6 = ((int*)&(DAT_005287b8))[DAT_00563ce8 * 3];
LAB_0043b1a7:
        Gfx_DrawSprite(var_u6,&var_dc,var_pu22);
      }
      else {
        if ((DAT_00525e74 == 0) || ((DAT_00525e74 == 1 && (g_IsSplitScreen == 1)))) {
          if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
            var_pu22 = &DAT_00563d70;
            var_u6 = ((int*)&(DAT_005287bc))[DAT_00563ce8 * 3];
          }
          else {
            var_pu22 = (char *)0x0;
            var_u6 = ((int*)&(DAT_005287bc))[DAT_00563ce8 * 3];
          }
          goto LAB_0043b1a7;
        }
        if ((DAT_00525e78 == 0) || ((DAT_00525e78 == 1 && (g_IsSplitScreen == 1)))) {
          if (*(int *)(g_Vehicles + 0x5d4) < 0x19) {
            var_pu22 = &DAT_00563d70;
            var_u6 = *(int *)(&DAT_005287c0 + DAT_00563ce8 * 0xc);
          }
          else {
            var_pu22 = (char *)0x0;
            var_u6 = *(int *)(&DAT_005287c0 + DAT_00563ce8 * 0xc);
          }
          goto LAB_0043b1a7;
        }
      }
      _sprintf((char *)var_a4,s_CONGRATULATIONS__004969a8 + g_LanguageId * 0xf5);
      var_l17 = __ftol();
      Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)var_l17);
      _sprintf((char *)var_a4,s_YOU_HAVE_COMPLETED_THE_004969cb + g_LanguageId * 0xf5);
      var_l17 = __ftol();
      Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)var_l17);
      _sprintf((char *)var_a4,s__s_CHAMPIONSHIP_004969ee + g_LanguageId * 0xf5);
      var_l17 = __ftol();
      Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)var_l17);
      if (DAT_00525e70 == 0) {
        var_pc4 = s_AT_FIRST_PLACE_WITH_00496a11 + g_LanguageId * 0xf5;
LAB_0043b3a9:
        _sprintf((char *)var_a4,var_pc4);
      }
      else {
        if (DAT_00525e74 == 0) {
          var_pc4 = s_AT_SECOND_PLACE_WITH_00496a34 + g_LanguageId * 0xf5;
          goto LAB_0043b3a9;
        }
        if (DAT_00525e78 == 0) {
          var_pc4 = s_AT_THIRD_PLACE_WITH_00496a57 + g_LanguageId * 0xf5;
          goto LAB_0043b3a9;
        }
      }
      var_l17 = __ftol();
      Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)var_l17);
      _sprintf((char *)var_a4,s_A_SCORE_OF__d_PTS__00496a7a + g_LanguageId * 0xf5);
      var_l17 = __ftol();
      Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2 + -0x44,(int)var_l17);
      var_i12 = g_LanguageId;
      if (g_IsSplitScreen == 0) {
        if (DAT_00525e70 == 0) {
          if (2 < DAT_00563ce8) goto LAB_0043b5f5;
          _sprintf((char *)var_a4,s_NOW_TRY_THE__s_LEAGUE__00496f68 + g_LanguageId * 0x28);
          ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        }
        else {
          ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
          _sprintf((char *)var_a4,s_YOU_MUST_ACHIEVE_THE_00497058 + var_i12 * 0x46);
          var_l17 = __ftol();
          Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2,(int)var_l17);
          _sprintf((char *)var_a4,s_GOLD_STATUE_TO_ADVANCE__0049707b + g_LanguageId * 0x46);
        }
        var_l17 = __ftol();
        Font_DrawText((char *)var_a4,g_FontId_Menu,g_ScreenWidth / 2,(int)var_l17);
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      }
LAB_0043b5f5:
      if (DAT_00552fc0 == 0) {
        ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,g_FontId_Small,
                     g_ScreenWidth / 2,g_ScreenHeight + -10);
        var_i12 = g_FontId_Small;
      }
      else {
        ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,g_FontId_Menu,
                     g_ScreenWidth / 2,g_ScreenHeight + -0x14);
        var_i12 = g_FontId_Menu;
      }
      var_i11 = 0;
      ((int*)&(g_FontAlignMode))[var_i12 * 400] = 0;
      do {
        if ((0xc < *(int *)((int)&DAT_006192a0 + var_i11)) && (*(int *)(g_Vehicles + 0x5d4) < 0x14)
           ) {
          var_i12 = g_PlayerHUDState[0xb];
          Math_RandomFloat0To1();
          var_d0[1] = (g_PlayerHUDState[0xd] - var_i12) / 2 + var_i12;
          var_l17 = __ftol();
          *(int *)((int)&DAT_00552d90 + var_i11) = (int)var_l17;
          Math_RandomFloat0To1();
          var_d0[1] = (g_PlayerHUDState[0xe] - g_PlayerHUDState[0xc]) / 2;
          var_l17 = __ftol();
          *(int *)((int)&DAT_00552df0 + var_i11) = (int)var_l17;
          if (*(int *)((int)&DAT_006192a0 + var_i11) < 100) {
            *(int *)((int)&DAT_006192a0 + var_i11) = 0;
          }
          else {
            Math_RandomFloat0To1();
            var_l17 = __ftol();
            *(int *)((int)&DAT_006192a0 + var_i11) = (int)var_l17;
          }
        }
        if (*(int *)((int)&DAT_006192a0 + var_i11) < 0xd) {
          _DAT_00563d74 = 0;
          _DAT_00563d70 = 0.5;
          var_dc = *(int *)((int)&DAT_00552d90 + var_i11) << 8;
          var_d8 = *(int *)((int)&DAT_00552df0 + var_i11) << 8;
        }
        var_i11 = var_i11 + 4;
      } while (var_i11 < 0x18);
    }
    else if (((DAT_00552f28 < 1) && (DAT_00563c60 == 0)) && (DAT_00563da0 == 0)) {
      fsin((double)_DAT_00552f68);
      var_dc = (g_ScreenWidth / 2 + -0x4c) * 0x100;
      var_l17 = __ftol();
      var_d8 = (int)var_l17;
      Gfx_DrawSprite(DAT_00528794,&var_dc,0);
    }
    else {
      var_i12 = 0x2580;
      do {
        var_i11 = 0;
        do {
          var_i8 = g_RenderTargetSurface + var_i11;
          var_i11 = var_i11 + 1;
          *(char *)(var_i8 + var_i12) = 0;
        } while (var_i11 < 0x140);
        var_i12 = var_i12 + 0x140;
      } while (var_i12 < 0xaf00);
      Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,g_ScreenHeight,8);
      g_pActiveDrawBuffer = DAT_00525e64;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      _sprintf((char *)var_a4,s_SORRY__YOU_MUST_REACH_THIRD_00496420 + g_LanguageId * 100);
      Font_DrawText((char *)var_a4,g_FontId_Menu,0xa0,0x28);
      _sprintf((char *)var_a4,s_PLACE_OR_BETTER_TO_PROCEED__00496452 + g_LanguageId * 100);
      Font_DrawText((char *)var_a4,g_FontId_Menu,0xa0,0x37);
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      _sprintf(var_50,s_RETRY_00494dd0 + g_LanguageId * 0xd2);
      var_u16 = Font_GetTextWidth((const char *)(var_50), g_FontId_Medium);
      Menu_AddLayoutItem(0x32,0,(int)var_u16,0x50,0);
      _sprintf(var_28,&DAT_00494d94 + g_LanguageId * 0xd2);
      var_u16 = Font_GetTextWidth((const char *)(var_28), g_FontId_Medium);
      Menu_AddLayoutItem(0x32,1,(int)var_u16,0x50,0);
      Menu_LayoutItems(g_RenderTargetSurface,0x140);
      Menu_ClearLayout();
      var_i12 = g_FontId_Large;
      if (DAT_0054f988 == 0) {
        var_i12 = g_FontId_Medium;
      }
      Font_DrawText(var_50,var_i12,0x40,0x5c);
      var_i12 = g_FontId_Large;
      if (DAT_0054f988 == 1) {
        var_i12 = g_FontId_Medium;
      }
      Font_DrawText(var_28,var_i12,0x40,0x6f);
      var_i12 = 0;
      var_u16 = Font_GetTextWidth((const char *)(var_50), g_FontId_Medium);
      if (0 < DAT_00552f28) {
        var_i11 = ((int)var_u16 + 0x5a) * 0x100;
        do {
          var_i12 = var_i12 + 1;
          var_d8 = 0x5800;
          var_dc = var_i11;
          Gfx_DrawSprite(DAT_005287e8,&var_dc,0);
          var_i11 = var_i11 + 0x1900;
        } while (var_i12 < DAT_00552f28);
      }
      g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
      Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
      Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,0x1e,0x13f,0x8c,0x563db0,g_ScreenWidth / 2 + -0xa0,
                   g_ScreenHeight / 2 + -0x46,g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
    }
  }
  if (g_PlayerCarChoice == 8) {
    var_i12 = 0;
    do {
      var_i12 = var_i12 + 1;
      *(char *)(g_RenderTargetSurface + -1 + var_i12) = 0;
    } while (var_i12 < 0x25800);
    Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
    g_pActiveDrawBuffer = DAT_00525e64;
    var_u16 = Font_GetTextWidth((const char *)(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e), g_FontId_Medium);
    Menu_AddLayoutItem(0x93 - (int)var_u16 / 2,0,(int)var_u16,1,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    var_i12 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText(s_TOTAL_SCORE_004955e0 + g_LanguageId * 0x1e,var_i12,0xa0,0xd);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    var_i11 = 0;
    var_i12 = g_NumRacers;
    if (0 < g_NumRacers) {
      var_pi3 = (int *)(g_Vehicles + 0x3a0);
      var_i8 = 0;
      do {
        var_i12 = *var_pi3;
        *(int *)((int)&DAT_00525e70 + var_i8) = var_i11;
        var_i20 = g_NumRacers;
        var_pi3 = var_pi3 + 0x1213;
        var_i11 = var_i11 + 1;
        *(int *)((int)&DAT_00553040 + var_i8) =
             *(int *)(&DAT_00492a24 + var_i12 * 4) + *(int *)((int)&DAT_00563d00 + var_i8);
        var_i8 = var_i8 + 4;
        var_i12 = g_NumRacers;
      } while (var_i11 < var_i20);
    }
    for (; -1 < var_i12; var_i12 = var_i12 + -1) {
      if (1 < g_NumRacers) {
        var_i8 = 4;
        var_i11 = g_NumRacers + -1;
        do {
          var_i20 = *(int *)((int)&DAT_00553040 + var_i8);
          if (*(int *)(var_i8 + 0x55303c) < var_i20) {
            var_u6 = *(int *)((int)&DAT_00525e70 + var_i8);
            *(int *)((int)&DAT_00553040 + var_i8) = *(int *)(var_i8 + 0x55303c);
            var_u1 = *(int *)((int)&DAT_00525e6c + var_i8);
            *(int *)(var_i8 + 0x55303c) = var_i20;
            *(int *)((int)&DAT_00525e70 + var_i8) = var_u1;
            *(int *)((int)&DAT_00525e6c + var_i8) = var_u6;
          }
          var_i8 = var_i8 + 4;
          var_i11 = var_i11 + -1;
        } while (var_i11 != 0);
      }
    }
    var_d0[0] = g_ScreenHeight / 2;
    var_a8 = (int)(g_ScreenHeight + (g_ScreenHeight >> 0x1f & 7U)) >> 3;
    var_u14 = (unsigned int)(*(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x3a0) * 4) + DAT_00563d00 <
                   *(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x4bec) * 4) + DAT_00563d04);
    var_u10 = (unsigned int)(*(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x4bec) * 4) + DAT_00563d04 <=
                   *(int *)(&DAT_00492a24 + *(int *)(g_Vehicles + 0x3a0) * 4) + DAT_00563d00);
    if (DAT_00525e70 == var_u14) {
      var_i12 = 0;
    }
    else if (DAT_00525e74 == var_u14) {
      var_i12 = 1;
    }
    else if (DAT_00525e78 == var_u14) {
      var_i12 = 2;
    }
    else if (DAT_00525e7c == var_u14) {
      var_i12 = 3;
    }
    else if (DAT_00525e80 == var_u14) {
      var_i12 = 4;
    }
    else {
      var_i12 = 5;
      if (DAT_00525e84 != var_u14) {
        var_i12 = var_a4[0];
      }
    }
    var_d0[1] = var_d0[0] - var_a8;
    var_dc = 0x2800;
    _DAT_00563d70 = 0.5;
    var_d8 = (var_d0[1] + -0xd) * 0x100;
    if (DAT_00552fc0 != 0) {
      _DAT_00563d70 = 1.0;
    }
    _DAT_00563d74 = 0;
    if (var_i12 < 3) {
      Gfx_DrawSprite(((int*)&(DAT_005287b8))[DAT_00563ce8 * 3 + var_i12],&var_dc,&DAT_00563d70);
    }
    var_dc = 0x4600;
    var_d8 = (var_d0[1] + -0xf) * 0x100;
    Gfx_DrawSprite(((int*)&(DAT_005286f4))[var_i12],&var_dc,0);
    Menu_AddLayoutItem(100,0,0x2d,var_d0[1] + -0x17,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    _sprintf((char *)var_a4,&g_Format_Str_s);
    var_i11 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)var_a4,var_i11,0x87,var_d0[1] + -0xb);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    _sprintf((char *)var_a4,s__d_PTS_00496948 + g_LanguageId * 0xf);
    var_u16 = Font_GetTextWidth((const char *)var_a4, g_FontId_Medium);
    Menu_AddLayoutItem(0xaf,0,(int)var_u16,var_d0[1] + -0x17,0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    var_i11 = g_FontId_Medium;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)var_a4,var_i11,(int)var_u16 / 2 + 0xbc,var_d0[1] + -0xb);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    if (DAT_00525e70 == var_u10) {
      var_i12 = 0;
    }
    else if (DAT_00525e74 == var_u10) {
      var_i12 = 1;
    }
    else if (DAT_00525e78 == var_u10) {
      var_i12 = 2;
    }
    else if (DAT_00525e7c == var_u10) {
      var_i12 = 3;
    }
    else if (DAT_00525e80 == var_u10) {
      var_i12 = 4;
    }
    else if (DAT_00525e84 == var_u10) {
      var_i12 = 5;
    }
    var_dc = 0x2800;
    _DAT_00563d70 = 0.5;
    var_d8 = (var_a8 + 0x19 + var_d0[0]) * 0x100;
    if (DAT_00552fc0 != 0) {
      _DAT_00563d70 = 1.0;
    }
    _DAT_00563d74 = 0;
    if (var_i12 < 3) {
      Gfx_DrawSprite(((int*)&(DAT_005287b8))[DAT_00563ce8 * 3 + var_i12],&var_dc,&DAT_00563d70);
    }
    var_dc = 0x4600;
    var_d8 = (var_a8 + 0xf + var_d0[0]) * 0x100;
    Gfx_DrawSprite(((int*)&(DAT_005286f4))[var_i12],&var_dc,0);
    var_d0[1] = var_a8 + 7 + var_d0[0];
    Menu_AddLayoutItem(100,0,0x2d,var_d0[1],0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    _sprintf((char *)var_a4,&g_Format_Str_s);
    var_i12 = var_a8 + 0x13 + var_d0[0];
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)var_a4,g_FontId_Medium,0x87,var_i12);
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    _sprintf((char *)var_a4,s__d_PTS_00496948 + g_LanguageId * 0xf);
    var_u16 = Font_GetTextWidth((const char *)var_a4, g_FontId_Medium);
    Menu_AddLayoutItem(0xaf,0,(int)var_u16,var_d0[1],0);
    Menu_LayoutItems(g_RenderTargetSurface,0x140);
    Menu_ClearLayout();
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 1;
    Font_DrawText((char *)var_a4,g_FontId_Medium,(int)var_u16 / 2 + 0xbc,var_i12);
    var_i11 = g_ScreenHeight;
    var_i12 = g_ScreenWidth;
    ((int*)&(g_FontAlignMode))[g_FontId_Medium * 400] = 0;
    Gfx_SetRenderTarget(&g_VirtualFramebuffer,var_i12,var_i12,var_i11,8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
    Gfx_BlitTransparentLUT(g_RenderTargetSurface,0,0,0x140,g_ScreenHeight,0x563db0,g_ScreenWidth / 2 + -0xa0,0,
                 g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
    var_i11 = g_LanguageId;
    var_i12 = g_FontId_Small;
    if (DAT_00552fc0 == 0) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i11 * 0x32,var_i12,0xa0,
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
  var_i12 = g_NumRacers;
  if ((DAT_00553068 == 1) &&
     (var_d0[1] = DAT_00525e44 / 100, (double)var_d0[1] <= _DAT_00553090)) {
    var_i12 = g_NumRacers + 1;
  }
  var_u10 = 0;
  if (0 < var_i12) {
    var_i11 = 0x57;
    var_i2 = var_i2 + -0x1a;
    do {
      var_i8 = 0;
      if (0 < g_NumRacers) {
        var_pi3 = (int *)(g_Vehicles + 0x3a0);
        do {
          if (*var_pi3 - var_u10 == 1) break;
          var_pi3 = var_pi3 + 0x1213;
          var_i8 = var_i8 + 1;
        } while (var_i8 < g_NumRacers);
      }
      if (DAT_00553068 == 1) {
        var_b15 = 0;
        if (g_NumRacers == var_i8) {
          var_i8 = 0;
          goto LAB_0043c0a0;
        }
      }
      else {
LAB_0043c0a0:
        var_b15 = g_NumRacers == var_i8;
      }
      if (((var_b15 || g_NumRacers < var_i8) || (g_PlayerCarChoice < 1)) ||
         ((5 < g_PlayerCarChoice || (DAT_0054f978 != 1)))) goto LAB_0043c34d;
      var_i21 = DAT_005db038;
      var_i20 = var_i11;
      var_i8 = var_i2;
      if (g_GameMode == 3) {
        if (g_RacePosition == var_u10) {
          var_i7 = g_ScreenWidth / 2 + -0xa0;
          var_i19 = var_i11 + -0x36;
          var_i18 = 0;
        }
        else {
          if ((int)var_u10 <= (int)g_RacePosition) goto LAB_0043c34d;
          if ((var_u10 & 1) == 0) {
            var_i8 = g_ScreenWidth / 2 + -0x69;
            var_i21 = 0x73;
            var_i20 = 0x37;
          }
          else {
            var_i8 = g_ScreenWidth / 2 + 0x2d;
            var_i21 = 0x109;
            var_i20 = 0xcd;
          }
          Gfx_BlitTransparentLUT(g_RenderTargetSurface,var_i20,var_i11 + -0x36,var_i21,var_i11,0x563db0,var_i8,var_i2,
                       g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
          var_i8 = var_i2 + 0xf;
          var_i20 = var_i11 + -0xf;
          var_i19 = var_i11 + -0x27;
          var_i7 = g_ScreenWidth / 2 + -0x37;
          var_i21 = 0xd7;
          var_i18 = 0x69;
        }
LAB_0043c33f:
        Gfx_BlitTransparentLUT(g_RenderTargetSurface,var_i18,var_i19,var_i21,var_i20,0x563db0,var_i7,var_i8,g_pLisaDrawCommandWritePtr,
                     0x140,g_ScreenWidth);
      }
      else {
        if (g_RacePosition == var_u10) {
          var_i7 = g_ScreenWidth / 2 + -0xa0;
          var_i19 = var_i11 + -0x36;
          var_i18 = 0;
          goto LAB_0043c33f;
        }
        if ((int)var_u10 < (int)g_RacePosition) {
          if (g_PlayerCarChoice == 1) {
            if ((var_u10 & 1) == 0) {
              var_i8 = g_ScreenWidth / 2 + -0xa0;
              var_i21 = 0x32;
              var_i20 = 0;
            }
            else {
              var_i8 = g_ScreenWidth / 2 + 0x6e;
              var_i21 = 0x140;
              var_i20 = 0x10e;
            }
            Gfx_BlitTransparentLUT(g_RenderTargetSurface,var_i20,var_i11 + -0x36,var_i21,var_i11,0x563db0,var_i8,var_i2,
                         g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
            var_i19 = var_i11 + -0x27;
            var_i7 = g_ScreenWidth / 2 + -0x6e;
            var_i18 = 0x32;
            var_i21 = 0x10e;
            var_i20 = var_i11 + -0xf;
            var_i8 = var_i2 + 0xf;
          }
          else {
            if ((g_PlayerCarChoice != 3) && (g_PlayerCarChoice != 5)) goto LAB_0043c34d;
            if ((var_u10 & 1) == 0) {
              var_i8 = g_ScreenWidth / 2 + -0x82;
              var_i21 = 0x50;
              var_i20 = 0x1e;
            }
            else {
              var_i8 = g_ScreenWidth / 2 + 0x4d;
              var_i21 = 0x122;
              var_i20 = 0xed;
            }
            Gfx_BlitTransparentLUT(g_RenderTargetSurface,var_i20,var_i11 + -0x36,var_i21,var_i11,0x563db0,var_i8,var_i2,
                         g_pLisaDrawCommandWritePtr,0x140,g_ScreenWidth);
            var_i19 = var_i11 + -0x27;
            var_i7 = g_ScreenWidth / 2 + -0x50;
            var_i18 = 0x50;
            var_i21 = 0xed;
            var_i20 = var_i11 + -0xf;
            var_i8 = var_i2 + 0xf;
          }
          goto LAB_0043c33f;
        }
      }
LAB_0043c34d:
      var_i11 = var_i11 + 0x3c;
      var_i2 = var_i2 + aiStack_74[1];
      var_u10 = var_u10 + 1;
    } while ((int)var_u10 < var_i12);
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
  int var_u1;
  int *var_pi2;
  int var_i3;
  int var_i4;
  int var_i5;
  int *var_pu6;
  int var_i7;
  int var_i8;
  int var_i9;
  int var_2c;
  int var_28;
  int aiStack_20 [8];
  var_i4 = 0;
  if (0 < g_LapsTotal) {
    do {
      var_i3 = 0;
      if (0 < g_NumRacers) {
        var_pi2 = (int *)(g_Vehicles + 0x3a0);
        do {
          if (*var_pi2 - var_i4 == 1) break;
          var_pi2 = var_pi2 + 0x1213;
          var_i3 = var_i3 + 1;
        } while (var_i3 < g_NumRacers);
        if (var_i3 < g_NumRacers) {
          aiStack_20[var_i4] = var_i3;
        }
      }
      var_i4 = var_i4 + 1;
    } while (var_i4 < g_LapsTotal);
  }
  Font_GetTextWidth((const char *)(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e), g_FontId_Medium);
  Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
  var_i4 = g_FontId_Small;
  g_pActiveDrawBuffer = DAT_00525e64;
  if (DAT_00552f10 == 4) {
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,var_i4,0xa0,0x19c);
    var_u1 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(var_u1,0x280,0x280,0xf0,8);
    var_i4 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e,var_i4,0x140,0xd3);
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
  }
  else {
    if (g_GameMode != 0) {
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d,var_i4,0xa0,0x19c);
      var_u1 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
      Gfx_SetRenderTarget(var_u1,0x280,0x280,0xf0,8);
      var_i3 = g_LanguageId;
      var_i4 = g_FontId_Menu;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
      Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + var_i3 * 0x2d,var_i4,0x140,0xd3);
      var_u1 = g_RenderTargetSurface;
      ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
      Gfx_SetRenderTarget(var_u1,0x140,0x140,0x1e0,8);
      DAT_005285c8 = 1;
      goto LAB_0043c75a;
    }
    Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32), g_FontId_Small);
    var_i4 = g_FontId_Small;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32,var_i4,0xa0,0x19c);
    var_u1 = g_RenderTargetSurface;
    ((int*)&(g_FontAlignMode))[g_FontId_Small * 400] = 0;
    Gfx_SetRenderTarget(var_u1,0x280,0x280,0xf0,8);
    var_i3 = g_LanguageId;
    var_i4 = g_FontId_Menu;
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 1;
    Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + var_i3 * 0x32,var_i4,0x140,0xd3);
    ((int*)&(g_FontAlignMode))[g_FontId_Menu * 400] = 0;
  }
  Gfx_SetRenderTarget(g_RenderTargetSurface,0x140,0x140,0x1e0,8);
LAB_0043c75a:
  var_i4 = 0;
  if (0 < g_NumRacers) {
    var_i3 = 0x28;
    var_pu6 = &DAT_005286f4;
    var_i5 = 0x3000;
    do {
      var_28 = var_i5;
      if (g_GameMode == 3) {
        var_2c = 0x7000;
        Gfx_DrawSprite(*var_pu6,&var_2c,0);
        var_i8 = 0x28;
        var_i7 = 0x8b;
        var_i9 = var_i3;
LAB_0043c897:
        Menu_AddLayoutItem(var_i7,0,var_i8,var_i9,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
      }
      else {
        var_2c = 0x3400;
        Gfx_DrawSprite(*var_pu6,&var_2c,0);
        Menu_AddLayoutItem(0x4f,0,0x28,var_i3,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        Menu_AddLayoutItem(0x92,0,0x5f,var_i3,0);
        Menu_LayoutItems(g_RenderTargetSurface,0x140);
        Menu_ClearLayout();
        if (DAT_00553068 == 1) {
          var_28 = var_i5 + 0x3c00;
          var_2c = 0x3400;
          Gfx_DrawSprite(var_pu6[1],&var_2c,0);
          Menu_AddLayoutItem(0x4f,0,0x28,var_i3 + 0x3c,0);
          Menu_LayoutItems(g_RenderTargetSurface,0x140);
          Menu_ClearLayout();
          var_i8 = 0x5f;
          var_i7 = 0x92;
          var_i9 = var_i3 + 0x3c;
          goto LAB_0043c897;
        }
      }
      var_i3 = var_i3 + 0x3c;
      var_pu6 = var_pu6 + 1;
      var_i5 = var_i5 + 0x3c00;
      var_i4 = var_i4 + 1;
    } while (var_i4 < g_NumRacers);
  }
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
  return;
}

/**
 * @original Camera_UpdateChase (IGN_WIN.EXE @ 0x0043c910, fx.c)
 * @fidelity ADAPTED
 */
void Camera_UpdateChase(void) {
  int var_i1;
  unsigned int var_u2;
  double var_d3;
  int var_i4;
  double *var_pd5;
  int var_i6;
  unsigned int var_u7;
  double var_f8;
  __int64 var_l9;
  __int64 var_20;
  unsigned int uStack_14;
  unsigned int uStack_c;
  double var_8;
  if (0.0 <= g_RaceTimer_P2) {
    uStack_c = 0x40390000;
  }
  else {
    uStack_c = 0x40240000;
  }
  var_pd5 = (double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
  if (*(int *)(g_Vehicles + 0x528 + g_ActiveVehicleIndex * 0x484c) == 0) {
    var_f8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x78) =
         (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 + (double)*var_pd5);
    var_i4 = g_Vehicles;
    var_i6 = g_VehicleConfigs;
    var_i1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0x84 + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(var_i6 + 0x80 + var_i1 * 200) = *(int *)(var_i4 + 8 + var_i1 * 0x484c);
    var_f8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x88) =
         (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 +
                 (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
    var_i4 = g_Vehicles;
    var_i6 = g_VehicleConfigs;
    var_i1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0xac + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(var_i6 + 0xa8 + var_i1 * 200) = *(int *)(var_i4 + 0xf8 + var_i1 * 0x484c);
    var_i4 = g_VehicleConfigs;
    var_i6 = g_ActiveVehicleIndex;
    var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
      *(int *)(g_VehicleConfigs + 0xb4 + g_ActiveVehicleIndex * 200) = *(int *)(var_i1 + 0x114);
      *(int *)(var_i4 + 0xb0 + var_i6 * 200) = *(int *)(var_i1 + 0x110);
    }
  }
  else {
    if (*(int *)((int)var_pd5 + 0x604) != 1) goto LAB_0043cd2c;
    var_f8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x78) =
         (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 + (double)*var_pd5);
    var_i4 = g_Vehicles;
    var_i6 = g_VehicleConfigs;
    var_i1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0x84 + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(var_i6 + 0x80 + var_i1 * 200) = *(int *)(var_i4 + 8 + var_i1 * 0x484c);
    var_f8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
    *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x88) =
         (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                  (double)_DAT_0047a6e0 +
                 (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
    var_i4 = g_Vehicles;
    var_i6 = g_VehicleConfigs;
    var_i1 = g_ActiveVehicleIndex;
    *(int *)(g_VehicleConfigs + 0xac + g_ActiveVehicleIndex * 200) =
         *(int *)(g_Vehicles + 0xfc + g_ActiveVehicleIndex * 0x484c);
    *(int *)(var_i6 + 0xa8 + var_i1 * 200) = *(int *)(var_i4 + 0xf8 + var_i1 * 0x484c);
    var_i4 = g_VehicleConfigs;
    var_i6 = g_ActiveVehicleIndex;
    var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x554 + g_ActiveVehicleIndex * 0x484c) == 0) {
      *(int *)(g_VehicleConfigs + 0xb4 + g_ActiveVehicleIndex * 200) = *(int *)(var_i1 + 0x114);
      *(int *)(var_i4 + 0xb0 + var_i6 * 200) = *(int *)(var_i1 + 0x110);
    }
  }
  var_f8 = (double)fcos((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x90) =
       (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                (double)_DAT_0047a6e8 + (double)*(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c))
  ;
  var_i4 = g_Vehicles;
  var_i6 = g_VehicleConfigs;
  var_i1 = g_ActiveVehicleIndex;
  *(int *)(g_VehicleConfigs + 0x9c + g_ActiveVehicleIndex * 200) =
       *(int *)(g_Vehicles + 0xc + g_ActiveVehicleIndex * 0x484c);
  *(int *)(var_i6 + 0x98 + var_i1 * 200) = *(int *)(var_i4 + 8 + var_i1 * 0x484c);
  var_f8 = (double)fsin((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200));
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0xa0) =
       (double)(var_f8 * (double)*(int *)(g_VehicleConfigs + 0x54 + g_ActiveVehicleIndex * 200) *
                (double)_DAT_0047a6e8 +
               (double)*(double *)(g_Vehicles + 0x10 + g_ActiveVehicleIndex * 0x484c));
  *(double *)(g_VehicleConfigs + 0xb8 + g_ActiveVehicleIndex * 200) =
       *(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) + _DAT_0047a6f0;
LAB_0043cd2c:
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  if (((*(int *)(g_Vehicles + 0x354 + g_ActiveVehicleIndex * 0x484c) == 0) &&
      (*(int *)(var_i1 + 0x358) == 0)) && (*(int *)(var_i1 + 0x35c) == 0)) {
    var_d3 = *(double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200) * _DAT_0047a6f8;
    var_pd5 = (double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200);
  }
  else {
    var_pd5 = (double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200);
    var_d3 = (_DAT_0047a700 - *(double *)(g_VehicleConfigs + 0x30 + g_ActiveVehicleIndex * 200)) * _DAT_0047a708
            + *var_pd5;
  }
  *var_pd5 = var_d3;
  var_20 = *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) -
             *(double *)(g_VehicleConfigs + 0xa8 + g_ActiveVehicleIndex * 200);
  var_i1 = g_VehicleConfigs + g_ActiveVehicleIndex * 200;
  if (_DAT_0047a6f0 < var_20) {
    var_20 = var_20 - _DAT_0047a710;
  }
  if (var_20 < _DAT_0047a718) {
    var_20 = var_20 + _DAT_0047a710;
  }
  *(double *)(var_i1 + 8) = var_20 * _DAT_0047a720 + *(double *)(var_i1 + 8);
  var_pd5 = (double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a710 <= *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200)) {
    *var_pd5 = *var_pd5 - _DAT_0047a710;
  }
  var_pd5 = (double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) <= 0.0) {
    *var_pd5 = *var_pd5 + _DAT_0047a710;
  }
  *(double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200 + 0x68) =
       *(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) * _DAT_0047a728 + _DAT_0047a730;
  var_pd5 = (double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200) < 0.0) {
    *var_pd5 = *var_pd5 + _DAT_0047a738;
  }
  var_pd5 = (double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a740 < *(double *)(g_VehicleConfigs + 0x68 + g_ActiveVehicleIndex * 200)) {
    *var_pd5 = *var_pd5 - _DAT_0047a738;
  }
  var_20 = *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) -
             *(double *)(g_VehicleConfigs + 0xb8 + g_ActiveVehicleIndex * 200);
  var_i1 = g_VehicleConfigs + g_ActiveVehicleIndex * 200;
  if (_DAT_0047a6f0 < var_20) {
    var_20 = var_20 - _DAT_0047a710;
  }
  if (var_20 < _DAT_0047a718) {
    var_20 = var_20 + _DAT_0047a710;
  }
  *(double *)(var_i1 + 0x28) = var_20 * _DAT_0047a720 + *(double *)(var_i1 + 0x28);
  var_pd5 = (double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a710 <= *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200)) {
    *var_pd5 = *var_pd5 - _DAT_0047a710;
  }
  var_pd5 = (double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) <= 0.0) {
    *var_pd5 = *var_pd5 + _DAT_0047a710;
  }
  *(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200) =
       *(double *)(g_VehicleConfigs + 0x28 + g_ActiveVehicleIndex * 200) * _DAT_0047a728 + _DAT_0047a730;
  var_pd5 = (double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200) < 0.0) {
    *var_pd5 = *var_pd5 + _DAT_0047a738;
  }
  var_pd5 = (double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200);
  if (_DAT_0047a740 < *(double *)(g_VehicleConfigs + 0x70 + g_ActiveVehicleIndex * 200)) {
    *var_pd5 = *var_pd5 - _DAT_0047a738;
  }
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  var_u2 = *(unsigned int *)(var_i1 + 0x364);
  var_u7 = (int)var_u2 >> 0x1f;
  if (*(int *)(g_Vehicles + 0x36c + g_ActiveVehicleIndex * 0x484c) == 3) {
    var_i6 = (var_u2 ^ var_u7) - var_u7;
    var_20 = (*(double *)(g_pTrackRoadSequence + 4 + var_i6 * 0x18) +
               *(double *)(g_pTrackRoadSequence + 0x10 + var_i6 * 0x18)) * _DAT_0047a6e8;
  }
  else if ((int)var_u2 < 0) {
    var_i6 = (var_u2 ^ var_u7) - var_u7;
    var_20 = (double)CONCAT44(*(int *)(g_pTrackRoadSequence + 0x14 + var_i6 * 0x18),
                                *(int *)(g_pTrackRoadSequence + 0x10 + var_i6 * 0x18));
  }
  else {
    var_20 = (double)CONCAT44(*(int *)(g_pTrackRoadSequence + 8 + var_u2 * 0x18),
                                *(int *)(g_pTrackRoadSequence + 4 + var_u2 * 0x18));
  }
  if (*(int *)(var_i1 + 0x528) == 0) {
    var_f8 = (double)fcos((((double)*(double *)(g_VehicleConfigs + 8 + g_ActiveVehicleIndex * 200) -
                           (double)var_20) - (double)_DAT_0047a748) * (double)_DAT_0047a750 +
                          (double)_DAT_0047a6f0);
    var_8 = (double)((var_f8 + (double)_DAT_0047a758) * (double)_DAT_0047a760);
  }
  else {
    var_8 = 0.0;
  }
  if (*(int *)(var_i1 + 0x35c) == 0) {
    var_20 = *(double *)(g_VehicleConfigs + 0xb0 + g_ActiveVehicleIndex * 200) * _DAT_0047a768;
  }
  else {
    var_20 = 0.0;
  }
  var_pd5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  if ((_DAT_0047a738 < *(double *)(g_VehicleConfigs + 0x48 + g_ActiveVehicleIndex * 200)) ||
     (var_pd5[9] < _DAT_0047a770)) {
    _DAT_004949e0 = 0;
  }
  var_pd5[9] = ((((*var_pd5 - (double)*(int *)(var_pd5 + 10)) - var_pd5[0x10]) * _DAT_0047a6e8 +
                var_8 * _DAT_0047a778 + var_20) - var_pd5[9]) /
              (double)((unsigned __int64)uStack_c << 0x20) + var_pd5[9];
  var_i1 = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
  uStack_14 = 0x40240000;
  if (0.0 <= g_RaceTimer_P2) {
    uStack_14 = uStack_c;
  }
  var_u2 = *(unsigned int *)(var_i1 + 0x364);
  if ((int)var_u2 < 0) {
    var_i6 = *(int *)(g_pTrackRoadSequence + 0xc +
                    ((var_u2 ^ (int)var_u2 >> 0x1f) - ((int)var_u2 >> 0x1f)) * 0x18);
  }
  else {
    var_i6 = *(int *)(g_pTrackRoadSequence + var_u2 * 0x18);
  }
  var_i6 = *(int *)(DAT_00552e40 + 4 + var_i6 * 0xc);
  if (var_i6 < 1) {
    var_20 = 0.0;
  }
  else if (var_i6 < 0xb) {
    var_20 = (double)(var_i6 * -0x32);
  }
  else if ((10 < var_i6) && (var_i6 < 0x15)) {
    var_20 = (double)((var_i6 * 5 + -0x32) * 10);
  }
  var_pd5 = (double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200);
  if (*(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) < var_20) {
    *var_pd5 = *(double *)(var_i1 + 0x118) * _DAT_0047a780 + *var_pd5;
  }
  var_pd5 = (double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200);
  if (var_20 < *(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200)) {
    *var_pd5 = *(double *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c) * _DAT_0047a788 + *var_pd5;
  }
  var_pd5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  *var_pd5 = ((double)*(int *)(g_VehicleConfigs + 0x50 + g_ActiveVehicleIndex * 200) -
            ((var_8 * _DAT_0047a790 - *(double *)(g_VehicleConfigs + 0x80 + g_ActiveVehicleIndex * 200)) +
             *(double *)(g_VehicleConfigs + 0xc0 + g_ActiveVehicleIndex * 200) + *var_pd5)) /
            (double)((unsigned __int64)uStack_14 << 0x20) + *var_pd5;
  var_d3 = *(double *)(g_VehicleConfigs + 0x80 + g_ActiveVehicleIndex * 200) + _DAT_0047a798;
  var_pd5 = (double *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
  if (*var_pd5 < var_d3) {
    *var_pd5 = var_d3;
  }
  var_i1 = g_VehicleConfigs + 0x54;
  var_i6 = g_ActiveVehicleIndex * 200;
  var_l9 = __ftol();
  *(int *)(var_i1 + var_i6) = (int)var_l9;
  return;
}

/**
 * @original Car_UpdateDynamicObjects (IGN_WIN.EXE @ 0x0043d540, fx.c)
 * @fidelity ADAPTED
 */
void Car_UpdateDynamicObjects(int car_idx) {
  int var_i1;
  int var_i2;
  int var_i3;
  int *var_pu4;
  int var_i5;
  int *var_pi6;
  double var_f7;
  __int64 var_l8;
  int *var_20;
  int var_c;
  var_i3 = car_idx * 0x20;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 4 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 8 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 0xc + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 0x10 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 0x14 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarTransforms + 0x18 + var_i3) = (int)var_l8;
  if ((g_DynamicObjectsPaused == 0) && (var_i1 = Lisa_UpdateObjectSpatialGrid((int *)(g_pCarTransforms + var_i3)), var_i1 != 0)) {
    Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
  }
  var_i1 = g_Vehicles + car_idx * 0x484c;
  var_f7 = (double)*(double *)(var_i1 + 0xb0) * (double)_DAT_0047a7e0;
  var_i5 = 0;
  fcos(var_f7);
  fsin(var_f7);
  if (*(int *)(var_i1 + 0x558) == 0) {
    var_pi6 = (int *)(g_PlayerHUDState + car_idx * 0x4c);
    if (0 < *(int *)(*var_pi6 * 0x30 + g_ParticleCount1)) {
      var_pu4 = &g_ParticleArray1 + car_idx * 0x820;
      do {
        var_i5 = var_i5 + 1;
        var_l8 = __ftol();
        *var_pu4 = (int)var_l8;
        var_l8 = __ftol();
        var_i1 = g_ParticleCount1;
        var_pu4[2] = (int)var_l8;
        var_pu4 = var_pu4 + 3;
      } while (var_i5 < *(int *)(*var_pi6 * 0x30 + var_i1));
    }
    var_i1 = 0;
    if (0 < *(int *)(*var_pi6 * 0x30 + 0xc + g_ParticleCount1)) {
      var_pu4 = (int *)(&g_ParticleArray2 + car_idx * 0x2080);
      do {
        var_i1 = var_i1 + 1;
        var_l8 = __ftol();
        *var_pu4 = (int)var_l8;
        var_l8 = __ftol();
        var_i5 = g_ParticleCount1;
        var_pu4[2] = (int)var_l8;
        var_pu4 = var_pu4 + 3;
      } while (var_i1 < *(int *)(*var_pi6 * 0x30 + 0xc + var_i5));
    }
  }
  else {
    var_i1 = 0;
    var_20 = &g_ParticleArray1 + car_idx * 0x820;
    var_pi6 = (int *)(g_PlayerHUDState + car_idx * 0x4c);
    do {
      var_i5 = 0;
      var_pu4 = var_20;
      if (0 < *(int *)(g_ParticleCount1 + (var_i1 + *var_pi6 * 4) * 0xc)) {
        do {
          var_l8 = __ftol();
          *var_pu4 = (int)var_l8;
          var_l8 = __ftol();
          var_pu4[1] = (int)var_l8;
          var_l8 = __ftol();
          var_pu4[2] = (int)var_l8;
          var_i5 = var_i5 + 1;
          var_pu4 = var_pu4 + 3;
        } while (var_i5 < *(int *)(g_ParticleCount1 + (var_i1 + *var_pi6 * 4) * 0xc));
      }
      var_i1 = var_i1 + 1;
      var_20 = var_20 + 0x208;
    } while (var_i1 < 4);
  }
  var_c = car_idx * 0x4c;
  var_i1 = 4;
  var_i5 = car_idx << 7;
  do {
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 4 + var_i5) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 8 + var_i5) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 0xc + var_i5) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 0x10 + var_i5) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 0x14 + var_i5) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pWheelTransforms + 0x18 + var_i5) = (int)var_l8;
    var_i2 = Lisa_UpdateObjectSpatialGrid((int *)(g_pWheelTransforms + var_i5));
    if (var_i2 != 0) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_WHE_00499580);
    }
    var_i5 = var_i5 + 0x20;
    var_i1 = var_i1 + -1;
  } while (var_i1 != 0);
  if ((*(int *)(g_PlayerHUDState + var_c) == 7) && (g_DynamicObjectsPaused == 0)) {
    var_l8 = __ftol();
    *(int *)(var_i3 + 4 + g_pCarReflectionTransforms) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(var_i3 + 8 + g_pCarReflectionTransforms) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(var_i3 + 0xc + g_pCarReflectionTransforms) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(var_i3 + 0x10 + g_pCarReflectionTransforms) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(var_i3 + 0x14 + g_pCarReflectionTransforms) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(var_i3 + 0x18 + g_pCarReflectionTransforms) = (int)var_l8;
    if ((g_DynamicObjectsPaused == 0) && (var_i1 = Lisa_UpdateObjectSpatialGrid((int *)(g_pCarReflectionTransforms + var_i3)), var_i1 != 0)) {
      Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
    }
  }
  var_l8 = __ftol();
  ((int*)&(DAT_00603718))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_00603720))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_00603724))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_0060372c))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_00603730))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_00603738))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_0060373c))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  ((int*)&(DAT_00603744))[car_idx * 0x1e] = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 4 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 8 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 0xc + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 0x10 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 0x14 + var_i3) = (int)var_l8;
  var_l8 = __ftol();
  *(int *)(g_pCarShadowTransforms + 0x18 + var_i3) = (int)var_l8;
  if ((g_DynamicObjectsPaused == 0) && (var_i1 = Lisa_UpdateObjectSpatialGrid((int *)(g_pCarShadowTransforms + var_i3)), var_i1 != 0)) {
    Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_SHA_00499558);
  }
  if ((*(int *)(g_PlayerHUDState + 4 + var_c) == 2) && (car_idx != 0)) {
    var_l8 = __ftol();
    *(int *)(g_pCarGhostTransforms + 4 + var_i3) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pCarGhostTransforms + 8 + var_i3) = (int)var_l8;
    var_l8 = __ftol();
    *(int *)(g_pCarGhostTransforms + 0xc + var_i3) = (int)var_l8;
    *(int *)(g_pCarGhostTransforms + 0x10 + var_i3) = 0;
    *(int *)(g_pCarGhostTransforms + 0x14 + var_i3) = 0;
    *(int *)(g_pCarGhostTransforms + 0x18 + var_i3) = 0;
    var_pi6 = (int *)(g_pCarGhostTransforms + var_i3);
    if (var_pi6[7] == 1) {
      var_i3 = Lisa_UpdateObjectSpatialGrid(var_pi6);
      if (var_i3 != 0) {
        Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_00499530);
        return;
      }
    }
    else {
      var_i3 = Lisa_MoveDynamicObject(var_pi6);
      if (var_i3 != 0) {
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
  int var_i1;
  int var_i2;
  int var_i3;
  int out_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  unsigned int var_u4;
  unsigned int var_u5;
  int *var_pu6;
  __int64 var_u7;
  if (DAT_00552fc0 == 0) {
    g_ScreenWidth = 0x140;
    g_ScreenHeight = 200;
    g_ColorDepth = 8;
    DAT_004ba6ec = 1;
    var_i2 = Gfx_RestoreSurface();
    if (var_i2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    var_i2 = 0;
    g_ScreenHeight = 200;
    g_ScreenWidth = 0x140;
    if (0 < g_NumRacers) {
      var_i3 = 0;
      do {
        var_i2 = var_i2 + 1;
        *(int *)(g_VehicleConfigs + 0x58 + var_i3) = 200;
        *(int *)(g_VehicleConfigs + 0x5c + var_i3) = 0xa4;
        var_i1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + 0x60 + var_i3) = 0;
        *(int *)(var_i1 + 100 + var_i3) = 0x40390000;
        var_i3 = var_i3 + 200;
      } while (var_i2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if ((((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && (DAT_0054f98c == 1)) && (g_HudEnabled == 1)
       ) {
      var_u7 = Palette_AdjustRGB(out_ECX,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)var_u7);
    }
  }
  if (DAT_00552fc0 == 1) {
    g_ScreenWidth = 0x280;
    g_ScreenHeight = 0x1e0;
    g_ColorDepth = 8;
    DAT_004ba6ec = 1;
    var_i2 = Gfx_RestoreSurface();
    if (var_i2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    var_i2 = 0;
    g_ScreenWidth = 0x280;
    g_ScreenHeight = 0x1e0;
    if (0 < g_NumRacers) {
      var_i3 = 0;
      do {
        var_i3 = var_i3 + 200;
        *(int *)(g_VehicleConfigs + -0x70 + var_i3) = 0x1a9;
        var_i2 = var_i2 + 1;
        *(int *)(g_VehicleConfigs + -0x6c + var_i3) = 0x1a9;
        var_i1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + -0x68 + var_i3) = 0;
        *(int *)(var_i1 + -100 + var_i3) = 0x40240000;
      } while (var_i2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if (((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && ((DAT_0054f98c == 1 && (g_HudEnabled == 1)))
       ) {
      var_u7 = Palette_AdjustRGB(extraout_ECX_00,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)var_u7);
    }
  }
  if (DAT_00552fc0 == 2) {
    g_ScreenWidth = 800;
    g_ScreenHeight = 600;
    g_ColorDepth = 8;
    DAT_004ba6ec = 1;
    var_i2 = Gfx_RestoreSurface();
    if (var_i2 == 0) {
      _sprintf(&DAT_006035f0,s_Cannot_use_this_graphics_mode__004989dc);
      _DAT_0054f970 = 1;
      return 0;
    }
    var_i2 = 0;
    g_ScreenWidth = 800;
    g_ScreenHeight = 600;
    if (0 < g_NumRacers) {
      var_i3 = 0;
      do {
        var_i3 = var_i3 + 200;
        *(int *)(g_VehicleConfigs + -0x70 + var_i3) = 0x226;
        var_i2 = var_i2 + 1;
        *(int *)(g_VehicleConfigs + -0x6c + var_i3) = 0x226;
        var_i1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + -0x68 + var_i3) = 0;
        *(int *)(var_i1 + -100 + var_i3) = 0x40240000;
      } while (var_i2 < g_NumRacers);
    }
    Lisa_ResetRasterizerContext();
    if (((DAT_00552f30 == 1) && (g_IsSplitScreen == 0)) && ((DAT_0054f98c == 1 && (g_HudEnabled == 1)))
       ) {
      var_u7 = Palette_AdjustRGB(extraout_ECX_01,*(int *)(DAT_00563c34 + 4) >> 0x1f,g_ActiveTrackPalette + 8);
      Gfx_FreeSurface((int)var_u7);
    }
  }
  Gfx_SetRenderTarget(&g_VirtualFramebuffer,g_ScreenWidth,g_ScreenWidth,g_ScreenHeight,8);
  var_u5 = g_ScreenWidth * g_ScreenHeight;
  if (0 < (int)var_u5) {
    var_pu6 = &g_VirtualFramebuffer;
    for (var_u4 = var_u5 >> 2; var_u4 != 0; var_u4 = var_u4 - 1) {
      *var_pu6 = 0;
      var_pu6 = var_pu6 + 1;
    }
    for (var_u5 = var_u5 & 3; var_u5 != 0; var_u5 = var_u5 - 1) {
      *(char *)var_pu6 = 0;
      var_pu6 = (int *)((int)var_pu6 + 1);
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
  int var_i1;
  int var_i2;
  int var_i3;
  int var_i4;
  __int64 var_l5;
  unsigned __int64 var_u6;
  __int64 var_u7;
  var_l5 = __ftol();
  DAT_005285c4 = (int)var_l5;
  var_l5 = __ftol();
  DAT_00552f50 = (int)var_l5;
  var_l5 = __ftol();
  DAT_00552d80 = (int)var_l5;
  var_l5 = __ftol();
  DAT_00552f5c = (int)var_l5;
  var_i2 = 0;
  if ((g_IsSplitScreen == 0) || (DAT_00552f58 == 1)) {
    var_i2 = 0;
    var_i4 = 0;
    if (0 < g_NumRacers) {
      do {
        var_i2 = var_i2 + 0x4c;
        var_i4 = var_i4 + 1;
        *(int *)(g_PlayerHUDState + -0x20 + var_i2) = (int)g_ScreenWidth / 2 - DAT_005285c4 / 2;
        *(int *)(g_PlayerHUDState + -0x1c + var_i2) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
        *(int *)(g_PlayerHUDState + -0x18 + var_i2) = DAT_005285c4 / 2 + (int)g_ScreenWidth / 2;
        *(int *)(g_PlayerHUDState + -0x14 + var_i2) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
      } while (var_i4 < g_NumRacers);
    }
  }
  else {
    var_i4 = 0;
    if (0 < g_NumRacers) {
      var_i3 = 0;
      do {
        *(int *)(g_PlayerHUDState + 0x2c + var_i2) =
             (((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) * 3 - DAT_005285c4 / 2)
             + 1;
        *(int *)(g_PlayerHUDState + 0x30 + var_i2) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
        *(int *)(g_PlayerHUDState + 0x34 + var_i2) =
             ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) * 3 + DAT_005285c4 / 2;
        *(int *)(g_PlayerHUDState + 0x38 + var_i2) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
        *(int *)(g_VehicleConfigs + 0x58 + var_i3) = *(int *)(g_VehicleConfigs + 0x58);
        *(int *)(g_VehicleConfigs + 0x5c + var_i3) = *(int *)(g_VehicleConfigs + 0x5c);
        var_i1 = g_VehicleConfigs;
        *(int *)(g_VehicleConfigs + 100 + var_i3) = *(int *)(g_VehicleConfigs + 100);
        var_i3 = var_i3 + 200;
        var_i2 = var_i2 + 0x4c;
        var_i4 = var_i4 + 1;
        *(int *)(var_i1 + -0x68 + var_i3) = *(int *)(var_i1 + 0x60);
      } while (var_i4 < g_NumRacers);
    }
    *(int *)(g_PlayerHUDState + 0x78) =
         ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) - DAT_005285c4 / 2;
    *(int *)(g_PlayerHUDState + 0x7c) = g_ScreenHeight / 2 - DAT_00552f50 / 2;
    *(int *)(g_PlayerHUDState + 0x80) =
         DAT_005285c4 / 2 + ((int)(g_ScreenWidth + ((int)g_ScreenWidth >> 0x1f & 3U)) >> 2) + -1;
    *(int *)(g_PlayerHUDState + 0x84) = DAT_00552f50 / 2 + g_ScreenHeight / 2;
  }
  var_u6 = Lisa_SetCameraViewport();
  g_ViewportMinX = 0;
  g_ViewportMaxX = g_ScreenWidth - 1;
  g_ViewportMinY = 0;
  g_ViewportMaxY = g_ScreenHeight + -1;
  var_u7 = Audio_LoadAssets(g_ScreenWidth,(unsigned int)(var_u6 >> 0x20));
  if (-1 < (int)var_u7) {
    _DAT_00498730 = g_pLisaDrawCommandQueue;
    _DAT_0049873c = g_pActiveTAB;
    _DAT_00498734 = &g_VirtualFramebuffer;
    _DAT_00498738 = &g_VirtualFramebuffer;
    Audio_StopSample();
    var_u7 = Audio_LoadAssets(g_ScreenHeight,g_ScreenWidth);
    if (-1 < (int)var_u7) {
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
  char var_24 [32];
  int var_4;
  if (DAT_004949b4 != 0) {
    _sprintf(var_24,s_FPS__d_00499658, 0);
    Font_PrintDirect(0x10e,0,var_24,(void*)0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  }
  if (DAT_004949dc == 1) {
    _sprintf(var_24,s_RECORDING__0049964c);
    Font_PrintDirect(0x96,10,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  }
  if (DAT_005532b0 == 1) {
    var_4 = DAT_00563bec / 10;
    _sprintf(var_24,s_ROLL___1f_00499640,(double)var_4);
    Font_PrintDirect(5,0x1e,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
    return;
  }
  _sprintf(var_24,s_OLDROADNR__d_00499630);
  Font_PrintDirect(5,10,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(var_24,s_SQUASHED2__d_00499620);
  Font_PrintDirect(5,0x14,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(var_24,s_TYPE1__d_00499614);
  Font_PrintDirect(0,0x28,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(var_24,s_TYPE2__d_00499608);
  Font_PrintDirect(0,0x32,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(var_24,s_SKILLNAD___3f_004995f8,
           ABS(*(double *)(g_Vehicles + 0x128) - *(double *)(g_Vehicles + 0x120)));
  Font_PrintDirect(0,0x3c,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  _sprintf(var_24,s_SPEED___3f_004995ec,ABS(*(double *)(g_Vehicles + 0x118)));
  Font_PrintDirect(0,0x46,var_24,0x563db0,g_ScreenWidth,g_ScreenHeightAlt,'\x14');
  return;
}

/**
 * @original HUD_CheckWrongWayHeading (IGN_WIN.EXE @ 0x0043edf0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_CheckWrongWayHeading(int player_idx) {
  int var_i1;
  int var_i2;
  int *var_pi3;
  double var_d4;
  unsigned int var_u5;
  double var_f6;
  double var_f7;
  __int64 var_l8;
  var_i1 = player_idx * 0x484c;
  var_u5 = *(unsigned int *)(g_Vehicles + 0x364 + player_idx * 0x484c);
  if ((int)var_u5 < 0) {
    var_d4 = *(double *)
             (g_pTrackRoadSequence + 0x10 + ((var_u5 ^ (int)var_u5 >> 0x1f) - ((int)var_u5 >> 0x1f)) * 0x18);
  }
  else {
    var_d4 = *(double *)(g_pTrackRoadSequence + 4 + var_u5 * 0x18);
  }
  var_f6 = Math_WrapAngle(var_d4 - _DAT_0047a888,6.2831853071796);
  var_f7 = Math_WrapAngle((double)CONCAT44(*(int *)(g_Vehicles + 0xfc + var_i1),
                                        *(int *)(g_Vehicles + 0xf8 + var_i1)),
                       6.2831853071796);
  if (((double)ABS((double)(double)var_f6 - var_f7) <= _DAT_0047a898) ||
     (_DAT_0047a8a0 <= (double)ABS((double)(double)var_f6 - var_f7))) {
    var_pi3 = (int *)(g_Vehicles + 0x580 + var_i1);
    if (0 < *var_pi3) {
      var_l8 = __ftol();
      *var_pi3 = (int)var_l8;
    }
  }
  else {
    var_i2 = g_Vehicles + 0x580;
    var_l8 = __ftol();
    *(int *)(var_i2 + var_i1) = (int)var_l8;
    if (0x78 < *(int *)(g_Vehicles + 0x580 + var_i1)) {
      *(int *)(g_Vehicles + 0x580 + var_i1) = 0x78;
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
  int *var_pu1;
  int var_i2;
  char *var_pc3;
  int var_i4;
  int var_i5;
  int var_i6;
  unsigned int var_u7;
  int var_i8;
  double *var_pd9;
  double var_f10;
  __int64 var_l11;
  int *var_pi12;
  int var_i13;
  int var_u14;
  char *var_pu15;
  int var_80;
  int var_7c;
  int var_74;
  int var_70;
  int var_6c;
  int var_68;
  int var_64;
  double var_60 [8];
  char var_20 [32];
  var_68 = player_idx * 0x4c;
  var_i4 = g_PlayerHUDState + var_68;
  Gfx_SetClipRect(*(int *)(var_i4 + 0x2c),*(int *)(var_i4 + 0x30),
               *(int *)(var_i4 + 0x34),*(int *)(var_i4 + 0x38));
  if (((*(unsigned int *)(g_VehicleConfigs + 0x3c + player_idx * 200) & 0x7fffffff) == 0 &&
       *(int *)(g_VehicleConfigs + 0x38 + player_idx * 200) == 0) || (g_IsDemoMode == 1)) {
    var_i8 = g_PlayerHUDState + var_68;
    var_i4 = *(int *)(var_i8 + 0x2c);
    var_74 = *(int *)(var_i8 + 0x30);
    var_i6 = *(int *)(var_i8 + 0x34);
    var_6c = *(int *)(var_i8 + 0x38);
  }
  else {
    var_f10 = Math_RandomFloat0To1();
    if (var_f10 < (double)_DAT_0047a8b0) {
      var_i8 = g_PlayerHUDState + var_68;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x2c);
      var_l11 = __ftol();
      var_i4 = (int)var_l11;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x30);
      var_l11 = __ftol();
      var_74 = (int)var_l11;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x34);
      var_l11 = __ftol();
      var_i6 = (int)var_l11;
      var_60[0] = (double)CONCAT44(((int*)&(var_60[0]))[1],*(int *)(var_i8 + 0x38));
      var_l11 = __ftol();
      var_6c = (int)var_l11;
    }
    else {
      var_i8 = g_PlayerHUDState + var_68;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x2c);
      var_l11 = __ftol();
      var_i4 = (int)var_l11;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x30);
      var_l11 = __ftol();
      var_74 = (int)var_l11;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i8 + 0x34);
      var_l11 = __ftol();
      var_i6 = (int)var_l11;
      var_60[0] = (double)CONCAT44(((int*)&(var_60[0]))[1],*(int *)(var_i8 + 0x38));
      var_l11 = __ftol();
      var_6c = (int)var_l11;
    }
  }
  if (g_GameMode != 3) {
    var_i8 = var_i4 + 7;
    var_7c = (var_74 + 7) * 0x100;
    var_80 = var_i8 * 0x100;
    Gfx_DrawSprite(DAT_005285e0,&var_80,0);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
    var_i5 = var_i8;
    if (g_RaceTimer_P2 == _DAT_0047a8c8) {
      var_i2 = *g_SpeedoPosition + var_74;
      var_i13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      var_pc3 = HUD_FormatLapTime();
    }
    else {
      var_i2 = *g_SpeedoPosition + var_74;
      var_i13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      var_pc3 = s__________00499730;
    }
    Font_DrawText(var_pc3,var_i13,var_i5,var_i2);
    var_7c = (*g_SpeedoConfig + var_74) * 0x100;
    var_80 = var_i8 * 0x100;
    Gfx_DrawSprite(DAT_005285e4,&var_80,0);
    var_i5 = var_i8;
    if (g_RaceTimer_P2 == _DAT_0047a8c8) {
      if (*(int *)(g_Vehicles + 0x374 + player_idx * 0x484c) == 0) {
        var_i2 = g_SpeedoPosition[1] + var_74;
        var_i13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
        var_pc3 = HUD_FormatLapTime();
      }
      else {
        var_i2 = g_SpeedoPosition[1] + var_74;
        var_i13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
        var_pc3 = HUD_FormatLapTime();
      }
    }
    else {
      var_i2 = g_SpeedoPosition[1] + var_74;
      var_i13 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      var_pc3 = s__________00499730;
    }
    Font_DrawText(var_pc3,var_i13,var_i5,var_i2);
    var_i5 = *(int *)(g_Vehicles + player_idx * 0x484c + 0x374);
    if (var_i5 == 1) {
      var_i13 = g_SpeedoPosition[2];
      var_i5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
LAB_0043f316:
      var_i13 = var_i13 + var_74;
      var_i2 = var_i8;
      var_pc3 = HUD_FormatLapTime();
      Font_DrawText(var_pc3,var_i5,var_i2,var_i13);
    }
    else if (1 < var_i5) {
      var_i13 = g_SpeedoPosition[2];
      var_i5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
      goto LAB_0043f316;
    }
    var_i5 = *(int *)(g_Vehicles + player_idx * 0x484c + 0x374);
    if (var_i5 == 2) {
      var_i13 = g_SpeedoPosition[3];
      var_i5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
    }
    else {
      if (var_i5 < 3) goto LAB_0043f3a7;
      var_i13 = g_SpeedoPosition[3];
      var_i5 = ((int*)&(DAT_00528624))[g_ActiveFontColor];
    }
    var_i13 = var_i13 + var_74;
    var_pc3 = HUD_FormatLapTime();
    Font_DrawText(var_pc3,var_i5,var_i8,var_i13);
  }
LAB_0043f3a7:
  if ((1 < g_NumRacers) || (DAT_00553068 == 1)) {
    if (g_GameMode == 3) {
      var_i8 = g_Vehicles + player_idx * 0x484c;
      if (*(int *)(g_Vehicles + 0x528 + player_idx * 0x484c) != 0) {
        var_u14 = *(int *)(var_i8 + 0x39c);
        goto LAB_0043f498;
      }
      _sprintf(var_20,s__d__d_00499728,*(int *)(var_i8 + 0x39c),g_PlayerCarModel);
    }
    else {
      var_80 = (g_SpeedoConfig[1] + var_i4) * 0x100;
      var_7c = (g_SpeedoConfig[2] + var_6c) * 0x100;
      Gfx_DrawSprite(DAT_005285e8,&var_80,0);
      var_i8 = g_Vehicles + player_idx * 0x484c;
      if (*(int *)(g_Vehicles + 0x528 + player_idx * 0x484c) == 0) {
        var_u14 = *(int *)(var_i8 + 0x39c);
      }
      else {
        var_u14 = *(int *)(var_i8 + 0x3a0);
      }
LAB_0043f498:
      _sprintf(var_20,&g_Format_Str_d,var_u14);
    }
    Font_DrawText(var_20,((int*)&(DAT_0052863c))[g_ActiveFontColor],g_SpeedoPosition[4] + var_i4,
                 g_SpeedoPosition[5] + var_6c);
  }
  HUD_RenderSpeedometerGauge(player_idx);
  var_i4 = player_idx * 0x484c;
  var_i5 = g_Vehicles + var_i4;
  var_i8 = *(int *)(var_i5 + 0x27c) << 2;
  var_60[0] = (double)CONCAT44(((int*)&(var_60[0]))[1],var_i8);
  if (*(double *)(var_i5 + 0x280) < (double)var_i8) {
    *(double *)(var_i5 + 0x280) = _DAT_00563d80 * _DAT_0047a8b0 + *(double *)(var_i5 + 0x280);
    var_i8 = var_i4 + g_Vehicles;
    var_60[0] = (double)(*(int *)(var_i8 + 0x27c) << 2);
    if (var_60[0] < *(double *)(var_i8 + 0x280)) {
      *(double *)(var_i8 + 0x280) = var_60[0];
    }
  }
  var_i5 = var_i4 + g_Vehicles;
  var_i8 = *(int *)(var_i5 + 0x27c) << 2;
  var_60[0] = (double)CONCAT44(((int*)&(var_60[0]))[1],var_i8);
  if ((double)var_i8 < *(double *)(var_i5 + 0x280)) {
    *(double *)(var_i5 + 0x280) = _DAT_00563d80 * _DAT_0047a8d0 + *(double *)(var_i5 + 0x280);
    var_i8 = var_i4 + g_Vehicles;
    var_60[0] = (double)(*(int *)(var_i8 + 0x27c) << 2);
    if (*(double *)(var_i8 + 0x280) < var_60[0]) {
      *(double *)(var_i8 + 0x280) = var_60[0];
    }
  }
  var_u14 = 0;
  var_80 = (g_SpeedoConfig[3] + var_i6) * 0x100;
  var_pi12 = &var_80;
  var_7c = (g_SpeedoConfig[4] + var_6c) * 0x100;
  var_l11 = __ftol();
  Gfx_DrawSprite(((int*)&(DAT_005285f4))[(int)var_l11],var_pi12,var_u14);
  var_i8 = var_6c;
  if (g_ActiveFontColor == 0) {
    var_60[0] = 4.2;
  }
  if (g_ActiveFontColor == 1) {
    var_60[0] = 2.1;
  }
  if (g_ActiveFontColor == 2) {
    var_60[0] = 1.0;
  }
  *g_pLisaDrawCommandQueue = g_LisaDrawCommandBuffer;
  g_pLisaDrawCommandQueue[1] = g_LisaDrawCommandBuffer + 0xc;
  g_pLisaDrawCommandQueue[2] = 0;
  *g_LisaDrawCommandBuffer = 0xf;
  g_LisaDrawCommandBuffer[1] = (g_SpeedoConfig[0x11] + var_i6) * 0x100;
  g_LisaDrawCommandBuffer[2] = (g_SpeedoConfig[0x12] + var_6c) * 0x100;
  g_LisaDrawCommandBuffer[3] = 0;
  g_LisaDrawCommandBuffer[4] = (g_SpeedoConfig[0x11] + var_i6) * 0x100;
  var_70 = g_SpeedoConfig[0x12] + var_6c;
  var_l11 = __ftol();
  var_i5 = var_6c;
  g_LisaDrawCommandBuffer[5] = (int)var_l11 << 8;
  g_LisaDrawCommandBuffer[6] = 0;
  g_LisaDrawCommandBuffer[7] = (g_SpeedoConfig[0x13] + var_i6) * 0x100;
  g_LisaDrawCommandBuffer[8] = (g_SpeedoConfig[0x12] + var_i8) * 0x100;
  g_LisaDrawCommandBuffer[9] = 0;
  g_LisaDrawCommandBuffer[10] = 0xd7;
  g_LisaDrawCommandBuffer[0xb] = 0;
  g_LisaDrawCommandBuffer[0xc] = 0xf;
  g_LisaDrawCommandBuffer[0xd] = (g_SpeedoConfig[0x13] + var_i6) * 0x100;
  g_LisaDrawCommandBuffer[0xe] = (g_SpeedoConfig[0x12] + var_6c) * 0x100;
  g_LisaDrawCommandBuffer[0xf] = 0;
  g_LisaDrawCommandBuffer[0x10] = (g_SpeedoConfig[0x11] + var_i6) * 0x100;
  var_70 = g_SpeedoConfig[0x12] + var_6c;
  var_l11 = __ftol();
  g_LisaDrawCommandBuffer[0x11] = (int)var_l11 << 8;
  g_LisaDrawCommandBuffer[0x12] = 0;
  g_LisaDrawCommandBuffer[0x13] = (g_SpeedoConfig[0x13] + var_i6) * 0x100;
  ((int*)&(var_60[0]))[0] = g_SpeedoConfig[0x12] + var_i5;
  var_l11 = __ftol();
  g_LisaDrawCommandBuffer[0x14] = (int)var_l11 << 8;
  g_LisaDrawCommandBuffer[0x15] = 0;
  g_LisaDrawCommandBuffer[0x16] = 0xd7;
  var_pu1 = g_LisaDrawCommandBuffer;
  g_LisaDrawCommandBuffer[0x17] = 0;
  Lisa_FlushRasterizerCommands(var_pu1,(unsigned int)((unsigned __int64)var_l11 >> 0x20));
  var_80 = (g_SpeedoConfig[0xd] + var_i6) * 0x100;
  var_7c = (g_SpeedoConfig[0xe] + var_6c) * 0x100;
  Gfx_DrawSprite(DAT_00528618,&var_80,0);
  var_i8 = g_Vehicles + var_i4;
  if (*(int *)(var_i8 + 0x53c) == 0) {
    if ((*(double *)(var_i8 + 0x534) < _DAT_0047a8f0) ||
       (_DAT_0047a8f8 <= *(double *)(var_i8 + 0x534))) {
      if (*(double *)(var_i8 + 0x534) < _DAT_0047a8f8) goto LAB_0043fa85;
      if ((g_IsDemoMode == 0) || (DAT_00552f10 != 2)) {
        *(int *)(var_i8 + 0x540) = *(int *)(var_i8 + 0x540) + 1;
        if (0xe < *(int *)(g_Vehicles + 0x540 + var_i4)) {
          *(int *)(g_Vehicles + 0x540 + var_i4) = 0;
        }
      }
      if (*(int *)(g_Vehicles + 0x540 + var_i4) < 7) {
        var_80 = (g_SpeedoConfig[0xf] + var_i6) * 0x100;
        var_7c = (g_SpeedoConfig[0x10] + var_6c) * 0x100;
        var_u14 = DAT_00528620;
      }
      else {
        var_80 = (g_SpeedoConfig[0xf] + var_i6) * 0x100;
        var_7c = (g_SpeedoConfig[0x10] + var_6c) * 0x100;
        var_u14 = DAT_0052861c;
      }
    }
    else {
      var_80 = (g_SpeedoConfig[0xf] + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x10] + var_6c) * 0x100;
      var_u14 = DAT_0052861c;
    }
LAB_0043fb28:
    Gfx_DrawSprite(var_u14,&var_80,0);
  }
  else {
LAB_0043fa85:
    if ((0 < *(int *)(var_i8 + 0x53c)) || (*(double *)(var_i8 + 0x534) < _DAT_0047a8f0)) {
      if ((g_IsDemoMode == 0) || (DAT_00552f10 != 2)) {
        *(int *)(var_i8 + 0x540) = *(int *)(var_i8 + 0x540) + 1;
        if (1 < *(int *)(g_Vehicles + 0x540 + var_i4)) {
          *(int *)(g_Vehicles + 0x540 + var_i4) = 0;
        }
      }
      var_80 = (g_SpeedoConfig[0xf] + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x10] + var_6c) * 0x100;
      var_u14 = ((int*)&(DAT_0052861c))[*(int *)(g_Vehicles + 0x540 + var_i4)];
      goto LAB_0043fb28;
    }
  }
  var_i6 = var_i4 + g_Vehicles;
  if ((*(int *)(var_i6 + 0x528) == 0) && (g_RaceTimer_P2 == _DAT_0047a8c8)) {
    var_u7 = *(unsigned int *)(var_i6 + 0x364);
    if ((int)var_u7 < 0) {
      var_i8 = *(int *)(g_pTrackRoadSequence + 0xc +
                      ((var_u7 ^ (int)var_u7 >> 0x1f) - ((int)var_u7 >> 0x1f)) * 0x18);
    }
    else {
      var_i8 = *(int *)(g_pTrackRoadSequence + var_u7 * 0x18);
    }
    if ((*(int *)(var_i6 + 0x580) < 0x1a) || (*(int *)(var_i6 + 0x368) <= (int)var_u7)) {
      if (*(int *)(DAT_00552e40 + 8 + var_i8 * 0xc) < 1) {
        *(int *)(var_i6 + 0x5f8) = 0;
      }
      else {
        *(int *)(var_i6 + 0x5f8) = *(int *)(var_i6 + 0x5f8) + 1;
        var_i6 = *(int *)(g_Vehicles + 0x5f8 + var_i4);
        var_i5 = var_i6 + (var_i6 >> 0x1f & 3U);
        var_u7 = var_i5 >> 0x1f;
        if ((((var_i5 >> 2 ^ var_u7) - var_u7 & 1 ^ var_u7) == var_u7) || (0x17 < var_i6)) {
          var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
          var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 -
                      *(int *)(DAT_00563d8c + 0x228) / 2) + var_i6) * 0x100;
          var_7c = (g_SpeedoConfig[0x1b] + var_74) * 0x100;
          var_i6 = *(int *)(DAT_00552e40 + 8 + var_i8 * 0xc);
          if (0x10 < var_i6) {
            var_i6 = 0x10;
          }
          Gfx_DrawSprite(((int*)&(DAT_00528664))[*(int *)(&DAT_00497d74 + (DAT_00563ce8 * 0x14 + var_i6) * 4)],
                       &var_80,0);
        }
      }
    }
    else {
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 -
                  *(int *)(DAT_00563d8c + 0x228) / 2) + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x1b] + var_74) * 0x100;
      Gfx_DrawSprite(DAT_005286a0,&var_80,0);
    }
  }
  if (g_GameMode == 3) {
    if (((*(int *)(g_Vehicles + var_i4 + 0x39c) == g_PlayerCarModel) &&
        (*(int *)(g_Vehicles + var_i4 + 0x528) == 0)) &&
       ((g_IsDemoMode == 0 || (DAT_00552f10 != 2)))) {
      if (g_LanguageId == 0) {
        var_pc3 = s_WARNING__YOU_RE_LAST__00499710;
LAB_0043fda0:
        _sprintf(var_20,var_pc3);
      }
      else {
        if (g_LanguageId == 1) {
          var_pc3 = s_SIE_SIND_LETZTER__004996fc;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 2) {
          var_pc3 = s_ATTENZIONE__SEI_ULTIMO_004996e4;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 3) {
          var_pc3 = s_AVISO__ERES_EL_ULTIMO_004996cc;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 4) {
          var_pc3 = s_VARNING__DU_LIGGER_SIST__004996b0;
          goto LAB_0043fda0;
        }
        if (g_LanguageId == 5) {
          var_pc3 = s_ATTENTION__VOUS_ETES_DERNIER__00499690;
          goto LAB_0043fda0;
        }
      }
      if (player_idx == 0) {
        if (g_IsSplitScreen == 0) {
          var_i6 = -1;
        }
        else {
          if (g_IsSplitScreen != 1) goto LAB_0043fdde;
          var_i6 = 0;
        }
      }
      else {
LAB_0043fdde:
        if ((player_idx != 1) || (g_IsSplitScreen != 1)) goto LAB_0043fe07;
        var_i6 = 1;
      }
      HUD_AddFloatingMessage(var_20,0,1,var_i6);
    }
LAB_0043fe07:
    if (DAT_00552fc0 == 0) {
      var_i8 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x30);
      var_u14 = DAT_005287ec;
    }
    else {
      var_i8 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x30);
      var_u14 = DAT_005287f0;
    }
    var_7c = (var_i6 + 8) * 0x100;
    var_80 = (var_i8 + 8) * 0x100;
    Gfx_DrawSprite(var_u14,&var_80,0);
    var_64 = (-(unsigned int)(DAT_00552fc0 == 0) & 0xffffff4c) + 300;
    var_i6 = g_NumRacers + -1;
    if (-1 < var_i6) {
      var_pd9 = var_60 + var_i6;
      var_i8 = var_i6 * 0x4c;
      var_i6 = var_i6 * 0x484c;
      do {
        if (_DAT_005285d8 < _DAT_0047a900) {
          *(int *)var_pd9 = 0;
          *(int *)((int)var_pd9 + 4) = 0;
        }
        else {
          *var_pd9 = ABS((double)*(int *)(g_Vehicles + 0x364 + var_i6)) / (double)DAT_005287f8;
        }
        if (*(int *)(g_Vehicles + 0x528 + var_i6) == 0) {
          var_i5 = g_PlayerHUDState + var_68;
          var_80 = (*(int *)(var_i5 + 0x2c) + 5) * 0x100;
          var_l11 = __ftol();
          var_70 = (int)var_l11;
          var_7c = ((*(int *)(var_i5 + 0x30) - var_70) + var_64 + 5) * 0x100;
          Gfx_DrawSprite(((int*)&(DAT_00528764))[*(int *)(g_PlayerHUDState + var_i8)],&var_80,0);
          if (*(int *)(g_Vehicles + 0x39c + var_i6) == 1) {
            var_80 = (*(int *)(var_68 + g_PlayerHUDState + 0x2c) + 0x13) * 0x100;
            var_7c = ((*(int *)(var_68 + g_PlayerHUDState + 0x30) - var_70) + var_64 + 8) *
                       0x100;
            Gfx_DrawSprite(DAT_005287f4,&var_80,0);
          }
        }
        var_i8 = var_i8 + -0x4c;
        var_i6 = var_i6 + -0x484c;
        var_pd9 = var_pd9 + -1;
      } while (var_60 <= var_pd9);
    }
    if (-1 < g_NumRacers + -1) {
      var_i6 = (g_NumRacers + -1) * 0x484c;
      do {
        var_i8 = g_Vehicles + var_i6;
        if (((*(int *)(var_i8 + 0x528) == 1) && (*(double *)(var_i8 + 0x60c) < _DAT_0047a908)) &&
           (*(int *)(var_i8 + 0x3a0) != 1)) {
          var_80 = *(int *)(var_68 + g_PlayerHUDState + 0x2c) << 8;
          var_i8 = *(int *)(var_68 + g_PlayerHUDState + 0x30);
          var_l11 = __ftol();
          var_u14 = 0;
          var_pi12 = &var_80;
          var_7c = ((var_i8 - (int)var_l11) + var_64) * 0x100;
          var_l11 = __ftol();
          Gfx_DrawSprite(((int*)&(DAT_0052873c))[(int)var_l11],var_pi12,var_u14);
        }
        var_i6 = var_i6 + -0x484c;
      } while (-1 < var_i6);
    }
    if ((player_idx == 1) && (*(int *)(g_Vehicles + 0x4d74) == 0)) {
      if (_DAT_005285d8 < _DAT_0047a900) {
        var_60[1] = 0.0;
      }
      else {
        var_60[1] = ABS((double)*(int *)(g_Vehicles + 0x4bb0)) / (double)DAT_005287f8;
      }
      var_80 = (*(int *)(var_68 + g_PlayerHUDState + 0x2c) + 5) * 0x100;
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x30);
      var_l11 = __ftol();
      var_7c = ((var_i6 - (int)var_l11) + var_64 + 5) * 0x100;
      Gfx_DrawSprite(((int*)&(DAT_00528764))[*(int *)(g_PlayerHUDState + 0x4c)],&var_80,0);
    }
  }
  if ((g_RaceTimer_P2 < _DAT_0047a910) || (_DAT_0047a8f8 <= g_RaceTimer_P2)) {
    if ((_DAT_0047a8f8 <= g_RaceTimer_P2) && (g_RaceTimer_P2 < _DAT_0047a918)) {
      if (DAT_00563c38 == 0) {
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        Audio_PlaySampleVol(0,4,0,0,0x10000,22000,0);
        DAT_00563c38 = 1;
      }
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x19] + var_74) * 0x100;
      var_u14 = DAT_00528658;
      goto LAB_00440490;
    }
    if ((_DAT_0047a918 <= g_RaceTimer_P2) && (g_RaceTimer_P2 < _DAT_0047a920)) {
      if (DAT_00563c38 == 1) {
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        Audio_PlaySampleVol(0,4,0,0,0x10000,22000,0);
        DAT_00563c38 = 2;
      }
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x19] + var_74) * 0x100;
      var_u14 = DAT_0052865c;
      goto LAB_00440490;
    }
    if ((g_RaceTimer_P2 == _DAT_0047a8c8) && (_DAT_005285d8 < _DAT_0047a900)) {
      if (DAT_00563c38 == 2) {
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        Audio_PlaySampleVol(0,4,1,0,0x10000,22000,0);
        DAT_00563c38 = 3;
      }
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 - g_SpeedoConfig[0x18] / 2)
                 + var_i6) * 0x100;
      var_7c = (g_SpeedoConfig[0x19] + var_74) * 0x100;
      var_u14 = DAT_00528660;
      goto LAB_00440490;
    }
  }
  else {
    var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
    var_80 = (((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 - g_SpeedoConfig[0x18] / 2) +
               var_i6) * 0x100;
    var_7c = (g_SpeedoConfig[0x19] + var_74) * 0x100;
    var_u14 = DAT_00528654;
LAB_00440490:
    Gfx_DrawSprite(var_u14,&var_80,0);
  }
  if (((g_RaceTimer < 0.0) || (0x77 < (int)((int*)&(DAT_00527f40))[player_idx])) ||
     ((g_GameMode == 2 && (g_NumRacers < 2)))) goto LAB_004407ac;
  if (g_GameMode == 3) {
LAB_00440508:
    if (1 < *(int *)(g_Vehicles + 0x3a0 + var_i4)) goto LAB_004407ac;
  }
  else if (3 < *(int *)(g_Vehicles + 0x3a0 + var_i4)) {
    if (g_GameMode != 3) goto LAB_004407ac;
    goto LAB_00440508;
  }
  var_i4 = g_Vehicles + var_i4;
  if (*(int *)(var_i4 + 0x528) == 1) {
    if (((0 < *(int *)(var_i4 + 0x5d4)) && (*(int *)(var_i4 + 0x3a0) < 4)) &&
       ((g_GameMode != 2 || (1 < g_NumRacers)))) {
      var_i6 = *(int *)(var_68 + g_PlayerHUDState + 0x2c);
      var_80 = ((*(int *)(var_68 + g_PlayerHUDState + 0x34) - var_i6) / 2 + var_i6) * 0x100;
      var_7c = (var_6c - var_74) / 2 << 8;
      ((int*)&(var_60[0]))[0] = *(int *)(var_i4 + 0x5d4);
      _DAT_00563d74 = 0;
      _DAT_00563d70 = (float)((int*)&(var_60[0]))[0] * (float)_DAT_0047a8b8;
      if (*(int *)(var_i4 + 0x5d4) < 0x19) {
        var_pu15 = &DAT_00563d70;
      }
      else {
        var_pu15 = (char *)0x0;
      }
      Gfx_DrawSprite(((int*)&(DAT_00528794))[*(int *)(var_i4 + 0x3a0)],&var_80,var_pu15);
      if ((99 < (int)((int*)&(DAT_006192a0))[player_idx * 10]) && (*(int *)(&DAT_005531c0 + player_idx * 4) == 0))
      {
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        var_l11 = __ftol();
        ((int*)&(var_60[0]))[0] = (int)var_l11;
        Audio_PlaySampleVol(0,4,5,0,0x10000,22000,0);
        *(int *)(&DAT_005531c0 + player_idx * 4) = 1;
      }
    }
    var_i6 = 6;
    var_i4 = player_idx * 0x28;
    do {
      if ((0xc < *(int *)((int)&DAT_006192a0 + var_i4)) && ((int)((int*)&(DAT_00527f40))[player_idx] < 0x28)) {
        var_i5 = g_PlayerHUDState + var_68;
        var_i8 = *(int *)(var_i5 + 0x2c);
        Math_RandomFloat0To1();
        ((int*)&(var_60[0]))[0] = (*(int *)(var_i5 + 0x34) - var_i8) / 2 + var_i8;
        var_l11 = __ftol();
        *(int *)((int)&DAT_00552d90 + var_i4) = (int)var_l11;
        Math_RandomFloat0To1();
        ((int*)&(var_60[0]))[0] = (var_6c - var_74) / 2;
        var_l11 = __ftol();
        *(int *)((int)&DAT_00552df0 + var_i4) = (int)var_l11;
        if (*(int *)((int)&DAT_006192a0 + var_i4) < 100) {
          *(int *)((int)&DAT_006192a0 + var_i4) = 0;
        }
        else {
          Math_RandomFloat0To1();
          var_l11 = __ftol();
          *(int *)((int)&DAT_006192a0 + var_i4) = (int)var_l11;
        }
      }
      if (*(int *)((int)&DAT_006192a0 + var_i4) < 0xd) {
        _DAT_00563d70 = 0.5;
        _DAT_00563d74 = 0;
        var_80 = *(int *)((int)&DAT_00552d90 + var_i4) << 8;
        var_7c = *(int *)((int)&DAT_00552df0 + var_i4) << 8;
      }
      var_i4 = var_i4 + 4;
      var_i6 = var_i6 + -1;
    } while (var_i6 != 0);
  }
LAB_004407ac:
  HUD_RenderFloatingMessages();
  return;
}
