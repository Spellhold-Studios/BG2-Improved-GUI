#include "UserPriestBook.h"

#include "stdafx.h"
#include "uicore.h"
#include "infgame.h"
#include "objcre.h"
#include "rescore.h"
#include "chitin.h"
#include "console.h"
#include "log.h"
#include "splcore.h"
#include "engpriestbk.h"

int CUIButtonPriestBook_KnownSpellOffset = 0;

CUIButtonPriestBookUp::CUIButtonPriestBookUp(CPanel& panel, ChuFileControlInfoBase& controlInfo) : CUIButton(panel, controlInfo, 1, TRUE) {}
CUIButtonPriestBookUp::~CUIButtonPriestBookUp() {}

void CUIButtonPriestBookUp::OnLClicked(POINT pt) {
	CScreenPriestBook* pPriestSpell = g_pChitin->pScreenPriestSpell;
	CPanel& panel = pPriestSpell->manager.GetPanel(2);

	CInfGame* pGame = g_pChitin->pGame;
	Enum e;

	if (pPriestSpell->nActivePlayerIdx < pGame->numInParty) {
		e = pGame->ePlayersPartyOrder[pPriestSpell->nActivePlayerIdx];
	} else {
		e = ENUM_INVALID_INDEX;
	}

	CCreatureObject* pCre;
	char threadVal;
	do {
		threadVal = pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
	} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

	if (threadVal == OBJECT_SUCCESS) {
		CUIButtonPriestBook_KnownSpellOffset = max(CUIButtonPriestBook_KnownSpellOffset - 4, 0);

		for (int i = 27; i <= 50; i++ ) {
			CUIButtonPriestBookKnownSpell& control = (CUIButtonPriestBookKnownSpell&)panel.GetUIControl(i);
			CreFileKnownSpell* kspell = pCre->GetKnownSpellPriest(pPriestSpell->currLevel, i - 27 + CUIButtonPriestBook_KnownSpellOffset);
			kspell ? control.SetSpell(kspell->name) : control.SetSpell(ResRef());
			control.SetRedraw();
		}
		pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
	} else {
		LPCTSTR lpsz = "CUIButtonPriestBookUp::OnLClicked(): GetGameObjectShare returned %d\r\n";
		console.writef(lpsz, threadVal);
		L.timestamp();
		L.appendf(lpsz, threadVal);
	}

	return;
}

CUIButtonPriestBookDn::CUIButtonPriestBookDn(CPanel& panel, ChuFileControlInfoBase& controlInfo) : CUIButton(panel, controlInfo, 1, TRUE) {}
CUIButtonPriestBookDn::~CUIButtonPriestBookDn() {}

void CUIButtonPriestBookDn::OnLClicked(POINT pt) {
	CScreenPriestBook* pPriestSpell = g_pChitin->pScreenPriestSpell;
	CPanel& panel = pPriestSpell->manager.GetPanel(2);

	CInfGame* pGame = g_pChitin->pGame;
	Enum e;

	if (pPriestSpell->nActivePlayerIdx < pGame->numInParty) {
		e = pGame->ePlayersPartyOrder[pPriestSpell->nActivePlayerIdx];
	} else {
		e = ENUM_INVALID_INDEX;
	}

	CCreatureObject* pCre;
	char threadVal;
	do {
		threadVal = pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
	} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

	if (threadVal == OBJECT_SUCCESS) {
		int nValues = pCre->KnownSpellsPriest[pPriestSpell->currLevel].GetCount() / 4;
		if (pCre->KnownSpellsPriest[pPriestSpell->currLevel].GetCount() % 4) nValues++;
		if (nValues < 6) {
			CUIButtonPriestBook_KnownSpellOffset = 0;
		} else {
			CUIButtonPriestBook_KnownSpellOffset = min(CUIButtonPriestBook_KnownSpellOffset + 4, 4 * (nValues - 6));
		}

		for (int i = 27; i <= 50; i++ ) {
			CUIButtonPriestBookKnownSpell& control = (CUIButtonPriestBookKnownSpell&)panel.GetUIControl(i);
			CreFileKnownSpell* kspell = pCre->GetKnownSpellPriest(pPriestSpell->currLevel, i - 27 + CUIButtonPriestBook_KnownSpellOffset);
			kspell ? control.SetSpell(kspell->name) : control.SetSpell(ResRef());
			control.SetRedraw();
		}

		pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
	} else {
		LPCTSTR lpsz = "CUIButtonPriestBookDn::OnLClicked(): GetGameObjectShare returned %d\r\n";
		console.writef(lpsz, threadVal);
		L.timestamp();
		L.appendf(lpsz, threadVal);
	}

	return;
}


void __stdcall
KeepSlotsWhenDrained(CCreatureObject& Cre,
                     CEffect&         eff,
                     unsigned int     CurLev,
                     unsigned int     CurrentSpells,
                     unsigned int     MaxAllowedSpells,
                     unsigned int     *BonusSpells) {
    unsigned int        Diff;
    POSITION            pos;
    CreFileMemSpell     *pMemSpell;
    CMemorizedSpellList *p;

    if (eff.effect.bFirstCall != TRUE)
        return;

    if (Cre.GetCurrentObject().GetClass() == CLASS_SORCERER)
        return;

    if (BonusSpells) {
        Diff = CurrentSpells - (MaxAllowedSpells + *BonusSpells);   // Priest
        p = & Cre.MemorizedSpellsPriest[CurLev];
    } else {
        Diff = CurrentSpells - MaxAllowedSpells;                    // Mage
        p = & Cre.MemorizedSpellsWizard[CurLev];
    }

    if (Diff == 0)
        return;

    pos = p->GetTailPosition();
    while (pos != NULL && Diff > 0) {
        pMemSpell = (CreFileMemSpell *) p->GetPrev(pos);
        if (pMemSpell) {
            if (!(pMemSpell->wFlags & 0x04) ) {     // skip already marked
                pMemSpell->wFlags |= (0x04 + 0x02); // mark as "drained + disabled"
                Diff --;
            }
        }
    }

}


void __stdcall
EffectRestorationApply(CCreatureObject& Cre, CEffect& eff) {
    POSITION            pos;
    CreFileMemSpell     *pMemSpell;
    CMemorizedSpellList *p;

    if (eff.effect.bFirstCall != TRUE)
        return;

    if (Cre.GetCurrentObject().GetClass() == CLASS_SORCERER)
        return;

    for (int CurLev = 0; CurLev < 7; CurLev++) {
        // Priest
        p = & Cre.MemorizedSpellsPriest[CurLev];
        pos = p->GetTailPosition();
        while (pos != NULL) {
            pMemSpell = (CreFileMemSpell *) p->GetPrev(pos);
            if (pMemSpell) {
                if (pMemSpell->wFlags & 0x04) {
                    pMemSpell->wFlags &= ~(0x04 + 0x02);   // remove "drained + disabled"
                }
            }
        }

        // Mage
        p = & Cre.MemorizedSpellsWizard[CurLev];
        pos = p->GetTailPosition();
        while (pos != NULL) {
            pMemSpell = (CreFileMemSpell *) p->GetPrev(pos);
            if (pMemSpell) {
                if (pMemSpell->wFlags & 0x04) {
                    pMemSpell->wFlags &= ~(0x04 + 0x02);
                }
            }
        }
    }

}


typedef struct _CNode
{
    struct _CNode*  pNext;
    struct _CNode*  pPrev;
    void*           data;
} CNode;


int __stdcall
IsDrainedSpell(CCreatureObject&  Cre,
                  unsigned char  Amount,
                  unsigned char  SpellLevel,
                  CNode&         Ptr) {
    CreFileMemSpell *pMemSpell = (CreFileMemSpell *) Ptr.data;

    if (pMemSpell->wFlags & 0x04)
        return 1;
    else
        return 0;
}


void __stdcall
CScreenPriestSpell_OnPortraitLClick_SwitchBook(int PlayerIdx) {
    CInfGame* pGame = g_pChitin->pGame;
    CScreenPriestBook& screenP = * g_pChitin->pScreenPriestSpell;
    CScreenMageBook&   screenM = * g_pChitin->pScreenWizSpell;
    Enum eChar = PlayerIdx < pGame->numInParty ? pGame->ePlayersPartyOrder[PlayerIdx] : ENUM_INVALID_INDEX;

    CCreatureObject* Cre = NULL;
    char threadVal;
    do {
        threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eChar, THREAD_ASYNCH, &Cre, INFINITE);
    } while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

    if (threadVal == OBJECT_SUCCESS) {
        if (!screenP.CanCastPriestSpells(Cre)) {
            if (screenM.CanCastMageSpells(Cre)) {
                screenP.OnLeftPanelButtonClick(5); // emulate wizbook click
            }
        }

        threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eChar, THREAD_ASYNCH, INFINITE);
    }
}


void __stdcall
CScreenWizSpell_OnPortraitLClick_SwitchBook(int PlayerIdx) {
    CInfGame* pGame = g_pChitin->pGame;
    CScreenPriestBook& screenP = * g_pChitin->pScreenPriestSpell;
    CScreenMageBook&   screenM = * g_pChitin->pScreenWizSpell;
    Enum eChar = PlayerIdx < pGame->numInParty ? pGame->ePlayersPartyOrder[PlayerIdx] : ENUM_INVALID_INDEX;

    CCreatureObject* Cre = NULL;
    char threadVal;
    do {
        threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eChar, THREAD_ASYNCH, &Cre, INFINITE);
    } while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

    if (threadVal == OBJECT_SUCCESS) {
        if (!screenM.CanCastMageSpells(Cre)) {
            if (screenP.CanCastPriestSpells(Cre)) {
                screenM.OnLeftPanelButtonClick(6); // emulate priest click
            }
        }

        threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eChar, THREAD_ASYNCH, INFINITE);
    }
}


void __declspec(naked)
KeepPriestSlotsWhenDrained_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-1190h]
    push    eax             ; pBonusSpells
    push    [ebp-11BCh]     ; MaxAllowedSpells
    push    [ebp-118Ch]     ; CurrentSpells
    push    [ebp-1198h]     ; SpellLevel
    push    [ebp-1424h]     ; CEffect
    push    [ebp+8]         ; Cre
    call    KeepSlotsWhenDrained

    pop     edx
    pop     ecx
    pop     eax

    mov     edx, [ebp-1198h]    ; Stolen bytes
    ret
}
}

void __declspec(naked)
KeepMageSlotsWhenDrained_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    0               ; pBonusSpells
    push    [ebp-11BCh]     ; MaxAllowedSpells
    push    [ebp-118Ch]     ; CurrentSpells
    push    [ebp-1198h]     ; SpellLevel
    push    [ebp-1424h]     ; CEffect
    push    [ebp+8]         ; Cre
    call    KeepSlotsWhenDrained

    pop     edx
    pop     ecx
    pop     eax

    mov     edx, [ebp-1198h]    ; Stolen bytes
    ret
}
}

#define CheckQuickLists              8C98DEh
#define RemoveSpellsPriest_Mode     -20h
#define RemoveSpellsPriest_Cre      -44h
#define RemoveSpellsPriest_Amount    0Ch
#define RemoveSpellsPriest_Level     08h
#define RemoveSpellsPriest_pos      -08h


void __declspec(naked)
RemoveSpellsPriest_asm() {
__asm
{
#define RemoveSpellsPriest_ContinueRemoveSpell     8D5F88h
#define RemoveSpellsPriest_SkipRemoveSpell         8D5FD4h


    push    1
    push    0
    push    0FFFFFFFFh
    lea     ecx, [ebp+RemoveSpellsPriest_Mode]
    push    ecx
    mov     ecx, [ebp+RemoveSpellsPriest_Cre]
    mov     eax, CheckQuickLists
    call    eax

    push    [ebp+RemoveSpellsPriest_pos]       ; PtrList position    
    push    [ebp+RemoveSpellsPriest_Level]     ; SpellLevel
    push    [ebp+RemoveSpellsPriest_Amount]    ; Amount
    push    [ebp+RemoveSpellsPriest_Cre]       ; Cre
    call    IsDrainedSpell
    test    eax,eax
    pop     eax
    jnz     L3_SkipRemoveSpell

    mov     eax, RemoveSpellsPriest_ContinueRemoveSpell    ; orig remove code
    jmp     eax

L3_SkipRemoveSpell:
    mov     eax, RemoveSpellsPriest_SkipRemoveSpell        ; skip removing
    jmp     eax
    
    ret
}
}




#define RemoveSpellsMage_Mode     -2Ch
#define RemoveSpellsMage_Cre      -64h
#define RemoveSpellsMage_Amount    0Ch
#define RemoveSpellsMage_Level     08h
#define RemoveSpellsMage_pos      -08h

void __declspec(naked)
RemoveSpellsMage_asm() {
__asm
{
#define RemoveSpellsMage_ContinueRemoveSpell     8D5D5Bh
#define RemoveSpellsMage_SkipRemoveSpell         8D5DBBh


    push    1
    push    0
    push    0FFFFFFFFh
    lea     ecx, [ebp+RemoveSpellsMage_Mode]
    push    ecx
    mov     ecx, [ebp+RemoveSpellsMage_Cre]
    mov     eax, CheckQuickLists
    call    eax

    push    [ebp+RemoveSpellsMage_pos]       ; PtrList position    
    push    [ebp+RemoveSpellsMage_Level]     ; SpellLevel
    push    [ebp+RemoveSpellsMage_Amount]    ; Amount
    push    [ebp+RemoveSpellsMage_Cre]       ; Cre
    call    IsDrainedSpell
    test    eax,eax
    pop     eax
    jnz     L2_SkipRemoveSpell

    mov     eax, RemoveSpellsMage_ContinueRemoveSpell    ; orig remove code
    jmp     eax

L2_SkipRemoveSpell:
    mov     eax, RemoveSpellsMage_SkipRemoveSpell        ; skip removing
    jmp     eax
    
    ret
}
}


void __declspec(naked)
EffectRestorationApply_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-0Ch]   ; CEffect
    push    [ebp+8]     ; Cre
    call    EffectRestorationApply

    pop     edx
    pop     ecx

    mov     eax, 1    ; Stolen bytes
    ret
}
}


void __declspec(naked)
CScreenPriestSpell_OnPortraitLClick_SwitchBook_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp+8]     // PlayerIdx
    call    CScreenPriestSpell_OnPortraitLClick_SwitchBook

    pop     edx
    pop     ecx

    add     esp, 4
    mov     esp, ebp    // Stolen bytes
    pop     ebp
    ret     4
}
}


void __declspec(naked)
CScreenWizSpell_OnPortraitLClick_SwitchBook_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp+8]     // PlayerIdx
    call    CScreenWizSpell_OnPortraitLClick_SwitchBook

    pop     edx
    pop     ecx

    add     esp, 4
    mov     esp, ebp    // Stolen bytes
    pop     ebp
    ret     4
}
}