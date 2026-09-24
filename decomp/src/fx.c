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
#include <string.h>
#define fsin sin
#define fcos cos
#define __ftol() (int)Math_RandomFloat()
#define CONCAT44(high, low) ((((__int64)(high)) << 32) | (low))
#include "main.h"
#include "fx.h"

// Reconstructed FX and HUD global constants and tables

extern double k_WreckRespawnSplineOffset;
extern double k_WreckRespawnAngleMin;
extern double k_WreckRespawnAngleModulo;
extern double k_WreckRespawnInterpolationScale;
extern double k_WreckRespawnHeadingMax;
extern double k_WreckRespawnHeadingMin;
extern double k_WreckRespawnVehicleClearance;

extern int g_WheelParticleDescriptor;
extern int g_TrackRoadSequenceNodeCount;
extern double k_CrashElevationStepUp;
extern double k_CrashElevationStepDown;
extern double k_CrashTumblePitchDelta;
extern double k_CrashTumbleRollDelta;
extern double k_CrashSoundRandomChance;
extern double k_RespawnSplineOffset;
extern double k_RespawnAngleMin;
extern double k_RespawnAngleModulo;
extern double k_RespawnInterpolationScale;
extern double k_RespawnHeadingMax;
extern double k_RespawnHeadingMin;
extern double k_RespawnVehicleClearance;

extern int g_TrigAngleTable[];
extern int g_SmokeParticleDescriptor;
extern char g_SurfaceDustFlagTable[];
extern int g_ForceDirtDebris;
extern int g_ShowFpsOverlay;
extern int g_ShowRecordingOverlay;
extern int g_ShowRollTelemetry;
extern int g_TelemetryRollAngle;
extern int g_WeatherActive;
extern int g_pAnimatedSceneryObjects;
extern int g_DebrisDynamicMeshes;
extern int g_CameraFovScaleX;
extern int g_CameraFovScaleY;
extern int g_CameraTargetHysteresis;

extern double k_SkidSmokeSpeedThreshold;
extern double k_SkidSmokeProbabilityFactor;
extern double k_SkidSmokeRandomScale;
extern double k_TireDirtSpeedThreshold;
extern double k_WaterSplashSpeedThreshold;
extern double k_LandingDustAngularOffset;
extern double k_SplashAccumIncrement;
extern double k_TurboProgressMinTimer;
extern double k_SpecialCarBonusLimit;
extern double k_SpecialCarBonusStep;
extern double k_WrongWayTrackHeadingOffset;
extern double k_WrongWayThresholdMin;
extern double k_WrongWayThresholdMax;


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

extern int g_TrigAngleTable;
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
extern int g_TelemetryRollAngle;
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
extern int g_WeatherActive;
extern int _DAT_00552f34;
extern int DAT_00552f34;
extern int DAT_0047a548;
extern int k_SkidSmokeRandomScale;
extern int DAT_00563d74;
extern int _DAT_0047a548;
extern int DAT_00528624;
extern int DAT_00528808;
extern int DAT_0047a798;
extern int DAT_00639378;
extern int DAT_0047a480;
extern int DAT_005287f0;
extern int k_WrongWayTrackHeadingOffset;
extern int DAT_0047a518;
extern int _DAT_0047a768;
extern int k_WrongWayThresholdMax;
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
extern int g_ShowRecordingOverlay;
extern int DAT_0047a3f0;
extern int _DAT_0047a550;
extern int DAT_0047a4f0;
extern int DAT_00563c38;
extern int DAT_00528818;
extern int DAT_00639340;
extern int DAT_0049847c;
extern int DAT_0047a898;
extern int k_WreckRespawnInterpolationScale;
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
extern int g_pAnimatedSceneryObjects;
extern int DAT_0047a318;
extern int _DAT_0047a6e8;
extern int DAT_006192b8;
extern int DAT_00639334;
extern int DAT_00563da0;
extern int _DAT_0047a788;
extern int g_TrackRoadSequenceNodeCount;
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
extern int k_WreckRespawnHeadingMin;
extern int k_RespawnHeadingMax;
extern int DAT_00528810;
extern int DAT_005db038;
extern int k_CrashTumblePitchDelta;
extern int DAT_00552d90;
extern int DAT_00527f74;
extern int DAT_005db000;
extern int DAT_00553784;
extern int g_SurfaceDustFlagTable;
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
extern int k_WreckRespawnVehicleClearance;
extern int DAT_0047a730;
extern int DAT_0063935c;
extern int DAT_0047a288;
extern int DAT_0047a6e8;
extern int DAT_00563ce8;
extern int k_WaterSplashSpeedThreshold;
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
extern int k_RespawnInterpolationScale;
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
extern int k_LandingDustAngularOffset;
extern int k_RespawnSplineOffset;
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
extern int k_WreckRespawnSplineOffset;
extern int DAT_0047a658;
extern int _DAT_0047a488;
extern int DAT_0052861c;
extern int k_SpecialCarBonusLimit;
extern int _DAT_0047a540;
extern int g_pCarTransforms;
extern int DAT_0063959b;
extern int g_CameraFovScaleX;
extern int _DAT_0047a760;
extern int DAT_0047a2d8;
extern int DAT_00563c34[];
extern int _DAT_0047a748;
extern int DAT_0047a310;
extern int DAT_00528878;
extern int g_pActivePLC[];
extern int k_TireDirtSpeedThreshold;
extern int DAT_00528848;
extern int DAT_00563c30[];
extern int g_ForceDirtDebris;
extern int k_CrashTumbleRollDelta;
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
extern int k_SplashAccumIncrement;
extern int DAT_00639354;
extern int DAT_0047a570;
extern int _DAT_0047a658;
extern int DAT_0063933c;
extern int DAT_0047a708;
extern int DAT_00603730;
extern int DAT_0047a710;
extern int k_SkidSmokeProbabilityFactor;
extern int DAT_00525e44;
extern int DAT_0047a760;
extern int DAT_0047a8b0;
extern int k_RespawnVehicleClearance;
extern int _DAT_0047a920;
extern int DAT_005daff0;
extern int k_TurboProgressMinTimer;
extern int DAT_00553068;
extern int DAT_00639344;
extern int DAT_00639577;
extern int DAT_0047a790;
extern int DAT_0047a748;
extern int DAT_0047a3f8;
extern int DAT_00528814;
extern int DAT_0047a718;
extern int DAT_00619310;
extern int g_DebrisDynamicMeshes;
extern int DAT_00528870;
extern int DAT_0047a700;
extern int DAT_0047a630;
extern int DAT_00553300;
extern int g_Format_Str_s;
extern int DAT_00563d28;
extern int DAT_0063936c;
extern int DAT_00528868;
extern int DAT_00553040;
extern int k_SpecialCarBonusStep;
extern int DAT_0063955b;
extern int DAT_0047a540;
extern int DAT_0052885c;
extern int _DAT_0047a6e0;
extern int DAT_00603720;
extern int DAT_0047a510;
extern int DAT_0052880c;
extern int g_CameraTargetHysteresis;
extern int DAT_0047a720;
extern int DAT_00563d00;
extern int DAT_0052883c;
extern int _DAT_0047a908;
extern int DAT_00494d94;
extern int DAT_00563d38;
extern int _DAT_0047a740;
extern int k_CrashSoundRandomChance;
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
extern int k_SkidSmokeSpeedThreshold;
extern int DAT_00639593;
extern int _DAT_0047a570;
extern int k_WrongWayThresholdMin;
extern int DAT_00528884;
extern int DAT_00639350;
extern int DAT_00552f50;
extern int DAT_0047a2d0;
extern int k_WreckRespawnAngleMin;
extern int DAT_004949e0;
extern int DAT_0047a308;
extern int DAT_0047a4d8;
extern int DAT_00639388;
extern int DAT_00563d70;
extern int g_ColorDepth;
extern int DAT_00603718;
extern int g_CameraFovScaleY;
extern int DAT_00552fdc;
extern int DAT_005285e0;
extern int DAT_00639374;
extern int DAT_0047a920;
extern int DAT_005533b0;
extern int DAT_006395b3;
extern int k_WreckRespawnHeadingMax;
extern int DAT_00497d74;
extern int g_pFXTransforms;
extern int _DAT_0063992b;
extern int DAT_0063955f;
extern int k_RespawnAngleMin;
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
extern int k_RespawnHeadingMin;
extern int _DAT_0047a720;
extern int DAT_0047a888;
extern int k_RespawnAngleModulo;
extern int k_CrashElevationStepDown;
extern int DAT_00639497;
extern int k_WreckRespawnAngleModulo;
extern int DAT_0047a520;
extern int DAT_0047a328;
extern int DAT_0054f970;
extern double k_WreckRespawnSplineOffset;
extern double k_WreckRespawnAngleMin;
extern double k_WreckRespawnAngleModulo;
extern double k_WreckRespawnInterpolationScale;
extern double k_WreckRespawnHeadingMax;
extern double k_WreckRespawnHeadingMin;
extern double k_WreckRespawnVehicleClearance;

extern int g_WheelParticleDescriptor;
extern int DAT_0063937c;
extern int _DAT_0049847c;
extern int g_VehicleCameraStates;
extern int DAT_0054f98c;
extern int g_RacePosition;
extern int DAT_00528828;
extern int g_ShowFpsOverlay;
extern int _DAT_0047a728;
extern int k_CrashElevationStepUp;
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
extern int g_SmokeParticleDescriptor;
extern int DAT_00603724;
extern int DAT_00528864;
extern int DAT_00563d8c;
extern int DAT_005285f4;
extern int DAT_0047a3c0;
extern double DAT_0063c64c[];
extern int g_ShowRollTelemetry;
extern int g_RaceTimer;
extern int DAT_0047a440;
extern int DAT_004986e0[];

#define g_CameraShakeActive _DAT_004949e0
#define g_TrackSegmentTable ((int *)&DAT_00552e40)
#define g_CarShadowVertices ((int *)&DAT_00603718)
#define g_CarTransforms ((DynamicObjectTransform *)&g_pCarTransforms)
#define g_WheelTransforms ((DynamicObjectTransform *)&g_pWheelTransforms)
#define g_CarReflectionTransforms ((DynamicObjectTransform *)&g_pCarReflectionTransforms)
#define g_CarShadowTransforms ((DynamicObjectTransform *)&g_pCarShadowTransforms)
#define g_CarGhostTransforms ((DynamicObjectTransform *)&g_pCarGhostTransforms)
#define g_pParticleArray1 ((int *)&g_ParticleArray1)
#define g_pParticleArray2 ((int *)&g_ParticleArray2)
#define g_pParticleCount1 ((int *)&g_ParticleCount1)
#define g_ParticleTemplateX ((int *)&DAT_00552e60)
#define g_ParticleTemplateY ((int *)&DAT_005533b0)
#define g_ParticleTemplateZ ((int *)&DAT_00553300)
#define g_pCarBaseMeshes ((int *)&DAT_005db000)
#define g_TrackChunkToNodeTable ((int *)&DAT_00553784)
#define s_TrackSurfaceMissing_004994cc ((const char *)&DAT_004994cc)
#define g_ActiveTrackSegmentAttribute DAT_0063c5d8
#define g_TrackSegmentTable ((int *)&DAT_00552e40)
#define g_CameraFovWobbleActive DAT_0054f92c
#define g_CameraFovWobblePhase (*(float *)&_DAT_00525e2c)
#define g_TrackFlybyCameras ((float *)&DAT_00498538)
#define g_TrackBackgroundColors ((int *)&DAT_00497ed8)
#define g_ScreenShakeTimerP1 DAT_00527f40
#define g_ScreenShakeTimerP2 DAT_00527f44
#define g_CinematicCameraAngleState DAT_00563d20
#define g_CinematicFocusedVehicle _DAT_00601664
#define g_ViewportBorderCornerTL ((void *)DAT_0054f960)
#define g_ViewportBorderCornerTR ((void *)DAT_0054f964)
#define g_ViewportBorderCornerBL ((void *)DAT_0054f968)
#define g_ViewportBorderCornerBR ((void *)DAT_0054f96c)
#define g_ViewportBorderMetrics ((int *)&DAT_00563c58)
#define g_GamePauseState DAT_00563c60
#define g_PostRaceSequenceState DAT_00563d88
#define g_TrackFlybyActiveTarget DAT_00563c5c
#define g_FlybyCameraOffsetX (*(double *)&_DAT_00563c48)
#define g_FlybyCameraOffsetY (*(double *)&_DAT_00563c40)
#define g_FlybyCameraOffsetZ (*(double *)&_DAT_00563c50)
#define g_FlybyLastCameraPosX _DAT_0054f9d8
#define g_FlybyLastCameraPosY _DAT_0054f9dc
#define g_FlybyLastCameraPosZ _DAT_0054f9e0
#define g_FlybyLastYaw DAT_00553098
#define g_FlybyLastPitch DAT_005530a0
#define g_FlybyLastRoll DAT_0055309c
#define g_SkyClearEnabled DAT_0054f934
#define g_ShowRollTelemetry DAT_00563c14
#define g_SplitScreenMode DAT_00552f58
#define g_ScreenSizeSetting DAT_00527f28
#define g_CameraPosX_Int DAT_005532c0
#define g_CameraPosY_Int DAT_005532b8
#define g_CameraPosZ_Int DAT_00553288
#define g_pLapTimerPanelSprite ((void *)DAT_005285e0)
#define g_pLapSplitPanelSprite ((void *)DAT_005285e4)
#define g_pRacePositionBadgeSprite ((void *)DAT_005285e8)
#define g_pGearDigitSprites ((void **)&DAT_005285f4)
#define g_pTurboGaugeBorderSprite ((void *)DAT_00528618)
#define g_pTurboIndicatorLightSprites ((void **)&DAT_0052861c)
#define g_pTurboIndicatorLightOnSprite ((void *)DAT_00528620)
#define g_FontTextColorTable ((int *)&DAT_00528624)
#define g_FontPositionColorTable ((int *)&DAT_0052863c)
#define g_pTrafficLightRedSprite ((void *)DAT_00528654)
#define g_pTrafficLightYellow1Sprite ((void *)DAT_00528658)
#define g_pTrafficLightYellow2Sprite ((void *)DAT_0052865c)
#define g_pTrafficLightGreenSprite ((void *)DAT_00528660)
#define g_pDirectionArrowSprites ((void **)&DAT_00528664)
#define g_DirectionArrowLookupTable ((int *)&DAT_00497d74)
#define g_TrackStyle DAT_00563ce8
#define g_pWrongWayBannerSprite ((void *)DAT_005286a0)
#define g_RadarHighResFlag DAT_00552fc0
#define g_pRadarProgressBarSprite_LowRes ((void *)DAT_005287ec)
#define g_pRadarProgressBarSprite_HighRes ((void *)DAT_005287f0)
#define g_pRadarCarBlipSprites ((void **)&DAT_00528764)
#define g_pRadarLeaderArrowSprite ((void *)DAT_005287f4)
#define g_pRadarEliminatedBlipSprites ((void **)&DAT_0052873c)
#define g_RaceStartTimer (*(double *)&_DAT_005285d8)
#define g_CountdownBeepStep DAT_00563c38
#define g_pFinishPlaceSprites ((void **)&DAT_00528794)
#define g_FinishFanfarePlayed ((int *)&DAT_005531c0)
#define g_FinishSparklePosX ((int *)&DAT_00552d90)
#define g_FinishSparklePosY ((int *)&DAT_00552df0)
#define g_FinishSparkleTimers ((int *)&DAT_006192a0)
#define g_SpriteScaleFactors ((float *)&_DAT_00563d70)
#define g_RpmNeedleDeltaScale (*(double *)&_DAT_00563d80)
#define g_IsTwoPlayerMode DAT_00553068
#define g_DemoSubState DAT_00552f10
#define g_PlayerFinishTimers ((int *)&DAT_00527f40)
#define g_pScreenConfig ((ScreenConfig *)DAT_00563d8c)


/**
 * @original Audio_PlaySampleVol (IGN_WIN.EXE @ 0x0043e710, fx.c)
 * @fidelity ADAPTED
 */
void Audio_PlaySampleVol(int channel, int sample_id, int sound_id, int pan, int volume, int pitch, int loop) {
    g_AudioEventParam1 = channel;
    g_AudioEventParam2 = sample_id;
    g_AudioEventParam3 = sound_id;
    g_AudioEventParam4 = pan;
    g_AudioEventParam5 = (int)Math_RandomFloat();
    g_AudioEventParam6 = pitch;
    g_AudioEventParam7 = loop;
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
__int64 Audio_LoadAssets(int width, unsigned int height) {
    __int64 result;
    result = Lisa_InitRasterizerTables(width, height);
    return (((__int64)(result >> 32)) << 32) | 1;
}

/**
 * @original Audio_StopSample (IGN_WIN.EXE @ 0x0043e6d0, fx.c)
 * @fidelity ADAPTED
 */
unsigned __int64 Audio_StopSample(void) {
    int *cmd_queue;
    unsigned __int64 flush_result;

    g_pLisaDrawCommandQueue[0] = (int)g_LisaDrawCommandBuffer;
    cmd_queue = g_pLisaDrawCommandQueue;
    g_pLisaDrawCommandQueue[1] = 0;
    g_LisaDrawCommandBuffer[0] = 0;
    g_LisaDrawCommandBuffer[1] = 0;
    g_LisaDrawCommandBuffer[2] = 0;
    flush_result = Lisa_FlushRasterizerCommands(0, (unsigned int)cmd_queue);
    return flush_result;
}

/**
 * @original FX_SpawnWeather (IGN_WIN.EXE @ 0x0043ec70, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWeather(void) {
    char text_buf[32];

    if (g_ShowDebugCoords != 0) {
        sprintf(text_buf, "X: %f", g_CameraPosX);
        Font_PrintDirect(0, 0, text_buf, (void *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(text_buf, "Y: %f", g_CameraPosY);
        Font_PrintDirect(0, 10, text_buf, (void *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(text_buf, "Z: %f", g_CameraPosZ);
        Font_PrintDirect(0, 20, text_buf, (void *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(text_buf, "X VIN: %f", g_CameraAngleX);
        Font_PrintDirect(0, 30, text_buf, (void *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenHeightAlt, 20);
        sprintf(text_buf, "Y VIN: %f", g_CameraAngleY);
        Font_PrintDirect(0, 40, text_buf, (void *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenHeightAlt, 20);
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
            } else if (g_RaceTimer >= 120.0 || g_ShowRollTelemetry == 1) {
                should_spawn = 1;
            }
        } else if (g_RaceTimer >= 120.0 || g_ShowRollTelemetry == 1) {
            should_spawn = 1;
        }

        if (g_IsSplitScreen == 1) {
            diff_x = pos_x - (int)g_pActiveCamera_P2[0];
            if (abs(diff_x) < 2500) {
                pos_z = p->pos_z / 1024;
                diff_z = pos_z - (int)g_pActiveCamera_P2[2];
                if (abs(diff_z) <= 2500) {
                    should_spawn = 1;
                } else if (g_RaceTimer >= 120.0) {
                    should_spawn = 1;
                }
            } else if (g_RaceTimer >= 120.0) {
                should_spawn = 1;
            }
        }
    }

    if (!should_spawn) {
        return 0xffffffff;
    }

    slot = &g_SceneryParticles[0];
    index = 0;
    while (index < 200) {
        if (slot->type == 0) {
            break;
        }
        slot++;
        index++;
    }

    if (index >= 200) {
        min_prio = 100;
        best_idx = 0;
        slot = &g_SceneryParticles[0];
        for (i = 0; i < 200; i++) {
            if (g_ParticlePriorityTable[slot->type] < min_prio) {
                best_idx = i;
                min_prio = g_ParticlePriorityTable[slot->type];
            }
            slot++;
        }
        if (min_prio < g_ParticlePriorityTable[p->type]) {
            index = best_idx;
        }
        if (index >= 200) {
            return 0xffffffff;
        }
    }

    g_SceneryParticles[index] = *p;
    return index;
}

/**
 * @original Race_RenderViewport (IGN_WIN.EXE @ 0x00436990, fx.c)
 * @fidelity ADAPTED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Race_RenderViewport(double delta_time) {
    VehicleState *veh;
    VehicleState *veh_p2;
    VehicleConfig *config;
    VehicleConfig *config_p2;
    DynamicObjectTransform *ghost_transform;
    DynamicObjectTransform *ghost_transform_p2;
    double cam_pos_x, cam_pos_y, cam_pos_z;
    double cam_pitch, cam_yaw;
    double cam_p2_pos_x, cam_p2_pos_y, cam_p2_pos_z;
    double cam_p2_pitch, cam_p2_yaw;
    double cos_val, sin_val;
    double wobble_sin;
    double phase;
    int camera_tilt_angle;
    int chunk_idx;
    int node_idx;
    int track_waypoint_idx;
    int border_pos[2];
    int screen_w, screen_h;
    int y, divider_x;
    int top_band_height;
    int cur_y, cur_x;
    int fb_row;
    int vp_y, vp_y2, vp_x1, vp_x2;
    int p2_y, p2_y2, p2_x1, p2_x2;
    uint8_t sky_color;
    uint8_t *fb;

    fb = (uint8_t *)&g_VirtualFramebuffer;

    /* 1. Camera FOV wobble oscillation */
    if (g_CameraFovWobbleActive == 1) {
        phase = (double)g_CameraFovWobblePhase;
        g_CameraFovWobblePhase = (float)(phase + 0.1);
        wobble_sin = sin(phase + 0.1);
        g_pActiveCamera[6] = wobble_sin;
    }

    /* 2. Demo mode: clear virtual framebuffer */
    if (g_IsDemoMode == 1) {
        int total_pixels = g_ScreenWidth * g_ScreenHeight;
        if (total_pixels > 0) {
            memset(&g_VirtualFramebuffer, 0, total_pixels);
        }
    }

    /* 3. Track-specific camera pitch/tilt override */
    camera_tilt_angle = 0;
    node_idx = *(int *)((uint8_t *)g_Vehicles + 0x364 + g_MenuCursorPos * 0x484c);

    if (g_CurrentTrackIndex == 5) {
        if (node_idx > 27 && node_idx < 70) {
            camera_tilt_angle = 30;
        }
    } else if (g_CurrentTrackIndex == 3) {
        if (node_idx > 0 && node_idx < 30) {
            camera_tilt_angle = 25;
        } else if (node_idx > 187 && node_idx < 197) {
            camera_tilt_angle = 30;
        }
    } else if (g_CurrentTrackIndex == 2) {
        top_band_height = (g_ScreenHeight / 200) * 25;
        vp_y = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
        vp_x1 = *(int *)(g_PlayerHUDState + 0x2c + g_MenuCursorPos * 0x4c);
        vp_x2 = *(int *)(g_PlayerHUDState + 0x34 + g_MenuCursorPos * 0x4c);

        for (cur_y = vp_y; cur_y < vp_y + top_band_height; cur_y++) {
            fb_row = cur_y * g_ScreenWidth;
            for (cur_x = vp_x1; cur_x < vp_x2; cur_x++) {
                fb[fb_row + cur_x] = 0x72;
            }
        }

        if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
            p2_y = *(int *)(g_PlayerHUDState + 0x7c);
            p2_x1 = *(int *)(g_PlayerHUDState + 0x78);
            p2_x2 = *(int *)(g_PlayerHUDState + 0x80);

            for (cur_y = p2_y; cur_y < p2_y + top_band_height; cur_y++) {
                fb_row = cur_y * g_ScreenWidth;
                for (cur_x = p2_x1; cur_x < p2_x2; cur_x++) {
                    fb[fb_row + cur_x] = 0x72;
                }
            }
        }
    } else if (g_CurrentTrackIndex == 1) {
        if (node_idx > -124 && node_idx < -113) {
            camera_tilt_angle = 30;
        }
    } else if (g_CurrentTrackIndex == 0) {
        if (node_idx > 36 && node_idx < 46) {
            camera_tilt_angle = 30;
        } else if (node_idx > 124 && node_idx < 140) {
            camera_tilt_angle = 35;
        }
    } else if (g_CurrentTrackIndex == 4) {
        if (node_idx > 6 && node_idx < 14) {
            camera_tilt_angle = 35;
        }
    }
    *(int *)((uint8_t *)g_pActiveCamera + 0xa4) = camera_tilt_angle;

    /* 4. Viewport background sky clearing */
    if (g_SkyClearEnabled == 1) {
        sky_color = (uint8_t)g_TrackBackgroundColors[g_CurrentTrackIndex];
        vp_y = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
        vp_y2 = *(int *)(g_PlayerHUDState + 0x38 + g_MenuCursorPos * 0x4c);
        vp_x1 = *(int *)(g_PlayerHUDState + 0x2c + g_MenuCursorPos * 0x4c);
        vp_x2 = *(int *)(g_PlayerHUDState + 0x34 + g_MenuCursorPos * 0x4c);

        for (cur_y = vp_y; cur_y < vp_y2; cur_y++) {
            fb_row = cur_y * g_ScreenWidth;
            for (cur_x = vp_x1; cur_x < vp_x2; cur_x++) {
                fb[fb_row + cur_x] = sky_color;
            }
        }

        if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
            p2_y = *(int *)(g_PlayerHUDState + 0x7c);
            p2_y2 = *(int *)(g_PlayerHUDState + 0x84);
            p2_x1 = *(int *)(g_PlayerHUDState + 0x78);
            p2_x2 = *(int *)(g_PlayerHUDState + 0x80);

            for (cur_y = p2_y; cur_y < p2_y2; cur_y++) {
                fb_row = cur_y * g_ScreenWidth;
                for (cur_x = p2_x1; cur_x < p2_x2; cur_x++) {
                    fb[fb_row + cur_x] = sky_color;
                }
            }
        }
    }

    /* 5. Player 1 camera calculation */
    veh = (VehicleState *)(g_Vehicles + g_MenuCursorPos * 0x484c);
    config = (VehicleConfig *)(g_VehicleConfigs + g_MenuCursorPos * 200);

    if (*(int *)((uint8_t *)veh + 0x5a4) == 0) {
        cam_pos_y = config->target_pos_x + config->cam_height;
        cam_pos_x = config->cam_pos_x;
        cam_pitch = config->pitch_offset + *(double *)((uint8_t *)config + 0x40) + config->chase_cam_dist_factor;
        cam_pos_z = config->cam_pos_z;
        cam_yaw = config->yaw_lag_angle;
    } else {
        cam_pos_y = config->target_pos_x + config->cam_height;
        cam_pos_x = config->cam_pos_y;
        cam_pitch = config->pitch_offset + *(double *)((uint8_t *)config + 0x40) + config->chase_cam_dist_factor;
        cam_pos_z = config->vehicle_yaw;
        cam_yaw = config->pitch_lag_angle;
    }

    if (g_SkyClearEnabled == 1) {
        cos_val = cos(veh->angle_yaw);
        sin_val = sin(veh->angle_yaw);
        cam_pos_y = veh->pos_y + 100.0;
        cam_pitch = 150.0;
        cam_pos_x = cos_val * (-150.0) + veh->pos_x;
        cam_pos_z = sin_val * (-150.0) + veh->pos_z;
        cam_yaw = veh->angle_yaw * (-572.95779513) + 900.0;
        while (cam_yaw < 0.0) {
            cam_yaw += 3600.0;
        }
        while (cam_yaw > 3599.0) {
            cam_yaw -= 3600.0;
        }
    }

    /* 6. Player 2 camera calculation (split-screen) */
    cam_p2_pos_x = 0.0;
    cam_p2_pos_y = 0.0;
    cam_p2_pos_z = 0.0;
    cam_p2_pitch = 0.0;
    cam_p2_yaw = 0.0;
    veh_p2 = (VehicleState *)(g_Vehicles + 0x484c);

    if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
        config_p2 = (VehicleConfig *)(g_VehicleConfigs + 200);

        if (*(int *)((uint8_t *)veh_p2 + 0x5a4) == 0) {
            cam_p2_pos_y = config_p2->target_pos_x + config_p2->cam_height;
            cam_p2_pos_x = config_p2->cam_pos_x;
            cam_p2_pitch = config_p2->pitch_offset + *(double *)((uint8_t *)config_p2 + 0x40) + config_p2->chase_cam_dist_factor;
            cam_p2_pos_z = config_p2->cam_pos_z;
            cam_p2_yaw = config_p2->yaw_lag_angle;
        } else {
            cam_p2_pos_y = config_p2->target_pos_x + config_p2->cam_height;
            cam_p2_pos_x = config_p2->cam_pos_y;
            cam_p2_pitch = config_p2->pitch_offset + *(double *)((uint8_t *)config_p2 + 0x40) + config_p2->chase_cam_dist_factor;
            cam_p2_pos_z = config_p2->vehicle_yaw;
            cam_p2_yaw = config_p2->pitch_lag_angle;
        }

        if (g_SkyClearEnabled == 1) {
            cos_val = cos(veh_p2->angle_yaw);
            sin_val = sin(veh_p2->angle_yaw);
            cam_p2_pos_y = veh_p2->pos_y + 100.0;
            cam_p2_pitch = 150.0;
            cam_p2_pos_x = cos_val * (-150.0) + veh_p2->pos_x;
            cam_p2_pos_z = sin_val * (-150.0) + veh_p2->pos_z;
            cam_p2_yaw = veh_p2->angle_yaw * (-572.95779513) + 900.0;
            while (cam_p2_yaw < 0.0) {
                cam_p2_yaw += 3600.0;
            }
            while (cam_p2_yaw > 3599.0) {
                cam_p2_yaw -= 3600.0;
            }
        }
    }

    /* 7. Screen shake timer updates */
    if (g_ScreenShakeTimerP1 > -1 && g_ScreenShakeTimerP1 < 120) {
        g_ScreenShakeTimerP1--;
    }
    if (g_ScreenShakeTimerP2 > -1 && g_ScreenShakeTimerP2 < 120) {
        g_ScreenShakeTimerP2--;
    }

    /* 8. Race timer advance and turbo countdown */
    if ((g_RaceTimer >= 0.0 && g_RaceTimer < 120.0) || g_PlayerCarChoice == 7) {
        g_RaceTimer += delta_time;
        if (veh->turbo_active == 1 && *(int *)((uint8_t *)veh + 0x5d4) < 25) {
            (*(int *)((uint8_t *)veh + 0x5d4))++;
        }
        if (g_IsSplitScreen == 1 && veh_p2->turbo_active == 1 && *(int *)((uint8_t *)veh_p2 + 0x5d4) < 25) {
            (*(int *)((uint8_t *)veh_p2 + 0x5d4))++;
        }
    } else if (g_RaceTimer >= 120.0 &&
               ((g_IsSplitScreen == 0 && veh->turbo_active == 1) ||
                (g_IsSplitScreen == 1 && veh->turbo_active == 1 && veh_p2->turbo_active == 1))) {
        /* Post-race fly-by cameras */
        if (g_CinematicCameraAngleState == 3) {
            g_CinematicFocusedVehicle = Race_FindFocusedVehicle();
            if (g_NumRacers - g_CinematicFocusedVehicle == -1) {
                g_CinematicCameraAngleState = 0;
                g_MenuCursorPos = 0;
                track_waypoint_idx = g_CurrentTrackIndex * 15;
                cam_pos_x = g_TrackFlybyCameras[track_waypoint_idx + 0];
                cam_pos_z = g_TrackFlybyCameras[track_waypoint_idx + 2];
                cam_pos_y = sin((double)g_GlobalFrameCount * 0.05) * 8.0 + g_TrackFlybyCameras[track_waypoint_idx + 1];
                cam_pitch = g_TrackFlybyCameras[track_waypoint_idx + 3] * 57.2957795;
                cam_yaw = g_TrackFlybyCameras[track_waypoint_idx + 4] * 57.2957795;

                if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
                    cam_p2_pos_x = cam_pos_x;
                    cam_p2_pos_y = cam_pos_y;
                    cam_p2_pos_z = cam_pos_z;
                    cam_p2_pitch = cam_pitch;
                    cam_p2_yaw = cam_yaw;
                }
            } else {
                VehicleConfig *focus_cfg = (VehicleConfig *)(g_VehicleConfigs + g_MenuCursorPos * 200);
                cam_pos_y = focus_cfg->cam_param1 + focus_cfg->cam_height;
                cam_pos_x = focus_cfg->cam_pos_x;
                cam_pos_z = focus_cfg->cam_pos_z;
                cam_pitch = focus_cfg->pitch_offset + *(double *)((uint8_t *)focus_cfg + 0x40) + focus_cfg->chase_cam_dist_factor;
                cam_yaw = focus_cfg->pitch_lag_angle;
            }
        } else {
            track_waypoint_idx = (g_CurrentTrackIndex * 3 + g_CinematicCameraAngleState) * 5;
            cam_pos_x = g_TrackFlybyCameras[track_waypoint_idx + 0];
            cam_pos_z = g_TrackFlybyCameras[track_waypoint_idx + 2];
            cam_pos_y = sin((double)g_GlobalFrameCount * 0.05) * 8.0 + g_TrackFlybyCameras[track_waypoint_idx + 1];
            cam_pitch = g_TrackFlybyCameras[track_waypoint_idx + 3] * 57.2957795;
            cam_yaw = g_TrackFlybyCameras[track_waypoint_idx + 4] * 57.2957795;

            if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
                int p2_waypoint = g_CurrentTrackIndex * 15;
                cam_p2_pos_x = g_TrackFlybyCameras[p2_waypoint + 0];
                cam_p2_pos_z = g_TrackFlybyCameras[p2_waypoint + 2];
                cam_p2_pos_y = sin((double)g_GlobalFrameCount * 0.05) * 8.0 + g_TrackFlybyCameras[p2_waypoint + 1];
                cam_p2_pitch = g_TrackFlybyCameras[p2_waypoint + 3] * 57.2957795;
                cam_p2_yaw = g_TrackFlybyCameras[p2_waypoint + 4] * 57.2957795;
            }
        }
    }

    /* 9. Debug telemetry camera coordinates override */
    if (g_ShowRollTelemetry == 1) {
        cam_pos_x = g_CameraPosX;
        cam_pos_y = g_CameraPosY;
        cam_pos_z = g_CameraPosZ;
        cam_pitch = g_CameraAngleX * 572.957795;
        cam_yaw = g_CameraAngleY * 572.957795;
    }

    /* 10. Load camera matrix and parameters for Player 1 */
    *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 0;
    g_pActiveCamera[0] = cam_pos_x;
    g_pActiveCamera[1] = cam_pos_y;
    g_pActiveCamera[2] = cam_pos_z;
    g_pActiveCamera[3] = cam_pitch;
    g_pActiveCamera[4] = cam_yaw;
    g_pActiveCamera[5] = 0.0;
    *(int *)((uint8_t *)g_pActiveCamera + 0x7c) = 0;
    *(int *)((uint8_t *)g_pActiveCamera + 0x80) = config->viewport_h;
    *(int *)((uint8_t *)g_pActiveCamera + 0x84) = config->viewport_w;
    *(int *)((uint8_t *)g_pActiveCamera + 0x88) = config->viewport_h;

    if (g_IsSplitScreen == 0) {
        *(int *)((uint8_t *)g_pActiveCamera + 0x9c) = g_ScreenWidth / 2;
    } else if (g_IsSplitScreen == 1) {
        *(int *)((uint8_t *)g_pActiveCamera + 0x9c) = (g_ScreenWidth * 3) / 4;
    }
    if (g_SplitScreenMode == 1) {
        *(int *)((uint8_t *)g_pActiveCamera + 0x9c) = g_ScreenWidth / 2;
    }

    if (g_ShowRollTelemetry == 1) {
        *(int *)((uint8_t *)g_pActiveCamera + 0x7c) = g_TelemetryRollAngle;
        if (g_ShowRollTelemetry == 1) {
            *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 1;
            *(int *)((uint8_t *)g_pActiveCamera + 0x70) = (int)cam_pos_x;
            *(int *)((uint8_t *)g_pActiveCamera + 0x74) = (int)cam_pos_y;
            *(int *)((uint8_t *)g_pActiveCamera + 0x78) = (int)cam_pos_z;
        }
        if (g_ShowRollTelemetry == 2) {
            VehicleState *target_v = (VehicleState *)(g_Vehicles + g_TrackFlybyActiveTarget * 0x484c);
            if (target_v->crash_flag1 == 0 && target_v->crash_flag2 == 0 && target_v->crash_flag3 == 0) {
                *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 1;
                g_pActiveCamera[0] = target_v->pos_x + g_FlybyCameraOffsetX;
                g_CameraPosX_Int = (int)g_pActiveCamera[0];
                g_pActiveCamera[1] = target_v->pos_y + g_FlybyCameraOffsetY;
                g_CameraPosY_Int = (int)g_pActiveCamera[1];
                g_pActiveCamera[2] = target_v->pos_z + g_FlybyCameraOffsetZ;
                g_CameraPosZ_Int = (int)g_pActiveCamera[2];

                *(int *)((uint8_t *)g_pActiveCamera + 0x70) = (int)g_pActiveCamera[0];
                *(int *)((uint8_t *)g_pActiveCamera + 0x74) = (int)g_pActiveCamera[1];
                *(int *)((uint8_t *)g_pActiveCamera + 0x78) = (int)g_pActiveCamera[2];
                g_FlybyLastCameraPosX = (int)g_pActiveCamera[0];
                g_FlybyLastCameraPosY = (int)g_pActiveCamera[1];
                g_FlybyLastCameraPosZ = (int)g_pActiveCamera[2];
                g_FlybyLastYaw = *(int *)((uint8_t *)g_pActiveCamera + 0x70);
                g_FlybyLastPitch = *(int *)((uint8_t *)g_pActiveCamera + 0x70);
                g_FlybyLastRoll = *(int *)((uint8_t *)g_pActiveCamera + 0x70);
            } else {
                *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 1;
                g_pActiveCamera[0] = (double)g_FlybyLastCameraPosX;
                g_pActiveCamera[1] = (double)g_FlybyLastCameraPosY;
                g_pActiveCamera[2] = (double)g_FlybyLastCameraPosZ;
                *(int *)((uint8_t *)g_pActiveCamera + 0x70) = g_FlybyLastYaw;
                *(int *)((uint8_t *)g_pActiveCamera + 0x74) = g_FlybyLastPitch;
                *(int *)((uint8_t *)g_pActiveCamera + 0x78) = g_FlybyLastRoll;
            }
        }
    }

    /* 11. Ghost replay car movement */
    if (g_RaceTimer_P2 >= 60.0 && g_RaceTimer_P2 < 130.0) {
        ghost_transform = &g_CarGhostTransforms[g_NumRacers];
        if (((int)g_RaceTimer_P2 & 1) == 0 && ghost_transform->is_placed == 0) {
            Lisa_MoveDynamicObject(ghost_transform);
        }
    }

    /* 12. Submit rasterizer draw queues and clip rects for Player 1 */
    g_pLisaCommandQueueMirror = g_pLisaDrawCommandQueue;
    g_pLisaFramebufferMirror1 = &g_VirtualFramebuffer;
    g_pLisaFramebufferMirror2 = &g_VirtualFramebuffer;
    g_pLisaActiveTABMirror = (void *)g_pActiveTAB;

    g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x2c + g_MenuCursorPos * 0x4c);
    g_ViewportMinY = *(int *)(g_PlayerHUDState + 0x30 + g_MenuCursorPos * 0x4c);
    g_ViewportMaxX = *(int *)(g_PlayerHUDState + 0x34 + g_MenuCursorPos * 0x4c) - 1;
    g_ViewportMaxY = *(int *)(g_PlayerHUDState + 0x38 + g_MenuCursorPos * 0x4c) - 1;

    node_idx = *(int *)((uint8_t *)veh + 0x364);
    if (node_idx < 0) {
        chunk_idx = *(int *)(g_pTrackRoadSequence + 0xc + (-node_idx) * 0x18);
    } else {
        chunk_idx = *(int *)(g_pTrackRoadSequence + node_idx * 0x18);
    }

    if (g_RaceTimer >= 120.0) {
        g_ActiveTrackSegmentAttribute = 0;
    } else {
        g_ActiveTrackSegmentAttribute = g_TrackSegmentTable[chunk_idx * 3];
    }

    Lisa_RenderScene();
    Lisa_FlushRasterizerCommands(0, 0);

    ghost_transform = &g_CarGhostTransforms[g_NumRacers];
    if (ghost_transform->is_placed == 1) {
        Lisa_DeleteDynamicObject(ghost_transform);
    }

    if ((veh->turbo_active == 0 || g_RaceTimer <= 120.0) && g_ShowRollTelemetry == 0) {
        HUD_RenderPlayerElements(g_MenuCursorPos);
    }

    /* 13. Split-screen Player 2 rendering */
    if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
        *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 0;
        g_pActiveCamera[0] = cam_p2_pos_x;
        g_pActiveCamera[1] = cam_p2_pos_y;
        g_pActiveCamera[2] = cam_p2_pos_z;
        g_pActiveCamera[3] = cam_p2_pitch;
        g_pActiveCamera[4] = cam_p2_yaw;
        g_pActiveCamera[5] = 0.0;
        *(int *)((uint8_t *)g_pActiveCamera + 0x7c) = 0;
        *(int *)((uint8_t *)g_pActiveCamera + 0x80) = *(int *)(g_VehicleConfigs + 0x120);
        *(int *)((uint8_t *)g_pActiveCamera + 0x84) = *(int *)(g_VehicleConfigs + 0x124);
        *(int *)((uint8_t *)g_pActiveCamera + 0x88) = *(int *)(g_VehicleConfigs + 0x120);
        *(int *)((uint8_t *)g_pActiveCamera + 0x9c) = g_ScreenWidth / (g_IsSplitScreen * 2 + 2);

        g_pLisaCommandQueueMirror = g_pLisaDrawCommandQueue;
        g_pLisaFramebufferMirror1 = &g_VirtualFramebuffer;
        g_pLisaFramebufferMirror2 = &g_VirtualFramebuffer;
        g_pLisaActiveTABMirror = (void *)g_pActiveTAB;

        g_ViewportMinX = *(int *)(g_PlayerHUDState + 0x78);
        g_ViewportMinY = *(int *)(g_PlayerHUDState + 0x7c);
        g_ViewportMaxX = *(int *)(g_PlayerHUDState + 0x80) - 1;
        g_ViewportMaxY = *(int *)(g_PlayerHUDState + 0x84) - 1;

        if (g_RaceTimer_P2 >= 60.0 && g_RaceTimer_P2 < 130.0) {
            ghost_transform_p2 = &g_CarGhostTransforms[g_NumRacers + 1];
            if (((int)g_RaceTimer_P2 & 1) == 0 && ghost_transform_p2->is_placed == 0) {
                Lisa_MoveDynamicObject(ghost_transform_p2);
            }
        }

        node_idx = *(int *)(g_Vehicles + 0x4bb0);
        if (node_idx < 0) {
            chunk_idx = *(int *)(g_pTrackRoadSequence + 0xc + (-node_idx) * 0x18);
        } else {
            chunk_idx = *(int *)(g_pTrackRoadSequence + node_idx * 0x18);
        }

        g_ActiveTrackSegmentAttribute = g_TrackSegmentTable[chunk_idx * 3];

        Lisa_RenderScene();
        Lisa_FlushRasterizerCommands(0, 0);

        ghost_transform_p2 = &g_CarGhostTransforms[g_NumRacers + 1];
        if (ghost_transform_p2->is_placed == 1) {
            Lisa_DeleteDynamicObject(ghost_transform_p2);
        }

        if (veh_p2->turbo_active == 0 || g_RaceTimer <= 120.0) {
            HUD_RenderPlayerElements(1);
        }
    }

    /* 14. Weather, Panorama, Results, and Borders */
    FX_SpawnWeather();

    if (g_RaceTimer_P2 >= 0.0 && g_RaceTimer_P2 < 100.0) {
        Lisa_RenderPanorama();
    }

    if (((g_IsSplitScreen == 0 && veh->turbo_active == 1) ||
         (g_IsSplitScreen == 1 && veh->turbo_active == 1 && veh_p2->turbo_active == 1)) &&
        g_RaceTimer >= 120.0 && g_GamePauseState == 0 && g_ShowRollTelemetry == 0) {
        HUD_RenderTrackResults();
    }

    screen_w = *(int *)(g_PlayerHUDState + 0x34);
    screen_h = *(int *)(g_PlayerHUDState + 0x38);

    if (g_ScreenSizeSetting == 0 && (g_IsSplitScreen == 0 || g_SplitScreenMode == 1)) {
        border_pos[0] = 0;
        border_pos[1] = 0;
        Gfx_DrawSprite(g_ViewportBorderCornerTL, border_pos, 0);

        border_pos[0] = (screen_w - g_ViewportBorderMetrics[2]) * 256;
        border_pos[1] = 0;
        Gfx_DrawSprite(g_ViewportBorderCornerTR, border_pos, 0);

        border_pos[0] = 0;
        border_pos[1] = (screen_h - g_ViewportBorderMetrics[3]) * 256;
        Gfx_DrawSprite(g_ViewportBorderCornerBL, border_pos, 0);

        border_pos[0] = (screen_w - g_ViewportBorderMetrics[2]) * 256;
        border_pos[1] = (screen_h - g_ViewportBorderMetrics[3]) * 256;
        Gfx_DrawSprite(g_ViewportBorderCornerBR, border_pos, 0);
    }

    if (g_GamePauseState == 1) {
        if (g_MasterMusicVolume < 1) {
            Ghost_SaveGhostData();
            g_PostRaceSequenceState = 2;
        }
        Lisa_RenderPanorama();
    }

    if (g_IsSplitScreen == 1 && g_SplitScreenMode == 0) {
        divider_x = g_ScreenWidth / 2;
        for (y = 0; y < g_ScreenHeight; y++) {
            fb[y * g_ScreenWidth + divider_x - 1] = 0;
            fb[y * g_ScreenWidth + divider_x] = 0;
        }
    }
}

/**
 * @original FX_UpdateWeatherBounds (IGN_WIN.EXE @ 0x00434580, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateWeatherBounds(void) {
    int node_idx;

    if (g_WeatherType == 1) {
        if (g_CurrentTrackIndex == 0) {
            node_idx = *(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c);
            if (node_idx > 45 && node_idx < 120) {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 1;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 1;
                }
            } else {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 0;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 0;
                }
            }
        }
    } else if (g_WeatherType == 3) {
        if (g_CurrentTrackIndex == 1) {
            node_idx = abs(*(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c));
            if (node_idx > 0 && node_idx < 30) {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 1;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 1;
                }
            } else {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 0;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 0;
                }
            }
        }
    } else if (g_WeatherType == 2) {
        if (g_CurrentTrackIndex == 4) {
            node_idx = abs(*(int *)(g_Vehicles + 0x364 + g_ActiveVehicleIndex * 0x484c));
            if (node_idx > 28 && node_idx < 100) {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 1;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 1;
                }
            } else {
                if (g_ActiveVehicleIndex == 0) {
                    g_WeatherActive[0] = 0;
                } else if (g_ActiveVehicleIndex == 1 && g_IsSplitScreen == 1) {
                    g_WeatherActive[1] = 0;
                }
            }
        }
    }
}
/**
 * @original FX_UpdateVehicleWreck (IGN_WIN.EXE @ 0x004307b0, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateVehicleWreck(int *wreck) {
  int *road_seq_ptr;
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
  char *voice_ptr;
  double *veh_ptr;
  int temp_val1;
  int *car_state_ptr;
  int temp_val2;
  double *other_veh_ptr;
  int veh_offset;
  int *spline_node_ptr;
  int obj_offset;
  unsigned int *model_data;
  int timer2;
  int timer1;
  int race_fin_offset;
  double trig_factor;
  __int64 ftol_result;
  double respawn_heading;
  int temp_i1;
  int world_pos_x_accum;
  int world_pos_z_accum;
  double heading_accum;
  int vertex_count_accum;
  int world_pos_y_accum;
  veh_ptr = g_Vehicles;
  world_pos_y_accum = 0;
  world_pos_x_accum = 0;
  pos_x = wreck[1];
  pos_y = wreck[2];
  world_pos_z_accum = 0;
  vertex_count_accum = 0;
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
    car_state_ptr = (int *)((int)g_Vehicles + veh_offset);
    car_state_ptr[0xe2] = car_state_ptr[1];
    car_state_ptr[0xe1] = *car_state_ptr;
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
      voice_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + veh_offset + 0x99c));
      if (voice_ptr != (char *)0x0) {
        *(int *)(voice_ptr + 0x10) = 0;
      }
      *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
    }
    if (*(int *)((int)g_Vehicles + veh_offset + 0x29c) == 1) {
      voice_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + veh_offset + 0x9a0));
      if (voice_ptr != (char *)0x0) {
        *(int *)(voice_ptr + 0x10) = 0;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
    g_ActiveParticle.rot_y = &g_WheelParticleDescriptor;
    g_ActiveParticle.rot_z = 3;
    g_ActiveParticle.rot_x = 0xffffffc4;
    ftol_result = __ftol();
    g_ActiveParticle.field_3c = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_40 = (int)ftol_result;
    ftol_result = __ftol();
    g_ActiveParticle.field_44 = (int)ftol_result;
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
      ftol_result = __ftol();
      g_ActiveParticle.pos_z = (int)ftol_result;
      Math_RandomFloat0To1();
      ftol_result = __ftol();
      g_ActiveParticle.vel_x = (int)ftol_result;
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
  if (obj_offset + 2 < g_TrackRoadSequenceNodeCount) {
    if (-1 < (int)rot_x) {
      respawn_heading = *(double *)(g_pTrackRoadSequence + rot_x * 6 + 0xd);
      race_fin_offset = g_pTrackRoadSequence[rot_x * 6 + 0xc];
      goto joined_r0x004317bd;
    }
    respawn_heading = *(double *)(g_pTrackRoadSequence + obj_offset * 6 + 0x10) - k_WreckRespawnSplineOffset;
    spline_node_ptr = g_pTrackRoadSequence + obj_offset * 6 + 0xf;
    race_fin_offset = *spline_node_ptr;
    if (respawn_heading < k_WreckRespawnAngleMin) {
      respawn_heading = respawn_heading + k_WreckRespawnAngleModulo;
    }
    if (race_fin_offset == 10000) {
      respawn_heading = *(double *)(g_pTrackRoadSequence + rot_x * 6 + 0xd) - k_WreckRespawnSplineOffset;
      race_fin_offset = g_pTrackRoadSequence[rot_x * 6 + 0xc];
      if (respawn_heading < k_WreckRespawnAngleMin) {
        respawn_heading = respawn_heading + k_WreckRespawnAngleModulo;
      }
    }
    if (race_fin_offset == -2) {
      race_fin_offset = 0;
      do {
        road_seq_ptr = spline_node_ptr + 6;
        spline_node_ptr = spline_node_ptr + 6;
        race_fin_offset = race_fin_offset + 1;
      } while (*road_seq_ptr == -2);
      respawn_heading = *(double *)(g_pTrackRoadSequence + (race_fin_offset + obj_offset) * 6 + 0xd);
      race_fin_offset = g_pTrackRoadSequence[(race_fin_offset + obj_offset) * 6 + 0xc];
      goto joined_r0x004317bd;
    }
  }
  else {
    respawn_heading = *(double *)(g_pTrackRoadSequence + 1);
    race_fin_offset = *g_pTrackRoadSequence;
joined_r0x004317bd:
    respawn_heading = respawn_heading - k_WreckRespawnSplineOffset;
    if (respawn_heading < k_WreckRespawnAngleMin) {
      respawn_heading = respawn_heading + k_WreckRespawnAngleModulo;
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
        world_pos_y_accum = world_pos_y_accum +
                   *(int *)(obj_offset + 8 + rot_x * 0xc) + (*(int *)(temp_val2 + 0xc) + 0x6400) * 3 +
                   *(int *)(obj_offset + 8 + rot_y * 0xc) + *(int *)(obj_offset + 8 + rot_z * 0xc);
        world_pos_x_accum = (((((world_pos_x_accum - *(int *)(obj_offset + 0xc + rot_x * 0xc)) + race_fin_offset) -
                     *(int *)(obj_offset + 0xc + rot_y * 0xc)) + race_fin_offset) -
                   *(int *)(obj_offset + 0xc + rot_z * 0xc)) + race_fin_offset;
        vertex_count_accum = vertex_count_accum + 3;
        world_pos_z_accum = world_pos_z_accum +
                   *(int *)(obj_offset + 0x10 + rot_x * 0xc) + (*(int *)(temp_val2 + 0x14) + 0x6400) * 3 +
                   *(int *)(obj_offset + 0x10 + rot_y * 0xc) + *(int *)(obj_offset + 0x10 + rot_z * 0xc);
      }
      model_data = model_data + 0xb;
      temp_val1 = temp_val1 + -1;
    } while (temp_val1 != 0);
  }
  heading_accum = respawn_heading;
  if (respawn_heading < 0.0) {
    heading_accum = respawn_heading + k_WreckRespawnAngleModulo;
  }
  velocity = (double)((timer1 - timer2) + 0x28);
  trig_factor = Math_LookupTrigAngle(velocity);
  *veh_ptr = (double)(((double)(world_pos_y_accum / vertex_count_accum) - (double)*(double *)((int)veh_ptr + 900)) *
                      trig_factor + (double)*(double *)((int)veh_ptr + 900));
  *(double *)((int)g_Vehicles + veh_offset + 8) =
       ((double)(world_pos_x_accum / vertex_count_accum + 0xfa) - *(double *)((int)g_Vehicles + veh_offset + 0x38c)) *
       velocity * k_WreckRespawnInterpolationScale + *(double *)((int)g_Vehicles + veh_offset + 0x38c);
  veh_ptr = g_Vehicles;
  trig_factor = Math_LookupTrigAngle(velocity);
  speed = heading_accum - k_WreckRespawnAngleModulo;
  *(double *)((int)veh_ptr + veh_offset + 0x10) =
       (double)(((double)(world_pos_z_accum / vertex_count_accum) -
                (double)*(double *)((int)veh_ptr + veh_offset + 0x394)) * trig_factor +
               (double)*(double *)((int)veh_ptr + veh_offset + 0x394));
  if (ABS(speed - *(double *)((int)g_Vehicles + veh_offset + 0x37c)) <
      ABS(heading_accum - *(double *)((int)g_Vehicles + veh_offset + 0x37c))) {
    heading_accum = speed;
  }
  *(double *)((int)g_Vehicles + veh_offset + 0xf8) =
       (heading_accum - *(double *)((int)g_Vehicles + veh_offset + 0x37c)) * velocity * k_WreckRespawnInterpolationScale +
       *(double *)((int)g_Vehicles + veh_offset + 0x37c);
  veh_ptr = (double *)((int)g_Vehicles + veh_offset + 0xf8);
  if (k_WreckRespawnHeadingMax < *(double *)((int)g_Vehicles + veh_offset + 0xf8)) {
    *veh_ptr = *veh_ptr - k_WreckRespawnAngleModulo;
  }
  veh_ptr = (double *)((int)g_Vehicles + veh_offset + 0xf8);
  if (*(double *)((int)g_Vehicles + veh_offset + 0xf8) < k_WreckRespawnHeadingMin) {
    *veh_ptr = *veh_ptr + k_WreckRespawnAngleModulo;
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
              (other_veh_ptr = (double *)((int)g_Vehicles + veh_offset),
              ABS(*veh_ptr - *other_veh_ptr) < k_WreckRespawnVehicleClearance)) &&
             ((ABS(veh_ptr[1] - other_veh_ptr[1]) < k_WreckRespawnVehicleClearance &&
              ((ABS(veh_ptr[2] - other_veh_ptr[2]) < k_WreckRespawnVehicleClearance &&
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
    int v_pos_y;
    int v_pos_x;
    int v_pos_z;
    int v_norm_x;
    int v_norm_y;
    int v_norm_z;
    int v_tang_x;
    int v_tang_y;
    int v_tang_z;
    int v_extra_a;
    int v_extra_b;
    int uv_anim_table;
    int submesh_idx;
    int world_pos_y;
    int world_pos_z;
    int texture_page;
    int roll_speed;
    int max_age;
    int *transform_ptr;
    int roll_angle;
    int world_pos_x;
    int age;
    int mesh_stride;

    v_pos_y = wheel[2];
    v_pos_x = wheel[1];
    v_pos_z = wheel[3];
    v_norm_x = wheel[4];
    v_norm_y = wheel[5];
    v_norm_z = wheel[6];
    v_tang_x = wheel[7];
    v_tang_y = wheel[8];
    v_tang_z = wheel[9];
    v_extra_a = wheel[10];
    v_extra_b = wheel[0xb];
    uv_anim_table = wheel[0xd];
    submesh_idx = wheel[0xc];
    world_pos_x = wheel[0xf];
    world_pos_y = wheel[0x10];
    world_pos_z = wheel[0x11];
    texture_page = wheel[0xe];
    roll_angle = wheel[0x12];
    roll_speed = wheel[0x13];
    max_age = wheel[0x15];
    age = wheel[0x14];

    if (age == 0) {
        mesh_stride = wheel_index * 800;
        *(int *)((int)&g_DebrisDynamicMeshes + mesh_stride) = 4;
        *(int *)((int)&g_DebrisDynamicMeshes + 4 + mesh_stride) = 2;
        *(int *)((int)&g_DebrisDynamicMeshes + 8 + mesh_stride) = v_pos_x;
        *(int *)((int)&g_DebrisDynamicMeshes + 0xc + mesh_stride) = v_pos_y;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x10 + mesh_stride) = v_pos_z;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x14 + mesh_stride) = v_norm_x;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x18 + mesh_stride) = v_norm_y;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x1c + mesh_stride) = v_norm_z;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x20 + mesh_stride) = v_tang_x;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x24 + mesh_stride) = v_tang_y;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x28 + mesh_stride) = v_tang_z;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x2c + mesh_stride) = v_extra_a;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x30 + mesh_stride) = v_extra_b;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x34 + mesh_stride) = submesh_idx;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x38 + mesh_stride) = 0x13;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x3c + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x40 + mesh_stride) = 1;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x44 + mesh_stride) = 2;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x60 + mesh_stride) = texture_page;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x64 + mesh_stride) = 0x13;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x68 + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x6c + mesh_stride) = 2;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x70 + mesh_stride) = 3;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x8c + mesh_stride) = texture_page;
        Lisa_SetDynamicObjectMesh(g_pFXTransforms, submesh_idx, (int *)(wheel_index * 0x20 + g_pFXTransforms),
                                 (int *)((int)&g_DebrisDynamicMeshes + mesh_stride), texture_page, 200, 0, 0x14, 0);
    }

    mesh_stride = wheel_index * 0x20;
    *(int *)(mesh_stride + 4 + g_pFXTransforms) = (int)(world_pos_x + (world_pos_x >> 0x1f & 0x3ffU)) >> 10;
    *(int *)(mesh_stride + 8 + g_pFXTransforms) = (int)(world_pos_y + (world_pos_y >> 0x1f & 0x3ffU)) >> 10;
    *(int *)(mesh_stride + 0xc + g_pFXTransforms) = (int)(world_pos_z + (world_pos_z >> 0x1f & 0x3ffU)) >> 10;
    *(int *)(mesh_stride + 0x10 + g_pFXTransforms) = 0;
    *(int *)(mesh_stride + 0x14 + g_pFXTransforms) = 0;
    *(int *)(mesh_stride + 0x18 + g_pFXTransforms) = 0;

    transform_ptr = (int *)(g_pFXTransforms + mesh_stride);
    if (transform_ptr[7] == 0) {
        Lisa_MoveDynamicObject(transform_ptr);
    } else {
        if (Lisa_UpdateObjectSpatialGrid(transform_ptr) != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_WATER_SPLA_00499468);
        }
    }

    roll_angle = roll_angle + roll_speed;
    mesh_stride = wheel_index * 800;
    transform_ptr = (int *)(uv_anim_table + ((int)(roll_angle + (roll_angle >> 0x1f & 0x3ffU)) >> 10) * 0x10);

    *(int *)((int)&g_DebrisDynamicMeshes + 0x48 + mesh_stride) = *transform_ptr << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x4c + mesh_stride) = transform_ptr[3] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x50 + mesh_stride) = *transform_ptr << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x54 + mesh_stride) = transform_ptr[1] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x58 + mesh_stride) = transform_ptr[2] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x5c + mesh_stride) = transform_ptr[1] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x74 + mesh_stride) = *transform_ptr << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x78 + mesh_stride) = transform_ptr[3] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x7c + mesh_stride) = transform_ptr[2] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x80 + mesh_stride) = transform_ptr[1] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x84 + mesh_stride) = transform_ptr[2] << 8;
    *(int *)((int)&g_DebrisDynamicMeshes + 0x88 + mesh_stride) = transform_ptr[3] << 8;

    age = age + 1;
    if (max_age <= age) {
        Lisa_DeleteDynamicObject((int *)(wheel_index * 0x20 + g_pFXTransforms));
        *wheel = 0;
    }
    wheel[0x14] = age;
    wheel[0x12] = roll_angle;
}

/**
 * @original FX_UpdateVehicleCrashSequence (IGN_WIN.EXE @ 0x00432040, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateVehicleCrashSequence(int *crash_seq) {
    int *road_seq_ptr;
    double tumble_delta;
    int crash_pos_x;
    int crash_pos_y;
    int crash_pos_z;
    int car_idx;
    unsigned int vi0;
    unsigned int vi1;
    unsigned int vi2;
    double current_x;
    int is_occupied;
    char *voice_ptr;
    int *car_state_ptr;
    int msh_entry_offset;
    int car_offset;
    int poly_count;
    double *veh_ptr;
    unsigned int *poly_ptr;
    int plc_model_idx;
    int *spline_ptr;
    int total_ticks;
    int world_pos_y_accum;
    int crash_tick;
    double trig_factor;
    __int64 ftol_result;
    double respawn_heading;
    int final_crash_tick;
    int world_pos_z_accum;
    double heading_accum;
    int vertex_count_accum;
    int world_pos_x_accum;

    world_pos_y_accum = 0;
    world_pos_x_accum = 0;
    world_pos_z_accum = 0;
    vertex_count_accum = 0;

    crash_pos_x = crash_seq[1];
    crash_pos_y = crash_seq[2];
    crash_pos_z = crash_seq[3];
    car_idx = crash_seq[4];
    crash_tick = crash_seq[5];
    total_ticks = crash_seq[6];

    car_offset = car_idx * 0x484c;
    *(int *)((int)g_Vehicles + car_offset + 0x534) = 0;
    *(int *)((int)g_Vehicles + car_offset + 0x538) = 0x40590000;
    *(int *)((int)g_Vehicles + car_offset + 0x53c) = 0;

    if (crash_tick == 0) {
        car_state_ptr = (int *)(car_offset + (int)g_Vehicles);
        car_state_ptr[0xe2] = car_state_ptr[1];
        car_state_ptr[0xe1] = *car_state_ptr;
        *(int *)((int)g_Vehicles + car_offset + 0x390) = *(int *)((int)g_Vehicles + car_offset + 0xc);
        *(int *)((int)g_Vehicles + car_offset + 0x38c) = *(int *)((int)g_Vehicles + car_offset + 8);
        *(int *)((int)g_Vehicles + car_offset + 0x398) = *(int *)((int)g_Vehicles + car_offset + 0x14);
        *(int *)((int)g_Vehicles + car_offset + 0x394) = *(int *)((int)g_Vehicles + car_offset + 0x10);
        *(int *)((int)g_Vehicles + car_offset + 0x380) = *(int *)((int)g_Vehicles + car_offset + 0xfc);
        *(int *)((int)g_Vehicles + car_offset + 0x37c) = *(int *)((int)g_Vehicles + car_offset + 0xf8);
        *(int *)((int)g_Vehicles + car_offset + 0x270) = 1;

        if (*(int *)((int)g_Vehicles + car_offset + 0x298) == 1) {
            voice_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + car_offset + 0x99c));
            if (voice_ptr != (char *)0x0) {
                *(int *)(voice_ptr + 0x10) = 0;
            }
            *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x298) = 0;
        }
        if (*(int *)((int)g_Vehicles + car_offset + 0x29c) == 1) {
            voice_ptr = Audio_GetVoice(*(int *)((int)g_Vehicles + car_offset + 0x9a0));
            if (voice_ptr != (char *)0x0) {
                *(int *)(voice_ptr + 0x10) = 0;
            }
            *(int *)((int)g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x29c) = 0;
        }
    }

    if (crash_tick < 0x1f) {
        int tumble_steps = 0x1e - crash_tick;
        if (10 < tumble_steps) {
            tumble_steps = 10;
        }
        tumble_delta = (double)tumble_steps;
        *(double *)((int)g_Vehicles + car_offset + 900) += tumble_delta;
        *(double *)((int)g_Vehicles + car_offset + 0x394) += tumble_delta;
        *(double *)((int)g_Vehicles + car_offset) += tumble_delta;

        if (crash_tick < 10) {
            current_x = *(double *)((int)g_Vehicles + car_offset + 8) + k_CrashElevationStepUp;
        } else {
            current_x = *(double *)((int)g_Vehicles + car_offset + 8) - k_CrashElevationStepDown;
        }
        *(double *)((int)g_Vehicles + car_offset + 8) = current_x;
        *(double *)((int)g_Vehicles + car_offset + 0x10) += tumble_delta;
        *(double *)((int)g_Vehicles + car_offset + 0x108) += k_CrashTumblePitchDelta;
        *(double *)((int)g_Vehicles + car_offset + 0x110) += k_CrashTumbleRollDelta;

        trig_factor = Math_RandomFloat0To1();
        if (trig_factor == (double)k_CrashSoundRandomChance) {
            __ftol();
            __ftol();
            Audio_PlaySampleVol(0, 2, 1, 0, 0x10000, 22000, 0);
        }

        if (crash_tick == 0x1e) {
            int entity_stride = car_idx * 0x20;
            if ((((int *)(g_pCarTransforms + entity_stride))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
                Lisa_DeleteDynamicObject((int *)(g_pCarTransforms + entity_stride));
            }
            int wheel_stride = car_idx * 0x80;
            if (((int *)(g_pWheelTransforms + wheel_stride))[7] != 0) {
                Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + wheel_stride));
            }
            if (*(int *)(g_pWheelTransforms + wheel_stride + 0x3c) != 0) {
                Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + wheel_stride + 0x20));
            }
            if (*(int *)(g_pWheelTransforms + wheel_stride + 0x5c) != 0) {
                Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + wheel_stride + 0x40));
            }
            if (*(int *)(g_pWheelTransforms + wheel_stride + 0x7c) != 0) {
                Lisa_DeleteDynamicObject((int *)(g_pWheelTransforms + wheel_stride + 0x60));
            }
            if ((((int *)(g_pCarShadowTransforms + entity_stride))[7] != 0) && (g_DynamicObjectsPaused == 0)) {
                Lisa_DeleteDynamicObject((int *)(g_pCarShadowTransforms + entity_stride));
            }
            if (((*(int *)(g_PlayerHUDState + car_idx * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
               (((int *)(entity_stride + g_pCarReflectionTransforms))[7] != 0)) {
                Lisa_DeleteDynamicObject((int *)(entity_stride + g_pCarReflectionTransforms));
            }
        }
    }

    if (total_ticks - crash_tick == 0x14) {
        int debris_count = 10;
        do {
            g_ActiveParticle.type = 0xb;
            g_ActiveParticle.pos_y = 0;
            g_ActiveParticle.pos_x = car_idx;
            Math_RandomFloat0To1();
            ftol_result = __ftol();
            g_ActiveParticle.pos_z = (int)ftol_result;
            Math_RandomFloat0To1();
            ftol_result = __ftol();
            g_ActiveParticle.vel_x = (int)ftol_result;
            g_ActiveParticle.vel_y = 0;
            g_ActiveParticle.vel_z = 0;
            g_ActiveParticle.drag = 0x2800;
            g_ActiveParticle.field_24 = 0x3c;
            g_ActiveParticle.gravity = 0;
            FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
            debris_count = debris_count - 1;
        } while (debris_count != 0);
        __ftol();
        __ftol();
        Audio_PlaySampleVol(0, 4, 3, 0, 0xe000, 0, 0);
    }

    if (crash_tick <= total_ticks - 0x28) {
        goto LAB_CRASH_NEXT_TICK;
    }

    unsigned int road_node = *(unsigned int *)((int)g_Vehicles + car_offset + 0x364);
    int node_abs = (road_node ^ (int)road_node >> 0x1f) - ((int)road_node >> 0x1f);

    if (node_abs + 2 < g_TrackRoadSequenceNodeCount) {
        if (-1 < (int)road_node) {
            respawn_heading = *(double *)(g_pTrackRoadSequence + road_node * 6 + 0xd);
            plc_model_idx = g_pTrackRoadSequence[road_node * 6 + 0xc];
            goto LAB_RESPAWN_WRAP;
        }
        respawn_heading = *(double *)(g_pTrackRoadSequence + node_abs * 6 + 0x10) - k_RespawnSplineOffset;
        spline_ptr = g_pTrackRoadSequence + node_abs * 6 + 0xf;
        plc_model_idx = *spline_ptr;
        if (respawn_heading < k_RespawnAngleMin) {
            respawn_heading = respawn_heading + k_RespawnAngleModulo;
        }
        if (plc_model_idx == 10000) {
            respawn_heading = *(double *)(g_pTrackRoadSequence + road_node * 6 + 0xd) - k_RespawnSplineOffset;
            plc_model_idx = g_pTrackRoadSequence[road_node * 6 + 0xc];
            if (respawn_heading < k_RespawnAngleMin) {
                respawn_heading = respawn_heading + k_RespawnAngleModulo;
            }
        }
        if (plc_model_idx == -2) {
            int skip_count = 0;
            do {
                road_seq_ptr = spline_ptr + 6;
                spline_ptr = spline_ptr + 6;
                skip_count = skip_count + 1;
            } while (*road_seq_ptr == -2);
            respawn_heading = *(double *)(g_pTrackRoadSequence + (skip_count + node_abs) * 6 + 0xd);
            plc_model_idx = g_pTrackRoadSequence[(skip_count + node_abs) * 6 + 0xc];
            goto LAB_RESPAWN_WRAP;
        }
    } else {
        respawn_heading = *(double *)(g_pTrackRoadSequence + 1);
        plc_model_idx = *g_pTrackRoadSequence;
LAB_RESPAWN_WRAP:
        respawn_heading = respawn_heading - k_RespawnSplineOffset;
        if (respawn_heading < k_RespawnAngleMin) {
            respawn_heading = respawn_heading + k_RespawnAngleModulo;
        }
    }

    msh_entry_offset = plc_model_idx * 0x14 + g_pActivePLC;
    plc_model_idx = *(int *)(msh_entry_offset + 0x10);
    int msh_data_offset = g_pActiveMSH + *(int *)(msh_entry_offset + 4) * 4;
    poly_count = *(int *)(msh_data_offset + 4);
    poly_ptr = (unsigned int *)(msh_data_offset + 8 + *(int *)(g_pActiveMSH + *(int *)(msh_entry_offset + 4) * 4) * 0xc);

    if (0 < poly_count) {
        do {
            vi0 = poly_ptr[1];
            vi1 = poly_ptr[2];
            vi2 = poly_ptr[3];
            if ((*poly_ptr & 0xffff0000) < 0x280000) {
                world_pos_y_accum = world_pos_y_accum +
                           *(int *)(msh_data_offset + 8 + vi0 * 0xc) + (*(int *)(msh_entry_offset + 0xc) + 0x6400) * 3 +
                           *(int *)(msh_data_offset + 8 + vi1 * 0xc) + *(int *)(msh_data_offset + 8 + vi2 * 0xc);
                world_pos_x_accum = (((((world_pos_x_accum - *(int *)(msh_data_offset + 0xc + vi0 * 0xc)) + plc_model_idx) -
                           *(int *)(msh_data_offset + 0xc + vi1 * 0xc)) + plc_model_idx) -
                         *(int *)(msh_data_offset + 0xc + vi2 * 0xc)) + plc_model_idx;
                world_pos_z_accum = world_pos_z_accum +
                           *(int *)(msh_data_offset + 0x10 + vi0 * 0xc) + (*(int *)(msh_entry_offset + 0x14) + 0x6400) * 3 +
                           *(int *)(msh_data_offset + 0x10 + vi1 * 0xc) + *(int *)(msh_data_offset + 0x10 + vi2 * 0xc);
                vertex_count_accum = vertex_count_accum + 3;
            }
            poly_ptr = poly_ptr + 0xb;
            poly_count = poly_count - 1;
        } while (poly_count != 0);
    }

    heading_accum = respawn_heading;
    if (respawn_heading < 0.0) {
        heading_accum = respawn_heading + k_RespawnAngleModulo;
    }
    tumble_delta = (double)((crash_tick - total_ticks) + 0x28);
    trig_factor = Math_LookupTrigAngle(tumble_delta);

    *(double *)((int)g_Vehicles + car_offset) =
        (double)(((double)(world_pos_y_accum / vertex_count_accum) - (double)*(double *)((int)g_Vehicles + car_offset + 900)) *
                 trig_factor + (double)*(double *)((int)g_Vehicles + car_offset + 900));

    *(double *)((int)g_Vehicles + car_offset + 8) =
        ((double)(world_pos_x_accum / vertex_count_accum + 0xfa) - *(double *)((int)g_Vehicles + car_offset + 0x38c)) *
        tumble_delta * k_RespawnInterpolationScale + *(double *)((int)g_Vehicles + car_offset + 0x38c);

    trig_factor = Math_LookupTrigAngle(tumble_delta);
    current_x = heading_accum - k_RespawnAngleModulo;
    *(double *)((int)g_Vehicles + car_offset + 0x10) =
        (double)(((double)(world_pos_z_accum / vertex_count_accum) -
                  (double)*(double *)((int)g_Vehicles + car_offset + 0x394)) * trig_factor +
                 (double)*(double *)((int)g_Vehicles + car_offset + 0x394));

    if (ABS(current_x - *(double *)((int)g_Vehicles + car_offset + 0x37c)) <
        ABS(heading_accum - *(double *)((int)g_Vehicles + car_offset + 0x37c))) {
        heading_accum = current_x;
    }

    *(double *)((int)g_Vehicles + car_offset + 0xf8) =
        (heading_accum - *(double *)((int)g_Vehicles + car_offset + 0x37c)) * tumble_delta * k_RespawnInterpolationScale +
        *(double *)((int)g_Vehicles + car_offset + 0x37c);

    if (k_RespawnHeadingMax < *(double *)((int)g_Vehicles + car_offset + 0xf8)) {
        *(double *)((int)g_Vehicles + car_offset + 0xf8) -= k_RespawnAngleModulo;
    }
    if (*(double *)((int)g_Vehicles + car_offset + 0xf8) < k_RespawnHeadingMin) {
        *(double *)((int)g_Vehicles + car_offset + 0xf8) += k_RespawnAngleModulo;
    }

LAB_CRASH_NEXT_TICK:
    final_crash_tick = crash_tick + 1;
    if (total_ticks <= final_crash_tick) {
        is_occupied = 0;
        int check_idx = 0;
        veh_ptr = g_Vehicles;
        if (0 < g_NumRacers) {
            do {
                if (((((car_idx != check_idx) &&
                      (veh_ptr = (double *)(car_offset + (int)g_Vehicles),
                       ABS(*(double *)((int)g_Vehicles + check_idx * 0x484c) - *veh_ptr) < k_RespawnVehicleClearance)) &&
                     ((ABS(*(double *)((int)g_Vehicles + check_idx * 0x484c + 8) - veh_ptr[1]) < k_RespawnVehicleClearance &&
                      ((ABS(*(double *)((int)g_Vehicles + check_idx * 0x484c + 0x10) - veh_ptr[2]) < k_RespawnVehicleClearance &&
                       (*(int *)((int)g_Vehicles + check_idx * 0x484c + 0x354) == 0)))))) && (*(int *)((int)g_Vehicles + check_idx * 0x484c + 0x358) == 0)) &&
                    (*(int *)((int)g_Vehicles + check_idx * 0x484c + 0x35c) == 0)) {
                    is_occupied = 1;
                }
                check_idx = check_idx + 1;
            } while (check_idx < g_NumRacers);
        }

        if ((!is_occupied) &&
            ((*(int *)((int)g_Vehicles + car_offset + 0x528) == 0 ||
              (*(int *)((int)g_Vehicles + car_offset + 0x604) == 1)))) {
            *crash_seq = 0;
            *(int *)((int)g_Vehicles + car_offset + 0x350) = 1;
            int car_entity_stride = car_idx * 0x20;
            *(int *)((int)g_Vehicles + car_offset + 0x35c) = 0;
            *(int *)((int)g_Vehicles + car_offset + 0x534) = 0;
            *(int *)((int)g_Vehicles + car_offset + 0x538) = 0x40590000;

            if ((((int *)(g_pCarTransforms + car_entity_stride))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
                Lisa_MoveDynamicObject((int *)(g_pCarTransforms + car_entity_stride));
            }
            int wheel_entity_stride = car_idx * 0x80;
            if (((int *)(g_pWheelTransforms + wheel_entity_stride))[7] == 0) {
                Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + wheel_entity_stride));
            }
            if (*(int *)(g_pWheelTransforms + wheel_entity_stride + 0x3c) == 0) {
                Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + wheel_entity_stride + 0x20));
            }
            if (*(int *)(g_pWheelTransforms + wheel_entity_stride + 0x5c) == 0) {
                Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + wheel_entity_stride + 0x40));
            }
            if (*(int *)(g_pWheelTransforms + wheel_entity_stride + 0x7c) == 0) {
                Lisa_MoveDynamicObject((int *)(g_pWheelTransforms + wheel_entity_stride + 0x60));
            }
            if ((((int *)(g_pCarShadowTransforms + car_entity_stride))[7] == 0) && (g_DynamicObjectsPaused == 0)) {
                Lisa_MoveDynamicObject((int *)(g_pCarShadowTransforms + car_entity_stride));
            }
            if (((*(int *)(g_PlayerHUDState + car_idx * 0x4c) == 7) && (g_DynamicObjectsPaused == 0)) &&
               (((int *)(car_entity_stride + g_pCarReflectionTransforms))[7] == 0)) {
                Lisa_MoveDynamicObject((int *)(car_entity_stride + g_pCarReflectionTransforms));
            }
            Car_UpdateDynamicObjects(car_idx);
        } else {
            final_crash_tick = crash_tick;
        }
    }

    crash_seq[2] = crash_pos_y;
    crash_seq[1] = crash_pos_x;
    crash_seq[3] = crash_pos_z;
    crash_seq[5] = final_crash_tick;
}

/**
 * @original FX_UpdateCarDebris (IGN_WIN.EXE @ 0x00432bb0, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateCarDebris(int *debris, int debris_idx) {
    int rot_vel_z;
    int rot_vel_x;
    int rot_vel_y;
    int age;
    int max_age;
    int mesh_stride;
    int scaled_alpha;
    int *transform_ptr;
    int update_res;
    int rot_z;
    int rot_y;
    __int64 ftol_result;
    int rot_x;

    rot_x = debris[2];
    rot_y = debris[3];
    rot_z = debris[4];
    rot_vel_z = debris[7];
    rot_vel_x = debris[5];
    rot_vel_y = debris[6];
    age = debris[8];
    max_age = debris[9];

    if (age == 0) {
        mesh_stride = debris_idx * 800;
        *(int *)(&g_DebrisDynamicMeshes + mesh_stride) = 1;
        *(int *)((int)&g_DebrisDynamicMeshes + 4 + mesh_stride) = 1;
        *(int *)((int)&g_DebrisDynamicMeshes + 8 + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0xc + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x10 + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x14 + mesh_stride) = 7;
        scaled_alpha = g_CameraFovScaleX;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x18 + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x1c + mesh_stride) = 0x2100;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x20 + mesh_stride) = 0x5400;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x24 + mesh_stride) = 0x4200;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x28 + mesh_stride) = 0x7500;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x2c + mesh_stride) = 0;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x30 + mesh_stride) = scaled_alpha * 0x96;
        scaled_alpha = g_CameraFovScaleY * 0x96;
        *(int *)((int)&g_DebrisDynamicMeshes + 0x34 + mesh_stride) = scaled_alpha;
        Lisa_SetDynamicObjectMesh(g_pFXTransforms, scaled_alpha, (int *)(debris_idx * 0x20 + g_pFXTransforms),
                                 (int *)((int)&g_DebrisDynamicMeshes + mesh_stride), 2, 200, 0, 0x14, 0);
    }

    scaled_alpha = age * -8 + 0xfa;
    *(int *)((int)&g_DebrisDynamicMeshes + 8 + debris_idx * 800) = scaled_alpha;
    if (scaled_alpha < 0x32) {
        *(int *)((int)&g_DebrisDynamicMeshes + 8 + debris_idx * 800) = 0x32;
    }

    if (0x59c00 < rot_x) {
        rot_x = rot_x + ((rot_x + 0x3ffU) / 0x5a000) * -0x5a000;
    }
    if (0x59c00 < rot_y) {
        rot_y = rot_y + ((rot_y + 0x3ffU) / 0x5a000) * -0x5a000;
    }
    if (0x59c00 < rot_z) {
        rot_z = rot_z + ((rot_z + 0x3ffU) / 0x5a000) * -0x5a000;
    }
    if (rot_x < 0) {
        rot_x = rot_x + ((0x59fffU - rot_x) / 0x5a000) * 0x5a000;
    }
    if (rot_y < 0) {
        rot_y = rot_y + ((0x59fffU - rot_y) / 0x5a000) * 0x5a000;
    }
    if (rot_z < 0) {
        rot_z = rot_z + ((0x59fffU - rot_z) / 0x5a000) * 0x5a000;
    }

    mesh_stride = debris_idx * 0x20;
    ftol_result = __ftol();
    *(int *)(mesh_stride + 4 + g_pFXTransforms) = (int)ftol_result;
    ftol_result = __ftol();
    *(int *)(mesh_stride + 8 + g_pFXTransforms) = (int)ftol_result;
    ftol_result = __ftol();
    *(int *)(mesh_stride + 0xc + g_pFXTransforms) = (int)ftol_result;
    *(int *)(mesh_stride + 0x10 + g_pFXTransforms) =
        (int)(rot_x * 10 + (rot_x * 10 >> 0x1f & 0x3ffU)) >> 10;
    *(int *)(mesh_stride + 0x14 + g_pFXTransforms) =
        (int)(rot_y * 10 + (rot_y * 10 >> 0x1f & 0x3ffU)) >> 10;
    *(int *)(mesh_stride + 0x18 + g_pFXTransforms) =
        (int)(rot_z * 10 + (rot_z * 10 >> 0x1f & 0x3ffU)) >> 10;

    transform_ptr = (int *)(g_pFXTransforms + mesh_stride);
    if (transform_ptr[7] == 0) {
        Lisa_MoveDynamicObject(transform_ptr);
    } else {
        update_res = Lisa_UpdateObjectSpatialGrid(transform_ptr);
        if (update_res != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_TRAN_SPRIT_004993b4);
        }
    }

    if (max_age <= age + 1) {
        Lisa_DeleteDynamicObject((int *)(mesh_stride + g_pFXTransforms));
        *debris = 0;
    }

    debris[2] = rot_x + rot_vel_x;
    debris[3] = rot_y + rot_vel_y;
    debris[4] = rot_z + rot_vel_z;
    debris[8] = age + 1;
}

/**
 * @original FX_SpawnWaterSplashes (IGN_WIN.EXE @ 0x00432f20, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnWaterSplashes(void) {
    int car_base_offset;
    __int64 pos_x_ftol;
    __int64 pos_z_ftol;
    __int64 ftol_result;

    car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (((((*(int *)(g_Vehicles + 0x5d8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) ||
          (*(int *)(car_base_offset + 0x5dc) == 0x5a)) || (*(int *)(car_base_offset + 0x5e0) == 0x5a)) ||
        (*(int *)(car_base_offset + 0x5e4) == 0x5a)) &&
       ((k_WaterSplashSpeedThreshold < *(double *)(car_base_offset + 0x118) && (*(int *)(car_base_offset + 0x270) == 0)))) {
        *(int *)(car_base_offset + 0x33c) = 6;
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
        int splash_count;
        Math_RandomFloat0To1();
        __ftol();
        car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
        if (*(int *)(g_Vehicles + 0x5e8 + g_ActiveVehicleIndex * 0x484c) == 0x5a) {
            fcos((double)*(double *)(car_base_offset + 0xf8));
            fsin((double)*(double *)(car_base_offset + 0xf8));
        }
        if (*(int *)(car_base_offset + 0x5ec) == 0x5a) {
            fcos((double)*(double *)(car_base_offset + 0xf8));
            fsin((double)*(double *)(car_base_offset + 0xf8));
        }
        if (*(int *)(car_base_offset + 0x5f0) == 0x5a) {
            fcos((double)*(double *)(car_base_offset + 0xf8));
            fsin((double)*(double *)(car_base_offset + 0xf8));
        }
        if (*(int *)(car_base_offset + 0x5f4) == 0x5a) {
            fcos((double)*(double *)(car_base_offset + 0xf8));
            fsin((double)*(double *)(car_base_offset + 0xf8));
        }
        splash_count = (*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) == 0) + 1;
        if (splash_count != 0) {
            pos_x_ftol = __ftol();
            pos_z_ftol = __ftol();
            do {
                Math_RandomFloat0To1();
                Math_RandomFloat0To1();
                fcos((double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c));
                fsin((double)*(double *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0xf8));
                g_ActiveParticle.type = 2;
                g_ActiveParticle.pos_x = (int)pos_x_ftol;
                ftol_result = __ftol();
                g_ActiveParticle.pos_y = (int)ftol_result;
                g_ActiveParticle.pos_z = (int)pos_z_ftol;
                ftol_result = __ftol();
                g_ActiveParticle.vel_x = (int)ftol_result;
                g_ActiveParticle.vel_y = 0x4cc;
                ftol_result = __ftol();
                g_ActiveParticle.vel_z = (int)ftol_result;
                g_ActiveParticle.drag = 0x3e1;
                g_ActiveParticle.gravity = 0xffffff67;
                Math_RandomFloat0To1();
                ftol_result = __ftol();
                g_ActiveParticle.field_24 = (int)ftol_result;
                g_ActiveParticle.field_28 = 0xfffffd9a;
                g_ActiveParticle.field_2c = 1;
                g_ActiveParticle.rot_x = 0;
                g_ActiveParticle.rot_y = 0xf;
                FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
                splash_count = splash_count - 1;
            } while (splash_count != 0);
        }
    }

    if (0 < *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c)) {
        *(int *)(g_Vehicles + 0x33c + g_ActiveVehicleIndex * 0x484c) -= 1;
    }
}

/**
 * @original FX_SpawnTireDirtDebris (IGN_WIN.EXE @ 0x004333e0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnTireDirtDebris(void) {
    int surface_code;
    int car_base_offset;
    __int64 vel_y_ftol;
    __int64 ftol_result;
    int wheel_idx;

    if ((k_TireDirtSpeedThreshold < *(double *)(g_Vehicles + 0x118 + g_ActiveVehicleIndex * 0x484c)) &&
       (*(int *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c + 0x270) == 0)) {
        wheel_idx = 0;
        do {
            *(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) = 0;
            car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
            surface_code = *(int *)(g_Vehicles + 0x150 + g_ActiveVehicleIndex * 0x484c);
            if (((((surface_code == 6) || (surface_code == 0x10)) || (surface_code == 0x1a)) || (g_ForceDirtDebris == 1)) &&
               (wheel_idx == 0)) {
                fcos((double)*(double *)(car_base_offset + 0xf8));
                fsin((double)*(double *)(car_base_offset + 0xf8));
                *(int *)(car_base_offset + 0x360) = 1;
            }
            car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
            surface_code = *(int *)(car_base_offset + 0x154);
            if ((((surface_code == 6) || (surface_code == 0x10)) || ((surface_code == 0x1a || (g_ForceDirtDebris == 1)))) &&
               (wheel_idx == 1)) {
                fcos((double)*(double *)(car_base_offset + 0xf8));
                fsin((double)*(double *)(car_base_offset + 0xf8));
                *(int *)(car_base_offset + 0x360) = 1;
            }
            car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
            surface_code = *(int *)(car_base_offset + 0x158);
            if ((((surface_code == 6) || (surface_code == 0x10)) || ((surface_code == 0x1a || (g_ForceDirtDebris == 1)))) &&
               (wheel_idx == 2)) {
                fcos((double)*(double *)(car_base_offset + 0xf8));
                fsin((double)*(double *)(car_base_offset + 0xf8));
                *(int *)(car_base_offset + 0x360) = 1;
            }
            car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
            surface_code = *(int *)(car_base_offset + 0x15c);
            if (((((surface_code == 6) || (surface_code == 0x10)) || (surface_code == 0x1a)) || (g_ForceDirtDebris == 1)) &&
               (wheel_idx == 3)) {
                fcos((double)*(double *)(car_base_offset + 0xf8));
                fsin((double)*(double *)(car_base_offset + 0xf8));
                *(int *)(car_base_offset + 0x360) = 1;
            }
            if (*(int *)(g_Vehicles + 0x360 + g_ActiveVehicleIndex * 0x484c) == 1) {
                int particle_batch = 2;
                vel_y_ftol = __ftol();
                do {
                    Math_RandomFloat0To1();
                    Math_RandomFloat0To1();
                    g_ActiveParticle.type = 8;
                    ftol_result = __ftol();
                    g_ActiveParticle.pos_x = (int)ftol_result;
                    ftol_result = __ftol();
                    g_ActiveParticle.pos_y = (int)ftol_result;
                    ftol_result = __ftol();
                    g_ActiveParticle.pos_z = (int)ftol_result;
                    ftol_result = __ftol();
                    g_ActiveParticle.vel_x = (int)ftol_result;
                    g_ActiveParticle.vel_y = (int)vel_y_ftol;
                    ftol_result = __ftol();
                    g_ActiveParticle.vel_z = (int)ftol_result;
                    g_ActiveParticle.rot_x = 0;
                    g_ActiveParticle.rot_y = 0;
                    g_ActiveParticle.rot_z = 0;
                    g_ActiveParticle.drag = 0x3d7;
                    g_ActiveParticle.gravity = 0xffffff67;
                    g_ActiveParticle.field_24 = 0x1ec00;
                    g_ActiveParticle.field_28 = 0xffffffe0;
                    g_ActiveParticle.field_2c = 1;
                    Math_RandomFloat0To1();
                    ftol_result = __ftol();
                    g_ActiveParticle.field_3c = (int)ftol_result;
                    Math_RandomFloat0To1();
                    ftol_result = __ftol();
                    g_ActiveParticle.field_40 = (int)ftol_result;
                    Math_RandomFloat0To1();
                    ftol_result = __ftol();
                    g_ActiveParticle.field_44 = (int)ftol_result;
                    g_ActiveParticle.field_48 = 0;
                    g_ActiveParticle.field_4c = 0x3c;
                    FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
                    particle_batch = particle_batch - 1;
                } while (particle_batch != 0);
            }
            wheel_idx = wheel_idx + 1;
        } while (wheel_idx < 4);
    }
}

/**
 * @original FX_SpawnLandingDustPuffs (IGN_WIN.EXE @ 0x004338d0, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnLandingDustPuffs(void) {
    int car_base_offset;
    int surface_code;
    double wheel_angle;
    __int64 ftol_result;

    car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    if (*(int *)(g_Vehicles + 0x278 + g_ActiveVehicleIndex * 0x484c) == 1) {
        surface_code = *(int *)(car_base_offset + 0x158);
        if (((surface_code < 0) || (0x27 < surface_code)) &&
           ((surface_code = *(int *)(car_base_offset + 0x15c), surface_code < 0 || (0x27 < surface_code)))) {
            surface_code = 0;
        }

        if (((int*)&(g_SurfaceDustFlagTable))[surface_code] == '\x01') {
            int wheel_idx;
            for (wheel_idx = 0; wheel_idx < 4; wheel_idx++) {
                wheel_angle = (double)*(double *)(g_Vehicles + 0xf8 + g_ActiveVehicleIndex * 0x484c) +
                              (double)k_LandingDustAngularOffset;
                fcos(wheel_angle);
                fsin(wheel_angle);
                g_ActiveParticle.type = 1;
                ftol_result = __ftol();
                g_ActiveParticle.pos_x = (int)ftol_result;
                ftol_result = __ftol();
                g_ActiveParticle.pos_y = (int)ftol_result;
                ftol_result = __ftol();
                g_ActiveParticle.pos_z = (int)ftol_result;
                Math_RandomFloat0To1();
                ftol_result = __ftol();
                g_ActiveParticle.vel_x = (int)ftol_result;
                g_ActiveParticle.vel_y = 0xfffffc00;
                Math_RandomFloat0To1();
                ftol_result = __ftol();
                g_ActiveParticle.vel_z = (int)ftol_result;
                g_ActiveParticle.gravity = 0;
                g_ActiveParticle.drag = 0x400;
                g_ActiveParticle.field_24 = 4;
                g_ActiveParticle.field_2c = 0xcc;
                g_ActiveParticle.field_28 = 0;
                Math_RandomFloat0To1();
                ftol_result = __ftol();
                g_ActiveParticle.rot_x = (int)ftol_result;
                Math_RandomFloat0To1();
                ftol_result = __ftol();
                g_ActiveParticle.rot_y = (int)ftol_result;
                g_ActiveParticle.field_3c = 0;
                g_ActiveParticle.field_40 = 0;
                g_ActiveParticle.field_44 = 0;
                g_ActiveParticle.field_48 = 0;
                g_ActiveParticle.field_4c = 0;
                g_ActiveParticle.life = 0;
                g_ActiveParticle.rot_z = &g_SmokeParticleDescriptor;
                g_ActiveParticle.field_58 = 0x28;
                g_ActiveParticle.field_54 = 0;
                FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
            }
        }
    }
}

/**
 * @original FX_SpawnTireSkidSmoke (IGN_WIN.EXE @ 0x00433f90, fx.c)
 * @fidelity ADAPTED
 */
void FX_SpawnTireSkidSmoke(void) {
    int car_base_offset;
    double skid_energy;
    int surface_code;
    double rand_val;
    __int64 ftol_result;

    car_base_offset = g_Vehicles + g_ActiveVehicleIndex * 0x484c;
    surface_code = *(int *)(car_base_offset + 0x158);
    if (((surface_code < 0) || (0x27 < surface_code)) &&
       ((surface_code = *(int *)(car_base_offset + 0x15c), surface_code < 0 || (0x27 < surface_code)))) {
        surface_code = 0;
    }

    if ((((surface_code == 4) || (surface_code == 0xe)) || (surface_code == 0x18)) &&
       ((k_SkidSmokeSpeedThreshold < *(double *)(car_base_offset + 0x118) && (*(int *)(car_base_offset + 0x270) == 0)))) {
        skid_energy = *(double *)(car_base_offset + 0x118) * *(double *)(car_base_offset + 0x118) * k_SkidSmokeProbabilityFactor;
        rand_val = Math_RandomFloat0To1();
        if (rand_val * (double)k_SkidSmokeRandomScale < (double)skid_energy) {
            g_ActiveParticle.type = 1;
            ftol_result = __ftol();
            g_ActiveParticle.pos_x = (int)ftol_result;
            ftol_result = __ftol();
            g_ActiveParticle.pos_y = (int)ftol_result;
            ftol_result = __ftol();
            g_ActiveParticle.pos_z = (int)ftol_result;
            g_ActiveParticle.vel_x = 0;
            Math_RandomFloat0To1();
            ftol_result = __ftol();
            g_ActiveParticle.vel_y = (int)ftol_result;
            g_ActiveParticle.vel_z = 0;
            g_ActiveParticle.drag = 0x3f5;
            g_ActiveParticle.field_24 = 4;
            g_ActiveParticle.field_28 = 0x800;
            g_ActiveParticle.field_2c = 0x3d;
            g_ActiveParticle.gravity = 0;
            Math_RandomFloat0To1();
            ftol_result = __ftol();
            g_ActiveParticle.rot_x = (int)ftol_result;
            Math_RandomFloat0To1();
            ftol_result = __ftol();
            g_ActiveParticle.rot_y = (int)ftol_result;
            g_ActiveParticle.field_3c = 0;
            g_ActiveParticle.field_40 = 0;
            g_ActiveParticle.field_44 = 0;
            g_ActiveParticle.field_48 = 0;
            g_ActiveParticle.field_4c = 0;
            g_ActiveParticle.life = 0;
            g_ActiveParticle.rot_z = &g_SmokeParticleDescriptor;
            g_ActiveParticle.field_58 = 100;
            g_ActiveParticle.field_54 = 0;
            FX_SpawnParticle((SceneryParticle *)&g_ActiveParticle.type);
        }
    }
}

/**
 * @original FX_UpdateWeatherGeometry (IGN_WIN.EXE @ 0x00434840, fx.c)
 * @fidelity ADAPTED
 */
void FX_UpdateWeatherGeometry(void) {
    int vp;
    int vehicle_idx;
    int *pMesh;
    int pair_idx;
    int num_pairs;
    int total_verts;
    int v_idx;
    int cmd_idx;
    int rand_x, rand_y, rand_z;
    int *pCmdRain;
    int *pCmdSnow;
    int grid_x, grid_z;
    int obj_idx;
    char *pVoice;
    int lightning_roll;
    int road_node;
    int road_idx;
    int road_height;
    unsigned int abs_node;

    if ((*(int *)(g_Vehicles + 0x528) != 1 || g_RaceTimer < 120.0) ||
        (g_WeatherType != 1 && g_WeatherType != 2)) {
        if (g_WeatherType == 1 || g_WeatherType == 2) {
            for (vp = 0; vp < g_WeatherViewportCount; vp++) {
                if (vp == 0) {
                    vehicle_idx = g_MenuCursorPos;
                    pMesh = g_pRainMesh_P2;
                } else {
                    vehicle_idx = 1;
                    pMesh = g_pRainMesh_P1;
                }

                if ((double)(g_WeatherAnimTick + 2) < (double)g_GlobalFrameCount) {
                    if (g_WeatherActive[vp] == 1 && g_RaceTimer < 0.0 &&
                        (double)pMesh[1] < 100.0) {
                        pair_idx = pMesh[1] / 2;
                        if (g_WeatherViewportCount == 1 && g_WeatherType == 1) {
                            Palette_AdjustRGB(pMesh[1], pMesh[1] >> 31, g_ActiveTrackPalette + 8);
                            Gfx_FreeSurface((int)pMesh);
                        }

                        rand_x = (int)Math_RandomFloat0To1();
                        rand_y = (int)Math_RandomFloat0To1();
                        rand_z = (int)Math_RandomFloat0To1();

                        if (g_WeatherType == 1) {
                            *pMesh += 4;
                            pMesh[1] += 2;
                            pMesh[pair_idx * 12 + 2] = rand_x;
                            pMesh[pair_idx * 12 + 3] = -rand_y;
                            pMesh[pair_idx * 12 + 4] = rand_z;
                            pMesh[pair_idx * 12 + 5] = rand_x + 4;
                            pMesh[pair_idx * 12 + 6] = 60 - rand_y;
                            pMesh[pair_idx * 12 + 7] = rand_z;
                            pMesh[pair_idx * 12 + 8] = rand_x;
                            pMesh[pair_idx * 12 + 9] = -800 - rand_y;
                            pMesh[pair_idx * 12 + 10] = rand_z;
                            pMesh[pair_idx * 12 + 11] = rand_x + 4;
                            pMesh[pair_idx * 12 + 12] = -740 - rand_y;
                            pMesh[pair_idx * 12 + 13] = rand_z;
                        } else if (g_WeatherType == 2) {
                            *pMesh += 2;
                            pMesh[1] += 2;
                            pMesh[pair_idx * 6 + 2] = rand_x;
                            pMesh[pair_idx * 6 + 3] = -rand_y;
                            pMesh[pair_idx * 6 + 4] = rand_z;
                            pMesh[pair_idx * 6 + 5] = rand_x;
                            pMesh[pair_idx * 6 + 6] = -800 - rand_y;
                            pMesh[pair_idx * 6 + 7] = rand_z;
                        }

                        total_verts = pair_idx * 2 + 2;
                        if (total_verts > 0) {
                            pCmdSnow = pMesh + (pair_idx + 1) * 6 + 2;
                            pCmdRain = pMesh + (pair_idx + 1) * 12 + 2;
                            v_idx = 0;
                            for (cmd_idx = 0; cmd_idx < total_verts; cmd_idx++) {
                                if (g_WeatherType == 1) {
                                    pCmdRain[0] = 0xd;
                                    pCmdRain[1] = v_idx;
                                    pCmdRain[2] = v_idx + 1;
                                    pCmdRain[3] = 0x7700;
                                    pCmdRain[4] = 0x7b00;
                                    pCmdRain[5] = g_RainTextureId;
                                } else if (g_WeatherType == 2) {
                                    pCmdSnow[0] = 7;
                                    pCmdSnow[1] = cmd_idx;
                                    pCmdSnow[2] = 0;
                                    pCmdSnow[3] = 0x7500;
                                    pCmdSnow[4] = 0x1f00;
                                    pCmdSnow[5] = 0x9400;
                                    pCmdSnow[6] = 0;
                                    pCmdSnow[7] = g_CameraFovScaleX * 100;
                                    pCmdSnow[8] = g_CameraFovScaleY * 100;
                                }
                                pCmdSnow += 9;
                                pCmdRain += 6;
                                v_idx += 2;
                            }
                        }
                    }

                    if ((g_WeatherActive[vp] == 0 || g_RaceTimer >= 0.0) && pMesh[1] > 0) {
                        if (g_WeatherType == 1) {
                            *pMesh -= 4;
                            pMesh[1] -= 2;
                        } else if (g_WeatherType == 2) {
                            *pMesh -= 2;
                            pMesh[1] -= 2;
                        }

                        num_pairs = pMesh[1] / 2;
                        if (g_WeatherViewportCount == 1 && g_WeatherType == 1) {
                            Palette_AdjustRGB(pMesh[1], pMesh[1] >> 31, g_ActiveTrackPalette + 8);
                            Gfx_FreeSurface((int)pMesh);
                        }

                        if (num_pairs * 2 > 0) {
                            pCmdSnow = pMesh + num_pairs * 6 + 2;
                            pCmdRain = pMesh + num_pairs * 12 + 2;
                            v_idx = 0;
                            for (cmd_idx = 0; cmd_idx < num_pairs * 2; cmd_idx++) {
                                if (g_WeatherType == 1) {
                                    pCmdRain[0] = 0xd;
                                    pCmdRain[1] = v_idx;
                                    pCmdRain[2] = v_idx + 1;
                                    pCmdRain[3] = 0x7700;
                                    pCmdRain[4] = 0x7b00;
                                    pCmdRain[5] = g_RainTextureId;
                                } else if (g_WeatherType == 2) {
                                    pCmdSnow[0] = 7;
                                    pCmdSnow[1] = cmd_idx;
                                    pCmdSnow[2] = 0;
                                    pCmdSnow[3] = 0x7500;
                                    pCmdSnow[4] = 0x1f00;
                                    pCmdSnow[5] = 0x9400;
                                    pCmdSnow[6] = 0;
                                    pCmdSnow[7] = g_CameraFovScaleX * 100;
                                    pCmdSnow[8] = g_CameraFovScaleY * 100;
                                }
                                pCmdSnow += 9;
                                pCmdRain += 6;
                                v_idx += 2;
                            }
                        }
                    }
                }

                if (g_WeatherType == 1 && *pMesh > 0) {
                    g_WeatherDropOffsetX[vp] += 5;
                    g_WeatherDropOffsetY[vp] -= 25;
                    if ((int)Math_RandomFloat0To1() == 0 && g_RaceTimer < 0.0) {
                        Audio_PlaySampleVol(1, 0, 1, 0, 0x10000, 0, 0);
                    }
                } else if (g_WeatherType == 2 && *pMesh > 0) {
                    g_SnowflakeAngle[vp] += 0.11781;
                    if (g_SnowflakeAngle[vp] > 6.2831853071796) {
                        g_SnowflakeAngle[vp] -= 6.2831853071796;
                    }
                    g_WeatherDropOffsetX[vp] = (int)sin(g_SnowflakeAngle[vp]);
                    g_WeatherDropOffsetY[vp] -= 5;
                    rand_x = (int)cos(*(double *)(g_Vehicles + 0x100 + vehicle_idx * 0x484c));
                    rand_z = (int)sin(*(double *)(g_Vehicles + 0x100 + vehicle_idx * 0x484c));

                    num_pairs = pMesh[1] / 2;
                    if (num_pairs > 0) {
                        int *pV = pMesh + 2;
                        while (num_pairs != 0) {
                            int nx = pV[0] + rand_x;
                            int nz = pV[2] + rand_z;
                            num_pairs--;
                            pV[3] = nx;
                            pV[5] = nz;
                            pV[9] = nx;
                            pV[11] = nz;
                            pV += 6;
                        }
                    }
                }

                if (g_WeatherDropOffsetY[vp] < -800) {
                    g_WeatherDropOffsetX[vp] = 0;
                    g_WeatherDropOffsetY[vp] += 800;
                }

                obj_idx = vp * 0x120;
                for (grid_z = 0; grid_z < 3; grid_z++) {
                    for (grid_x = 0; grid_x < 3; grid_x++) {
                        rand_x = (int)Math_RandomFloat0To1();
                        rand_y = (int)Math_RandomFloat0To1();
                        rand_z = (int)Math_RandomFloat0To1();

                        *(int *)(g_WeatherGridObjects + 4 + obj_idx) =
                            g_WeatherDropOffsetX[vp] + ((rand_x + grid_x) * 5 - 5) * 200;
                        *(int *)(g_WeatherGridObjects + 8 + obj_idx) =
                            g_WeatherDropOffsetY[vp] + rand_y;
                        *(int *)(g_WeatherGridObjects + 0xc + obj_idx) =
                            ((grid_z + rand_z) * 5 - 5) * 200;
                        *(int *)(g_WeatherGridObjects + 0x10 + obj_idx) = 0;
                        *(int *)(g_WeatherGridObjects + 0x14 + obj_idx) = 0;
                        *(int *)(g_WeatherGridObjects + 0x18 + obj_idx) = 0;

                        if (Lisa_UpdateObjectSpatialGrid((int *)(g_WeatherGridObjects + obj_idx)) != 0) {
                            Log_DebugPrintf("FEL VID LI_MOVEOBJECT_WEATHER\n");
                        }
                        obj_idx += 0x20;
                    }
                }

                pVoice = Audio_GetVoice(g_WeatherAudioVoices[vp]);
                if (pVoice != NULL) {
                    *(int *)(pVoice + 0xc) = (int)Math_RandomFloat0To1();
                }

                if (g_WeatherType == 1 && g_WeatherActive[vp] == 1 && g_LightningEnabled == 1) {
                    if ((int)Math_RandomFloat0To1() == 0) {
                        lightning_roll = (int)Math_RandomFloat0To1();
                        abs_node = *(unsigned int *)(g_Vehicles + 0x364 + vehicle_idx * 0x484c);
                        road_node = abs((int)abs_node) + lightning_roll;
                        if (road_node < g_TrackRoadSequenceNodeCount) {
                            road_idx = *(int *)(g_pTrackRoadSequence + road_node * 0x18);
                        } else {
                            road_idx = *(int *)(g_pTrackRoadSequence + lightning_roll * 0x18);
                        }
                        road_height = *(int *)(road_idx * 0x14 + g_pActivePLC + 0x10);

                        g_ActiveParticle.type = 6;
                        g_ActiveParticle.pos_x = (int)Math_RandomFloat0To1();
                        g_ActiveParticle.pos_y = road_height * -1024;
                        g_ActiveParticle.pos_z = (int)Math_RandomFloat0To1();
                        g_ActiveParticle.vel_z = 6;
                        g_ActiveParticle.vel_y = 0;
                        g_ActiveParticle.vel_x = vehicle_idx;

                        FX_SpawnParticle(&g_ActiveParticle);
                        Audio_PlaySampleVol(1, 0, 2, 0, 0x10000, 0, 0);
                    }
                }
            }

            if ((double)(g_WeatherAnimTick + 2) < (double)g_GlobalFrameCount) {
                g_WeatherAnimTick = (int)g_GlobalFrameCount;
            }
        }
    } else if ((g_pRainMesh_P2[1] > 0 || g_pRainMesh_P1[1] > 0) &&
               ((g_IsSplitScreen == 0 && g_RaceTimer >= 120.0) ||
                (g_IsSplitScreen == 1 && *(int *)(g_Vehicles + 0x4d74) == 1 && g_RaceTimer >= 120.0))) {
        Gfx_FreeSurface(g_ActiveTrackPalette + 8);
        g_pRainMesh_P2[1] = 0;
        g_pRainMesh_P1[1] = 0;
    }

    if (g_WeatherType == 1 || g_WeatherType == 3) {
        for (vp = 0; vp < g_WeatherViewportCount; vp++) {
            pVoice = Audio_GetVoice(g_WeatherAudioVoices[vp]);
            if (pVoice != NULL) {
                if (g_WeatherActive[vp] == 1 && *(int *)(pVoice + 0xc) < 30000) {
                    *(int *)(pVoice + 0xc) += 2500;
                } else if (g_WeatherActive[vp] == 0 && *(int *)(pVoice + 0xc) > 0) {
                    *(int *)(pVoice + 0xc) -= 2500;
                    if (*(int *)(pVoice + 0xc) < 0) {
                        *(int *)(pVoice + 0xc) = 0;
                    }
                }
            }
        }
    }
}

/**
 * @original FX_FrameTick (IGN_WIN.EXE @ 0x00435350, fx.c)
 * @fidelity ADAPTED
 */
void FX_FrameTick(void) {
    int i;
    VehicleState *v;
    double pitch_rad;
    double roll_deg;
    double angle_step;

    for (g_ActiveVehicleIndex = 0; g_ActiveVehicleIndex < g_NumRacers; g_ActiveVehicleIndex++) {
        v = (VehicleState *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
        pitch_rad = v->angle_pitch * g_Const_DegToRad;

        if (v->suspension_override == 0) {
            v->smooth_pitch = ((pitch_rad + *(double *)((char *)v + 0xb8)) * g_Const_0_05 - v->smooth_pitch) *
                              g_Const_0_1 + v->smooth_pitch;
            angle_step = (double)Math_AngleMod(g_ActiveVehicleIndex * 0x909);
            roll_deg = angle_step;
            while (roll_deg > g_Const_Pi) {
                roll_deg -= g_Const_TwoPi;
            }
            while (roll_deg < g_Const_NegPi) {
                roll_deg += g_Const_TwoPi;
            }
            v->smooth_roll = roll_deg * g_Const_0_1 + v->smooth_roll;
        } else {
            v->smooth_pitch = pitch_rad * g_Const_0_05;
            v->smooth_roll = v->angle_roll * g_Const_DegToRad * g_Const_0_05;
        }

        FX_SpawnWaterSplashes();

        if (v->water_splash_accum > 0.0 && v->water_splash_accum < g_Const_0_05) {
            v->water_splash_accum += k_SplashAccumIncrement;
        }
    }

    FX_UpdateWeatherGeometry();

    if (((g_IsSplitScreen == 0 && *(int *)(g_Vehicles + 0x528) == 1) ||
         (g_IsSplitScreen == 1 && *(int *)(g_Vehicles + 0x528) == 1 && *(int *)(g_Vehicles + 0x4d74) == 1)) &&
        (g_RaceTimer >= k_TurboProgressMinTimer && g_IsGamePaused == 0)) {
        g_TurboMeterFill += 10;
        if (g_TurboMeterFill > 319) {
            g_TurboMeterFill = 319;
            if (g_GameMode == 3) {
                if (g_LapsTotal - 1 > 5 - g_RacePosition) {
                    g_RacePosition--;
                }
            } else {
                if (g_RacePosition < g_LapsTotal - 1) {
                    g_TurboMeterFill = 0;
                    g_RacePosition++;
                }
                if (g_LapRecordSeconds <= (double)(g_TargetLapTimeCents / 100) || g_RacePosition > 1 ||
                    ((*(int *)(g_Vehicles + 0x528) != 0 || g_LapsTotal != 0) &&
                     (*(int *)(g_Vehicles + 0x528) != 1 || g_LapsTotal != 1))) {
                    /* skip */
                } else {
                    g_RacePosition++;
                }
                g_TurboMeterFill = 0;
            }
        }
    }

    for (i = 0; i < 10; i++) {
        g_HudAnimTimers[i]++;
        g_HudAnimTimers[i + 10]++;
    }

    if (g_PlayerCarChoice == 7 && g_HudAnimSinPhase < k_SpecialCarBonusLimit) {
        g_HudAnimSinPhase += k_SpecialCarBonusStep;
    }
}

/**
 * @original Pos_InitAnimatedObjects (IGN_WIN.EXE @ 0x004356d0, fx.c)
 * @fidelity ADAPTED
 */
void Pos_InitAnimatedObjects(void) {
    int *anim_node;
    int *keyframe_ptr;
    int delta_x, delta_y, delta_z;
    int *base_node;
    int anim_idx;
    int frame_idx;
    int *next_keyframe;

    if (0 < *g_pActivePLC) {
        for (anim_idx = 0; anim_idx < *g_pActivePLC; anim_idx++) {
            int node_offset = *(int *)(g_pActivePOS + anim_idx * 4);
            if (node_offset != -1) {
                keyframe_ptr = (int *)(g_pActivePOS + (node_offset + anim_idx) * 4);
                delta_x = keyframe_ptr[2];
                delta_y = keyframe_ptr[3];
                delta_z = keyframe_ptr[4];
                base_node = keyframe_ptr + 2;
                frame_idx = 0;
                if (*keyframe_ptr != 1 && -1 < *keyframe_ptr - 1) {
                    anim_node = base_node;
                    next_keyframe = keyframe_ptr + 8;
                    do {
                        frame_idx = frame_idx + 1;
                        *anim_node = *anim_node - *next_keyframe;
                        anim_node[1] = anim_node[1] - next_keyframe[1];
                        anim_node[2] = anim_node[2] - next_keyframe[2];
                        anim_node = anim_node + 6;
                        next_keyframe = next_keyframe + 6;
                    } while (frame_idx < *keyframe_ptr - 1);
                }
                keyframe_ptr = base_node + frame_idx * 6;
                *keyframe_ptr = base_node[frame_idx * 6] - delta_x;
                keyframe_ptr[1] = keyframe_ptr[1] - delta_y;
                keyframe_ptr[2] = keyframe_ptr[2] - delta_z;
            }
        }
    }
}

/**
 * @original Pos_UpdateAnimatedObjects (IGN_WIN.EXE @ 0x004357a0, fx.c)
 * @fidelity ADAPTED
 */
void Pos_UpdateAnimatedObjects(void) {
    int keyframe_base;
    int *transform_field;
    int frame_pos;
    int entity_offset;
    int pos_table_offset;
    int plc_idx;

    pos_table_offset = 0;
    if (0 < *g_pActivePLC) {
        entity_offset = 0;
        for (plc_idx = 0; plc_idx < *g_pActivePLC; plc_idx++) {
            if ((*(int *)(g_pActivePOS + pos_table_offset) != -1) &&
                (*(int *)((int)&g_pAnimatedSceneryObjects + 0x1c + entity_offset) == 1)) {
                frame_pos = *(int *)(g_pActivePOS + pos_table_offset) + plc_idx;
                keyframe_base = g_pActivePOS + frame_pos * 4;
                if (*(int *)(g_pActivePOS + 4 + frame_pos * 4) == *(int *)(g_pActivePOS + frame_pos * 4)) {
                    *(int *)(keyframe_base + 4) = 0;
                }
                frame_pos = keyframe_base + 8 + *(int *)(keyframe_base + 4) * 0x18;
                transform_field = (int *)((int)&g_pAnimatedSceneryObjects + 4 + entity_offset);
                *transform_field = *transform_field - *(int *)(keyframe_base + 8 + *(int *)(keyframe_base + 4) * 0x18);
                transform_field = (int *)((int)&g_pAnimatedSceneryObjects + 8 + entity_offset);
                *transform_field = *transform_field - *(int *)(frame_pos + 4);
                transform_field = (int *)((int)&g_pAnimatedSceneryObjects + 0xc + entity_offset);
                *transform_field = *transform_field - *(int *)(frame_pos + 8);
                *(int *)((int)&g_pAnimatedSceneryObjects + 0x10 + entity_offset) = *(int *)(frame_pos + 0xc);
                *(int *)((int)&g_pAnimatedSceneryObjects + 0x14 + entity_offset) = *(int *)(frame_pos + 0x10);
                *(int *)((int)&g_pAnimatedSceneryObjects + 0x18 + entity_offset) = *(int *)(frame_pos + 0x14);

                frame_pos = *(int *)((int)&g_pAnimatedSceneryObjects + 0x10 + entity_offset);
                if (0xe0f < frame_pos) {
                    *(int *)((int)&g_pAnimatedSceneryObjects + 0x10 + entity_offset) = frame_pos - 0xe10;
                }
                frame_pos = *(int *)((int)&g_pAnimatedSceneryObjects + 0x14 + entity_offset);
                if (0xe0f < frame_pos) {
                    *(int *)((int)&g_pAnimatedSceneryObjects + 0x14 + entity_offset) = frame_pos - 0xe10;
                }
                frame_pos = *(int *)((int)&g_pAnimatedSceneryObjects + 0x18 + entity_offset);
                if (0xe0f < frame_pos) {
                    *(int *)((int)&g_pAnimatedSceneryObjects + 0x18 + entity_offset) = frame_pos - 0xe10;
                }

                if (Lisa_UpdateObjectSpatialGrid((int *)((int)&g_pAnimatedSceneryObjects + entity_offset)) != 0) {
                    Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_ANIM_OBJ_004994ac);
                }
                *(int *)(keyframe_base + 4) = *(int *)(keyframe_base + 4) + 1;
            }
            entity_offset = entity_offset + 0x20;
            pos_table_offset = pos_table_offset + 4;
        }
    }
}

/**
 * @original Math_LookupTrigAngle (IGN_WIN.EXE @ 0x004358e0, fx.c)
 * @fidelity ADAPTED
 */
double Math_LookupTrigAngle(double angle) {
    int index = (int)(angle * g_Const_200_0);
    return (double)*(int *)(&g_TrigAngleTable[index]) * g_Const_0_005;
}

/**
 * @original Camera_UpdateOverview (IGN_WIN.EXE @ 0x00435910, fx.c)
 * @fidelity ADAPTED
 */
void Camera_UpdateOverview(void) {
    VehicleState *veh;
    VehicleConfig *config;
    uint8_t *cam_state;
    int *seq_node_ptr;
    int *lookahead_ptr;
    uint8_t *plc_entry;
    uint8_t *msh_entry;
    unsigned int *poly_ptr;
    int *vertex_data;
    int *surf_data;
    int *particle_dst;
    int *particle_src;
    int road_node;
    int node_idx;
    int chunk_idx;
    int model_idx;
    int poly_count;
    int vertex_count;
    int skip_count;
    int plc_x, plc_y, plc_z;
    int sum_x, sum_y, sum_z;
    int avg_x, avg_y, avg_z;
    int v0_idx, v1_idx, v2_idx;
    int normal_x, normal_y, normal_z;
    int p0_x, p0_y, p0_z;
    int p1_x, p1_y, p1_z;
    int emitter_idx, particle_idx, template_idx, particle_count;
    int car_model;
    int i, wheel_offset;
    double track_yaw;
    double ground_y;
    double cos_val, sin_val;

    veh = (VehicleState *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
    config = (VehicleConfig *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);
    cam_state = (uint8_t *)&g_VehicleCameraStates + g_ActiveVehicleIndex * 464;

    sum_x = 0;
    sum_y = 0;
    sum_z = 0;
    vertex_count = 0;

    road_node = veh->current_node_idx;
    node_idx = (road_node < 0) ? -road_node : road_node;

    if (node_idx + 2 < g_TrackRoadSequenceNodeCount) {
        if (road_node < 0) {
            track_yaw = *(double *)(g_pTrackRoadSequence + node_idx * 6 + 0x10) - 0.0;
            seq_node_ptr = g_pTrackRoadSequence + node_idx * 6 + 0xf;
            chunk_idx = *seq_node_ptr;
            if (track_yaw < 0.05) {
                track_yaw += 0.2;
            }
            if (chunk_idx == 10000) {
                track_yaw = *(double *)(g_pTrackRoadSequence + road_node * 6 + 0xd) - 0.0;
                chunk_idx = g_pTrackRoadSequence[road_node * 6 + 0xc];
                if (track_yaw < 0.05) {
                    track_yaw += 0.2;
                }
            }
            if (chunk_idx == -2) {
                skip_count = 0;
                do {
                    lookahead_ptr = seq_node_ptr + 6;
                    seq_node_ptr = seq_node_ptr + 6;
                    skip_count++;
                } while (*lookahead_ptr == -2);
                track_yaw = *(double *)(g_pTrackRoadSequence + (skip_count + node_idx) * 6 + 0xd);
                chunk_idx = g_pTrackRoadSequence[(skip_count + node_idx) * 6 + 0xc];
            }
        } else {
            track_yaw = *(double *)(g_pTrackRoadSequence + road_node * 6 + 0xd);
            chunk_idx = g_pTrackRoadSequence[road_node * 6 + 0xc];
        }
    } else {
        track_yaw = *(double *)(g_pTrackRoadSequence + 1);
        chunk_idx = *g_pTrackRoadSequence;
    }

    track_yaw -= 0.0;
    if (track_yaw < 0.05) {
        track_yaw += 0.2;
    }

    /* Compute centroid of road chunk drivable polygons */
    plc_entry = (uint8_t *)g_pActivePLC + chunk_idx * 20;
    plc_x = *(int *)(plc_entry + 0x0c);
    plc_y = *(int *)(plc_entry + 0x10);
    plc_z = *(int *)(plc_entry + 0x14);
    model_idx = *(int *)(plc_entry + 4);

    msh_entry = (uint8_t *)g_pActiveMSH + model_idx * 4;
    poly_count = *(int *)(msh_entry + 4);
    poly_ptr = (unsigned int *)(msh_entry + 8 + (*(int *)msh_entry) * 12);
    vertex_data = (int *)(msh_entry + 8);

    while (poly_count > 0) {
        v0_idx = (int)poly_ptr[1];
        v1_idx = (int)poly_ptr[2];
        v2_idx = (int)poly_ptr[3];

        if ((*poly_ptr & 0xffff0000) < 0x280000) {
            sum_x += vertex_data[v0_idx * 3] + (plc_x + 25600) * 3 +
                     vertex_data[v1_idx * 3] + vertex_data[v2_idx * 3];
            sum_y += (((-vertex_data[v0_idx * 3 + 1] + plc_y) -
                       vertex_data[v1_idx * 3 + 1] + plc_y) -
                       vertex_data[v2_idx * 3 + 1]) + plc_y;
            sum_z += vertex_data[v0_idx * 3 + 2] + (plc_z + 25600) * 3 +
                     vertex_data[v1_idx * 3 + 2] + vertex_data[v2_idx * 3 + 2];
            vertex_count += 3;
        }
        poly_ptr += 11;
        poly_count--;
    }

    avg_x = sum_x / vertex_count;
    avg_z = sum_z / vertex_count;
    avg_y = sum_y / vertex_count + 250;

    g_ActiveTrackSegmentAttribute = g_TrackSegmentTable[chunk_idx * 3];

    /* Setup orthographic camera for track culling */
    *(int *)((uint8_t *)g_pActiveCamera + 0x38) = 0;
    g_pActiveCamera[0] = (double)avg_x;
    g_pActiveCamera[1] = (double)(sum_y / vertex_count + 750);
    g_pActiveCamera[2] = (double)avg_z;
    g_pActiveCamera[3] = 850.0;
    g_pActiveCamera[4] = 0.0;
    g_pActiveCamera[5] = 0.0;
    *(int *)((uint8_t *)g_pActiveCamera + 0x7c) = 0;
    *(int *)((uint8_t *)g_pActiveCamera + 0x80) = config->viewport_h;
    *(int *)((uint8_t *)g_pActiveCamera + 0x84) = config->viewport_w;
    *(int *)((uint8_t *)g_pActiveCamera + 0x88) = avg_z;

    Lisa_CullObjectsOrthographic();

    /* Ground surface probe and snap */
    surf_data = (int *)Track_FindSurfaceHeight(avg_x, avg_y, avg_z, -1, 50);
    if (*surf_data == -1) {
        avg_x -= 20;
        surf_data = (int *)Track_FindSurfaceHeight(avg_x, avg_y, avg_z, -1, 50);
        if (*surf_data == -1) {
            Log_DebugPrintf(s_TrackSurfaceMissing_004994cc);
        }
    }

    /* Copy surface probe info to all 4 wheels (16 ints = 64 bytes each) */
    for (wheel_offset = 0; wheel_offset < 4; wheel_offset++) {
        for (i = 0; i < 16; i++) {
            veh->wheel_surface_info[wheel_offset * 16 + i] = surf_data[i];
        }
    }

    normal_x = surf_data[1];
    normal_y = surf_data[2];
    normal_z = surf_data[3];
    p0_x = surf_data[4];
    p0_y = surf_data[5];
    p0_z = surf_data[6];
    p1_x = surf_data[7];
    p1_y = surf_data[8];
    p1_z = surf_data[9];

    /* Reset car mesh and particles if car was damaged / wrecked */
    if (veh->mesh_damage_flag != 0) {
        veh->detached_wheel_mask = 0;
        car_model = *(int *)(g_PlayerHUDState + g_ActiveVehicleIndex * 0x4c);
        if (*(int *)(g_PlayerHUDState + 4 + g_ActiveVehicleIndex * 0x4c) != 2 || g_ActiveVehicleIndex == 0) {
            veh->wreck_debris_flag = 0;
        }

        if (Lisa_SetDynamicObjectMesh(g_pCarBaseMeshes[car_model],
                                      g_PlayerHUDState,
                                      &g_pCarTransforms[g_ActiveVehicleIndex],
                                      (void *)g_pCarBaseMeshes[car_model],
                                      1,
                                      (short)(g_ActiveVehicleIndex + 100),
                                      0, -1, 0) != 0) {
            Log_DebugPrintf(s_Error_while_changing_car_mesh_00499248);
        }

        particle_dst = g_pParticleArray1 + g_ActiveVehicleIndex * 0x820;
        for (emitter_idx = 0; emitter_idx < 4; emitter_idx++) {
            template_idx = emitter_idx + car_model * 4;
            particle_count = *(int *)((uint8_t *)g_pParticleCount1 + template_idx * 12);
            if (particle_count > 0) {
                particle_src = (int *)(*(int *)((uint8_t *)g_ParticleTemplateX + template_idx * sizeof(int)) + 8);
                for (particle_idx = 0; particle_idx < particle_count; particle_idx++) {
                    particle_dst[particle_idx * 3] = *(int *)((uint8_t *)g_ParticleTemplateY + template_idx * sizeof(int)) + particle_src[particle_idx * 3];
                    particle_dst[particle_idx * 3 + 1] = particle_src[particle_idx * 3 + 1];
                    particle_dst[particle_idx * 3 + 2] = *(int *)((uint8_t *)g_ParticleTemplateZ + template_idx * sizeof(int)) + particle_src[particle_idx * 3 + 2];
                }
            }
            particle_dst += 0x208;
        }
        veh->mesh_damage_flag = 0;
    }

    /* Reset vehicle transforms and linear velocity */
    veh->pos_x = (double)avg_x;
    veh->pos_y = (double)avg_y;
    veh->pos_z = (double)avg_z;
    veh->vel_x = 0.0;
    veh->vel_y = 0.0;
    veh->vel_z = 0.0;
    veh->accel_x = 0.0;
    veh->accel_y = 0.0;
    veh->accel_z = 0.0;

    veh->angle_yaw = track_yaw;
    veh->angle_pitch = 0.0;
    veh->angle_roll = 0.0;
    veh->angular_vel_roll = 0.0;

    /* Axle positions from wheelbase offsets */
    cos_val = cos(veh->angle_yaw);
    sin_val = sin(veh->angle_yaw);
    veh->rear_axle_pos_x = veh->pos_x - cos_val * (double)veh->rear_axle_offset;
    veh->rear_axle_pos_z = veh->pos_z - sin_val * (double)veh->rear_axle_offset;
    veh->front_axle_pos_x = veh->pos_x + cos_val * (double)veh->front_axle_offset;
    veh->front_axle_pos_z = veh->pos_z + sin_val * (double)veh->front_axle_offset;

    veh->rear_axle_vel_x = 0.0;
    veh->rear_axle_vel_z = 0.0;
    veh->front_axle_vel_x = 0.0;
    veh->front_axle_vel_z = 0.0;

    /* Reset 4 wheel contact surface IDs to -1 */
    veh->wheel_surface_id[0] = -1;
    veh->wheel_surface_id[1] = -1;
    veh->wheel_surface_id[2] = -1;
    veh->wheel_surface_id[3] = -1;

    /* Interpolate ground height from surface triangle normal */
    ground_y = -((double)p0_y +
                 ((double)(p1_y - p0_y) * (double)normal_y +
                  (double)normal_x * ((double)(p1_x - p0_x) - (double)(avg_x - p0_x)) +
                  (double)normal_z * ((double)(p1_z - p0_z) - (double)(avg_z - p0_z))) /
                 (double)normal_y);

    veh->ground_y = ground_y;
    veh->ground_y_rear = ground_y;

    veh->is_airborne = 1;
    veh->checkpoint_pass1 = 0;
    veh->checkpoint_pass2 = 0;
    veh->checkpoint_pass3 = 0;
    veh->current_gear = 1;
    *(int *)(g_PlayerHUDState + 0x10 + g_ActiveVehicleIndex * 0x4c) = 0;
    veh->checkpoint_counter = 0;
    veh->throttle_input = 0;
    veh->brake_input = 0;
    veh->steering_angle = 0.0;
    veh->landing_impact = 0;
    veh->landing_flag = 0;
    veh->target_rpm = 0.0;
    veh->handbrake = 0;
    veh->surface_type = 0;
    veh->current_node_idx = g_TrackChunkToNodeTable[chunk_idx * 3];
    veh->target_node_idx = veh->current_node_idx;

    veh->angular_drag_x = 0.0;
    veh->angular_drag_y = 0.0;
    veh->angular_drag_z = 0.0;
    veh->wheel_angle_fl = 0.0;
    veh->wheel_angle_fr = 0.0;
    veh->wheel_rot_vel_fl = 0.0;
    veh->wheel_rot_vel_fr = 0.0;

    /* Chassis roll/pitch equilibrium angles */
    veh->chassis_roll_spring =
        (veh->angular_vel_roll * 8.0 + veh->wheel_rot_vel_fl) * (-0.2);
    veh->chassis_pitch_spring =
        (veh->angle_roll * 8.0 + veh->wheel_rot_vel_fr) * (-0.2);

    config->elevation_smooth_offset = 0.0;

    /* Camera state snapshot */
    *(double *)(cam_state + 0x28) = veh->angle_yaw;
    *(double *)(cam_state + 0x30) = 0.0;
    *(double *)(cam_state + 0x68) = veh->pos_x * 0.5;
    *(double *)(cam_state + 0x70) = veh->pos_z * 0.5;
    *(double *)(cam_state + 0x78) = 0.0;
    *(double *)(cam_state + 0x80) = 0.0;
    *(double *)(cam_state + 0x88) = 0.0;
    *(double *)(cam_state + 0x90) = 0.0;
    *(double *)(cam_state + 0x98) = 0.0;
    *(double *)(cam_state + 0xac) = 0.0;
    *(int *)(cam_state + 0xd4) = 0;

    Car_UpdateDynamicObjects(g_ActiveVehicleIndex);
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
    int top_rank;
    int *car_rank_ptr;
    int racer_idx;
    int best_car_idx;

    if (g_GameMode == 3) {
        top_rank = 1;
        best_car_idx = 0;
        if (0 < g_NumRacers) {
            car_rank_ptr = (int *)(g_Vehicles + 0x39c);
            for (racer_idx = 0; racer_idx < g_NumRacers; racer_idx++) {
                if ((car_rank_ptr[99] == 0) && (top_rank < *car_rank_ptr)) {
                    top_rank = *car_rank_ptr;
                    best_car_idx = racer_idx;
                }
                car_rank_ptr = (int *)((char *)car_rank_ptr + 0x484c);
            }
        }
        if (top_rank == 1) {
            top_rank = g_NumRacers + 1;
        }
        if (best_car_idx == g_MenuCursorPos) {
            g_CameraTargetHysteresis = 0;
        } else {
            g_CameraTargetHysteresis = g_CameraTargetHysteresis + 1;
            if (0x14 < g_CameraTargetHysteresis) {
                g_MenuCursorPos = best_car_idx;
                return top_rank;
            }
        }
    } else {
        top_rank = g_NumRacers + 1;
        best_car_idx = 0;
        if (0 < g_NumRacers) {
            car_rank_ptr = (int *)(g_Vehicles + 0x39c);
            for (racer_idx = 0; racer_idx < g_NumRacers; racer_idx++) {
                if ((car_rank_ptr[99] == 0) && (*car_rank_ptr < top_rank)) {
                    top_rank = *car_rank_ptr;
                    best_car_idx = racer_idx;
                }
                car_rank_ptr = (int *)((char *)car_rank_ptr + 0x484c);
            }
        }
        if (best_car_idx == g_MenuCursorPos) {
            g_CameraTargetHysteresis = 0;
            return top_rank;
        }
        g_CameraTargetHysteresis = g_CameraTargetHysteresis + 1;
        if (0x14 < g_CameraTargetHysteresis) {
            g_MenuCursorPos = best_car_idx;
            return top_rank;
        }
    }
    return top_rank;
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
    int text_w;
    int i;
    int restart_font_idx;
    int restart_font_unsel;
    int *prompt_surface;
    int *clear_ptr;
    int font_id;
    const char *cd_text;
    char text_buf[16];
    int sprite_coords[3];
    int screen_w;

    prompt_surface = (int *)(g_RenderTargetSurface + 0x25800);
    Gfx_SetRenderTarget(prompt_surface, 0x140, 0x140, 0x1e0, 8);
    g_pActiveDrawBuffer = g_pHudDrawCommandBuffer;

    clear_ptr = prompt_surface;
    for (i = 8000; i != 0; i--) {
        *clear_ptr++ = 0;
    }

    if ((g_MultiplayerMode == 2 || g_MultiplayerMode == 3) && g_TimeTrialActive != 1 &&
        g_IsAttractDemoMode != 1 && g_ChampionshipCredits > 0 && g_PlayerCarChoice < 6) {
        restart_font_idx = 1;
        restart_font_unsel = 0;
    } else {
        restart_font_idx = 4;
        restart_font_unsel = 4;
    }

    text_w = (int)Font_GetTextWidth((const char *)(s_CONTINUE_00494d58 + g_LanguageId * 0xd2), g_FontId_Medium);
    Menu_AddLayoutItem(0x39, 0, text_w, 0, 0);

    text_w = (int)Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
    Menu_AddLayoutItem(0x39, 1, text_w, 0, 0);

    text_w = (int)Font_GetTextWidth((const char *)(s_QUIT_00494d94 + g_LanguageId * 0xd2), g_FontId_Medium);
    Menu_AddLayoutItem(0x39, 2, text_w, 0, 0);

    text_w = (int)Font_GetTextWidth((const char *)(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2), g_FontId_Medium);
    Menu_AddLayoutItem(0x39, 3, text_w, 0, 0);

    Menu_LayoutItems(prompt_surface, 0x140);
    Menu_ClearLayout();

    if (g_AudioCdTrackMode == 0) {
        cd_text = s_DEFAULT_00494dee + g_LanguageId * 0xd2;
    } else if (g_AudioCdTrackMode == -1) {
        cd_text = s_RANDOM_00494e0c + g_LanguageId * 0xd2;
    } else {
        cd_text = "00";
    }

    font_id = (int)Font_GetTextWidth(cd_text, g_FontId_Small);
    Menu_AddLayoutItem(text_w + 0x57, 3, font_id, 0, 1);
    Menu_LayoutItems(prompt_surface, 0x140);
    Menu_ClearLayout();

    font_id = (g_MenuCursorPos == 0) ? g_FontId_Medium : g_FontId_Large;
    Font_DrawText(s_CONTINUE_00494d58 + g_LanguageId * 0xd2, font_id, 0x46, 0xc);

    font_id = (g_MenuCursorPos == 1) ? ((int *)&g_FontId_Large)[restart_font_idx] : ((int *)&g_FontId_Large)[restart_font_unsel];
    Font_DrawText(s_RESTART_00494d76 + g_LanguageId * 0xd2, font_id, 0x46, 0x1f);

    font_id = (g_MenuCursorPos == 2) ? g_FontId_Medium : g_FontId_Large;
    Font_DrawText(s_QUIT_00494d94 + g_LanguageId * 0xd2, font_id, 0x46, 0x32);

    font_id = (g_MenuCursorPos == 3) ? g_FontId_Medium : g_FontId_Large;
    Font_DrawText(s_CD_TRACK_00494db2 + g_LanguageId * 0xd2, font_id, 0x46, 0x45);

    if (g_AudioCdTrackMode == 0) {
        sprintf(text_buf, "%s", s_DEFAULT_00494dee + g_LanguageId * 0xd2);
    } else if (g_AudioCdTrackMode == -1) {
        sprintf(text_buf, "%s", s_RANDOM_00494e0c + g_LanguageId * 0xd2);
    } else {
        sprintf(text_buf, "%d", g_CurrentCdTrackNumber);
    }

    ((int *)&g_FontAlignMode)[g_FontId_Small * 400] = 1;
    Font_DrawText(text_buf, g_FontId_Small, font_id / 2 + text_w + 100, 0x45);
    ((int *)&g_FontAlignMode)[g_FontId_Small * 400] = 0;

    if (g_GameMode == 0) {
        text_w = (int)Font_GetTextWidth((const char *)(s_RESTART_00494d76 + g_LanguageId * 0xd2), g_FontId_Medium);
        if (g_ChampionshipCredits > 0) {
            int spr_x = (text_w + 0x67) * 256;
            for (i = 0; i < g_ChampionshipCredits; i++) {
                sprite_coords[1] = 0x1b00;
                sprite_coords[0] = spr_x;
                Gfx_DrawSprite(g_pCreditIconSprite, sprite_coords, 0);
                spr_x += 0x1a00;
            }
        }
    }

    screen_w = g_ScreenWidth;
    Gfx_BlitTransparentLUT((int)prompt_surface, 0, 0, 0x140, 100, (void *)g_VirtualFramebuffer,
                           screen_w / 2 - 0xa0, g_ScreenHeight / 2 - 0x32,
                           g_pLisaDrawCommandWritePtr, 0x140, screen_w);
    Gfx_SetRenderTarget(&g_VirtualFramebuffer, screen_w, screen_w, g_ScreenHeight, 8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
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
    g_pActiveDrawBuffer = g_pHudDrawCommandBuffer;

    clear_ptr = prompt_surface;
    for (clear_count = 8000; clear_count != 0; clear_count--) {
        *clear_ptr++ = 0;
    }

    if (g_ConfirmationPromptType == 2) {
        text_width = (int)Font_GetTextWidth((const char *)(s_RESTART___Y_N__00495300 + g_LanguageId * 0x1e), g_FontId_Medium);
    } else if (g_ConfirmationPromptType == 1) {
        text_width = (int)Font_GetTextWidth((const char *)(s_QUIT___Y_N__00495248 + g_LanguageId * 0x1e), g_FontId_Medium);
    } else {
        text_width = 0;
    }

    Menu_AddLayoutItem(0x94 - text_width / 2, 0, text_width, 0, 0);
    Menu_LayoutItems(prompt_surface, 0x140);
    Menu_ClearLayout();

    font_id = g_FontId_Medium;
    ((int *)&g_FontAlignMode)[g_FontId_Medium * 400] = 1;
    if (g_ConfirmationPromptType == 2) {
        prompt_text = s_RESTART___Y_N__00495300 + g_LanguageId * 0x1e;
        Font_DrawText(prompt_text, font_id, 0xa0, 0xc);
    } else if (g_ConfirmationPromptType == 1) {
        prompt_text = s_QUIT___Y_N__00495248 + g_LanguageId * 0x1e;
        Font_DrawText(prompt_text, font_id, 0xa0, 0xc);
    }

    screen_w = g_ScreenWidth;
    ((int *)&g_FontAlignMode)[g_FontId_Medium * 400] = 0;
    Gfx_BlitTransparentLUT((int)prompt_surface, 0, 0, 0x140, 0x22, (void *)g_VirtualFramebuffer,
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
      _sprintf(var_28,s_QUIT_00494d94 + g_LanguageId * 0xd2);
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
    int racer_rank_map[8];
    int lap_idx;
    int racer_idx;
    int *pVehicleRank;
    int font_small;
    int font_menu;
    int spr_coords[2];
    int item_y;
    int spr_x;
    int i;
    void **pCarIcon;

    for (lap_idx = 0; lap_idx < g_LapsTotal; lap_idx++) {
        for (racer_idx = 0; racer_idx < g_NumRacers; racer_idx++) {
            pVehicleRank = (int *)(g_Vehicles + 0x3a0 + racer_idx * 0x484c);
            if (*pVehicleRank - lap_idx == 1) {
                racer_rank_map[lap_idx] = racer_idx;
                break;
            }
        }
    }

    Font_GetTextWidth((const char *)(s_TRACK_RESULTS_00495470 + g_LanguageId * 0x1e), g_FontId_Medium);
    Gfx_SetRenderTarget(g_RenderTargetSurface, 0x140, 0x140, 0x1e0, 8);
    font_small = g_FontId_Small;
    g_pActiveDrawBuffer = g_pHudDrawCommandBuffer;

    if (g_MultiplayerMode == 4) {
        ((int *)&g_FontAlignMode)[font_small * 400] = 1;
        Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e, font_small, 0xa0, 0x19c);
        ((int *)&g_FontAlignMode)[font_small * 400] = 0;

        Gfx_SetRenderTarget(g_RenderTargetSurface, 0x280, 0x280, 0xf0, 8);
        font_menu = g_FontId_Menu;
        ((int *)&g_FontAlignMode)[font_menu * 400] = 1;
        Font_DrawText(s_WAITING_FOR_HOST_004957a8 + g_LanguageId * 0x1e, font_menu, 0x140, 0xd3);
        ((int *)&g_FontAlignMode)[font_menu * 400] = 0;
    } else {
        if (g_GameMode != 0) {
            ((int *)&g_FontAlignMode)[font_small * 400] = 1;
            Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d, font_small, 0xa0, 0x19c);
            ((int *)&g_FontAlignMode)[font_small * 400] = 0;

            Gfx_SetRenderTarget(g_RenderTargetSurface, 0x280, 0x280, 0xf0, 8);
            font_menu = g_FontId_Menu;
            ((int *)&g_FontAlignMode)[font_menu * 400] = 1;
            Font_DrawText(s_PLAY_TRACK_AGAIN___Y_N__00495698 + g_LanguageId * 0x2d, font_menu, 0x140, 0xd3);
            ((int *)&g_FontAlignMode)[font_menu * 400] = 0;

            Gfx_SetRenderTarget(g_RenderTargetSurface, 0x140, 0x140, 0x1e0, 8);
            g_PlayAgainPromptActive = 1;
            goto layout_racers;
        }

        Font_GetTextWidth((const char *)(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32), font_small);
        ((int *)&g_FontAlignMode)[font_small * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32, font_small, 0xa0, 0x19c);
        ((int *)&g_FontAlignMode)[font_small * 400] = 0;

        Gfx_SetRenderTarget(g_RenderTargetSurface, 0x280, 0x280, 0xf0, 8);
        font_menu = g_FontId_Menu;
        ((int *)&g_FontAlignMode)[font_menu * 400] = 1;
        Font_DrawText(s_PRESS_RETURN_TO_CONTINUE_00495860 + g_LanguageId * 0x32, font_menu, 0x140, 0xd3);
        ((int *)&g_FontAlignMode)[font_menu * 400] = 0;
    }

    Gfx_SetRenderTarget(g_RenderTargetSurface, 0x140, 0x140, 0x1e0, 8);

layout_racers:
    if (g_NumRacers > 0) {
        item_y = 0x28;
        pCarIcon = (void **)&g_pCarIconSprites;
        spr_x = 0x3000;
        for (i = 0; i < g_NumRacers; i++) {
            if (g_GameMode == 3) {
                spr_coords[1] = spr_x;
                spr_coords[0] = 0x7000;
                Gfx_DrawSprite(*pCarIcon, spr_coords, 0);
                Menu_AddLayoutItem(0x8b, 0, 0x28, item_y, 0);
                Menu_LayoutItems(g_RenderTargetSurface, 0x140);
                Menu_ClearLayout();
            } else {
                spr_coords[1] = spr_x;
                spr_coords[0] = 0x3400;
                Gfx_DrawSprite(*pCarIcon, spr_coords, 0);
                Menu_AddLayoutItem(0x4f, 0, 0x28, item_y, 0);
                Menu_LayoutItems(g_RenderTargetSurface, 0x140);
                Menu_ClearLayout();
                Menu_AddLayoutItem(0x92, 0, 0x5f, item_y, 0);
                Menu_LayoutItems(g_RenderTargetSurface, 0x140);
                Menu_ClearLayout();

                if (g_ShowSecondPlayerFlag == 1) {
                    spr_coords[1] = spr_x + 0x3c00;
                    spr_coords[0] = 0x3400;
                    Gfx_DrawSprite(pCarIcon[1], spr_coords, 0);
                    Menu_AddLayoutItem(0x4f, 0, 0x28, item_y + 0x3c, 0);
                    Menu_LayoutItems(g_RenderTargetSurface, 0x140);
                    Menu_ClearLayout();
                    Menu_AddLayoutItem(0x92, 0, 0x5f, item_y + 0x3c, 0);
                    Menu_LayoutItems(g_RenderTargetSurface, 0x140);
                    Menu_ClearLayout();
                }
            }
            item_y += 0x3c;
            pCarIcon++;
            spr_x += 0x3c00;
        }
    }

    Gfx_SetRenderTarget((int *)g_VirtualFramebuffer, g_ScreenWidth, g_ScreenWidth, g_ScreenHeight, 8);
    g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;
}

/**
 * @original Camera_UpdateChase (IGN_WIN.EXE @ 0x0043c910, fx.c)
 * @fidelity ADAPTED
 */
void Camera_UpdateChase(void) {
    VehicleState *veh;
    VehicleConfig *config;
    double smooth_divisor;
    double cos_yaw;
    double sin_yaw;
    double yaw_diff;
    double pitch_diff;
    double track_pitch;
    double pitch_offset;
    double incline_term;
    double road_incline_target;
    double min_height;
    int road_node;
    int node_idx;
    int road_type;

    smooth_divisor = (g_RaceTimer_P2 >= 0.0) ? 25.0 : 10.0;
    veh = (VehicleState *)(g_Vehicles + g_ActiveVehicleIndex * 0x484c);
    config = (VehicleConfig *)(g_VehicleConfigs + g_ActiveVehicleIndex * 200);

    if (veh->turbo_active == 0 || *(int *)((uint8_t *)veh + 0x604) == 1) {
        cos_yaw = cos(config->camera_yaw);
        config->target_pos_x = cos_yaw * (double)config->chase_camera_distance * (-0.5) + veh->pos_x;
        config->target_pos_y = veh->pos_y;
        sin_yaw = sin(config->camera_yaw);
        config->target_pos_z = sin_yaw * (double)config->chase_camera_distance * (-0.5) + veh->pos_z;
        config->vehicle_yaw = veh->angle_yaw;
        if (*(int *)((uint8_t *)veh + 0x554) == 0) {
            config->vehicle_pitch = veh->angle_pitch;
        }
    }

    cos_yaw = cos(config->camera_yaw);
    config->cam_pos_x = cos_yaw * (double)config->chase_camera_distance * 0.5 + veh->pos_x;
    config->cam_pos_y = veh->pos_y;
    sin_yaw = sin(config->camera_yaw);
    config->cam_pos_z = sin_yaw * (double)config->chase_camera_distance * 0.5 + veh->pos_z;
    config->camera_yaw_desired = veh->angle_yaw + 3.141592654;

    /* Camera zoom distance factor easing */
    if (veh->crash_flag1 == 0 && veh->crash_flag2 == 0 && veh->crash_flag3 == 0) {
        config->chase_cam_dist_factor *= 0.98;
    } else {
        config->chase_cam_dist_factor += (150.0 - config->chase_cam_dist_factor) * 0.03333333333;
    }

    /* Camera yaw lag */
    yaw_diff = config->camera_yaw - config->vehicle_yaw;
    if (yaw_diff > 3.141592654) {
        yaw_diff -= 6.283185307;
    }
    if (yaw_diff < -3.141592654) {
        yaw_diff += 6.283185307;
    }
    config->camera_yaw += yaw_diff * (-0.125);
    if (config->camera_yaw >= 6.283185307) {
        config->camera_yaw -= 6.283185307;
    }
    if (config->camera_yaw <= 0.0) {
        config->camera_yaw += 6.283185307;
    }

    config->yaw_lag_angle = config->camera_yaw * (-572.9577951) + 900.0;
    if (config->yaw_lag_angle < 0.0) {
        config->yaw_lag_angle += 3600.0;
    }
    if (config->yaw_lag_angle > 3599.0) {
        config->yaw_lag_angle -= 3600.0;
    }

    /* Camera pitch lag */
    pitch_diff = config->camera_pitch - config->camera_yaw_desired;
    if (pitch_diff > 3.141592654) {
        pitch_diff -= 6.283185307;
    }
    if (pitch_diff < -3.141592654) {
        pitch_diff += 6.283185307;
    }
    config->camera_pitch += pitch_diff * (-0.125);
    if (config->camera_pitch >= 6.283185307) {
        config->camera_pitch -= 6.283185307;
    }
    if (config->camera_pitch <= 0.0) {
        config->camera_pitch += 6.283185307;
    }

    config->pitch_lag_angle = config->camera_pitch * (-572.9577951) + 900.0;
    if (config->pitch_lag_angle < 0.0) {
        config->pitch_lag_angle += 3600.0;
    }
    if (config->pitch_lag_angle > 3599.0) {
        config->pitch_lag_angle -= 3600.0;
    }

    /* Track road pitch */
    road_node = *(int *)((uint8_t *)veh + 0x364);
    node_idx = (road_node < 0) ? ~road_node : road_node;

    if (*(int *)((uint8_t *)veh + 0x36c) == 3) {
        track_pitch = (*(double *)(g_pTrackRoadSequence + 4 + node_idx * 0x18) +
                       *(double *)(g_pTrackRoadSequence + 0x10 + node_idx * 0x18)) * 0.5;
    } else if (road_node < 0) {
        track_pitch = *(double *)(g_pTrackRoadSequence + 0x10 + node_idx * 0x18);
    } else {
        track_pitch = *(double *)(g_pTrackRoadSequence + 4 + road_node * 0x18);
    }

    if (veh->turbo_active == 0) {
        cos_yaw = cos(((config->camera_yaw - track_pitch) - 1.570796327) * 2.0 + 3.141592654);
        pitch_offset = (cos_yaw + 1.0) * 50.0;
    } else {
        pitch_offset = 0.0;
    }

    if (veh->crash_flag3 == 0) {
        incline_term = config->vehicle_pitch * 114.591559;
    } else {
        incline_term = 0.0;
    }

    if (config->pitch_offset > 3600.0 || config->pitch_offset < -3600.0) {
        g_CameraShakeActive = 0;
    }

    /* Elevation offset smoothing */
    config->pitch_offset = ((((config->cam_height - (double)config->chase_target_height) - config->target_pos_y) * 0.5 +
                            pitch_offset * 3.0 + incline_term) - config->pitch_offset) / smooth_divisor + config->pitch_offset;

    /* Road incline response */
    if (road_node < 0) {
        road_type = *(int *)(g_pTrackRoadSequence + 0xc + node_idx * 0x18);
    } else {
        road_type = *(int *)(g_pTrackRoadSequence + road_node * 0x18);
    }
    road_type = *(int *)(g_TrackSegmentTable + 4 + road_type * 0xc);

    road_incline_target = 0.0;
    if (road_type >= 1 && road_type < 11) {
        road_incline_target = (double)(road_type * -50);
    } else if (road_type > 10 && road_type < 21) {
        road_incline_target = (double)((road_type * 5 - 50) * 10);
    }

    if (config->elevation_smooth_offset < road_incline_target) {
        config->elevation_smooth_offset += *(double *)((uint8_t *)veh + 0x118) * 0.3333333333;
    }
    if (road_incline_target < config->elevation_smooth_offset) {
        config->elevation_smooth_offset += *(double *)((uint8_t *)veh + 0x118) * (-0.3333333333);
    }

    /* Target elevation */
    config->cam_height = ((double)config->chase_target_height -
                         ((pitch_offset * -2.0 - config->target_pos_y) + config->elevation_smooth_offset + config->cam_height)) /
                         smooth_divisor + config->cam_height;

    min_height = config->target_pos_y + 200.0;
    if (config->cam_height < min_height) {
        config->cam_height = min_height;
    }

    config->chase_camera_distance += (int)((400.0 - (double)config->chase_camera_distance) * 0.1);
}

/**
 * @original Car_UpdateDynamicObjects (IGN_WIN.EXE @ 0x0043d540, fx.c)
 * @fidelity ADAPTED
 */
void Car_UpdateDynamicObjects(int car_idx) {
    VehicleState *veh;
    DynamicObjectTransform *car_xform;
    DynamicObjectTransform *wheel_xform;
    DynamicObjectTransform *refl_xform;
    DynamicObjectTransform *shadow_xform;
    DynamicObjectTransform *ghost_xform;
    int *shadow_quad;
    double angle_rad;
    double cos_ang;
    double sin_ang;
    double shadow_x;
    double shadow_z;
    double diff_pos;
    int car_model;
    int i;
    int emitter_idx;
    int count;
    int *out_emitter;
    int *player_hud;

    veh = (VehicleState *)(g_Vehicles + car_idx * 0x484c);
    player_hud = (int *)(g_PlayerHUDState + car_idx * 0x4c);
    car_model = *player_hud;

    /* Primary car transform */
    car_xform = &g_CarTransforms[car_idx];
    car_xform->pos_x = (int)veh->pos_x;
    car_xform->pos_y = (int)veh->pos_y;
    car_xform->pos_z = (int)veh->pos_z;
    car_xform->rot_x = (int)*(double *)((uint8_t *)veh + 0x584);
    car_xform->rot_y = (int)(veh->angle_yaw * 572.9577951);
    car_xform->rot_z = (int)*(double *)((uint8_t *)veh + 0x58c);

    if (g_DynamicObjectsPaused == 0) {
        if (Lisa_UpdateObjectSpatialGrid(car_xform) != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
        }
    }

    /* Particle emitter coordinate transforms */
    angle_rad = *(double *)((uint8_t *)veh + 0xb0) * 0.01745329252;
    cos_ang = cos(angle_rad);
    sin_ang = sin(angle_rad);

    if (*(int *)((uint8_t *)veh + 0x558) == 0) {
        count = *(int *)(g_pParticleCount1 + car_model * 0x30 / 4);
        if (count > 0) {
            out_emitter = &g_pParticleArray1[car_idx * 0x820 / 4];
            for (i = 0; i < count; i++) {
                double tmpl_x = (double)*(int *)(g_ParticleTemplateX + car_model * 4 + i);
                double tmpl_y = (double)*(int *)(g_ParticleTemplateY + car_model * 4 + i);
                double tmpl_z = (double)*(int *)(g_ParticleTemplateZ + car_model * 4 + i);
                out_emitter[0] = (int)(tmpl_x * cos_ang - tmpl_y * sin_ang);
                out_emitter[2] = (int)tmpl_z;
                out_emitter += 3;
            }
        }

        count = *(int *)(g_pParticleCount1 + (car_model * 0x30 + 0xc) / 4);
        if (count > 0) {
            out_emitter = &g_pParticleArray2[car_idx * 0x2080 / 4];
            for (i = 0; i < count; i++) {
                double tmpl_x = (double)*(int *)(g_ParticleTemplateX + car_model * 4 + i);
                double tmpl_y = (double)*(int *)(g_ParticleTemplateY + car_model * 4 + i);
                double tmpl_z = (double)*(int *)(g_ParticleTemplateZ + car_model * 4 + i);
                out_emitter[0] = (int)(tmpl_x * cos_ang - tmpl_y * sin_ang);
                out_emitter[2] = (int)tmpl_z;
                out_emitter += 3;
            }
        }
    } else {
        out_emitter = &g_pParticleArray1[car_idx * 0x820 / 4];
        for (emitter_idx = 0; emitter_idx < 4; emitter_idx++) {
            count = *(int *)(g_pParticleCount1 + (emitter_idx + car_model * 4) * 3);
            if (count > 0) {
                for (i = 0; i < count; i++) {
                    double tmpl_x = (double)*(int *)(g_ParticleTemplateX + (emitter_idx + car_model * 4) * 4 + i);
                    double tmpl_y = (double)*(int *)(g_ParticleTemplateY + (emitter_idx + car_model * 4) * 4 + i);
                    double tmpl_z = (double)*(int *)(g_ParticleTemplateZ + (emitter_idx + car_model * 4) * 4 + i);
                    out_emitter[0] = (int)(tmpl_x * cos_ang - tmpl_y * sin_ang);
                    out_emitter[1] = (int)(tmpl_x * sin_ang + tmpl_y * cos_ang);
                    out_emitter[2] = (int)(tmpl_z * 1.5);
                    out_emitter += 3;
                }
            }
            out_emitter += 0x208 / 4;
        }
    }

    /* 4 Wheel transforms */
    for (i = 0; i < 4; i++) {
        wheel_xform = &g_WheelTransforms[car_idx * 4 + i];
        wheel_xform->pos_x = (int)veh->pos_x;
        wheel_xform->pos_y = (int)veh->pos_y;
        wheel_xform->pos_z = (int)veh->pos_z;
        wheel_xform->rot_x = (int)(*(double *)((uint8_t *)veh + 0x108) * 572.9577951);
        wheel_xform->rot_y = (int)(veh->angle_yaw * 572.9577951);
        wheel_xform->rot_z = (int)(*(double *)((uint8_t *)veh + 0x110) * 572.9577951);

        if (Lisa_UpdateObjectSpatialGrid(wheel_xform) != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_WHE_00499580);
        }
    }

    /* Police car reflection (car model 7) */
    if (car_model == 7 && g_DynamicObjectsPaused == 0) {
        refl_xform = &g_CarReflectionTransforms[car_idx];
        refl_xform->pos_x = (int)veh->pos_x;
        refl_xform->pos_y = (int)veh->pos_y;
        refl_xform->pos_z = (int)veh->pos_z;
        refl_xform->rot_x = (int)*(double *)((uint8_t *)veh + 0x584);
        refl_xform->rot_y = (int)(veh->angle_yaw * 572.9577951);
        refl_xform->rot_z = (int)*(double *)((uint8_t *)veh + 0x58c);

        if (Lisa_UpdateObjectSpatialGrid(refl_xform) != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4);
        }
    }

    /* Shadow quad geometry vertices */
    shadow_quad = &g_CarShadowVertices[car_idx * 30];
    shadow_quad[0] = (int)*(double *)((uint8_t *)veh + 0x30c);
    shadow_quad[2] = (int)(*(double *)((uint8_t *)veh + 0x32c) * 1.2);
    shadow_quad[3] = (int)*(double *)((uint8_t *)veh + 0x314);
    shadow_quad[5] = (int)(*(double *)((uint8_t *)veh + 0x334) * 1.2);
    shadow_quad[6] = (int)*(double *)((uint8_t *)veh + 0x304);
    shadow_quad[8] = (int)(*(double *)((uint8_t *)veh + 0x324) * 1.2);
    shadow_quad[9] = (int)*(double *)((uint8_t *)veh + 0x2fc);
    shadow_quad[11] = (int)(*(double *)((uint8_t *)veh + 0x31c) * 1.2);

    /* Shadow dynamic object transform */
    diff_pos = veh->pos_y - *(double *)((uint8_t *)veh + 0x120);
    shadow_x = veh->pos_x + diff_pos;
    shadow_z = veh->pos_z + diff_pos;
    if (shadow_x <= 0.0 || shadow_x >= 50000.0) {
        shadow_x = 28000.0;
    }
    if (veh->pos_y < -2500.0 || veh->pos_y > 2500.0) {
        shadow_x = 0.0;
    }
    if (shadow_z <= 0.0 || shadow_z >= 50000.0) {
        shadow_z = 28000.0;
    }

    shadow_xform = &g_CarShadowTransforms[car_idx];
    shadow_xform->pos_x = (int)shadow_x;
    shadow_xform->pos_y = (int)veh->pos_y;
    shadow_xform->pos_z = (int)shadow_z;
    shadow_xform->rot_x = (int)(*(double *)((uint8_t *)veh + 0x108) * 572.9577951);
    shadow_xform->rot_y = (int)(veh->angle_yaw * 572.9577951);
    shadow_xform->rot_z = (int)(*(double *)((uint8_t *)veh + 0x110) * 572.9577951);

    if (g_DynamicObjectsPaused == 0) {
        if (Lisa_UpdateObjectSpatialGrid(shadow_xform) != 0) {
            Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_SHA_00499558);
        }
    }

    /* Ghost car dynamic object transform */
    if (player_hud[1] == 2 && car_idx != 0) {
        ghost_xform = &g_CarGhostTransforms[car_idx];
        ghost_xform->pos_x = (int)veh->pos_x;
        ghost_xform->pos_y = (int)veh->pos_y;
        ghost_xform->pos_z = (int)veh->pos_z;
        ghost_xform->rot_x = 0;
        ghost_xform->rot_y = 0;
        ghost_xform->rot_z = 0;

        if (ghost_xform->is_placed == 1) {
            if (Lisa_UpdateObjectSpatialGrid(ghost_xform) != 0) {
                Log_DebugPrintf(s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_00499530);
                return;
            }
        } else {
            if (Lisa_MoveDynamicObject(ghost_xform) != 0) {
                Log_DebugPrintf(s_FEL_VID_LI_PLACEOBJECT_HANDLE_CA_00499508);
            }
        }
    }
}

/**
 * @original Video_SetGraphicsMode (IGN_WIN.EXE @ 0x0043dea0, fx.c)
 * @fidelity ADAPTED
 */
int Video_SetGraphicsMode(void) {
    int i;
    int *config;

    if (g_GraphicsResolutionMode == 0) {
        g_ScreenWidth = 320;
        g_ScreenHeight = 200;
        g_ColorDepth = 8;
        g_VideoModeActive = 1;
        if (!Gfx_RestoreSurface()) {
            sprintf(g_ErrorMessageBuffer, "%s", s_Cannot_use_this_graphics_mode__004989dc);
            g_GraphicsInitErrorFlag = 1;
            return 0;
        }
        for (i = 0; i < g_NumRacers; i++) {
            config = (int *)(g_VehicleConfigs + i * 200);
            config[0x58 / 4] = 200;
            config[0x5c / 4] = 164;
            config[0x60 / 4] = 0;
            config[100 / 4] = 0x40390000;
        }
        Lisa_ResetRasterizerContext();
        if (g_WeatherActive[0] == 1 && g_IsSplitScreen == 0 &&
            g_WeatherViewportCount == 1 && g_WeatherType == 1) {
            Gfx_FreeSurface((int)Palette_AdjustRGB(0, g_pRainMesh_P2[1] >> 31, g_ActiveTrackPalette + 8));
        }
    } else if (g_GraphicsResolutionMode == 1) {
        g_ScreenWidth = 640;
        g_ScreenHeight = 480;
        g_ColorDepth = 8;
        g_VideoModeActive = 1;
        if (!Gfx_RestoreSurface()) {
            sprintf(g_ErrorMessageBuffer, "%s", s_Cannot_use_this_graphics_mode__004989dc);
            g_GraphicsInitErrorFlag = 1;
            return 0;
        }
        for (i = 0; i < g_NumRacers; i++) {
            config = (int *)(g_VehicleConfigs + i * 200);
            config[0x58 / 4] = 425;
            config[0x5c / 4] = 425;
            config[0x60 / 4] = 0;
            config[100 / 4] = 0x40240000;
        }
        Lisa_ResetRasterizerContext();
        if (g_WeatherActive[0] == 1 && g_IsSplitScreen == 0 &&
            g_WeatherViewportCount == 1 && g_WeatherType == 1) {
            Gfx_FreeSurface((int)Palette_AdjustRGB(0, g_pRainMesh_P2[1] >> 31, g_ActiveTrackPalette + 8));
        }
    } else if (g_GraphicsResolutionMode == 2) {
        g_ScreenWidth = 800;
        g_ScreenHeight = 600;
        g_ColorDepth = 8;
        g_VideoModeActive = 1;
        if (!Gfx_RestoreSurface()) {
            sprintf(g_ErrorMessageBuffer, "%s", s_Cannot_use_this_graphics_mode__004989dc);
            g_GraphicsInitErrorFlag = 1;
            return 0;
        }
        for (i = 0; i < g_NumRacers; i++) {
            config = (int *)(g_VehicleConfigs + i * 200);
            config[0x58 / 4] = 550;
            config[0x5c / 4] = 550;
            config[0x60 / 4] = 0;
            config[100 / 4] = 0x40240000;
        }
        Lisa_ResetRasterizerContext();
        if (g_WeatherActive[0] == 1 && g_IsSplitScreen == 0 &&
            g_WeatherViewportCount == 1 && g_WeatherType == 1) {
            Gfx_FreeSurface((int)Palette_AdjustRGB(0, g_pRainMesh_P2[1] >> 31, g_ActiveTrackPalette + 8));
        }
    }

    Gfx_SetRenderTarget(&g_VirtualFramebuffer, g_ScreenWidth, g_ScreenWidth, g_ScreenHeight, 8);
    memset(&g_VirtualFramebuffer, 0, g_ScreenWidth * g_ScreenHeight);
    Lisa_Init();
    Timer_GetDeltaTime();
    return 0;
}

/**
 * @original Lisa_Init (IGN_WIN.EXE @ 0x0043e2a0, fx.c)
 * @fidelity ADAPTED
 */
void Lisa_Init(void) {
    int i;

    if (g_IsSplitScreen != 0 && g_SplitScreenMode != 1) {
        g_CameraViewportWidth = (int)((double)g_ScreenWidth * g_ViewportScreenScaleTable[g_ScreenSizeSetting][0] * 0.5);
    } else {
        g_CameraViewportWidth = (int)((double)g_ScreenWidth * g_ViewportScreenScaleTable[g_ScreenSizeSetting][0]);
    }
    g_CameraViewportHeight = (int)((double)g_ScreenHeight * g_ViewportScreenScaleTable[g_ScreenSizeSetting][1]);
    g_CameraFovScaleX = (int)((double)g_ScreenWidth * 204.796875);
    g_CameraFovScaleY = (int)((double)g_ScreenHeight * 327.675);

    if (g_IsSplitScreen == 0 || g_SplitScreenMode == 1) {
        for (i = 0; i < g_NumRacers; i++) {
            int *hud_vp = (int *)(g_PlayerHUDState + i * 0x4c);
            hud_vp[0x2c / 4] = g_ScreenWidth / 2 - g_CameraViewportWidth / 2;
            hud_vp[0x30 / 4] = g_ScreenHeight / 2 - g_CameraViewportHeight / 2;
            hud_vp[0x34 / 4] = g_ScreenWidth / 2 + g_CameraViewportWidth / 2;
            hud_vp[0x38 / 4] = g_ScreenHeight / 2 + g_CameraViewportHeight / 2;
        }
    } else {
        for (i = 0; i < g_NumRacers; i++) {
            int *hud_vp = (int *)(g_PlayerHUDState + i * 0x4c);
            int *cfg = (int *)(g_VehicleConfigs + i * 200);
            int *cfg0 = (int *)g_VehicleConfigs;

            hud_vp[0x2c / 4] = (g_ScreenWidth * 3 / 4) - g_CameraViewportWidth / 2 + 1;
            hud_vp[0x30 / 4] = g_ScreenHeight / 2 - g_CameraViewportHeight / 2;
            hud_vp[0x34 / 4] = (g_ScreenWidth * 3 / 4) + g_CameraViewportWidth / 2;
            hud_vp[0x38 / 4] = g_ScreenHeight / 2 + g_CameraViewportHeight / 2;

            cfg[0x58 / 4] = cfg0[0x58 / 4];
            cfg[0x5c / 4] = cfg0[0x5c / 4];
            cfg[100 / 4] = cfg0[100 / 4];
            cfg[0x60 / 4] = cfg0[0x60 / 4];
        }
        *(int *)(g_PlayerHUDState + 0x78) = (g_ScreenWidth / 4) - g_CameraViewportWidth / 2;
        *(int *)(g_PlayerHUDState + 0x7c) = g_ScreenHeight / 2 - g_CameraViewportHeight / 2;
        *(int *)(g_PlayerHUDState + 0x80) = (g_ScreenWidth / 4) + g_CameraViewportWidth / 2 - 1;
        *(int *)(g_PlayerHUDState + 0x84) = g_ScreenHeight / 2 + g_CameraViewportHeight / 2;
    }

    Lisa_SetCameraViewport();
    g_ViewportMinX = 0;
    g_ViewportMaxX = g_ScreenWidth - 1;
    g_ViewportMinY = 0;
    g_ViewportMaxY = g_ScreenHeight - 1;

    if (Audio_LoadAssets(g_ScreenWidth, g_ScreenHeight) >= 0) {
        g_pLisaCommandQueueMirror = g_pLisaDrawCommandQueue;
        g_pLisaActiveTABMirror = g_pActiveTAB;
        g_pLisaFramebufferMirror1 = &g_VirtualFramebuffer;
        g_pLisaFramebufferMirror2 = &g_VirtualFramebuffer;
        Audio_StopSample();
        if (Audio_LoadAssets(g_ScreenHeight, g_ScreenWidth) >= 0) {
            g_CameraFovHalfX = g_CameraFovScaleX * 500;
            g_CameraFovHalfY = g_CameraFovScaleY * 500;
            return;
        }
    }

    Log_DebugPrintf(s_Error_while_initializing_lisaGM_004995c8);
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
    char text_buf[32];
    int roll_tenths;

    if (g_ShowFpsOverlay != 0) {
        _sprintf(text_buf, s_FPS__d_00499658, 0);
        Font_PrintDirect(0x10e, 0, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    }
    if (g_ShowRecordingOverlay == 1) {
        _sprintf(text_buf, s_RECORDING__0049964c);
        Font_PrintDirect(0x96, 10, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    }
    if (g_ShowRollTelemetry == 1) {
        roll_tenths = g_TelemetryRollAngle / 10;
        _sprintf(text_buf, s_ROLL___1f_00499640, (double)roll_tenths);
        Font_PrintDirect(5, 0x1e, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
        return;
    }
    _sprintf(text_buf, s_OLDROADNR__d_00499630);
    Font_PrintDirect(5, 10, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    _sprintf(text_buf, s_SQUASHED2__d_00499620);
    Font_PrintDirect(5, 0x14, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    _sprintf(text_buf, s_TYPE1__d_00499614);
    Font_PrintDirect(0, 0x28, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    _sprintf(text_buf, s_TYPE2__d_00499608);
    Font_PrintDirect(0, 0x32, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    _sprintf(text_buf, s_SKILLNAD___3f_004995f8,
             ABS(*(double *)(g_Vehicles + 0x128) - *(double *)(g_Vehicles + 0x120)));
    Font_PrintDirect(0, 0x3c, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
    _sprintf(text_buf, s_SPEED___3f_004995ec, ABS(*(double *)(g_Vehicles + 0x118)));
    Font_PrintDirect(0, 0x46, text_buf, (void*)0x563db0, g_ScreenWidth, g_ScreenHeightAlt, '\x14');
}

/**
 * @original HUD_CheckWrongWayHeading (IGN_WIN.EXE @ 0x0043edf0, fx.c)
 * @fidelity ADAPTED
 */
void HUD_CheckWrongWayHeading(int player_idx) {
    int car_base_offset;
    int *warning_timer_ptr;
    double track_heading;
    unsigned int road_node_idx;
    double track_angle_wrapped;
    double car_heading_wrapped;
    __int64 ftol_result;

    car_base_offset = player_idx * 0x484c;
    road_node_idx = *(unsigned int *)(g_Vehicles + 0x364 + player_idx * 0x484c);
    if ((int)road_node_idx < 0) {
        track_heading = *(double *)(g_pTrackRoadSequence + 0x10 + ((road_node_idx ^ (int)road_node_idx >> 0x1f) - ((int)road_node_idx >> 0x1f)) * 0x18);
    } else {
        track_heading = *(double *)(g_pTrackRoadSequence + 4 + road_node_idx * 0x18);
    }

    track_angle_wrapped = Math_WrapAngle(track_heading - k_WrongWayTrackHeadingOffset, 6.2831853071796);
    car_heading_wrapped = Math_WrapAngle((double)CONCAT44(*(int *)(g_Vehicles + 0xfc + car_base_offset),
                                                          *(int *)(g_Vehicles + 0xf8 + car_base_offset)),
                                         6.2831853071796);

    if (((double)ABS((double)track_angle_wrapped - car_heading_wrapped) <= k_WrongWayThresholdMin) ||
        (k_WrongWayThresholdMax <= (double)ABS((double)track_angle_wrapped - car_heading_wrapped))) {
        warning_timer_ptr = (int *)(g_Vehicles + 0x580 + car_base_offset);
        if (0 < *warning_timer_ptr) {
            ftol_result = __ftol();
            *warning_timer_ptr = (int)ftol_result;
        }
    } else {
        ftol_result = __ftol();
        *(int *)(g_Vehicles + 0x580 + car_base_offset) = (int)ftol_result;
        if (0x78 < *(int *)(g_Vehicles + 0x580 + car_base_offset)) {
            *(int *)(g_Vehicles + 0x580 + car_base_offset) = 0x78;
        }
    }
}

/**
 * @original HUD_RenderPlayerElements (IGN_WIN.EXE @ 0x0043ef30, fx.c)
 * @fidelity ADAPTED
 */
void HUD_RenderPlayerElements(int player_idx) {
    PlayerHUDState *hud;
    VehicleState *veh;
    VehicleConfig *vcfg;
    int vp_x1, vp_y1, vp_x2, vp_y2;
    int sprite_coords[2];
    char text_buf[32];
    int panel_x;
    int text_color;
    const char *time_str;
    int rank;
    int target_gear;
    int gear_digit;
    double scale;
    int turbo_bar_height;
    void *turbo_sprite;
    int node_idx;
    int road_seg;
    int curve_severity;
    int hud_w;
    int sprite_w;
    int arrow_idx;
    const char *warning_msg;
    int msg_player;
    void *radar_sprite;
    int bar_length;
    int i;
    double progress;
    int blip_offset_y;
    int icon_idx;
    int anim_frame;
    double p2_progress;
    void *light_sprite;
    int light_w;
    int hud_h;
    float finish_scale;
    void *scale_ptr;
    int s;
    int slot;
    double shake;

    hud = &((PlayerHUDState *)g_PlayerHUDState)[player_idx];
    veh = &g_Vehicles[player_idx];
    vcfg = &g_VehicleConfigs[player_idx];

    /* Setup viewport clipping boundary */
    Gfx_SetClipRect(hud->vp_x1, hud->vp_y1, hud->vp_x2, hud->vp_y2);

    /* Viewport camera shake jitter calculation */
    if ((vcfg->cam_param2 == 0 && vcfg->cam_param1 == 0) || (g_IsDemoMode == 1)) {
        vp_x1 = hud->vp_x1;
        vp_y1 = hud->vp_y1;
        vp_x2 = hud->vp_x2;
        vp_y2 = hud->vp_y2;
    } else {
        shake = (double)vcfg->cam_param1 * 0.5;
        if (Math_RandomFloat0To1() < 0.5) {
            vp_x1 = hud->vp_x1 - (int)shake;
            vp_y1 = hud->vp_y1 - (int)shake;
            vp_x2 = hud->vp_x2 - (int)shake;
            vp_y2 = hud->vp_y2 - (int)shake;
        } else {
            vp_x1 = hud->vp_x1 + (int)shake;
            vp_y1 = hud->vp_y1 + (int)shake;
            vp_x2 = hud->vp_x2 + (int)shake;
            vp_y2 = hud->vp_y2 + (int)shake;
        }
    }

    /* 1. Lap Timing & Split Displays (Standard Race Modes) */
    if (g_GameMode != 3) {
        panel_x = vp_x1 + 7;
        text_color = g_FontTextColorTable[g_ActiveFontColor];

        /* Draw lap timer header panel */
        sprite_coords[0] = panel_x * 256;
        sprite_coords[1] = (vp_y1 + 7) * 256;
        Gfx_DrawSprite(g_pLapTimerPanelSprite, sprite_coords, 0);

        g_pActiveDrawBuffer = g_pLisaDrawCommandWritePtr;

        /* Current lap time text */
        if (g_RaceTimer_P2 == -1.0) {
            time_str = HUD_FormatLapTime();
        } else {
            time_str = "--:--:--";
        }
        Font_DrawText(time_str, text_color, panel_x, g_SpeedoPosition[0] + vp_y1);

        /* Split lap panel background */
        sprite_coords[0] = panel_x * 256;
        sprite_coords[1] = (g_SpeedoConfig[0] + vp_y1) * 256;
        Gfx_DrawSprite(g_pLapSplitPanelSprite, sprite_coords, 0);

        /* Lap split 1 */
        if (g_RaceTimer_P2 == -1.0) {
            time_str = HUD_FormatLapTime();
        } else {
            time_str = "--:--:--";
        }
        Font_DrawText(time_str, text_color, panel_x, g_SpeedoPosition[1] + vp_y1);

        /* Lap split 2 */
        if (veh->lap_number >= 1) {
            time_str = HUD_FormatLapTime();
            Font_DrawText(time_str, text_color, panel_x, g_SpeedoPosition[2] + vp_y1);
        }

        /* Lap split 3 */
        if (veh->lap_number >= 2) {
            time_str = HUD_FormatLapTime();
            Font_DrawText(time_str, text_color, panel_x, g_SpeedoPosition[3] + vp_y1);
        }
    }

    /* 2. Position Ranking Badge */
    if (g_NumRacers > 1 || g_IsTwoPlayerMode == 1) {
        if (g_GameMode == 3) {
            if (veh->is_finished != 0) {
                rank = veh->car_model_id;
                sprintf(text_buf, "%d", rank);
            } else {
                sprintf(text_buf, "%d/%d", veh->car_model_id, g_PlayerCarModel);
            }
        } else {
            sprite_coords[0] = (g_SpeedoConfig[1] + vp_x1) * 256;
            sprite_coords[1] = (g_SpeedoConfig[2] + vp_y2) * 256;
            Gfx_DrawSprite(g_pRacePositionBadgeSprite, sprite_coords, 0);

            if (veh->is_finished == 0) {
                rank = veh->car_model_id;
            } else {
                rank = veh->race_rank;
            }
            sprintf(text_buf, "%d", rank);
        }
        Font_DrawText(text_buf, g_FontPositionColorTable[g_ActiveFontColor],
                      g_SpeedoPosition[4] + vp_x1, g_SpeedoPosition[5] + vp_y2);
    }

    /* 3. Tachometer, Speedometer & Gear Indicator */
    HUD_RenderSpeedometerGauge(player_idx);

    target_gear = veh->current_gear << 2;
    if (veh->gear_display_val < (double)target_gear) {
        veh->gear_display_val += g_RpmNeedleDeltaScale * 0.5;
        if (veh->gear_display_val > (double)target_gear) {
            veh->gear_display_val = (double)target_gear;
        }
    } else if (veh->gear_display_val > (double)target_gear) {
        veh->gear_display_val += g_RpmNeedleDeltaScale * (-0.5);
        if (veh->gear_display_val < (double)target_gear) {
            veh->gear_display_val = (double)target_gear;
        }
    }

    sprite_coords[0] = (g_SpeedoConfig[3] + vp_x2) * 256;
    sprite_coords[1] = (g_SpeedoConfig[4] + vp_y2) * 256;
    gear_digit = (int)veh->gear_display_val;
    Gfx_DrawSprite(g_pGearDigitSprites[gear_digit], sprite_coords, 0);

    /* 4. Turbo Boost Bar & Indicators */
    scale = 1.0;
    if (g_ActiveFontColor == 0) {
        scale = 4.2;
    } else if (g_ActiveFontColor == 1) {
        scale = 2.1;
    } else if (g_ActiveFontColor == 2) {
        scale = 1.0;
    }

    g_pLisaDrawCommandQueue[0] = g_LisaDrawCommandBuffer;
    g_pLisaDrawCommandQueue[1] = g_LisaDrawCommandBuffer + 12;
    g_pLisaDrawCommandQueue[2] = 0;

    turbo_bar_height = (int)(veh->turbo_charge * scale);

    /* Triangle 1 */
    g_LisaDrawCommandBuffer[0] = 0xf;
    g_LisaDrawCommandBuffer[1] = (g_SpeedoConfig[0x11] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[2] = (g_SpeedoConfig[0x12] + vp_y2) * 256;
    g_LisaDrawCommandBuffer[3] = 0;
    g_LisaDrawCommandBuffer[4] = (g_SpeedoConfig[0x11] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[5] = (g_SpeedoConfig[0x12] + vp_y2 - turbo_bar_height) * 256;
    g_LisaDrawCommandBuffer[6] = 0;
    g_LisaDrawCommandBuffer[7] = (g_SpeedoConfig[0x13] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[8] = (g_SpeedoConfig[0x12] + vp_y2) * 256;
    g_LisaDrawCommandBuffer[9] = 0;
    g_LisaDrawCommandBuffer[10] = 0xd7;
    g_LisaDrawCommandBuffer[11] = 0;

    /* Triangle 2 */
    g_LisaDrawCommandBuffer[12] = 0xf;
    g_LisaDrawCommandBuffer[13] = (g_SpeedoConfig[0x13] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[14] = (g_SpeedoConfig[0x12] + vp_y2) * 256;
    g_LisaDrawCommandBuffer[15] = 0;
    g_LisaDrawCommandBuffer[16] = (g_SpeedoConfig[0x11] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[17] = (g_SpeedoConfig[0x12] + vp_y2 - turbo_bar_height) * 256;
    g_LisaDrawCommandBuffer[18] = 0;
    g_LisaDrawCommandBuffer[19] = (g_SpeedoConfig[0x13] + vp_x2) * 256;
    g_LisaDrawCommandBuffer[20] = (g_SpeedoConfig[0x12] + vp_y2 - turbo_bar_height) * 256;
    g_LisaDrawCommandBuffer[21] = 0;
    g_LisaDrawCommandBuffer[22] = 0xd7;
    g_LisaDrawCommandBuffer[23] = 0;

    Lisa_FlushRasterizerCommands(g_LisaDrawCommandBuffer, 0);

    /* Turbo meter frame sprite */
    sprite_coords[0] = (g_SpeedoConfig[0xd] + vp_x2) * 256;
    sprite_coords[1] = (g_SpeedoConfig[0xe] + vp_y2) * 256;
    Gfx_DrawSprite(g_pTurboGaugeBorderSprite, sprite_coords, 0);

    /* Turbo indicator light status */
    turbo_sprite = NULL;
    if (veh->turbo_active == 0) {
        if (veh->turbo_charge < 20.0 || veh->turbo_charge >= 100.0) {
            if (veh->turbo_charge >= 100.0) {
                if (g_IsDemoMode == 0 || g_DemoSubState != 2) {
                    veh->turbo_flash_timer++;
                    if (veh->turbo_flash_timer > 14) {
                        veh->turbo_flash_timer = 0;
                    }
                }
                if (veh->turbo_flash_timer < 7) {
                    turbo_sprite = g_pTurboIndicatorLightOnSprite;
                } else {
                    turbo_sprite = g_pTurboIndicatorLightSprites[0];
                }
            }
        } else {
            turbo_sprite = g_pTurboIndicatorLightSprites[0];
        }
    } else {
        if (veh->turbo_active > 0 || veh->turbo_charge < 20.0) {
            if (g_IsDemoMode == 0 || g_DemoSubState != 2) {
                veh->turbo_flash_timer++;
                if (veh->turbo_flash_timer > 1) {
                    veh->turbo_flash_timer = 0;
                }
            }
            turbo_sprite = g_pTurboIndicatorLightSprites[veh->turbo_flash_timer];
        }
    }

    if (turbo_sprite != NULL) {
        sprite_coords[0] = (g_SpeedoConfig[0xf] + vp_x2) * 256;
        sprite_coords[1] = (g_SpeedoConfig[0x10] + vp_y2) * 256;
        Gfx_DrawSprite(turbo_sprite, sprite_coords, 0);
    }

    /* 5. Direction Curve Warning & Wrong Way Notification */
    if (veh->is_finished == 0 && g_RaceTimer_P2 == -1.0) {
        node_idx = veh->current_node_idx;
        if (node_idx < 0) {
            road_seg = ((RoadSequenceNode *)g_pTrackRoadSequence)[-node_idx].road_index_rev;
        } else {
            road_seg = ((RoadSequenceNode *)g_pTrackRoadSequence)[node_idx].road_index;
        }

        if (veh->wrong_way_timer < 26 || veh->target_node_idx <= node_idx) {
            curve_severity = ((TrackSegmentAttribute *)g_TrackSegmentTable)[road_seg].curve_severity;
            if (curve_severity < 1) {
                veh->turn_warning_timer = 0;
            } else {
                veh->turn_warning_timer++;
                if (((veh->turn_warning_timer / 4) & 1) == 0 || veh->turn_warning_timer > 23) {
                    hud_w = hud->vp_x2 - hud->vp_x1;
                    sprite_w = g_pScreenConfig->arrow_sprite_width;
                    sprite_coords[0] = (((hud_w - sprite_w) / 2) + hud->vp_x1) * 256;
                    sprite_coords[1] = (g_SpeedoConfig[0x1b] + vp_y1) * 256;
                    if (curve_severity > 16) {
                        curve_severity = 16;
                    }
                    arrow_idx = g_DirectionArrowLookupTable[g_TrackStyle * 20 + curve_severity];
                    Gfx_DrawSprite(g_pDirectionArrowSprites[arrow_idx], sprite_coords, 0);
                }
            }
        } else {
            hud_w = hud->vp_x2 - hud->vp_x1;
            sprite_w = g_pScreenConfig->arrow_sprite_width;
            sprite_coords[0] = (((hud_w - sprite_w) / 2) + hud->vp_x1) * 256;
            sprite_coords[1] = (g_SpeedoConfig[0x1b] + vp_y1) * 256;
            Gfx_DrawSprite(g_pWrongWayBannerSprite, sprite_coords, 0);
        }
    }

    /* 6. Elimination Mode Track Radar & "You're Last" Warning */
    if (g_GameMode == 3) {
        if (veh->car_model_id == g_PlayerCarModel && veh->is_finished == 0 &&
            (g_IsDemoMode == 0 || g_DemoSubState != 2)) {
            switch (g_LanguageId) {
                case 0:  warning_msg = "WARNING, YOU'RE LAST!"; break;
                case 1:  warning_msg = "SIE SIND LETZTER!"; break;
                case 2:  warning_msg = "ATTENZIONE, SEI ULTIMO!"; break;
                case 3:  warning_msg = "AVISO, ERES EL ULTIMO!"; break;
                case 4:  warning_msg = "VARNING, DU LIGGER SIST!"; break;
                case 5:  warning_msg = "ATTENTION, VOUS ETES DERNIER!"; break;
                default: warning_msg = "WARNING, YOU'RE LAST!"; break;
            }

            msg_player = -1;
            if (player_idx == 0) {
                if (g_IsSplitScreen == 0) {
                    msg_player = -1;
                } else if (g_IsSplitScreen == 1) {
                    msg_player = 0;
                }
            } else if (player_idx == 1 && g_IsSplitScreen == 1) {
                msg_player = 1;
            }

            if (msg_player != -1 || g_IsSplitScreen == 0) {
                HUD_AddFloatingMessage(warning_msg, 0, 1, msg_player);
            }
        }

        radar_sprite = (g_RadarHighResFlag == 0) ? g_pRadarProgressBarSprite_LowRes : g_pRadarProgressBarSprite_HighRes;
        sprite_coords[0] = (hud->vp_x1 + 8) * 256;
        sprite_coords[1] = (hud->vp_y1 + 8) * 256;
        Gfx_DrawSprite(radar_sprite, sprite_coords, 0);

        bar_length = (g_RadarHighResFlag == 0) ? 120 : 300;

        /* Draw blips for active racers */
        for (i = g_NumRacers - 1; i >= 0; i--) {
            if (g_RaceStartTimer < 250.0) {
                progress = 0.0;
            } else {
                progress = abs(g_Vehicles[i].current_node_idx) / (double)g_TrackRoadSequenceNodeCount;
            }

            if (g_Vehicles[i].is_finished == 0) {
                blip_offset_y = (int)(progress * (double)bar_length);
                sprite_coords[0] = (hud->vp_x1 + 5) * 256;
                sprite_coords[1] = ((hud->vp_y1 - blip_offset_y) + bar_length + 5) * 256;
                icon_idx = ((PlayerHUDState *)g_PlayerHUDState)[i].racer_icon_idx;
                Gfx_DrawSprite(g_pRadarCarBlipSprites[icon_idx], sprite_coords, 0);

                if (g_Vehicles[i].car_model_id == 1) {
                    sprite_coords[0] = (hud->vp_x1 + 19) * 256;
                    sprite_coords[1] = ((hud->vp_y1 - blip_offset_y) + bar_length + 8) * 256;
                    Gfx_DrawSprite(g_pRadarLeaderArrowSprite, sprite_coords, 0);
                }
            }
        }

        /* Draw blips for eliminated racers */
        for (i = g_NumRacers - 1; i >= 0; i--) {
            if (g_Vehicles[i].is_finished == 1 && g_Vehicles[i].elimination_timer < 10.0 && g_Vehicles[i].race_rank != 1) {
                if (g_RaceStartTimer < 250.0) {
                    progress = 0.0;
                } else {
                    progress = abs(g_Vehicles[i].current_node_idx) / (double)g_TrackRoadSequenceNodeCount;
                }
                blip_offset_y = (int)(progress * (double)bar_length);
                sprite_coords[0] = hud->vp_x1 * 256;
                sprite_coords[1] = ((hud->vp_y1 - blip_offset_y) + bar_length) * 256;
                anim_frame = (int)g_Vehicles[i].elimination_timer;
                Gfx_DrawSprite(g_pRadarEliminatedBlipSprites[anim_frame], sprite_coords, 0);
            }
        }

        /* Player 2 blip in split-screen */
        if (player_idx == 1 && g_Vehicles[1].is_finished == 0) {
            if (g_RaceStartTimer < 250.0) {
                p2_progress = 0.0;
            } else {
                p2_progress = abs(g_Vehicles[1].current_node_idx) / (double)g_TrackRoadSequenceNodeCount;
            }
            blip_offset_y = (int)(p2_progress * (double)bar_length);
            sprite_coords[0] = (hud->vp_x1 + 5) * 256;
            sprite_coords[1] = ((hud->vp_y1 - blip_offset_y) + bar_length + 5) * 256;
            icon_idx = ((PlayerHUDState *)g_PlayerHUDState)[1].racer_icon_idx;
            Gfx_DrawSprite(g_pRadarCarBlipSprites[icon_idx], sprite_coords, 0);
        }
    }

    /* 7. Start Countdown Traffic Lights & Audio Cues */
    light_sprite = NULL;
    if (g_RaceTimer_P2 < 50.0 || g_RaceTimer_P2 >= 100.0) {
        if (g_RaceTimer_P2 >= 100.0 && g_RaceTimer_P2 < 150.0) {
            if (g_CountdownBeepStep == 0) {
                Audio_PlaySampleVol(0, 4, 0, 0, 0x10000, 22000, 0);
                g_CountdownBeepStep = 1;
            }
            light_sprite = g_pTrafficLightYellow1Sprite;
        } else if (g_RaceTimer_P2 >= 150.0 && g_RaceTimer_P2 < 200.0) {
            if (g_CountdownBeepStep == 1) {
                Audio_PlaySampleVol(0, 4, 0, 0, 0x10000, 22000, 0);
                g_CountdownBeepStep = 2;
            }
            light_sprite = g_pTrafficLightYellow2Sprite;
        } else if (g_RaceTimer_P2 == -1.0 && g_RaceStartTimer < 250.0) {
            if (g_CountdownBeepStep == 2) {
                Audio_PlaySampleVol(0, 4, 1, 0, 0x10000, 22000, 0);
                g_CountdownBeepStep = 3;
            }
            light_sprite = g_pTrafficLightGreenSprite;
        }
    } else {
        light_sprite = g_pTrafficLightRedSprite;
    }

    if (light_sprite != NULL) {
        hud_w = hud->vp_x2 - hud->vp_x1;
        light_w = g_SpeedoConfig[0x18];
        sprite_coords[0] = (((hud_w - light_w) / 2) + hud->vp_x1) * 256;
        sprite_coords[1] = (g_SpeedoConfig[0x19] + vp_y1) * 256;
        Gfx_DrawSprite(light_sprite, sprite_coords, 0);
    }

    /* 8. Finish Podium Banner & Sparkle Particles */
    if (g_RaceTimer >= 0.0 && g_PlayerFinishTimers[player_idx] <= 119) {
        if (g_GameMode != 2 || g_NumRacers >= 2) {
            if ((g_GameMode == 3 && veh->race_rank <= 1) || (g_GameMode != 3 && veh->race_rank <= 3)) {
                if (veh->is_finished == 1) {
                    if (veh->finish_banner_timer > 0 && veh->race_rank < 4 && (g_GameMode != 2 || g_NumRacers > 1)) {
                        hud_w = hud->vp_x2 - hud->vp_x1;
                        hud_h = vp_y2 - vp_y1;
                        sprite_coords[0] = (hud_w / 2 + hud->vp_x1) * 256;
                        sprite_coords[1] = (hud_h / 2) * 256;

                        finish_scale = (float)veh->finish_banner_timer * 0.04f;
                        g_SpriteScaleFactors[0] = finish_scale;
                        g_SpriteScaleFactors[1] = 0.0f;
                        scale_ptr = (veh->finish_banner_timer < 25) ? (void *)g_SpriteScaleFactors : NULL;

                        Gfx_DrawSprite(g_pFinishPlaceSprites[veh->race_rank], sprite_coords, scale_ptr);

                        if (g_FinishSparkleTimers[player_idx * 10] > 99 && g_FinishFanfarePlayed[player_idx] == 0) {
                            Audio_PlaySampleVol(0, 4, 5, 0, 0x10000, 22000, 0);
                            g_FinishFanfarePlayed[player_idx] = 1;
                        }
                    }

                    /* Finish sparkles update loop */
                    for (s = 0; s < 6; s++) {
                        slot = player_idx * 10 + s;
                        if (g_FinishSparkleTimers[slot] > 12 && g_PlayerFinishTimers[player_idx] < 40) {
                            hud_w = hud->vp_x2 - hud->vp_x1;
                            hud_h = vp_y2 - vp_y1;
                            g_FinishSparklePosX[slot] = (int)((Math_RandomFloat0To1() - 0.5f) * (float)(hud_w / 2) + (float)hud->vp_x1 + (float)(hud_w / 2));
                            g_FinishSparklePosY[slot] = (int)((Math_RandomFloat0To1() - 0.5f) * (float)(hud_h / 2) + (float)(hud_h / 2));

                            if (g_FinishSparkleTimers[slot] < 100) {
                                g_FinishSparkleTimers[slot] = 0;
                            } else {
                                g_FinishSparkleTimers[slot] = (int)(Math_RandomFloat0To1() * 12.0f);
                            }
                        }

                        if (g_FinishSparkleTimers[slot] < 13) {
                            g_SpriteScaleFactors[0] = 0.5f;
                            g_SpriteScaleFactors[1] = 0.0f;
                            sprite_coords[0] = g_FinishSparklePosX[slot] * 256;
                            sprite_coords[1] = g_FinishSparklePosY[slot] * 256;
                        }
                    }
                }
            }
        }
    }

    HUD_RenderFloatingMessages();
}
