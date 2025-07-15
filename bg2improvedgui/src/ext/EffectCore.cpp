#include "EffectCore.h"

#include "stdafx.h"
#include "chitin.h"
#include "options.h"
#include "effcore.h"
#include "effopcode.h"
#include "EffectCommon.h"
#include "EffectOpcode.h"
#include "console.h"
#include "log.h"

CEffect& (*Tramp_CEffect_CreateEffect)(ITEM_EFFECT&, POINT&, Enum, POINT&, Enum) =
    SetFP(static_cast<CEffect& (*)(ITEM_EFFECT&, POINT&, Enum, POINT&, Enum)>   (&CEffect::CreateEffect),       0x4F3EC2);
BOOL (CEffect::*Tramp_CEffect_ApplyTiming)(CCreatureObject&) =
    SetFP(static_cast<BOOL (CEffect::*)(CCreatureObject&)>                      (&CEffect::ApplyTiming),        0x4FFFA0);

BOOL (CEffect::*Tramp_CEffect_CheckSave)(CCreatureObject&, char&, char&, char&, char&, char&, char&) =
    SetFP(static_cast<BOOL (CEffect::*)(CCreatureObject&, char&, char&, char&, char&, char&, char&)>(&CEffect::CheckSave),      0x501B29);
void (CEffectList::*Tramp_CEffectList_TryDispel)(CCreatureObject&, POSITION, BOOL, BOOL, char, char) =
    SetFP(static_cast<void (CEffectList::*)(CCreatureObject&, POSITION, BOOL, BOOL, char, char)>    (&CEffectList::TryDispel),  0x543EC8);

CEffect& DETOUR_CEffect::DETOUR_CreateEffect(ITEM_EFFECT& eff, POINT& ptSource, Enum eSource, POINT& ptDest, Enum e2) {
    if (0) IECString("DETOUR_CEffect::DETOUR_CreateEffect");

    CEffect* pEffect = NULL;
    switch (eff.opcode) {
    case CEFFECT_OPCODE_SET_STAT:   //0x13E
        if (pGameOptionsEx->bEngineExpandedStats) pEffect = new CEffectSetStat(eff, ptSource, eSource, ptDest.x, ptDest.y, FALSE, e2);
        break;
    case CEFFECT_OPCODE_CAST_SPELL: // 0x92
        if (pGameOptionsEx->bEffSavingThrowImprovedInvisibleBonus &&
            pGameOptionsEx->bEffSavingThrowFix) {
            // bg2fixpack bugged solution workaround
            ResRef spellname  = eff.resource;
            spellname.MakeUpper();
            if (spellname == ResRef("SPWI405A") ||  // Improved invis (mage)
                spellname == ResRef("SPDR401A") ||  // Invisible Stalker Improved Invisibility
                spellname == ResRef("SPIN687A") ||  // Create Shadows
                spellname == ResRef("SPIN698A") ||  // Cerebus Improved Invisibility
                spellname == ResRef("SPWI505A") ||  // Shadow door (mage)
                spellname == ResRef("SPWI607A") ||  // Mislead
                spellname == ResRef("SPWI721A") ||  // Mass invisibility
                spellname == ResRef("BALTH10A") ||  // Shadow Stance!
                spellname == ResRef("SPIN544A")     // PSIONIC_SUPERIOR_INVISIBILITY
               ) {
                eff.resource = ResRef("nonexist");  // skip casting
            }
        }
        break;
    default:
        break;
    }

    if (pEffect == NULL) {
        //Let the engine handle the standard opcodes
        pEffect = &Tramp_CEffect_CreateEffect(eff, ptSource, eSource, ptDest, e2);
    }
    else {
        switch (eff.opcode) {
        case CEFFECT_OPCODE_POISON:
            if (pGameOptionsEx->bEffPoisonFix) {
                pEffect->effect.nParam4 = g_pChitin->pGame->m_WorldTimer.nGameTime;
            }
            break;
        case CEFFECT_OPCODE_DISEASE:
            if (pGameOptionsEx->bEffDiseaseFix) {
                pEffect->effect.nParam4 = g_pChitin->pGame->m_WorldTimer.nGameTime;
            }
            break;
        case CEFFECT_OPCODE_REGENERATION:
            if (pGameOptionsEx->bEffRegenerationFix) {
                pEffect->effect.nParam4 = g_pChitin->pGame->m_WorldTimer.nGameTime;
            }
            break;
        default:
            break;
        }
    }

    return *pEffect;

}

BOOL DETOUR_CEffect::DETOUR_ApplyTiming(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CEffect::DETOUR_ApplyTiming");

    if (creTarget.o.IsNonScript()) return FALSE;

	//1000: if gameTime < duration (ticks), Apply; else purge
    //0: convert to 1000, Apply now, set duration [sec]
    //10: convert to 1000, Apply now, set duration (duration in ticks)
	    //1: Apply now, add to main effect list (permanent until death)
	    //2: Apply now, add to equipped effect list (while equipped)
	//3: convert to 6, Apply later, set duration [sec]
	//4: convert to 7, Apply later, set duration [sec]
	//5: convert to 8, Apply later, set duration [sec]
	//6: if gameTime >= duration (ticks), convert to 1000, Apply now set duration [sec]
	//7: if gameTime >= duration (ticks), convert to 1, Apply now
	//8: if gameTime >= duration (ticks), convert to 2, Apply now
	    //9: Apply now, will not be removed from CEffectList

    switch (effect.nTiming) {
    case TIMING_END_TICKS: //0x1000, apply until expired
        if (g_pChitin->pGame->m_WorldTimer.nGameTime >= effect.nDuration) {
            bPurge = TRUE;
            return TRUE;
        }
        break;
    case TIMING_INSTANT_LIMITED: //0, apply, set expiry time (duration in sec)
        effect.nTiming = TIMING_END_TICKS; //0x1000
        effect.nDuration = min(effect.nDuration, CEFFECT_DURATION_MAX);
        effect.nDuration = effect.nDuration * 15 + g_pChitin->pGame->m_WorldTimer.nGameTime;
        break;
    case TIMING_INSTANT_LIMITED_TICKS: //10, apply, set expiry time (duration in ticks)
        effect.nTiming = TIMING_END_TICKS; //0x1000
        effect.nDuration += g_pChitin->pGame->m_WorldTimer.nGameTime;
        break;
    case TIMING_DELAY_LIMITED: //3, set expiry time (duration in sec)
        effect.nTiming = TIMING_DELAYEND_LIMITED; //6
        effect.nDuration = min(effect.nDuration, CEFFECT_DURATION_MAX);
        effect.nDuration = effect.nDuration * 15 + g_pChitin->pGame->m_WorldTimer.nGameTime;
        return TRUE;
        break;
    case TIMING_DELAY_PERMANENT: //4, set expiry time (duration in sec)
        effect.nTiming = TIMING_DELAYEND_PERMANENT; //7
        effect.nDuration = min(effect.nDuration, CEFFECT_DURATION_MAX);
        effect.nDuration = effect.nDuration * 15 + g_pChitin->pGame->m_WorldTimer.nGameTime;
        return TRUE;
        break;
    case TIMING_DELAY_EQUIPPED: //5
        effect.nTiming = TIMING_DELAYEND_EQUIPPED; //8
        effect.nDuration = min(effect.nDuration, CEFFECT_DURATION_MAX);
        effect.nDuration = effect.nDuration * 15 + g_pChitin->pGame->m_WorldTimer.nGameTime;
        return TRUE;
        break;
    case TIMING_DELAYEND_LIMITED: //6, end delay if expired, then apply, set expiry time (duration in sec)
        if (g_pChitin->pGame->m_WorldTimer.nGameTime >= effect.nDuration) {
            effect.nTiming = TIMING_END_TICKS;//0x1000

            //nDurationAfterDelay = min(nDurationAfterDelay, CEFFECT_DURATION_MAX);
            nDurationAfterDelay = 3; // force 3 sec because time after apply is not declared in effect
            effect.nDuration = nDurationAfterDelay * 15 + g_pChitin->pGame->m_WorldTimer.nGameTime;
            OnAdd(creTarget);
            DisplayString(creTarget);
            break;
        } else {
            return TRUE;
        }
        break;
    case TIMING_DELAYEND_PERMANENT: //7, end delay if expired, then apply
        if (g_pChitin->pGame->m_WorldTimer.nGameTime >= effect.nDuration) {
            effect.nTiming = TIMING_INSTANT_PERMANENT; //1
            OnAdd(creTarget);
            DisplayString(creTarget);
            break;
        } else {
            return TRUE;
        }
        break;
    case TIMING_DELAYEND_EQUIPPED: //8, end delay if expired, then apply
        if (g_pChitin->pGame->m_WorldTimer.nGameTime >= effect.nDuration) {
            effect.nTiming = TIMING_INSTANT_EQUIPPED; //2
            OnAdd(creTarget);
            DisplayString(creTarget);
            break;
        } else {
            return TRUE;
        }
        break;
    default:
        break;
    }

    if (pGameOptionsEx->bEffStackingConfig &&
        (effect.nSaveType & CEFFECT_STACKING_LIMIT)) {
        CEffectList& listEquipped = creTarget.GetEquippedEffectsList();
        CEffectList& listMain = creTarget.GetMainEffectsList();
        CEffect* pFound = NULL;
        
        pFound = CEffectList_FindPrevious(listEquipped, *this, listEquipped.GetCurrentPosition(), creTarget);
        if (pFound == NULL) pFound = CEffectList_FindPrevious(listMain, *this, listMain.GetCurrentPosition(), creTarget);

        if (pFound) {
            effect.nSaveType |= CEFFECT_STACKING_SUSPEND;
            if (pGameOptionsEx->bDebugVerbose) {
                LPCTSTR lpsz = "DETOUR_CEffect::DETOUR_ApplyTiming(): Effect %X suspended (like %x)\r\n";
                console.writef(lpsz, (DWORD)this, (DWORD)pFound);
                L.timestamp();
                L.appendf(lpsz, (DWORD)this, (DWORD)pFound);
            }
            return TRUE;
        } else {
            if (effect.nSaveType & CEFFECT_STACKING_SUSPEND) {
                effect.nSaveType &= ~(CEFFECT_STACKING_SUSPEND);
                if (pGameOptionsEx->bDebugVerbose) {
                    LPCTSTR lpsz = "DETOUR_CEffect::DETOUR_ApplyTiming(): Effect %X resumed\r\n";
                    console.writef(lpsz, (DWORD)this);
                    L.timestamp();
                    L.appendf(lpsz, (DWORD)this);
                }
            }
        }
    }

    return ApplyEffect(creTarget);
}

BOOL DETOUR_CEffect::DETOUR_CheckSave(CCreatureObject& creTarget, char& rollSaveDeath, char& rollSaveWands, char& rollSavePoly, char& rollSaveBreath, char& rollSaveSpells, char& rollMagicResist) {
    if (0) IECString("DETOUR_CEffect::DETOUR_CheckSave");

    char cRollSaveDeath = rollSaveDeath & ~CRESAVE_USED;
    char cRollSaveWands = rollSaveWands & ~CRESAVE_USED;
    char cRollSavePoly = rollSavePoly & ~CRESAVE_USED;
    char cRollSaveBreath = rollSaveBreath & ~CRESAVE_USED;
    char cRollSaveSpells = rollSaveSpells & ~CRESAVE_USED;
    char cRollMagicResist = rollMagicResist & ~CRESAVE_USED;

    short wSaveRollTotal = 0;
    short RollRaw;
    BOOL bPrintMsg = FALSE;
    STRREF srSuccessSave;
    CDerivedStats& cdsTarget = creTarget.GetDerivedStats();

    //Mirror images and stone skins ignore checking saves
    //Unless poison effects
    if (effect.nOpcode == CEFFECT_OPCODE_POISON) {
        if (creTarget.nMirrorImages > 0 ||
            creTarget.GetDerivedStats().StoneSkins > 0 ||
            creTarget.GetDerivedStats().StoneSkinGolem > 0) {
            return FALSE;
        }
    }
    if (effect.nOpcode == CEFFECT_OPCODE_DISPLAY_ICON &&
        effect.nParam2 == 6) { //ICON_POISONED
        if (creTarget.nMirrorImages > 0 ||
            creTarget.GetDerivedStats().StoneSkins > 0 ||
            creTarget.GetDerivedStats().StoneSkinGolem > 0) {
            return FALSE;
        }
    }

    short wSaveThreshold = 20; //if multiple save types, checks against the easiest save type

    //Magic Resistance
    if (!(effect.dwFlags & CEFFECT_FLAG_IGNORE_RESISTANCE) &&
        cdsTarget.resistMagic > cRollMagicResist && //threshold > roll
        effect.dwFlags & CEFFECT_FLAG_DISPELLABLE) {
        if (!(rollMagicResist & CRESAVE_USED)) {
            CMessageDisplayDialogue* pMDD = IENew CMessageDisplayDialogue();
            pMDD->eTarget = creTarget.id;
            pMDD->eSource = creTarget.id;
            pMDD->srOwner = creTarget.GetLongNameStrRef();
            pMDD->srText = 19224; //'Magic Resistance'
            pMDD->rgbOwner = g_ColorDefaultText;
            pMDD->rgbText = g_ColorDefaultText;
            pMDD->u1c = -1;
            pMDD->u20 = 0;
            pMDD->bFloatText = false;
            pMDD->u22 = 0;
            pMDD->bPlaySound = true;

            g_pChitin->messages.Send(*pMDD, FALSE);
            rollMagicResist |= CRESAVE_USED;
        }
        return FALSE;
    }

    //Modified from vanilla, use only the lowest char
    if ((effect.nSaveType & 0xFF) == CEFFECT_SAVETYPE_NONE)
        return TRUE;

    if (pGameOptionsEx->bEffSavingThrowFix) {
        char cSaveTypeUsed = 0;

        //determine best saving throw
        if (effect.nSaveType & CEFFECT_SAVETYPE_DEATH) {
            if (cdsTarget.saveDeath < wSaveThreshold) {
                wSaveRollTotal = cRollSaveDeath;
                wSaveThreshold = cdsTarget.saveDeath;
                srSuccessSave = 14009;
                cSaveTypeUsed = 1;
            }
        }
        if (effect.nSaveType & CEFFECT_SAVETYPE_WANDS) {
            if (cdsTarget.saveWands < wSaveThreshold) {
                wSaveRollTotal = cRollSaveWands;
                wSaveThreshold = cdsTarget.saveWands;
                srSuccessSave = 14006;
                cSaveTypeUsed = 2;
            }
        }
        if (effect.nSaveType & CEFFECT_SAVETYPE_POLYMORPH) {
            if (cdsTarget.savePoly < wSaveThreshold) {
                wSaveRollTotal = cRollSavePoly;
                wSaveThreshold = cdsTarget.savePoly;
                srSuccessSave = 14005;
                cSaveTypeUsed = 3;
            }
        }
        if (effect.nSaveType & CEFFECT_SAVETYPE_BREATH) {
            if (cdsTarget.saveBreath < wSaveThreshold) {
                wSaveRollTotal = cRollSaveBreath;
                wSaveThreshold = cdsTarget.saveBreath;
                srSuccessSave = 14004;
                cSaveTypeUsed = 4;
            }
        }
        if (effect.nSaveType & CEFFECT_SAVETYPE_SPELLS) {
            if (cdsTarget.saveSpell < wSaveThreshold) {
                wSaveRollTotal = cRollSaveSpells;
                wSaveThreshold = cdsTarget.saveSpell;
                srSuccessSave = 14003;
                cSaveTypeUsed = 5;
            }
        }

        switch (cSaveTypeUsed) {
        case 1: //death
            if (!(rollSaveDeath & CRESAVE_USED)) {
                rollSaveDeath |= CRESAVE_USED;
                bPrintMsg = TRUE;
            }
            break;
        case 2: //wands
            if (!(rollSaveWands & CRESAVE_USED)) {
                rollSaveWands |= CRESAVE_USED;
                bPrintMsg = TRUE;
            }
            break;
        case 3: //poly
            if (!(rollSavePoly & CRESAVE_USED)) {
                rollSavePoly |= CRESAVE_USED;
                bPrintMsg = TRUE;
            }
            break;
        case 4: //breath
            if (!(rollSaveBreath & CRESAVE_USED)) {
                rollSaveBreath |= CRESAVE_USED;
                bPrintMsg = TRUE;
            }
            break;
        case 5: //spells
            if (!(rollSaveSpells & CRESAVE_USED)) {
                rollSaveSpells |= CRESAVE_USED;
                bPrintMsg = TRUE;
            }
            break;
        default: //none
            break;
        }

        RollRaw = wSaveRollTotal;
        //apply modifiers to roll
        wSaveRollTotal += effect.nSaveBonus;

        if (effect.nSchool != 0 &&
            g_pChitin->pGame->MapCharacterSpecializationToSchool(creTarget.BaseStats.dwReversedkit) == effect.nSchool) {
            wSaveRollTotal += 2;
        }

        CCreatureObject* pCreSource = NULL;
        char returnVal;
        do {
            returnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eSource, THREAD_ASYNCH, &pCreSource, INFINITE);
        } while (returnVal == OBJECT_SHARING || returnVal == OBJECT_DENYING);
        if (returnVal == OBJECT_SUCCESS) {
            if (pCreSource->GetObjectType() == CGAMEOBJECT_TYPE_CREATURE) {
                wSaveRollTotal += creTarget.GetDerivedStats().m_SaveBonusVsObject.GetModValue(pCreSource->GetCurrentObject());

                if (pGameOptionsEx->bEffSavingThrowImprovedInvisibleBonus) {
                    //improved invis saving throw bonus inserted here
                    if (!pCreSource->CanSeeInvisible() &&
                        (cdsTarget.stateFlags & STATE_IMPROVEDINVISIBILITY)) {
                        wSaveRollTotal += 4;
                    }
                }

            }
            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eSource, THREAD_ASYNCH, INFINITE);
        }
    }

    //original code
    if (!pGameOptionsEx->bEffSavingThrowFix) {

        if (effect.nSaveType & CEFFECT_SAVETYPE_DEATH) {
            if (cdsTarget.saveDeath < wSaveThreshold) {
                wSaveRollTotal = cRollSaveDeath;
                wSaveThreshold = cdsTarget.saveDeath;
                srSuccessSave = 14009;
                if (!(rollSaveDeath & CRESAVE_USED)) {
                    if (wSaveRollTotal >= wSaveThreshold) {
                        rollSaveDeath |= CRESAVE_USED;
                        bPrintMsg = TRUE;
                    }
                }
            }
        }

        if (effect.nSaveType & CEFFECT_SAVETYPE_WANDS) {
            if (cdsTarget.saveWands < wSaveThreshold) {
                wSaveRollTotal = cRollSaveWands;
                wSaveThreshold = cdsTarget.saveWands;
                srSuccessSave = 14006;
                //srFailedSave = 10049;
                if (!(rollSaveWands & CRESAVE_USED)) {
                    if (wSaveRollTotal >= wSaveThreshold) {
                        rollSaveWands |= CRESAVE_USED;
                        bPrintMsg = TRUE;
                    }
                }
            }
        }

        if (effect.nSaveType & CEFFECT_SAVETYPE_POLYMORPH) {
            if (cdsTarget.savePoly < wSaveThreshold) {
                wSaveRollTotal = cRollSavePoly;
                wSaveThreshold = cdsTarget.savePoly;
                srSuccessSave = 14005;
                //srFailedSave = 10050;
                if (!(rollSavePoly & CRESAVE_USED)) {
                    if (wSaveRollTotal >= wSaveThreshold) {
                        rollSavePoly |= CRESAVE_USED;
                        bPrintMsg = TRUE;
                    }
                }
            }
        }

        if (effect.nSaveType & CEFFECT_SAVETYPE_BREATH) {
            if (cdsTarget.saveBreath < wSaveThreshold) {
                wSaveRollTotal = cRollSaveBreath;
                wSaveThreshold = cdsTarget.saveBreath;
                srSuccessSave = 14004;
                //srFailedSave = 10051;
                if (!(rollSaveBreath & CRESAVE_USED)) {
                    if (wSaveRollTotal >= wSaveThreshold) {
                        rollSaveBreath |= CRESAVE_USED;
                        bPrintMsg = TRUE;
                    }
                }
            }
        }

        if (effect.nSaveType & CEFFECT_SAVETYPE_SPELLS) {
            if (cdsTarget.saveSpell < wSaveThreshold) {
                wSaveRollTotal = cRollSaveSpells;
                wSaveThreshold = cdsTarget.saveSpell;
                srSuccessSave = 14003;
                //srFailedSave = 10052;
                if (!(rollSaveSpells & CRESAVE_USED)) {
                    if (wSaveRollTotal >= wSaveThreshold) {
                        rollSaveSpells |= CRESAVE_USED;
                        bPrintMsg = TRUE;
                    }
                }
            }
        }

        RollRaw = wSaveRollTotal;
        wSaveRollTotal += effect.nSaveBonus;

        if (effect.nSchool != 0 &&
            g_pChitin->pGame->MapCharacterSpecializationToSchool(creTarget.BaseStats.dwReversedkit) == effect.nSchool) {
            wSaveRollTotal += 2;
        }

        CCreatureObject* pCreSource = NULL;
        char returnVal;
        do {
            returnVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eSource, THREAD_ASYNCH, &pCreSource, INFINITE);
        } while (returnVal == OBJECT_SHARING || returnVal == OBJECT_DENYING);
        if (returnVal == OBJECT_SUCCESS) {
            if (pCreSource->GetObjectType() == CGAMEOBJECT_TYPE_CREATURE) {
                wSaveRollTotal += creTarget.GetDerivedStats().m_SaveBonusVsObject.GetModValue(pCreSource->GetCurrentObject());
            }
            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eSource, THREAD_ASYNCH, INFINITE);
        }
    }

    if (wSaveRollTotal >= wSaveThreshold) {
        //saved
        if (bPrintMsg &&
            !(cdsTarget.stateFlags & STATE_DEAD)) {
            if (g_pChitin->pGame->m_GameOptions.m_nEffectTextLevel & 1) {
                IECString sRoll;
                IECString sTreshold;
                CStrRef csr;
                IECString sText;

				g_pChitin->m_TlkTbl.GetTlkString(srSuccessSave, csr);

                if (pGameOptionsEx->bUI_CombatExtendedTextFull) {
                    if (wSaveRollTotal - RollRaw > 0)
                        sRoll.Format("%d + %d", RollRaw, wSaveRollTotal - RollRaw);
                    else
                    if (wSaveRollTotal == RollRaw)
                        sRoll.Format("%d", RollRaw);
                    else
                        sRoll.Format("%d - %d", RollRaw, -(wSaveRollTotal - RollRaw));

                    sTreshold.Format("%d", wSaveThreshold);
                    if ( wSaveRollTotal > wSaveThreshold)
                        sText = csr.text + " : " + sRoll + " > " + sTreshold;
                    else
                        sText = csr.text + " : " + sRoll + " >= " + sTreshold;

                } else {
                    sRoll.Format("%d", wSaveRollTotal);
				    sText = csr.text + " : " + sRoll;
                }

                CMessageDisplayText* pMDT = IENew CMessageDisplayText();
                pMDT->eTarget = creTarget.id;
                pMDT->eSource = creTarget.id;
                pMDT->sLeft = creTarget.GetLongName();
                pMDT->sRight = sText;
                pMDT->rgbLeft = g_ColorDefaultText;
                pMDT->rgbRight = g_ColorDefaultText;
                pMDT->marker = -1;
                pMDT->moveToTop = 0;
                pMDT->bFloatText = false;
                pMDT->overrideDialogMode = 0;
                pMDT->u23 = 0;

                g_pChitin->messages.Send(*pMDT, FALSE);
            } else if (g_pChitin->pGame->m_GameOptions.m_nEffectTextLevel & 8) {
                CMessageDisplayDialogue* pMDD = IENew CMessageDisplayDialogue();
                pMDD->eTarget = creTarget.id;
                pMDD->eSource = creTarget.id;
                pMDD->srOwner = creTarget.GetLongNameStrRef();
                pMDD->srText = srSuccessSave;
                pMDD->rgbOwner = g_ColorDefaultText;
                pMDD->rgbText = g_ColorDefaultText;
                pMDD->u1c = -1;
                pMDD->u20 = 0;
                pMDD->bFloatText = false;
                pMDD->u22 = 0;
                pMDD->bPlaySound = true;

                g_pChitin->messages.Send(*pMDD, FALSE);
            }
        }

        return FALSE;
    } else
        return TRUE;    // not saved
}

void DETOUR_CEffectList::DETOUR_TryDispel(
    CCreatureObject& creTarget,
    POSITION posSkip,
    BOOL bCheckDispellableFlag,
    BOOL bCheckProbability,
    char cRand,
    unsigned char cDispelLevel
) {
    BOOL bDispel = TRUE;
    int nDispelChance = 50;

    POSITION pos = GetHeadPosition();
    POSITION posCurr = pos;
    while (pos) {
        posCurr = pos;
        CEffect* pEff = (CEffect*)GetNext(pos);
        if (bCheckProbability) {
            nDispelChance = 50;
            if (pEff->effect.nSourceCreCasterLevel > cDispelLevel) {
                //original code without proper brackets
                //nDispelChance += pEff->effect.nSourceCreLevel * 10 - nDispelLevel;
                nDispelChance += (pEff->effect.nSourceCreCasterLevel - cDispelLevel) * 10;
            } else {
                nDispelChance -= (cDispelLevel - pEff->effect.nSourceCreCasterLevel) * 5;
            }
            if (cRand == 0) {
                bDispel = FALSE;
            } else {
                //bDispel = (cRand > nDispelChance || cRand > 99) ? 1 : 0;
                bDispel = (cRand > nDispelChance || cRand > 98) ? 1 : 0; //roll is 0-99, allow 99 for always dispel
            }
        } else {
            bDispel = TRUE;
        }

        if (posCurr != posSkip && bDispel) {
            if (!bCheckDispellableFlag || pEff->effect.dwFlags & 1) {
                RemoveAt(posCurr);
                pEff->OnRemove(creTarget);
                delete pEff;
                pEff = NULL;
            }
        }

    } //while

    this->posCurrent = 0;

    return;
}


BOOL static __stdcall
CGameEffect_CheckSelfPatchedEffect(CEffect& IteratorEff, CEffect& AbilityEff, BOOL bInitialChecks) {
    EffFileData& effect =     IteratorEff.effect;
    EffFileData& abil_effect = AbilityEff.effect;

    if (bInitialChecks == FALSE)     // Opcode or nParam1 mismatch
        return FALSE;                // jump to orig code

    if (effect.nParentItemSlot != 9) // not shield slot
        return FALSE;                // jump to orig code

    if (effect.nOpcode == 7 ||
        effect.nOpcode == 8 ||
        effect.nOpcode == 9 ||
        effect.nOpcode == 51 ||
        effect.nOpcode == 52 ) { 
            if (effect.nParam6 == AbilityEff.effect.nParam2) // tobex hack, unpatched nParam2 stored at nParam6
                return TRUE;        // return match
    }

    return FALSE;   // jump to orig code
}


CPP_MEMBERHOOK_1args(CGameEffectSetColor7, CGameEffectSetColor7_Apply,   0x5049AC, CCreatureObject *)
CPP_MEMBERHOOK_1args(CGameEffectSetColor8, CGameEffectSetColor8_Apply,   0x504AA1, CCreatureObject *)
CPP_MEMBERHOOK_1args(CGameEffectSetColor9, CGameEffectSetColor9_Apply,   0x504BF9, CCreatureObject *)
CPP_MEMBERHOOK_1args(CGameEffectSetColor51, CGameEffectSetColor51_Apply, 0x504D4C, CCreatureObject *)
CPP_MEMBERHOOK_1args(CGameEffectSetColor52, CGameEffectSetColor52_Apply, 0x504E8A, CCreatureObject *)


int
DETOUR_CGameEffectSetColor7::DETOUR_CGameEffectSetColor7_Apply(CCreatureObject *Cre) {
    EffFileData& effect = * (EffFileData*) (this + 4);
    if (effect.nParentItemSlot == 9) {          // shield slot
        if (effect.nParam5 != 0x5A) {           // tobex mark    
            effect.nParam6 = effect.nParam2;    // tobex hack, store unpatched nParam2 to nParam6
            effect.nParam5 = 0x5A;
        }
    }
    return CGameEffectSetColor7::CGameEffectSetColor7_Apply(Cre);
}

int
DETOUR_CGameEffectSetColor8::DETOUR_CGameEffectSetColor8_Apply(CCreatureObject *Cre) {
    EffFileData& effect = * (EffFileData*) (this + 4);
    if (effect.nParentItemSlot == 9) {          // shield slot
        if (effect.nParam5 != 0x5A) {           // tobex mark    
            effect.nParam6 = effect.nParam2;    // tobex hack, store unpatched nParam2 to nParam6
            effect.nParam5 = 0x5A;
        }
    }
    return CGameEffectSetColor8::CGameEffectSetColor8_Apply(Cre);
}

int
DETOUR_CGameEffectSetColor9::DETOUR_CGameEffectSetColor9_Apply(CCreatureObject *Cre) {
    EffFileData& effect = * (EffFileData*) (this + 4);
    if (effect.nParentItemSlot == 9) {          // shield slot
        if (effect.nParam5 != 0x5A) {           // tobex mark    
            effect.nParam6 = effect.nParam2;    // tobex hack, store unpatched nParam2 to nParam6
            effect.nParam5 = 0x5A;
        }
    }
    return CGameEffectSetColor9::CGameEffectSetColor9_Apply(Cre);
}

int
DETOUR_CGameEffectSetColor51::DETOUR_CGameEffectSetColor51_Apply(CCreatureObject *Cre) {
    EffFileData& effect = * (EffFileData*) (this + 4);
    if (effect.nParentItemSlot == 9) {          // shield slot
        if (effect.nParam5 != 0x5A) {           // tobex mark    
            effect.nParam6 = effect.nParam2;    // tobex hack, store unpatched nParam2 to nParam6
            effect.nParam5 = 0x5A;
        }
    }
    return CGameEffectSetColor51::CGameEffectSetColor51_Apply(Cre);
}

int
DETOUR_CGameEffectSetColor52::DETOUR_CGameEffectSetColor52_Apply(CCreatureObject *Cre) {
    EffFileData& effect = * (EffFileData*) (this + 4);
    if (effect.nParentItemSlot == 9) {          // shield slot
        if (effect.nParam5 != 0x5A) {           // tobex mark    
            effect.nParam6 = effect.nParam2;    // tobex hack, store unpatched nParam2 to nParam6
            effect.nParam5 = 0x5A;
        }
    }
    return CGameEffectSetColor52::CGameEffectSetColor52_Apply(Cre);
}


void __declspec(naked)
NonSchoolBonuses_asm() {
__asm
{
    ; eax - kit school
    mov     ecx, [ebp-4]                     ; effect
    cmp     [ecx]CEffect.effect.nSchool, 0   ; effect.school
    jz      nonschool_effect

    cmp     eax, [ecx]CEffect.effect.nSchool ; kit_school vs effect.school
                                             ; if Z=1, enable bonus
    ret

nonschool_effect:
    mov     al,1
    cmp     al,2            ; Z=0, no bonus
    ret
}
}


void __declspec(naked)
CGameEffect_Compare_BeforeRemove_asm() {
__asm
{
    push    ecx
	push    edx

    push    [ebp-4]     // bMatch
    push    [ebp-14h]   // ability effect
    push    [ebp+8]     // effect from list

    call    CGameEffect_CheckSelfPatchedEffect
    test    eax,eax     // 1 - match, 0 - not match

    pop     edx
	pop     ecx

    jz      CGameEffect_Compare_BeforeRemove_asm_NotMatch
    ret

CGameEffect_Compare_BeforeRemove_asm_NotMatch:
    mov     dword ptr [ebp-4], 0 // stolen bytes
    ret
}
}