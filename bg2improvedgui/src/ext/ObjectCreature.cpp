#include "ObjectCreature.h"

#include "animext.h"
#include "chitin.h"
#include "itmcore.h"
#include "objvef.h"
#include "vidcore.h"
#include "options.h"
#include "console.h"
#include "log.h"
#include "objtrig.h"

#include "InfGameCommon.h"
#include "ObjectCommon.h"
#include "ObjectStats.h"
#include "UserMageBook.h"
#include "UserPriestBook.h"

CCreatureObject& (CCreatureObject::*Tramp_CCreatureObject_Construct_10)(void*, unsigned int, BOOL, int, int, int, unsigned int, int, int, int) =
    SetFP(static_cast<CCreatureObject& (CCreatureObject::*)(void*, unsigned int, BOOL, int, int, int, unsigned int, int, int, int)>
                                                                            (&CCreatureObject::Construct),              0x87FB08);
CreFileKnownSpell* (CCreatureObject::*Tramp_CCreatureObject_GetKnownSpellPriest)(int, int) =
    SetFP(static_cast<CreFileKnownSpell* (CCreatureObject::*)(int, int)>    (&CCreatureObject::GetKnownSpellPriest),    0x8CB91F);
CreFileKnownSpell* (CCreatureObject::*Tramp_CCreatureObject_GetKnownSpellMage)(int, int) =
    SetFP(static_cast<CreFileKnownSpell* (CCreatureObject::*)(int, int)>    (&CCreatureObject::GetKnownSpellMage),      0x8CB949);
BOOL (CCreatureObject::*Tramp_CCreatureObject_AddMemSpellPriest)(int, int, int*) =
    SetFP(static_cast<BOOL (CCreatureObject::*)(int, int, int*)>            (&CCreatureObject::AddMemSpellPriest),      0x8CBB64);
BOOL (CCreatureObject::*Tramp_CCreatureObject_AddMemSpellMage)(int, int, int*) =
    SetFP(static_cast<BOOL (CCreatureObject::*)(int, int, int*)>            (&CCreatureObject::AddMemSpellMage),        0x8CBBEA);
void (CCreatureObject::*Tramp_CCreatureObject_ValidateAttackSequence)(char*) =
    SetFP(static_cast<void (CCreatureObject::*)(char*)>                     (&CCreatureObject::ValidateAttackSequence), 0x8D6D78);
BOOL (CCreatureObject::*Tramp_CCreatureObject_EvaluateStatusTrigger)(Trigger&) =
    SetFP(static_cast<BOOL (CCreatureObject::*)(Trigger&)>                  (&CCreatureObject::EvaluateStatusTrigger),        0x8F6C0E);
ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ExecuteAction)() =
    SetFP(static_cast<ACTIONRESULT (CCreatureObject::*)()>                  (&CCreatureObject::ExecuteAction),          0x8E2276);
short (CCreatureObject::*Tramp_CCreatureObject_GetProficiencyInItem)(CItem&) =
    SetFP(static_cast<short (CCreatureObject::*)(CItem&)>                   (&CCreatureObject::GetProficiencyInItem),   0x90C663);
ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ActionPickPockets)(CCreatureObject&) =
    SetFP(static_cast<ACTIONRESULT (CCreatureObject::*)(CCreatureObject&)>  (&CCreatureObject::ActionPickPockets),      0x9431AE);
ACTIONRESULT (CCreatureObject::*Tramp_CCreatureObject_ActionJumpToAreaEntranceMove)(IECString) =
    SetFP(static_cast<ACTIONRESULT (CCreatureObject::*)(IECString)>         (&CCreatureObject::ActionJumpToAreaEntranceMove), 0x953CE9);
void (CCreatureObject::*Tramp_CCreatureObject_UpdateFaceTalkerTimer)() =
    SetFP(static_cast<void (CCreatureObject::*)()>                          (&CCreatureObject::UpdateFaceTalkerTimer),  0x955CD7);
void (CCreatureObject::*Tramp_CCreatureObject_SetCurrentAction)(Action&) =
    SetFP(static_cast<void (CCreatureObject::*)(Action&)>                   (&CCreatureObject::SetCurrentAction),   0x8F97AA);

extern void RenderPortrait_SetCurrentAction(CCreatureObject&, short);

void DETOUR_CCreatureObject::DETOUR_SetCurrentAction(Action& action) {
    short prev_opcode = currentAction.opcode;
    (this->*Tramp_CCreatureObject_SetCurrentAction)(action);

    // remove multi-target items/spell swap exploit
    int targetcnt = this->lstTargetIds.GetCount();

    if (action.opcode == ACTION_USEITEM ||
        action.opcode == ACTION_USEITEM_POINT) {
        int slot = this->currentAction.m_specificID;
        if (slot != 0xFFFFFFFF) {   // sometimes slot is invalid
            CItem* Item = this->Inventory.items[slot];
            if (Item) {
                Item->Demand();
                int Ability = this->currentAction.m_specificID2;
                ItmFileAbility* ItmAbil = Item->GetAbility(Ability);
                if (ItmAbil) {
                    if (targetcnt > 1 && targetcnt != ItmAbil->targetCnt) {
                            this->lstTargetIds.RemoveAll();
                            console.writef("Multi-target spell/item swap exploit detected \n");
                    }
                }
                Item->Release();
            }
        }
    }

    if (action.opcode == ACTION_SPELL ||
        action.opcode == ACTION_SPELL_NO_DEC ||
        action.opcode == ACTION_SPELL_POINT ||
        action.opcode == ACTION_SPELL_POINT_NO_DEC ||
        action.opcode == ACTION_FORCESPELL ||
        action.opcode == ACTION_FORCESPELL_POINT ) {
        if (!action.sName1.IsEmpty()) {
            ResSplContainer Spell(action.sName1);
            Spell.Demand();
            SplFileAbility* pAbility = Spell.GetSpellAbility(0);
            if (pAbility) {
                if (targetcnt > 1 && targetcnt != pAbility->targetNum) {
                    this->lstTargetIds.RemoveAll();
                    console.writef("Multi-target spell/item swap exploit detected \n");
                }
            }
            Spell.Release();
        }
    }

    // render Action icon
    if (pGameOptionsEx)     // if tobex is alive
        if (pGameOptionsEx->bUI_ShowActionOnPortrait)
            RenderPortrait_SetCurrentAction(*this, prev_opcode);
}


#pragma optimize( "", off )  // disable inline optimization for CCreatureObject_Unmarshal() stack check
CCreatureObject& DETOUR_CCreatureObject::DETOUR_Construct(
    void* pFile,
    unsigned int dwSize,
    BOOL bHasSpawned,
    int nTicksTillRemoveIn,
    int nMaxMvtDistance,
    int nMaxMvtDistanceToObject,
    unsigned int nSchedule,
    int nDestX,
    int nDestY,
    int nFacing
) {
    memset(this, 0, 0x6774); // fill CCreatureObject memory with zeroes to avoid non-zero counters and other fields
    return (this->*Tramp_CCreatureObject_Construct_10)(pFile, dwSize, bHasSpawned, nTicksTillRemoveIn, nMaxMvtDistance, nMaxMvtDistanceToObject, nSchedule, nDestX, nDestY, nFacing);
}
#pragma optimize( "", on )


CreFileKnownSpell* DETOUR_CCreatureObject::DETOUR_GetKnownSpellPriest(int nLevel, int nIndex) {
    int Eip;
    GetEip(Eip);

    if (Eip == 0x78744E) { //from CScreenPriestBook::Update()
        if (this->o.Class == CLASS_SHAMAN)
            return (CreFileKnownSpell *)NULL;

        nIndex += CUIButtonPriestBook_KnownSpellOffset;
    }

    return (this->*Tramp_CCreatureObject_GetKnownSpellPriest)(nLevel, nIndex);
}

CreFileKnownSpell* DETOUR_CCreatureObject::DETOUR_GetKnownSpellMage(int nLevel, int nIndex) {
    int Eip;
    GetEip(Eip);

    if (Eip == 0x7B8952) { //from CScreenMageBook::Update()
        nIndex += CUIButtonMageBook_KnownSpellOffset;
    }

    return (this->*Tramp_CCreatureObject_GetKnownSpellMage)(nLevel, nIndex);
}

BOOL DETOUR_CCreatureObject::DETOUR_AddMemSpellPriest(int nLevel , int nIndex, int* pIndex) {
    int Eip;
    GetEip(Eip);

    if (Eip == 0x7892C9) { //from CUIButtonPriestBookKnownSpell::OnLClicked()
        nIndex += CUIButtonPriestBook_KnownSpellOffset;
    }

    return (this->*Tramp_CCreatureObject_AddMemSpellPriest)(nLevel, nIndex, pIndex);
}

BOOL DETOUR_CCreatureObject::DETOUR_AddMemSpellMage(int nLevel , int nIndex, int* pIndex) {
    int Eip;
    GetEip(Eip);

    if (Eip == 0x7BEEC7) { //from CUIButtonMageBookKnownSpell::OnLClicked()
        nIndex += CUIButtonMageBook_KnownSpellOffset;
    }

    return (this->*Tramp_CCreatureObject_AddMemSpellMage)(nLevel, nIndex, pIndex);
}

void DETOUR_CCreatureObject::DETOUR_ValidateAttackSequence(char* pSeq) {
    if (0) IECString("DETOUR_CCreatureObject::DETOUR_ValidateAttackSequence");

    if (*pSeq == SEQ_SHOOT) {
        CItem* pItm = Inventory.items[Inventory.nSlotSelected];
        if (pItm) {
            pItm->Demand();
            ItmFileAbility* ability = pItm->GetAbility(Inventory.nAbilitySelected);
            if (ability) {
                if (ability->attackType != ITEMABILITYATTACKTYPE_RANGED) *pSeq = SEQ_ATTACK;

                if (CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly(*this)) *pSeq = SEQ_ATTACK; //new line
            } else {
                *pSeq = SEQ_READY;
            }
            pItm->Release();
        } else {
            *pSeq = SEQ_READY;
        }
    } else if (*pSeq == SEQ_ATTACK) {
        CItem* pItm = Inventory.items[Inventory.nSlotSelected];
        if (pItm) {
            pItm->Demand();
            ItmFileAbility* ability = pItm->GetAbility(Inventory.nAbilitySelected);
            if (ability) {
                if (ability->attackType == ITEMABILITYATTACKTYPE_RANGED) *pSeq = SEQ_SHOOT;

                if (CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly(*this)) *pSeq = SEQ_ATTACK; //new line
            } else {
                *pSeq = SEQ_READY;
            }
            pItm->Release();
        } else {
            *pSeq = SEQ_READY;
        }
    }
    return;
}

void __stdcall CCreatureObject_GetClassAbilities(CCreatureObject& cre, unsigned char cClass, int nLevels, IECPtrList& cpl) {
    if (0) IECString("CCreatureObject_GetClassAbilities4");

    IECString sAbilName;
    IECString sSpell;
    CRuleTable* pRuleTable = NULL;
    CDerivedStats* pcds = NULL;
    unsigned short wStartLevel;
    unsigned short cSubclassLevel;

    if (cClass == CLASS_RANGER && cre.BaseStats.dwFlags & CREFLAG_FALLEN_RANGER) {
        pRuleTable = &g_pChitin->pGame->CLABRN05;
    }
    else if (cClass == CLASS_PALADIN && cre.BaseStats.dwFlags & CREFLAG_FALLEN_PALADIN) {
        pRuleTable = &g_pChitin->pGame->CLABPA05;
    } else {
        pRuleTable = &g_pChitin->pGame->GetClassAbilityTable(cClass, cre.BaseStats.kitLow | (cre.BaseStats.kitHigh << 16));
    }

    wStartLevel = 0;
    pcds = &cre.GetDerivedStats();
    cSubclassLevel = pcds->GetSubclassLevel(cre.o.GetClass(), cClass);
    if (nLevels != -1) {
        wStartLevel = cSubclassLevel - nLevels;
    }
    for (int nCol = wStartLevel; nCol < cSubclassLevel; nCol++) {
        if (nCol >= pRuleTable->nCols) break;
        for (int nRow = 0; nRow < pRuleTable->nRows; nRow++) {
            if (nCol < pRuleTable->nCols &&
                nRow < pRuleTable->nRows &&
                nCol >= 0 &&
                nRow >= 0) {
                sAbilName = *((pRuleTable->pDataArray) + (pRuleTable->nCols * nRow + nCol));
            } else {
                sAbilName = pRuleTable->defaultVal;
            }

            if (sAbilName.CompareNoCase(pRuleTable->GetDefaultValue())) {
                sSpell = sAbilName.Right(sAbilName.GetLength() - 3); //remove XX_ prefix
                sAbilName = sAbilName.Left(3); //XX_ prefix
                if (sAbilName.CompareNoCase("GA_") == 0) {
                    cpl.AddTail(new ResRef((LPCTSTR)sSpell));
                }
            }
        }
    }

    POSITION pos = cpl.GetHeadPosition();
    while (pos) {
        ResRef* pr = (ResRef*)cpl.GetNext(pos);
    }

    return;
}

void __stdcall CCreatureObject_GetClassAbilities(CCreatureObject& cre, CDerivedStats& cdsTarget, IECPtrList& cpl) {
    if (0) IECString("CCreatureObject_GetClassAbilities3");

    unsigned char cClass = cre.oBase.GetClass();
    switch (cClass) {
    case CLASS_MAGE:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_CLERIC:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_THIEF:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_THIEF,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_BARD:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_BARD,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_PALADIN:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_PALADIN,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_MAGE:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_CLERIC:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_THIEF:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_THIEF,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_MAGE_THIEF:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_THIEF,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_DRUID:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_DRUID,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_RANGER:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_RANGER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_MAGE_THIEF:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_THIEF,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_CLERIC_MAGE:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_CLERIC_THIEF:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_THIEF,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_DRUID:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_DRUID,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_FIGHTER_MAGE_CLERIC:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MAGE,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_CLERIC_RANGER:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_CLERIC,
            cre.cdsCurrent.GetClericClassLevel(cClass) - cdsTarget.GetClericClassLevel(cClass),
            cpl
        );
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_RANGER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_SORCERER:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_SORCERER,
            cre.cdsCurrent.GetMageClassLevel(cClass) - cdsTarget.GetMageClassLevel(cClass),
            cpl
        );
        break;
    case CLASS_MONK:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_MONK,
            cre.cdsCurrent.GetThiefClassLevel(cClass) - cdsTarget.GetThiefClassLevel(cClass),
            cpl
        );
        break;
    default:
        CCreatureObject_GetClassAbilities(
            cre,
            CLASS_FIGHTER,
            cre.cdsCurrent.GetFighterClassLevel(cClass) - cdsTarget.GetFighterClassLevel(cClass),
            cpl
        );
        break;
    }

    return;
}

void __stdcall CCreatureObject_JoinParty_UpdateClassAbilities(CCreatureObject& cre, CDerivedStats& cds) {
    if (0) IECString("CCreatureObject_JoinParty_UpdateClassAbilities");

    IECPtrList cplInnate;
    IECPtrList cplModify;
    POSITION pos;
    POSITION pos2;
    CreFileMemSpell* pMemSpell;

    //create a list of all used class abilities
    pos = cre.MemorizedSpellsInnate[0].GetHeadPosition();
    while (pos) {
        pMemSpell = (CreFileMemSpell*)cre.MemorizedSpellsInnate[0].GetNext(pos);
        if (!(pMemSpell->wFlags & CREMEMSPELL_MEMORIZED)) {
            cplInnate.AddTail(new ResRef(pMemSpell->name));
        }
    }

    //create a list of all class abilities to be modified
    CCreatureObject_GetClassAbilities(cre, cds, cplModify);

    //trim modify list to only include class abilities that need to be un-memorised
    pos = cplModify.GetHeadPosition();
    while (pos) {
        bool bMatch = false;

        POSITION posCurrent = pos;
        ResRef* pr = (ResRef*)cplModify.GetNext(pos);
        pos2 = cplInnate.GetHeadPosition();
        while (pos2) {
            POSITION posCurrent2 = pos2;
            ResRef* pr2 = (ResRef*)cplInnate.GetNext(pos2);
            if (*pr == *pr2) {
                bMatch = true;
                cplInnate.RemoveAt(posCurrent2);
                delete pr2;
                pr2 = NULL;
                break;
            }
        }
        if (pos2 == NULL && bMatch == false) {
            cplModify.RemoveAt(posCurrent);
            delete pr;
            pr = NULL;
        }
    }

    //vanilla code
    cre.RemoveNewSpecialAbilities(cds);
    cre.ApplyClassAbilities(cds, FALSE);

    //un-memorise previously used abilities
    pos = cplModify.GetHeadPosition();
    while (pos) {
        ResRef* prName = (ResRef*)cplModify.GetNext(pos);

        pos2 = cre.MemorizedSpellsInnate[0].GetHeadPosition();
        while (pos2) {
            pMemSpell = (CreFileMemSpell*)cre.MemorizedSpellsInnate[0].GetNext(pos2);
            if (pMemSpell->name == *prName && pMemSpell->wFlags & CREMEMSPELL_MEMORIZED) {
                pMemSpell->wFlags &= ~(CREMEMSPELL_MEMORIZED);
                break;
            }
        }
    }

    while (!cplInnate.IsEmpty()) delete (ResRef*)cplInnate.RemoveHead();
    while (!cplModify.IsEmpty()) delete (ResRef*)cplModify.RemoveHead();

    return;
}

BOOL DETOUR_CCreatureObject::DETOUR_EvaluateStatusTrigger(Trigger& t) {
    if (0) IECString("DETOUR_CCreatureObject::DETOUR_EvaluateTrigger");

    // pointless
    //Trigger tTemp = t;
    //switch (tTemp.opcode) {
    //default:
    //    return (this->*Tramp_CCreatureObject_EvaluateStatusTrigger)(t);
    //    break;
    //}
    //
    // return FALSE;

    return (this->*Tramp_CCreatureObject_EvaluateStatusTrigger)(t);
}

ACTIONRESULT DETOUR_CCreatureObject::DETOUR_ExecuteAction() {
    if (0) IECString("DETOUR_CCreatureObject::DETOUR_ExecuteAction");

    bInActionExecution = TRUE;
    ACTIONRESULT ar = ACTIONRESULT_DONE;

    if (currentAction.opcode == ACTION_BREAK_INSTANTS) SetCurrentAction(GetTopAction(g_pActionTemp));

    switch (currentAction.opcode) {
    case ACTION_BREAK_INSTANTS:
        ar = ACTIONRESULT_DONE; //re-implement here to prevent weird behaviour for double BreakInstants()
        break;
    default:
        return (this->*Tramp_CCreatureObject_ExecuteAction)();
        break;
    }

    nLastActionReturn = ar;
    bInActionExecution = FALSE;

    return ar;
}

short DETOUR_CCreatureObject::DETOUR_GetProficiencyInItem(CItem& itm) {
    short wStarsBG1 = 0;
    switch (itm.GetType()) {
    case ITEMTYPE_HAND:
        wStarsBG1 = 1;
        break;
    case ITEMTYPE_LARGE_SWORD:
        wStarsBG1 = GetProficiency(CREBG1PROF_LARGE_SWORD);
        break;
    case ITEMTYPE_DAGGER:
    case ITEMTYPE_SMALL_SWORD:
        wStarsBG1 = GetProficiency(CREBG1PROF_SMALL_SWORD);
        break;
    case ITEMTYPE_BOW:
        wStarsBG1 = GetProficiency(CREBG1PROF_BOW);
        break;
    case ITEMTYPE_SPEAR:
    case ITEMTYPE_HALBERD:
        wStarsBG1 = GetProficiency(CREBG1PROF_SPEAR);
        break;
    case ITEMTYPE_MACE:
    case ITEMTYPE_HAMMER:
    case ITEMTYPE_STAFF:
        wStarsBG1 = GetProficiency(CREBG1PROF_BLUNT);
        break;
    case ITEMTYPE_MORNING_STAR:
    case ITEMTYPE_FLAIL:
        wStarsBG1 = GetProficiency(CREBG1PROF_SPIKED);
        break;
    case ITEMTYPE_AXE:
        wStarsBG1 = GetProficiency(CREBG1PROF_AXE);
        break;
    case ITEMTYPE_SLING:
    case ITEMTYPE_DART:
    case ITEMTYPE_XBOW:
        wStarsBG1 = GetProficiency(CREBG1PROF_MISSILE);
        break;
    default:
        break;
    }
    
    short wStarsBG2 = 0;
    unsigned char cItmProfType = itm.GetProficiencyType();
    
    if (cItmProfType < 89 || cItmProfType > 134) { //outside standard proficiencies
        //if ( !cItmProfType ) wStarsBG2 = 1; //original code
        if (!cItmProfType) {
            switch (itm.GetType()) {
            case ITEMTYPE_ARROW:
            case ITEMTYPE_BOLT:
            case ITEMTYPE_BULLET:
                wStarsBG2 = 0;
                break;
            default:
                wStarsBG2 = 1;
            break;
            }
        }
    } else {
        wStarsBG2 = GetProficiency(cItmProfType);
    }

    if (wStarsBG1 > 0 || wStarsBG2 > 0)
        wStarsBG1 = max(wStarsBG1, wStarsBG2);
    
    if (wStarsBG1 < 0 &&
        g_pChitin->pGame->GetPartyMemberSlot(id) == ENUM_INVALID_INDEX)
        wStarsBG1 = 0;
    
    return wStarsBG1;
}

ACTIONRESULT DETOUR_CCreatureObject::DETOUR_ActionPickPockets(CCreatureObject& creTarget) {
    if (0) IECString("DETOUR_CCreatureObject::DETOUR_ActionPickPockets");

    if (!pGameOptionsEx->bActionPickpocketRemainHidden &&
        !pGameOptionsEx->bTriggerPickpocketFailed &&
        (!pGameOptionsEx->bEngineExternStealSlots || !pRuleEx->m_StealSlots.m_2da.bLoaded)
    ) {
        return (this->*Tramp_CCreatureObject_ActionPickPockets)(creTarget);
    }

    if (&creTarget == NULL ||
        creTarget.bScheduled == FALSE ||
        creTarget.bActive == FALSE ||
        creTarget.bFree == FALSE)
        return ACTIONRESULT_ERROR;

    if (this->GetDerivedStats().ButtonDisable[1]) {
        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_DISABLED_ARMOR, 0, 0, 0, -1, 0, IECString(""));
        return ACTIONRESULT_ERROR;
    }

    if (creTarget.o.MatchCriteria(this->o.GetOpposingEAObject(), FALSE, FALSE, FALSE)) {
        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_FAILED_HOSTILE, 0, 0, 0, -1, 0, IECString(""));
        return ACTIONRESULT_ERROR;
    }

    if (creTarget.bInStore) return ACTIONRESULT_ERROR;

    POINT ptTarget_tmp = {creTarget.currentLoc.x / PIXELS_PER_SEARCHMAP_PT_X, creTarget.currentLoc.y / PIXELS_PER_SEARCHMAP_PT_Y};
    POINT ptSource_tmp = {this->currentLoc.x / PIXELS_PER_SEARCHMAP_PT_X, this->currentLoc.y / PIXELS_PER_SEARCHMAP_PT_Y};
    int nDistance = GetLongestAxialDistance(ptSource_tmp, ptTarget_tmp);

    if (nDistance > 4 ||
        this->pArea->CheckPointsAccessible(creTarget.currentLoc, this->currentLoc, this->visibleTerrainTable, FALSE, this->GetVisualRange()) == FALSE
    ) {
        ACTIONRESULT ar = this->ActionMoveToObject(creTarget);
        if (ar == ACTIONRESULT_DONE) ar = ACTIONRESULT_SUCCESS;
        return ar;
    }

    if (!pGameOptionsEx->bActionPickpocketRemainHidden &&
        this->GetDerivedStats().stateFlags & STATE_INVISIBLE) {
        ITEM_EFFECT eff;
        CEffect::CreateItemEffect(eff, 0x88);
        eff.timing = TIMING_INSTANT_PERMANENT;

        POINT ptDest = {-1, -1};

        CEffect& ceff = CEffect::CreateEffect(eff, this->currentLoc, this->id, ptDest, ENUM_INVALID_INDEX);

        CMessageApplyEffect* pMsg = IENew CMessageApplyEffect();
        pMsg->eTarget = this->id;
        pMsg->eSource = this->id;
        pMsg->pCEffect = &ceff;
        pMsg->u10 = 0;
        g_pChitin->messages.Send(*pMsg, FALSE);
    }

    int nDieRoll = IERand(100) + 1;

    if (nDieRoll == 100 ||
        nDieRoll >= this->GetDerivedStats().pickPockets - creTarget.GetDerivedStats().pickPockets ||
        creTarget.GetDerivedStats().pickPockets == 0x0FF
    ) {
        //failed
        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_FAILED, 0, 0, 0, -1, 0, IECString(""));

        if (!pGameOptionsEx->bTriggerPickpocketFailedOnly) {
            Trigger tAttackedBy(TRIGGER_ATTACKED_BY, this->o, 0);
            CMessageSetTrigger* pMsgST = IENew CMessageSetTrigger();
            pMsgST->eTarget = creTarget.id;
            pMsgST->eSource = this->id;
            pMsgST->t = tAttackedBy;
            g_pChitin->messages.Send(*pMsgST, FALSE);
        }

        if (pGameOptionsEx->bTriggerPickpocketFailed ||
            pGameOptionsEx->bTriggerPickpocketFailedOnly) {
            Trigger tPickpocketFailed(TRIGGER_PICKPOCKET_FAILED, this->o, 0);
            CMessageSetTrigger* pMsgST2 = IENew CMessageSetTrigger();
            pMsgST2->eTarget = creTarget.id;
            pMsgST2->eSource = this->id;
            pMsgST2->t = tPickpocketFailed;
            g_pChitin->messages.Send(*pMsgST2, FALSE);
        }

        if (pGameOptionsEx->bTriggerPickpocketFailedOnly) {
            CMessageFaceTalker* pMsgFT = IENew CMessageFaceTalker();
            pMsgFT->eTarget = creTarget.id;
            pMsgFT->eSource = creTarget.id;
            pMsgFT->eTalker = ENUM_INVALID_INDEX;
            pMsgFT->nTicks = 0;
            g_pChitin->messages.Send(*pMsgFT, FALSE);
        }

        if (pGameOptionsEx->bActionPickpocketRemainHidden &&
            this->GetDerivedStats().stateFlags & STATE_INVISIBLE) {
            ITEM_EFFECT eff;
            CEffect::CreateItemEffect(eff, 0x88);

            POINT dest = {-1, -1};

            CEffect& ceff = CEffect::CreateEffect(eff, this->currentLoc, this->id, dest, ENUM_INVALID_INDEX);

            CMessageApplyEffect* pcmAE = IENew CMessageApplyEffect();
            pcmAE->eTarget = this->id;
            pcmAE->eSource = this->id;
            pcmAE->pCEffect = &ceff;
            pcmAE->u10 = 0;
            g_pChitin->messages.Send(*pcmAE, FALSE);
        }

        if (this->o.EnemyAlly <= EA_GOODCUTOFF) {
            if (creTarget.InDialogAction() ||
                creTarget.bInDialogue) {
                CMessageInterruptDialogue* pMsgID = IENew CMessageInterruptDialogue();
                pMsgID->eTarget = creTarget.id;
                pMsgID->eSource = this->id;
                g_pChitin->messages.Send(*pMsgID, FALSE);
            }
        }

        return ACTIONRESULT_DONE;
    }

    BOOL bNotStealableArray[39]; //FIX_ME, should use a global variable
    for (int iSlotIdx = 0; iSlotIdx < *g_pInventorySlots; iSlotIdx++) {
        bNotStealableArray[iSlotIdx] = 0;
    }

    if (pGameOptionsEx->bEngineExternStealSlots && pRuleEx->m_StealSlots.m_2da.bLoaded) {
        for (int iRow = 0; iRow < *g_pInventorySlots; iRow++) {
            if (iRow == SLOT_FIST) {
                bNotStealableArray[iRow] = 1;
                continue;
            }

            if (iRow >= pRuleEx->m_StealSlots.nRows) break;
            int nCol = 0;
            if (nCol >= pRuleEx->m_StealSlots.nCols) break;

            IECString sValue = *((pRuleEx->m_StealSlots.pDataArray) + (pRuleEx->m_StealSlots.nCols * iRow + nCol));
            int nValue = atoi((LPCTSTR)sValue);
            if (nValue == 0 || nValue > this->GetDerivedStats().pickPockets) bNotStealableArray[iRow] = 1;
        }

        IECString sRowName("SLOT_EQUIPPED");
        IECString sColName("SKILL");
        IECString sValue(pRuleEx->m_StealSlots.GetValue(sColName, sRowName));
        int nValue = atoi((LPCTSTR)sValue);

        if (nValue == 0 || nValue > this->GetDerivedStats().pickPockets) {
            bNotStealableArray[creTarget.Inventory.nSlotSelected] = 1;

            short nSlotEquippedLauncher = creTarget.GetSlotOfEquippedLauncherOfAmmo(creTarget.Inventory.nSlotSelected, creTarget.Inventory.nAbilitySelected);
            if (nSlotEquippedLauncher != -1) bNotStealableArray[nSlotEquippedLauncher] = 1;
        }

    } else {
        bNotStealableArray[SLOT_FIST] = 1;
        bNotStealableArray[SLOT_ARMOR] = 1;
        bNotStealableArray[SLOT_BELT] = 1;
        bNotStealableArray[SLOT_BOOTS] = 1;
        bNotStealableArray[SLOT_CLOAK] = 1;
        bNotStealableArray[SLOT_GAUNTLETS] = 1;
        bNotStealableArray[SLOT_HELMET] = 1;
        bNotStealableArray[SLOT_SHIELD] = 1;
        bNotStealableArray[creTarget.Inventory.nSlotSelected] = 1;

        short nSlotEquippedLauncher = creTarget.GetSlotOfEquippedLauncherOfAmmo(creTarget.Inventory.nSlotSelected, creTarget.Inventory.nAbilitySelected);
        if (nSlotEquippedLauncher != -1) bNotStealableArray[nSlotEquippedLauncher] = 1;
    }

    IECString sNameTarget(creTarget.szScriptName);
    short nSlotToStealFirst = IERand(*g_pInventorySlots);
    short nSlotToSteal = nSlotToStealFirst;
    while (bNotStealableArray[nSlotToSteal] ||
        creTarget.Inventory.items[nSlotToSteal] == NULL ||
        creTarget.Inventory.items[nSlotToSteal]->dwFlags & CREITEM_UNSTEALABLE ||
        !(creTarget.Inventory.items[nSlotToSteal]->GetFlags() & ITEMFLAG_DROPPABLE) ||
        creTarget.Inventory.items[nSlotToSteal]->dwFlags & CREITEM_UNDROPPABLE ||
        (sNameTarget.CompareNoCase("MINSC") == 0 && creTarget.Inventory.items[nSlotToSteal]->itm.name == "MISC84") ||
        (sNameTarget.CompareNoCase("ALORA") == 0 && creTarget.Inventory.items[nSlotToSteal]->itm.name == "MISC88") ||
        (sNameTarget.CompareNoCase("EDWIN") == 0 && creTarget.Inventory.items[nSlotToSteal]->itm.name == "MISC89") ||
        creTarget.Inventory.items[nSlotToSteal]->GetType() == ITEMTYPE_CONTAINER
    ) {
        if (nSlotToStealFirst % 2) {
            nSlotToSteal--;
            if (nSlotToSteal < 0) nSlotToSteal = static_cast<short>(*g_pInventorySlots) - 1;
        } else {
            nSlotToSteal++;
            if (nSlotToSteal >= static_cast<short>(*g_pInventorySlots)) nSlotToSteal = 0;
        }

        if (nSlotToSteal == nSlotToStealFirst) {
            PrintEventMessage(EVENTMESSAGE_PICKPOCKET_NO_ITEMS, 0, 0, 0, -1, 0, IECString(""));
            return ACTIONRESULT_DONE;
        }
    }

    if (IERand(*g_pRandStealGoldChance) == 0 &&
        g_pChitin->pGame->GetPartyMemberSlot(this->id) == -1) {
        int nGoldToSteal = IERand(*g_pRandGoldToSteal);
        CMessageModifyPartyGold* pMsgMPG = IENew CMessageModifyPartyGold();
        pMsgMPG->eTarget = this->id;
        pMsgMPG->eSource = this->id;
        pMsgMPG->nGold = nGoldToSteal;
        pMsgMPG->cMode = 1; //sum
        pMsgMPG->bPrintMessage = true;
        g_pChitin->messages.Send(*pMsgMPG, FALSE);

        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_SUCCESS, 0, 0, 0, -1, 0, IECString(""));
        return ACTIONRESULT_DONE;
    }

    CItem* pItemToSteal = creTarget.Inventory.items[nSlotToSteal];
    if (g_pChitin->pGame->GetPartyMemberSlot(this->id) == -1) {
        //non-party
        for (int iSlotIdx = 0; iSlotIdx < 20; iSlotIdx++) {
            if (this->Inventory.items[iSlotIdx + SLOT_MISC0] == NULL) {
                CItem* pItem = IENew CItem(*pItemToSteal);
                this->Inventory.items[iSlotIdx + SLOT_MISC0] = pItem;

                CMessageRemoveItem* pMsgRI = IENew CMessageRemoveItem();
                pMsgRI->eTarget = creTarget.id;
                pMsgRI->eSource = this->id;
                pMsgRI->wSlot = nSlotToSteal;
                g_pChitin->messages.Send(*pMsgRI, FALSE);

                PrintEventMessage(EVENTMESSAGE_PICKPOCKET_SUCCESS, 0, 0, 0, -1, 0, IECString(""));
                return ACTIONRESULT_DONE;
            }
        }

        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_INV_FULL, 0, 0, 0, -1, 0, IECString(""));
        return ACTIONRESULT_ERROR;

    } else {
        //party
        for (int iSlotIdx = 0; iSlotIdx < *g_pNumInventorySlots; iSlotIdx++) {
            if (this->Inventory.items[iSlotIdx + SLOT_MISC3] == NULL) {
                CItem* pItem = IENew CItem(*pItemToSteal);
                this->Inventory.items[iSlotIdx + SLOT_MISC3] = pItem;

                CMessageRemoveItem* pMsgRI = IENew CMessageRemoveItem();
                pMsgRI->eTarget = creTarget.id;
                pMsgRI->eSource = this->id;
                pMsgRI->wSlot = nSlotToSteal;
                g_pChitin->messages.Send(*pMsgRI, FALSE);

                PrintEventMessage(EVENTMESSAGE_PICKPOCKET_SUCCESS, 0, 0, 0, -1, 0, IECString(""));
                return ACTIONRESULT_DONE;
            }
        }

        PrintEventMessage(EVENTMESSAGE_PICKPOCKET_INV_FULL, 0, 0, 0, -1, 0, IECString(""));
        return ACTIONRESULT_ERROR;
    }
}

ACTIONRESULT DETOUR_CCreatureObject::DETOUR_ActionJumpToAreaEntranceMove(IECString sArea) {
    this->pArea->m_bSaved = false; //re-marshal the previous area 
    ACTIONRESULT ar = (this->*Tramp_CCreatureObject_ActionJumpToAreaEntranceMove)(sArea);
    //not the area after the jump
    return ar;
}


void DETOUR_CCreatureObject::DETOUR_UpdateFaceTalkerTimer() {
    if (nFaceTalkerTimer > 0) {
        POSITION pos = pendingTriggers.GetHeadPosition();
        while (pos) {
            Trigger* pt = (Trigger*)pendingTriggers.GetNext(pos);
            if (pt->opcode == TRIGGER_ATTACKED_BY ||
                pt->opcode == TRIGGER_HIT_BY) {
                nFaceTalkerTimer = 0;
                nFaceTalkerId = ENUM_INVALID_INDEX;
                break;
            }
        }
    }
    return;
}

void __stdcall CCreatureObject_PrintExtraCombatInfoText(CCreatureObject& creSelf, IECString& sText) {
    if (0) IECString("CCreatureObject_PrintExtraCombatInfoText");

    if (g_pChitin->pGame->m_GameOptions.m_bDisplayExtraCombatInfo) {
        ABGR colorOwner = *(g_pColorRangeArray + creSelf.BaseStats.colors.colorMajor);
        ABGR colorText = g_ColorDefaultText;
        IECString sOwner = creSelf.GetLongName();
        g_pChitin->pScreenWorld->PrintToConsole(sOwner, sText, colorOwner, colorText, -1, 0);
    }
    return;
}

BOOL __stdcall CCreatureObject_ShouldAvertCriticalHit(CCreatureObject& creTarget, CCreatureObject& creSource) {
    if (0) IECString("CCreatureObject_ShouldAvertCriticalHit");

    CItem* pItem = NULL;
    BOOL   result = FALSE;

    for (int i = 0; i < 10; i++) {
        pItem = creTarget.Inventory.items[i];
        if (pItem) {
            if (i != SLOT_HELMET &&
                (pItem->GetFlags() & ITEMFLAG_TOGGLE_CRITICALHIT)) {
                //avert critical hit if flag set (not SLOT_HELMET)
                result = TRUE;
                break;
            }
            if (i == SLOT_HELMET &&
                !(pItem->GetFlags() & ITEMFLAG_TOGGLE_CRITICALHIT)) {
                //avert critical hit unless flag set (SLOT_HELMET)
                result = TRUE;
                break;
            }
        }
    }

    pItem = creTarget.Inventory.items[creTarget.Inventory.nSlotSelected];
    if (pItem) {
        if (pItem->GetFlags() & ITEMFLAG_TOGGLE_CRITICALHIT) {
            result = TRUE;
        }
    }

    if (result != TRUE) {
        CConditionalSpellList& List = creSource.cdsCurrent.m_ConditionalSpells;
        POSITION pos = List.GetHeadPosition();

        while (pos != NULL) {
            CConditionalSpell *Node = (CConditionalSpell *) List.GetNext(pos);
            if (Node->t.opcode == TRIGGER_CRITICALHIT) {
                creSource.CastSpell(Node->rResource1, creTarget, TRUE, 0x9049, NULL, TRUE, FALSE);
            }
        }
    }

    return result;
}


void __stdcall
CCreatureObject_ShouldAvertCriticalHit_SpellOnly(CCreatureObject& creTarget, CCreatureObject& creSource) {
    CConditionalSpellList& List = creSource.cdsCurrent.m_ConditionalSpells;
    POSITION pos = List.GetHeadPosition();

    while (pos != NULL) {
        CConditionalSpell *Node = (CConditionalSpell *) List.GetNext(pos);
        if (Node->t.opcode == TRIGGER_CRITICALHIT) {
            creSource.CastSpell(Node->rResource1, creTarget, TRUE, 0x9049, NULL, TRUE, FALSE);
        }
    }
}


BOOL __stdcall CCreatureObject_ApplyDamage_TryBackstab(CCreatureObject& creSource, CItem& itmMain, ItmFileAbility& abilMain, ItmFileAbility& abilLauncher, short orTarget, short orToTarget, CCreatureObject& creTarget) {
    if (0) IECString("CCreatureObject_ApplyDamage_TryBackstab");

    /* original code
    CDerivedStats& cds = creSource.GetDerivedStats();

    if ((abilMain.attackType != ITEMABILITYATTACKTYPE_RANGED) &&
        (cds.stateFlags & STATE_INVISIBLE || cds.m_BackstabEveryHit) &&
        cds.m_BackstabDamageMultiplier != 1) {

        if (creSource.o.EnemyAlly > EA_GOODCUTOFF || //not good
            cds.m_BackstabEveryHit ||
            ObjectCommon_InBackstabPosition(orTarget, orToTarget)) {
            
            if (creTarget.GetKitUnusableFlag() & KIT_BARBARIAN) {
                creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_FAIL, 0, 0, 0, -1, FALSE, IECString());
                return FALSE;
            }

            if (creTarget.GetDerivedStats().m_BackstabImmunity) {
                creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_FAIL, 0, 0, 0, -1, FALSE, IECString());
                return FALSE;
            }

            if (!(itmMain.GetUnusableFlags() & ITEMUNUSABLE_THIEF)) {
                //success, game prints event message and calculates multiplier
                return TRUE;
            } else {
                creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_WEAPON_UNSUITABLE, 0, 0, 0, -1, FALSE, IECString());
                return FALSE;
            }
        }
    }

    return FALSE;*/
    
    CDerivedStats& cds = creSource.GetDerivedStats();

    BOOL bToggleBackstab = FALSE;
    BOOL bIgnoreInvisible = cds.bBackstabEveryHit;
    BOOL bIgnorePosition = cds.bBackstabEveryHit;

    if (pGameOptionsEx->bItemsBackstabRestrictionsConfig) {
        bToggleBackstab = abilMain.flags & ITEMABILITYFLAG_TOGGLE_BACKSTAB;
        if (&abilLauncher != NULL && !bToggleBackstab) {
            bToggleBackstab = abilLauncher.flags & ITEMABILITYFLAG_TOGGLE_BACKSTAB;
        }
    }

    if (pGameOptionsEx->bEffBackstabEveryHitConfig) {
        bIgnoreInvisible = (cds.bBackstabEveryHit & 0x1) || (cds.bBackstabEveryHit & 0x2);
        bIgnorePosition = (cds.bBackstabEveryHit & 0x1) || (cds.bBackstabEveryHit & 0x4);
    }

    if (
        (cds.stateFlags & STATE_INVISIBLE || bIgnoreInvisible) &&
        cds.BackstabDamageMultiplier != 1 &&
        (creSource.o.EnemyAlly > EA_GOODCUTOFF || bIgnorePosition || ObjectCommon_InBackstabPosition(orTarget, orToTarget))
    ) {

        if (creTarget.GetKitUnusableFlag() & KIT_BARBARIAN) {
            creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_FAIL, 0, 0, 0, -1, FALSE, IECString());
            return FALSE;
        }

        if (creTarget.GetDerivedStats().bBackstabImmunity) {
            creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_FAIL, 0, 0, 0, -1, FALSE, IECString());
            return FALSE;
        }

        if (
            (!(itmMain.GetUnusableFlags() & ITEMUNUSABLE_THIEF) && abilMain.attackType != ITEMABILITYATTACKTYPE_RANGED && !bToggleBackstab) ||
            (!(itmMain.GetUnusableFlags() & ITEMUNUSABLE_THIEF) && abilMain.attackType == ITEMABILITYATTACKTYPE_RANGED && bToggleBackstab) ||
            (itmMain.GetUnusableFlags() & ITEMUNUSABLE_THIEF && bToggleBackstab)
        ) {
            //success, game prints event message and calculates multiplier
            return TRUE;
        } else {
            creSource.PrintEventMessage(EVENTMESSAGE_BACKSTAB_WEAPON_UNSUITABLE, 0, 0, 0, -1, FALSE, IECString());
            return FALSE;
        }
    }

    return FALSE;
}

int __stdcall CCreatureObject_ApplyDamage_CalculateDamageBonus(CCreatureObject& creSource, ItmFileAbility& abilMain, short* pwDamage, CCreatureObject& creTarget) {
    if (0) IECString("CCreatureObject_ApplyDamage_CalculateDamageBonus");

    short wDamageBonus = 0;

    switch (abilMain.damType) {
    case 1: //piercing
        wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS) / 100;
        break;
    case 2: //crushing
        wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CRUSHINGDAMAGEBONUS) / 100;
        break;
    case 3: //slashing
        wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_SLASHINGDAMAGEBONUS) / 100;
        break;
    case 4: //missile
        wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_MISSILEDAMAGEBONUS) / 100;
        break;
    case 5: //stunning
        wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_STUNNINGDAMAGEBONUS) / 100;
        break;
    case 6: //piercing/crushing
        if (&creTarget != NULL) {
            if (creTarget.GetDerivedStats().resistPiercing > creTarget.GetDerivedStats().resistCrushing) {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CRUSHINGDAMAGEBONUS) / 100;
            } else {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS) / 100;
            }
        } else {
            wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS) / 100;
        }
        break;
    case 7: //piercing/slashing
        if (&creTarget != NULL) {
            if (creTarget.GetDerivedStats().resistPiercing > creTarget.GetDerivedStats().resistSlashing) {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_SLASHINGDAMAGEBONUS) / 100;
            } else {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS) / 100;
            }
        } else {
            wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS) / 100;
        }
        break;
    case 8: //crushing/slashing
        if (&creTarget != NULL) {
            if (creTarget.GetDerivedStats().resistCrushing > creTarget.GetDerivedStats().resistSlashing) {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CRUSHINGDAMAGEBONUS) / 100;
            } else {
                wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_SLASHINGDAMAGEBONUS) / 100;
            }
        } else {
            wDamageBonus = *pwDamage * creSource.GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CRUSHINGDAMAGEBONUS) / 100;
        }
        break;
    default:
        break;
    }

    wDamageBonus += creSource.GetDerivedStats().damageBonus;
    *pwDamage += wDamageBonus;
    return wDamageBonus;
}

BOOL __stdcall CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly(CCreatureObject& cre) {
    if (0) IECString("CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly");

    CItem* pItm = cre.Inventory.items[cre.Inventory.nSlotSelected];
    if (pItm == NULL) return FALSE;
    pItm->Demand();
    ItmFileAbility* ability = pItm->GetAbility(cre.Inventory.nAbilitySelected);
    if (ability == NULL) {
        pItm->Release();
        return FALSE;
    }

    short nSlot;
    if (ability->attackType == ITEMABILITYATTACKTYPE_RANGED &&
        cre.Inventory.nSlotSelected >= SLOT_WEAPON0 &&
        cre.Inventory.nSlotSelected <= SLOT_WEAPON3 &&
        pItm->GetType() != ITEMTYPE_BOW &&
        pItm->GetType() != ITEMTYPE_SLING &&
        pItm->GetType() != ITEMTYPE_XBOW &&
        cre.GetFirstEquippedLauncherOfAbility(ability, nSlot) == NULL) {
        
        //restrict to humanoid animation IDs only (this is Infinity Animations-friendly)
        CAnimation* pAnimation = cre.animation.pAnimation;
        if (pAnimation == NULL) {
            LPCTSTR lpsz = "CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly(): cre.animation.pAnimation == NULL\r\n";
            L.timestamp();
            L.append(lpsz);
            console.write(lpsz);
        } else {
            if (
                (pAnimation->wAnimId >= 0x5000 &&
                pAnimation->wAnimId < 0x5400 &&
                ((CAnimation5000*)pAnimation)->sPrefix[0] == 'C' //avoid clash with Infinity Animations
                ) ||
                pAnimation->wAnimId & 0x6000
                ) {
                pItm->Release();
                return TRUE;
            }
        }

    }
    pItm->Release();
    return FALSE;
};

BOOL __stdcall CCreatureObject_Spell_IsOverrideInvisible(CCreatureObject& creSource, CCreatureObject& creTarget) {
    if (0) IECString("CCreatureObject_Spell_IsOverrideInvisible");

    if (&creSource == &creTarget) return TRUE;

    IECString sSpell;
    if (creSource.currentAction.GetSName1().IsEmpty()) {
        creSource.GetSpellIdsName(creSource.currentAction.m_specificID, &sSpell);
    } else {
        sSpell = creSource.currentAction.GetSName1();
    }
    
    ResSplContainer resSpell;
    resSpell.SetResRef(ResRef(sSpell), TRUE, TRUE);
    resSpell.Demand();
    if (resSpell.pRes) {
        if (resSpell.GetSpellFlags() & SPELLFLAG_TARGET_INVISIBLE) {
            resSpell.Release();
            return TRUE;
        }
    }
    resSpell.Release();

    if (!creSource.CanSeeInvisible()) {
        if (creTarget.GetDerivedStats().stateFlags & STATE_INVISIBLE ||
            creTarget.GetDerivedStats().stateFlags & STATE_IMPROVEDINVISIBILITY) {
            return FALSE;
        }
    }

    if (!creSource.CanSeeInvisible()) {
        if (creTarget.GetDerivedStats().sanctuary) {
            return FALSE;
        }
    }

    return TRUE;
};

BOOL __stdcall CCreatureObject_IsDeadInFrontVerticalList(CCreatureObject& cre) {
    if (0) IECString("CCreatureObject_IsDeadInFrontVerticalList");

    if (cre.GetVertListType() == LIST_FRONT) {
        CDerivedStats& cds = cre.GetDerivedStats();
        if (cds.stateFlags & STATE_DEAD) {
            CAnimation* pAnimation = cre.animation.pAnimation;
            if (pAnimation == NULL) {
                LPCSTR lpsz = "CCreatureObject_IsDeadInFrontVerticalList(): pAnimation == NULL\r\n";
                L.timestamp();
                L.append(lpsz);
                console.write(lpsz);
                return FALSE;
            }
            if (!pAnimation->CanUseMiddleVertList()) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL __stdcall CCreatureObject_Spell_IsOverrideSilence(CCreatureObject& creSource) {
    //called when creSource has STATE_SILENCED set

    if (0) IECString("CCreatureObject_Spell_IsOverrideSilence");

    IECString sSpell;
    if (creSource.currentAction.GetSName1().IsEmpty()) {
        creSource.GetSpellIdsName(creSource.currentAction.m_specificID, &sSpell);
    } else {
        sSpell = creSource.currentAction.GetSName1();
    }

    if (!sSpell.Compare("SPWI219")) return TRUE; //Vocalize
    if (!sSpell.Compare("SPCL412")) return TRUE; //Set Snare
    if (!sSpell.Compare("SPCL414")) return TRUE; //Set Special Snare
    if (!sSpell.Compare("SPIN649")) return TRUE; //Pocket Plane
    
    ResSplContainer resSpell;
    resSpell.SetResRef(ResRef(sSpell), TRUE, TRUE);
    resSpell.Demand();
    if (resSpell.pRes) {
        if (resSpell.GetSpellFlags() & SPELLFLAG_NO_VOICE) {
            resSpell.Release();
            return TRUE;
        }
    }
    resSpell.Release();

    return FALSE;
}

LPCTSTR __stdcall CCreatureObject_DoSpellCasting_GetGenderLetter(CCreatureObject& creSource, ResSplContainer& resSpell, SplFileAbility& ability) {
    if (0) IECString("CCreatureObject_DoSpellCasting_GetGenderLetter");

    if (pGameOptionsEx->bSpellsUnvoicedConfig &&
        creSource.GetDerivedStats().stateFlags & STATE_SILENCED &&
        resSpell.GetSpellFlags() & SPELLFLAG_NO_VOICE)
        return "S";

    if (pGameOptionsEx->bSpellsCastingFix) {
        unsigned char gender = creSource.o.Gender;
        unsigned char sex = creSource.BaseStats.sex;

        if (ability.castSpeed - creSource.GetDerivedStats().mentalSpeed < 3) return "S";

        switch (gender) {
        case GENDER_MALE:
            return "M";
            break;
        case GENDER_FEMALE:
            return "F";
            break;
        case GENDER_OTHER:
        case GENDER_NIETHER:
            return "S";
            break;
        default:
            if (sex == 1) return "M";
            if (sex == 2) return "F";
            return "S";
            break;
        }
    } else {
        //original code
        if (ability.castSpeed < 3) return "S";

        unsigned char gender = creSource.o.Gender;
        if (gender == GENDER_FEMALE) return "F";
        if (gender == GENDER_OTHER ||
            gender == GENDER_NIETHER)
            return "S";

        return "M";
    }
}

short __stdcall CCreatureObject_DoSpellCasting_GetCastingSpeed(CCreatureObject& creSource, SplFileAbility& ability) {
    if (0) IECString("CCreatureObject_DoSpellCasting_GetCastingSpeed");

    return max(0, ability.castSpeed - creSource.GetDerivedStats().mentalSpeed) * 100 / 10;
}

BOOL __stdcall CCreatureObject_UseItem_CannotTargetInvisible(CCreatureObject& creSource, CCreatureObject& creTarget) {
    if (0) IECString("CCreatureObject_UseItem_CannotTargetInvisible");

    if (&creSource == &creTarget) return FALSE;

    ItmFileAbility* itmAbility = creSource.currentItem->GetAbility(creSource.currentItemAbility);
    if (itmAbility &&
        itmAbility->flags & ITEMFLAG_NO_TARGET_INVIS) {
        if (!creSource.CanSeeInvisible()) {
            if (creTarget.GetDerivedStats().stateFlags & STATE_INVISIBLE ||
                creTarget.GetDerivedStats().stateFlags & STATE_IMPROVEDINVISIBILITY ||
                creTarget.GetDerivedStats().sanctuary) {
                creSource.PrintEventMessage(EVENTMESSAGE_SPELLFAILED_INVISIBLE, 0, 0, 0, -1, 0, IECString());
                return TRUE;
            }
        }
    }

    return FALSE;
}

void __stdcall CCreatureObject_UseItem_OverrideAnimation(CCreatureObject& creSource) {
    if (0) IECString("CCreatureObject_UseItem_OverrideAnimation");

    IECString sRowname(creSource.currentItem->itm.name.GetResRefNulled());

    creSource.currentItem->Demand();
    int nCol = 0;
    if (creSource.currentItemAbility != 0 &&
        creSource.currentItem->GetAbility(creSource.currentItemAbility) != NULL) {
        nCol = creSource.currentItemAbility;
    }
    creSource.currentItem->Release();

    IECString sColname = "SEQUENCE";
    if (nCol &&
        nCol < g_pChitin->pGame->ITEMANIM.nCols) {
        sColname.Format("%d", nCol);
    }

    IECString sSeq = g_pChitin->pGame->ITEMANIM.GetValue(sColname, sRowname);

    int nSeq;
    if (sSeq.Compare((LPCTSTR)g_pChitin->pGame->ITEMANIM.defaultVal)) {
        sscanf_s((LPCTSTR)sSeq, "%d", &nSeq);

        CMessageSetAnimationSequence* pMsg = IENew CMessageSetAnimationSequence();
        pMsg->eSource = creSource.id;
        pMsg->eTarget = creSource.id;
        pMsg->nSeq = nSeq;
        g_pChitin->messages.Send(*pMsg, FALSE);
    }

    return;
}

BOOL __stdcall CCreatureObject_AttackOnce_DoHalfAttack(CCreatureObject& creSource, char cInRoundIdx) {
    if (0) IECString("CCreatureObject_AttackOnce_DoHalfAttack");

    const char cInRoundOffset = 9;
    int nthAttack = cInRoundIdx - 9;
    CDerivedStats* pcds = &creSource.GetDerivedStats();
    int nAttacks = pcds->numAttacks - 5;

    if (nthAttack < nAttacks) { //full attack
        creSource.bIsAttacking = TRUE;
        creSource.bStatisticalAttack = true;
        if (!pGameOptionsEx->bActionAttackOnceFix) creSource.wDoHalfAttack = 0; //original code
        
        if (nthAttack == nAttacks - 1) { //last full attack
            creSource.bLeftWeaponAttack = TRUE;
        }
        if (nAttacks == 2) creSource.bLeftWeaponAttack = FALSE;
    } else {
        if (nthAttack == nAttacks) { //half attack
            if (!pGameOptionsEx->bActionAttackOnceFix) { //original code
                creSource.bIsAttacking = TRUE;
                creSource.bStatisticalAttack = true;
                creSource.bLeftWeaponAttack = FALSE;
                creSource.wDoHalfAttack = 1;
                if (IERand(2) == 0) {   // 0-1
                    creSource.bStatisticalAttack = TRUE;
                    creSource.bLeftWeaponAttack = FALSE;
                    creSource.wDoHalfAttack = 0;
                } else {
                    creSource.bStatisticalAttack = false;
                    creSource.bLeftWeaponAttack = FALSE;
                    creSource.wDoHalfAttack = 0;
                }
            } else { //fixed code
                if (creSource.wDoHalfAttack == 1) { //do half attack
                    creSource.bIsAttacking = TRUE;
                    creSource.bStatisticalAttack = true;
                    creSource.bLeftWeaponAttack = FALSE;
                    if (nAttacks == 2) creSource.bLeftWeaponAttack = TRUE;

                    creSource.wDoHalfAttack = 0;
                } else { //don't do half attack
                    creSource.bStatisticalAttack = false;
                    creSource.bLeftWeaponAttack = FALSE;

                    creSource.wDoHalfAttack = 1;
                }
            }
        } else { //nthAttack > nAttacks
            creSource.bStatisticalAttack = FALSE;
            creSource.bLeftWeaponAttack = FALSE;
            if (!pGameOptionsEx->bActionAttackOnceFix) creSource.wDoHalfAttack = 0; //original code
        }
    }

    return creSource.bStatisticalAttack;
}

void __stdcall CCreatureObject_UpdateModalState_DoBardSongNormal(CCreatureObject& creSource) {
    ResRef rBardSong = "bardsong";
    creSource.CastSpell(
        rBardSong,
        creSource,
        FALSE,
        -1,
        NULL,
        FALSE,
        FALSE);
    return;
}

void __stdcall CCreatureObject_SetDifficultyLuckModifier(CCreatureObject& cre) {
    if (!pRuleEx->m_DiffMod.m_2da.bLoaded) {
        //original code
        if (g_pChitin->cNetwork.bSessionOpen) {
            if (g_pChitin->pGame->m_GameOptions.m_nMPDifficultyMultiplier < -25)
                cre.cdsCurrent.luck += 6;
        } else {
            if (g_pChitin->pGame->m_GameOptions.m_nDifficultyMultiplier < -25)
                cre.cdsCurrent.luck += 6;
        }

        return;
    }
    
    int nLuckMod = 0;
    int nCol = 1;
    int nRow = g_pChitin->pGame->m_GameOptions.m_nDifficultyLevel - 1;
    IECString sLuckMod;
    
    if (nCol < pRuleEx->m_DiffMod.nCols &&
        nRow < pRuleEx->m_DiffMod.nRows &&
        nCol >= 0 &&
        nRow >= 0) {
        sLuckMod = *((pRuleEx->m_DiffMod.pDataArray) + (pRuleEx->m_DiffMod.nCols * nRow + nCol));
    } else {
        sLuckMod = pRuleEx->m_DiffMod.defaultVal;
    }
    sscanf_s((LPCTSTR)sLuckMod, "%d", &nLuckMod);

    //if (g_pChitin->cNetwork.bSessionOpen) {
    //    cre.cdsCurrent.luck += nLuckMod;
    //} else {
        cre.cdsCurrent.luck += nLuckMod;
    //}

    return;
}



bool __stdcall
IsInsideDisarmSafeZone(CCreatureObject& Cre, CTriggerObject& Trap ) {
    long trap_left   = Trap.rectBounds.left;
    long trap_top    = Trap.rectBounds.top;
    long trap_right  = Trap.rectBounds.right;
    long trap_bottom = Trap.rectBounds.bottom;

    unsigned char MoveScale     = Cre.animation.pAnimation->nMovementRateDefault;
    unsigned char personalspace = Cre.animation.pAnimation->nFootCircleSize;

    // bg2ee code
    if (MoveScale > 8)
        MoveScale = 2*MoveScale - 12;
    else
        MoveScale = 4;

    int DisarmSafeZoneWidth = MoveScale + personalspace;
    if ( (Cre.currentLoc.x >  trap_left   - DisarmSafeZoneWidth) &&
         (Cre.currentLoc.x <= trap_right  + DisarmSafeZoneWidth) &&
         (Cre.currentLoc.y >  trap_top    - DisarmSafeZoneWidth) &&
         (Cre.currentLoc.y <= trap_bottom + DisarmSafeZoneWidth) ) {
            Cre.DropPath();
            return true;
    } else {
            return false;
    }
}


void __stdcall
CGameAIBase_FireSpell_InjectProjectileUpdate(
            CProjectileBAM* Projectile,
            uint EIP,
            uint ClassObj) {
    if (EIP == 0x46BFA9 ||  // CContingencyList::Process
        EIP == 0x46BFDD ||
        EIP == 0x46C011 ||

        EIP == 0x46C3BC ||  // CContingencyList::TriggerSequencer
        EIP == 0x46C3F0 ||
        EIP == 0x46C424 ||

        EIP == 0x46BAE2 ||  // CContingencyList::ProcessTrigger
        EIP == 0x46BB16 ||
        EIP == 0x46BB4A ) {
            // Flush Projectile list, normally it processed by Area::AIUpdate() after messages stuff.
            // AIUpdate() move effects from Area queue to message queue, so all effects with target!=self
            // will be placed immediate after effects with target==self.
            // This fix fit all spell's effects to message queue as one commit
            if (ClassObj !=0xAABE54)    // exclude CProjectileInstant, it destroy himself in CProjectileInstant::Fire()
                Projectile->AIUpdate();
    }

}


BOOL gAltPressed;
extern void PlayToggleSound(bool On_Off);

void
ToggleNPCTooltip() {
    gAltPressed = !gAltPressed;

    if (gAltPressed)
        PlayToggleSound(true);
    else
        PlayToggleSound(false);
}


short static __stdcall
CGameSprite_FloatingHP_NPC(CInfGame* pGame, CCreatureObject& Cre, Enum id) {
    short result = pGame->GetPartyMemberSlot(id);
    bool bTabPressed = to_bool(g_pChitin->pScreenWorld->keymap[89].nTicksPressed);
    bool bAltPressed = to_bool(pGameOptionsEx->bUI_ShowNPCFloatName && gAltPressed);
    bool bHPEnable   = to_bool(pGameOptionsEx->bUI_ShowNPCFloatHP);

    if (result == -1) {   // NPC
        if ( bTabPressed || bAltPressed ) {
            CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;
            uchar Allegiance = Cre.o.EnemyAlly;

            if (!Cre.canBeSeen)        // Out of range
                return  -1;

            if (cds.stateFlags & STATE_INVISIBLE)
                return  -1;

            if (cds.stateFlags & 0xFC0) // any dead state
                return  -1;

            if (pGameOptionsEx->bUI_ShowNPCFloatHP_Enemy &&
                bTabPressed && bHPEnable)
                if (Allegiance == EA_ENEMY ||
                    Allegiance == EA_GOODBUTRED ||
                    Allegiance == EA_EVILCUTOFF )
                    return (short)0xA5A5;

            if (pGameOptionsEx->bUI_ShowNPCFloatHP_Ally &&
                bTabPressed && bHPEnable)
                if (Allegiance <= EA_CONTROLLEDFORCEADD )    // 0<x<8 =ALLY/FAMILIAR/CONTROLLED/CHARMED
                    return (short)0xA5A5;

            if (pGameOptionsEx->bUI_ShowNPCFloatHP_Neutral &&
                bTabPressed && bHPEnable) {
                if (Allegiance == EA_EVILBUTGREEN ||
                    Allegiance == EA_EVILBUTBLUE ||
                    Allegiance == EA_GOODBUTBLUE ||
                    Allegiance == EA_GOODCUTOFF ||
                    Allegiance == EA_NEUTRAL )
                    return (short)0xA5A5;
            }

            if (bAltPressed)
                    return (short)0xA4A4;
        }
    } else      // Party
    if (!pGameOptionsEx->bUI_ShowNPCFloatHP_Party)
        return -1;

    return result;
}


void static __stdcall
CGameSprite_FloatingHP_SetText(CCreatureObject& Cre, IECString& str, ushort Marker) {
    if (Marker == 0xA4A4) {    // Name market
        short result = g_pChitin->pGame->GetPartyMemberSlot(Cre.id);
        if (result == -1) {   // NPC
            str = Cre.GetLongName();    // override text over head
        }
    }
}


void static __stdcall
CGameSprite_DoMoraleFailure_AddIcon(CCreatureObject& Cre, uint Mode) {
    CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;

    Action* QueuedAct = NULL;
    if (Cre.queuedActions.GetCount() > 0)
        QueuedAct = (Action*)Cre.queuedActions.GetHead();

    if (Cre.BaseStats.morale <= cds.moraleBreak ||
        Cre.bMoraleBroken) {
        if (Mode == 3) {    // Update icons
            if (Cre.berserkActive &&
                QueuedAct &&
                QueuedAct->opcode == 124   // Berserk-In-Panic opcode
               ) {
                if (!Cre.PortraitIcons.Find((void*)4))
                    Cre.PortraitIcons.AddTail((void*)4);    // Berserk

                if (!Cre.PortraitIcons.Find((void*)36))
                    Cre.PortraitIcons.AddTail((void*)36);   // Panic
            }

            if (Cre.BaseStats.stateFlags & 4) { // Panic
                if (!Cre.PortraitIcons.Find((void*)36))
                    Cre.PortraitIcons.AddTail((void*)36);   // Panic
            }
        } else {
            if (Mode == 2) {    // Add Berserk
                if (!Cre.PortraitIcons.Find((void*)4))
                    Cre.PortraitIcons.AddTail((void*)4);    // Berserk
            }

            // Add Berserk/RunAway/Panic
            if (!Cre.PortraitIcons.Find((void*)36))
                Cre.PortraitIcons.AddTail((void*)36);       // Panic
        }
    }
}


void static __stdcall
CGameSprite_EndMoraleFailure_RemoveIcon(CCreatureObject& Cre) {
    POSITION pos;
    CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;

    if (Cre.BaseStats.stateFlags & STATE_PANIC) {
        pos = Cre.PortraitIcons.Find((void*) 36);   // Panic
        if (pos)
            Cre.PortraitIcons.RemoveAt(pos);
    }

    if (Cre.berserkActive) {
        pos = Cre.PortraitIcons.Find((void*) 4);    // Berserk
        if (pos)
            Cre.PortraitIcons.RemoveAt(pos);

        pos = Cre.PortraitIcons.Find((void*) 36);   // Panic
        if (pos)
            Cre.PortraitIcons.RemoveAt(pos);
    }
}


short static __stdcall
CGameSprite_Swing_FixWeaponSpeed(
        CCreatureObject& Cre,
        CItem* Item,
        CItem* ItemLauncher,
        ItmFileAbility* ItemLauncherAbility,
        short ItemSpeed) {

    short nRow;
    short TotalSpeed;

    if (ItemLauncher) {
        nRow = Cre.GetProficiencyInItem(*ItemLauncher);
        if (ItemLauncherAbility)
            TotalSpeed = ItemLauncherAbility->speed;    // ignore ammo speed
        else
            TotalSpeed = ItemSpeed;                     // failed ability, use ammo speed
    }
    else {
        nRow = Cre.GetProficiencyInItem(*Item);
        TotalSpeed = ItemSpeed;
    }

    // WSPECIAL.2da
    short nCol = 2;             // SPEED
    int Value;
	IECString sHit;
	if (nCol  < g_pChitin->pGame->WSPECIAL.nCols &&
		nRow  < g_pChitin->pGame->WSPECIAL.nRows &&
		nCol  >= 0 &&
		nRow  >= 0) {
		sHit  = *((g_pChitin->pGame->WSPECIAL.pDataArray) + (g_pChitin->pGame->WSPECIAL.nCols * nRow  + nCol));
	} else {
		sHit  = g_pChitin->pGame->WSPECIAL.defaultVal;
	}
	sscanf_s((LPCTSTR)sHit,  "%d", &Value); // Value is negative

    Value = -Value;
    if (TotalSpeed - Value > 0)
        return  (TotalSpeed - Value);
    else
        return  0;  // max bonus

}


BOOL static __stdcall
CGameSprite_ClearBumpPath_CheckParty(CCreatureObject& Cre, CCreatureObject& CreToBump, BOOL bCurBumbable) {
    if (bCurBumbable == FALSE)
        return FALSE;

    if (Cre.o.EnemyAlly > EA_CONTROLLEDCUTOFF) {                        // NPC only
        if ( (CreToBump.o.EnemyAlly <= EA_CONTROLLEDCUTOFF ||           // can bump Party/Ally only
              pGameOptionsEx->bEngine_EnemyCanBumpPartyUnmoveable) &&   // can bump anything
             (CreToBump.cdsCurrent.stateFlags & STATE_INVISIBLE ||      // invisible or sanctuary
              CreToBump.cdsCurrent.sanctuary) )
            return TRUE;        // invisible Party/Ally are bumpable
        else
            return FALSE;       // not Party/Ally or not invisible, pure improved inv is not bumpable
    } else
        return bCurBumbable;    // no changes
}



uchar static __stdcall
CGameSprite_CheckBumpableUnmoveable(CCreatureObject& Cre, uchar MovementRate) {
    if (MovementRate == 0)  // holded
        return 1;

    if ( //Cre.o.EnemyAlly <= EA_CONTROLLEDCUTOFF &&        // can bump Party/Ally only
         (Cre.cdsCurrent.stateFlags & STATE_INVISIBLE ||    // invisible or sanctuary
          Cre.cdsCurrent.sanctuary) )
        return 1;
    else
        return 0;
    
}


//uint static __stdcall
//CGameSprite_Render_CheckInvisibiltyEffect(CCreatureObject& Cre, uint Flags) {
//    if (Flags & STATE_INVISIBLE)    // already invisible
//        return Flags;
//
//    if (Cre.bInvisible)
//        return (Flags | STATE_INVISIBLE);
//
//    CEffectList& SpellEffectList = Cre.GetMainEffectsList();
//    POSITION pos_spells = SpellEffectList.GetHeadPosition();
//    CEffect*    pEff = NULL;
//
//    while (pos_spells) {
//        pEff = (CEffect *) SpellEffectList.GetNext(pos_spells);
//        if (pEff == NULL) {
//            continue;
//        }
//
//        EffFileData& effect = pEff->effect;
//        int OpCode    = effect.nOpcode;
//
//        int ticks     = (effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime) / 15; // x/TICKS_PER_SECOND
//        if (ticks <= 0)               // effect is out
//            continue;
//
//        if (OpCode == 20 && effect.nParam2 == 0) // non improved Invisibilty effect
//            return (Flags | STATE_INVISIBLE);
//    }
//
//    return Flags;
//}


//void static __stdcall
//CGameSprite_MoveToPoint1_Log(CCreatureObject& Cre, CSearchRequest& SR, int mode) {
//    //SR.collisionSearch = 0;
//    //SR.collisionDelay = 0;
//
//    if (Cre.rSaveName == "*AGAIN4") {
//        console.write_debug("New Search Request mode=%d x=%d y=%d \n", mode, Cre.curDest.x, Cre.curDest.y);
//        console.write_debug("nGregsRetryCounter =%d \n", Cre.nGregsRetryCounter);
//    }
//}


void __declspec(naked)
DisarmAction_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     ; CTriggerObject
    push    [ebp-494h]  ; CCreatureObject 
    call    IsInsideDisarmSafeZone

    pop     edx
    pop     ecx

    cmp     al, 0
    jz      orig_code_4

;stopmoving:
    mov     word ptr [ebp-50h], 0FFFFh ; ACTIONRESULT_DONE
    pop     eax
    mov     eax, 09h    ; Stolen bytes
    cmp     eax, 10h    ; 9 <= 10 always true
    ret

orig_code_4:
    pop     eax
    add     eax, ecx   ; Stolen bytes
    cmp     eax, 10h
    ret
}
}


void __declspec(naked)
CCreatureObject_ShouldAvertCriticalHit_SpellOnly_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-294h]  ; CreSource
    push    [ebp+8]     ; CreTarget
    call    CCreatureObject_ShouldAvertCriticalHit_SpellOnly

    pop     edx
    pop     ecx
    pop     eax
    
    mov     edx, [ecx+42BAh]   ; Stolen bytes
    ret
}
}


void __declspec(naked)
CGameAIBase_FireSpell_InjectProjectileUpdate_asm() {
__asm
{
    push    edx         // Class vtable
    push    [esp+8*4]
    push    [esp+8*4]
    push    [esp+8*4]
    push    [esp+8*4]
    push    [esp+8*4]
    push    [esp+8*4]
    push    [esp+8*4]
    call    dword ptr [edx+64h]

    pop     edx
    push    ecx
    push    edx

    push    edx         // Class vtable
    push    [ebp+4]     // EIP
    push    [ebp-0B0h]  // CProjectileBAM
    call    CGameAIBase_FireSpell_InjectProjectileUpdate

    pop     edx
    pop     ecx
    
    mov     dword ptr [ebp-0B0h], 0   // Stolen bytes
    ret     7*4
}
}


void __declspec(naked)
CInfGame_GetCharacterPortrait_FakePartyMember_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-78h]   // ID
    push    [ebp-0ACh]  // Cre
    push    ecx         // CInfGame
    call    CGameSprite_FloatingHP_NPC
    // return ax - char number
    pop     edx
    pop     ecx

    mov     [ebp-0BCh], ax  // Tobex new variable on stack
    cmp     ax, 0xA5A5      // Show HP marker
    jz      CInfGame_GetCharacterPortrait_FakePartyMember_asm_Show
    cmp     ax, 0xA4A4      // Name marker
    jz      CInfGame_GetCharacterPortrait_FakePartyMember_asm_Show

    ret     4

CInfGame_GetCharacterPortrait_FakePartyMember_asm_Show:
    mov     dword ptr [ebp+08h], 1    // force_showhp = 1
    mov     dword ptr [ebp+0Ch], 2    // durationNew = 2
    mov     dword ptr [ebp+10h], 2    // durationExisting = 2
    xor     eax,eax     // fake party member

    ret     4    
}
}


void __declspec(naked)
CGameSprite_FloatingHP_SetText_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-0BCh]      // FloatMode
    lea     eax, [ebp-18h]  // CString
    push    eax
    push    [ebp-0ACh]      // Cre
    call    CGameSprite_FloatingHP_SetText

    pop     edx
    pop     ecx
    pop     eax
    
    mov     eax, [ebp-0ACh]  // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_Marshal_FirstCharFix_asm() {
__asm
{
    mov     eax, [ebp-6Ch]  //  Cre
    mov     al, [eax]CCreatureObject.cFirstResSlot
    mov     [ecx]CSavedGamePartyCreature.m_cFirstResSlot, al  // ecx - CSavedGamePartyCreature
    ret
}
}


void __declspec(naked)
CGameSprite_CheckCombatStatsWeapon_AddElfRacialThacBonus_asm() {
__asm
{
    and     eax, 0FFFFh // Stolen byte, ItemType
    cmp     eax, 15     // Bow
    jz      CGameSprite_CheckCombatStatsWeapon_AddElfRacialThacBonus_asm_Enable
    ret

CGameSprite_CheckCombatStatsWeapon_AddElfRacialThacBonus_asm_Enable:
    mov     eax, 14h    // fake type to enable bonbus
    ret
}
}


void __declspec(naked)
CGameSprite_CheckCombatStatsWeapon_AddHalflingRacialThacBonus_asm() {
__asm
{
    and     eax, 0FFFFh // Stolen byte, ItemType
    cmp     eax, 18     // Sling
    jz      CGameSprite_CheckCombatStatsWeapon_AddHalflingRacialThacBonus_asm_Enable
    ret

CGameSprite_CheckCombatStatsWeapon_AddHalflingRacialThacBonus_asm_Enable:
    mov     eax, 18h    // fake type to enable bonbus
    ret
}
}


void __declspec(naked)
CGameSprite_CheckCombatStatsWeapon_SwapWeaponText_asm() {
__asm
{
    mov     ecx, [ebp+8] // Item
    call    CItem::GetType
    and     eax, 0FFFFh
    cmp     eax, 15    // Bow
    jz      CGameSprite_CheckCombatStatsWeapon_SwapWeaponLauncherText_asm_Swap
    cmp     eax, 27    // CrossBow
    jz      CGameSprite_CheckCombatStatsWeapon_SwapWeaponLauncherText_asm_Swap
    cmp     eax, 18    // Sling
    jz      CGameSprite_CheckCombatStatsWeapon_SwapWeaponLauncherText_asm_Swap

    push    0A4DAE5h   // Stolen bytes
    ret

CGameSprite_CheckCombatStatsWeapon_SwapWeaponLauncherText_asm_Swap:
    mov     [esp+8], 0B579F0h  // "+Weapon Bonus %d" -> "+Launcher %d"

    push    0A4DAE5h   // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_DoMoraleFailure_UpdateIcons_asm() {
__asm
{   
    mov     [ebp-44h], ecx  // Stolen bytes
    mov     eax, [ebp-44h]

    push    eax
    push    ecx
    push    edx

    push    3           // Update Icon
    push    [ebp-44h]   // Cre
    call    CGameSprite_DoMoraleFailure_AddIcon

    pop     edx
    pop     ecx
    pop     eax
    
    ret
}
}



void __declspec(naked)
CGameSprite_DoMoraleFailure_AddBerserk_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    2           // Mode Berserk
    push    [ebp-44h]   // Cre
    call    CGameSprite_DoMoraleFailure_AddIcon

    pop     edx
    pop     ecx
    pop     eax
    
    mov     dword ptr [ebp-4], 2  // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_DoMoraleFailure_AddRunAway_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    1           // Mode RunAway
    push    [ebp-44h]   // Cre
    call    CGameSprite_DoMoraleFailure_AddIcon

    pop     edx
    pop     ecx
    pop     eax
    
    mov     dword ptr [ebp-4], 2  // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_DoMoraleFailure_AddPanic_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    0           // Mode Panic
    push    [ebp-44h]   // Cre
    call    CGameSprite_DoMoraleFailure_AddIcon

    pop     edx
    pop     ecx
    pop     eax
    
    mov     dword ptr [ebp-4], 2  // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_EndMoraleFailure_RemoveIcon_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-28h]   // Cre
    call    CGameSprite_EndMoraleFailure_RemoveIcon

    pop     edx
    pop     ecx
    pop     eax
    
    add     ecx, 6C48h  // Stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_Swing_FixWeaponSpeed_asm() {
__asm
{
    movzx   ax, byte ptr [edx+12h]  // item speed
    push    ecx
    push    edx

    push    eax
    push    [ebp-98h]   // ItemLauncherAbility
    push    [ebp-0B4h]  // ItemLauncher
    push    [ebp-2Ch]   // Item
    push    [ebp-614h]  // Cre
    call    CGameSprite_Swing_FixWeaponSpeed
    ; ret ax

    pop     edx
    pop     ecx

    ret
}
}


//void __declspec(naked)
//CGameSprite_Render_CheckInvisibiltyEffects_asm() {
//__asm
//{
//    mov     edx, [ecx]  // stolen byte
//    push    ecx
//
//    push    edx         // cds state flags
//    push    [ebp-204h]  // Cre
//    call    CGameSprite_Render_CheckInvisibiltyEffect
//    ; ret eax
//
//    mov     edx, eax
//    pop     ecx
//
//    and     edx, 10h  // stolen byte
//
//    ret
//}
//}
//
//
//void __declspec(naked)
//CGameSprite_RenderMarkers_CheckInvisibiltyEffects_asm() {
//__asm
//{
//    mov     edx, [ecx]  // stolen byte
//    push    ecx
//
//    push    edx         // cds state flags
//    push    [ebp-10Ch]  // Cre
//    call    CGameSprite_Render_CheckInvisibiltyEffect
//    ; ret eax
//
//    mov     edx, eax
//    pop     ecx
//
//    and     edx, 10h  // stolen byte
//
//    ret
//}
//}


//void __declspec(naked)
//CGameSprite_MoveToPoint1_asm() {
//__asm
//{
//    push    eax
//    push    ecx
//    push    edx
//
//    push    1
//    push    [ebp-1Ch]   // SearchRequest
//    push    [ebp-474h]  // Cre
//    call    CGameSprite_MoveToPoint1_Log
//
//    pop     edx
//    pop     ecx
//    pop     eax
//
//    mov     ecx, [ebp-474h]  // stolen byte
//    ret
//}
//}
//
//
//void __declspec(naked)
//CGameSprite_MoveToPoint2_asm() {
//__asm
//{
//    push    eax
//    push    ecx
//    push    edx
//
//    push    2
//    push    [ebp-70h]   // SearchRequest
//    push    [ebp-188h]  // Cre
//    call    CGameSprite_MoveToPoint1_Log
//
//    pop     edx
//    pop     ecx
//    pop     eax
//
//    mov     ecx, [eax+343Eh]  // stolen byte
//    ret
//}
//}


void __declspec(naked)
CGameSprite_CheckBumpable_asm() {
__asm
{
    add     esp, 4   // no return

    cmp     eax, 15                             // <= EA_CONTROLLEDCUTOFF
    jg      CGameSprite_CheckBumpable_asm_Orig    // bumpable= 0

    mov     eax, 1
    jmp     CGameSprite_CheckBumpable_asm_Exit    // bumpable= 1

CGameSprite_CheckBumpable_asm_Orig:
    xor     eax, eax  // stolen byte

CGameSprite_CheckBumpable_asm_Exit:
    push    0894884h
    ret
}
}


void __declspec(naked)
CGameSprite_ClearBumpPath_CheckParty_asm() {
__asm
{
    push    ecx
    
    push    edx         // current bump state
    push    [ebp-70h]   // Cre to bump
    push    [ebp-1A0h]  // Cre
    call    CGameSprite_ClearBumpPath_CheckParty
    // return eax

    mov     edx, eax
    pop     ecx

    mov     [ebp-100h], edx  // stolen byte
    ret
}
}


void __declspec(naked)
CGameSprite_CheckBumpable2_asm() {
__asm
{
    push    ecx
    push    edx

    push    eax        // movement rate
    push    [ebp-3Ch]  // Cre
    call    CGameSprite_CheckBumpableUnmoveable
    // return al

    pop     edx
    pop     ecx
    test    al,al
    jz      CGameSprite_CheckBumpable2_asm_Continue

    add     esp, 4   // no return
    push    0894884h
    ret

CGameSprite_CheckBumpable2_asm_Continue:    
    add     esp, 4   // no return
    push    08947E3h
    ret
}
}