#include "EffectOpcode.h"

#include <math.h>

#include "stdafx.h"
#include "effopcode.h"
#include "effcore.h"
#include "engworld.h"
#include "infcursor.h"
#include "infgame.h"
#include "msgcore.h"
#include "options.h"
#include "console.h"
#include "log.h"
#include "chitin.h"
#include "objcre.h"
#include "MessageExt.h"
#include "ObjectStats.h"
#include "InfGameCommon.h"
#include "objvef.h"

extern uint  gStaticIconsMode;

#define PercentWithLimits(what, value, minrange, maxrange)          \
        int percent = ((int)(what) * (int)(value)) / 100;           \
        AssignWithLimits((what), percent, (minrange), (maxrange));


#define SUM_MACRO(field, minrange, maxrange)                                                        \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                           \
            AddWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange));   \
            bRefreshStats = TRUE;                                                                   \
            bPurge = TRUE;                                                                          \
        } else {                                                                                    \
            creTarget.cdsDiff.##field += effect.nParam1;                                            \
            bPurge = FALSE;                                                                         \
        }


#define INST_SUM_MACRO(field, minrange, maxrange)                                                   \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                           \
            AddWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange));   \
            bRefreshStats = TRUE;                                                                   \
            bPurge = TRUE;                                                                          \
        } else {                                                                                    \
            AddWithLimits(creTarget.cdsCurrent.##field, effect.nParam1, (minrange), (maxrange));    \
            bPurge = FALSE;                                                                         \
        }


#define SUB_MACRO(field, minrange, maxrange)                                                        \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                           \
            SubWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange));   \
            bRefreshStats = TRUE;                                                                   \
            bPurge = TRUE;                                                                          \
        } else {                                                                                    \
            creTarget.cdsDiff.##field -= effect.nParam1;                                            \
            bPurge = FALSE;                                                                         \
        }


#define INST_SUB_MACRO(field, minrange, maxrange)                                                   \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                           \
            SubWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange));   \
            bRefreshStats = TRUE;                                                                   \
            bPurge = TRUE;                                                                          \
        } else {                                                                                    \
            SubWithLimits(creTarget.cdsCurrent.##field, effect.nParam1, (minrange), (maxrange));    \
            bPurge = FALSE;                                                                         \
        }


#define SET_MACRO(field, minrange, maxrange)                                                         \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                            \
            AssignWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange)); \
            bRefreshStats = TRUE;                                                                    \
            bPurge = TRUE;                                                                           \
        } else {                                                                                     \
            AssignWithLimits(creTarget.cdsCurrent.##field,  effect.nParam1, (minrange), (maxrange)); \
            bPurge = FALSE;                                                                          \
        }


#define PERCENT_MACRO(field, minrange, maxrange)                                                      \
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {                                             \
            PercentWithLimits(creTarget.BaseStats.##field, effect.nParam1, (minrange), (maxrange)); \
            bRefreshStats = TRUE;                                                                     \
            bPurge = TRUE;                                                                            \
        } else {                                                                                      \
            PercentWithLimits(creTarget.cdsCurrent.##field, effect.nParam1, (minrange), (maxrange));  \
            bPurge = FALSE;                                                                           \
        }



BOOL (CEffectAttacksPerRoundMod::*Tramp_CEffectAttacksPerRoundMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectAttacksPerRoundMod::*)(CCreatureObject&)>(&CEffectAttacksPerRoundMod::ApplyEffect),  0x5030F6);
BOOL (CEffectCharismaMod::*Tramp_CEffectCharismaMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectCharismaMod::*)(CCreatureObject&)>       (&CEffectCharismaMod::ApplyEffect),         0x5046EC);
BOOL (CEffectConstitutionMod::*Tramp_CEffectConstitutionMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectConstitutionMod::*)(CCreatureObject&)>   (&CEffectConstitutionMod::ApplyEffect),     0x50515F);
BOOL (CEffectDamage::*Tramp_CEffectDamage_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDamage::*)(CCreatureObject&)>            (&CEffectDamage::ApplyEffect),              0x505570);
BOOL (CEffectDexterityMod::*Tramp_CEffectDexterityMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDexterityMod::*)(CCreatureObject&)>      (&CEffectDexterityMod::ApplyEffect),        0x510DCF);
BOOL (CEffectIntelligenceMod::*Tramp_CEffectIntelligenceMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectIntelligenceMod::*)(CCreatureObject&)>   (&CEffectIntelligenceMod::ApplyEffect),     0x5121DB);
BOOL (CEffectLoreMod::*Tramp_CEffectLoreMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectLoreMod::*)(CCreatureObject&)>           (&CEffectLoreMod::ApplyEffect),             0x512733);
BOOL (CEffectLuckMod::*Tramp_CEffectLuckMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectLuckMod::*)(CCreatureObject&)>           (&CEffectLuckMod::ApplyEffect),             0x5129F3);
BOOL (CEffectPoison::*Tramp_CEffectPoison_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectPoison::*)(CCreatureObject&)>            (&CEffectPoison::ApplyEffect),              0x513024);
BOOL (CEffectSaveVsDeathMod::*Tramp_CEffectSaveVsDeathMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectSaveVsDeathMod::*)(CCreatureObject&)>    (&CEffectSaveVsDeathMod::ApplyEffect),      0x514B87);
BOOL (CEffectSaveVsWandsMod::*Tramp_CEffectSaveVsWandsMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectSaveVsWandsMod::*)(CCreatureObject&)>    (&CEffectSaveVsWandsMod::ApplyEffect),      0x514E4C);
BOOL (CEffectSaveVsPolyMod::*Tramp_CEffectSaveVsPolyMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectSaveVsPolyMod::*)(CCreatureObject&)>     (&CEffectSaveVsPolyMod::ApplyEffect),       0x515111);
BOOL (CEffectSaveVsBreathMod::*Tramp_CEffectSaveVsBreathMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectSaveVsBreathMod::*)(CCreatureObject&)>   (&CEffectSaveVsBreathMod::ApplyEffect),     0x5153C7);
BOOL (CEffectSaveVsSpellMod::*Tramp_CEffectSaveVsSpellMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectSaveVsSpellMod::*)(CCreatureObject&)>    (&CEffectSaveVsSpellMod::ApplyEffect),      0x51568C);
BOOL (CEffectMageMemSpellMod::*Tramp_CEffectMageMemSpellMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectMageMemSpellMod::*)(CCreatureObject&)>   (&CEffectMageMemSpellMod::ApplyEffect),     0x5161AB);
BOOL (CEffectStrengthMod::*Tramp_CEffectStrengthMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectStrengthMod::*)(CCreatureObject&)>       (&CEffectStrengthMod::ApplyEffect),         0x516673);
BOOL (CEffectWisdomMod::*Tramp_CEffectWisdomMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectWisdomMod::*)(CCreatureObject&)>         (&CEffectWisdomMod::ApplyEffect),           0x516DF1);
BOOL (CEffectDispel::*Tramp_CEffectDispel_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDispel::*)(CCreatureObject&)>            (&CEffectDispel::ApplyEffect),              0x5184DA);
BOOL (CEffectStealthMod::*Tramp_CEffectStealthMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectStealthMod::*)(CCreatureObject&)>        (&CEffectStealthMod::ApplyEffect),          0x51877E);
BOOL (CEffectPriestMemSpellMod::*Tramp_CEffectPriestMemSpellMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectPriestMemSpellMod::*)(CCreatureObject&)> (&CEffectPriestMemSpellMod::ApplyEffect),   0x518AA3);
BOOL (CEffectBlindness::*Tramp_CEffectBlindness_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectBlindness::*)(CCreatureObject&)>         (&CEffectBlindness::ApplyEffect),           0x51ADF9);
BOOL (CEffectDisease::*Tramp_CEffectDisease_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDisease::*)(CCreatureObject&)>           (&CEffectDisease::ApplyEffect),             0x51B1FB);
BOOL (CEffectOpenLocksMod::*Tramp_CEffectOpenLocksMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectOpenLocksMod::*)(CCreatureObject&)>      (&CEffectOpenLocksMod::ApplyEffect),        0x51D1B8);
BOOL (CEffectFindTrapsMod::*Tramp_CEffectFindTrapsMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectFindTrapsMod::*)(CCreatureObject&)>      (&CEffectFindTrapsMod::ApplyEffect),        0x51D420);
BOOL (CEffectPickPocketsMod::*Tramp_CEffectPickPocketsMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectPickPocketsMod::*)(CCreatureObject&)>    (&CEffectPickPocketsMod::ApplyEffect),      0x51D688);
BOOL (CEffectFatigueMod::*Tramp_CEffectFatigueMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectFatigueMod::*)(CCreatureObject&)>        (&CEffectFatigueMod::ApplyEffect),          0x51D8F0);
BOOL (CEffectIntoxicationMod::*Tramp_CEffectIntoxicationMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectIntoxicationMod::*)(CCreatureObject&)>   (&CEffectIntoxicationMod::ApplyEffect),     0x51DBB0);
BOOL (CEffectTrackingMod::*Tramp_CEffectTrackingMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectTrackingMod::*)(CCreatureObject&)>       (&CEffectTrackingMod::ApplyEffect),         0x51DE70);
BOOL (CEffectLevelMod::*Tramp_CEffectLevelMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectLevelMod::*)(CCreatureObject&)>          (&CEffectLevelMod::ApplyEffect),            0x51E36C);
BOOL (CEffectStrengthExMod::*Tramp_CEffectStrengthExMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectStrengthExMod::*)(CCreatureObject&)>     (&CEffectStrengthExMod::ApplyEffect),       0x51E0AC);
BOOL (CEffectRegeneration::*Tramp_CEffectRegeneration_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectRegeneration::*)(CCreatureObject&)>      (&CEffectRegeneration::ApplyEffect),        0x51C615);
BOOL (CEffectMoraleBreakMod::*Tramp_CEffectMoraleBreakMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectMoraleBreakMod::*)(CCreatureObject&)>    (&CEffectMoraleBreakMod::ApplyEffect),      0x51F080);
BOOL (CEffectReputationMod::*Tramp_CEffectReputationMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectReputationMod::*)(CCreatureObject&)>     (&CEffectReputationMod::ApplyEffect),       0x51F4C1);
BOOL (CEffectAid::*Tramp_CEffectAid_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectAid::*)(CCreatureObject&)>               (&CEffectAid::ApplyEffect),                 0x52720A);
BOOL (CEffectBless::*Tramp_CEffectBless_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectBless::*)(CCreatureObject&)>             (&CEffectBless::ApplyEffect),               0x5273F7);
BOOL (CEffectChant::*Tramp_CEffectChant_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectChant::*)(CCreatureObject&)>             (&CEffectChant::ApplyEffect),               0x527499);
BOOL (CEffectHolyMight::*Tramp_CEffectHolyMight_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectHolyMight::*)(CCreatureObject&)>         (&CEffectHolyMight::ApplyEffect),           0x527563);
BOOL (CEffectChantBad::*Tramp_CEffectChantBad_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectChantBad::*)(CCreatureObject&)>          (&CEffectChantBad::ApplyEffect),            0x5274FE);
BOOL (CEffectDisableSpelltype::*Tramp_CEffectDisableSpelltype_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDisableSpelltype::*)(CCreatureObject&)>  (&CEffectDisableSpelltype::ApplyEffect),    0x52BA62);
BOOL (CEffectDisableButton::*Tramp_CEffectDisableButton_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDisableButton::*)(CCreatureObject&)>     (&CEffectDisableButton::ApplyEffect),       0x52B97B);
BOOL (CEffectLearnSpell::*Tramp_CEffectLearnSpell_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectLearnSpell::*)(CCreatureObject&)>        (&CEffectLearnSpell::ApplyEffect),          0x52C250);
BOOL (CEffectMagicResistMod::*Tramp_CEffectMagicResistMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectMagicResistMod::*)(CCreatureObject&)>    (&CEffectMagicResistMod::ApplyEffect),      0x52EB97);
BOOL (CEffectPoisonResistMod::*Tramp_CEffectPoisonResistMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectPoisonResistMod::*)(CCreatureObject&)>   (&CEffectPoisonResistMod::ApplyEffect),     0x52F0A5);
BOOL (CEffectUseEFFFile::*Tramp_CEffectUseEFFFile_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectUseEFFFile::*)(CCreatureObject&)>        (&CEffectUseEFFFile::ApplyEffect),          0x52FBAE);
BOOL (CEffectCastSpellOnCondition::*Tramp_CEffectCastSpellOnCondition_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectCastSpellOnCondition::*)(CCreatureObject&)>
                                                                            (&CEffectCastSpellOnCondition::ApplyEffect),0x53AFB7);
BOOL (CEffectProficiencyMod::*Tramp_CEffectProficiencyMod_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectProficiencyMod::*)(CCreatureObject&)>    (&CEffectProficiencyMod::ApplyEffect),      0x53B776);
CEffectRepeatingEff& (CEffectRepeatingEff::*Tramp_CEffectRepeatingEff_Construct_5)(ITEM_EFFECT&, POINT&, Enum, int, int) =
    SetFP(static_cast<CEffectRepeatingEff& (CEffectRepeatingEff::*)(ITEM_EFFECT&, POINT&, Enum, int, int)>
                                                                            (&CEffectRepeatingEff::Construct),          0x561AA0);
BOOL (CEffectRepeatingEff::*Tramp_CEffectRepeatingEff_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectRepeatingEff::*)(CCreatureObject&)>      (&CEffectRepeatingEff::ApplyEffect),        0x541C3B);
BOOL (CEffectWingBuffet::*Tramp_CEffectWingBuffet_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectWingBuffet::*)(CCreatureObject&)>        (&CEffectWingBuffet::ApplyEffect),          0x53C791);
BOOL (CEffectDisintegrate::*Tramp_CEffectDisintegrate_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectDisintegrate::*)(CCreatureObject&)>      (&CEffectDisintegrate::ApplyEffect),        0x53F01D);
BOOL (CEffectRemoveProjectile::*Tramp_CEffectRemoveProjectile_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectRemoveProjectile::*)(CCreatureObject&)>  (&CEffectRemoveProjectile::ApplyEffect),    0x53F5D8);
BOOL (CEffectEnableButton::*Tramp_CEffectEnableButton_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectEnableButton::*)(CCreatureObject&)>      (&CEffectEnableButton::ApplyEffect),        0x52B9AA);
BOOL (CEffectCutScene2::*Tramp_CEffectCutScene2_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectCutScene2::*)(CCreatureObject&)>         (&CEffectCutScene2::ApplyEffect),           0x542815);
BOOL (CEffectAnimationRemoval::*Tramp_CEffectAnimationRemoval_ApplyEffect)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffectAnimationRemoval::*)(CCreatureObject&)>  (&CEffectAnimationRemoval::ApplyEffect),    0x549C0C);
//BOOL (CEffectCharm::*Tramp_CEffectCharm_ApplyEffect)(CCreatureObject&) =
//	SetFP(static_cast<BOOL (CEffectCharm::*)(CCreatureObject&)>	(&CEffectCharm::ApplyEffect),	0x50391D);



BOOL DETOUR_CEffectAttacksPerRoundMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectAttacksPerRoundMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //add
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            float fNumAttacks = CDerivedStats_NumAttacksShortToFloat(creTarget.BaseStats.numAttacks) + CDerivedStats_NumAttacksShortToFloat((short)(effect.nParam1));
            AssignWithLimits(creTarget.BaseStats.numAttacks, (unsigned char)CDerivedStats_NumAttacksFloatToShort(fNumAttacks), 0, 10);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else {
            float fNumAttacks = CDerivedStats_NumAttacksShortToFloat(creTarget.cdsDiff.numAttacks) + CDerivedStats_NumAttacksShortToFloat((short)(effect.nParam1));
            creTarget.cdsDiff.numAttacks = CDerivedStats_NumAttacksFloatToShort(fNumAttacks);
            bPurge = FALSE;
        }
        break;
    case 1: //set
        SET_MACRO(numAttacks, 0, 10);
        break;
    case 2: //percent
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            float fNumAttacks = (float) ((CDerivedStats_NumAttacksShortToFloat(creTarget.BaseStats.numAttacks) * (float)effect.nParam1) / 100.0);
            fNumAttacks = (float) (floor(fNumAttacks * 2.0 + 0.5) / 2.0); //round to nearest 0.5
            AssignWithLimits(creTarget.BaseStats.numAttacks, (unsigned char)CDerivedStats_NumAttacksFloatToShort(fNumAttacks), 1, 10);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else {
            float fNumAttacks = (float) ((CDerivedStats_NumAttacksShortToFloat(creTarget.BaseStats.numAttacks) * (float)effect.nParam1) / 100.0);
            fNumAttacks = (float) (floor(fNumAttacks * 2.0 + 0.5) / 2.0); //round to nearest 0.5
            AssignWithLimits(creTarget.cdsCurrent.numAttacks,  CDerivedStats_NumAttacksFloatToShort(fNumAttacks), 1, 10);

            bPurge = FALSE;
        }
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectAttacksPerRoundMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectCharismaMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectCharismaMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(charisma, 0, 25);
        break;
    case 1: //set
        SET_MACRO(charisma, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(charisma, 0, 25);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectCharisma::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectConstitutionMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectConstitutionMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(constitution, 0, 25);
        break;
    case 1: //set
        SET_MACRO(constitution, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(constitution, 0, 25);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectConstitution::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    return TRUE;
}

BOOL DETOUR_CEffectDamage::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDamage::DETOUR_ApplyEffect");

    if (!pGameOptionsEx->bEffApplyConcCheckDamage &&
        !pGameOptionsEx->bEffDamageFix &&
        !pGameOptionsEx->bEffNoDamageNoSpellInterrupt &&
        !pGameOptionsEx->bEffDamageAwaken) {
        return (this->*Tramp_CEffectDamage_ApplyEffect)(creTarget);
    }

    if (creTarget.bUnmarshalling) return TRUE;

    creTarget.bForceReinitAnimColors = TRUE;
    int nPrevHP = creTarget.BaseStats.currentHP;
    
    CAnimation* pAnimation = creTarget.animation.pAnimation;
    if (pAnimation == NULL) {
        LPCTSTR lpsz = "DETOUR_CEffectDamage::DETOUR_ApplyEffect(): pAnimation == NULL\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        bPurge = TRUE;
        return TRUE;
    }

    if (pAnimation->IsInvulnerable()) {
        bPurge = TRUE;
        return TRUE;
    }

    BOOL bCheckSlayerChange;
    if (g_pChitin->pGame->m_GameSave.nCutSceneStatusOverride <= 0) {
        if (g_pChitin->pScreenWorld->u1230 &&
            g_pChitin->pScreenWorld->u1234 <= 0) {
            bCheckSlayerChange = TRUE;
        } else {
            bCheckSlayerChange = FALSE;
        }
    } else {
        bCheckSlayerChange = TRUE;
    }

    if (bCheckSlayerChange) {
        if (effect.rParentResource == "SPIN823") {  // SLAYER_CHANGE_2
            bPurge = TRUE;
            return TRUE;
        }
    }

    short wOrient;
    if (eSource == ENUM_INVALID_INDEX) {
        wOrient = creTarget.GetOrientationTo(effect.ptSource);
    } else {
        CCreatureObject* pCre;
        char nReturnVal;
        do {
            nReturnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eSource, THREAD_ASYNCH, &pCre, INFINITE);
        } while (nReturnVal == OBJECT_SHARING || nReturnVal == OBJECT_DENYING);

        if (nReturnVal == OBJECT_SUCCESS) {
            wOrient = creTarget.GetOrientationTo(pCre->currentLoc);
            nReturnVal = g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eSource, THREAD_ASYNCH, INFINITE);
        } else {
            wOrient = 0;
        }
    }

    if (creTarget.BaseStats.stateFlags & STATE_STONE_DEATH) {
        CEffectInstantDeath* pEff = new CEffectInstantDeath();
        pEff->effect.ptSource = effect.ptSource;
        pEff->eSource = eSource;
        pEff->enum2 = enum2;
        pEff->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_STONE_EXPLODE;
        creTarget.AddEffect(*pEff, 1, TRUE, TRUE);
        bPurge = TRUE;
        return TRUE;
    }

    if (creTarget.BaseStats.stateFlags & STATE_FROZEN_DEATH) {
        CEffectInstantDeath* pEff = new CEffectInstantDeath();
        pEff->effect.ptSource = effect.ptSource;
        pEff->eSource = eSource;
        pEff->enum2 = enum2;
        pEff->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_FROZEN_EXPLODE;
        creTarget.AddEffect(*pEff, 1, TRUE, TRUE);
        bPurge = TRUE;
        return TRUE;
    }

    if (creTarget.BaseStats.currentHP <= -10 ||
        creTarget.BaseStats.stateFlags & STATE_DEAD) {
        bPurge = TRUE;
        return TRUE;
    }

    int nDamageAmount = 0;
    for (unsigned int i = 0; i < effect.nDice; i++) {
        nDamageAmount += DieRoll(effect.nDieSides, -creTarget.cdsPrevious.luck) + 1;
    }
    effect.nParam1 += nDamageAmount;
    
    BOOL bBaseDamageDone = (BOOL)effect.nParam1;
    short wAnimSeq = creTarget.wAnimSequenceSimplified;

    unsigned int nDamageType = effect.nParam2 & 0xFFFF0000;
    int nDamageBehavior = effect.nParam2 & 0xFFFF;

    effect.nParam6 = effect.nParam1;    // orig damage amount

    if (nDamageBehavior == EFFECTDAMAGE_BEHAVIOR_NORMAL) {
        if (pGameOptionsEx->bEngineExpandedStats) {
            switch (nDamageType) {
            case DAMAGETYPE_ACID:
            case DAMAGETYPE_COLD:
            case DAMAGETYPE_CRUSHING:
            case DAMAGETYPE_STUNNING:
            case DAMAGETYPE_PIERCING:
            case DAMAGETYPE_SLASHING:
            case DAMAGETYPE_ELECTRICITY:
            case DAMAGETYPE_FIRE:
            case DAMAGETYPE_POISON:
            case DAMAGETYPE_MAGIC:
            case DAMAGETYPE_MISSILE:
            case DAMAGETYPE_MAGICFIRE:
            case DAMAGETYPE_MAGICCOLD:
                effect.nParam1 += effect.nParam1 * effect.nParam3 / 100;
                break;
            default:
                break;
            }
        }

        effect.nParam5 = 0; // reducing percentage

#define SET_PARAM5(x) \
    if (creTarget.cdsPrevious.##x > 0) { \
                effect.nParam5 = creTarget.cdsPrevious.##x; \
    }

        switch (nDamageType) {
        case DAMAGETYPE_ACID:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistAcid / 100;
            SET_PARAM5(resistAcid)
            break;
        case DAMAGETYPE_COLD:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistCold / 100;
            SET_PARAM5(resistCold)
            break;
        case DAMAGETYPE_CRUSHING:
        case DAMAGETYPE_STUNNING:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistCrushing / 100;
            SET_PARAM5(resistCrushing)
            break;
        case DAMAGETYPE_PIERCING:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistPiercing / 100;
            SET_PARAM5(resistPiercing)
            break;
        case DAMAGETYPE_SLASHING:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistSlashing / 100;
            SET_PARAM5(resistSlashing)
            break;
        case DAMAGETYPE_ELECTRICITY:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistElectricity / 100;
            SET_PARAM5(resistElectricity)
            break;
        case DAMAGETYPE_FIRE:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistFire / 100;
            SET_PARAM5(resistFire)
            break;
        case DAMAGETYPE_POISON:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistPoison / 100;
            SET_PARAM5(resistPoison)
            break;
        case DAMAGETYPE_MAGIC:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistMagicDamage / 100;
            SET_PARAM5(resistMagicDamage)
            break;
        case DAMAGETYPE_MISSILE:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistMissile / 100;
            SET_PARAM5(resistMissile)
            break;
        case DAMAGETYPE_MAGICFIRE:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistMagicFire / 100;
            SET_PARAM5(resistMagicFire)
            break;
        case DAMAGETYPE_MAGICCOLD:
            effect.nParam1 -= effect.nParam1 * creTarget.cdsPrevious.resistMagicCold / 100;
            SET_PARAM5(resistMagicCold)
            break;
        default:
            break;
        }

        if (g_pChitin->pGame->GetPartyMemberSlot(creTarget.id) != -1) {
            if (g_pChitin->cNetwork.bSessionOpen) {
                effect.nParam1 += effect.nParam1 * g_pChitin->pGame->m_GameOptions.m_nMPDifficultyMultiplier / 100;
            } else {
                effect.nParam1 += effect.nParam1 * g_pChitin->pGame->m_GameOptions.m_nDifficultyMultiplier / 100;
            }
            
            if (effect.nParam1 > creTarget.cdsPrevious.maxHP &&
                creTarget.cdsPrevious.maxHP < 14) {
                effect.nParam1 = creTarget.cdsPrevious.maxHP - 1;
            }
        }

    }

    BOOL bPreviousSpellInterruptState = creTarget.bInterruptSpellcasting;
    creTarget.bInterruptSpellcasting = TRUE;

    if (pGameOptionsEx->bEffApplyConcCheckDamage) {
        int nBaseDifficulty = 0;
        int nLevelMod = 0;

        if ((creTarget.bInCasting || creTarget.bStartedCasting) &&
            (creTarget.currentAction.opcode == ACTION_SPELL ||
             creTarget.currentAction.opcode == ACTION_SPELL_NO_DEC ||
             creTarget.currentAction.opcode == ACTION_SPELL_POINT ||
             creTarget.currentAction.opcode == ACTION_SPELL_POINT_NO_DEC ||
             creTarget.currentAction.opcode == ACTION_FORCESPELL ||
             creTarget.currentAction.opcode == ACTION_FORCESPELL_POINT)) {
            IECString sSpell;
            if (creTarget.currentAction.GetSName1().IsEmpty()) {
                creTarget.GetSpellIdsName(creTarget.currentAction.m_specificID, &sSpell);
            } else {
                sSpell = creTarget.currentAction.GetSName1();
            }
    
            ResSplContainer resSpell;
            resSpell.SetResRef(ResRef(sSpell), TRUE, TRUE);
            resSpell.Demand();
            if (resSpell.pRes) {
                nLevelMod = resSpell.GetSpellLevel();
            }
            resSpell.Release();
        }

        int nDifficulty = nBaseDifficulty + nLevelMod + effect.nParam1;
        int nRoll = DieRoll(20, creTarget.cdsPrevious.luck);
        if (nRoll >= nDifficulty) { //if pass
            creTarget.bInterruptSpellcasting = bPreviousSpellInterruptState;
        }
    }

    if (pGameOptionsEx->bEffNoDamageNoSpellInterrupt) {
        if (nDamageBehavior == EFFECTDAMAGE_BEHAVIOR_NORMAL &&
            effect.nParam1 <= 0) {
            creTarget.bInterruptSpellcasting = bPreviousSpellInterruptState;
        }
    }

    if (creTarget.ePuppet != ENUM_INVALID_INDEX) {
        Enum ePuppet = creTarget.ePuppet;
        CCreatureObject* pCrePuppet;
        char nReturnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectDeny(ePuppet, THREAD_ASYNCH, &pCrePuppet, INFINITE);
        if (nReturnVal == OBJECT_SUCCESS) {
            creTarget.ePuppet = ENUM_INVALID_INDEX;
            pCrePuppet->bRemoveFromArea = TRUE;
            nReturnVal = g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectDeny(ePuppet, THREAD_ASYNCH, INFINITE);
        }
        creTarget.bForceRefresh = TRUE;
    }

    if (nDamageType == DAMAGETYPE_POISON) {
        if (creTarget.wPoisonTimer <= 0) {
            creTarget.wPoisonTimer = 100;
            CMessagePlaySoundset* pMsg = IENew CMessagePlaySoundset();
            pMsg->eTarget = creTarget.id;
            pMsg->eSource = creTarget.id;
            pMsg->nSoundIdx = SOUNDSET_DAMAGE;
            pMsg->bPrintToConsole = TRUE;
            pMsg->bLimitLength = TRUE;
            g_pChitin->messages.Send(*pMsg, FALSE);
        }
    } else {
        if (bBaseDamageDone) {
            CMessagePlaySoundset* pMsg = IENew CMessagePlaySoundset();
            pMsg->eTarget = creTarget.id;
            pMsg->eSource = creTarget.id;
            pMsg->nSoundIdx = SOUNDSET_DAMAGE;
            pMsg->bPrintToConsole = TRUE;
            pMsg->bLimitLength = TRUE;
            g_pChitin->messages.Send(*pMsg, FALSE);
        }
    }

    switch (nDamageBehavior) {
    case EFFECTDAMAGE_BEHAVIOR_NORMAL:
        if (bBaseDamageDone) DisplayDamageAmount(creTarget);
        if (effect.nParam1 > 0) {
            Trigger tTookDamage(TRIGGER_TOOK_DAMAGE, effect.nParam1);
            CMessageSetTrigger* pMsg = IENew CMessageSetTrigger();
            pMsg->eTarget = creTarget.id;
            pMsg->eSource = creTarget.id;
            pMsg->t = tTookDamage;
            g_pChitin->messages.Send(*pMsg, FALSE);
        }
        creTarget.BaseStats.currentHP -= effect.nParam1;
        break;
    case EFFECTDAMAGE_BEHAVIOR_SETVALUE:
        creTarget.BaseStats.currentHP = effect.nParam1 < creTarget.cdsPrevious.maxHP ? effect.nParam1 : creTarget.cdsPrevious.maxHP;
        break;
    case EFFECTDAMAGE_BEHAVIOR_SETPERCENT:
        creTarget.BaseStats.currentHP = creTarget.BaseStats.currentHP * effect.nParam1 / 100;
        break;
    case EFFECTDAMAGE_BEHAVIOR_LOSEPERCENT:
        creTarget.BaseStats.currentHP -= creTarget.cdsPrevious.maxHP * effect.nParam1 / 100;
        break;
    default:
        break;
    }

    creTarget.bShowDamageArrow = TRUE;
    creTarget.wBloodPortraitFlashTimer = 128;
    creTarget.wDamageLocatorArrowTimer = 128;

    if (g_pChitin->pCursor->nCurrentCursorIdx == CURSOR_TOOLTIP_SCROLL &&
        creTarget.pArea != NULL &&
        creTarget.pArea->iPicked == creTarget.id &&
        g_pChitin->pGame->GetPartyMemberSlot(creTarget.id) != -1) {
        creTarget.UpdateHPStatusTooltip(*g_pChitin->pCursor->pCtrlTarget);
    }

    PlayHitSound(nDamageType, creTarget);

    switch (nDamageType) {
    case DAMAGETYPE_PIERCING:
    case DAMAGETYPE_CRUSHING:
    case DAMAGETYPE_SLASHING:
    case DAMAGETYPE_MISSILE:
        if (nPrevHP - creTarget.BaseStats.currentHP < 6) {
            creTarget.CreateGore(0, wOrient, PARTICLETYPE_BLOOD_SMALL);
        } else {
            creTarget.CreateGore(0, wOrient, PARTICLETYPE_BLOOD_MEDIUM);
        }
        break;
    case DAMAGETYPE_COLD:
        PlaySound(ResRef("HIT_06"), creTarget);
        creTarget.StartSpriteEffect(GORETYPE_COLD, 0, (nPrevHP - creTarget.BaseStats.currentHP) * 3);
        break;
    case DAMAGETYPE_MAGICCOLD: //omitted in original code
        if (pGameOptionsEx->bEffDamageFix) {
            PlaySound(ResRef("HIT_06"), creTarget);
            creTarget.StartSpriteEffect(GORETYPE_COLD, 0, (nPrevHP - creTarget.BaseStats.currentHP) * 3);
        }
        break;
    case DAMAGETYPE_FIRE:
    case DAMAGETYPE_MAGICFIRE:
        PlaySound(ResRef("FIRE"), creTarget);
        creTarget.StartSpriteEffect(GORETYPE_FIRE, 0, (nPrevHP - creTarget.BaseStats.currentHP) * 3);
        break;
    case DAMAGETYPE_ELECTRICITY:
        PlaySound(ResRef("HIT_05"), creTarget);
        creTarget.StartSpriteEffect(GORETYPE_SHOCK, 0, (nPrevHP - creTarget.BaseStats.currentHP) * 3);
        break;
    case DAMAGETYPE_MAGIC:
        PlaySound(ResRef("HIT_07"), creTarget);
        break;
    default:
        break;
    }

    if (wAnimSeq != SEQ_DAMAGE &&
        wAnimSeq != SEQ_DIE &&
        wAnimSeq != SEQ_TWITCH &&
        wAnimSeq != SEQ_SLEEP) {
        if (pGameOptionsEx->bEffNoDamageNoSpellInterrupt &&
            nDamageBehavior == EFFECTDAMAGE_BEHAVIOR_NORMAL &&
            effect.nParam1 <= 0 &&
            (wAnimSeq == SEQ_CONJURE || wAnimSeq == SEQ_CAST)) {
        } else {
            creTarget.SetAnimationSequence(SEQ_DAMAGE);
        }
    }

    if (creTarget.cdsPrevious.minHitPoints > 0 &&
        creTarget.BaseStats.currentHP <= 0) {
        creTarget.BaseStats.currentHP = (short) creTarget.cdsPrevious.minHitPoints;
    }

    bool bStunned = false; //added to prevent Awaken on Damage to reverse the unconsciousness effect of fist below

    if (creTarget.BaseStats.currentHP <= 0) {
        if (g_pChitin->pGame->GetPartyMemberSlot(creTarget.id) != -1 &&
            creTarget.BaseStats.currentHP > -20) {
            creTarget.BaseStats.currentHP = 0;
        }
        CEffectInstantDeath* pEff = new CEffectInstantDeath();
        pEff->eSource = eSource;
        pEff->enum2 = enum2;

        switch (nDamageType) {
        case DAMAGETYPE_STUNNING:
            {
            bStunned = true;

            delete pEff;
            pEff = NULL;
            ITEM_EFFECT* pIF = new ITEM_EFFECT;
            CreateItemEffect(*pIF, CEFFECT_OPCODE_UNCONSCIOUSNESS);
            pIF->timing = 0;
            pIF->duration = (1 - creTarget.BaseStats.currentHP) * 15;
            POINT ptSrc;
            ptSrc.x = -1;
            ptSrc.y = -1;
            POINT ptDest;
            ptDest.x = -1;
            ptDest.y = -1;

            pEff = (CEffectInstantDeath*)&CreateEffect(*pIF, ptSrc, ENUM_INVALID_INDEX, ptDest, ENUM_INVALID_INDEX);
            pEff->effect.ptSource = creTarget.currentLoc;
            pEff->eSource = creTarget.id;
            pEff->enum2 = ENUM_INVALID_INDEX;
            pEff->effect.ptDest = effect.ptDest;

            if (pGameOptionsEx->bEffDamageFix) {
                delete pIF;
            }

            }
            
            creTarget.BaseStats.currentHP = 1;

            break;
        case DAMAGETYPE_ACID:
            pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -5 ? EFFECTINSTANTDEATH_TYPE_ACID : EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        case DAMAGETYPE_COLD:
        case DAMAGETYPE_MAGICCOLD:
            pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -5 ? EFFECTINSTANTDEATH_TYPE_FROZEN : EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        case DAMAGETYPE_CRUSHING:
        case DAMAGETYPE_PIERCING:
        case DAMAGETYPE_SLASHING:
            pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -8 ? EFFECTINSTANTDEATH_TYPE_CHUNKED : EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        case DAMAGETYPE_ELECTRICITY:
            pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -5 ? EFFECTINSTANTDEATH_TYPE_ELECTRIC : EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        case DAMAGETYPE_FIRE:
            pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -5 ? EFFECTINSTANTDEATH_TYPE_BURNING : EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        case DAMAGETYPE_MAGICFIRE: //omitted in original code
            if (pGameOptionsEx->bEffDamageFix) {
                pEff->effect.nParam2 = creTarget.BaseStats.currentHP < -5 ? EFFECTINSTANTDEATH_TYPE_BURNING : EFFECTINSTANTDEATH_TYPE_NORMAL;
            } else {
                pEff->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_NORMAL;
            }
            break;
        case DAMAGETYPE_POISON:
        case DAMAGETYPE_MAGIC:
        case DAMAGETYPE_MISSILE:
            pEff->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        default:
            LPCTSTR lpsz = "DETOUR_CEffectDamage::DETOUR_ApplyEffect(): WEIRD damage\r\n";
            L.timestamp();
            L.append(lpsz);
            console.write(lpsz);
            pEff->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_NORMAL;
            break;
        }

        creTarget.AddEffect(*pEff, 1, TRUE, TRUE);
    }

    if (pGameOptionsEx->bEffDamageAwaken && !bStunned) {
        creTarget.BaseStats.stateFlags &= ~STATE_SLEEPING;
        creTarget.cdsCurrent.stateFlags &= ~STATE_SLEEPING;
        creTarget.EffectsEquipped.RemoveEffect(creTarget, CEFFECT_OPCODE_UNCONSCIOUSNESS, creTarget.EffectsEquipped.posItrPrev, -1, ResRef(), FALSE);
        creTarget.EffectsMain.RemoveEffect(creTarget, CEFFECT_OPCODE_UNCONSCIOUSNESS, creTarget.EffectsMain.posItrPrev, -1, ResRef(), FALSE);
    }

    int n = creTarget.cdsPrevious.maxHP < 1 ? 1 : creTarget.cdsPrevious.maxHP;
    if (nPrevHP * 100 / n > *g_pHPHurtThresholdPercent &&
        creTarget.BaseStats.currentHP * 100 / n < *g_pHPHurtThresholdPercent &&
        creTarget.BaseStats.currentHP * 100 / n > 0) { //went from above threshold to below threshold
        CMessagePlaySoundset* pMsg = IENew CMessagePlaySoundset();
        pMsg->eTarget = creTarget.id;
        pMsg->eSource = creTarget.id;
        pMsg->nSoundIdx = SOUNDSET_HURT;
        pMsg->bPrintToConsole = TRUE;
        pMsg->bLimitLength = FALSE;
        g_pChitin->messages.Send(*pMsg, FALSE);

        creTarget.SetAutoPauseInfo(8);
    } else if (creTarget.BaseStats.currentHP * 100 / n < *g_pHPHurtThresholdPercent &&
               creTarget.BaseStats.currentHP * 100 / n > 0 &&
               IERand(10) == 0) { //was below threshold and still below threshold, with 1/16 chance
        CMessagePlaySoundset* pMsg = IENew CMessagePlaySoundset();
        pMsg->eTarget = creTarget.id;
        pMsg->eSource = creTarget.id;
        pMsg->nSoundIdx = SOUNDSET_HURT;
        pMsg->bPrintToConsole = TRUE;
        pMsg->bLimitLength = FALSE;
        g_pChitin->messages.Send(*pMsg, FALSE);

        creTarget.SetAutoPauseInfo(8);
    }

    bPurge = TRUE;
    return TRUE;
};

BOOL DETOUR_CEffectDexterityMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDexterityMod::DETOUR_ApplyEffect");

    //case 0, 1, 2, default as original
    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(dexterity, 0, 25);
        break;
    case 1: //set
        SET_MACRO(dexterity, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(dexterity, 0, 25);
        break;
    case 3: //IWD1/P&P Cat's Grace spell
        if (!pRuleEx->m_ClassSpellAbility.m_2da.bLoaded) {
            LPCTSTR lpsz = "DETOUR_CEffectDexterityMod::DETOUR_ApplyEffect(): CLSSPLAB.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
            bPurge = TRUE;
            break;
        }
        {
            short wParam1Low = effect.nParam1 & 0xFFFF;
            short wParam1High = effect.nParam1 >> 16;

            if (wParam1High == 0) {
                int nDieSides;

                if (wParam1Low > 0) {
                    nDieSides = wParam1Low;
                } else {
                    IECString sClass = g_pChitin->pGame->GetClassString(creTarget.oBase.GetClass(), KIT_TRUECLASS);
                    IECString& sDieSides = pRuleEx->m_ClassSpellAbility.GetValue(IECString("DEX"), sClass);
                    sscanf_s((LPCTSTR)sDieSides, "%d", &nDieSides);
                    if (nDieSides == 0) nDieSides = 6;
                }

                wParam1High = DieRoll(nDieSides, 0) + 1;
                effect.nParam1 = wParam1Low | (wParam1High << 16);
            }

            if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
                AddWithLimits(creTarget.BaseStats.dexterity, wParam1High, 0, 20);
                bRefreshStats = TRUE;
                bPurge = TRUE;
            } else {
                AddWithLimits(creTarget.cdsCurrent.dexterity, wParam1High, 0, 20);
                bPurge = FALSE;
            }
        }
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectDexterityMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectIntelligenceMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectIntelligenceMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(intelligence, 0, 25);
        break;
    case 1: //set
        SET_MACRO(intelligence, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(intelligence, 0, 25);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectIntelligence::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectLoreMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectLoreMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(lore, 0, 100);
        break;
    case 1: //set
        SET_MACRO(lore, 0, 100);
        break;
    case 2: //percentage
        PERCENT_MACRO(lore, 0, 100);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectLore::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectLuckMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectLuckMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(luck, -20, 20);
        break;
    case 1: //set
        SET_MACRO(luck, -20, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(luck, -20, 20);
        break;
    case 3: //instantaneous sum
        INST_SUM_MACRO(luck, -20, 20);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectLuckMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectPoison::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectPoison::DETOUR_ApplyEffect");

    CRepeatingPoison* pRepeat = IENew CRepeatingPoison();
    pRepeat->eSource = eSource;

    short wParam2Low = effect.nParam2 & 0xFFFF;
    short wParam2High = effect.nParam2 >> 16;
    creTarget.cdsCurrent.stateFlags |= STATE_POISONED;

    int nTicksBegan = effect.nParam4;

    switch (wParam2Low) {
    case 0:
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = effect.nParam1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 1:
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = creTarget.BaseStats.currentHP * effect.nParam1 / 100;;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 2:
        pRepeat->wMode = 2;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 3:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    case 4:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam3;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectPoison::DETOUR_ApplyEffect(): invalid effect.nParam2 low word (%d)\r\n";
        console.writef(lpsz, wParam2Low);
        L.timestamp();
        L.appendf(lpsz, wParam2Low);
        bPurge = TRUE;
        break;
    }

    creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);

    return TRUE;
}


// orig BG2 and Tobex_v26 check SaveVSxxx range -20..20, but BaseStats.SavingThrowxxx stats cannot be < 0

BOOL DETOUR_CEffectSaveVsBreathMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectSaveVsBreathMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUB_MACRO(saveBreath, 0, 20);
        break;
    case 1: //set
        SET_MACRO(saveBreath, 0, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(saveBreath, 0, 20);
        break;
    case 3: //instantaneous sum
        INST_SUB_MACRO(saveBreath, 0, 20);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectSaveVsBreathMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectSaveVsDeathMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectSaveVsDeathMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUB_MACRO(saveDeath, 0, 20);
        break;
    case 1: //set
        SET_MACRO(saveDeath, 0, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(saveDeath, 0, 20);
        break;
    case 3: //instantaneous sum
        INST_SUB_MACRO(saveDeath, 0, 20);
        break;    
    default:
        LPCTSTR lpsz = "DETOUR_CEffectSaveVsDeathMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectSaveVsPolyMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectSaveVsPolyMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUB_MACRO(savePoly, 0, 20);
        break;
    case 1: //set
        SET_MACRO(savePoly, 0, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(savePoly, 0, 20);
        break;
    case 3: //instantaneous sum
        INST_SUB_MACRO(savePoly, 0, 20);
        break;    
    default:
        LPCTSTR lpsz = "DETOUR_CEffectSaveVsPolyMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectSaveVsSpellMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectSaveVsSpellMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUB_MACRO(saveSpell, 0, 20);
        break;
    case 1: //set
        SET_MACRO(saveSpell, 0, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(saveSpell, 0, 20);
        break;
    case 3: //instantaneous sum
        INST_SUB_MACRO(saveSpell, 0, 20);
        break;    
    default:
        LPCTSTR lpsz = "DETOUR_CEffectSaveVsSpellMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectSaveVsWandsMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectSaveVsWandsMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUB_MACRO(saveWands, 0, 20);
        break;
    case 1: //set
        SET_MACRO(saveWands, 0, 20);
        break;
    case 2: //percentage
        PERCENT_MACRO(saveWands, 0, 20);
        break;
    case 3: //instantaneous sum
        INST_SUB_MACRO(saveWands, 0, 20);
        break;    
    default:
        LPCTSTR lpsz = "DETOUR_CEffectSaveVsWandsMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectMageMemSpellMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectMageMemSpellMod::DETOUR_ApplyEffect");

    int bit = 1;

    if (effect.nParam2 & 0x200 &&
        effect.nParam1 >= 0 &&
        effect.nParam1 < 9) {
        CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoMage[effect.nParam1];
        splLvl.wMaxMemSpells *= 2;
        splLvl.wCurrMemSpells *= 2;
    } else if (effect.nParam2 == 0) { //remainder is the original function
        for (int i = 0; i < effect.nParam1 && i < 9; i++) {
            CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoMage[i];
            splLvl.wMaxMemSpells *= 2;
            splLvl.wCurrMemSpells *= 2;
        }
    } else {
        for (int i = 0; i < 9; i++) {
            CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoMage[i];
            if (effect.nParam2 & bit &&
                splLvl.wMaxMemSpells > 0) {
                splLvl.wMaxMemSpells += (short)effect.nParam1;
                splLvl.wCurrMemSpells += (short)effect.nParam1;
            }
            bit = bit << 1;
        }
    }

    return TRUE;
};

BOOL DETOUR_CEffectStrengthMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectStrengthMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(strength, 0, 25);
        break;
    case 1: //set
        SET_MACRO(strength, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(strength, 0, 25);
        break;
    case 3: //IWD1/P&P Strength Spell
        if (!pRuleEx->m_ClassSpellAbility.m_2da.bLoaded) {
            LPCTSTR lpsz = "DETOUR_CEffectStrengthMod::DETOUR_ApplyEffect(): CLSSPLAB.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
            bPurge = TRUE;
            break;
        }
        {
            short wParam1Low = effect.nParam1 & 0xFFFF;
            short wParam1High = effect.nParam1 >> 16;
            int nMaxStrengthEx;

            if (wParam1High == 0) {
                int nDieSides;

                if (wParam1Low > 0) {
                    nDieSides = effect.nParam1;
                } else {
                    IECString sClass = g_pChitin->pGame->GetClassString(creTarget.oBase.GetClass(), KIT_TRUECLASS);
                    IECString& sDieSides = pRuleEx->m_ClassSpellAbility.GetValue(IECString("STR"), sClass);
                    sscanf_s((LPCTSTR)sDieSides, "%d", &nDieSides);
                    if (nDieSides == 0) nDieSides = 6;
                }

                wParam1High = DieRoll(nDieSides, 0) + 1;
                effect.nParam1 = wParam1Low | (wParam1High << 16);
            }

            if (wParam1Low > 0) {
                nMaxStrengthEx = 100;
            } else {
                IECString sClass = g_pChitin->pGame->GetClassString(creTarget.oBase.GetClass(), KIT_TRUECLASS);
                IECString& sMaxStrEx = pRuleEx->m_ClassSpellAbility.GetValue(IECString("STREX"), sClass);
                sscanf_s((LPCTSTR)sMaxStrEx, "%d", &nMaxStrengthEx);
            }

            if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
                char strengthEffective = CDerivedStats_GetEffectiveStrengthSpell(creTarget.BaseStats.strength, creTarget.BaseStats.strengthEx) + wParam1High;
                char strength = 0;
                char strengthEx = 0;
                CDerivedStats_GetRealStrengthSpell(strengthEffective, strength, strengthEx);
                AssignWithLimits(creTarget.BaseStats.strength, strength, 0, 18);
                AssignWithLimits(creTarget.BaseStats.strengthEx, strengthEx, 0, (nMaxStrengthEx ? 100 : 0));
                bRefreshStats = TRUE;
                bPurge = TRUE;
            } else {
                char strengthEffective = CDerivedStats_GetEffectiveStrengthSpell((char&)creTarget.cdsCurrent.strength, (char&)creTarget.cdsCurrent.strengthEx) + wParam1High;
                char strength = 0;
                char strengthEx = 0;
                CDerivedStats_GetRealStrengthSpell(strengthEffective, strength, strengthEx);
                AssignWithLimits(creTarget.cdsCurrent.strength, strength, 0, 18);
                AssignWithLimits(creTarget.cdsCurrent.strengthEx, strengthEx, 0, (nMaxStrengthEx ? 100 : 0));
                bPurge = FALSE;
            }
        }
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectStrengthMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }

    return TRUE;
}

BOOL DETOUR_CEffectWisdomMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectWisdomMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(wisdom, 0, 25);
        break;
    case 1: //set
        SET_MACRO(wisdom, 0, 25);
        break;
    case 2: //percentage
        PERCENT_MACRO(wisdom, 0, 25);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectWisdom::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectDispel::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDispel::DETOUR_ApplyEffect");

    int nParam2Low = effect.nParam2 & 0xFFFF;
    int nParam2High = effect.nParam2 >> 16;
    char nRand = IERand(100);

    switch (nParam2Low) {
    case 0:
        creTarget.EffectsMain.TryDispel(creTarget, creTarget.EffectsMain.posItrPrev, TRUE, FALSE, 0, 0);
        creTarget.EffectsEquipped.TryDispel(creTarget, creTarget.EffectsEquipped.posItrPrev, TRUE, FALSE, 0, 0);
        break;
    case 1:
        creTarget.EffectsMain.TryDispel(creTarget, creTarget.EffectsMain.posItrPrev, TRUE, TRUE, nRand, effect.nSourceCreCasterLevel);
        creTarget.EffectsEquipped.TryDispel(creTarget, creTarget.EffectsEquipped.posItrPrev, TRUE, TRUE, nRand, effect.nSourceCreCasterLevel);
        break;
    default:
        creTarget.EffectsMain.TryDispel(creTarget, creTarget.EffectsMain.posItrPrev, TRUE, TRUE, nRand, effect.nParam1);
        creTarget.EffectsEquipped.TryDispel(creTarget, creTarget.EffectsEquipped.posItrPrev, TRUE, TRUE, nRand, effect.nParam1);
        break;
    }

    bool bDispelMagicalItem = false;

    if (creTarget.Inventory.items[SLOT_MISC19]) {
        if (!(creTarget.Inventory.items[SLOT_MISC19]->GetFlags() & ITEMFLAG_NOT_DISPELLABLE)) {
            switch (nParam2High) {
            case 0:
                bDispelMagicalItem = true;
                break;
            case 1:
                bDispelMagicalItem = false;
                break;
            case 2:
            default:
                CDerivedStats& cds = creTarget.GetDerivedStats();
                int nItemLevel = max(
                    cds.GetEffectiveMageLevel(creTarget.o.GetClass()),
                    cds.GetEffectiveClericLevel(creTarget.o.GetClass())
                );
                
                int nDispelLevel;
                switch (nParam2Low) {
                case 0:
                case 1:
                default:
                    nDispelLevel = effect.nSourceCreCasterLevel;
                    break;
                case 2:
                    nDispelLevel = effect.nParam1;
                    break;
                }

                int nDifficulty = 50 + (nItemLevel - nDispelLevel) * (nItemLevel > nDispelLevel ? 10 : 5);
                if (nRand == 99 ||
                    nRand > nDifficulty) {
                    bDispelMagicalItem = true;
                } else {
                    bDispelMagicalItem = false;
                }
                break;
            }
        }
    }
    
    if (bDispelMagicalItem) {
        creTarget.UnequipAll(TRUE);
        CCreatureObject::RemoveItem(creTarget, SLOT_MISC19);
        creTarget.EquipAll(TRUE);
    }

    creTarget.bForceReinitAnimColors = TRUE;
    creTarget.bForceResetAnimColors = TRUE;
    creTarget.bForceMsgResetAnimColors = TRUE;

    bRefreshStats = TRUE;
    bPurge = TRUE;

    return TRUE;
};

BOOL DETOUR_CEffectStealthMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectStealthMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            AddWithLimits(creTarget.BaseStats.stealth, effect.nParam1, 0, 255);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else if (effect.nTiming == TIMING_VERY_PERMANENT) {
            creTarget.cdsDiff.moveSilently9 += effect.nParam1;
            bPurge = FALSE;
        } else {
            creTarget.cdsDiff.stealth += effect.nParam1;
            bPurge = FALSE;
        }
        break;
    case 1: //set
        SET_MACRO(stealth, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(stealth, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectStealthMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectPriestMemSpellMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectPriestMemSpellMod::DETOUR_ApplyEffect");

    int bit = 1;

    if (effect.nParam2 & 0x200 &&
        effect.nParam1 >= 0 &&
        effect.nParam1 < 7) {
        CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoPriest[effect.nParam1];
        splLvl.wMaxMemSpells *= 2;
        splLvl.wCurrMemSpells *= 2;
    } else if (effect.nParam2 == 0) { //remainder is the original function
        for (int i = 0; i < effect.nParam1 && i < 7; i++) {
            CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoPriest[i];
            splLvl.wMaxMemSpells *= 2;
            splLvl.wCurrMemSpells *= 2;
        }
    } else {
        for (int i = 0; i < 7; i++) {
            CreFileMemorizedSpellLevel& splLvl = creTarget.cdsCurrent.MemInfoPriest[i];
            if (effect.nParam2 & bit &&
                splLvl.wMaxMemSpells > 0) {
                splLvl.wMaxMemSpells += (short)effect.nParam1;
                splLvl.wCurrMemSpells += (short)effect.nParam1;
            }
            bit = bit << 1;
        }
    }

    return TRUE;
};

BOOL DETOUR_CEffectBlindness::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectBlindness::DETOUR_ApplyEffect");

    //Change to spell description - overwrites Blindness Fix
    if (pGameOptionsEx->bEffBlindnessAsSpellDesc) {
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) { //permanent
            if (!(creTarget.BaseStats.stateFlags & STATE_BLIND)) {
                creTarget.BaseStats.THAC0 -= (-4);
                creTarget.BaseStats.ArmorClassBase = min(20, creTarget.BaseStats.ArmorClassBase - (-4));
            }
            creTarget.BaseStats.stateFlags |= STATE_BLIND;
            if (!(creTarget.cdsCurrent.stateFlags & STATE_BLIND)) {
                creTarget.cdsCurrent.THAC0 -= (-4);
            }
            creTarget.cdsCurrent.stateFlags |= STATE_BLIND;
        
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else {
            //limited
            if (!(creTarget.cdsCurrent.stateFlags & STATE_BLIND)) {
                creTarget.cdsCurrent.THAC0 -= (-4);
                creTarget.cdsDiff.ArmorClassBase -= (-4);
            }
            creTarget.cdsCurrent.stateFlags |= STATE_BLIND;

            bPurge = FALSE;
        }

        return TRUE;
    }

    //Remove cumulative penalty
    if (pGameOptionsEx->bEffBlindnessFix) {
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) { //permanent
            if (!(creTarget.BaseStats.stateFlags & STATE_BLIND)) creTarget.BaseStats.THAC0 -= (-10);
            creTarget.BaseStats.stateFlags |= STATE_BLIND;
            if (!(creTarget.cdsCurrent.stateFlags & STATE_BLIND)) creTarget.cdsCurrent.THAC0 -= (-10);
            creTarget.cdsCurrent.stateFlags |= STATE_BLIND;
        
            bPurge = TRUE;
        } else {
            //limited
            if (!(creTarget.cdsCurrent.stateFlags & STATE_BLIND)) creTarget.cdsCurrent.THAC0 -= (-10);
            creTarget.cdsCurrent.stateFlags |= STATE_BLIND;
        }

        return TRUE;
    }

    return TRUE;
}

BOOL DETOUR_CEffectDisease::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDisease::DETOUR_ApplyEffect");

    CRepeatingDisease* pRepeat = NULL;

    short wParam2Low = effect.nParam2 & 0xFFFF;
    short wParam2High = effect.nParam2 >> 16;
    int nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
    int nTicksBegan = effect.nParam4;

    switch (wParam2Low) {
    case 0:
        pRepeat = IENew CRepeatingDisease();
        pRepeat->eSource = eSource;
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = effect.nParam1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);
        break;
    case 1:
        pRepeat = IENew CRepeatingDisease();
        pRepeat->eSource = eSource;
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = creTarget.BaseStats.currentHP * effect.nParam1 / 100;;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);
        break;
    case 2:
        pRepeat = IENew CRepeatingDisease();
        pRepeat->eSource = eSource;
        pRepeat->wMode = 2;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);
        break;
    case 3:
        pRepeat = IENew CRepeatingDisease();
        pRepeat->eSource = eSource;
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);
        break;
    case 4:
        creTarget.cdsDiff.strength -= effect.nParam1;
        break;
    case 5:
        creTarget.cdsDiff.dexterity -= effect.nParam1;
        break;
    case 6:
        creTarget.cdsDiff.constitution -= effect.nParam1;
        break;
    case 7:
        creTarget.cdsDiff.intelligence -= effect.nParam1;
        break;
    case 8:
        creTarget.cdsDiff.wisdom -= effect.nParam1;
        break;
    case 9:
        creTarget.cdsDiff.charisma -= effect.nParam1;
        break;
    case 10:
        creTarget.cdsCurrent.stateFlags |= STATE_SLOWED;
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectDisease::DETOUR_ApplyEffect(): invalid effect.nParam2 low word (%d)\r\n";
        console.writef(lpsz, wParam2Low);
        L.timestamp();
        L.appendf(lpsz, wParam2Low);
        bPurge = TRUE;
        break;
    }
    return TRUE;
}

BOOL DETOUR_CEffectOpenLocksMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectOpenLocksMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            AddWithLimits(creTarget.BaseStats.lockpicking, effect.nParam1, 0, 255);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else if (effect.nTiming == TIMING_VERY_PERMANENT) {
            creTarget.cdsDiff.openLocks9 += effect.nParam1;
            bPurge = FALSE;
        } else {
            creTarget.cdsDiff.lockpicking += effect.nParam1;
            bPurge = FALSE;
        }
        break;
    case 1: //set
        SET_MACRO(lockpicking, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(lockpicking, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectOpenLocksMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectFindTrapsMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectFindTrapsMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            AddWithLimits(creTarget.BaseStats.findTraps, effect.nParam1, 0, 255);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else if (effect.nTiming == TIMING_VERY_PERMANENT) {
            creTarget.cdsDiff.findTraps9 += effect.nParam1;
            bPurge = FALSE;
        } else {
            creTarget.cdsDiff.findTraps += effect.nParam1;
            bPurge = FALSE;
        }
        break;
    case 1: //set
        SET_MACRO(findTraps, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(findTraps, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectFindTrapsMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectPickPocketsMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectPickPocketsMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        if (effect.nTiming == TIMING_INSTANT_PERMANENT) {
            AddWithLimits(creTarget.BaseStats.pickPockets, effect.nParam1, 0, 255);
            bRefreshStats = TRUE;
            bPurge = TRUE;
        } else if (effect.nTiming == TIMING_VERY_PERMANENT) {
            creTarget.cdsDiff.pickPockets9 += effect.nParam1;
            bPurge = FALSE;
        } else {
            creTarget.cdsDiff.pickPockets += effect.nParam1;
            bPurge = FALSE;
        }
        break;
    case 1: //set
        SET_MACRO(pickPockets, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(pickPockets, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectPickPocketsMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectFatigueMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectFatigueMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(fatigue, 0, 100);
        break;
    case 1: //set
        SET_MACRO(fatigue, 0, 100);
        break;
    case 2: //percentage
        PERCENT_MACRO(fatigue, 0, 100);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectFatigue::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectIntoxicationMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectIntoxicationMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(intoxication, 0, 100);
        break;
    case 1: //set
        SET_MACRO(intoxication, 0, 100);
        break;
    case 2: //percentage
        PERCENT_MACRO(intoxication, 0, 100);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectIntoxication::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectTrackingMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectTrackingMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(tracking, 0, 255);
        break;
    case 1: //set
        SET_MACRO(tracking, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(tracking, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectTrackingMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectLevelMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectLevelMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(levelPrimary, 0, 100);
        break;
    case 1: //set
        SET_MACRO(levelPrimary, 0, 100);
        break;
    case 2: //percentage
        PERCENT_MACRO(levelPrimary, 0, 100);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectLevel::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectStrengthExMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectStrengthExMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(strengthEx, 0, 100);
        break;
    case 1: //set
        SET_MACRO(strengthEx, 0, 100);
        break;
    case 2: //percentage
        PERCENT_MACRO(strengthEx, 0, 100);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectStrengthEx::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectRegeneration::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectRegeneration::DETOUR_ApplyEffect");

    CRepeatingRegen* pRepeat = IENew CRepeatingRegen();

    short wParam2Low = effect.nParam2 & 0xFFFF;
    short wParam2High = effect.nParam2 >> 16;

    int nTicksBegan = effect.nParam4;

    switch (wParam2Low) {
    case 0:
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = effect.nParam1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 1:
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = creTarget.BaseStats.currentHP * effect.nParam1 / 100;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 2:
        pRepeat->wMode = 2;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 3:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    case 4:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam3;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectRegeneration::DETOUR_ApplyEffect(): invalid effect.nParam2 low word (%d)\r\n";
        console.writef(lpsz, wParam2Low);
        L.timestamp();
        L.appendf(lpsz, wParam2Low);
        bPurge = TRUE;
        break;
    }

    creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);

    return TRUE;
}

BOOL DETOUR_CEffectMoraleBreakMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectMoraleBreakMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(moraleBreak, 0, 255);
        break;
    case 1: //set
        SET_MACRO(moraleBreak, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(moraleBreak, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectMoraleBreakMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectReputationMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectReputationMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
    case 0: //sum
        SUM_MACRO(reputation, 0, 255);
        break;
    case 1: //set
        SET_MACRO(reputation, 0, 255);
        break;
    case 2: //percentage
        PERCENT_MACRO(reputation, 0, 255);
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectReputationMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
        console.writef(lpsz, effect.nParam2);
        L.timestamp();
        L.appendf(lpsz, effect.nParam2);
        bPurge = TRUE;
        break;
    }
    
    return TRUE;
}

BOOL DETOUR_CEffectAid::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectAid::DETOUR_ApplyEffect");
    if (creTarget.BaseStats.currentHP <= 0) return TRUE;
    if (!(creTarget.cdsCurrent.stateFlags & STATE_AID))
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_AID, effect.nParam1);
    return (this->*Tramp_CEffectAid_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectBless::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectBless::DETOUR_ApplyEffect");
    if (!(creTarget.cdsCurrent.stateFlags & STATE_BLESS))
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_BLESS, effect.nParam1);
    return (this->*Tramp_CEffectBless_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectChant::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectChant::DETOUR_ApplyEffect");
    if (!(creTarget.cdsCurrent.stateFlags & STATE_CHANT))
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CHANT, effect.nParam1);
    return (this->*Tramp_CEffectChant_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectHolyMight::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectHolyMight::DETOUR_ApplyEffect");
    if (!(creTarget.cdsCurrent.stateFlags & STATE_DRAWUPONHOLYMIGHT))
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_DRAWUPONHOLYMIGHT, effect.nParam1);
    return (this->*Tramp_CEffectHolyMight_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectChantBad::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectChantBad::DETOUR_ApplyEffect");
    if (!(creTarget.cdsCurrent.stateFlags & STATE_CHANTBAD))
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CHANTBAD, effect.nParam1);
    return (this->*Tramp_CEffectChantBad_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectDisableSpelltype::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDisableSpelltype::DETOUR_ApplyEffect");
    if ((unsigned int)effect.nParam2 < 3)
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_DISABLESPELLTYPEWIZARD + effect.nParam2, 1);
    return (this->*Tramp_CEffectDisableSpelltype_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectDisableButton::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDisableButton::DETOUR_ApplyEffect");

    if ((unsigned int)effect.nParam2 < 14)
        creTarget.cdsCurrent.ButtonDisable[effect.nParam2] = 1;
    if ((unsigned int)effect.nParam2 < 15)
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_BUTTONDISABLESTEALTH + effect.nParam2, 1);

    return TRUE;
}

BOOL DETOUR_CEffectLearnSpell::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectLearnSpell::DETOUR_ApplyEffect");

    int wParam1High = effect.nParam1 >> 16;
    unsigned int dwKit;

    IECString sLearnSpellMod;
    int nLearnSpellMod;
    int nRow = creTarget.cdsPrevious.intelligence;
    int nCol = 0; //LEARN_SPELL
    if (nCol < g_pChitin->pGame->INTMOD.nCols &&
        nRow < g_pChitin->pGame->INTMOD.nRows &&
        nCol >= 0 &&
        nRow >= 0) {
        sLearnSpellMod = *((g_pChitin->pGame->INTMOD.pDataArray) + (g_pChitin->pGame->INTMOD.nCols * nRow + nCol));
    } else {
        sLearnSpellMod = g_pChitin->pGame->INTMOD.defaultVal;
    }
    sscanf_s((LPCTSTR)sLearnSpellMod, "%d", &nLearnSpellMod);

    int nRand = IERand(100);

    if (g_pChitin->cNetwork.bSessionOpen &&
        g_pChitin->pGame->m_GameOptions.m_nMPDifficultyMultiplier < 0) {
        nRand = 1;
    } else
    if (g_pChitin->pGame->m_GameOptions.m_nDifficultyMultiplier < 0) {
        nRand = 1;
    }

    ResSplContainer resSpell;
    resSpell.SetResRef(effect.rResource, TRUE, TRUE);
    resSpell.Demand();

    if (!resSpell.pRes) {
        resSpell.Release();
        bPurge = TRUE;
        return TRUE;
    }

    short wLevel = resSpell.GetSpellLevel();
    short wType = resSpell.GetSpellType();
    int nSchoolExclusionFlags = resSpell.GetExclusionFlags() & 0x3FC0; //keep only spell school bits
    int nKnownSpellsOfLevel = 0;

    while (creTarget.GetKnownSpellMage(wLevel - 1, nKnownSpellsOfLevel) != NULL) {
        nKnownSpellsOfLevel++;
    }

    if (wParam1High & EFFECTLEARNSPELL_RESTRICT_MAX_SPELLS &&
        nKnownSpellsOfLevel > g_pChitin->pGame->GetIntModMaxSpellsPerLevel(creTarget.cdsCurrent)) {
        resSpell.Release();
        bPurge = TRUE;
        return TRUE;
    }

    if (wParam1High & EFFECTLEARNSPELL_RESTRICT_SCHOOL &&
        wType == SPELLTYPE_MAGE &&
        creTarget.GetKitUnusableFlag() & nSchoolExclusionFlags) {
        resSpell.Release();
        bPurge = TRUE;
        return TRUE;
    }

    if (wParam1High & EFFECTLEARNSPELL_NO_SORCERER &&
        creTarget.GetCurrentObject().GetClass() == CLASS_SORCERER) {
        resSpell.Release();
        bPurge = TRUE;
        return TRUE;
    }

    BOOL bAlreadyKnown = FALSE;
    POSITION pos;
    if (wParam1High & EFFECTLEARNSPELL_NO_XP_DUPLICATE) {
        switch (wType) {
        case SPELLTYPE_MAGE:
            pos = creTarget.KnownSpellsWizard[wLevel - 1].GetHeadPosition();
            while (pos != NULL && bAlreadyKnown == FALSE) {
                CreFileKnownSpell* pKSpell = (CreFileKnownSpell*)creTarget.KnownSpellsWizard[wLevel - 1].GetNext(pos);
                if (pKSpell->name == resSpell.name) {
                    bAlreadyKnown = TRUE;
                }
            }
            break;
        case SPELLTYPE_PRIEST:
            pos = creTarget.KnownSpellsPriest[wLevel - 1].GetHeadPosition();
            while (pos != NULL && bAlreadyKnown == FALSE) {
                CreFileKnownSpell* pKSpell = (CreFileKnownSpell*)creTarget.KnownSpellsPriest[wLevel - 1].GetNext(pos);
                if (pKSpell->name == resSpell.name) {
                    bAlreadyKnown = TRUE;
                }
            }
            break;
        default:
            break;
        }
    }

    dwKit = creTarget.BaseStats.kitLow | (creTarget.BaseStats.kitHigh << 16);
    if (dwKit != KIT_TRUECLASS &&
        dwKit != KIT_WILDMAGE   ) {
        if (g_pChitin->pGame->MapCharacterSpecializationToSchool(creTarget.BaseStats.dwReversedkit) == resSpell.GetSpellSchoolPrimary()) {
            nLearnSpellMod += 15;
        } else {
            nLearnSpellMod -= 15;
        }
    }

    if ((nRand <= nLearnSpellMod) ||
        wParam1High & EFFECTLEARNSPELL_SUCCESS_ALWAYS) { //success
        switch (wType) {
        case SPELLTYPE_MAGE:
            creTarget.AddKnownSpellMage(effect.rResource, wLevel - 1);
            break;
        case SPELLTYPE_PRIEST:
            creTarget.AddKnownSpellPriest(effect.rResource, wLevel - 1);
            break;
        default:
            creTarget.AddKnownSpell(effect.rResource, FALSE);
            break;
        }

        if (!bAlreadyKnown &&
            !(wParam1High & EFFECTLEARNSPELL_NO_XP_ALWAYS) &&
            creTarget.GetCurrentObject().EnemyAlly <= EA_CONTROLLEDCUTOFF) {
            IECString sXPBonus;
            int nXPBonus;
            nRow = 2; //LEARN_SPELL
            nCol = wLevel - 1;

            if (nCol < g_pChitin->pGame->XPBONUS.nCols &&
                nRow < g_pChitin->pGame->XPBONUS.nRows &&
                nCol >= 0 &&
                nRow >= 0) {
                sXPBonus = *((g_pChitin->pGame->XPBONUS.pDataArray) + (g_pChitin->pGame->XPBONUS.nCols * nRow + nCol));
            } else {
                sXPBonus = g_pChitin->pGame->XPBONUS.defaultVal;
            }
            sscanf_s((LPCTSTR)sXPBonus, "%d", &nXPBonus);

            g_pChitin->pGame->AddExperienceParty(nXPBonus);
        }
    }

    resSpell.Release();
    bPurge = TRUE;
    return TRUE;
}

BOOL DETOUR_CEffectMagicResistMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectMagicResistMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
        case 0: //instantaneous sum
            INST_SUM_MACRO(resistMagic, 0, 100);
            break;
        case 1: //set
            SET_MACRO(resistMagic, 0, 100);
            break;
        case 2: //percentage
            PERCENT_MACRO(resistMagic, 0, 100);
            break;
        case 3: //sum
            SUM_MACRO(resistMagic, 0, 100);
            break;
        default:
            LPCTSTR lpsz = "DETOUR_CEffectMagicResistMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
            console.writef(lpsz, effect.nParam2);
            L.timestamp();
            L.appendf(lpsz, effect.nParam2);
            bPurge = TRUE;
            break;
    }
    return TRUE;
}

BOOL DETOUR_CEffectPoisonResistMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectPoisonResistMod::DETOUR_ApplyEffect");

    switch (effect.nParam2) {
        case 0:
        case 78: //TEMPORARY ONLY workaround for BG2Fixpack immunity batch for disease
            //set
            creTarget.cdsCurrent.resistPoison = effect.nParam1;
            break;
        case 1:
            //sum
            creTarget.cdsDiff.resistPoison += effect.nParam1;
            break;
        case 2:
            //percentage
            creTarget.cdsCurrent.resistPoison = (creTarget.cdsCurrent.resistPoison * effect.nParam1) / 100;
            break;
        case 3:
            //instantaneous sum
            creTarget.cdsCurrent.resistPoison += effect.nParam1;
            break;
        default:
            LPCTSTR lpsz = "DETOUR_CEffectPoisonResistMod::DETOUR_ApplyEffect(): invalid effect.nParam2 (%d)\r\n";
            console.writef(lpsz, effect.nParam2);
            L.timestamp();
            L.appendf(lpsz, effect.nParam2);
            bPurge = TRUE;
            break;
    }
    return TRUE;
}

BOOL DETOUR_CEffectUseEFFFile::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectUseEFFFile::DETOUR_ApplyEffect");

    BOOL bValidTarget = FALSE;
    Object oTarget = creTarget.GetCurrentObject();
    Object oCriteria;
    unsigned char cValue = effect.nParam1 & 0xFF;
    int nIdsType = effect.nParam2;

    switch (nIdsType) {
    case 2: //EA
        oCriteria.EnemyAlly = cValue;
        if (oTarget.MatchCriteria(oCriteria, FALSE, FALSE, FALSE)) bValidTarget = TRUE;
        break;
    case 3: //GENERAL
        if (oTarget.General == cValue || cValue == 0) bValidTarget = TRUE;
        break;
    case 4: //RACE
        if (oTarget.Race == cValue || cValue == 0) bValidTarget = TRUE;
        break;
    case 5: //CLASS
        {
            unsigned char cClassOrg;
            unsigned char cClassNew;
            unsigned char cClass = oTarget.GetClass();
            
            oTarget.GetDualClasses(&cClassNew, &cClassOrg);
            
            if (cClassNew != cClassOrg &&
                !oTarget.HasActiveSubclass(cClassOrg, TRUE)
            ) { //dual-class
                cClass = cClassNew;
            }

            if (cClass == cValue || cValue == 0) bValidTarget = TRUE;
        }
        break;
    case 6: //SPECIFIC
        if (oTarget.Specific == cValue || cValue == 0) bValidTarget = TRUE;
        break;
    case 7: //GENDER
        if (oTarget.Gender == cValue || cValue == 0) bValidTarget = TRUE;
        break;
    case 8: //ALIGN
        if (oTarget.Alignment == cValue || cValue == ALIGN_ANY) bValidTarget = TRUE;
        if (cValue == ALIGN_MASK_GOOD && (oTarget.Alignment & 0xF) == ALIGN_MASK_GOOD) bValidTarget = TRUE;
        if (cValue == ALIGN_MASK_NEUTRAL && (oTarget.Alignment & 0xF) == ALIGN_MASK_NEUTRAL) bValidTarget = TRUE;
        if (cValue == ALIGN_MASK_EVIL && (oTarget.Alignment & 0xF) == ALIGN_MASK_EVIL) bValidTarget = TRUE;
        if (cValue == ALIGN_LAWFUL_MASK && (oTarget.Alignment & 0xF0) == ALIGN_LAWFUL_MASK) bValidTarget = TRUE;
        if (cValue == ALIGN_NEUTRAL_MASK && (oTarget.Alignment & 0xF0) == ALIGN_NEUTRAL_MASK) bValidTarget = TRUE;
        if (cValue == ALIGN_CHAOTIC_MASK && (oTarget.Alignment & 0xF0) == ALIGN_CHAOTIC_MASK) bValidTarget = TRUE;
        break;
    default:
        break;
    }

    if (bValidTarget) {
        ResEffContainer rec(effect.rResource);
        CEffect* pEff = &rec.CreateCEffect();
        if (pEff) {
            pEff->effect.ptSource = effect.ptSource;
            pEff->enum2 = enum2;
            pEff->eSource = eSource;
            pEff->effect.ptDest = effect.ptDest;
            pEff->effect.nDuration = effect.nDuration;
            pEff->effect.nTiming = effect.nTiming & 0xFFFF;
            pEff->effect.bFirstCall = effect.bFirstCall;

            //new - restore child effect's parameters and resources from previous application
            if (effect.nParam3) { //bSuccess
                if (pEff->effect.nOpcode == CEFFECT_OPCODE_USE_EFF_FILE) {
                    pEff->effect.pad[0] = effect.pad[0];
                    pEff->effect.pad[1] = effect.pad[1];
                    pEff->effect.pad[2] = effect.pad[2];
                    pEff->effect.pad[3] = effect.pad[3];
                } else {
                    pEff->effect.nParam1 = effect.pad[0];
                    pEff->effect.nParam2 = effect.pad[1];
                    pEff->effect.nParam3 = effect.pad[2];
                    pEff->effect.nParam4 = effect.pad[3];
                }
            }

            if (effect.nParam3 || //bSuccess
                pEff->TryApplyEffect(
                    creTarget,
                    &creTarget.cRollSaveDeath,
                    &creTarget.cRollSaveWand,
                    &creTarget.cRollSavePolymorph,
                    &creTarget.cRollSaveBreath,
                    &creTarget.cRollSaveSpell,
                    &creTarget.cRollResistMagic,
                    &creTarget.cRollEffectProbability
                )
            ) {
                effect.nParam3 = TRUE;
                pEff->ApplyEffect(creTarget);
                if (pEff->bPurge) bPurge = TRUE;
                effect.bFirstCall = pEff->effect.bFirstCall;

                //new - store child effect's parameters and resources
                if (pEff->effect.nOpcode == CEFFECT_OPCODE_USE_EFF_FILE) {
                    effect.pad[0] = pEff->effect.pad[0];
                    effect.pad[1] = pEff->effect.pad[1];
                    effect.pad[2] = pEff->effect.pad[2];
                    effect.pad[3] = pEff->effect.pad[3];
                } else {
                    effect.pad[0] = pEff->effect.nParam1;
                    effect.pad[1] = pEff->effect.nParam2;
                    effect.pad[2] = pEff->effect.nParam3;
                    effect.pad[3] = pEff->effect.nParam4;
                }
            } else bPurge = TRUE;

            delete pEff;
            pEff = NULL;
        }
    } else bPurge = TRUE;

    return TRUE;
}

BOOL DETOUR_CEffectCastSpellOnCondition::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectCastSpellOnCondition::DETOUR_ApplyEffect");

    Trigger t(TRIGGER_NONE, 0);
    Object o;

    CConditionalSpell* pConditionalSpell = new CConditionalSpell();
    pConditionalSpell->dwFlags = 0;
    pConditionalSpell->rResource1 = effect.rResource;
    pConditionalSpell->rResource2 = effect.rResource2;
    pConditionalSpell->rResource3 = effect.rResource3;

    short wParam3High = effect.nParam3 >> 16; //bool game ticks set
    short wParam3Low = effect.nParam3 & 0xFFFF; //set flags, portrait icon, purge after trigger
    unsigned short wParam2High = effect.nParam2 >> 16; //condition check period
    short wParam2Low = effect.nParam2 & 0xFFFF; //condition

    if (wParam3High == 0) {
        effect.nParam4 = g_pChitin->pGame->m_WorldTimer.nGameTime; //time effect first applied
        wParam3High = 1;
        effect.nParam3 = wParam3High << 16 | wParam3Low;
    }

    switch (wParam2Low) {
    case 0:
        t.opcode = TRIGGER_HIT_BY;
        break;
    case 1:
        t.opcode = TRIGGER_SEE;
        t.o.oids.id1 = OBJECT_NEARESTENEMYOF;
        break;
    case 2:
        t.opcode = TRIGGER_HP_PERCENT_LT;
        t.i = 50;
        t.o.oids.id1 = OBJECT_MYSELF;
        break;
    case 3:
        t.opcode = TRIGGER_HP_PERCENT_LT;
        t.i = 25;
        t.o.oids.id1 = OBJECT_MYSELF;
        break;
    case 4:
        t.opcode = TRIGGER_HP_PERCENT_LT;
        t.i = 10;
        t.o.oids.id1 = OBJECT_MYSELF;
        break;
    case 5:
        t.opcode = TRIGGER_STATE_CHECK;
        t.i = STATE_HELPLESS;
        t.o.oids.id1 = OBJECT_MYSELF;
        break;
    case 6:
        t.opcode = TRIGGER_STATE_CHECK;
        t.i = STATE_POISONED;
        t.o.oids.id1 = OBJECT_MYSELF;
        break;
    case 7:
        t.opcode = TRIGGER_ATTACKED_BY;
        break;
    case 8:
        t.opcode = TRIGGER_PERSONAL_SPACE_DISTANCE;
        t.i = 4;
        break;
    case 9:
        t.opcode = TRIGGER_PERSONAL_SPACE_DISTANCE;
        t.i = 10;
        break;
    case 11:
        t.opcode = TRIGGER_TOOK_DAMAGE;
        break;
    case 12:
        t.opcode = TRIGGER_CRITICALHIT;
        break;
    default:
        break;
    }
    pConditionalSpell->t = t;

    switch (effect.nParam1) {
    case 0:
        o.oids.id1 = OBJECT_MYSELF;
        break;
    case 1:
        o.oids.id1 = OBJECT_LASTHITTER;
        break;
    case 2:
        o.oids.id1 = OBJECT_NEARESTENEMYOF;
        break;
    case 3:
        o.oids.id1 = OBJECT_NOTHING;
        break;
    default:
        break;
    }
    pConditionalSpell->oTarget = o;

    if (wParam3Low) {
        pConditionalSpell->dwFlags |= 0x1;
        if (!creTarget.PortraitIcons.Find((void*)0x4B, NULL)) {
            creTarget.PortraitIcons.AddTail((void*)0x4B);
        }
    }

    pConditionalSpell->eff = *this;
    
    STRREF strrefName;
    STRREF strrefDescription;
    g_pChitin->pGame->GetContingencyConditionTexts(&strrefName, &strrefDescription, wParam2Low);
    pConditionalSpell->strrefCondition = strrefName;

    g_pChitin->pGame->GetContingencyTargetTexts(&strrefName, &strrefDescription, (short)effect.nParam1);
    pConditionalSpell->strrefTarget = strrefName;

    creTarget.cdsCurrent.m_ConditionalSpells.AddTail(pConditionalSpell);

    if (effect.nParentResourceType == 1 && //spell
        effect.rParentResource.IsValid()) {
        CSpellProtection* pSpellProt = IENew CSpellProtection();
        pSpellProt->rSpell = effect.rParentResource;
        pSpellProt->strrefMsg = creTarget.GetCurrentObject().EnemyAlly <= EA_CONTROLLEDCUTOFF ? 0x806C : -1;
        creTarget.cdsCurrent.m_SpellProtections.AddTail(pSpellProt);
    }

    return TRUE;
}

BOOL DETOUR_CEffectProficiencyMod::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectProficiencyMod::DETOUR_ApplyEffect");

    int wParam2Low = effect.nParam2 & 0xFFFF;  //proficiency
    int wParam2High = effect.nParam2 >> 16; //behaviour

    /*
    unsigned char nClassId = creTarget.o.GetClass();
    unsigned char nClassNew;
    unsigned char nClassOrg;
    creTarget.o.GetDualClasses(&nClassNew, &nClassOrg);
    BOOL bTwoClasses = creTarget.o.HasActiveSubclass(nClassOrg, TRUE);
    unsigned int dwKit = creTarget.BaseStats.kit[1] | (creTarget.BaseStats.kit[0] << 16);
    int dwProfsMax = 5;
    if (nClassId < 21) g_pChitin->pGame->GetWeapProfMax(nClassId, nClassNew, nClassOrg, bTwoClasses, wParam2Low, dwKit);
    */

    switch (wParam2Low) {
    case STATS_PROFICIENCYBASTARDSWORD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyBastardSword += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyBastardSword = max(creTarget.cdsCurrent.proficiencyBastardSword, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYLONGSWORD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyLongSword += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyLongSword = max(creTarget.cdsCurrent.proficiencyLongSword, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSHORTSWORD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyShortSword += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyShortSword = max(creTarget.cdsCurrent.proficiencyShortSword, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYAXE:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyAxe += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyAxe = max(creTarget.cdsCurrent.proficiencyAxe, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYTWOHANDEDSWORD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyTwoHandedSword += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyTwoHandedSword = max(creTarget.cdsCurrent.proficiencyTwoHandedSword, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYKATANA:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyKatana += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyKatana = max(creTarget.cdsCurrent.proficiencyKatana, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSCIMITARWAKISASHININJATO:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyScimitarWakizashiNinjato += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyScimitarWakizashiNinjato = max(creTarget.cdsCurrent.proficiencyScimitarWakizashiNinjato, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYDAGGER:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyDagger += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyDagger = max(creTarget.cdsCurrent.proficiencyDagger, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYWARHAMMER:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyWarhammer += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyWarhammer = max(creTarget.cdsCurrent.proficiencyWarhammer, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSPEAR:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencySpear += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencySpear = max(creTarget.cdsCurrent.proficiencySpear, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYHALBERD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyHalberd += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyHalberd = max(creTarget.cdsCurrent.proficiencyHalberd, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYFLAILMORNINGSTAR:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyFlailMorningstar += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyFlailMorningstar = max(creTarget.cdsCurrent.proficiencyFlailMorningstar, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYMACE:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyMace += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyMace = max(creTarget.cdsCurrent.proficiencyMace, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYQUARTERSTAFF:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyQuarterstaff += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyQuarterstaff = max(creTarget.cdsCurrent.proficiencyQuarterstaff, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYCROSSBOW:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyCrossbow += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyCrossbow = max(creTarget.cdsCurrent.proficiencyCrossbow, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYLONGBOW:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyLongbow += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyLongbow = max(creTarget.cdsCurrent.proficiencyLongbow, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSHORTBOW:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyShortbow += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyShortbow = max(creTarget.cdsCurrent.proficiencyShortbow, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYDART:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyDart += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyDart = max(creTarget.cdsCurrent.proficiencyDart, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSLING:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencySling += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencySling = max(creTarget.cdsCurrent.proficiencySling, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYBLACKJACK:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyBlackjack += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyBlackjack = max(creTarget.cdsCurrent.proficiencyBlackjack, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYGUN:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyGun += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyGun = max(creTarget.cdsCurrent.proficiencyGun, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYMARTIALARTS:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyMartialArts += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyMartialArts = max(creTarget.cdsCurrent.proficiencyMartialArts, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCY2HANDED:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyTwoHanded += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyTwoHanded = max(creTarget.cdsCurrent.proficiencyTwoHanded, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSWORDANDSHIELD:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencySwordAndShield += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencySwordAndShield = max(creTarget.cdsCurrent.proficiencySwordAndShield, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCYSINGLEWEAPON:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencySingleWeapon += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencySingleWeapon = max(creTarget.cdsCurrent.proficiencySingleWeapon, effect.nParam1);
            break;
        }
        break;
    case STATS_PROFICIENCY2WEAPON:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyTwoWeapon += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyTwoWeapon = max(creTarget.cdsCurrent.proficiencyTwoWeapon, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY1:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra1 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra1 = max(creTarget.cdsCurrent.proficiencyExtra1, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY2:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra2 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra2 = max(creTarget.cdsCurrent.proficiencyExtra2, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY3:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra3 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra3 = max(creTarget.cdsCurrent.proficiencyExtra3, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY4:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra4 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra4 = max(creTarget.cdsCurrent.proficiencyExtra4, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY5:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra5 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra5 = max(creTarget.cdsCurrent.proficiencyExtra5, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY6:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra6 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra6 = max(creTarget.cdsCurrent.proficiencyExtra6, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY7:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra7 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra7 = max(creTarget.cdsCurrent.proficiencyExtra7, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY8:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra8 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra8 = max(creTarget.cdsCurrent.proficiencyExtra8, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY9:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra9 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra9 = max(creTarget.cdsCurrent.proficiencyExtra9, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY10:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra10 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra10 = max(creTarget.cdsCurrent.proficiencyExtra10, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY11:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra11 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra11 = max(creTarget.cdsCurrent.proficiencyExtra11, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY12:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra12 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra12 = max(creTarget.cdsCurrent.proficiencyExtra12, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY13:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra13 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra13 = max(creTarget.cdsCurrent.proficiencyExtra13, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY14:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra14 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra14 = max(creTarget.cdsCurrent.proficiencyExtra14, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY15:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra15 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra15 = max(creTarget.cdsCurrent.proficiencyExtra15, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY16:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra16 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra16 = max(creTarget.cdsCurrent.proficiencyExtra16, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY17:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra17 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra17 = max(creTarget.cdsCurrent.proficiencyExtra17, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY18:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra18 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra18 = max(creTarget.cdsCurrent.proficiencyExtra18, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY19:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra19 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra19 = max(creTarget.cdsCurrent.proficiencyExtra19, effect.nParam1);
            break;
        }
        break;
    case STATS_EXTRAPROFICIENCY20:
        switch (wParam2High) {
        case 1: //increment
            creTarget.cdsDiff.proficiencyExtra20 += effect.nParam1;
            bPurge = FALSE;
        default: //set higher only
            creTarget.cdsCurrent.proficiencyExtra20 = max(creTarget.cdsCurrent.proficiencyExtra20, effect.nParam1);
            break;
        }
        break;
    default:
        LPCTSTR lpsz = "DETOUR_CEffectProficiencyMod::DETOUR_ApplyEffect(): Invalid wParam2Low\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        break;
    }
    return TRUE;
}

CEffectRepeatingEff& DETOUR_CEffectRepeatingEff::DETOUR_Construct(ITEM_EFFECT& eff, POINT& ptSource, Enum eSource, int ptDestX, int ptDestY) {
    if (0) IECString("DETOUR_CEffectRepeatingEff::DETOUR_Construct");

    CEffectRepeatingEff& e = (this->*Tramp_CEffectRepeatingEff_Construct_5)(eff, ptSource, eSource, ptDestX, ptDestY);
    e.effect.nParam4 = g_pChitin->pGame->m_WorldTimer.nGameTime;
    return e;
}

BOOL DETOUR_CEffectRepeatingEff::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectRepeatingEff::DETOUR_ApplyEffect");

    CRepeatingEff* pRepeat = IENew CRepeatingEff();
    pRepeat->eSource = eSource;

    short wParam2Low = effect.nParam2 & 0xFFFF;
    short wParam2High = effect.nParam2 >> 16;
    pRepeat->rResource = effect.rResource;
    pRepeat->pEffect = this;

    int nDuration = effect.nDuration;
    if (!(effect.nTiming == TIMING_INSTANT_LIMITED  ||
          effect.nTiming == TIMING_DELAY_LIMITED    ||
          effect.nTiming == TIMING_DELAYEND_LIMITED ||
          effect.nTiming == TIMING_END_TICKS)) {
            nDuration = INT_MAX;
    }

    int nTicksBegan = effect.nParam4;

    switch (wParam2Low) {
    case 0:
        pRepeat->wMode = 0;
        pRepeat->wPostDelayDuration = effect.nParam1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 2:
        pRepeat->wMode = 2;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam1;
        pRepeat->wPeriod = 1;
        pRepeat->nTicksLeft = nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = 0;
        break;
    case 3:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = 1;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    case 4:
        pRepeat->wMode = 3;
        pRepeat->wPostDelayDuration = 1;
        pRepeat->wAmount = effect.nParam3;
        pRepeat->wPeriod = effect.nParam1;
        pRepeat->nTicksLeft = nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime;
        pRepeat->wSecondsElapsed = (g_pChitin->pGame->m_WorldTimer.nGameTime - nTicksBegan) / 15 % pRepeat->wPeriod;
        break;
    case 1:
    default:
        LPCTSTR lpsz = "DETOUR_CEffectRepeatingEff::DETOUR_ApplyEffect(): invalid effect.nParam2 low word (%d)\r\n";
        console.writef(lpsz, wParam2Low);
        L.timestamp();
        L.appendf(lpsz, wParam2Low);
        bPurge = TRUE;
        break;
    }

    creTarget.cdsCurrent.m_RepeatingEffs.AddTail(pRepeat);

    return TRUE;
}

BOOL DETOUR_CEffectWingBuffet::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectWingBuffet::DETOUR_ApplyEffect");

    if ((unsigned int)effect.nParam2 < 5)
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_WINGBUFFET, effect.nParam2);
    return (this->*Tramp_CEffectWingBuffet_ApplyEffect)(creTarget);
}

BOOL DETOUR_CEffectDisintegrate::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectDisintegrate::DETOUR_ApplyEffect");

    Object& o = creTarget.GetCurrentObject();
    BOOL bMatch = FALSE;
    switch (effect.nParam2) {
    case 2:
        if (o.EnemyAlly == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
        break;
    case 3:
        if (o.General == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
        break;
    case 4:
        if (o.Race == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
        break;
    case 5:
        {
            unsigned char nClass = o.GetClass();
            unsigned char nClassNew;
            unsigned char nClassOrg;
            o.GetDualClasses(&nClassNew, &nClassOrg);
            if (nClassNew != nClassOrg && //dual-classed
                !o.HasActiveSubclass(nClassOrg, TRUE)) {
                nClass = nClassNew;
            }
            if (nClass == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
            break;
        }
    case 6:
        if (o.Specific == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
        break;
    case 7:
        if (o.Gender == effect.nParam1 || effect.nParam1 == 0) bMatch = TRUE;
        break;
    case 8:
        if (
            (o.Alignment == effect.nParam1 || effect.nParam1 == 0) ||
            (effect.nParam1 == ALIGN_MASK_GOOD && (o.Alignment & 0xF) == ALIGN_MASK_GOOD) ||
            (effect.nParam1 == ALIGN_MASK_NEUTRAL && (o.Alignment & 0xF) == ALIGN_MASK_NEUTRAL) ||
            (effect.nParam1 == ALIGN_MASK_EVIL && (o.Alignment & 0xF) == ALIGN_MASK_EVIL) ||
            (effect.nParam1 == ALIGN_LAWFUL_MASK && (o.Alignment & 0xF0) == ALIGN_LAWFUL_MASK) ||
            (effect.nParam1 == ALIGN_NEUTRAL_MASK && (o.Alignment & 0xF0) == ALIGN_NEUTRAL_MASK) ||
            (effect.nParam1 == ALIGN_CHAOTIC_MASK && (o.Alignment & 0xF0) == ALIGN_CHAOTIC_MASK)
        ) bMatch = TRUE;
        break;
    default:
        break;
    }

    if (bMatch) {
        CEffectInstantDeath* pEffect = new CEffectInstantDeath();
        pEffect->effect.nParam2 = EFFECTINSTANTDEATH_TYPE_DISINTEGRATE;
        pEffect->effect.ptSource = effect.ptSource;
        pEffect->eSource = eSource;
        pEffect->enum2 = enum2;
        creTarget.AddEffect(*pEffect, 1, TRUE, TRUE);
    }

    bPurge = TRUE;
    return TRUE;
}

BOOL DETOUR_CEffectRemoveProjectile::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectRemoveProjectile::DETOUR_ApplyEffect");

    CArea* pArea = creTarget.pArea;
    if (pArea == NULL) return TRUE;
    
    switch (effect.nParam1) {
    case 1:
        {
            CMessageRemoveAreaAirEffectSpecific* pMsg = new CMessageRemoveAreaAirEffectSpecific();
            pMsg->rAreaName = pArea->rAreaName;
            pMsg->rResource = effect.rResource;
            g_pChitin->messages.Send(*pMsg, FALSE);
        }
        break;
    default:
        {
            CMessageRemoveAreaAirEffects* pMsg = IENew CMessageRemoveAreaAirEffects();
            pMsg->rAreaName = pArea->rAreaName;
            g_pChitin->messages.Send(*pMsg, FALSE);
        }
        break;
    }
    bPurge = 1;
    return TRUE;
    
}

BOOL DETOUR_CEffectEnableButton::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectEnableButton::DETOUR_ApplyEffect");

    if ((unsigned int)effect.nParam2 < 14)
        creTarget.cdsCurrent.ButtonDisable[effect.nParam2] = 0;
    if ((unsigned int)effect.nParam2 < 15) {
        CDerivedStats_SetStat(creTarget.cdsCurrent, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_BUTTONDISABLESTEALTH + effect.nParam2, 0);
    //if ((unsigned int)effect.nParam2 < 15) {
        creTarget.EffectsEquipped.RemoveEffect(creTarget, CEFFECT_OPCODE_DISABLE_BUTTON, creTarget.EffectsEquipped.posItrPrev, effect.nParam2, effect.rResource, FALSE);
        creTarget.EffectsMain.RemoveEffect(creTarget, CEFFECT_OPCODE_DISABLE_BUTTON, creTarget.EffectsMain.posItrPrev, effect.nParam2, effect.rResource, FALSE);
    }

    bPurge = TRUE;
    return TRUE;
}

BOOL DETOUR_CEffectCutScene2::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectCutScene2::DETOUR_ApplyEffect");

    CInfGame* pGame = g_pChitin->pGame;
    CInfGame::CGameSave* pGameSave = &pGame->m_GameSave;

    if (pGameSave->inputMode == 0x1016E || pGameSave->inputMode == 0x3016E) {
        bPurge = TRUE;
        return TRUE;
    }

    switch (effect.nParam1) {
        case 1:
            //Use StorePartyLocations() list
            pGame->StorePartyLocations(FALSE);
            break;
        case 2:
            //Do not store party member locations
            break;
        case 0:
        default:
            //Use pocket plane list
            pGame->StorePartyLocations(TRUE);
            break;
    }

    CMessageCutSceneMode* pcmCSM = IENew CMessageCutSceneMode();
    pcmCSM->eTarget = creTarget.id;
    pcmCSM->eSource = creTarget.id;
    pcmCSM->bEnable = true;
    g_pChitin->messages.Send(*pcmCSM, FALSE);

    CMessageAddActionHead* pcmA = IENew CMessageAddActionHead();
    pcmA->eTarget = pGame->ePlayersPartyOrder[0];
    pcmA->eSource = pGame->ePlayersPartyOrder[0];
    pcmA->a.opcode = 120; //StartCutScene

    IECString tmpstr;
    effect.rResource.CopyToString(tmpstr);
    pcmA->a.sName1 = effect.nParam2 ? tmpstr : "cut250a";

    pcmA->a.dwFlags |= 1;
    g_pChitin->messages.Send(*pcmA, FALSE);

    bPurge = TRUE;
    return TRUE;
}

BOOL DETOUR_CEffectAnimationRemoval::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffectAnimationRemoval::DETOUR_ApplyEffect");

    DWORD nSize = pRuleEx->m_nStats - 200;

    if (creTarget.cdsCurrent.animationRemoval) {
        int* pStatsEx = (int*)creTarget.cdsCurrent.animationRemoval;
        pStatsEx[0] = effect.nParam2;
    } else {
        LPCTSTR lpsz = "DETOUR_CEffectAnimationRemoval::DETOUR_ApplyEffect(): pStatsEx == NULL\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
    }

    return TRUE;
}

CEffectSetStat::CEffectSetStat(ITEM_EFFECT& data, POINT& ptSource, Enum eSource, int destX, int destY, BOOL bUseDice, Enum e2) \
    : CEffect(data, ptSource, eSource, destX, destY, bUseDice, e2) {}


CEffect& CEffectSetStat::Duplicate() {
    ITEM_EFFECT* ItemEffect = this->GetItemEffect();
    CEffectSetStat* pceff = new CEffectSetStat(*ItemEffect, effect.ptSource, eSource, effect.ptDest.x, effect.ptDest.y, FALSE, ENUM_INVALID_INDEX);
    pceff->Unmarshal(effect);
    IEFree(ItemEffect);
    return *pceff;
}

BOOL CEffectSetStat::ApplyEffect(CCreatureObject& creTarget) {
    short unsigned nOpcode = effect.nParam2 & 0xFFFF;
    short nModType = effect.nParam2 >> 16;

    DWORD nSize = pRuleEx->m_nStats;

    if (nOpcode < 387) {
        LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): Tried to set a stat with index < 387 (expected 387-%d)\r\n";
        L.timestamp();
        L.appendf(lpsz, nSize);
        console.writef(lpsz, nSize);
        return TRUE;
    }

    if (nOpcode > nSize) {
        LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): nOpcode %d out of bounds (expected maximum %d)\r\n";
        L.timestamp();
        L.appendf(lpsz, nOpcode, nSize);
        console.writef(lpsz, nOpcode, nSize);
        return TRUE;
    }

    if (!creTarget.cdsCurrent.animationRemoval) {
        LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): creTarget.cdsCurrent.pStatsEx == NULL\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        return TRUE;
    }

    int* pStatsEx = (int*)creTarget.cdsCurrent.animationRemoval;

    switch (nModType) {
    case 0: //sum
        pStatsEx[nOpcode - 200 - 1] += effect.nParam1;
        break;
    case 1: //set
        pStatsEx[nOpcode - 200 - 1] = effect.nParam1;
        break;
    case 2: //percent
        pStatsEx[nOpcode - 200 - 1] = pStatsEx[nOpcode - 200 - 1] * effect.nParam1 / 100;
        break;
    case 3: //multiply
        pStatsEx[nOpcode - 200 - 1] *= effect.nParam1;
        break;
    case 4: //divide
        if (effect.nParam1 != 0) {
            pStatsEx[nOpcode - 200 - 1] /= effect.nParam1;
        } else {
            LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): Tried to divide by zero\r\n";
            L.timestamp();
            L.appendf(lpsz, nSize);
            console.writef(lpsz, nSize);
            return TRUE;
        }
        break;
    case 5: //modulus
        if (effect.nParam1 != 0) {
            pStatsEx[nOpcode - 200 - 1] %= effect.nParam1;
        } else {
            LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): Tried to divide by zero\r\n";
            L.timestamp();
            L.appendf(lpsz, nSize);
            console.writef(lpsz, nSize);
            return TRUE;
        }
        break;
    case 6: //logical and
        pStatsEx[nOpcode - 200 - 1] = pStatsEx[nOpcode - 200 - 1] && effect.nParam1;
        break;
    case 7: //logical or
        pStatsEx[nOpcode - 200 - 1] = pStatsEx[nOpcode - 200 - 1] || effect.nParam1;
        break;
    case 8: //bitwise and
        pStatsEx[nOpcode - 200 - 1] = pStatsEx[nOpcode - 200 - 1] & effect.nParam1;
        break;
    case 9: //bitwise or
        pStatsEx[nOpcode - 200 - 1] = pStatsEx[nOpcode - 200 - 1] | effect.nParam1;
        break;
    case 10: //not
        pStatsEx[nOpcode - 200 - 1] = !pStatsEx[nOpcode - 200 - 1];
        break;
    default:
        LPCTSTR lpsz = "CEffectSetStat::ApplyEffect(): nModType %d invalid (expected 0-10)\r\n";
        L.timestamp();
        L.appendf(lpsz, nModType);
        console.writef(lpsz, nModType);
        break;
    }

    return TRUE;
}

extern CCreatureObject* gCreateAnimationCre;

void __stdcall CEffectPolymorph_ApplyEffect_ReinitAnimation(CEffect& eff, CCreatureObject& creTarget, CCreatureObject& creNew) {
    short wOrient = creTarget.wOrientInstant;
    short wAnimId = eff.effect.nParam1;
    AnimData* pAD = &creTarget.animation;
    bool bAnimMatch = wAnimId == pAD->pAnimation->wAnimId;

    if (!bAnimMatch) {
        delete pAD->pAnimation;
        pAD->pAnimation = NULL;

        gCreateAnimationCre = & creTarget;  // track creature
        pAD->pAnimation = CAnimation::CreateAnimation(wAnimId, creNew.BaseStats.colors, wOrient);
        gCreateAnimationCre = NULL;
    }

    for (int i = 0; i < 7; i++) {
        ColorPal* pColor = IENew struct ColorPal;
        pColor->m_cColorGroup = i;
        pColor->m_cGroupRangeId = *((char*)&creNew.BaseStats.colors + i);
        creTarget.cdsCurrent.ColorListPal.AddTail(pColor);
        if (bAnimMatch) pAD->pAnimation->SetColorRange(pColor->m_cColorGroup, pColor->m_cGroupRangeId);
    }

    return;
}


BOOL static __stdcall
CheckForDeadCre(CCreatureObject& Cre, CVisualEffect& vis_eff) {
    CDerivedStats& cds = Cre.GetDerivedStats();

    if (cds.stateFlags & STATE_DEAD &&  // or cre.BaseStats.stateFlags & STATE_DEATH ?
        vis_eff.m_dwFlags & 0x08)       // target = self
        return TRUE;                    // stop effect if owner is dead

    if (!Cre.bFree &&               // mazed/imprisoned
       vis_eff.m_dwFlags & 0x08)    // target = self
        return TRUE;                // stop effect if owner is out of area

    return FALSE;   // Cre on area
}


void static __stdcall
ApplyTracking(CCreatureObject& Cre, CVisualEffect& vis_eff, POINT* effect_pos) {
    if (vis_eff.m_dwFlags & 0x08 ||     // target = self
        vis_eff.m_dwFlags & 0x10)       // target = preset target
        *effect_pos = Cre.currentLoc;   // move effect if owner or target are moving
}


void static __stdcall
MarkVisualEffect(Enum ObjID, CEffect& eff) {
    CVisualEffect*   vis_eff;
    char returnVal;

    do {
        returnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(ObjID, THREAD_ASYNCH, &vis_eff, -1);
    } while (returnVal == OBJECT_SHARING || returnVal == OBJECT_DENYING);
    if (returnVal == OBJECT_SUCCESS) {
        if (eff.effect.nTargetType == 1 &&  // target = self
            eff.effect.nParam2 == 0) {      // Effect State = Over target (unattached)
            vis_eff->m_dwFlags |= 0x08;
        }

        if (eff.effect.nTargetType == 2 &&  // target = preset target
            eff.effect.nParam2 == 0) {      // Effect State = Over target (unattached)
            vis_eff->m_dwFlags |= 0x10;
        }

        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
    }
}


BOOL static __stdcall
CProjectileCastingGlow_AIUpdate_CheckInterrupt(CProjectileCastingGlow& Pro) {
    Enum Cre_id = Pro.eTarget;
    CCreatureObject* Cre;
    BOOL    result = FALSE;
    
    char nResult;
    do {
        nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(Cre_id, THREAD_ASYNCH, &Cre, INFINITE);
    } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

    if (nResult != OBJECT_SUCCESS &&
        nResult != OBJECT_DELETED) 
        return FALSE;

    if (nResult == OBJECT_DELETED) {
        Pro.OnArrival();    // Remove Projectile
        return TRUE;
    }

    if (Cre->nObjType == CGAMEOBJECT_TYPE_CREATURE) {
        if (!Cre->bStartedCasting &&
            !Cre->bInCasting && 
            Cre->animation.wCurrentSequence != SEQ_CAST &&
            Cre->animation.wCurrentSequence != SEQ_CONJURE &&
            Cre->animation.wCurrentSequence != SEQ_READY && // continue even on READY sequence
            Cre->u6733 == 0x66 &&                           // ApplyCastingEffect
            Pro.delay == 0) {                               // projectile is started
                Pro.OnArrival();    // Remove Projectile
                result=TRUE;

                if (Cre->sndCasting.IsSoundPlaying(0)) {
                        Cre->sndCasting.Stop();
                }
        } else
                result=FALSE;
    }

    g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(Cre_id, THREAD_ASYNCH, INFINITE);
    return result;
}


void static __stdcall
CEffectSparkle_Apply_AddCreId(CSparkleCluster& Sparkle, CCreatureObject& Cre) {
    Sparkle.o.eTarget = Cre.id; // use eTarget(unused) as parent id
    Sparkle.o.Race = 0x55;      // our marker
}


BOOL static __stdcall
CSparkle_AIUpdate_CheckInterrupt(CSparkleCluster& Sparkle) {
    if (Sparkle.o.Race == 0x55) {   // marker
        Enum Cre_id = Sparkle.o.eTarget;
        CCreatureObject* Cre;
        BOOL    result = FALSE;

        char returnVal;
        do {
            returnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(Cre_id, THREAD_ASYNCH, &Cre, -1);
        } while (returnVal == OBJECT_SHARING || returnVal == OBJECT_DENYING);

        if (returnVal != OBJECT_SUCCESS &&
            returnVal != OBJECT_DELETED) 
            return FALSE;

        if (returnVal == OBJECT_DELETED) {
            // remove Sparkle effect
            POSITION pos = Sparkle.particleList.GetHeadPosition();
            while (pos) {
			    CParticle* ptr = (CParticle*) Sparkle.particleList.GetNext(pos);
			    IEFree(ptr);
		    }
            Sparkle.particleList.RemoveAll();
            return TRUE;
        }

        if (Cre->nObjType == CGAMEOBJECT_TYPE_CREATURE) {
            if (!Cre->bInCasting &&
                !Cre->bStartedCasting &&
                Cre->animation.wCurrentSequence != SEQ_CAST &&
                Cre->animation.wCurrentSequence != SEQ_CONJURE &&
                Cre->animation.wCurrentSequence != SEQ_READY &&     // continue even on READY sequence) { 
                Cre->u6733 == 0x66) {                               // ApplyCastingEffect
                    // remove Sparkle effect
                    POSITION pos = Sparkle.particleList.GetHeadPosition();
                    while (pos) {
			            CParticle* ptr = (CParticle*) Sparkle.particleList.GetNext(pos);
			            IEFree(ptr);
		            }
                    Sparkle.particleList.RemoveAll();
                    result=TRUE;

            } else
                    result=FALSE;
        }

        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(Cre_id, THREAD_ASYNCH, INFINITE);
        return result;

    } else
        return FALSE;
}


void __stdcall
CAIGroup_ClearActions_CheckInterrupt(CCreatureObject& Cre) {
    short current_opcode = Cre.currentAction.opcode;

    if (current_opcode == ACTION_SPELL               ||
        current_opcode == ACTION_SPELL_POINT         ||
        current_opcode == ACTION_SPELL_NO_DEC        ||
        current_opcode == ACTION_SPELL_POINT_NO_DEC  ||
        current_opcode == ACTION_FORCESPELL          ||
        current_opcode == ACTION_FORCESPELL_POINT)   {
        if ((Cre.bInCasting || Cre.bStartedCasting) &&
             Cre.bInterruptSpellcasting != TRUE) {   // EffectDamage::Apply() handles bInterruptSpellcasting state and output feedback
            //Cre.PrintEventMessage(EVENTMESSAGE_SPELLFAILED_INTERRUPT, 0, 0, 0, -1, 0, IECString(""));
            STRREF MyStrRef = 3720; // ~Spell Failed: Action Change~
            Cre.DisplayTextRef(Cre.GetLongNameStrRef(), MyStrRef, *(g_pColorRangeArray + Cre.BaseStats.colors.colorMajor), 0xBED7D7u);

            if (pGameOptionsEx) { // if tobex is alive
                if (pGameOptionsEx->bEff_StopInterruptedCastingAnimation)
                    if (Cre.sndCasting.IsSoundPlaying(0)) {
                        Cre.sndCasting.Stop();
                    }
            }

        }
    }
}


void __stdcall
CGameSprite_SetSequence_StopCastingSound(CCreatureObject& Cre, short NewSeq) {
    if (NewSeq != SEQ_CAST &&
        NewSeq != SEQ_CONJURE &&
        NewSeq != SEQ_READY &&  // continue playing casting PostEFF sound even on READY sequence
        Cre.u6733 == 0x66)      // ApplyCastingEffect
        if (Cre.sndCasting.IsSoundPlaying(0)) {
            Cre.sndCasting.Stop();
        }
}


void __stdcall
CGameSprite_ApplyCastingEffect_Trigger(CCreatureObject& Cre) {
    Cre.u6733 = 0x66;  // ApplyCastingEffect
}


void __stdcall
CGameSprite_ApplyCastingEffectPost_Trigger(CCreatureObject& Cre) {
    Cre.u6733 = 0x00;  // ApplyCastingEffectPost
}


//BOOL DETOUR_CEffectCharm::DETOUR_ApplyEffect(CCreatureObject& creTarget) {
//    Object oTarget = creTarget.GetCurrentObject();
//
//    if (effect.bFirstCall == 1 &&
//        (oTarget.Race == RACE_ELF || oTarget.Race == RACE_HALF_ELF)) {
//        int nDieRoll = IERand(100) + 1;
//
//        if (oTarget.Race == RACE_ELF && nDieRoll <= 90) {       // 90% ELF charm resistance
//            bPurge = TRUE;
//            return TRUE;
//        }
//
//        if (oTarget.Race == RACE_HALF_ELF && nDieRoll <= 30) {  // 30% HALF_ELF charm resistance
//            bPurge = TRUE;
//            return TRUE;
//        }
//    }
//
//    return (this->*Tramp_CEffectCharm_ApplyEffect)(creTarget);
//}


CPP_MEMBERHOOK_1args(CEffectInvisibility, CEffectInvisibility_Apply, 0x5124E5, CCreatureObject&)
int DETOUR_CEffectInvisibility::DETOUR_CEffectInvisibility_Apply(CCreatureObject& Cre) {
    int res = (this->*Tramp_CEffectInvisibility_Apply)(Cre);
    CEffect& eff = * (CEffect*)this;
    if (eff.effect.nParam2 == 1 &&  // Improved Invisibility
        eff.bPurge != TRUE) {
        if (!Cre.PortraitIcons.Find((void*)109, NULL))
            Cre.PortraitIcons.AddTail((void*)109);
    }

    return res;
}


// #65 Blur::Apply
CPP_MEMBERHOOK_1args(CEffectBlur, CEffectBlur_Apply, 0x503700, CCreatureObject&)
int DETOUR_CEffectBlur::DETOUR_CEffectBlur_Apply(CCreatureObject& Cre) {
    int res = (this->*Tramp_CEffectBlur_Apply)(Cre);
    if (((CEffect*)this)->bPurge != TRUE) {
        if (gStaticIconsMode != 2)
            if (!Cre.PortraitIcons.Find((void*)61, NULL))
                Cre.PortraitIcons.AddTail((void*)61);
    }

    return res;
}


// #159 Mirror Number Image::Apply
CPP_MEMBERHOOK_1args(CEffectMirrorNumberImage, CEffectMirrorNumberImage_Apply, 0x521245, CCreatureObject&)
int DETOUR_CEffectMirrorNumberImage::DETOUR_CEffectMirrorNumberImage_Apply(CCreatureObject& Cre) {
    int res = (this->*Tramp_CEffectMirrorNumberImage_Apply)(Cre);
    if (((CEffect*)this)->bPurge != TRUE) {
        if (!Cre.PortraitIcons.Find((void*)61, NULL))
            Cre.PortraitIcons.AddTail((void*)61);
    }

    return res;
}


void __stdcall
CEffectCreateInventoryItem_CheckSimulacrum(CItem& Item, CCreatureObject& Cre) {
    CEffectList& listMain = Cre.EffectsMain;
    CEffect* pEff = NULL;

    POSITION pos = listMain.GetHeadPosition();        
    while (pos != NULL) {
        pEff = (CEffect *) listMain.GetNext(pos);
        if (pEff->effect.nParentResourceType == 1  &&       // spell
            pEff->effect.rParentResource == "SIMULACR") {   // Simulacrum marker
                Item.dwFlags |= CREITEM_UNDROPPABLE;
                break;
            }
    }
}


void  __declspec(naked)
CVisualEffect_AIUpdate_CheckForDeath_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-2B4h]  // VEffect
    push    edx         // Cre
    call    CheckForDeadCre
    test    eax,eax

    pop     edx
    pop     ecx
    pop     eax
    jz      CVisualEffect_AIUpdate_CheckForDeath_continue

    // remove effect
    add     esp, 4      // kill retadr
    push    0656144h    // delete effect
    ret

CVisualEffect_AIUpdate_CheckForDeath_continue:
    mov     ax, [edx+33E8h] // Stolen bytes
    ret
}
}


void  __declspec(naked)
CVisualEffect_Apply_AddSelfMarker_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-0ACh]  // Ceffect
    push    eax         // objID of CVisualEffect
    call    MarkVisualEffect

    pop     edx
    pop     ecx
    pop     eax

    mov     [ebp-0B4h], eax // Stolen bytes
    ret
}
}


void  __declspec(naked)
CVisualEffect_AIUpdate_ApplyTracking_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-1Ch]  // eff_pos
    push    eax
    push    [ebp-2B4h]      // VEffect
    push    [ebp-34h]       // Cre
    call    ApplyTracking

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp-2B4h] // Stolen bytes
    ret
}
}



void  __declspec(naked)
CProjectileCastingGlow_AIUpdate_CheckInterrupt_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-18h]      // ProjectileCastingGlow
    call    CProjectileCastingGlow_AIUpdate_CheckInterrupt
    test    eax,eax

    pop     edx
    pop     ecx
    pop     eax
    jz      CProjectileCastingGlow_AIUpdate_CheckInterrupt_continue

    add     esp, 4
    push    0615AB6h    // abort
    ret

CProjectileCastingGlow_AIUpdate_CheckInterrupt_continue:
    mov     ecx, [ebp-18h] // Stolen bytes
    xor     edx, edx
    ret
}
}


void  __declspec(naked)
CEffectSparkle_Apply_AddCreId_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // Cre
    push    [ebp-10h]   // CSparkleCluster
    call    CEffectSparkle_Apply_AddCreId
    
    pop     edx
    pop     ecx
    pop     eax
    
    mov     ecx, [ebp-0A4h] // Stolen bytes
    ret
}
}


void  __declspec(naked)
CSparkle_AIUpdate_CheckInterrupt_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-90h]   // CSparkleCluster
    call    CSparkle_AIUpdate_CheckInterrupt
    test    eax,eax

    pop     edx
    pop     ecx
    pop     eax
    jz      CSparkle_AIUpdate_CheckInterrupt_continue

    add     esp, 4
    push    05FAB22h    // Remove path
    ret
    
CSparkle_AIUpdate_CheckInterrupt_continue:
    mov     edx, [ebp-90h] // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameAIBase_ClearActions_CheckInterrupt_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-28h]   // Cre
    call    CAIGroup_ClearActions_CheckInterrupt

    pop     edx
    pop     ecx
    pop     eax

    mov     word ptr [edx+2B6h], 0FFFFh // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_SetSequence_StopCastingSound_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // wSequence
    push    [ebp-150h]  // Cre
    call    CGameSprite_SetSequence_StopCastingSound

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp-150h] // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_ApplyCastingEffect_Trigger_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-1F4h]   // Cre
    call    CGameSprite_ApplyCastingEffect_Trigger

    pop     edx
    pop     ecx
    pop     eax

    mov     dword ptr [ebp-4], 0    // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_ApplyCastingEffectPost_Trigger_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-6Ch]   // Cre
    call    CGameSprite_ApplyCastingEffectPost_Trigger

    pop     edx
    pop     ecx
    pop     eax

    mov     dword ptr [ebp-1Ch], 0 // Stolen bytes
    ret
}
}



void  __declspec(naked)
CEffectCreateInventoryItem_Apply_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp+8] // Cre
    push    eax     // Item
    call    CEffectCreateInventoryItem_CheckSimulacrum

    pop     edx
    pop     ecx
    pop     eax

    mov     [ebp-0ACh], eax // Stolen bytes
    ret
}
}