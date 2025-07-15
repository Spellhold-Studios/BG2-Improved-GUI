#include "chitin.h"

struct CCreatureFileMemorizedSpell         // Size=0xc
{
    ResRef SpellId;                 // Offset=0x0
    ushort flags;                   // Offset=0x8
    ushort pad;                     // Offset=0xa
};


class CEffectDisableButton_Shaman : public CEffect {
    public:
	virtual void OnRemove(CCreatureObject& creTarget);
};


void CRuleTables_GetClassString_asm();
void ButtonCharGenClassSelection_GetClassFromButtonIndex_asm();
void CRuleTables_GetClassStringLower_asm();
void ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF_asm();
void ButtonCharGenClassSelection_OnLButtonClick_OpenSubKits_asm();
void CScreenCreateChar_CompleteCharacterClass_SetAnimIDClassSuffix_asm();
void CRuleTables_GetTHAC0_asm();
void CRuleTables_GetClassStringMixed_asm();
void CScreenCreateChar_ResetAbilities_RollStrengthEx_asm();
void CScreenCreateChar_IsDoneButtonClickable_EnableMultiClassPanel_asm();
void ButtonCharGenMenu_OnLButtonClick_PrepareSkillPoints_asm();
void CRuleTables_GetProficiencySlots_SelectClassLevels_asm();
void CRuleTables_GetProficiencyClassIndex_asm();
void CRuleTables_GetNumLevelUpAbilities_asm();
void CRuleTables_GetMaxMemorizedSpellsPriest_asm();

void CScreenCharacter_ResetLevelUpPanel_SetSkillPoints_asm();
void CScreenCharacter_UpdateLevelUpPanel_SelectSkills_asm();
void CScreenCharacter_UpdateLevelUpPanel_SetFirstSkillType_asm();

void CScreenGenChar_UpdateMainPanel_ShowProficienciesList_asm();
void CScreenGenChar_UpdateMainPanel_SummonPopup_asm();
void CScreenGenChar_UpdateMainPanel_ShowPriestSpellList_asm();

void CScreenGenChar_ResetChooseMagePanel_SwitchToShaman_asm();
void GetXXXSpell_asm();
void GetKnownSpellIndexXXX_asm();
void CScreenGenChar_ResetChooseMagePanel_CheckAllowedSpell_asm();
void CScreenGenChar_ResetChooseMagePanel_PushBOOKNAME_asm();

void ButtonCharGenChooseMageSelection_AddKnownSpell_asm();
void ButtonCharGenChooseMageSelection_RemoveKnownSpell_asm();
void CRuleTables_GetHitPoints_asm();
void CRuleTables_RollHitPoints_asm();
void CGameSprite_GetSkillValue_asm();
void CRuleTables_FindSavingThrow_asm();
void CRuleTables_GetSavingThrow_asm();
void CScreenCharacter_UpdateExperience_asm();
void CScreenMultiPlayer_UpdateExperience_asm();
void CRuleTables_GetNextLevelXP_asm();
void CScreenCreateChar_CompleteCharacterWrapup_PriestSpells_asm();
void CScreenCreateChar_CompleteCharacterWrapup_MageSpells_asm();
void Object_IsUsableSubClass_asm();
void CRuleTables_GetNextLevel_asm();
void CGameSprite_AddNewSpecialAbilities_asm();
void CRuleTables_GetClassAbilityTable_asm();
void CScreenCharacter_ResetLevelUpPanel_ShowNewPriestSpells_asm();
void CScreenCharacter_UpdateMainPanel_ShowProficiencies_asm();
void CRuleTables_GetClassHelp_asm();
void CRuleTables_GetHPCONBonusTotal_asm();
void CGameSprite_CheckCombatStatsWeapon_SetTHACPenalty_asm();
void CheckItemNotUsableByClass_asm();
void CheckItemUsableByClass_asm();

void CScreenWizSpell_UpdateMainPanel_SetSorcererViewMode_asm();
void CScreenWizSpell_UpdateMainPanel_GetKnownSpellMage_asm();
void IsArcaneClass_asm();
void CGameSprite_SorcererSpellCount_asm();
void CScreenWizSpell_UpdateMainPanel_GetCDSMaxMemSpells_asm();
void UpdateLeftPanelButtonStates_asm();
void LeftPanel_LClick_asm();
void CGameSprite_Rest_AddShaman_asm();
void CGameSprite_Spell_AddShaman_asm();
void CGameSprite_SpellPoint_AddShaman_asm();
void CGameSprite_SorcererSpellDecrement1_asm();
void CGameSprite_SorcererSpellDecrement2_asm();
void CGameSprite_RemoveSpellsPriest_AddShaman_asm();
void CGameSprite_RemoveSpell_AddShaman_asm();
void CGameSprite_SetMemorizedFlag_AddShaman_asm();
void CEffectRememorizeSpell_ApplyEffect_AddShaman_asm();
void CGameSprite_SorcererSpellRememorize1_asm();
void CGameSprite_SorcererSpellRememorize2_asm();
void CDerivedStats_GetPriestLevelCast_AddShaman_asm();
void CGameSprite_GetMemorizedSpellPriest_SkipSpell_asm();
void CInfGame_SelectToolbar_AddShaman_asm();
void CInfButtonArray_SetState_AddShaman_asm();
void CGameSprite_FeedBack_FindTraps_asm();
void CGameSprite_FeedBack_StopFindTraps_asm();
void CScreenCharacter_OnDoneButtonClick_FakeSorcerer_asm();
void CScreenCharacter_OnDoneButtonClick_GetXXXLevel_asm();
void CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm();
void CScreenCharacter_ResetChooseMagePanel_FakeSorcerer_asm();
void ButtonCharacterChooseMageSelection_OnLButtonClick_RemoveKnownSpell_asm();
void ButtonCharacterChooseMageSelection_OnLButtonClick_AddKnownSpell_asm();
void CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell_asm();
void CScreenCharacter_ResetLevelUpPanel_SetOldSorcererLevel_asm();
void CScreenCharacter_ResetChooseMagePanel_BookName_asm();
void CInfButtonArray_UpdateButtons_ToolTipFindTraps_asm();
void CInfButtonArray_UpdateButtons_ToolTipBattleSong_asm();
void CGameSprite_GetSecretDoorDetection_asm();
void CRuleTables_GetBaseLore_asm();
void CGameSprite_RemoveNewSpecialAbilities_asm();
void CRuleTables_GetNumQuickWeaponSlots_asm();
void CDerivedStats_GetWizardLevel_asm();
void CScreenCharacter_OnCancelButtonClick_GetXXXLevel_asm();
void CScreenCharacter_OnCancelButtonClick_SPLSRCKN1_asm();
void CScreenCharacter_OnCancelButtonClick_SPLSRCKN2_asm();
void CScreenCharacter_RemoveAbilities_RemoveAllSpellsPriest_asm();
void CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints_asm();
void CGameEffectLevelDrain_OnAddSpecific_asm();
void CGameEffectDisableSpellType_ApplyEffect_asm();
void GetMultiplayerClassName_asm();
void CScreenCreateChar_GetCharacterVersion_asm();
void CRuleTables_GetRaiseDeadCost_asm();
void CScreenMultiPlayer_ResetViewCharacterPanel_ShowMageSpells_asm();
void CScreenMultiPlayer_ResetViewCharacterPanel_ShowPriestSpells_asm();
void CGameSprite_FeedBack_StartSongText_asm();
void CGameSprite_FeedBack_StopSongText_asm();
void CGameEffectDisableSpellType_ApplyEffect_TextFeedBack_asm();
void CGameEffectDisableSpellType_ApplyEffect_RemoveQuickSpells_asm();
void CEffectDisableButton_OnRemove_Shaman_asm();
void CGameSprite_SetPath_CancelDance_asm();
void CGameSprite_ExecuteAction_MakeUnselectable_asm();
void CGameSprite_ProcessPendingTriggers_asm();
void CGameEffectRandomSummon_Apply_CheckLimits_asm();
void CEffectSpellOnCondition_Apply_AddCriticalHit_asm();
void CInfButtonArray_PreRenderButton_asm();
//void ShamanDance_InitIcon();


void Log_GetClass_asm();

class CUIButtonShamanClass : public CUICheckButton {
public:
    CUIButtonShamanClass(CPanel& panel, ChuFileControlInfoBase& controlInfo);
    virtual ~CUIButtonShamanClass() {} //v0
};