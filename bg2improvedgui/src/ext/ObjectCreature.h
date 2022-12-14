#ifndef OBJECTCREATURE_H
#define OBJECTCREATURE_H

#include "objcre.h"

extern CCreatureObject& (CCreatureObject::*Tramp_CCreatureObject_Construct_10)(void*, unsigned int, BOOL, int, int, int, unsigned int, int, int, int);
extern CreFileKnownSpell* (CCreatureObject::*Tramp_CCreatureObject_GetKnownSpellPriest)(int, int);
extern CreFileKnownSpell* (CCreatureObject::*Tramp_CCreatureObject_GetKnownSpellMage)(int, int);
extern BOOL (CCreatureObject::*Tramp_CCreatureObject_AddMemSpellPriest)(int, int, int*);
extern BOOL (CCreatureObject::*Tramp_CCreatureObject_AddMemSpellMage)(int, int, int*);
extern void (CCreatureObject::*Tramp_CCreatureObject_ValidateAttackSequence)(char*);
extern BOOL (CCreatureObject::*Tramp_CCreatureObject_EvaluateStatusTrigger)(Trigger&);
extern ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ExecuteAction)();
extern short (CCreatureObject::*Tramp_CCreatureObject_GetProficiencyInItem)(CItem&);
extern ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ActionPickPockets)(CCreatureObject&);
extern ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ActionJumpToAreaEntranceMove)(IECString);
extern void (CCreatureObject::*Tramp_CCreatureObject_UpdateFaceTalkerTimer)();
extern void (CCreatureObject::*Tramp_CCreatureObject_SetCurrentAction)(Action&);

class DETOUR_CCreatureObject : public CCreatureObject {
public:
	CCreatureObject& DETOUR_Construct(void* pFile, unsigned int dwSize, BOOL bHasSpawned, int nTicksTillRemove, int nMaxMvtDistance, int nMaxMvtDistanceToObject, unsigned int nSchedule, int nDestX, int nDestY, int nFacing);
	CreFileKnownSpell* DETOUR_GetKnownSpellPriest(int nLevel, int nIndex);
	CreFileKnownSpell* DETOUR_GetKnownSpellMage(int nLevel, int nIndex);
	BOOL DETOUR_AddMemSpellPriest(int nLevel , int nIndex, int* pIndex);
	BOOL DETOUR_AddMemSpellMage(int nLevel , int nIndex, int* pIndex);
	void DETOUR_ValidateAttackSequence(char* pSeq);
	BOOL DETOUR_EvaluateStatusTrigger(Trigger& t);
	ACTIONRESULT DETOUR_ExecuteAction();
	short DETOUR_GetProficiencyInItem(CItem& itm);
	ACTIONRESULT DETOUR_ActionPickPockets(CCreatureObject& creTarget);
	ACTIONRESULT DETOUR_ActionJumpToAreaEntranceMove(IECString sArea);
	void DETOUR_UpdateFaceTalkerTimer();
    void DETOUR_SetCurrentAction(Action&);
};

void __stdcall CCreatureObject_PrintExtraCombatInfoText(CCreatureObject& creSelf, IECString& sText);
BOOL __stdcall CCreatureObject_ShouldAvertCriticalHit(CCreatureObject& creTarget, CCreatureObject& creSource);
BOOL __stdcall CCreatureObject_ApplyDamage_TryBackstab(CCreatureObject& creSource, CItem& itmMain, ItmFileAbility& abilMain, ItmFileAbility& abilLauncher, short orTarget, short orToTarget, CCreatureObject& creTarget);
int __stdcall CCreatureObject_ApplyDamage_CalculateDamageBonus(CCreatureObject& creSource, ItmFileAbility& abilMain, short* pwDamage, CCreatureObject& creTarget);
BOOL __stdcall CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly(CCreatureObject& cre);
void __stdcall CCreatureObject_GetClassAbilities(CCreatureObject& cre, unsigned char cClass, int nLevels, IECPtrList& cpl);
void __stdcall CCreatureObject_GetClassAbilities(CCreatureObject& cre, CDerivedStats& cdsTarget, IECPtrList& cpl);
void __stdcall CCreatureObject_JoinParty_UpdateClassAbilities(CCreatureObject& cre, CDerivedStats& cds);
BOOL __stdcall CCreatureObject_Spell_IsOverrideInvisible(CCreatureObject& creSource, CCreatureObject& creTarget);
BOOL __stdcall CCreatureObject_IsDeadInFrontVerticalList(CCreatureObject& cre);
BOOL __stdcall CCreatureObject_Spell_IsOverrideSilence(CCreatureObject& creSource);
LPCTSTR __stdcall CCreatureObject_DoSpellCasting_GetGenderLetter(CCreatureObject& creSource, ResSplContainer& resSpell, SplFileAbility& ability);
short __stdcall CCreatureObject_DoSpellCasting_GetCastingSpeed(CCreatureObject& creSource, SplFileAbility& ability);
BOOL __stdcall CCreatureObject_UseItem_CannotTargetInvisible(CCreatureObject& creSource, CCreatureObject& creTarget);
void __stdcall CCreatureObject_UseItem_OverrideAnimation(CCreatureObject& creSource);
BOOL __stdcall CCreatureObject_AttackOnce_DoHalfAttack(CCreatureObject& creSource, char cInRoundIdx);
void __stdcall CCreatureObject_UpdateModalState_DoBardSongNormal(CCreatureObject& creSource);
void __stdcall CCreatureObject_SetDifficultyLuckModifier(CCreatureObject& cre);


void DisarmAction_asm();
void CCreatureObject_ShouldAvertCriticalHit_SpellOnly_asm();
void CGameAIBase_FireSpell_InjectProjectileUpdate_asm();
void CInfGame_GetCharacterPortrait_FakePartyMember_asm();
void CGameSprite_FloatingHP_SetText_asm();
void CChitin_AsynchronousUpdate_CheckAltKey_asm();
void CGameSprite_Marshal_FirstCharFix_asm();
void CGameSprite_CheckCombatStatsWeapon_AddElfRacialThacBonus_asm();
void CGameSprite_CheckCombatStatsWeapon_AddHalflingRacialThacBonus_asm();
void CGameSprite_CheckCombatStatsWeapon_SwapWeaponText_asm();
void CGameSprite_DoMoraleFailure_UpdateIcons_asm();
void CGameSprite_DoMoraleFailure_AddBerserk_asm();
void CGameSprite_DoMoraleFailure_AddRunAway_asm();
void CGameSprite_DoMoraleFailure_AddPanic_asm();
void CGameSprite_EndMoraleFailure_RemoveIcon_asm();
void CGameSprite_Swing_FixWeaponSpeed_asm();
void CGameSprite_Render_CheckInvisibiltyEffects_asm();
void CGameSprite_RenderMarkers_CheckInvisibiltyEffects_asm();
//void CGameSprite_MoveToPoint1_asm();
//void CGameSprite_MoveToPoint2_asm();
void CGameSprite_CheckBumpable_asm();
void CGameSprite_CheckBumpable2_asm();
void CGameSprite_ClearBumpPath_CheckParty_asm();

#endif //OBJECTCREATURE_H