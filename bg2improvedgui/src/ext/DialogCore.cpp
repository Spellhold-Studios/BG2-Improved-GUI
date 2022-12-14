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
			BG2afterBG1 = bg1done_marker.nValue > 0 ? true : false;
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