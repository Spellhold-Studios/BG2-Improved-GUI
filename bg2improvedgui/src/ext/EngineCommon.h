#ifndef ENGINECOMMON_H
#define ENGINECOMMON_H

#include "objcre.h"
#include "engmagebk.h"

void EngineCommon_ApplySoundset(CCreatureObject& cre);

void EngineNoActive_asm();
void CChitin_WinMain_AddWaitForEvent_asm();
void CBaldurChitin_MainAIThread_TriggerDisplaySync_asm();
void CBaldurChitin_MainAIThread_DoubleRateFrame_asm();
void CreateWinMainSyncEvent();
void TimeGetTime_WaitCycle_asm();
void CBaldurChitin_InitXXX_SetKeyboardSpeed_asm();
void CChitin_SynchronousUpdate_CheckScreen_asm();
void CChitin_WinMain_AddDisplayDoneEvent_asm();
void RenderOneFrame_asm();
void WaitDisplay_asm();
void SleepEx_Emu_asm();
void SleepEx_Log_asm();
void CChitin_OnAltTab_asm();
void CChitin_AsynchronousUpdate_CheckWindowEdge_asm();
void CChitin_AsynchronousUpdate_CheckCursorPT_asm();
void GetAsyncKeyState_asm();
void CInfGame_Marshal_SaveGameInject_asm();
void CInfGame_UnMarshal_LoadGameInject_asm();
int __cdecl z_uncompress(uchar *dest, ulong *destLen, uchar *source, ulong sourceLen);
int __cdecl z_compress2(uchar *dest, ulong *destLen, uchar *source, ulong sourceLen, int level);
bool IsBG1Part();
void FakeGetDiskFreeSpaceA_asm();
void CProjectileBAM_AIUpdate_LogTick_asm();
void CProjectileBAM_Render_LogTick_asm();
void CProjectileBAM_CProjectileBAM_asm();
void CAnimationD000_PatchTickRate_asm();
void CAnimation_Render_LogTick_asm();
void CGameSprite_AIUpdateFly_LogTick_asm();
void CGameSprite_ProcessEffectList_IgnoreFly_asm();
void CGameSprite_AIUpdate_IncrementFrame_asm();
void CInfinity_Render_Log_asm();
void CGameArea_Render_Log_asm();
void AdjustViewPosition_ContinueScroll_asm();
void AdjustViewPosition_Dividex2_asm();
void Scroll_Log_asm();
void AdjustViewPosition_GetScrollSpeed_asm();


#endif //ENGINECOMMON_H