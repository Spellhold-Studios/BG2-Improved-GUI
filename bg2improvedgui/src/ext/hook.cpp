#include "hook.h"

#include "stdafx.h"
#include "detoursext.h"
#include "options.h"

#include "AnimationCore.h"
#include "Animation1000.h"
#include "Animation1200.h"
#include "Animation1300.h"
#include "Animation2000.h"
#include "Animation5000.h"
#include "Animation6400.h"
#include "Animation7000.h"
#include "Animation7300.h"
#include "Animation8000.h"
#include "Animation9000.h"
#include "AnimationA000.h"
#include "AnimationC000.h"
#include "AnimationE000.h"
#include "AreaCore.h"
#include "ChitinCore.h"
#include "DialogCore.h"
#include "EffectCore.h"
#include "EffectOpcode.h"
#include "EngineChargen.h"
#include "EngineInventory.h"
#include "EngineRecord.h"
#include "EngineMageBook.h"
#include "EnginePriestBook.h"
#include "EngineWorld.h"
#include "InfButtonArray.h"
#include "InfGameCore.h"
#include "ItemCore.h"
#include "LogCore.h"
#include "LuaCore.h"
#include "NetworkCore.h"
#include "ObjectCore.h"
#include "ObjectCreature.h"
#include "ObjectStats.h"
#include "ScriptCore.h"
#include "SoundCore.h"
#include "SpellCore.h"
#include "UserCore.h"
#include "UserButton.h"
#include "UserMageBook.h"
#include "HandOffSlot.h"

void InitHooks() {

    //General
    DetourMemberFunction(Tramp_CBaldurChitin_Construct, DETOUR_CBaldurChitin::DETOUR_Construct);
    DetourMemberFunction(Tramp_CRuleTables_Construct, DETOUR_CRuleTables::DETOUR_Construct);
    DetourMemberFunction(Tramp_CRuleTables_Deconstruct, DETOUR_CRuleTables::DETOUR_Deconstruct);

    //Action
    if (pGameOptionsEx->bActionExpandedActions) {
        DetourMemberFunction(Tramp_CInfGame_InitGame, DETOUR_CInfGame::DETOUR_InitGame);
        DetourMemberFunction(Tramp_CGameAIBase_QueueActions, DETOUR_CGameAIBase::DETOUR_QueueActions);
        DetourMemberFunction(Tramp_CGameAIBase_ClearAllActions, DETOUR_CGameAIBase::DETOUR_ClearAllActions);
        DetourMemberFunction(Tramp_CGameAIBase_ExecuteAction, DETOUR_CGameAIBase::DETOUR_ExecuteAction);
        DetourMemberFunction(Tramp_CCreatureObject_ExecuteAction, DETOUR_CCreatureObject::DETOUR_ExecuteAction);
    }

    //Debug
    DetourFunction(Tramp_CloseLogAndErr, DETOUR_CloseLogAndErr);

    if (pGameOptionsEx->bDebugExpandedLUAConsole) {
        DetourFunction(Tramp_IElua_init, DETOUR_IElua_init);
        DetourMemberFunction(Tramp_CLUAConsole_StartStore, DETOUR_CLUAConsole::DETOUR_StartStore);
    }

    if (pGameOptionsEx->bDebugExternalLogging)
        DetourFunction(Tramp_WriteToFile, DETOUR_WriteToFile);

    if (pGameOptionsEx->bDebugLogDialogueBar) {
        DetourMemberFunction(Tramp_CScreenWorld_PrintToConsole_6, DETOUR_CScreenWorld::DETOUR_PrintToConsoleColor)
        DetourMemberFunction(Tramp_CScreenWorld_PrintToConsole_4, DETOUR_CScreenWorld::DETOUR_PrintToConsole)
    }

    if (pGameOptionsEx->bDebugLogFailures)
        DetourFunction(Tramp_AssertFailedQuit, DETOUR_AssertFailedQuit);

    if (pGameOptionsEx->bDebugLogWarnings)
        DetourFunction(Tramp_AssertFailedContinue, DETOUR_AssertFailedContinue);

    if (pGameOptionsEx->bDebugLogNetworkErrors)
        DetourMemberFunction(Tramp_CNetwork_PrintNetworkError, DETOUR_CNetwork::DETOUR_PrintNetworkError);

    if (pGameOptionsEx->bDisableHiddenPatches != TRUE) {
        //Effect
        DetourMemberFunction(Tramp_CEffect_CreateEffect, DETOUR_CEffect::DETOUR_CreateEffect);
        DetourMemberFunction(Tramp_CEffect_ApplyTiming, DETOUR_CEffect::DETOUR_ApplyTiming);
        DetourMemberFunction(Tramp_CEffect_CheckSave, DETOUR_CEffect::DETOUR_CheckSave);
        DetourMemberFunction(Tramp_CDerivedStats_OpAdd, DETOUR_CDerivedStats::DETOUR_OpAdd);
        DetourMemberFunction(Tramp_CEffectDamage_ApplyEffect, DETOUR_CEffectDamage::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CItem_GetAbilityEffect, DETOUR_CItem::DETOUR_GetAbilityEffect);
        DetourMemberFunction(Tramp_ResSplContainer_GetAbilityEffect, DETOUR_ResSplContainer::DETOUR_GetAbilityEffect);
        DetourMemberFunction(Tramp_CCreatureObject_ActionPickPockets, DETOUR_CCreatureObject::DETOUR_ActionPickPockets);

        //UI
        DetourFunction(Tramp_CreateUIControl, DETOUR_CreateUIControl);
        
        //used by bUserExternMageSpellHiding, bEngineExternMageSpellsCap
        DetourMemberFunction(Tramp_CRuleTables_GetMageSpellRef, DETOUR_CRuleTables::DETOUR_GetMageSpellRef);

        //uncleared lstTargetIds fix
        DetourMemberFunction(Tramp_CCreatureObject_SetCurrentAction, DETOUR_CCreatureObject::DETOUR_SetCurrentAction);

        // zero creature memory after allocating before calling broken constructor
        DetourMemberFunction(Tramp_CCreatureObject_Construct_10, DETOUR_CCreatureObject::DETOUR_Construct);
    }
    
    if (pGameOptionsEx->bEffApplyEffItemtypeFix ||
        pGameOptionsEx->bEffApplyEffItemFix ||
        pGameOptionsEx->bUI_UnlimitedHandOffSlot) {
        DetourMemberFunction(Tramp_CItem_Equip, DETOUR_CItem::DETOUR_Equip);
    }

    if (pGameOptionsEx->bUI_UnlimitedHandOffSlot) {
        DetourMemberFunction(Tramp_CItem_UnEquip, DETOUR_CItem::DETOUR_UnEquip);
    }

    if (pGameOptionsEx->bEffAttacksPerRoundFix) {
        DetourMemberFunction(Tramp_CEffectAttacksPerRoundMod_ApplyEffect, DETOUR_CEffectAttacksPerRoundMod::DETOUR_ApplyEffect);
    }

    if (pGameOptionsEx->bEffCharismaModFix)
        DetourMemberFunction(Tramp_CEffectCharismaMod_ApplyEffect, DETOUR_CEffectCharismaMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffConstitutionModFix)
        DetourMemberFunction(Tramp_CEffectConstitutionMod_ApplyEffect, DETOUR_CEffectConstitutionMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffBlindnessFix || pGameOptionsEx->bEffBlindnessAsSpellDesc)
        DetourMemberFunction(Tramp_CEffectBlindness_ApplyEffect, DETOUR_CEffectBlindness::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffButtonFXMod &&
        pGameOptionsEx->bEngineExpandedStats) {
        DetourMemberFunction(Tramp_CEffectDisableButton_ApplyEffect, DETOUR_CEffectDisableButton::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectEnableButton_ApplyEffect, DETOUR_CEffectEnableButton::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CButtonArray_CheckButtonEnabled, DETOUR_CButtonArray::DETOUR_CheckButtonEnabled);
    }

    if (pGameOptionsEx->bEffCastSpellConditionMod) {
        DetourMemberFunction(Tramp_CEffectCastSpellOnCondition_ApplyEffect, DETOUR_CEffectCastSpellOnCondition::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CConditionalSpellList_EvaluateTriggers, DETOUR_CConditionalSpellList::DETOUR_EvaluateTriggers);
    }

    if (pGameOptionsEx->bEffCutScene2Mod)
        DetourMemberFunction(Tramp_CEffectCutScene2_ApplyEffect, DETOUR_CEffectCutScene2::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffDexterityMod)
        DetourMemberFunction(Tramp_CEffectDexterityMod_ApplyEffect, DETOUR_CEffectDexterityMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffDiseaseFix)
        DetourMemberFunction(Tramp_CEffectDisease_ApplyEffect, DETOUR_CEffectDisease::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffDisintegrateFix)
        DetourMemberFunction(Tramp_CEffectDisintegrate_ApplyEffect, DETOUR_CEffectDisintegrate::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffDispelFix)
        DetourMemberFunction(Tramp_CEffectList_TryDispel, DETOUR_CEffectList::DETOUR_TryDispel);

    if (pGameOptionsEx->bEffDispelMagicalItemConfig)
        DetourMemberFunction(Tramp_CEffectDispel_ApplyEffect, DETOUR_CEffectDispel::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffFatigueModFix)
        DetourMemberFunction(Tramp_CEffectFatigueMod_ApplyEffect, DETOUR_CEffectFatigueMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffFindTrapsModFix)
        DetourMemberFunction(Tramp_CEffectFindTrapsMod_ApplyEffect, DETOUR_CEffectFindTrapsMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffIntelligenceModFix)
        DetourMemberFunction(Tramp_CEffectIntelligenceMod_ApplyEffect, DETOUR_CEffectIntelligenceMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffIntoxicationModFix)
        DetourMemberFunction(Tramp_CEffectIntoxicationMod_ApplyEffect, DETOUR_CEffectIntoxicationMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffLearnSpellMod)
        DetourMemberFunction(Tramp_CEffectLearnSpell_ApplyEffect, DETOUR_CEffectLearnSpell::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffLevelModFix)
        DetourMemberFunction(Tramp_CEffectLevelMod_ApplyEffect, DETOUR_CEffectLevelMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffLoreModFix)
        DetourMemberFunction(Tramp_CEffectLoreMod_ApplyEffect, DETOUR_CEffectLoreMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffLuckMod)
        DetourMemberFunction(Tramp_CEffectLuckMod_ApplyEffect, DETOUR_CEffectLuckMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffMageMemSpellMod)
        DetourMemberFunction(Tramp_CEffectMageMemSpellMod_ApplyEffect, DETOUR_CEffectMageMemSpellMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffMagicResistModFix)
        DetourMemberFunction(Tramp_CEffectMagicResistMod_ApplyEffect, DETOUR_CEffectMagicResistMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffMoraleBreakModFix)
        DetourMemberFunction(Tramp_CEffectMoraleBreakMod_ApplyEffect, DETOUR_CEffectMoraleBreakMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffOpenLocksModFix)
        DetourMemberFunction(Tramp_CEffectOpenLocksMod_ApplyEffect, DETOUR_CEffectOpenLocksMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffPickPocketsModFix)
        DetourMemberFunction(Tramp_CEffectPickPocketsMod_ApplyEffect, DETOUR_CEffectPickPocketsMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffPoisonFix)
        DetourMemberFunction(Tramp_CEffectPoison_ApplyEffect, DETOUR_CEffectPoison::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffPoisonResistMod)
        DetourMemberFunction(Tramp_CEffectPoisonResistMod_ApplyEffect, DETOUR_CEffectPoisonResistMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffProficiencyModMod) {
        DetourMemberFunction(Tramp_CEffectProficiencyMod_ApplyEffect, DETOUR_CEffectProficiencyMod::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CDerivedStats_LimitStats, DETOUR_CDerivedStats::DETOUR_LimitStats);
    }

    if (pGameOptionsEx->bEffPriestMemSpellMod)
        DetourMemberFunction(Tramp_CEffectPriestMemSpellMod_ApplyEffect, DETOUR_CEffectPriestMemSpellMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffRegenerationFix)
        DetourMemberFunction(Tramp_CEffectRegeneration_ApplyEffect, DETOUR_CEffectRegeneration::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffRemoveProjectileMod)
        DetourMemberFunction(Tramp_CEffectRemoveProjectile_ApplyEffect, DETOUR_CEffectRemoveProjectile::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffRepeatingEffFix) {
        DetourMemberFunction(Tramp_CEffectRepeatingEff_Construct_5, DETOUR_CEffectRepeatingEff::DETOUR_Construct);
        DetourMemberFunction(Tramp_CEffectRepeatingEff_ApplyEffect, DETOUR_CEffectRepeatingEff::DETOUR_ApplyEffect);
    }

    if (pGameOptionsEx->bEffReputationModFix)
        DetourMemberFunction(Tramp_CEffectReputationMod_ApplyEffect, DETOUR_CEffectReputationMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffSaveVsBreathMod)
        DetourMemberFunction(Tramp_CEffectSaveVsBreathMod_ApplyEffect, DETOUR_CEffectSaveVsBreathMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffSaveVsDeathMod)
        DetourMemberFunction(Tramp_CEffectSaveVsDeathMod_ApplyEffect, DETOUR_CEffectSaveVsDeathMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffSaveVsPolyMod)
        DetourMemberFunction(Tramp_CEffectSaveVsPolyMod_ApplyEffect, DETOUR_CEffectSaveVsPolyMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffSaveVsSpellMod)
        DetourMemberFunction(Tramp_CEffectSaveVsSpellMod_ApplyEffect, DETOUR_CEffectSaveVsSpellMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffSaveVsWandsMod)
        DetourMemberFunction(Tramp_CEffectSaveVsWandsMod_ApplyEffect, DETOUR_CEffectSaveVsWandsMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffStealthModFix)
        DetourMemberFunction(Tramp_CEffectStealthMod_ApplyEffect, DETOUR_CEffectStealthMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffStrengthExModFix)
        DetourMemberFunction(Tramp_CEffectStrengthExMod_ApplyEffect, DETOUR_CEffectStrengthExMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffStrengthMod)
        DetourMemberFunction(Tramp_CEffectStrengthMod_ApplyEffect, DETOUR_CEffectStrengthMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffTrackingModFix)
        DetourMemberFunction(Tramp_CEffectTrackingMod_ApplyEffect, DETOUR_CEffectTrackingMod::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffUseEFFFileFix)
        DetourMemberFunction(Tramp_CEffectUseEFFFile_ApplyEffect, DETOUR_CEffectUseEFFFile::DETOUR_ApplyEffect);

    if (pGameOptionsEx->bEffWisdomModFix)
        DetourMemberFunction(Tramp_CEffectWisdomMod_ApplyEffect, DETOUR_CEffectWisdomMod::DETOUR_ApplyEffect);

    //Engine
    if (pGameOptionsEx->bEngineCloneCreatureFix)
        DetourMemberFunction(Tramp_CCreatureObject_ActionJumpToAreaEntranceMove, DETOUR_CCreatureObject::DETOUR_ActionJumpToAreaEntranceMove);

    if (pGameOptionsEx->bEngineDisableInvPauseSP)
        DetourMemberFunction(Tramp_CScreenInventory_Init, DETOUR_CScreenInventory::DETOUR_Init);

    if (pGameOptionsEx->bEngineDisableEndPlayer1Dead)
        DetourMemberFunction(Tramp_CInfGame_SetLoseCutscene, DETOUR_CInfGame::DETOUR_SetLoseCutscene);

    if (pGameOptionsEx->bEngineExpandedStats) {
        DetourMemberFunction(Tramp_CEffectAid_ApplyEffect, DETOUR_CEffectAid::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectBless_ApplyEffect, DETOUR_CEffectBless::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectChant_ApplyEffect, DETOUR_CEffectChant::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectHolyMight_ApplyEffect, DETOUR_CEffectHolyMight::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectChantBad_ApplyEffect, DETOUR_CEffectChantBad::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectDisableSpelltype_ApplyEffect, DETOUR_CEffectDisableSpelltype::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectWingBuffet_ApplyEffect, DETOUR_CEffectWingBuffet::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CEffectAnimationRemoval_ApplyEffect, DETOUR_CEffectAnimationRemoval::DETOUR_ApplyEffect);
        DetourMemberFunction(Tramp_CDerivedStats_Construct_3, DETOUR_CDerivedStats::DETOUR_Construct3);
        DetourMemberFunction(Tramp_CDerivedStats_Construct_0, DETOUR_CDerivedStats::DETOUR_Construct0);
        DetourMemberFunction(Tramp_CDerivedStats_Reload, DETOUR_CDerivedStats::DETOUR_Reload);
        DetourMemberFunction(Tramp_CDerivedStats_OpAssign, DETOUR_CDerivedStats::DETOUR_OpAssign);
        DetourMemberFunction(Tramp_CDerivedStats_ClearStats, DETOUR_CDerivedStats::DETOUR_ClearStats);
        DetourMemberFunction(Tramp_CDerivedStats_GetStat, DETOUR_CDerivedStats::DETOUR_GetStat);
        DetourMemberFunction(Tramp_CDerivedStats_MarshalTemplate, DETOUR_CDerivedStats::DETOUR_MarshalTemplate);
        DetourMemberFunction(Tramp_CDerivedStats_UnmarshalTemplate, DETOUR_CDerivedStats::DETOUR_UnmarshalTemplate);
        DetourMemberFunction(Tramp_CDerivedStats_Deconstruct, DETOUR_CDerivedStats::DETOUR_Deconstruct);
    }

    if (pGameOptionsEx->bEngineExternHPTables)
        DetourMemberFunction(Tramp_CRuleTables_CalculateNewHPSubclass, DETOUR_CRuleTables::DETOUR_CalculateNewHPSubclass);

    if (pGameOptionsEx->bEngineExternClassRaceRestrictions) {
        DetourFunction(Tramp_CAnimation_IsAnimation5000, DETOUR_CAnimation::DETOUR_IsAnimation5000);
        DetourMemberFunction(Tramp_CAnimation5000_Construct, DETOUR_CAnimation5000::DETOUR_Construct);
        DetourMemberFunction(Tramp_CRuleTables_IsMageSchoolAllowed, DETOUR_CRuleTables::DETOUR_IsMageSchoolAllowed);
        DetourMemberFunction(Tramp_CScreenCharGen_ClassPanelOnUpdate, DETOUR_CScreenCharGen::DETOUR_ClassPanelOnUpdate);
        DetourMemberFunction(Tramp_CScreenCharGen_MulticlassPanelOnUpdate, DETOUR_CScreenCharGen::DETOUR_MulticlassPanelOnUpdate);
        DetourMemberFunction(Tramp_CScreenCharGen_MageSchoolPanelOnUpdate, DETOUR_CScreenCharGen::DETOUR_MageSchoolPanelOnUpdate);
    }

    if (pGameOptionsEx->bEngineLauncherProficiencyFix)
        DetourMemberFunction(Tramp_CCreatureObject_GetProficiencyInItem, DETOUR_CCreatureObject::DETOUR_GetProficiencyInItem);

    if (pGameOptionsEx->bEngineProficiencyRestrictions)
        DetourMemberFunction(Tramp_CRuleTables_GetWeapProfMax, DETOUR_CRuleTables::DETOUR_GetWeapProfMax);

    if (pGameOptionsEx->bEngineFakeTalkFix)
        DetourMemberFunction(Tramp_CCreatureObject_UpdateFaceTalkerTimer, DETOUR_CCreatureObject::DETOUR_UpdateFaceTalkerTimer);

    if (pGameOptionsEx->bEngineOptimiseVarCode) {
        DetourMemberFunction(Tramp_CVariableMap_Add, DETOUR_CVariableMap::DETOUR_Add);
        DetourMemberFunction(Tramp_CVariableMap_Find, DETOUR_CVariableMap::DETOUR_Find);
        DetourMemberFunction(Tramp_CVariableMap_GetHash, DETOUR_CVariableMap::DETOUR_GetHash);
    }

    if (pGameOptionsEx->bEngineMoveAreasFix)
        DetourMemberFunction(Tramp_CMoveAreasList_MoveAllTo, DETOUR_CMoveAreasList::DETOUR_MoveAllTo);

    // if (pGameOptionsEx->bEngineSpellTurningFix) moved to bDisableHiddenPatches section
    // DetourMemberFunction(Tramp_CCreatureObject_Construct_10, DETOUR_CCreatureObject::DETOUR_Construct);
    
    //Items
    if (pGameOptionsEx->bItemsUseAnimPercentThrowingWeapons)
        DetourMemberFunction(Tramp_CCreatureObject_ValidateAttackSequence, DETOUR_CCreatureObject::DETOUR_ValidateAttackSequence);

    //Music
    if (pGameOptionsEx->bMusicSonglistExtend) {
        DetourMemberFunction(Tramp_CSoundMixer_InitSonglist, DETOUR_CSoundMixer::DETOUR_InitSonglist);
        DetourMemberFunction(Tramp_CArea_GetSong, DETOUR_CArea::DETOUR_GetSong);
    }

    //Sound
    if (pGameOptionsEx->bSoundAnimAttackSounds)
        DetourMemberFunction(Tramp_CAnimation_PlayCurrentSequenceSound, DETOUR_CAnimation::DETOUR_PlayCurrentSequenceSound);

    if (pGameOptionsEx->bSoundExternWalkSounds) {
        DetourMemberFunction(Tramp_CAnimation_GetWalkingSound, DETOUR_CAnimation::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation1000_GetWalkingSound, DETOUR_CAnimation1000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation1200_GetWalkingSound, DETOUR_CAnimation1200::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation1300_GetWalkingSound, DETOUR_CAnimation1300::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation2000_GetWalkingSound, DETOUR_CAnimation2000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation5000_GetWalkingSound, DETOUR_CAnimation5000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation6400_GetWalkingSound, DETOUR_CAnimation6400::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation7000_GetWalkingSound, DETOUR_CAnimation7000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation7300_GetWalkingSound, DETOUR_CAnimation7300::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation8000_GetWalkingSound, DETOUR_CAnimation8000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimation9000_GetWalkingSound, DETOUR_CAnimation9000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimationA000_GetWalkingSound, DETOUR_CAnimationA000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimationC000_GetWalkingSound, DETOUR_CAnimationC000::DETOUR_GetWalkingSound);
        DetourMemberFunction(Tramp_CAnimationE000_GetWalkingSound, DETOUR_CAnimationE000::DETOUR_GetWalkingSound);
    }

    if (pGameOptionsEx->bSoundSoundsetSubtitles) {
        DetourMemberFunction(Tramp_CScreenRecord_UpdateCharacter, DETOUR_CScreenRecord::DETOUR_UpdateCharacter);
        DetourMemberFunction(Tramp_CScreenCharGen_InitSoundset, DETOUR_CScreenCharGen::DETOUR_InitSoundset);
    }

    //Trigger
    if (pGameOptionsEx->bTriggerExpandedTriggers) {
        DetourMemberFunction(Tramp_CScriptBlock_Evaluate, DETOUR_CScriptBlock::DETOUR_Evaluate);
        //DetourMemberFunction(Tramp_CCreatureObject_EvaluateStatusTrigger, DETOUR_CCreatureObject::DETOUR_EvaluateStatusTrigger);
        DetourMemberFunction(Tramp_CGameAIBase_EvaluateStatusTrigger, DETOUR_CGameAIBase::DETOUR_EvaluateStatusTrigger);
    }

    if (pGameOptionsEx->bTriggerOptimiseBagSearch)
        DetourMemberFunction(Tramp_CInfGame_GetNumOfItemInBag, DETOUR_CInfGame::DETOUR_GetNumOfItemInBag);

    if (pGameOptionsEx->bUserContingencySelectSpell) {
        DetourMemberFunction(Tramp_CUICheckButtonMageBookContChoice_Construct_2CPanel_ChuFileControlInfoBase, DETOUR_CUICheckButtonMageBookContChoice::DETOUR_Construct);
        DetourMemberFunction(Tramp_CUICheckButtonMageBookContChoice_OnLClicked, DETOUR_CUICheckButtonMageBookContChoice::DETOUR_OnLClicked);
        DetourMemberFunction(Tramp_CUICheckButtonMageBookContSelected_Construct_2CPanel_ChuFileControlInfoBase, DETOUR_CUICheckButtonMageBookContSelected::DETOUR_Construct);
        DetourMemberFunction(Tramp_CUICheckButtonMageBookContSelected_OnLClicked, DETOUR_CUICheckButtonMageBookContSelected::DETOUR_OnLClicked);
        DetourMemberFunction(Tramp_CScreenMageBook_ContingencySelectOnLoad, DETOUR_CScreenMageBook::DETOUR_ContingencySelectOnLoad);
        DetourMemberFunction(Tramp_CScreenMageBook_ContingencySelectOnUpdate, DETOUR_CScreenMageBook::DETOUR_ContingencySelectOnUpdate);
        DetourMemberFunction(Tramp_CScreenMageBook_ClearContingencySpell, DETOUR_CScreenMageBook::DETOUR_ClearContingencySpell);
        DetourMemberFunction(Tramp_CScreenMageBook_CreateContingencySpellList, DETOUR_CScreenMageBook::DETOUR_CreateContingencySpellList);
    }

    if (pGameOptionsEx->bUserExternMageSpellHiding)
        DetourMemberFunction(Tramp_CRuleTables_GetMageSpellRefAutoPick, DETOUR_CRuleTables::DETOUR_GetMageSpellRefAutoPick);

    if (pGameOptionsEx->bUserChargenMageSpellScroll) {
        DetourMemberFunction(Tramp_CScreenCharGen_MageBookPanelOnUpdate, DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnUpdate);
        DetourMemberFunction(Tramp_CScreenCharGen_MageBookPanelOnLoad, DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnLoad);
    }

    if (pGameOptionsEx->bUserChargenKitSelectScroll) {
        DetourMemberFunction(Tramp_CScreenCharGen_KitPanelOnUpdate, DETOUR_CScreenCharGen::DETOUR_KitPanelOnUpdate);
        DetourMemberFunction(Tramp_CScreenCharGen_KitPanelOnLoad, DETOUR_CScreenCharGen::DETOUR_KitPanelOnLoad);
        DetourMemberFunction(Tramp_CUICheckButtonChargenKit_GetKitId, DETOUR_CUICheckButtonChargenKit::DETOUR_GetKitId);
        DetourMemberFunction(Tramp_CUICheckButtonChargenKit_GetKitHelpText, DETOUR_CUICheckButtonChargenKit::DETOUR_GetKitHelpText);
    }

    if (pGameOptionsEx->bUserRecordMageSpellScroll) {
        DetourMemberFunction(Tramp_CScreenRecord_MageBookPanelOnUpdate, DETOUR_CScreenRecord::DETOUR_MageBookPanelOnUpdate);
        DetourMemberFunction(Tramp_CScreenRecord_MageBookPanelOnLoad, DETOUR_CScreenRecord::DETOUR_MageBookPanelOnLoad);
    }

    if (pGameOptionsEx->bUserMageBookScroll) {
        DetourMemberFunction(Tramp_CScreenMageBook_SetLevel, DETOUR_CScreenMageBook::DETOUR_SetLevel);
        DetourMemberFunction(Tramp_CCreatureObject_GetKnownSpellMage, DETOUR_CCreatureObject::DETOUR_GetKnownSpellMage);
        DetourMemberFunction(Tramp_CCreatureObject_AddMemSpellMage, DETOUR_CCreatureObject::DETOUR_AddMemSpellMage);
    }

    if (pGameOptionsEx->bUserPriestBookScroll) {
        DetourMemberFunction(Tramp_CScreenPriestBook_SetLevel, DETOUR_CScreenPriestBook::DETOUR_SetLevel);
        DetourMemberFunction(Tramp_CCreatureObject_GetKnownSpellPriest, DETOUR_CCreatureObject::DETOUR_GetKnownSpellPriest);
        DetourMemberFunction(Tramp_CCreatureObject_AddMemSpellPriest, DETOUR_CCreatureObject::DETOUR_AddMemSpellPriest);
    }
   
    if (pGameOptionsEx->bActionAddKitFix ||
        pGameOptionsEx->bTriggerKitFix)  {
        DetourMemberFunction(Tramp_CRuleTables_MapCharacterSpecializationToSchool, DETOUR_CRuleTables::DETOUR_MapCharacterSpecializationToSchool);
    }

    if (pGameOptionsEx->bUI_SequenceWindowButton)
        DetourMemberFunction(Tramp_CUIContingencyButton_OnLClick, DETOUR_CUIContingencyButton::DETOUR_CUIContingencyButton_OnLClick);

    if (pGameOptionsEx->bEff_SelfPatchedEffectRemovingFix) {
        DetourMemberFunction(Tramp_CGameEffectSetColor7_Apply,  DETOUR_CGameEffectSetColor7::DETOUR_CGameEffectSetColor7_Apply);
        DetourMemberFunction(Tramp_CGameEffectSetColor8_Apply,  DETOUR_CGameEffectSetColor8::DETOUR_CGameEffectSetColor8_Apply);
        DetourMemberFunction(Tramp_CGameEffectSetColor9_Apply,  DETOUR_CGameEffectSetColor9::DETOUR_CGameEffectSetColor9_Apply);
        DetourMemberFunction(Tramp_CGameEffectSetColor51_Apply, DETOUR_CGameEffectSetColor51::DETOUR_CGameEffectSetColor51_Apply);
        DetourMemberFunction(Tramp_CGameEffectSetColor52_Apply, DETOUR_CGameEffectSetColor52::DETOUR_CGameEffectSetColor52_Apply);
    }

    if (pGameOptionsEx->bEff_ImprovedInvisibilityIcon) {
        DetourMemberFunction(Tramp_CEffectInvisibility_Apply, DETOUR_CEffectInvisibility::DETOUR_CEffectInvisibility_Apply);
    }

    if (pGameOptionsEx->bEff_BlurAndMirrorImage_Icon) {
        DetourMemberFunction(Tramp_CEffectBlur_Apply, DETOUR_CEffectBlur::DETOUR_CEffectBlur_Apply);
        DetourMemberFunction(Tramp_CEffectMirrorNumberImage_Apply, DETOUR_CEffectMirrorNumberImage::DETOUR_CEffectMirrorNumberImage_Apply);
    }

    if (pGameOptionsEx->bUI_ExtendedRecordScreenText) {
        DetourMemberFunction(Tramp_CScreenRecord_UpdateStyleBonus, DETOUR_CScreenRecord::DETOUR_UpdateStyleBonus);
    }

    if (0) {    // test, please ignore
        //DetourMemberFunction(Tramp_ResBamContainer_SetRes, DETOUR_ResContainer::DETOUR_ResBamContainer_SetRes);
        //DetourMemberFunction(Tramp_FindKey, DETOUR_ResContainer::DETOUR_FindKey);
    }

    if (0) {    // test, please ignore
        //DetourMemberFunction(Tramp_CEffectCharm_ApplyEffect, DETOUR_CEffectCharm::DETOUR_ApplyEffect);
    }

    return;
}