#include "DialogCore.h"

#include "chitin.h"
#include "ScriptAction.h"

void __stdcall CDlgResponse_QueueActions(CDlgResponse& cdResponse, CCreatureObject& cre) {
	if (0) IECString("CDlgResponse_QueueActions");

	if (cdResponse.m_response.m_actions.GetCount()) {
		if (!cdResponse.bAddedInterrupts) {
			cdResponse.bAddedInterrupts = TRUE;

			//new - add ClearVariables
			Action* pAClearBlockVars = new Action();
			pAClearBlockVars->opcode = ACTION_CLEAR_BLOCK_VARIABLES;
			cdResponse.m_response.m_actions.AddHead(pAClearBlockVars);

			Action* pAInterruptFalse = new Action();
			pAInterruptFalse->opcode = ACTION_SET_INTERRUPT;
			pAInterruptFalse->m_specificID = 0;
			cdResponse.m_response.m_actions.AddHead(pAInterruptFalse);

			Action* pAInterruptTrue = new Action();
			pAInterruptTrue->opcode = ACTION_SET_INTERRUPT;
			pAInterruptTrue->m_specificID = 1;
			cdResponse.m_response.m_actions.AddTail(pAInterruptTrue);
		}

		//new - atomic set global
		POSITION pos = cdResponse.m_response.m_actions.GetHeadPosition();
		POSITION posOld;
		while ((posOld = pos) != NULL) {
			Action* pa = (Action*)cdResponse.m_response.m_actions.GetNext(pos);
			if (pa->opcode == ACTION_DIALOG_SET_GLOBAL ||
				pa->opcode == ACTION_DIALOG_SG ||
				pa->opcode == ACTION_DIALOG_INCREMENT_GLOBAL) {
				if (CGameAIBase_AtomicSetGlobal(cre, *pa)) {
					cdResponse.m_response.m_actions.RemoveAt(posOld);
					delete pa;
				}
			}
		}

		CMessageQueueActions* pMsgQA = IENew CMessageQueueActions();
		pMsgQA->eSource = cre.id;
		pMsgQA->eTarget = cre.id;
		pMsgQA->r = cdResponse.m_response;
		pMsgQA->bClearActionQueue = FALSE;
		pMsgQA->bSkipIfAlreadyQueued = FALSE;
		g_pChitin->messages.Send(*pMsgQA, FALSE);

		CMessageSetTriggerRemovalTimer* pMsgSTRT = IENew CMessageSetTriggerRemovalTimer();
		pMsgSTRT->eSource = cre.id;
		pMsgSTRT->eTarget = cre.id;
		pMsgSTRT->wTicks = 75;
		g_pChitin->messages.Send(*pMsgSTRT, FALSE);
	}

	return;
}


BOOL static __stdcall
CheckBG1Chapter() {
    if (pGameOptionsEx->bUI_GreetingBeforeDialog_BG2Chapters) {
        return TRUE;
    } else {
        bool    BG1part;
        bool    BG2afterBG1;
        CVariable& bg1done_marker = g_pChitin->pGame->m_GlobalVariables.Find("ENDOFBG1");
        CVariable& bg1part_marker = g_pChitin->pGame->m_GlobalVariables.Find("TETHTORIL");

        if (&bg1part_marker != NULL) {
			BG1part = true;
        } else {
            BG1part = false;
        }

		if (&bg1done_marker != NULL) {
			BG2afterBG1 = bg1done_marker.intValue > 0 ? true : false;
        } else {
            BG2afterBG1 = false;
        }

        if (BG1part &&
            BG2afterBG1 == false)
            return TRUE;
        else
            return FALSE;
    }
}


BOOL __stdcall 
GameSprite_PlayerDialog_DisplaySayNothing(CCreatureObject &Cre) {
    if (Cre.BaseStats.soundset[62] != 0 && // DIALOG_DEFAULT
        Cre.BaseStats.soundset[62] != 0xFFFFFFFF)
        return FALSE;
    else
        return TRUE;
}


ushort __stdcall 
CCreatureObject_PlaySound_CheckJoinable(CCreatureObject &Cre, ushort nums) {

    // check PDIALOG.2da for joinable char
    CRuleTable* pRuleTable = NULL;
    pRuleTable = &g_pChitin->pGame->PDIALOG;
    int nCol = 0;
    for (int nRow = 0; nRow < pRuleTable->nRows; nRow++) {
        IECString& rowHeader = *(pRuleTable->pRowHeaderArray + nRow);
        if (!rowHeader.CompareNoCase(Cre.szScriptName)) {
            return 0;   // return num=0 sounds
        }
    }

    return nums;
}


void  __declspec(naked)
CGameSprite_PlayerDialog_InsertGreeting1_asm() {
__asm {
    push    edx
    push    ecx

    mov     eax, [ebp+8]                ; Dialog Cre
    cmp     byte ptr [eax.u664d], 0x55
    jz      CGameSprite_PlayerDialog_InsertGreeting1_continue
    
    call    CheckBG1Chapter
    test    eax,eax
    jz      CGameSprite_PlayerDialog_InsertGreeting1_continue

    mov     eax, [ebp+8]                ; Dialog Cre
    mov     byte ptr [eax.u664d], 0x55  ; Greeting done

    push    eax
    mov     ecx, eax
    mov     eax, 08D1EB5h               ; CGameSprite::PlayDialogSound
    call    eax


CGameSprite_PlayerDialog_InsertGreeting1_continue:
    pop     ecx
    pop     edx

    mov     ecx, [ebp-400h]  ; Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_PlayerDialog_InsertGreeting2_asm() {
__asm {
    push    edx
    push    ecx

    mov     eax, [ebp-430h]             ; Dialog Cre
    cmp     byte ptr [eax.u664d], 0x55
    jz      CGameSprite_PlayerDialog_InsertGreeting2_continue

    call    CheckBG1Chapter
    test    eax,eax
    jz      CGameSprite_PlayerDialog_InsertGreeting2_continue

    mov     eax, [ebp-430h]             ; Dialog Cre
    mov     byte ptr [eax.u664d], 0x55  ; Greeting done

    push    eax
    mov     ecx, eax
    mov     eax, 08D1EB5h               ; CGameSprite::PlayDialogSound
    call    eax


CGameSprite_PlayerDialog_InsertGreeting2_continue:
    pop     ecx
    pop     edx

    mov     ecx, [ebp-430h]  ; Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_PlayerDialog_ToggleGreeting1_asm() {
__asm {
    push    eax

    mov     eax, [ebp+8]                ; Dialog Cre
    mov     byte ptr [eax.u664d], 0x00  ; Reset Greeting

    pop     eax
    cmp     dword ptr [ebp-1FCh], 0     ; Stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_PlayerDialog_ToggleGreeting2_asm() {
__asm {
    push    eax

    mov     eax, [ebp-430h]             ; Dialog Cre
    mov     byte ptr [eax.u664d], 0x00  ; Reset Greeting

    pop     eax
    mov     edx, [ebp+8]                ; Stolen bytes
    mov     eax, [edx]
    ret
}
}


void  __declspec(naked)
GameSprite_PlayerDialog_DisplaySayNothing_asm() {
__asm {

    push    [ebp+8]     // CreTarget
    call    GameSprite_PlayerDialog_DisplaySayNothing
    test    eax, eax
    jz      GameSprite_PlayerDialog_DisplaySayNothing_asm_exit

    mov     eax, 0x8D42AA
    jmp     eax


GameSprite_PlayerDialog_DisplaySayNothing_asm_exit:
    ret     0x10
}
}


void  __declspec(naked)
CCreatureObject_PlaySound_CheckJoinable_asm() {
__asm {
    push    edx
    push    ecx

    push    eax
    push    [ebp-218h]      //Cre
    call    CCreatureObject_PlaySound_CheckJoinable
    // ax = num existance sounds

    pop     ecx
    pop     edx

    mov     [ebp-0A4h], ax  // stolen bytes
    ret
}
}