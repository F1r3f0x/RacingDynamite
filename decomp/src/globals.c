/*
 * globals.c - Global variable definitions and engine subsystem stubs
 */
#include <stddef.h>
#include <stdint.h>
#include "lisa3d.h"
#include <i86.h>
#include <conio.h>

/* --- Engine Subsystem Callbacks & Stubs --- */
int App_SetVideoMode(void *a, void *b, void *c, void *d) {
    union REGS r;
    (void)a; (void)b; (void)c; (void)d;
    r.w.ax = 0x0013; /* Set 320x200 256-color Mode 13h */
    int386(0x10, &r, &r);
    return 1;
}
int Audio_GetChannel(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Audio_GetVoice(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Audio_PlaySample(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Audio_PlaySound(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Audio_StopSound(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int FX_SpawnCameraParticle(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int FX_UpdateEngineSmoke(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int FX_UpdateSparks(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int FX_UpdateTurboFlames(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Font_PrintDirect(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_BlitTransparentLUT(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_DrawSprite(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_FreeSurface(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_RestoreSurface(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_SetClipRect(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_SetRenderTarget(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Gfx_SpriteOp(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Ghost_SaveGhostData(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_AddFloatingMessage(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_DrawDemoWatermark(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_DrawPlayerSplitTimer(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_FormatLapTime(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_RenderFloatingMessages(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_RenderSpeedometerGauge(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_ShowAnnouncementBanner(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int HUD_UpdateLapCounters(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Lisa_InitDynamicObjectNode(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Log_DebugPrintf(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Math_AngleMod(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Math_RandomFloat0To1(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Math_WrapAngle(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Menu_AddLayoutItem(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Menu_ClearLayout(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Menu_LayoutItems(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Obstacle_ResetActions(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Obstacle_SetTriggerState(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Palette_AdjustRGB(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Sound_FreeSample(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Subsystem_AddCallback(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Subsystem_Register(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Subsystem_Unregister(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Surface_GetHeightAtPoint(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Track_FindSurfaceHeight(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Track_LoadPlacementsAndCars(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Unknown_553a0(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 0; }
int Unknown_553c8(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 1; }
int Unknown_558d0(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return 1; }
int Unknown_558ec(void *a, void *b, void *c, void *d) {
    union REGS r;
    (void)a; (void)b; (void)c; (void)d;
    r.w.ax = 0x0003; /* Restore 80x25 standard text mode */
    int386(0x10, &r, &r);
    return 0;
}
int Unknown_559e4(void *a, void *b, void *c, void *d) {
    union REGS r;
    (void)a; (void)b; (void)c; (void)d;
    r.w.ax = 0x0013; /* Set 320x200 256-color VGA mode */
    int386(0x10, &r, &r);
    return 1;
}
#pragma aux _wstart2_ "_wstart2_";
void _wstart2_(void) {}

/* --- Global Data & State Variables --- */
uint32_t g_ActiveFontColor[16] = {0};
uint32_t g_ActiveParticle[16] = {0};
uint32_t g_ActiveTrackPalette[16] = {0};
uint32_t g_ActiveTrackSegmentAttribute[16] = {0};
uint32_t g_ActiveVehicleIndex[16] = {0};
uint32_t g_AudioCdTrackMode[16] = {0};
uint32_t g_AudioEventParam1[16] = {0};
uint32_t g_AudioEventParam2[16] = {0};
uint32_t g_AudioEventParam3[16] = {0};
uint32_t g_AudioEventParam4[16] = {0};
uint32_t g_AudioEventParam5[16] = {0};
uint32_t g_AudioEventParam6[16] = {0};
uint32_t g_AudioEventParam7[16] = {0};
uint32_t g_CameraAngleX[16] = {0};
uint32_t g_CameraAngleY[16] = {0};
uint32_t g_CameraFovHalfX[16] = {0};
uint32_t g_CameraFovHalfY[16] = {0};
uint32_t g_CameraFovScaleX[16] = {0};
uint32_t g_CameraFovScaleY[16] = {0};
uint32_t g_CameraFovWobbleActive[16] = {0};
uint32_t g_CameraFovWobblePhase[16] = {0};
uint32_t g_CameraPosX[16] = {0};
uint32_t g_CameraPosX_Int[16] = {0};
uint32_t g_CameraPosY[16] = {0};
uint32_t g_CameraPosY_Int[16] = {0};
uint32_t g_CameraPosZ[16] = {0};
uint32_t g_CameraPosZ_Int[16] = {0};
uint32_t g_CameraShakeActive[16] = {0};
uint32_t g_CameraTargetHysteresis[16] = {0};
uint32_t g_CameraViewportHeight[16] = {0};
uint32_t g_CameraViewportWidth[16] = {0};
uint32_t g_CarShadowVertices[16] = {0};
uint32_t g_ChampionshipAdvanceAllowed[16] = {0};
uint32_t g_ChampionshipCredits[16] = {0};
uint32_t g_ChampionshipNextTrackIndex[16] = {0};
uint32_t g_ChampionshipPoints[16] = {0};
uint32_t g_ChampionshipWonFlag[16] = {0};
uint32_t g_CinematicCameraAngleState[16] = {0};
uint32_t g_CinematicFocusedVehicle[16] = {0};
uint32_t g_ColorDepth[16] = {0};
uint32_t g_ConfirmationPromptType[16] = {0};
uint32_t g_Const_0_005[16] = {0};
uint32_t g_Const_0_05[16] = {0};
uint32_t g_Const_0_1[16] = {0};
uint32_t g_Const_1000[16] = {0};
uint32_t g_Const_1024[16] = {0};
uint32_t g_Const_200_0[16] = {0};
uint32_t g_Const_256[16] = {0};
uint32_t g_Const_262144[16] = {0};
uint32_t g_Const_512[16] = {0};
uint32_t g_Const_DegToRad[16] = {0};
uint32_t g_Const_Neg256[16] = {0};
uint32_t g_Const_NegPi[16] = {0};
uint32_t g_Const_NegTenthDegToRad[16] = {0};
uint32_t g_Const_Pi[16] = {0};
uint32_t g_Const_TenthDegToRad[16] = {0};
uint32_t g_Const_TenthDegToRadFloat[16] = {0};
uint32_t g_Const_TwoPi[16] = {0};
uint32_t g_CountdownBeepStep[16] = {0};
uint32_t g_CurrentCdTrackNumber[16] = {0};
uint32_t g_CurrentTrackIndex[16] = {0};
uint32_t g_DebrisDynamicMeshes[16] = {0};
uint32_t g_DemoSubState[16] = {0};
uint32_t g_DifficultyLevel[16] = {0};
uint32_t g_DirectionArrowLookupTable[16] = {0};
uint32_t g_DynamicObjectsPaused[16] = {0};
uint32_t g_ErrorMessageBuffer[16] = {0};
uint32_t g_FinishFanfarePlayed[16] = {0};
uint32_t g_FinishSparklePosX[16] = {0};
uint32_t g_FinishSparklePosY[16] = {0};
uint32_t g_FinishSparkleTimers[16] = {0};
uint32_t g_FlybyCameraOffsetX[16] = {0};
uint32_t g_FlybyCameraOffsetY[16] = {0};
uint32_t g_FlybyCameraOffsetZ[16] = {0};
uint32_t g_FlybyLastCameraPosX[16] = {0};
uint32_t g_FlybyLastCameraPosY[16] = {0};
uint32_t g_FlybyLastCameraPosZ[16] = {0};
uint32_t g_FlybyLastPitch[16] = {0};
uint32_t g_FlybyLastRoll[16] = {0};
uint32_t g_FlybyLastYaw[16] = {0};
uint32_t g_FontAlignMode[16] = {0};
uint32_t g_FontId_Large[16] = {0};
uint32_t g_FontId_Medium[16] = {0};
uint32_t g_FontId_Menu[16] = {0};
uint32_t g_FontId_Small[16] = {0};
uint32_t g_FontPositionColorTable[16] = {0};
uint32_t g_FontTextColorTable[16] = {0};
uint32_t g_ForceDirtDebris[16] = {0};
uint32_t g_GamePauseState[16] = {0};
uint32_t g_GlobalFrameCount[16] = {0};
uint32_t g_GraphicsInitErrorFlag[16] = {0};
uint32_t g_GraphicsResolutionMode[16] = {0};
uint32_t g_HighScoreRecorded[16] = {0};
uint32_t g_HudAnimSinPhase[16] = {0};
uint32_t g_HudAnimTimers[16] = {0};
uint32_t g_IsAttractDemoMode[16] = {0};
uint32_t g_IsDemoMode[16] = {0};
uint32_t g_IsSplitScreen[16] = {0};
uint32_t g_IsTwoPlayerMode[16] = {0};
uint32_t g_LapRecordSeconds[16] = {0};
uint32_t g_LapsTotal[16] = {0};
uint32_t g_LightningEnabled[16] = {0};
uint32_t g_LisaActiveLightingMode[16] = {0};
uint32_t g_LisaActiveMaterial[16] = {0};
uint32_t g_LisaActivePageCount[16] = {0};
uint32_t g_LisaActiveShdSize[16] = {0};
uint32_t g_LisaActiveSubmeshFlags[16] = {0};
uint32_t g_LisaActiveTabSize[16] = {0};
uint32_t g_LisaActiveTextureID[16000] = {0};
uint32_t g_LisaAllocatedBufferCount[16] = {0};
uint32_t g_LisaAllocatedBufferMax[16] = {0};
uint32_t g_LisaAspectScale[16] = {0};
uint32_t g_LisaBackfaceSign[16] = {0};
uint32_t g_LisaCameraDistance[16] = {0};
uint32_t g_LisaCameraFocalLength[16] = {0};
uint32_t g_LisaCameraFocalScale[16] = {0};
uint32_t g_LisaCameraMatrix_00[16] = {0};
uint32_t g_LisaCameraMatrix_01[16] = {0};
uint32_t g_LisaCameraMatrix_02[16] = {0};
uint32_t g_LisaCameraMatrix_10[16] = {0};
uint32_t g_LisaCameraMatrix_11[16] = {0};
uint32_t g_LisaCameraMatrix_12[16] = {0};
uint32_t g_LisaCameraMatrix_20[16] = {0};
uint32_t g_LisaCameraMatrix_21[16] = {0};
uint32_t g_LisaCameraMatrix_22[16] = {0};
uint32_t g_LisaCameraMatrix_X[16] = {0};
uint32_t g_LisaCameraMatrix_Y[16] = {0};
uint32_t g_LisaCameraOffsetX[16] = {0};
uint32_t g_LisaCameraOffsetY[16] = {0};
uint32_t g_LisaCameraPitch[16] = {0};
uint32_t g_LisaCameraZoom[16] = {0};
uint32_t g_LisaClipBottom[16] = {0};
uint32_t g_LisaClipLeft[16] = {0};
uint32_t g_LisaClipRight[16] = {0};
uint32_t g_LisaClipSubpixelBottom[16] = {0};
uint32_t g_LisaClipSubpixelLeft[16] = {0};
uint32_t g_LisaClipSubpixelRight[16] = {0};
uint32_t g_LisaClipSubpixelTop[16] = {0};
uint32_t g_LisaClipTop[16] = {0};
uint32_t g_LisaCurrentVertexIndex[16] = {0};
uint32_t g_LisaDefaultOffset_X[16] = {0};
uint32_t g_LisaDefaultOffset_Y[16] = {0};
uint32_t g_LisaDefaultOffset_Z[16] = {0};
uint32_t g_LisaDefaultScale_X[16] = {0};
uint32_t g_LisaDefaultScale_Y[16] = {0};
uint32_t g_LisaDisableFiltering[16] = {0};
int *g_LisaDrawCommandBuffer = NULL;
uint32_t g_LisaDrawCommands[16] = {0};
uint32_t g_LisaEnableMipmaps[16] = {0};
uint32_t g_LisaFrustumNear[16] = {0};
uint32_t g_LisaFrustumPlaneLeft[16] = {0};
uint32_t g_LisaFrustumPlaneRight[16] = {0};
uint32_t g_LisaFrustumPlaneTop[16] = {0};
uint32_t g_LisaGridCellsX[16] = {0};
uint32_t g_LisaGridWorldHeight[16] = {0};
uint32_t g_LisaGridWorldWidth[16] = {0};
uint32_t g_LisaMipmapQuality[16] = {0};
uint32_t g_LisaObjMat_11[16] = {0};
uint32_t g_LisaObjMat_20[16] = {0};
uint32_t g_LisaObjMat_21[16] = {0};
uint32_t g_LisaObjMat_CosPitch[16] = {0};
uint32_t g_LisaObjMat_CosRoll[16] = {0};
uint32_t g_LisaObjMat_CosYaw[16] = {0};
uint32_t g_LisaObjMat_Scale[16] = {0};
uint32_t g_LisaObjMat_SinPitch[16] = {0};
uint32_t g_LisaObjMat_SinRoll[16] = {0};
uint32_t g_LisaObjMat_Tmp1[16] = {0};
uint32_t g_LisaObjMat_Tmp2[16] = {0};
uint32_t g_LisaObjMat_Tmp3[16] = {0};
uint32_t g_LisaObjMat_Tmp4[16] = {0};
uint32_t g_LisaObjMat_Tmp5[16] = {0};
uint32_t g_LisaObjMat_Tmp6[16] = {0};
uint32_t g_LisaObjectMatrix_22[16] = {0};
void *g_LisaOpcodeTable[32] = {0};
uint32_t g_LisaPerspectiveDepthTable[16] = {0};
uint32_t g_LisaRasterizerAccumulator[16] = {0};
uint32_t g_LisaRasterizerJmpTable[16] = {0};
void *g_LisaRenderTexturedOp11_FuncPtr = NULL;
void *g_LisaRenderTexturedOp15_FuncPtr = NULL;
uint32_t g_LisaScanlinePitch[16] = {0};
uint32_t g_LisaScreenPitch[1000] = {0};
uint32_t g_LisaShadingEnabled[16] = {0};
uint32_t g_LisaSubmeshBoundRadius[16] = {0};
uint32_t g_LisaSubmeshCenterWorldX[16] = {0};
uint32_t g_LisaSubmeshCenterWorldY[16] = {0};
uint32_t g_LisaSubmeshCenterWorldZ[16] = {0};
uint32_t g_LisaSubmeshClipMask[16] = {0};
uint32_t g_LisaSubmeshDepthOffset[16] = {0};
uint32_t g_LisaSubmeshFlags[16] = {0};
uint32_t g_LisaSubmeshLodLevel[16] = {0};
uint32_t g_LisaSubmeshPolyCount[16] = {0};
uint32_t g_LisaSubmeshTmp1[16] = {0};
uint32_t g_LisaSubmeshTmp4[16] = {0};
uint32_t g_LisaSubmeshTmp5[16] = {0};
uint32_t g_LisaSubmeshVertexStride[16] = {0};
uint32_t g_LisaTexturePageSizes[16] = {0};
uint32_t g_LisaTransformedVertices[16] = {0};
uint32_t g_LisaViewportCenterX[16] = {0};
uint32_t g_LisaViewportCenterY[16] = {0};
uint32_t g_LisaViewportHeight[16] = {0};
uint32_t g_LisaViewportQuarter[16] = {0};
uint32_t g_LisaViewportRemaining[16] = {0};
uint32_t g_LisaViewportWidth[16] = {0};
uint32_t g_LisaVisibleObjects[16] = {0};
uint32_t g_LisaVisibleSubmeshes[16] = {0};
uint32_t g_MasterMusicVolume[16] = {0};
int g_MenuCursorPos = 0;
uint32_t g_MultiplayerMode[16] = {0};
int g_NumRacers = 0;
uint32_t g_ParticleArray1[16] = {0};
uint32_t g_ParticleArray2[16] = {0};
uint32_t g_ParticleCount1[16] = {0};
int g_ParticlePriorityTable[12] = { 0, 128, 64, 128, 0, 192, 64, 192, 0, 256, 64, 0 };
uint32_t g_ParticleTemplateX[16] = {0};
uint32_t g_ParticleTemplateY[16] = {0};
uint32_t g_ParticleTemplateZ[16] = {0};
uint32_t g_PlayAgainPromptActive[16] = {0};
uint32_t g_PlayerCarChoice[16] = {0};
uint32_t g_PlayerCarModel[16] = {0};
uint32_t g_PlayerFinishTimers[16] = {0};
uint8_t g_PlayerHUDState[512] = {0};
uint32_t g_PostRaceSequenceState[16] = {0};
uint32_t g_RaceFinishingPointsTable[16] = {0};
uint32_t g_RacePosition[16] = {0};
uint32_t g_RaceStartTimer[16] = {0};
uint32_t g_RaceTimer[16] = {0};
uint32_t g_RaceTimer_P2[16] = {0};
uint32_t g_RadarHighResFlag[16] = {0};
uint32_t g_RainTextureId[16] = {0};
uint32_t g_RenderTargetSurface[16] = {0};
uint32_t g_ResultsMenuSelection[16] = {0};
uint32_t g_ResultsOverlayActive[16] = {0};
uint32_t g_ResultsRenderedLaps[16] = {0};
uint32_t g_RpmNeedleDeltaScale[16] = {0};
uint32_t g_SavedProfileLeague[16] = {0};
uint8_t g_SceneryParticles[20000] = {0};
uint32_t g_ScreenHeightAlt[16] = {0};
uint32_t g_ScreenShakeTimerP1[16] = {0};
uint32_t g_ScreenShakeTimerP2[16] = {0};
uint32_t g_ScreenSizeSetting[16] = {0};
uint32_t g_ShowDebugCoords[16] = {0};
uint32_t g_ShowFpsOverlay[16] = {0};
uint32_t g_ShowRecordingOverlay[16] = {0};
uint32_t g_ShowRollTelemetry[16] = {0};
uint32_t g_ShowSecondPlayerFlag[16] = {0};
uint32_t g_SkyClearEnabled[16] = {0};
uint32_t g_SmokeParticleDescriptor[16] = {0};
uint32_t g_SnowflakeAngle[16] = {0};
uint32_t g_SortedChampionshipPoints[16] = {0};
uint32_t g_SortedRacerIndices[16] = {0};
uint32_t g_SpeedoConfig[16] = {0};
uint32_t g_SpeedoPosition[16] = {0};
uint32_t g_SpriteScaleFactors[16] = {0};
uint32_t g_SubpixelMaxX[16] = {0};
uint32_t g_SubpixelMaxY[16] = {0};
uint32_t g_SubpixelMinX[16] = {0};
uint32_t g_SubpixelMinY[16] = {0};
uint32_t g_SurfaceDustFlagTable[16] = {0};
uint32_t g_TargetLapTimeCents[16] = {0};
uint32_t g_TelemetryRollAngle[16] = {0};
uint32_t g_TimeTrialActive[16] = {0};
int g_TrackBackgroundColors[7] = { 72, 157, 130, 0, 56, 45, 76 };
uint32_t g_TrackChunkToNodeTable[16] = {0};
uint32_t g_TrackFlybyActiveTarget[16] = {0};
double g_TrackFlybyCameras[128] = {0};
uint32_t g_TrackHighScoreNames[16] = {0};
uint32_t g_TrackHighScoreTimes[16] = {0};
uint32_t g_TrackRoadSequenceNodeCount[16] = {0};
uint32_t g_TrackSegmentTable[16] = {0};
uint32_t g_TrackStyle[16] = {0};
uint32_t g_TrigAngleTable[16] = {0};
uint32_t g_TrophyFanfarePlayed[16] = {0};
uint32_t g_TurboMeterFill[16] = {0};
uint32_t g_UnlockedLeagueIndex[16] = {0};
uint32_t g_VehicleCameraStates[16] = {0};
uint8_t g_VehicleConfigs[8 * 200] = {0};
uint8_t g_Vehicles[8 * 0x484c] = {0};
uint32_t g_VideoModeActive[16] = {0};
uint32_t g_ViewportBorderCornerBL[16] = {0};
uint32_t g_ViewportBorderCornerBR[16] = {0};
uint32_t g_ViewportBorderCornerTL[16] = {0};
uint32_t g_ViewportBorderCornerTR[16] = {0};
uint32_t g_ViewportBorderMetrics[16] = {0};
uint32_t g_ViewportMaxX[16] = {0};
uint32_t g_ViewportMaxY[16] = {0};
uint32_t g_ViewportMinX[16] = {0};
uint32_t g_ViewportMinY[16] = {0};
double g_ViewportScreenScaleTable[4][2] = {
    { 1.0, 1.0 },
    { 0.8, 1.0 },
    { 0.6, 1.0 },
    { 0.5, 0.83 }
};
uint8_t g_VirtualFramebuffer[320 * 200] = {0};
uint8_t *g_pVirtualFramebuffer = g_VirtualFramebuffer;

/* Authentic DOS keyboard subsystem state & tables (MAINDOS.EXE) */
int32_t  g_KeyRepeatActiveTimer[256] = {0};       /* ds:0x20a1b0 - ms counter per key */
uint8_t  g_KeyToggleState[256] = {0};            /* ds:0x20a5b0 - toggle state */
uint8_t  g_KeyRepeatTriggered[256] = {0};        /* ds:0x20a7b0 - key auto-repeat flag */
uint8_t  g_KeyToggleMask[256] = {0};             /* ds:0x20a8b0 - toggle mode mask */
uint8_t  g_KeyReleasedFlag[256] = {0};           /* ds:0x20a9b0 - key release edge flag */
uint8_t  g_KeyJustPressed[256] = {0};            /* ds:0x20aab0 - key press edge flag */
uint8_t  g_KeyPreviousDown[256] = {0};           /* ds:0x20abb0 - previous poll down state */
uint8_t  g_KeyboardState[256] = {0};             /* ds:0x20acb0 - active key down state */
uint8_t  g_KeyScancodeRingBuf[16] = {0};         /* ds:0x20adb0 - ISR ring buffer */
uint8_t  g_KeyRawState[256] = {0};               /* ds:0x20adc0 - raw hardware down state */
char    *g_KeyAsciiRingWritePtr = NULL;          /* ds:0x20aec0 - pointer into ASCII ring buffer */
int32_t  g_KeyLastPollTick = 0;                  /* ds:0x20aec4 - tick of last poll */
int32_t  g_KeyRepeatInterval = 0;                /* ds:0x20aec8 - repeat rate threshold */
int32_t  g_KeyRepeatInitialDelay = 0;            /* ds:0x20aecc - initial repeat delay */
int32_t  g_KeyDriverInstalled = 0;               /* ds:0x20aed0 - 1 if keyboard initialized */
void   (*g_KeyCallback)(int, int) = NULL;        /* ds:0x20aed4 - user key callback function */
char     g_KeyAsciiRingBuf[64] = {0};            /* ds:0x20af3c - ASCII ring buffer (32B payload + mirror) */
uint8_t  g_KeyIsrScancodeHead = 0;               /* ds:0xc5d19 - head index in ring buffer */
uint8_t  g_KeyIsrInstalled = 0;                  /* ds:0xc5d1a - 1 if INT 9 hooked */
const uint8_t g_ScancodeToAsciiTable[84] = {     /* ds:0xc5d20 - scancode to ASCII map */
    0xff, 0xff, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0xff, 0xff,
    0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d, 0xff, 0xff, 0x41, 0x53,
    0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3b, 0x27, 0xff, 0xff, 0x5c, 0x5a, 0x58, 0x43, 0x56,
    0x42, 0x4e, 0x4d, 0x2c, 0x2e, 0x2f, 0xff, 0x2a, 0xff, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x37, 0x38, 0x39, 0x2d, 0x34, 0x35, 0x36, 0x2b, 0x31,
    0x32, 0x33, 0x30, 0x2e
};

/* Authentic Menu & Palette subsystem state (MAINDOS.EXE) */
uint8_t *g_pMenuPalBlack = NULL;                 /* ds:0xea204 - 768B all-zero palette */
uint8_t *g_pMenuPalWhite = NULL;                 /* ds:0xea208 - 768B all-0xFF palette */
uint8_t *g_pMenuPalWork = NULL;                  /* ds:0xea20c - 768B interpolated work palette */
uint8_t *g_pMenuPalDefault = NULL;               /* ds:0xea200 - 768B MENU.COL palette */
uint8_t *g_pMenuCol = NULL;                      /* alias to g_pMenuPalDefault */
uint8_t *g_pMenuPalActive = NULL;                /* ds:0xea1fc - currently loaded target palette */
uint8_t *g_pMenuBuffer1 = NULL;                  /* ds:0xea400 - 64,321 byte 8bpp menu buffer 1 */
uint8_t *g_pMenuBuffer2 = NULL;                  /* ds:0xea404 - 64,321 byte 8bpp menu buffer 2 */
uint8_t *g_pMenuBuffer3 = NULL;                  /* ds:0xea408 - 64,321 byte 8bpp menu buffer 3 */
double   g_MenuDeltaTime = 0.0;                  /* ds:0xea224 - frame delta time */
double   g_MenuTimeSeconds = 0.0;                /* ds:0xea198 - cumulative menu elapsed time */
double   g_MenuCdpFrameAccum = 0.0;              /* ds:0xea1a0 - CDP frame step accumulator */
int32_t  g_MenuCdpActiveIndex = 0;               /* ds:0xea1d0 - index of active CDP (0..5) */
int32_t  g_MenuCdpPendingLoad = 0;               /* ds:0xea1d4 - flag indicating CDP needs to be opened */
int32_t  g_MenuAudioVoiceActive = 0;             /* ds:0xea268 - audio voice status flag */
int32_t  g_MenuVideoResolutionMode = 0;          /* ds:0xea194 - video mode index (0 = 320x200) */

uint8_t  g_GameSettings[0x598] = {0};            /* ds:0xe9bfc .. 0xea193 - game settings */
uint8_t *g_pMenuCdpFiles[6] = {NULL};            /* ds:0xea1b8 .. 0xea1cc - 6 loaded CDP file buffers */
CdpFile  g_MenuCdp = {0};                       /* ds:0xea1d8 - active menu CDP player */
uint8_t *g_pMenuTab = NULL;                      /* ds:0xea210 - 64KB transparency lookup table pointer */
uint8_t *g_pMenuTabAlloc = NULL;                 /* ds:0xea214 - raw allocation for menu.tab */
const uint8_t *g_pMenuTransTable = NULL;         /* ds:0xea218 - trans table pointer */
uint8_t *g_pMenuBilar = NULL;                    /* ds:0xea750 - bilar.pic (124,806 bytes) */
uint8_t *g_pMenuTrackSpr = NULL;                 /* ds:0xea754 - trk_spr.pic (79,695 bytes) */
uint8_t *g_pMenuLogo = NULL;                     /* ds:0xea758 - ign_logo.pic (9,861 bytes) */
uint8_t *g_pMenuCarSel = NULL;                   /* ds:0xea75c - car_sel.pic (18,870 bytes) */
uint8_t *g_pMenuFlags = NULL;                    /* ds:0xea760 - flaggor.pic (38,796 bytes) */
int32_t  g_MenuSelectedLanguage = 0;             /* ds:0xea764 */
int32_t  g_MenuIntroResetBuffers = 1;            /* ds:0xbecfc */
int32_t  g_MenuIntroVideoEnabled = 1;            /* ds:0xbed18 */
const char *g_MenuCdpNames[6] = {                /* ds:0xbed00 */
    "baltazar\\data\\ign1.cdp",
    "baltazar\\data\\ign2.cdp",
    "baltazar\\data\\ign3_0.cdp",
    "baltazar\\data\\ign3_1.cdp",
    "baltazar\\data\\ign3_2.cdp",
    "baltazar\\data\\ign3_3.cdp"
};
uint8_t *g_pMenuLisaEngine = NULL;               /* ds:0xea3c0 - engine context allocated in Menu_Init */
uint32_t g_WeatherActive[16] = {0};
uint32_t g_WeatherAnimTick[16] = {0};
uint32_t g_WeatherAudioVoices[16] = {0};
uint32_t g_WeatherDropOffsetX[16] = {0};
uint32_t g_WeatherDropOffsetY[16] = {0};
uint32_t g_WeatherGridObjects[16] = {0};
uint32_t g_WeatherType[16] = {0};
uint32_t g_WeatherViewportCount[16] = {0};
uint32_t g_WheelParticleDescriptor[16] = {0};
double g_pActiveCamera[32] = {0};
double g_pActiveCamera_P2[32] = {0};
void *g_pActiveDrawBuffer = NULL;
void *g_pActiveMSH = NULL;
void *g_pActivePOS = NULL;
void *g_pActiveSHD = NULL;
void *g_pActiveTAB = NULL;
void *g_pAnimatedSceneryObjects = NULL;
void *g_pBronzeTrophySprite = NULL;
void *g_pCarBaseMeshes = NULL;
void *g_pCarGhostTransforms = NULL;
void *g_pCarIconSprites = NULL;
void *g_pCarReflectionTransforms = NULL;
void *g_pCarShadowTransforms = NULL;
void *g_pCarTransforms = NULL;
void *g_pCreditIconSprite = NULL;
void *g_pDirectionArrowSprites = NULL;
void *g_pFXTransforms = NULL;
void *g_pFinishPlaceSprites = NULL;
void *g_pGearDigitSprites = NULL;
void *g_pGoldTrophySprite = NULL;
void *g_pHudDrawCommandBuffer = NULL;
void *g_pLapSplitPanelSprite = NULL;
void *g_pLapTimerPanelSprite = NULL;
void *g_pLisaActiveMipTable = NULL;
void *g_pLisaActiveShading = NULL;
void *g_pLisaActiveSubmesh = NULL;
void *g_pLisaActiveTABMirror = NULL;
void *g_pLisaAllocatedBuffers = NULL;
void *g_pLisaAllocatedBuffersEnd = NULL;
void *g_pLisaCommandQueueMirror = NULL;
int g_pLisaDrawCommandQueue[16] = {0};
void *g_pLisaDrawCommandTail = NULL;
void *g_pLisaDrawCommandWritePtr = NULL;
void *g_pLisaEdgeBuffer = NULL;
void *g_pLisaFramebufferMirror1 = NULL;
void *g_pLisaFramebufferMirror2 = NULL;
void *g_pLisaFreeObjectsArray = NULL;
void *g_pLisaGridCells = NULL;
void *g_pLisaScanlineBuffer = NULL;
void *g_pLisaShutdownCallbacks = NULL;
void *g_pLisaSpanBuffer = NULL;
void *g_pLisaSubmeshPolygon = NULL;
void *g_pLisaTexturePagePointers = NULL;
void *g_pLisaTexturePageTable1 = NULL;
void *g_pLisaTexturePageTable2 = NULL;
void *g_pLisaTextureSheets = NULL;
void *g_pPlayerResultsCarIcon = NULL;
void *g_pRacePositionBadgeSprite = NULL;
void *g_pRadarCarBlipSprites = NULL;
void *g_pRadarEliminatedBlipSprites = NULL;
void *g_pRadarLeaderArrowSprite = NULL;
void *g_pRadarProgressBarSprite_HighRes = NULL;
void *g_pRadarProgressBarSprite_LowRes = NULL;
void *g_pRainMesh_P1 = NULL;
void *g_pRainMesh_P2 = NULL;
void *g_pResultsCarIcons = NULL;
void *g_pScreenConfig = NULL;
void *g_pSilverTrophySprite = NULL;
void *g_pTrackRoadSequence = NULL;
void *g_pTrafficLightGreenSprite = NULL;
void *g_pTrafficLightRedSprite = NULL;
void *g_pTrafficLightYellow1Sprite = NULL;
void *g_pTrafficLightYellow2Sprite = NULL;
void *g_pTurboGaugeBorderSprite = NULL;
void *g_pTurboIndicatorLightOnSprite = NULL;
void *g_pTurboIndicatorLightSprites = NULL;
void *g_pWheelTransforms = NULL;
void *g_pWrongWayBannerSprite = NULL;
void *g_ppLisaNextFreeObject = NULL;
float k_CrashElevationStepDown = 0.0f;
float k_CrashElevationStepUp = 0.0f;
float k_CrashSoundRandomChance = 0.0f;
float k_CrashTumblePitchDelta = 0.0f;
float k_CrashTumbleRollDelta = 0.0f;
float k_RespawnAngleMin = 0.0f;
float k_RespawnAngleModulo = 0.0f;
float k_RespawnHeadingMax = 0.0f;
float k_RespawnHeadingMin = 0.0f;
float k_RespawnInterpolationScale = 0.0f;
float k_RespawnSplineOffset = 0.0f;
float k_RespawnVehicleClearance = 0.0f;
float k_SkidSmokeProbabilityFactor = 0.0f;
float k_SkidSmokeRandomScale = 0.0f;
float k_SkidSmokeSpeedThreshold = 0.0f;
float k_SpecialCarBonusLimit = 0.0f;
float k_SpecialCarBonusStep = 0.0f;
float k_SplashAccumIncrement = 0.0f;
float k_TireDirtSpeedThreshold = 0.0f;
float k_TurboProgressMinTimer = 0.0f;
float k_WaterSplashSpeedThreshold = 0.0f;
float k_WreckRespawnAngleMin = 0.0f;
float k_WreckRespawnAngleModulo = 0.0f;
float k_WreckRespawnHeadingMax = 0.0f;
float k_WreckRespawnHeadingMin = 0.0f;
float k_WreckRespawnInterpolationScale = 0.0f;
float k_WreckRespawnSplineOffset = 0.0f;
float k_WreckRespawnVehicleClearance = 0.0f;
float k_WrongWayThresholdMax = 0.0f;
float k_WrongWayThresholdMin = 0.0f;
float k_WrongWayTrackHeadingOffset = 0.0f;
char s_AT_FIRST_PLACE_WITH_00496a11[512] = {0};
char s_AT_SECOND_PLACE_WITH_00496a34[512] = {0};
char s_AT_THIRD_PLACE_WITH_00496a57[512] = {0};
char s_A_SCORE_OF__d_PTS__00496a7a[512] = {0};
char s_CD_TRACK_00494db2[512] = {0};
char s_CONGRATULATIONS__004969a8[512] = {0};
char s_CONTINUE_00494d58[512] = {0};
char s_Cannot_use_this_graphics_mode__004989dc[512] = {0};
char s_DEFAULT_00494dee[512] = {0};
char s_Error_while_changing_car_mesh_00499248[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_ANIM_OBJ_004994ac[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_00499530[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_HANDLE_CAR_004995a4[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_HANDLE_SHA_00499558[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_HANDLE_WHE_00499580[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_TRAN_SPRIT_004993b4[512] = {0};
char s_FEL_VID_LI_MOVEOBJECT_WATER_SPLA_00499468[512] = {0};
char s_FEL_VID_LI_PLACEOBJECT_HANDLE_CA_00499508[512] = {0};
char s_FPS__d_00499658[512] = {0};
char s_GOLD_STATUE_TO_ADVANCE__0049707b[512] = {0};
char s_NOW_TRY_THE__s_LEAGUE__00496f68[512] = {0};
char s_OLDROADNR__d_00499630[512] = {0};
char s_PLACE_OR_BETTER_TO_PROCEED__00496452[512] = {0};
char s_PLAY_TRACK_AGAIN___Y_N__00495698[512] = {0};
char s_PRESS_RETURN_TO_CONTINUE_00495860[512] = {0};
char s_QUIT_00494d94[512] = {0};
char s_QUIT___Y_N__00495248[512] = {0};
char s_RANDOM_00494e0c[512] = {0};
char s_RECORDING__0049964c[512] = {0};
char s_RESTART_00494d76[512] = {0};
char s_RESTART___Y_N__00495300[512] = {0};
char s_RETRY_00494dd0[512] = {0};
char s_ROLL___1f_00499640[512] = {0};
char s_SKILLNAD___3f_004995f8[512] = {0};
char s_SORRY__YOU_MUST_REACH_THIRD_00496420[512] = {0};
char s_SPEED___3f_004995ec[512] = {0};
char s_SQUASHED2__d_00499620[512] = {0};
char s_TOTAL_SCORE_004955e0[512] = {0};
char s_TRACK_RESULTS_00495470[512] = {0};
char s_TRACK_SCORE_00495528[512] = {0};
char s_TYPE1__d_00499614[512] = {0};
char s_TYPE2__d_00499608[512] = {0};
char s_TrackSurfaceMissing[512] = {0};
char s_WAITING_FOR_HOST_004957a8[512] = {0};
char s_WELL_DONE__PRESS_RETURN_TO_ADVAN_004961c8[512] = {0};
char s_YOU_HAVE_COMPLETED_THE_004969cb[512] = {0};
char s_YOU_HAVE_COMPLETED_THIS_DIFFICUL_00495f70[512] = {0};
char s_YOU_MUST_ACHIEVE_THE_00497058[512] = {0};
char s__d_PTS_00496948[512] = {0};
char s__s_CHAMPIONSHIP_004969ee[512] = {0};
char s_pal_checksum_fmt[512] = {0};
char s_pal_chk_str1[512] = {0};
char s_pal_chk_str2[512] = {0};
char s_rb[512] = {0};
char s_tab_tab[512] = {0};

/* TODO: unported (called from fx.c); void signature matches lisa3d.h */
void Lisa_ResetRasterizerContext(void) { }
