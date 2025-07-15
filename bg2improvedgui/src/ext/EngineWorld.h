#ifndef ENGINEWORLD_H
#define ENGINEWORLD_H

#include "engworld.h"

extern POSITION (CScreenWorld::*Tramp_CScreenWorld_PrintToConsole_6)(IECString&, IECString&, ABGR, ABGR, int, bool);
extern POSITION (CScreenWorld::*Tramp_CScreenWorld_PrintToConsole_4)(IECString&, IECString&, int, bool);

class DETOUR_CScreenWorld : public CScreenWorld {
public:
	POSITION DETOUR_PrintToConsoleColor(IECString& sLeft, IECString& sRight, ABGR colLeft, ABGR colRight, int nUserArg, bool bResetScrollbar);
	POSITION DETOUR_PrintToConsole(IECString& sLeft, IECString& sRight, int nUserArg, bool bResetScrollbar);
};

void CScreenWorldMap_EngineGameInit_asm();
void CScreenWorldMap_EngineActivated_asm();
void xxx_InvalidateCursorRect_asm();

void CScreenWorld_Pause_asm();
void CTimerWorld_HardPause_asm();
void CTimerWorld_StopTime_asm();

void CScreenWorld_Unpause_asm();
void CTimerWorld_HardUnPause_asm();
void CTimerWorld_StartTime_asm();
void CGameArea_OnActivation_asm();

void UpdateSoundList_ActivateFromAllLoopingQueue_asm();
void SoundPlay0CheckPause_asm();
void SoundPlay1CheckPause_asm();
void SetVolumeCheckPause_asm();
void CSoundMixer_UpdateSoundList_RemoveFromPlayingNow_asm();
void CSoundMixer_UpdateSoundList_MismatchAreaRemoveFromPlayingNow_asm();
void CSoundMixer_UpdateSoundListPriority_RemoveFromPlayingNow_asm();
void CSoundMixer_UpdateSoundListPriority_MismatchAreaRemoveFromPlayingNow_asm();
void CSound_PlayWaiting_asm();
void CSoundMixer_TransferBuffer_asm();
void CMessage_TimeStop_asm();
void TimeStopEnded_asm();
void CVoice_CVoice_asm();
void CScreenStore_Activated_asm();
void CScreenWorld_TogglePauseGame_UnPause_asm();

#endif //ENGINEWORLD_H