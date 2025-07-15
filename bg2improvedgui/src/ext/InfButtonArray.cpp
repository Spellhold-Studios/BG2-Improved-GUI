#include "InfButtonArray.h"

#include "chitin.h"
#include "ObjectStats.h"
#include "InfGameCommon.h"

BOOL (CButtonArray::*Tramp_CButtonArray_CheckButtonEnabled)(int) =
	SetFP(static_cast<BOOL (CButtonArray::*)(int)>	(&CButtonArray::CheckButtonEnabled),	0x671C9E);

BOOL DETOUR_CButtonArray::DETOUR_CheckButtonEnabled(int nIdx) {
	if (0) IECString("DETOUR_CButtonArray::DETOUR_CheckButtonEnabled");

	unsigned short wStatBtIdx = USHRT_MAX;
	switch (nIdx) {
	case CBUTTONARRAY_BTID_TALK:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_TALK;
		break;
	case CBUTTONARRAY_BTID_STEALTH:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_STEALTH;
		break;
	case CBUTTONARRAY_BTID_THIEVING:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_THIEVING;
		break;
	case CBUTTONARRAY_BTID_CASTSPELL:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_CASTSPELL;
		break;
	case CBUTTONARRAY_BTID_QUICKITEM1:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKITEM1;
		break;
	case CBUTTONARRAY_BTID_QUICKITEM2:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKITEM2;
		break;
	case CBUTTONARRAY_BTID_QUICKITEM3:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKITEM3;
		break;
	case CBUTTONARRAY_BTID_TURNUNDEAD:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_TURNUNDEAD;
		break;
	case CBUTTONARRAY_BTID_QUICKSPELL1:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKSPELL1;
		break;
	case CBUTTONARRAY_BTID_QUICKSPELL2:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKSPELL2;
		break;
	case CBUTTONARRAY_BTID_QUICKSPELL3:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_QUICKSPELL3;
		break;
	case CBUTTONARRAY_BTID_USEITEM:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_USEITEM;
		break;
	case CBUTTONARRAY_BTID_ABILITY:
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_ABILITY;
		break;
	case CBUTTONARRAY_BTID_BARDSONG: //new
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_BARDSONG;
		break;
	case CBUTTONARRAY_BTID_FINDTRAPS: //new
		wStatBtIdx = CDERIVEDSTATS_BUTTONDISABLE_FINDTRAPS;
		break;
	default:
		return TRUE;
		break;
	}

	if (g_pChitin->pGame->m_PartySelection.memberList.GetCount() == 0) return TRUE;

	Enum e = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
	CCreatureObject* pCre = NULL;
	
	char nResult;
	do {
		nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
	} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
	if (nResult != OBJECT_SUCCESS) return TRUE;

	BOOL bButtonAllowed = TRUE;
	if (wStatBtIdx < 14) bButtonAllowed = !pCre->GetDerivedStats().ButtonDisable[wStatBtIdx];
	if (wStatBtIdx == 14) bButtonAllowed = !pCre->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_BUTTONDISABLEFINDTRAPS);

	g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
	return bButtonAllowed;
}


BOOL static __stdcall
CInfButtonArray_HasInnates(CCreatureObject& Cre) {
    POSITION pos;
    CreFileMemSpell* pMemSpell;
    bool bFound = false;

    pos = Cre.MemorizedSpellsInnate[0].GetHeadPosition();
    while (pos) {
        pMemSpell = (CreFileMemSpell*)Cre.MemorizedSpellsInnate[0].GetNext(pos);
        DWORD* pSpellName = (DWORD*) &pMemSpell->name;  // first 4 char
        if (*pSpellName != 'RPPS' &&                        // skip SPPR*(HLA)
            ( *pSpellName != 'IWPS' ||                      // skip SPWI*(HLA)
               (pMemSpell->name == ResRef("SPWI420D") ||    // Minor Sequencer Innate
                pMemSpell->name == ResRef("SPWI710D") ||    // Spell Sequencer Innate
                pMemSpell->name == ResRef("SPWI809D"))      // Spell Trigger Innate
            ) &&
            pMemSpell->wFlags & CREMEMSPELL_MEMORIZED) {
            return TRUE;
        }
    }
    
    return FALSE;
}

typedef struct _CNode
{
    struct _CNode*  pNext;
    struct _CNode*  pPrev;
    CButtonData*   data;
} CNode;

#define InfoPanelPriestJournal  0
#define InfoPanelInnate         1
#define InfoPanelSpell          2
#define InfoPanelItemAbility    3
#define InfoPanelSpellMenu      4
#define InfoPanelSelectOpcode   5

int     g_PriestInfoMode = InfoPanelPriestJournal;
int     g_CurentPageIndex = 0;
ResRef  g_OverideIcon;
STRREF  g_PriestInfoCustomText;
STRREF  g_PriestInfoCustomHeaderText;

void 
SummonPriestInfoPanel(ResRef* Name, int ToolBarType, CButtonData* ButtonData) {
    CScreenPriestBook& ScreenPriest = * g_pChitin->pScreenPriestSpell;
    CManager& Manager = ScreenPriest.GetManager();
#ifndef _DEBUG
    CSingleLock Lock((CSyncObject*) &Manager.u36, 0);
    Lock.Lock(-1);
#endif

    g_PriestInfoMode = ToolBarType;
    g_CurentPageIndex = g_pChitin->pGame->m_CButtonArray.PageShifted;
    g_PriestInfoCustomText = 0;
    g_PriestInfoCustomHeaderText = 0;

    if (ToolBarType == InfoPanelInnate ||
        ToolBarType == InfoPanelSelectOpcode) {
        g_OverideIcon = ButtonData->rUsageIcon;
        if (Name->IsValid())
            ScreenPriest.CurrentSpell = *Name;
        else
            ScreenPriest.CurrentSpell = "";
    } else
    if (ToolBarType == InfoPanelSpell) {
        if (Name->IsValid())
            ScreenPriest.CurrentSpell = *Name;
        else
            ScreenPriest.CurrentSpell = "";
    } else
    if (ToolBarType == InfoPanelItemAbility) {
        ResRef NameResource = "";
        int Slot =        ButtonData->abilityId.wItemSlotIdx;
        int nAbilityIdx = ButtonData->abilityId.wItemAbilityIdx;
        int AbilityColumn = -1;

        Enum e = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
        CCreatureObject* pCre = NULL;
        char nResult;
        do {
            nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
        } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

        if (nResult == OBJECT_SUCCESS) {
            CItem* Item = pCre->Inventory.items[Slot];
            if (Item) {
                Item->Demand();

                for (int abil_idx = 0; abil_idx <= nAbilityIdx; abil_idx++) {
                    ItmFileAbility* ability = Item->GetAbility(abil_idx);
                    if (&ability) {
                        if (ability->quickSlotType == 3) { // item location
                            AbilityColumn++;
                        }
                    }
                }

                bool bFoundStrRef = false;
                if (pRuleEx->SpellInfo_ABILDESC.m_2da.bLoaded) {
                    POINT  pos;
                    IECString restring = Item->itm.name.GetResRefStr();

                    if (pRuleEx->SpellInfo_ABILDESC.FindString(restring, &pos, TRUE)) {
                        POINT pos0 = {AbilityColumn, pos.y};
                        IECString& sAbil = pRuleEx->SpellInfo_ABILDESC.GetValue(pos0);
                                
                        long strref = 0;
                        if (!sAbil.IsEmpty())
		                    sscanf_s((LPCTSTR)sAbil, "%d", &strref);

                        if (strref != -1 && strref != 0) {
                            g_PriestInfoCustomText       = strref;
                            g_PriestInfoCustomHeaderText = ButtonData->strrefName;  // button name
                            NameResource = "ABILDESC"; // fake spell name
                            bFoundStrRef = true;
                        }
                    }
                }

                if (!bFoundStrRef) {
                    // find spell cast if exist
                    ItmFileAbility* ability = Item->GetAbility(nAbilityIdx);
                    for (int i = 0; i <= (ability->nEffects - 1); i++ ) {
                        CEffect* Eff = Item->GetAbilityEffect(nAbilityIdx, i, pCre);
                        if (Eff == NULL) {
                            continue;
                        }

                        if (Eff->effect.nOpcode == CEFFECT_OPCODE_CAST_SPELL ||
                            Eff->effect.nOpcode == CEFFECT_OPCODE_CAST_SPELL_POINT) {
                            NameResource = Eff->effect.rResource;
                            break;
                        }
                    }
                    if (NameResource.IsEmpty()) {
                        g_PriestInfoCustomText = 30;
                        g_PriestInfoCustomHeaderText = ButtonData->strrefName;  // button name
                        NameResource = "ABILDESC"; // fake spell name

                    }
                }

                Item->Demand();
            }

            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
        }

        g_OverideIcon = ButtonData->rUsageIcon;
        ScreenPriest.CurrentSpell = NameResource;
    } else
    if (ToolBarType == InfoPanelSpellMenu) {
        if (Name->IsValid())
            ScreenPriest.CurrentSpell = *Name;
        else
            ScreenPriest.CurrentSpell = "";
    }

    g_pChitin->pScreenWorld->SelectEngine(ScreenPriest);
    ScreenPriest.SummonPopup(3);   // InfoPanel
#ifndef _DEBUG
    Lock.Unlock();
#endif
}


void static
CUIButtonQuickPanel_OnRClicked(CUIButton *Button, int X, int Y, int ToolBarType) {
    CPtrList* g_pCQuickObjectList = (CPtrList*) *( (DWORD *)0xB83D0C);
    CButtonArray* ButtonArray = & g_pChitin->pGame->m_CButtonArray;

    POSITION pos = g_pCQuickObjectList->FindIndex(ButtonArray->PageShifted);
    int Index = 0;

    if (g_pCQuickObjectList->GetCount() > 12) Index = 1;    // Toolbar has page up/down buttons
    if (ButtonArray->nButtonArrayTypePreviousIdx == 15) Index++;

    CNode* pNode = NULL;
    while (pos != NULL) {
        if (Index == Button->index) {
            pNode = (CNode *) pos;
            break;
        } else {
            g_pCQuickObjectList->GetNext(pos);
            Index++;
        }
    }

    if (pNode) {
        SummonPriestInfoPanel(& pNode->data->abilityId.rSpellName, ToolBarType, pNode->data);
    }
}


void static __stdcall 
CUIControlButtonAction_OnRButtonClick(CUIButton *Button, int X, int Y) {
    CButtonArray* ButtonArray = & g_pChitin->pGame->m_CButtonArray;

    //if (ButtonArray->buttons[Button->index].bDisabled) {
    //    ButtonArray->OnRButtonPressed(Button->index);
    //    return;
    //}

    if (ButtonArray->nButtonArrayTypeCurrentIdx == 106) { // Innate bar
        CUIButtonQuickPanel_OnRClicked(Button, X, Y, InfoPanelInnate);
        return;
    }

    if (ButtonArray->nButtonArrayTypeCurrentIdx == 103) { // Spell bar
        CUIButtonQuickPanel_OnRClicked(Button, X, Y, InfoPanelSpell);
        return;
    }

    if (ButtonArray->nButtonArrayTypeCurrentIdx == 105) { // Item Ability bar
        CUIButtonQuickPanel_OnRClicked(Button, X, Y, InfoPanelItemAbility);
        return;
    }

    if (ButtonArray->nButtonArrayTypeCurrentIdx == 111) { // Select Spell Opcode
        CUIButtonQuickPanel_OnRClicked(Button, X, Y, InfoPanelSelectOpcode);
        return;
    }

    // default    
    ButtonArray->OnRButtonPressed(Button->index);
}

extern int gPrev_ToolbarState;
extern int gPrev_ToolbarBook;
BOOL __stdcall OpenSpellToolbar(CButtonArray* Array, int* nState, int SavedBook);

void static __stdcall 
CloseInfoPanel() {
    CButtonArray* ButtonArray = & g_pChitin->pGame->m_CButtonArray;
    if (g_PriestInfoMode == InfoPanelInnate) {
        int ButtonIndex = 0xB;      // Innate Toolbar Button

        g_pChitin->pScreenPriestSpell->SelectEngine(* g_pChitin->pScreenWorld);
        ButtonArray->nActiveButtonIdx = ButtonArray->nButtonIdx[ButtonIndex];

        ButtonArray->SetState(106); // 106 - Innate Toolbar
        ButtonArray->PageShifted = g_CurentPageIndex;
        ButtonArray->UpdateButtons();

        g_PriestInfoMode = InfoPanelPriestJournal;
    } else
    if (g_PriestInfoMode == InfoPanelSpell) {
        int ButtonIndex = 0x6;      // Spell Toolbar Button

        g_pChitin->pScreenPriestSpell->SelectEngine(* g_pChitin->pScreenWorld);
        ButtonArray->nActiveButtonIdx = ButtonArray->nButtonIdx[ButtonIndex];

        ButtonArray->SetState(103); // 103 - Spell Toolbar
        ButtonArray->PageShifted = g_CurentPageIndex;
        ButtonArray->UpdateButtons();

        g_PriestInfoMode = InfoPanelPriestJournal;
    } else
    if (g_PriestInfoMode == InfoPanelItemAbility) {
        int ButtonIndex = 0x7;      // Item Ability Button

        g_pChitin->pScreenPriestSpell->SelectEngine(* g_pChitin->pScreenWorld);
        ButtonArray->nActiveButtonIdx = ButtonArray->nButtonIdx[ButtonIndex];

        ButtonArray->SetState(105); // 105 - Item Ability Toolbar
        ButtonArray->PageShifted = g_CurentPageIndex;
        ButtonArray->UpdateButtons();

        g_PriestInfoMode = InfoPanelPriestJournal;
    } else
    if (g_PriestInfoMode == InfoPanelSpellMenu) {
        g_pChitin->pScreenPriestSpell->SelectEngine(* g_pChitin->pScreenWorld);
        OpenSpellToolbar(ButtonArray, &gPrev_ToolbarState, gPrev_ToolbarBook);
        ButtonArray->UpdateButtons();

        g_PriestInfoMode = InfoPanelPriestJournal;
    } else
    if (g_PriestInfoMode == InfoPanelSelectOpcode) {

        g_pChitin->pScreenPriestSpell->SelectEngine(* g_pChitin->pScreenWorld);
        ButtonArray->nActiveButtonIdx = 100;

        ButtonArray->SetState(111); // 111 - Select Opcode
        ButtonArray->PageShifted = g_CurentPageIndex;
        ButtonArray->UpdateButtons();

        g_PriestInfoMode = InfoPanelPriestJournal;
    }

    // ModalState ?
    //Enum e = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
    //CCreatureObject* pCre = NULL;
    //char nResult;
    //do {
    //    nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
    //} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

    //if (nResult == OBJECT_SUCCESS) {
    //    pCre->SetModalState(0, 0); // modalState=0, bUpdateToolbar=0
    //}
    //g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
}


void static __stdcall
CScreenPriestSpell_CResRefAssign(ResRef& ResRef_Button, ResRef& tmp, IECString& Name_B) {
    if (g_PriestInfoMode == InfoPanelInnate ||
        g_PriestInfoMode == InfoPanelItemAbility) {
        ResRef_Button = g_OverideIcon;
    } else {    // InfoPanelSpell, InfoPanelSpellMenu
        // if icon with 'B' at end exist, use it as prefered
        if (!ResRef_Button.IsEmpty()) {
            if (&g_pChitin->m_ResHandler.m_KeyTable.FindKey(ResRef(Name_B), CRES_TYPE_BAM, FALSE)) {
                ResRef_Button = Name_B;
            }
        }
    }
}


void static __stdcall
CScreenPriestSpell_UpdateInfoPanel_ChangeTittle(CScreenPriestBook& screen, CPanel& panel, STRREF ref ) {
    IECString sTextTitle = GetTlkString(ref);
    UpdateLabel(screen,
                panel,
                0x10000000 + 0xFFFF,
                "%s",
                (LPCTSTR)sTextTitle);
}


void static __stdcall
PostRender(uint ButtonIndex, POINT &Location, RECT &ClipControl) {
    CButtonArray* ButtonArray = & g_pChitin->pGame->m_CButtonArray;
    int bDoubleResolution = g_pChitin->bDoubleResolution;
    int i;
    //POINT Rhomb2[4];

    //if (ButtonIndex != 5)
    //    return;

    if (ButtonArray->nButtonIdx[ButtonIndex] != 11) // Hide in Shadow
        return;

    //if (ButtonArray->buttons[ButtonIndex].bDisabled)
    //    return;

    if (!ButtonArray->buttons[ButtonIndex].bEnabled)
        return;

    console.write_debug("Render ButtonIndex=%d \n", ButtonIndex);
    console.write_debug("Render nButtonIdx=%d \n", ButtonArray->nButtonIdx[ButtonIndex]);
    console.write_debug("Render nButtonArrayTypeCurrentIdx=%d \n", ButtonArray->nButtonArrayTypeCurrentIdx);
    console.write_debug("Render nActiveButtonIdx=%d \n", ButtonArray->nActiveButtonIdx);

    CVideoMode* pVideoMode;

    if ( g_pChitin->pEngineActive )
        pVideoMode = g_pChitin->pEngineActive->pVideoMode;
    else
        pVideoMode = NULL;

    POINT offset;
    offset.x = 0;
    offset.y = 0;

    #define ICON_SIZE_Y 36
    #define ICON_SIZE_X 36

    //    Rhomb2[0].x = Location.x + (1 + bDoubleResolution)*2;      
    //    Rhomb2[1].x = Location.x + (1 + bDoubleResolution)*2 + ICON_SIZE_X; 
    //    Rhomb2[2].x = Location.x + (1 + bDoubleResolution)*2 + ICON_SIZE_X;
    //    Rhomb2[3].x = Location.x + (1 + bDoubleResolution)*2;      

    //    Rhomb2[0].y = Location.y + (1 + bDoubleResolution)*2;
    //    Rhomb2[1].y = Location.y + (1 + bDoubleResolution)*2;
    //    Rhomb2[2].y = Location.y + (1 + bDoubleResolution)*2 + ICON_SIZE_Y;
    //    Rhomb2[3].y = Location.y + (1 + bDoubleResolution)*2 + ICON_SIZE_Y;

    //pVideoMode->FillPoly3d(
    //    Rhomb2,
    //    4,
    //    ClipControl,
    //    0x008000,
    //    offset);

    for (i = ICON_SIZE_Y; i > 18; i --) {
        //  0---1
        //  |   |   36*36
        //  |   |
        //  3---2
        //Rhomb2[0].x = Location.x + (1 + bDoubleResolution)*2;      
        //Rhomb2[1].x = Location.x + (1 + bDoubleResolution)*2 + ICON_SIZE_X; 
        //Rhomb2[2].x = Location.x + (1 + bDoubleResolution)*2 + ICON_SIZE_X;
        //Rhomb2[3].x = Location.x + (1 + bDoubleResolution)*2;      

        //Rhomb2[0].y = Location.y + (1 + bDoubleResolution)*2 + i - 1;
        //Rhomb2[1].y = Location.y + (1 + bDoubleResolution)*2 + i - 1;
        //Rhomb2[2].y = Location.y + (1 + bDoubleResolution)*2 + i;
        //Rhomb2[3].y = Location.y + (1 + bDoubleResolution)*2 + i;

        //pVideoMode->OutlinePoly(
        //    Rhomb2,
        //    4,
        //    ClipControl,
        //    0x00a000,
        //    offset);

        pVideoMode->DrawLine3d(
            Location.x + (1 + bDoubleResolution)*2,                 // x0
            Location.y + (1 + bDoubleResolution)*2 + i,             // y0
            Location.x + (1 + bDoubleResolution)*2 + ICON_SIZE_X,   // x1
            Location.y + (1 + bDoubleResolution)*2 + i,             // y1
            ClipControl,
            0x00a000);
    }
}


void  __stdcall
CInfButtonArray_UpdateButtons_HideWizardSpellFromQuickList(CButtonArray& ba, int nButton, CButtonData* Button) {

	if (g_pChitin->pGame->m_PartySelection.memberList.GetCount() == 0)
        return;

	Enum e = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
	CCreatureObject* Cre = NULL;
	
	char nResult;
	do {
		nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &Cre, INFINITE);
	} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
    if (nResult == OBJECT_SUCCESS) {
        ResRef tmp = Button->abilityId.rSpellName;
        tmp.MakeUpper();
        if (Cre->cdsCurrent.ButtonDisableSpl[0] == 1 && // wizard spells disabled
            memcmp(tmp.GetResRef(), "SPWI", 4) == 0) {
            ba.buttons[nButton].bDisabled = 1;
        }

        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
    }

}


void __declspec(naked)
CInfButtonArray_UpdateButtons_CheckInnateList_asm() {
__asm
{
    push    ecx
    push    edx

    push    ecx   // Cre
    call    CInfButtonArray_HasInnates
    
    ; eax - result
    pop     edx
    pop     ecx
    ret     8
}
}


void __declspec(naked)
ClericThief_TurnUndeadInnate_asm() {
__asm
{
    mov     [ebp-74Ch], edx // stolen bytes

    cmp     edx, 1      // Turn Undead
    jnz     ClericThief_TurnUndeadInnate_continue

    add     esp, 4      // kill ret addr
    push    066E63FH    // non-innate toolbar route
    ret

ClericThief_TurnUndeadInnate_continue:
    ret
}
}


void __declspec(naked)
CUIControlButtonAction_OnRButtonClick_asm() {
__asm
{
    push    [esp+8]
    push    [esp+8]
    push    ecx
    call    CUIControlButtonAction_OnRButtonClick

    ret     12
}
}


void __declspec(naked)
CScreenPriestSpell_OnDoneButtonClick_asm() {
__asm
{
    // orig code
    mov     ecx, [ebp-30h]
    mov     eax, 07879BCh   // CScreenPriestSpell__DismissPopup
    call    eax

    // 
    call    CloseInfoPanel

    ret
}
}


void __declspec(naked)
CScreenPriestSpell_EscapeKeyDown_asm() {
__asm
{
    // orig code
    mov     ecx, [ebp-28h]
    mov     eax, 07879BCh   // CScreenPriestSpell__DismissPopup
    call    eax

    // 
    call    CloseInfoPanel

    ret
}
}


void __declspec(naked)
CScreenPriestSpell_IconAssign_asm() {
__asm
{
    cmp     dword ptr [g_PriestInfoMode], InfoPanelPriestJournal
    jnz     CScreenPriestSpell_IconAssign_asm_CustomMode

    // orig code
    push    099A4A5h   // CResRef::operator=(CString const &)
    ret

CScreenPriestSpell_IconAssign_asm_CustomMode:
    push    [esp+8] // cstring src
    push    [esp+8] // resref  tmp
    push    ecx     // resref  dst
    call    CScreenPriestSpell_CResRefAssign

    ret     8
}
}


void __declspec(naked)
CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm() {
__asm
{
    push    edx
    push    ecx

    cmp     dword ptr [g_PriestInfoMode], InfoPanelPriestJournal
    jz      CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm_SpellMode

    cmp     dword ptr [g_PriestInfoMode], InfoPanelSpellMenu
    jz      CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm_SpellMode

    push    13707       // ~Information~
    push    [ebp+8]     // panel
    push    [ebp-50h]   // screen
    call    CScreenPriestSpell_UpdateInfoPanel_ChangeTittle

    pop     ecx
    pop     edx

    cmp     dword ptr [g_PriestInfoCustomHeaderText], 0
    jz      CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm_Skip

    mov     eax, g_PriestInfoCustomHeaderText
    ret

CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm_SpellMode:
    push    16189       // restore original ~SPELL INFORMATION~
    push    [ebp+8]     // panel
    push    [ebp-50h]   // screen
    call    CScreenPriestSpell_UpdateInfoPanel_ChangeTittle

    pop     ecx
    pop     edx

CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm_Skip:
    // orig code
    push    06436A4h   // CSpell::GetGenericName
    ret
}
}


void __declspec(naked)
CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm() {
__asm
{
    cmp     dword ptr [g_PriestInfoMode], InfoPanelPriestJournal
    jz      CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm_Skip
    cmp     dword ptr [g_PriestInfoCustomText], 0
    jz      CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm_Skip

    mov     eax, g_PriestInfoCustomText
    ret

CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm_Skip:
    // orig code
    push    0643728h   // CSpell::GetDescription
    ret
}
}


void __declspec(naked)
CScreenPriestSpell_LoadIconSpell_asm() {
__asm
{
    cmp     dword ptr [ebp-18h], 000B72024h // empty string
    jz      CScreenPriestSpell_LoadIconSpell_asm_EmptyString

    mov     edx, [ebp-18h]  // stolen bytes
    mov     eax, [edx-8]
    ret

CScreenPriestSpell_LoadIconSpell_asm_EmptyString:
    add     esp, 4
    push    078A2D3h        // skip  CString::SetAt()
    ret
}
}


void __declspec(naked)
CInfButtonArray_PostRenderButton_asm() {
__asm
{
    push    eax
    push    edx
    push    ecx

    push    [ebp+0Ch]   ; ClipControl
    push    [ebp+8]     ; RenderLocation
    push    [ebp+14h]   ; ButtonNum
    call    PostRender

    pop     ecx
    pop     edx
    pop     eax

    lea     ecx, [ebp-0F4h]  // stolen bytes
    ret
}
}


void  __declspec(naked)
CInfButtonArray_UpdateButtons_HideWizardSpellFromQuickList_asm() {
__asm {

    lea     eax, [ebp-98h]  // Spell resref   
    push    eax
    push    [ebp-9Ch]       // nButton
    push    [ebp-0A34h]     // ButtonArray
    call    CInfButtonArray_UpdateButtons_HideWizardSpellFromQuickList

    mov     edx, [ebp-600h]  // stolen bytes
    ret
}
}