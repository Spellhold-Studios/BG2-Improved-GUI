#ifndef AREACOMMON_H
#define AREACOMMON_H

#include "arecore.h"

void CArea_RemoveAreaAirEffectSpecific(CArea& area, ResRef& rResource);
void Area_GetFirstNode_asm();
void Area_GetPrevNode_asm();
void Area_DumpEnum_asm();

void CGameArea_OnActivation_SetSongTypeDay_asm();
void CGameArea_OnActivation_SetSongTypeNight_asm();
void CMarker_RenderSprite_asm();
void CGameSprite_LeaveArea_CheckBattleSong_asm();
void CGameSprite_LeaveAreaName_CheckBattleSong_asm();

#endif //AREACOMMON_H