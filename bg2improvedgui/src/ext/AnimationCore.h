#ifndef ANIMATIONCORE_H
#define ANIMATIONCORE_H

#include "animcore.h"

extern void (CAnimation::*Tramp_CAnimation_PlayCurrentSequenceSound)(CCreatureObject&);
extern BOOL (*Tramp_CAnimation_IsAnimation5000)(unsigned short);
extern LPCTSTR (CAnimation::*Tramp_CAnimation_GetWalkingSound)(short);

class DETOUR_CAnimation : public CAnimation {
public:
	void DETOUR_PlayCurrentSequenceSound(CCreatureObject& cre);
	static BOOL DETOUR_IsAnimation5000(unsigned short wAnimId);
	LPCTSTR DETOUR_GetWalkingSound(short wTerrainCode);
};

void CItem_TranslateAnimationType_CheckMageAnimation_asm();
void CItem_TranslateAnimationType_FixAnimCode_asm();
void CAnimation5000_SetArmorSound_asm();
void CAnimation6400_SetArmorSound_asm();
void CAnimation5000_SetArmorSoundRobe_asm();
void CAnimation6400_SetArmorSoundRobe_asm();
void CAnimation1000_SetReadySound_asm();
void CAnimationA000_SetReadySound_asm();
void CAnimationXXX_OverrideAniSndReference_asm();
void CAnimation_SetAnimationType_PostHooks_asm();
void CGameSpriteSwing_ForceProcessMessage_asm();
void CGameSpriteSwing_SwapMessage_asm();
void CGameSprite_DecodeSwingSound_Attack4_asm();
void CAnimation2000_SetAnimationSequence_SetShootSeq_asm();

void CAnimation1000_InjectPrefixOverride_asm();
void CAnimation1200_InjectPrefixOverride_asm();
void CAnimation1300_InjectPrefixOverride_asm();
void CAnimation2000_InjectPrefixOverride_asm();
void CAnimation3000_InjectPrefixOverride_asm();
void CAnimation5000_InjectPrefixOverride_asm();
void CAnimation6400_InjectPrefixOverride_asm();
void CAnimation7000_InjectPrefixOverride_asm();
void CAnimation7300_InjectPrefixOverride_asm();
void CAnimation8000_InjectPrefixOverride_asm();
void CAnimation9000_InjectPrefixOverride_asm();
void CAnimationA000_InjectPrefixOverride_asm();
void CAnimationC000_InjectPrefixOverride_asm();
void CAnimationE000_InjectPrefixOverride_asm();
void CInfGame_GetAnimationBam_InjectPaperDollPrefix_asm();
void CUIButtonInventoryAppearance_GetAnimationVidCell_asm();
void CAnimation5000_EquipArmor_SkipPrefixChange_asm();
void CAnimation6400_EquipWeapon_SkipOffHand_asm();
void CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm();
void CAnimation6400_SetColorRange_CheckOffHandWeapon_asm();
void CGameSprite_SetSequence_Log_asm();
void CCreativeObject_SetCurrentAction_SetREADY_asm();

#define isInfinityAnimationActive ( *(unsigned int *)0xAA470E == 0x4141B500 )

#endif //ANIMATIONCORE_H