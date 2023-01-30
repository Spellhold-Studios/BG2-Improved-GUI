#ifndef SOUNDCORE_H
#define SOUNDCORE_H

#include "sndcore.h"

extern BOOL (CSoundMixer::*Tramp_CSoundMixer_InitSonglist)(int, char**);

extern char gCastVoiceFilePrefix[]; 

struct DETOUR_CSoundMixer : public CSoundMixer {
	BOOL DETOUR_InitSonglist(int nSongs, char** pSongFileArray);
};

void CResWave_CopyWaveData_Normalize_asm();
void CCreatureObject_PlaySound_COMMON_asm();
void CCreatureObject_PlaySound_MORALE_asm();
void CCreatureObject_PlaySound_HAPPY_asm();
void CCreatureObject_PlaySound_UNHAPPYANNOYED_asm();
void CCreatureObject_PlaySound_UNHAPPYSERIOUS_asm();
void CCreatureObject_PlaySound_UNHAPPYBREAKINGPOINT_asm();
void CCreatureObject_PlaySound_LEADER_asm();
void CCreatureObject_PlaySound_TIRED_asm();
void CCreatureObject_PlaySound_BORED_asm();
void CCreatureObject_PlaySound_DAMAGE_asm();
void CCreatureObject_PlaySound_DYING_asm();
void CCreatureObject_PlaySound_HURT_asm();
void CCreatureObject_PlaySound_AREA_FOREST_asm();
void CCreatureObject_PlaySound_AREA_CITY_asm();
void CCreatureObject_PlaySound_AREA_DUNGEON_asm();
void CCreatureObject_PlaySound_AREA_DAY_asm();
void CCreatureObject_PlaySound_AREA_NIGHT_asm();
void CCreatureObject_PlaySound_ACTION_asm();
void CCreatureObject_PlaySound_ACTION2_asm();
void CGameSprite_VerbalConstant_asm();


//void CCreatureObject_PlaySound_LimitActionSoundToBG1_asm();
void CSound_Stop_Logging_asm();
void CSound_Stop_Logging2_asm();
void CSoundMixer_ClearChannel_Logging_asm();
void CGameSprite_DecodeSwingSound_DisableExclusive_asm();
void CGameSprite_ApplyCastingEffect_PatchFilePrefix_asm();
void CGameSprite_ApplyCastingEffectPost_PatchFilePrefix_asm();
void CGameSprite_ApplyCastingEffect_PatchAnimId_asm();
void CSoundMixer_CleanUp_Log_asm();
void CSoundMixer_Initialize_Log_asm();
void CGameArea_OnActivation_Log_asm();

void CCacheStatus_Update_Log_asm();
void CSound_SetVolume_Log2_asm();
void CSound_ResetVolume_Log2_asm();
void CSoundMixer_Initialize_Log3_asm();
void CInfGame_LoadGame_ReActivateArea_asm();



#endif //SOUNDCORE_H