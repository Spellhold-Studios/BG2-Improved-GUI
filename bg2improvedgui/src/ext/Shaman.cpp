#include "Shaman.h"
#include "InfGameCommon.h"
#include "ObjectStats.h"
#include "EffectCommon.h"

#define IECString_Constructor_CharPtr(obj,arg) THISCALL_1(0xA5093B, obj, arg) // IECString::IECString(LPCSTR lpsz)

#define CLASS_SHAMAN_21         21

#define STRREF_SHAMAN_KIT_DESC  6570  // long text
#define STRREF_SHAMAN_BOOKNAME  6573  // ~SHAMAN BOOK~ , realms
#define STRREF_MAGE_BOOKNAME    11989 // ~MAGE BOOK~   , realms
#define STRREF_DETECTILLUSIONS  6577  // ~Detecting Illusions~
#define STRREF_STOPDETECTILLUS  6578  // ~Stopped Detecting Illusions~
#define STRREF_SHAMANDANCE      6579  // ~Shamanic Dance~
#define STRREF_DANCINGDANCE     6580  // ~Dancing Shamanic Dance~
#define STRREF_STOPDANCINGDANCE 6581  // ~Stopped Dancing Shamanic Dance~

char* SHAMAN_STRING = "SHAMAN";

static DWORD gGetXXXSpell;
static DWORD gGetKnownSpellXXX;
static DWORD gBookName;
static DWORD gGetXXXLevel;
static DWORD gGetKnownSpellIndexXXX;
static int SavedPickForMe_width  = 0;
static int SavedPickForMe_height = 0;


void
CEffectDisableButton_Shaman::OnRemove(CCreatureObject& Cre) {
    if (Cre.o.Class == CLASS_SHAMAN) {
        if (Cre.nModalState != 1) { // != 1  song was stopped, restore disabled button
            CDerivedStats& cds = Cre.GetDerivedStats();

            unsigned int nParam2 = this->effect.nParam2;
            // hack for CInfButtonArray::CheckActivation()
            cds.ButtonDisable[nParam2] = 0;

            // tobex new
            if (pGameOptionsEx->bEffButtonFXMod &&
                pGameOptionsEx->bEngineExpandedStats) {
                if (nParam2 == 14) { // find traps button
                    CDerivedStats_SetStat(cds, CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_BUTTONDISABLEFINDTRAPS, 0);
                }
            }
        }
    }
}


void static __stdcall
CancelShamanDanceIfMoving(CCreatureObject& Cre) {
    if (Cre.o.Class == CLASS_SHAMAN) {
        if (Cre.nModalState == 1) {                         // 1 - song/dance
            Cre.SetModalState(0,1);                         // reset to null state
            CDerivedStats_SetStat(Cre.GetDerivedStats(),    // instant remove summons
                CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_SHAMANDANCE, 0);

            ResRef Res = "SPSH003";     // main dance spell
            Cre.EffectsEquipped.RemoveAllEffectsFromSourceRes(
                Cre,
                Cre.EffectsEquipped.posItrPrev,
                Res);
            Cre.EffectsMain.RemoveAllEffectsFromSourceRes(
                Cre,
                Cre.EffectsMain.posItrPrev,
                Res);
        }
    }
}


void static __stdcall
CGameSprite_ExecuteAction_MakeUnselectable(CCreatureObject& Cre) {
    int param2 = Cre.currentAction.m_specificID2;
    if (param2 != 0) { // MakeUnselectable(xxx,yyy)
        if (param2 == 1)
            Cre.u62b5 = 0x33; // allow script processing
        else
        if (param2 == 2)
            Cre.u62b5 = 0x34; // allow area selection, unused
    }
}



bool static __stdcall
CGameSprite_ProcessPendingTriggers(CCreatureObject& Cre) {
    return (Cre.u62b5 == 0x33);
}


int GetNumSummonedSpirits()
{
    CCreatureObject* Summon;
    Enum mID;
    int count = 0;
    CEnumList& List = g_pChitin->pGame->celControlledObjects;

    POSITION pos = List.GetHeadPosition();
    while (pos != NULL) {
        mID = (Enum) List.GetNext(pos);
        if (g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(mID, 0, &Summon, -1) == OBJECT_SUCCESS) {
            if (Summon->o.Gender == 6 &&        // summon
                Summon->o.Specific == 250){     // spirit
                count++;
            }
            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(mID, 0, -1);
        }
    }

    return count;
}


BOOL static __stdcall
CGameEffectRandomSummon_Apply_IsShamanSummonLimit(CEffect& eff, CCreatureObject& Cre, CCreatureObject& Summon) {
    STRREF  txtref;
    int     lev = eff.effect.nLevelMin;

    if (Cre.o.Class == CLASS_SHAMAN &&
        eff.effect.nParam2 == 0x10) {       // shaman summon type
        if (eff.effect.nParam3 > GetNumSummonedSpirits()) {
            if (lev < 6)
                txtref = 9523;              // ~A minor animal spirit answers your call~
            else
            if (lev < 12)
                txtref = 9524;              // ~A major animal spirit answers your call~
            else
            if (lev < 18)
                txtref = 9525;              // ~A minor nature spirit answers your call~
            else
                txtref = 9526;              // ~A major nature spirit answers your call~

            IECString sText = GetTlkString(txtref);
            uchar color = Cre.BaseStats.colors.colorMajor;
            ABGR colorOwner = *(g_pColorRangeArray + Cre.BaseStats.colors.colorMajor);
            ABGR colorText = g_ColorDefaultText;
            IECString sOwner = Cre.GetLongName();

            g_pChitin->pScreenWorld->PrintToConsole(sOwner, sText, colorOwner, colorText, -1, 0);

            return FALSE;
        }
        else
            return TRUE;
    }
    else
        return FALSE;
}


void static __stdcall
CGameSprite_Rest_AddShaman(CCreatureObject& Cre, BOOL bMemorizeSpells) {
    CAbilityId SpellAbility;
    SpellAbility.itemType   = 1;
    SpellAbility.itemNum    = -1;
    SpellAbility.abilityNum = -1;
    SpellAbility.toolTip    = -1;
    SpellAbility.ResName.Clean();

    if (Cre.o.Class == CLASS_SHAMAN && bMemorizeSpells ) {
        for ( uint Index = 0; Index < 7; Index++ ) {
            CCreatureFileMemorizedSpell* pMemSpell;
            POSITION pos = Cre.MemorizedSpellsPriest[Index].GetHeadPosition();

            while (pos != NULL) {
                pMemSpell = (CCreatureFileMemorizedSpell*) Cre.MemorizedSpellsPriest[Index].GetNext(pos);
                SpellAbility.ResName = pMemSpell->SpellId;
                Cre.CheckQuickLists(&SpellAbility, -1, 0, 0);
                IEFree(pMemSpell);
            }

            Cre.MemorizedSpellsPriest[Index].RemoveAll();

            ushort MaxMemSpells = Cre.GetDerivedStats().MemInfoPriest[Index].wMaxMemSpells;

            for ( int i = 0; i < Cre.KnownSpellsPriest[Index].GetCount(); i++ ) {
                CreFileKnownSpell* KnownSpell = Cre.GetKnownSpellPriest(Index, i);
                for ( uint j = 0; j < MaxMemSpells; j++ )
                {
                    CCreatureFileMemorizedSpell* NewNode = IENew CCreatureFileMemorizedSpell;
                    NewNode->SpellId = KnownSpell->name;
                    NewNode->flags = 0;
                    NewNode->pad = 0;
                    Cre.MemorizedSpellsPriest[Index].AddTail(NewNode);
                }
            }
        }
    }
}


void static __stdcall
CGameSprite_RemoveSpellsPriest_AddShaman(CCreatureObject& Cre, uchar Level, short Amount) {
    if (Cre.o.Class == CLASS_SHAMAN) {
        ResRef resref = "";
        char count = Cre.SorcererSpellCount(Level + 1, resref);
        ushort wCurrMemSpells = Cre.cdsCurrent.MemInfoPriest[Level].wCurrMemSpells;
        if (count > wCurrMemSpells) {
            for (int i = 0; i < Amount; i++ ) {
                Cre.SorcererSpellDecrement(Level + 1, &resref, 1);
            }
        }
    }
}


//void static __stdcall
//Log_GetClass(int EIP) {
//
//	LPCTSTR szFormat = "GetClass: 0x%X \r\n";
//    L.timestamp();
//	L.appendf(szFormat, EIP-4);
//}


CUIButtonShamanClass::CUIButtonShamanClass(
                                    CPanel& panel,
                                    ChuFileControlInfoBase& controlInfo)
                                    : CUICheckButton(panel, controlInfo, 1, TRUE)
{
    IECString sKitName;

    SetVT(this, 0xAAFDD8);  // CUICheckButtonChargenClass
    g_pChitin->pGame->GetDetailedClassString(CLASS_SHAMAN, KIT_TRUECLASS, 0, sKitName, NULL);
    CUIButton::SetText(sKitName);      // ~shaman~
}


    void static __stdcall
    CRuleTables_GetClassString_InjectDefault(IECString* result, BYTE nClass, char* nullstr ) {
        if (nClass == CLASS_SHAMAN) {    // shaman
            IECString_Constructor_CharPtr(result, SHAMAN_STRING);
        } else {
            IECString_Constructor_CharPtr(result, nullstr);
        }
    }


    BYTE static __stdcall
    GetClassFromButton(CUIButton& button) {
        if (button.index == 18)     // "shaman" button
            return CLASS_SHAMAN;
        else
            return 0;               // error
    }


    DWORD static __stdcall
    ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF(CUIButton& button) {
        if (button.index == 18)             // "shaman" button
            return STRREF_SHAMAN_KIT_DESC;  // shaman kit text description strref
        else
            return 0;                       // error
    }


    DWORD static __stdcall
    CRuleTables_GetClassStringLower(BYTE nClass) {
        if (nClass == CLASS_SHAMAN)
            return 6569;    // ~shaman~
        else
            return 0;       // error
    }


    DWORD static __stdcall
    GetClassTextTemplate(BYTE nClass) {
        if (nClass == CLASS_SHAMAN)
            return 6571;    // ~Shaman~
        else
            return 0;       // error
    }


    BOOL static __stdcall
    Generate_AnimID_ClassSuffix(Object& obj, WORD* pClassPrefix) {
        if (obj.Class == CLASS_SHAMAN) {
            *pClassPrefix = 0x0000;   // cleric class animation
            return TRUE;
        } 
        else
            return FALSE;       // error
    }


    BOOL static __stdcall
    IsShamanClassByNumber(BYTE nClass) {
        if (nClass == CLASS_SHAMAN) {
            return TRUE;
        } 
        else
            return FALSE;
    }


    BOOL static __stdcall
    IsShamanClassByObject(Object& obj) {
        if (obj.Class == CLASS_SHAMAN) {
            return TRUE;
        } 
        else
            return FALSE;
    }


    BOOL static __stdcall
    IsShamanClassByCreature(CCreatureObject& Cre) {
        if (Cre.o.Class == CLASS_SHAMAN) {
            return TRUE;
        } 
        else
            return FALSE;
    }

    BOOL static __stdcall
    IsShamanClassByCreatureBase(CCreatureObject& Cre) {
        if (Cre.oBase.Class == CLASS_SHAMAN) {
            return TRUE;
        } 
        else
            return FALSE;
    }


BOOL static __stdcall
IsShamanDancing(CCreatureObject& Cre) {
    if (Cre.o.Class == CLASS_SHAMAN &&
        Cre.nModalState == 1) { // 1 = Song
        return TRUE;
    } 
    else
        return FALSE;
}


    int  static __stdcall
    GetProficiencyClassIndex(BYTE nClass, DWORD EIP) {
        if (nClass == CLASS_SHAMAN) {
            int col;
            if (EIP == 0x63D625) {  // CRuleTables::Add25StartEquipment
                for (col = 52; col < g_pChitin->pGame->_25STWEAP.nCols; col++) {     // 52 - last official wildmage column
                    IECString& name = g_pChitin->pGame->_25STWEAP.GetColName(col);
			
                    if (name.CompareNoCase(SHAMAN_STRING) == 0)
                        break;
                    else
                        continue;
                }

                return col;
            } else {                // CRuleTables::GetClassProficiency
                for (col = 52; col < g_pChitin->pGame->WEAPPROF.nCols; col++) {
                    IECString& name = g_pChitin->pGame->WEAPPROF.GetColName(col);
			
                    if (name.CompareNoCase(SHAMAN_STRING) == 0)
                        break;
                    else
                        continue;
                }

                return col;
            }
        } 
        else
            return 0;   // error
    }


    void static __stdcall
    SummonShamanSpellsPanel(CCreatureObject& Cre, Object& oBase, CDerivedStats& cds, CScreenCharGen& Screen) {
        Cre.RemoveAllSpellsPriest();
        int BonusSpells = 0;
        int MaxLevel;

        for (MaxLevel = 7; MaxLevel > 0; MaxLevel--)
            { if (g_pChitin->pGame->GetMaxMemorizedSpellsPriest(
                   oBase,
                   cds,
                   MaxLevel,
                   &BonusSpells))
                break;
            }
       Screen.nCurrentMageBookLevel = 1;
       Screen.nMaxMageBookLevel = MaxLevel;
    }


    void static __stdcall
    GetMaxMemorizedSpellsShaman(Object& obj, CDerivedStats& cds, IECString& sLevel,
                                       IECString& sSpellLevel, int* result) {
     if ( obj.Class == CLASS_SHAMAN ) {
        int lev = cds.GetClericClassLevel(CLASS_SHAMAN);
        sLevel.Format("%d", lev);
        IECString& sValue = pRuleEx->Shaman_MXSPLSHM.GetValue(sSpellLevel, sLevel);
        int Value;
        sscanf_s((LPCTSTR)sValue, "%d", &Value);
        if ( Value > *result ) {
            *result = Value;
        }
      }
    }


    void static __stdcall
    CScreenGenChar_ResetChooseMagePanel_SwitchToShaman(CScreenCharGen& Screen, CCreatureObject& Cre,
                                                       Object& obj, CPanel& panel) {
        CUIButton& PickForMe = (CUIButton&) panel.GetUIControl(30); // "Pick Spells For Me"

        if ( obj.Class == CLASS_SHAMAN ) {
            IECString sValue;
            int level = Cre.cdsCurrent.GetClericClassLevel(CLASS_SHAMAN);
            int nCol  = Screen.nCurrentMageBookLevel - 1;
            int nRow  = level - 1;
            CRuleTable& SPLSHMKN = pRuleEx->Shaman_SPLSHMKN;
            if ( nCol < SPLSHMKN.nCols &&
                 nRow < SPLSHMKN.nRows &&
                 nCol >= 0 &&
                 nRow >= 0 )
              sValue = *((SPLSHMKN.pDataArray) + (SPLSHMKN.nCols * nRow + nCol));
            else
              sValue = SPLSHMKN.defaultVal;

            int nValue;
            sscanf_s((LPCTSTR)sValue, "%d", &nValue);
            Screen.nSpellsRemaining = nValue;

            if (PickForMe.width != 0) {
                SavedPickForMe_width  = PickForMe.width;
                SavedPickForMe_height = PickForMe.height;
            }
            PickForMe.width=0;  // hide this way, usual enable(0)/visible(0) will revert back to 1 by engine
            PickForMe.height=0;

            // change pointer to call spells
            gGetXXXSpell = 0x633556;     // CRuleTables::GetPriestSpell
            gBookName = STRREF_SHAMAN_BOOKNAME;
        }
        else {  // orig mage panel
            if (SavedPickForMe_width != 0) {
                PickForMe.width  = SavedPickForMe_width;
                PickForMe.height = SavedPickForMe_height;
            }

            gGetXXXSpell = 0x633691;    // CRuleTables::GetMageSpell
            gBookName = STRREF_MAGE_BOOKNAME;
        }
    }


    void static __stdcall
    CScreenCharacter_UpdateLevelUpPanel_SelectSkills(CScreenRecord& Screen, CPanel& Panel, CCreatureObject& Cre) {
        int Value = Cre.GetSkillValue(5, Screen.m_nSelectedDualClass, 0); // 5 - Detect Illusion
        UpdateLabel(Screen,
                    Panel,
                    0x1000002B,
                    "%d%",
                    Value);

        IECString sSkillName = GetTlkString(CCreatureObject_GetSkillName(5));
        UpdateLabel(Screen,
                    Panel,
                    0x10000020,
                    "%s%",
                    (LPCTSTR)sSkillName);
    }


    BOOL static __stdcall
    IsFilteredSpellToSelect(ResSplContainer& Spell, int Counter) {
        if (Counter+1 >= 50)   // > SPPR*50
            return TRUE;

            if (Spell.name == "SPPR721" ||  // HLAs
                Spell.name == "SPPR722" ||
                Spell.name == "SPPR723" ||
                Spell.name == "SPPR724" ||
                Spell.name == "SPPR725" ||
                Spell.name == "SPPR726" ||
                Spell.name == "SPPR727" ||
                Spell.name == "SPPR728" ||
                Spell.name == "SPPR729" ||
                Spell.name == "SPPR730" ||
                Spell.name == "SPPR731" ||
                Spell.name == "SPPR732")
                return TRUE;

        if (Spell.GetExclusionFlags() & 0x80000000) // druid/ranger/shaman disabled
            return TRUE;
        else
            return FALSE;
    }


    void static __stdcall
    CScreenGenChar_UpdateMainPanel_AddDetectIllusion(CScreenCharGen& Screen, CUITextArea& pTextArea, int detectillusion) {
        CEngine_UpdateText(Screen, pTextArea, (char *)0xB85FA0);    // NULL string
        IECString sProficiencies = GetTlkString(8442);              // "Proficiencies"
        CEngine_UpdateText(Screen, pTextArea, "%s", (LPCTSTR)sProficiencies);
        IECString sDetectIllusion = GetTlkString(34121);            // "Detect Illusion"
        CEngine_UpdateText(Screen, pTextArea, "%s: %d%", (LPCTSTR)sDetectIllusion, detectillusion);
    }


    void CRuleTables::GetShamanSkillPoints(unsigned char nLevel, unsigned char *nDetect) {
        IECString sLevel;
        sLevel.Format("%d", nLevel);
        IECString& sValue = pRuleEx->Shaman_SKILLSHM.GetValue( *((IECString *)0xB80FB0), sLevel); // IECString("DETECT_ILLUSION")
        int nValue;
        sscanf_s((LPCTSTR)sValue, "%d", &nValue);
        *nDetect = nValue;
    }


    void static __stdcall
    SetShamanSkillPoints(CCreatureObject& Cre, CreFileData& BaseStats, unsigned char nLevel) {
        if (Cre.o.Class == CLASS_SHAMAN) {
            g_pChitin->pGame->GetShamanSkillPoints(nLevel, &BaseStats.detectIllusion);
                BaseStats.pickPockets   = 0;
                BaseStats.lockpicking   = 0;
                BaseStats.findTraps     = 0;
                BaseStats.stealth       = 0;
                BaseStats.hideInShadows = 0;
                BaseStats.setTraps      = 0;
        }
    }


    void static __stdcall
    CCreatureObject_AddNewSpecialAbilitiesShaman(CCreatureObject& Cre, CDerivedStats& old_cds, BOOL bDisplayFeedback) {
        uchar cur_level = Cre.cdsCurrent.GetClericClassLevel(Cre.oBase.Class);
        uchar old_level =        old_cds.GetClericClassLevel(Cre.oBase.Class);
        Cre.AddClassAbilities(CLASS_SHAMAN, cur_level - old_level, bDisplayFeedback);
    }


    void static __stdcall
    CCreatureObject_RemoveNewSpecialAbilitiesShaman(CCreatureObject& Cre, CDerivedStats& old_cds) {
        uchar cur_level = Cre.cdsCurrent.GetClericClassLevel(Cre.oBase.Class);
        uchar old_level =        old_cds.GetClericClassLevel(Cre.oBase.Class);
        Cre.RemoveClassAbilities(CLASS_SHAMAN, cur_level - old_level);
    }


    CRuleTable* __stdcall
    GetShamanAbilityTable() {
        return &pRuleEx->Shaman_CLABSH01;
    }


    void static __stdcall
    AddDetectIllusionText(CScreenRecord& Screen, CUITextArea& pTextArea, CDerivedStats& cdsCurrent) {
        IECString sDetectIllusion = GetTlkString(34121); // "Detect Illusion"
        CEngine_UpdateText(Screen, pTextArea, "%s: %d%", (LPCTSTR)sDetectIllusion, cdsCurrent.detectIllusion);
    }


// apply shaman filter before main druid item filter
BOOL static __stdcall
CheckItemUsableByShamanClass(CItem& Item, uint* pErrorCode, uint ItemNotUsableMask, uint baseFlags) {
    short type = Item.GetType();
    uchar prof = Item.GetProficiencyType();

    // enable axe
    if (type == ITEMTYPE_AXE)
        return TRUE;

    // enable short bow
    if (type == ITEMTYPE_BOW &&
        prof == STATS_PROFICIENCYSHORTBOW)
        return TRUE;

    // enable arrows
    if (type == ITEMTYPE_ARROW)
        return TRUE;

    // disable scimitar/japan*
    if (type == ITEMTYPE_LARGE_SWORD &&
        prof == STATS_PROFICIENCYSCIMITARWAKISASHININJATO) {
            *pErrorCode = 9382;
            return FALSE;
    }

    // disable medium/large shield
    // shaman allow only buckler but there is no 100% way to distinguish small shield vs buckler
    if (type == ITEMTYPE_SHIELD) {
        BOOL forbid = FALSE;

        Item.itm.pRes->Demand();
            if (Item.itm.pRes->pFile->m_cAnimationType[1] == '3' || // medium shield anim
                Item.itm.pRes->pFile->m_cAnimationType[1] == '4' )  // large  shield anim
                forbid = TRUE;

            if (Item.itm.pRes->pFile->m_strrefGenericName == 6645 || // "Small Shield"
                Item.itm.pRes->pFile->m_strrefGenericName == 215  || // "Medium Shield"
                Item.itm.pRes->pFile->m_strrefGenericName == 6640)   // "Large Shield"
                forbid = TRUE;
        Item.itm.pRes->Release();

        if (forbid) {
            *pErrorCode = 9382;
            return FALSE;
        }
    }

    // unique shaman items
    if (Item.itm.name == "BDAMUL26" ||  // Heart of the Mountain
        Item.itm.name == "BDHELM16" ||  // Circlet of Lost Souls
        Item.itm.name == "BDSTAF02" ||  // The Soulherder's Staff +2
        Item.itm.name == "A7#LEAT1" ||  // Spirit Guide +4 (Improved Shamanic Dance by Argent77) 
        Item.itm.name == "A7#CLCK1" )   // Shroud of Souls (Improved Shamanic Dance by Argent77) 
        return TRUE;


    // main druid filter
    if ( g_pChitin->pGame->CheckItemNotUsableByClass(CLASS_SHAMAN, ItemNotUsableMask, baseFlags) ) {
        *pErrorCode = 9382;
        return FALSE;
    }
    else {
        return TRUE;
    }
}


void static __stdcall
ConfigMageScreen(CScreenRecord& Screen, BOOL Enable) {
    if (Enable) {
        // change pointer to spells
        gGetKnownSpellXXX = 0x8CB91F;   // CCreatureObject::GetKnownSpellPriest
        gBookName = STRREF_SHAMAN_BOOKNAME;
    }
    else {
        gGetKnownSpellXXX = 0x8CB949;   // CCreatureObject::GetKnownSpellMage
        gBookName = STRREF_MAGE_BOOKNAME;
    }

    
    IECString sBookName = GetTlkString(gBookName);
    CPanel& Panel = Screen.GetManager().GetPanel(8); // Sorcerer Book
    UpdateLabel(Screen,
                Panel,
                0x10000000 + 100,   // orig guimg.chu has unchangeable label with id=FFFF
                "%s",
                (LPCTSTR)sBookName);
}


BOOL static __stdcall
IsArcaneClassForMageScreen(Object& obj) {
    if (obj.Class == CLASS_SHAMAN)
        return TRUE;
    else
        return FALSE;
}


void static __stdcall
CInfButtonArray_SetState_AddShaman(CButtonArray& btn_array) {
    btn_array.nButtonIdx[0] = 5;    // talk
    btn_array.nButtonIdx[1] = 27;   // weapon 1
    btn_array.nButtonIdx[2] = 28;   // weapon 2
    btn_array.nButtonIdx[3] = 2;    // song
    btn_array.nButtonIdx[4] = 4;    // detect traps
    btn_array.nButtonIdx[5] = 24;   // quick spell 1
    btn_array.nButtonIdx[6] = 3;    // select spell
    btn_array.nButtonIdx[7] = 14;   // item ability
    btn_array.nButtonIdx[8] = 21;   // quick slot 1
    btn_array.nButtonIdx[9] = 22;   // quick slot 2
    btn_array.nButtonIdx[10] = 23;  // quick slot 3
    btn_array.nButtonIdx[11] = 10;  // innate
    btn_array.nButtonArrayTypeCurrentIdx = 21;
    btn_array.nButtonArrayTypePreviousIdx = 21;
}


BOOL static __stdcall
IsShamanClassByButtonSet() {
    if (g_pChitin->pGame->m_CButtonArray.nButtonArrayTypeCurrentIdx == CLASS_SHAMAN)
        return TRUE;
    else
        return FALSE;
}


BOOL static __stdcall
CScreenCharacter_OnDoneButtonClick_SwitchToShaman(Object& obj) {
 if ( obj.Class == CLASS_SHAMAN ) {
        // change pointer 
        gGetXXXLevel = 0x4749F9;     // CRuleTables::GetPriestLevel
        //gBookName = SHAMAN_BOOKNAME;
        return TRUE;
    }
    else {
        gGetXXXLevel = 0x4748AE;    // CDerivedStats::GetWizardLevel
        //gBookName = MAGE_BOOKNAME;
        return FALSE;
    }
}


BOOL static __stdcall
CScreenCharacter_ResetChooseMagePanel_SwitchToShaman(Object& obj) {
 if ( obj.Class == CLASS_SHAMAN ) {
        // change pointer to call spells
        gGetXXXSpell = 0x633556;            // CRuleTables::GetPriestSpell
        gGetKnownSpellIndexXXX = 0x8CBF91;  // CGameSprite::GetKnownSpellIndexPriest
        gBookName = STRREF_SHAMAN_BOOKNAME;
        return TRUE;
    }
    else {
        gGetXXXSpell = 0x633691;            // CRuleTables::GetMageSpell
        gGetKnownSpellIndexXXX = 0x8CBF67;  // CGameSprite::GetKnownSpellIndexMage
        gBookName = STRREF_MAGE_BOOKNAME;
        return FALSE;
    }
}


BOOL static __stdcall
IsNeedRemoveAllSpellsPriest(CCreatureObject& Cre, int nClass) {
    if (nClass == CLASS_SHAMAN ||
        Cre.oDerived.Class == CLASS_SHAMAN) {
        return FALSE;
    } 
    else
        return TRUE;
}


void static __stdcall
CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints(CCreatureObject& Cre, const CDerivedStats& cds) {
    uchar detectIllusion_SkillPoints;
    g_pChitin->pGame->GetShamanSkillPoints((uchar)cds.levelPrimary, &detectIllusion_SkillPoints);
    Cre.cdsDiff.detectIllusion = detectIllusion_SkillPoints - Cre.BaseStats.detectIllusion;
}

static CVidCell gShamanDanceIcon;

void static __stdcall
ShamanDance_InitIcon() {
    static ResRef bamfile; 

    if (bamfile.IsEmpty()) {
        bamfile = "GUIBSHA1";
        gShamanDanceIcon.bam.SetResRef(&bamfile, 1, 1);
        gShamanDanceIcon.bam.pBam->Demand();               // never released
        gShamanDanceIcon.bam.pBam->m_bCopyResData = TRUE;
        gShamanDanceIcon.bDoubleResolution = g_pChitin->bDoubleResolution;
    }
}


    void  __declspec(naked)
    CRuleTables_GetClassString_asm() {
    __asm {
        push    ecx
        push    edx

        push    [esp+0Ch]   // orig null-string
        push    [ebp+0Ch]   // nClass
        push    ecx         // result CString
        call    CRuleTables_GetClassString_InjectDefault

        pop     edx
        pop     ecx
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenClassSelection_GetClassFromButtonIndex_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-4]     // CUIButton
        call    GetClassFromButton

        test    al, al
        pop     edx
        pop     ecx
        jz      exit_ButtonCharGenClassSelection_GetClassFromButtonIndex

        // al - class
        add     esp, 4       // kill ret_addr
        push    07340A9h     // continue
        ret

    exit_ButtonCharGenClassSelection_GetClassFromButtonIndex:
        mov     ecx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetClassStringLower_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]         // class
        call    CRuleTables_GetClassStringLower

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CRuleTables_GetClassStringLower

        mov     [ebp-84h], eax  // TextRefID

        add     esp, 4          // kill ret_addr
        push    0635DF2h        // continue
        ret

    exit_CRuleTables_GetClassStringLower:
        mov     ecx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-94h]       // Button
        call    ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF

        mov     [ebp-44h], eax  // TextRefID
        add     esp, 4          // kill ret_addr
        push    0733DE8h        // skip error
        ret

    exit_ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF:
        mov     ecx, 1          // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenClassSelection_OnLButtonClick_OpenSubKits_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-94h]   // Button
        call    ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF  // used as check for shaman button

        test    eax, eax
        pop     edx
        pop     ecx
        jz      ButtonCharGenClassSelection_OnLButtonClick_OpenSubKits_exit

        add     esp, 4      // kill ret_addr
        push    0733E75h    // no SubKit path
        ret

    ButtonCharGenClassSelection_OnLButtonClick_OpenSubKits_exit:
        mov     ecx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCreateChar_CompleteCharacterClass_SetAnimIDClassSuffix_asm() {
    __asm {
        push    ecx
        push    edx

        lea     eax, [ebp-8D0h]
        push    eax             // wClassSuffix
        lea     eax, [ebp-8ECh]
        push    eax             // Object
        call    Generate_AnimID_ClassSuffix

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_SetAnimIDClassSuffix

        add     esp, 4          // kill ret_addr
        push    0723D33h        // skip error
        ret

    exit_SetAnimIDClassSuffix:
        mov     edx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetTHAC0_asm() {
    __asm {
        // al    - Class
        // esp+4 - Class
        // esp+8 - SubClass
        // esp+c - cds
        // ecx   - CRuleTables
        cmp     al, CLASS_SHAMAN_21
        jnz     exit_CRuleTables_GetTHAC0

        mov     byte ptr [esp+8], al    // SubClass = CLASS_SHAMAN_21

    exit_CRuleTables_GetTHAC0:
        push    0631EF8h                // CRuleTables_GetTHAC0
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetClassStringMixed_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]         // Class
        call    GetClassTextTemplate

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CRuleTables_GetClassStringMixed

        mov     dword ptr [ebp-84h], eax // textrefid
        add     esp, 4                   // kill ret_addr
        push    0636AFDh                 // skip error
        ret

    exit_CRuleTables_GetClassStringMixed:
        mov     dword ptr [ebp-84h], 0FFFFFFFFh  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCreateChar_ResetAbilities_RollStrengthEx_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-10h]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCreateChar_ResetAbilities_RollStrengthEx

        add     esp, 4                  // kill ret_addr
        push    071CAF2h                // skip error
        ret

    exit_CScreenCreateChar_ResetAbilities_RollStrengthEx:
        mov     edx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCreateChar_IsDoneButtonClickable_EnableMultiClassPanel_asm() {
    __asm {
        push    ecx
        push    edx

        push    eax         // class
        call    IsShamanClassByNumber

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_IsAllowedMultiClassPanel

        add     esp, 4      // kill ret_addr
        push    072B528h    // skip error
        ret

    exit_IsAllowedMultiClassPanel:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenMenu_OnLButtonClick_PrepareSkillPoints_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-1Ch]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_PrepareSkillPoints

        push    1               // nLevel
        push    [ebp-34h]       // BaseStats
        push    [ebp-24h]       // Cre
        call    SetShamanSkillPoints

        add     esp, 4                  // kill ret_addr
        push    0729C12h                // skip error
        ret

    exit_PrepareSkillPoints:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetProficiencySlots_SelectClassLevels_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_SelectClassLevels

        add     esp, 4                  // kill ret_addr
        push    062FBB5h                // skip error
        ret

    exit_SelectClassLevels:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetProficiencyClassIndex_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+4]       // caller EIP
        push    [ebp+8]       // class
        call    GetProficiencyClassIndex

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_GetProficiencyClassIndex

        // eax - nColumn 
        add     esp, 4                  // kill ret_addr
        push    062C43Bh                // skip error
        ret

    exit_GetProficiencyClassIndex:
        mov     eax, 1                  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetNumLevelUpAbilities_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_GetNumLevelUpAbilities

        add     esp, 4                  // kill ret_addr
        push    062C770h                // skip error
        ret

    exit_GetNumLevelUpAbilities:
        mov     ecx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetMaxMemorizedSpellsPriest_asm() {
    __asm {
        push    eax
        push    ecx
        push    edx

        lea     eax, [ebp-28h]  // *Result
        push    eax             
        lea     eax, [ebp-38h]  // sSpellLevel
        push    eax
        lea     eax, [ebp-48h]  // sLevel
        push    eax
        push    [ebp+0Ch]       // cds
        push    [ebp+8]         // Object
        call    GetMaxMemorizedSpellsShaman

        pop     edx
        pop     ecx
        pop     eax

        mov     dword ptr [edx], 0  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCharacter_ResetLevelUpPanel_SetSkillPoints_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-948h]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCharacter_ResetLevelUpPanel_SetSkillPoints

        mov     eax, [ebp-950h] // cds
        mov     ax, [eax]CDerivedStatsTemplate.levelPrimary
        push    eax
        push    [ebp-954h]      // BaseStats
        push    [ebp+8]         // Cre
        call    SetShamanSkillPoints

        add     esp, 4          // kill ret_addr
        push    06DEC9Ah        // skip error
        ret

    exit_CScreenCharacter_ResetLevelUpPanel_SetSkillPoints:
        mov     edx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCharacter_UpdateLevelUpPanel_SelectSkills_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-80h]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCharacter_UpdateLevelUpPanel_SelectSkills

        push    [ebp+8]     // Cre
        push    [ebp-14h]   // Panel
        push    [ebp-130h]  // Screen
        call    CScreenCharacter_UpdateLevelUpPanel_SelectSkills

        add     esp, 4                  // kill ret_addr
        push    06E2E98h                // skip error
        ret

    exit_CScreenCharacter_UpdateLevelUpPanel_SelectSkills:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCharacter_UpdateLevelUpPanel_SetFirstSkillType_asm() {
    __asm {
        push    eax
        push    ecx
        push    edx

        push    [ebp-80h]         // Object
        call    IsShamanClassByObject

        test    eax,eax
        pop     edx
        pop     ecx
        pop     eax
        jz      exit_CScreenCharacter_UpdateLevelUpPanel_SetFirstSkillType

        mov     [edx+1442h], 5   // detect illusion skill
        ret

    exit_CScreenCharacter_UpdateLevelUpPanel_SetFirstSkillType:
        mov     [edx+1442h], al  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenGenChar_ResetChooseMagePanel_SwitchToShaman_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]         // Panel
        lea     eax, [ebp-0ACh] // Object
        push    eax
        push    [ebp+0Ch]       // Cre
        push    [ebp-1E4h]      // Screen
        call    CScreenGenChar_ResetChooseMagePanel_SwitchToShaman

        pop     edx
        pop     ecx

        mov     edx, [ebp-1E4h]  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    GetXXXSpell_asm() {
    __asm {
        mov     eax, [gGetXXXSpell]
        push    eax
        ret
    }
    }


void  __declspec(naked)
GetKnownSpellIndexXXX_asm() {
__asm {
    mov     eax, [gGetKnownSpellIndexXXX]
    push    eax
    ret
}
}


    void  __declspec(naked)
    CScreenGenChar_ResetChooseMagePanel_CheckAllowedSpell_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-1F8h]          // SpellCounter        
        lea     eax, [ebp-13Ch]     // SplContainer
        push    eax
        call    IsFilteredSpellToSelect

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_CScreenGenChar_ResetChooseMagePanel_CheckAllowedSpell

        add     esp, 4                  // kill ret_addr
        push    071BBB2h                // skip spell
        ret

    exit_CScreenGenChar_ResetChooseMagePanel_CheckAllowedSpell:
        add     ecx, 4312h  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenGenChar_ResetChooseMagePanel_PushBOOKNAME_asm() {
    __asm {
        mov     eax, [esp]          // ret addr
        push    eax                 // copy ret addr
        mov     eax, [gBookName]
        mov     [esp+4], eax        // simulate "push    11989"
        ret
    }
    }


    void  __declspec(naked)
    CScreenGenChar_UpdateMainPanel_ShowProficienciesList_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-54h]   // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenGenChar_UpdateMainPanel_ShowProficienciesList

        push    [ebp-88h]   // detectillusion
        push    [ebp-58h]   // textarea
        push    [ebp-1F8h]  // screen  
        call    CScreenGenChar_UpdateMainPanel_AddDetectIllusion

        add     esp, 4      // kill ret_addr
        push    0718FCAh    // skip error
        ret

    exit_CScreenGenChar_UpdateMainPanel_ShowProficienciesList:
        mov     ecx, 1      // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenGenChar_UpdateMainPanel_SummonPopup_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-54h]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenGenChar_UpdateMainPanel_SummonPopup

        push    [ebp-1F8h]  // Screen
        push    [ebp-5Ch]   // cds
        push    [ebp-54h]   // Object
        push    [ebp+8]     // Cre
        call    SummonShamanSpellsPanel

        add     esp, 4      // kill ret_addr
        push    07179D9h    // Open Spell Select panel
        ret

    exit_CScreenGenChar_UpdateMainPanel_SummonPopup:
        mov     ecx, 1      // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenGenChar_UpdateMainPanel_ShowPriestSpellList_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-54h]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenGenChar_UpdateMainPanel_ShowPriestSpellList

        add     esp, 4                  // kill ret_addr
        push    0719265h                // Show Priest Spell List
        ret

    exit_CScreenGenChar_UpdateMainPanel_ShowPriestSpellList:
        mov     ecx, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenChooseMageSelection_AddKnownSpell_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-34h]       // Cre
        call    IsShamanClassByCreature

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_ButtonCharGenChooseMageSelection_AddKnownSpell

        push    08CC4F4h    // AddKnownSpellPriest
        ret

    exit_ButtonCharGenChooseMageSelection_AddKnownSpell:
        push    08CC524h    // AddKnownSpellMage
        ret
    }
    }


    void  __declspec(naked)
    ButtonCharGenChooseMageSelection_RemoveKnownSpell_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-34h]       // Cre
        call    IsShamanClassByCreature

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_ButtonCharGenChooseMageSelection_RemoveKnownSpell

        push    08CC63Ah    // RemoveKnownSpellPriest
        ret

    exit_ButtonCharGenChooseMageSelection_RemoveKnownSpell:
        push    08CC664h    // RemoveKnownSpellMage
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetHitPoints_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+8]         // Object
        call    IsShamanClassByObject

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_CRuleTables_GetHitPoints

        mov     byte ptr [ebp-14h], CLASS_SHAMAN_21
        mov     byte ptr [ebp-4], 1
        ret

    exit_CRuleTables_GetHitPoints:
        mov     byte ptr [ebp-14h], 2  // stolen bytes
        mov     byte ptr [ebp-4], 1
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_RollHitPoints_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+0Ch]         // nClass
        call    IsShamanClassByNumber

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_CRuleTables_RollHitPoints

        add     edx, 3A0h  // HPPRS
        ret

    exit_CRuleTables_RollHitPoints:
        add     edx, 358h  // stolen bytes, HPWAR
        ret
    }
    }


    void  __declspec(naked)
    CGameSprite_GetSkillValue_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-30h]         // Object
        call    IsShamanClassByObject

        pop     edx
        pop     ecx
        test    eax,eax
        jz      exit_CGameSprite_GetSkillValue

        ret     // keep detectIllusion

    exit_CGameSprite_GetSkillValue:
        mov     dword ptr [ebp-4], 0  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_FindSavingThrow_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp+0Ch]    // nClass
        call    IsShamanClassByNumber

        test    eax,eax
        pop     edx
        pop     ecx
        jz      exit_CRuleTables_FindSavingThrow

        add     ecx, 25Ch   // SAVEPRS
        ret

    exit_CRuleTables_FindSavingThrow:
        add     ecx, 280h   // stolen bytes, SAVEWAR
        ret
    }
    }


void  __declspec(naked)
CRuleTables_GetSavingThrow_asm() {
__asm {

    // al    - class
    // esp+8 - 

    cmp     al, CLASS_SHAMAN_21
    jnz     exit_CRuleTables_GetSavingThrow

    mov     byte ptr [esp+8], al    // CLASS_SHAMAN_21

exit_CRuleTables_GetSavingThrow:
    push    0630470h                // stolen bytes
    ret
}
}


    void  __declspec(naked)
    CScreenCharacter_UpdateExperience_asm() {
    __asm {

        push    ecx
        push    edx

        push    [ebp-2Ch]   // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCharacter_UpdateExperience

        add     esp, 4      // kill ret_addr
        push    06D8D11h    // skip error
        ret


    exit_CScreenCharacter_UpdateExperience:
        mov     eax, 1      // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenMultiPlayer_UpdateExperience_asm() {
    __asm {

        push    ecx
        push    edx

        push    [ebp-1Ch]       // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenMultiPlayer_UpdateExperience

        add     esp, 4                  // kill ret_addr
        push    0774D59h                // skip error
        ret


    exit_CScreenMultiPlayer_UpdateExperience:
        mov     edx, 1  // stolen bytes
        ret
    }
    }


void  __declspec(naked)
CRuleTables_GetNextLevelXP_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp+8]       // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CRuleTables_GetNextLevelXP

    add     esp, 4                  // kill ret_addr
    push    06333DCh                // skip error
    ret


exit_CRuleTables_GetNextLevelXP:
    mov     edx, 1  // stolen bytes
    ret
}
}


    void  __declspec(naked)
    CScreenCreateChar_CompleteCharacterWrapup_PriestSpells_asm() {
    __asm {
        push    ecx
        push    edx

        lea     eax, [ebp-28h]       // Object
        push    eax
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCreateChar_CompleteCharacterWrapup_PriestSpells

        add     esp, 4                  // kill ret_addr
        push    07251A9h                // skip error
        ret

    exit_CScreenCreateChar_CompleteCharacterWrapup_PriestSpells:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCreateChar_CompleteCharacterWrapup_MageSpells_asm() {
    __asm {
        push    ecx
        push    edx

        lea     eax, [ebp-28h]       // Object
        push    eax
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCreateChar_CompleteCharacterWrapup_MageSpells

        add     esp, 4                  // kill ret_addr
        push    0725212h                // skip error
        ret


    exit_CScreenCreateChar_CompleteCharacterWrapup_MageSpells:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


//void  __declspec(naked)
//Log_GetClass_asm() {
//__asm {
//
//    push    ecx
//    push    edx
//
//    mov     eax, [ebp+4]       // IP
//    push    eax
//    call    Log_GetClass
//
//    pop     edx
//    pop     ecx
//
//    mov     [ebp-4], ecx  // stolen bytes
//    mov     eax, [ebp-4]
//    ret
//}
//}


void  __declspec(naked)
Object_IsUsableSubClass_asm() {
__asm {

    push    ecx
    push    edx

    push    ecx         // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_Object_IsUsableSubClass

    mov     ecx, 11         // fake druid

exit_Object_IsUsableSubClass:
    mov     [ebp-38h], ecx  // stolen bytes
    mov     edx, [ebp-38h]
    ret
}
}


    void  __declspec(naked)
    CRuleTables_GetNextLevel_asm() {
    __asm {
        // esp+8 - SubClass
        // dl    - Class
        cmp     dl, CLASS_SHAMAN_21
        jnz     exit_CRuleTables_GetNextLevel

        mov     byte ptr [esp+8], dl // CLASS SHAMAN

    exit_CRuleTables_GetNextLevel:
        push    0632DACh             // CRuleTables::GetNextLevelSubClass
        ret
    }
    }


    void  __declspec(naked)
    CGameSprite_AddNewSpecialAbilities_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-4]         // Cre
        call    IsShamanClassByCreatureBase

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CGameSprite_AddNewSpecialAbilities

        push    [ebp+0Ch]   // bDisplayFeedback
        push    [ebp+8]     // old_cds
        push    [ebp-4]     // Cre
        call    CCreatureObject_AddNewSpecialAbilitiesShaman

        add     esp, 4           // kill ret_addr
        add     esp, 4           // kill "push ecx"
        push    08D2E08h
        ret

    exit_CGameSprite_AddNewSpecialAbilities:
        add     ecx, 3698h  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CRuleTables_GetClassAbilityTable_asm() {
    __asm {

        push    ecx
        push    edx

        push    [ebp+8]         // nClass
        call    IsShamanClassByNumber

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CRuleTables_GetClassAbilityTable

        call    GetShamanAbilityTable

        // eax - CLAB* table
        add     esp, 4           // kill ret_addr
        push    063ACD4h
        ret

    exit_CRuleTables_GetClassAbilityTable:
        mov     eax, 1  // stolen bytes
        ret
    }
    }


    void  __declspec(naked)
    CScreenCharacter_ResetLevelUpPanel_ShowNewPriestSpells_asm() {
    __asm {

        push    ecx
        push    edx

        push    [ebp-948h]      // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CScreenCreateChar_ResetLevelUpPanel_ShowNewPriestSpells

        mov     eax, 1    // fake priest class
        ret

    exit_CScreenCreateChar_ResetLevelUpPanel_ShowNewPriestSpells:
        push    0415BD0H  // CAIObjectType::IsUsableSubClass
        ret
    }
    }


    void  __declspec(naked)
    CScreenCharacter_UpdateMainPanel_ShowProficiencies_asm() {
    __asm {

        push    ecx
        push    edx

        push    [ebp-5Ch]      // Object
        call    IsShamanClassByObject

        test    eax, eax
        jz      exit_CScreenCharacter_UpdateMainPanel_ShowProficiencies

        push    [ebp-64h]   // cdsCurrent
        push    [ebp-74h]   // pTextArea
        push    [ebp-378h]  // Screen
        call    AddDetectIllusionText

    exit_CScreenCharacter_UpdateMainPanel_ShowProficiencies:
        pop     edx
        pop     ecx
        mov     eax, [edx+246h]  // stolen bytes
        ret
    }
    }


void  __declspec(naked)
CRuleTables_GetClassHelp_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp+8]      // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CRuleTables_GetClassHelp

    mov     eax, STRREF_SHAMAN_KIT_DESC   // Shaman Kit Description

    add     esp, 4           // kill ret_addr
    push    062BEE0h
    ret

exit_CRuleTables_GetClassHelp:
    and     ecx, 0FFh  // stolen bytes
    ret
}
}


void  __declspec(naked)
CRuleTables_GetHPCONBonusTotal_asm() {
__asm {

    push    eax
    push    ecx
    push    edx
    
    push    [ebp+8]     // Object
    call    IsShamanClassByObject

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CRuleTables_GetHPCONBonusTotal

    mov     eax, 11         // fake druid

exit_CRuleTables_GetHPCONBonusTotal:
    and     eax, 0FFh  // stolen bytes

    ret
}
}


void  __declspec(naked)
CGameSprite_CheckCombatStatsWeapon_SetTHACPenalty_asm() {
__asm {

    push    ecx
    push    edx
    
    push    [ebp-124h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_CheckCombatStatsWeapon_SetTHACPenalty

    mov     eax, 1         // IsUsableSubClass(DRUID)=1
    ret

exit_CGameSprite_CheckCombatStatsWeapon_SetTHACPenalty:
    push    0415BD0H  // stolen bytes
    ret
}
}


void  __declspec(naked)
CheckItemNotUsableByClass_asm() {
__asm {

    push    ecx
    push    edx
    
    push    [ebp+8]  // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CheckItemNotUsableByClass

    add     esp, 4       // kill ret_addr
    push    069DF92H    // Druid item restrictions
    ret

exit_CheckItemNotUsableByClass:
    mov     dword ptr [ebp-4], 0  // stolen bytes
    ret
}
}


void  __declspec(naked)
CheckItemUsableByClass_asm() {
__asm {

    push    ecx
    push    edx
    push    eax
    
    push    [ebp+0Ch]  // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     eax
    jz      exit_CheckItemUsableByClass

    push    [ebp+20h]   // baseFlags
    push    eax         // ItemNotUsableMask
    push    [ebp+1Ch]   // pErrorCode
    push    [ebp+8]     // Item
    call    CheckItemUsableByShamanClass
    pop     edx
    pop     ecx

    mov     dword ptr [ebp-8], eax
    add     esp, 4       // kill ret_addr
    push    069E150H    // exit
    ret

exit_CheckItemUsableByClass:
    pop     edx
    pop     ecx
    mov     dword ptr [ebp-8], 1  // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenWizSpell_UpdateMainPanel_SetSorcererViewMode_asm() {
__asm {

    push    ecx
    push    edx
    push    eax
    
    push    [ebp-4Ch]  // Cre
    call    IsShamanClassByCreature
    push    eax

    push    eax
    push    [ebp-0E0h]      // Screen
    call    ConfigMageScreen

    pop     eax // IsShamanClassByCreature()
    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CScreenWizSpell_UpdateMainPanel_SetSorcererViewMode

    mov     eax,1       // enable SorcererViewMode

exit_CScreenWizSpell_UpdateMainPanel_SetSorcererViewMode:
    neg     eax         // stolen bytes
    sbb     eax, eax
    inc     eax
    ret
}
}


void  __declspec(naked)
CScreenWizSpell_UpdateMainPanel_GetKnownSpellMage_asm() {
__asm {
    mov     eax, [gGetKnownSpellXXX]
    push    eax
    ret
}
}


void  __declspec(naked)
IsArcaneClass_asm() {
__asm {

    push    ecx
    push    edx
    
    push    [ebp-8]  // Object
    call    IsArcaneClassForMageScreen

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_IsArcaneClass

    mov     dword ptr [ebp-10h], 1
    ret

exit_IsArcaneClass:
    mov     dword ptr [ebp-10h], 0         // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_SorcererSpellCount_asm() {
__asm {

    push    ecx
    push    edx
    push    eax
    
    push    [ebp-30h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SorcererSpellCount

    mov     edx, [ecx+eax+87eh+4]     // Cre.MemorizedSpellsPriest[X].m_pNodeHead
    ret

exit_CGameSprite_SorcererSpellCount:
    mov     edx, [ecx+eax+942h+4]     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenWizSpell_UpdateMainPanel_GetCDSMaxMemSpells_asm() {
__asm {

    push    ecx
    push    edx
    push    eax
    
    push    [ebp-4Ch]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CScreenWizSpell_UpdateMainPanel_GetCDSMaxMemSpells

    mov     dx, [ecx+eax+7B2h]     // cds.MemInfoPriest[XXX].wMaxMemSpells
    ret

exit_CScreenWizSpell_UpdateMainPanel_GetCDSMaxMemSpells:
    mov     dx, [ecx+eax+722h]     // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_Rest_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    
    push    [ebp+8]     // bMemorizeSpells
    push    [ebp-1D0h]  // Cre
    call    CGameSprite_Rest_AddShaman

    pop     edx
    pop     ecx

    mov     byte ptr [ebp-34h], 0   // stolen bytes

    add     esp, 4                   // kill ret addr
    push    0931FD1h    
    ret
}
}


void  __declspec(naked)
CGameSprite_Spell_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-764h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_Spell_AddShaman

    mov     eax, 13h   // fake sorcerer

exit_CGameSprite_Spell_AddShaman:
    and     eax, 0FFh                   // kill ret addr
    ret
}
}


void  __declspec(naked)
CGameSprite_SorcererSpellDecrement1_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-84h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SorcererSpellDecrement1

    mov     eax, [edx+ecx+87eh+4] // MemorizedSpellsPriest.m_pNodeHead
    ret

exit_CGameSprite_SorcererSpellDecrement1:
    mov     eax, [edx+ecx+946h]   // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_SorcererSpellDecrement2_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-84h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SorcererSpellDecrement2

    cmp     dword ptr [edx+ecx+87eh+0Ch], 0 // MemorizedSpellsPriest.m_nCount 
    ret

exit_CGameSprite_SorcererSpellDecrement2:
    cmp     dword ptr [edx+ecx+94Eh], 0     // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_SpellPoint_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-5D0h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SpellPoint_AddShaman

    mov     eax, 13h   // fake sorcerer

exit_CGameSprite_SpellPoint_AddShaman:
    and     eax, 0FFh                   // kill ret addr
    ret
}
}



void  __declspec(naked)
CGameSprite_RemoveSpellsPriest_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-44h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    jz      exit_CGameSprite_RemoveSpellsPriest_AddShaman

    push    [ebp+0Ch]   // Amount
    push    [ebp+8]     // Level
    push    [ebp-44h]   // Cre
    call    CGameSprite_RemoveSpellsPriest_AddShaman

    xor     eax, eax        // force exit 

exit_CGameSprite_RemoveSpellsPriest_AddShaman:
    pop     edx
    pop     ecx
    and     eax, 0FFFFh     // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_RemoveSpell_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-0B0h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_RemoveSpell_AddShaman

    mov     eax, 13h   // fake sorcerer

exit_CGameSprite_RemoveSpell_AddShaman:
    and     eax, 0FFh                   // kill ret addr
    ret
}
}


void  __declspec(naked)
CGameSprite_SetMemorizedFlag_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-124h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SetMemorizedFlag_AddShaman

    mov     eax, 13h   // fake sorcerer

exit_CGameSprite_SetMemorizedFlag_AddShaman:
    and     eax, 0FFh                   // kill ret addr
    ret
}
}


void  __declspec(naked)
CEffectRememorizeSpell_ApplyEffect_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp+8]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CEffectRememorizeSpell_ApplyEffect_AddShaman

    mov     eax, 13h   // fake sorcerer

exit_CEffectRememorizeSpell_ApplyEffect_AddShaman:
    and     eax, 0FFh                   // kill ret addr
    ret
}
}


void  __declspec(naked)
CGameSprite_SorcererSpellRememorize1_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-84h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SorcererSpellRememorize1

    mov     eax, [edx+ecx+87eh+4] // MemorizedSpellsPriest.m_pNodeHead
    ret

exit_CGameSprite_SorcererSpellRememorize1:
    mov     eax, [edx+ecx+946h]   // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_SorcererSpellRememorize2_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-84h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_SorcererSpellRememorize2

    cmp     dword ptr [edx+ecx+87eh+0Ch], 0 // MemorizedSpellsPriest.m_nCount 
    ret

exit_CGameSprite_SorcererSpellRememorize2:
    cmp     dword ptr [edx+ecx+94Eh], 0     // stolen bytes
    ret
}
}


void  __declspec(naked)
CDerivedStats_GetPriestLevelCast_AddShaman_asm() {
__asm {

    and     eax, 0FFh
    cmp     eax, CLASS_SHAMAN_21
    jnz     exit_CDerivedStats_GetPriestLevelCast_AddShaman

    mov     eax, 11 // fake druid

exit_CDerivedStats_GetPriestLevelCast_AddShaman:
    ret
}
}


void  __declspec(naked)
CGameSprite_GetMemorizedSpellPriest_SkipSpell_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    mov     eax, [ebp+4]    // ret addr
    cmp     eax, 0787501h   // CScreenPriestSpell::UpdateMainPanel
    jnz     exit_CGameSprite_GetMemorizedSpellPriest_SkipSpell_POP

    push    [ebp-4]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_GetMemorizedSpellPriest_SkipSpell

    xor     eax, eax     // return NULL spell
    add     esp, 4
    push    08CB9FDh 
    ret

exit_CGameSprite_GetMemorizedSpellPriest_SkipSpell:
    lea     edx, [ecx+eax+87Eh]     // stolen bytes
    ret

exit_CGameSprite_GetMemorizedSpellPriest_SkipSpell_POP:
    pop     eax
    pop     edx
    pop     ecx
    lea     edx, [ecx+eax+87Eh]     // stolen bytes
    ret
}
}


void  __declspec(naked)
CInfGame_SelectToolbar_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-0Ch]  // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CInfGame_SelectToolbar_AddShaman

    mov     eax, CLASS_SHAMAN_21 // new shaman toolbar
    add     esp, 4
    push    068E632h
    ret

exit_CInfGame_SelectToolbar_AddShaman:
    and     edx, 0FFh     // stolen bytes
    ret
}
}



void  __declspec(naked)
CInfButtonArray_SetState_AddShaman_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    ecx  // State
    call    IsShamanClassByNumber

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CInfButtonArray_SetState_AddShaman

    push    [ebp-0D4h]  // ButtonArray
    call    CInfButtonArray_SetState_AddShaman

    add     esp, 4
    push    06644EDh
    ret

exit_CInfButtonArray_SetState_AddShaman:
    mov     [ebp-100h], ecx     // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_FeedBack_FindTraps_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-0E34h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_FeedBack_FindTraps

    mov     dword ptr [ebp-2Ch], STRREF_DETECTILLUSIONS
    ret

exit_CGameSprite_FeedBack_FindTraps:
    mov     dword ptr [ebp-2Ch], 50699     // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_FeedBack_StopFindTraps_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-0E34h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_FeedBack_StopFindTraps

    mov     dword ptr [ebp-30h], STRREF_STOPDETECTILLUS
    ret

exit_CGameSprite_FeedBack_StopFindTraps:
    mov     dword ptr [ebp-30h], 50700     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCharacter_OnDoneButtonClick_FakeSorcerer_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-5Ch]  // Object
    call    CScreenCharacter_OnDoneButtonClick_SwitchToShaman

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CScreenCharacter_OnDoneButtonClick_FakeSorcerer

    mov     eax, 13h   // fake sorcerer
    
exit_CScreenCharacter_OnDoneButtonClick_FakeSorcerer:
    and     eax, 0FFh     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCharacter_OnDoneButtonClick_GetXXXLevel_asm() {
__asm {
    mov     eax, [gGetXXXLevel]
    push    eax
    ret
}
}


void  __declspec(naked)
CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-5Ch]  // Object
    call    IsShamanClassByObject

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CScreenCharacter_OnDoneButtonClick_SPLSRCKN


    mov	    edx, [pRuleEx]
    lea	    edx, [edx]CRuleTablesEx.Shaman_SPLSHMKN // pRuleEx->Shaman_SPLSHMKN
    ret

exit_CScreenCharacter_OnDoneButtonClick_SPLSRCKN:

    add     edx, 598h     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCharacter_ResetChooseMagePanel_FakeSorcerer_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    lea     eax, [ebp-0B0h]  // Object
    push    eax
    call    CScreenCharacter_ResetChooseMagePanel_SwitchToShaman

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CScreenCharacter_ResetChooseMagePanel_FakeSorcerer

    mov     eax, 13h   // fake sorcerer
    
exit_CScreenCharacter_ResetChooseMagePanel_FakeSorcerer:
    and     eax, 0FFh     // stolen bytes
    ret
}
}


void  __declspec(naked)
ButtonCharacterChooseMageSelection_OnLButtonClick_AddKnownSpell_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-40h]       // Cre
    call    IsShamanClassByCreature

    pop     edx
    pop     ecx

    test    eax,eax
    jz      exit_ButtonCharacterChooseMageSelection_OnLButtonClick_AddKnownSpell


    push    08CC4F4h    // AddKnownSpellPriest
    ret


exit_ButtonCharacterChooseMageSelection_OnLButtonClick_AddKnownSpell:
    push    08CC524h    // AddKnownSpellMage
    ret

}
}


void  __declspec(naked)
ButtonCharacterChooseMageSelection_OnLButtonClick_RemoveKnownSpell_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-40h]       // Cre
    call    IsShamanClassByCreature

    pop     edx
    pop     ecx

    test    eax,eax
    jz      exit_ButtonCharacterChooseMageSelection_OnLButtonClick_RemoveKnownSpell

    push    08CC63Ah    // RemoveKnownSpellPriest
    ret


exit_ButtonCharacterChooseMageSelection_OnLButtonClick_RemoveKnownSpell:
    push    08CC664h    // RemoveKnownSpellMage
    ret

}
}


void  __declspec(naked)
CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp+8]     // Cre
    call    IsShamanClassByCreature

    test    eax,eax
    jz      exit_CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell

    push    [ebp-1F0h]           // SpellCounter
    lea     eax, [ebp-140h]      // SplContainer
    push    eax
    call    IsFilteredSpellToSelect

    test    eax,eax
    jz      exit_CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell

    pop     edx
    pop     ecx
    add     esp, 4                  // kill ret_addr
    push    06E1A1Dh                // skip spell
    ret


exit_CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell:
    pop     edx
    pop     ecx
    add     ecx, 4312h  // stolen bytes
    ret
}
}


    void  __declspec(naked)
    CScreenCharacter_ResetLevelUpPanel_SetOldSorcererLevel_asm() {
    __asm {
        push    eax
        push    ecx
        push    edx

        push    [ebp-948h]  // Object
        call    IsShamanClassByObject

        test    eax, eax
        pop     edx
        pop     ecx
        pop     eax
        jz      exit_CScreenCharacter_ResetLevelUpPanel_SetOldSorcererLevel

        mov     eax, 13h   // fake sorcerer
    
    exit_CScreenCharacter_ResetLevelUpPanel_SetOldSorcererLevel:
        and     eax, 0FFh  // stolen bytes
        ret
    }
    }


void  __declspec(naked)
CScreenCharacter_ResetChooseMagePanel_BookName_asm() {
__asm {

    push    ecx
    push    edx

    lea     eax, [ebp-0B0h]  // Object
    push    eax
    call    IsShamanClassByObject

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CScreenCharacter_ResetChooseMagePanel_BookName

    mov     dword ptr [esp+4], STRREF_SHAMAN_BOOKNAME     
    
exit_CScreenCharacter_ResetChooseMagePanel_BookName:
    lea     eax, [ebp-160h] // stolen bytes
    ret
}
}


void  __declspec(naked)
CInfButtonArray_UpdateButtons_ToolTipFindTraps_asm() {
__asm {

    push    ecx
    push    edx

    call    IsShamanClassByButtonSet

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CInfButtonArray_UpdateButtons_ToolTipFindTraps

    mov     dword ptr [esp+4], STRREF_DETECTILLUSIONS
    
exit_CInfButtonArray_UpdateButtons_ToolTipFindTraps:
    mov     eax, [ebp-58h]  // stolen bytes
    mov     edx, [eax]
    ret
}
}


void  __declspec(naked)
CInfButtonArray_UpdateButtons_ToolTipBattleSong_asm() {
__asm {

    push    ecx
    push    edx

    call    IsShamanClassByButtonSet

    test    eax, eax
    pop     edx
    pop     ecx
    jz      exit_CInfButtonArray_UpdateButtons_ToolTipBattleSong

    mov     dword ptr [esp+4], STRREF_SHAMANDANCE     
    
exit_CInfButtonArray_UpdateButtons_ToolTipBattleSong:
    mov     eax, [ebp-58h]  // stolen bytes
    mov     edx, [eax]
    ret
}
}


void  __declspec(naked)
CGameSprite_GetSecretDoorDetection_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp-10h]  // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CGameSprite_GetSecretDoorDetection

    mov     dword ptr [ebp-4], 10 // druid door detection skill
    ret
    
exit_CGameSprite_GetSecretDoorDetection:
    mov     dword ptr [ebp-4], 5 // stolen bytes
    ret
}
}


void  __declspec(naked)
CRuleTables_GetBaseLore_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    [ebp+8]  // Object
    call    IsShamanClassByObject

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CRuleTables_GetBaseLore

    mov     eax, CLASS_SHAMAN_21
    
exit_CRuleTables_GetBaseLore:
    mov     [ebp-1Ch], al // stolen bytes
    mov     byte ptr [ebp-10h], 1
    ret
}
}


    void  __declspec(naked)
    CGameSprite_RemoveNewSpecialAbilities_asm() {
    __asm {
        push    ecx
        push    edx

        push    [ebp-4]  // Cre
        call    IsShamanClassByCreatureBase

        test    eax, eax
        pop     edx
        pop     ecx
        jz      exit_CGameSprite_RemoveNewSpecialAbilities

        push    [ebp+8]     // old_cds 
        push    [ebp-4]     // Cre
        call    CCreatureObject_RemoveNewSpecialAbilitiesShaman

        add     esp, 4
        push    08D390Bh
        ret
    
    exit_CGameSprite_RemoveNewSpecialAbilities:
        add     ecx, 3698h // stolen bytes
        ret
    }
    }


void  __declspec(naked)
CRuleTables_GetNumQuickWeaponSlots_asm() {
__asm {

    push    ecx
    push    edx
    push    eax

    push    eax  // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     eax
    pop     edx
    pop     ecx
    jz      exit_CRuleTables_GetNumQuickWeaponSlots

    mov     eax, 11     // fake druid
    
exit_CRuleTables_GetNumQuickWeaponSlots:
    and     eax, 0FFh   // stolen bytes
    ret
}
}


void  __declspec(naked)
CDerivedStats_GetWizardLevel_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // nClass
    call    IsShamanClassByNumber

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CDerivedStats_GetWizardLevel

    mov     cl, 0
    mov     [ebp-4], cl     // zero WizardLevel
    ret
    
exit_CDerivedStats_GetWizardLevel:
    mov     cl, [eax+46h]   // stolen bytes
    mov     [ebp-4], cl
    ret
}
}


void  __declspec(naked)
CScreenCharacter_OnCancelButtonClick_GetXXXLevel_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-40h]       // Cre
    call    IsShamanClassByCreature

    pop     edx
    pop     ecx

    test    eax,eax
    jz      exit_CScreenCharacter_OnCancelButtonClick_GetXXXLevel

    push    04749F9h    // CDerivedStats::GetPriestLevel
    ret


exit_CScreenCharacter_OnCancelButtonClick_GetXXXLevel:
    push    04748AEh    // CDerivedStats::GetWizardLevel
    ret

}
}


void  __declspec(naked)
CScreenCharacter_OnCancelButtonClick_SPLSRCKN1_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-40h]       // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CScreenCharacter_OnCancelButtonClick_SPLSRCKN1


    mov	    edx, [pRuleEx]
    lea	    edx, [edx]CRuleTablesEx.Shaman_SPLSHMKN  // pRuleEx->Shaman_SPLSHMKN
    ret

exit_CScreenCharacter_OnCancelButtonClick_SPLSRCKN1:

    add     edx, 598h     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCharacter_OnCancelButtonClick_SPLSRCKN2_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-40h]       // Cre
    call    IsShamanClassByCreature

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CScreenCharacter_OnCancelButtonClick_SPLSRCKN2


    mov	    eax, [pRuleEx]
    lea	    eax, [eax]CRuleTablesEx.Shaman_SPLSHMKN  // pRuleEx->Shaman_SPLSHMKN
    ret

exit_CScreenCharacter_OnCancelButtonClick_SPLSRCKN2:

    add     eax, 598h     // stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCharacter_RemoveAbilities_RemoveAllSpellsPriest_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp+0Ch]   // nClass
    push    [ebp+8]     // Cre
    call    IsNeedRemoveAllSpellsPriest

    pop     edx
    pop     ecx

    test    eax,eax
    jnz     exit_CScreenCharacter_RemoveAbilities_RemoveAllSpellsPriest

    ret


exit_CScreenCharacter_RemoveAbilities_RemoveAllSpellsPriest:
    push    095056Ch    // CGameSprite::RemoveAllSpellsPriest
    ret

}
}


void  __declspec(naked)
CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp+8]       // Cre
    call    IsShamanClassByCreature

    test    eax,eax
    jz      exit_CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints

    lea     eax, [ebp-117Ch]    // cds
    push    eax
    push    [ebp+8]             // Cre
    call    CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints


exit_CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints:
    pop     edx
    pop     ecx
    mov     dword ptr [ebp-11D4h], 0    // Stolen bytes
    ret

}
}


void  __declspec(naked)
CGameEffectLevelDrain_OnAddSpecific_asm() {
__asm {

    push    ecx
    push    edx

    push    edx     // nClass
    call    IsShamanClassByNumber

    pop     edx
    pop     ecx
    test    eax,eax
    jz      exit_CGameEffectLevelDrain_OnAddSpecific

    mov     edx, 11     // fake druid

exit_CGameEffectLevelDrain_OnAddSpecific:
    and     edx, 0FFh   // Stolen bytes
    ret

}
}


void  __declspec(naked)
CGameEffectDisableSpellType_ApplyEffect_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-4]     // Object
    call    IsShamanClassByObject

    pop     edx
    pop     ecx
    test    eax,eax
    jz      exit_CGameEffectDisableSpellType_ApplyEffect

    mov     eax, 1     // priest class
    ret

exit_CGameEffectDisableSpellType_ApplyEffect:
    push    0415BD0H   // Stolen bytes
    ret

}
}


void  __declspec(naked)
GetMultiplayerClassName_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-34h]     // Object
    call    IsShamanClassByNumber

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_GetMultiplayerClassName

    mov     eax, 11     // fake druid

exit_GetMultiplayerClassName:
    and     eax, 0FFh   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenCreateChar_GetCharacterVersion_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // Object
    call    IsShamanClassByObject

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CScreenCreateChar_GetCharacterVersion

    mov     eax, 11     // fake druid

exit_CScreenCreateChar_GetCharacterVersion:
    and     eax, 0FFh   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CRuleTables_GetRaiseDeadCost_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // Object
    call    IsShamanClassByObject

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CRuleTables_GetRaiseDeadCost

    mov     eax, 11     // fake druid

exit_CRuleTables_GetRaiseDeadCost:
    and     eax, 0FFh   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenMultiPlayer_ResetViewCharacterPanel_ShowMageSpells_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-50h]     // Object
    call    IsShamanClassByObject

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CScreenMultiPlayer_ResetViewCharacterPanel_ShowMageSpells

    mov     eax, 11     // fake druid

exit_CScreenMultiPlayer_ResetViewCharacterPanel_ShowMageSpells:
    and     eax, 0FFh   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CScreenMultiPlayer_ResetViewCharacterPanel_ShowPriestSpells_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-50h]     // Object
    call    IsShamanClassByObject

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CScreenMultiPlayer_ResetViewCharacterPanel_ShowPriestSpells

    mov     eax, 11     // fake druid

exit_CScreenMultiPlayer_ResetViewCharacterPanel_ShowPriestSpells:
    and     eax, 0FFh   // Stolen bytes
    ret
}
}



void  __declspec(naked)
CGameSprite_FeedBack_StartSongText_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-0E34h]     // cre
    call    IsShamanClassByCreature

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CGameSprite_FeedBack_StartSongText

    mov     dword ptr [ebp-3Ch], STRREF_DANCINGDANCE
    ret

exit_CGameSprite_FeedBack_StartSongText:
    mov     dword ptr [ebp-3Ch], 50705   // Stolen bytes
    ret
}
}

void  __declspec(naked)
CGameSprite_FeedBack_StopSongText_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-0E34h]     // cre
    call    IsShamanClassByCreature

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CGameSprite_FeedBack_StopSongText

    mov     dword ptr [ebp-40h], STRREF_STOPDANCINGDANCE
    ret

exit_CGameSprite_FeedBack_StopSongText:
    mov     dword ptr [ebp-40h], 50706   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameEffectDisableSpellType_ApplyEffect_TextFeedBack_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+8]     // cre
    call    IsShamanDancing

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CGameEffectDisableSpellType_ApplyEffect_TextFeedBack

    add     esp, 4
    push    052BC5Dh            // skip text output
    ret

exit_CGameEffectDisableSpellType_ApplyEffect_TextFeedBack:
    mov     eax, [edx+42BAh]   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CEffectDisableButton_OnRemove_Shaman_asm() {
__asm {
    jmp CEffectDisableButton_Shaman::OnRemove
}
}


void  __declspec(naked)
CGameSprite_SetPath_CancelDance_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-50h]     // cre
    call    CancelShamanDanceIfMoving

    pop     edx
    pop     ecx
    pop     eax

    mov     [eax+33EAh], ecx   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_ExecuteAction_MakeUnselectable_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-3868h]     // cre
    call    CGameSprite_ExecuteAction_MakeUnselectable

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp-3868h]   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_ProcessPendingTriggers_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-20h]     // cre
    call    CGameSprite_ProcessPendingTriggers

    test    al, al
    pop     edx
    pop     ecx
    pop     eax
    jz      exit_CGameSprite_ProcessPendingTriggers

    xor     eax, eax
    cmp     eax, 0                      // 0 - 0, jle is TRUE
    ret

exit_CGameSprite_ProcessPendingTriggers:
    cmp     dword ptr [ecx+63D4h], 0   // Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameEffectRandomSummon_Apply_CheckLimits_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-58h]     // summon
    push    [ebp+8]       // cre
    push    [ebp-330h]    // effect
    call    CGameEffectRandomSummon_Apply_IsShamanSummonLimit

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax
    jnz     overlimit_silent_CGameEffectRandomSummon_Apply_CheckLimits

    cmp     dword ptr [ebp-24h], 5  // stolen bytes
    jl      exit_CGameEffectSummon_ApplyVisualEffect_Self

    // overlimit
    ret

overlimit_silent_CGameEffectRandomSummon_Apply_CheckLimits:
    add     esp, 4
    push    0522218h    // overlimit, no feedback
    ret

exit_CGameEffectSummon_ApplyVisualEffect_Self:
    add     esp, 4
    push    0522232h    // continue
    ret
}
}



void  __declspec(naked)
CEffectSpellOnCondition_Apply_AddCriticalHit_asm() {
__asm {

    cmp     eax, 0x0C                   // new condition
    jnz     exit_CEffectSpellOnCondition_Apply_AddCriticalHit

    mov     word ptr [ebp-48h], 04130h  // new trigger

exit_CEffectSpellOnCondition_Apply_AddCriticalHit:
    mov     [ebp-11Ch], eax   // Stolen bytes
    ret
}
}

#define CVidCell_FrameSet_asm 09CBEDAH

void  __declspec(naked)
CInfButtonArray_PreRenderButton_asm() {
__asm {
    add     esp, 4          // kill ret addr

    call    ShamanDance_InitIcon

    mov     eax, [ebp-68h]                          // buttonarray
    cmp     [eax.nButtonArrayTypeCurrentIdx], 21    // shaman set
    jnz     exit_CInfButtonArray_PreRenderButton
    mov     dx, [ebp-1Ch]                           // icon num
    cmp     dx, 22
    jl      exit_CInfButtonArray_PreRenderButton
    cmp     dx, 25
    jg      exit_CInfButtonArray_PreRenderButton

    // render from custom VidCell
    mov     dx, [ebp-1Ch]
    push    edx // nBAM
    lea     ecx, gShamanDanceIcon
    mov     eax, CVidCell_FrameSet_asm
    call    eax
    push    0FFFFFFFFh
    mov     eax, [ebp-18h]
    push    eax
    push    0
    push    0
    lea     ecx, [ebp-14h]
    push    ecx
    mov     edx, [ebp+8]
    mov     eax, [edx+4]
    push    eax
    mov     ecx, [ebp+8]
    mov     edx, [ecx]
    push    edx
    push    0
    lea     ecx, gShamanDanceIcon
    mov     eax, [ebp-68h]
    mov     edx, [gShamanDanceIcon]
    call    dword ptr [edx+8]
    push    0671BF6h
    ret

exit_CInfButtonArray_PreRenderButton:
    mov     dx, [ebp-1Ch]   // Stolen bytes
    push    edx             // Stolen bytes
    push    0671BB6h
    ret
}
}


void  __declspec(naked)
CGameEffectDisableSpellType_ApplyEffect_RemoveQuickSpells_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-4]     // Object
    call    IsShamanClassByObject

    pop     edx
    pop     ecx
    test    eax,eax
    jz      exit_CGameEffectDisableSpellType_ApplyEffect_RemoveQuickSpells

    mov     eax, 4          // non exist spelltype
    mov     [ebp-60h], eax
    ret

exit_CGameEffectDisableSpellType_ApplyEffect_RemoveQuickSpells:
    mov     eax, [edx+1Ch]  // Stolen bytes
    mov     [ebp-60h], eax
    ret

}
}

