#include "ObjectCore.h"

#include "chitin.h"
#include "objcre.h"
#include "ScriptAction.h"

BOOL (CGameAIBase::*Tramp_CGameAIBase_EvaluateStatusTrigger)(Trigger&) =
	SetFP(static_cast<BOOL (CGameAIBase::*)(Trigger&)>	(&CGameAIBase::EvaluateStatusTrigger),	0x47F4F2);
void (CGameAIBase::*Tramp_CGameAIBase_ClearAllActions)(BOOL) =
	SetFP(static_cast<void (CGameAIBase::*)(BOOL)>					(&CGameAIBase::ClearAllActions),	0x47838F);
ACTIONRESULT (CGameAIBase::*Tramp_CGameAIBase_ExecuteAction)() =
	SetFP(static_cast<ACTIONRESULT (CGameAIBase::*)()>	(&CGameAIBase::ExecuteAction),		0x47891B);
void (CGameAIBase::*Tramp_CGameAIBase_QueueActions)(Response&, BOOL, BOOL) =
	SetFP(static_cast<void (CGameAIBase::*)(Response&, BOOL, BOOL)>	(&CGameAIBase::QueueActions),		0x48DA78);

BOOL DETOUR_CGameAIBase::DETOUR_EvaluateStatusTrigger(Trigger& t) {
	if (0) IECString("DETOUR_CGameAIBase::DETOUR_EvaluateTrigger");

	Trigger tTemp = t;
	CCreatureObject* pCre = NULL;
	CGameAIBase* pSprite = NULL;
	Object oCriteria;
	int nFindTargetResult = 0;
	BOOL bResult = FALSE;

	switch (tTemp.opcode) {
	case TRIGGER_MOVEMENT_RATE:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		CAnimation* pAnim = pCre->animation.pAnimation;
		if (pAnim == NULL) break;
		unsigned char nMvt = pAnim->GetCurrentMovementRate();
		bResult = (signed int)nMvt == tTemp.i;
		}
		break;
	case TRIGGER_MOVEMENT_RATE_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		CAnimation* pAnim = pCre->animation.pAnimation;
		if (pAnim == NULL) break;
		unsigned char nMvt = pAnim->GetCurrentMovementRate();
		bResult = (signed int)nMvt > tTemp.i;
		}
		break;
	case TRIGGER_MOVEMENT_RATE_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		CAnimation* pAnim = pCre->animation.pAnimation;
		if (pAnim == NULL) break;
		unsigned char nMvt = pAnim->GetCurrentMovementRate();
		bResult = (signed int)nMvt < tTemp.i;
		}
		break;
	case TRIGGER_NUM_MIRRORIMAGES:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		unsigned char nImages = pCre->nMirrorImages;
		bResult = (signed int)nImages == tTemp.i;
		}
		break;
	case TRIGGER_NUM_MIRRORIMAGES_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		unsigned char nImages = pCre->nMirrorImages;
		bResult = (signed int)nImages > tTemp.i;
		}
		break;
	case TRIGGER_NUM_MIRRORIMAGES_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		unsigned char nImages = pCre->nMirrorImages;
		bResult = (signed int)nImages < tTemp.i;
		}
		break;
	case TRIGGER_BOUNCING_SPELL_LEVEL:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		bResult = pCre->GetDerivedStats().BounceSplLvl[tTemp.i] || pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].bOn;
		}
		break;
	case TRIGGER_NUM_BOUNCING_SPELL_LEVEL:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumBounce = 0;
		if (pCre->GetDerivedStats().BounceSplLvl[tTemp.i]) {
			nNumBounce = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].bOn)
				nNumBounce = pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumBounce == tTemp.i2;
		}
		break;
	case TRIGGER_NUM_BOUNCING_SPELL_LEVEL_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumBounce = 0;
		if (pCre->GetDerivedStats().BounceSplLvl[tTemp.i]) {
			nNumBounce = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].bOn)
				nNumBounce = pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumBounce > (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_NUM_BOUNCING_SPELL_LEVEL_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumBounce = 0;
		if (pCre->GetDerivedStats().BounceSplLvl[tTemp.i]) {
			nNumBounce = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].bOn)
				nNumBounce = pCre->GetDerivedStats().BounceSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumBounce < (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_IMMUNE_SPELL_LEVEL:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		bResult = pCre->GetDerivedStats().ProtSplLvl[tTemp.i] || pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].bOn;
		}
		break;
	case TRIGGER_NUM_IMMUNE_SPELL_LEVEL:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumProt = 0;
		if (pCre->GetDerivedStats().ProtSplLvl[tTemp.i]) {
			nNumProt = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].bOn)
				nNumProt = pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumProt == tTemp.i2;
		}
		break;
	case TRIGGER_NUM_IMMUNE_SPELL_LEVEL_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumProt = 0;
		if (pCre->GetDerivedStats().ProtSplLvl[tTemp.i]) {
			nNumProt = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].bOn)
				nNumProt = pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumProt > (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_NUM_IMMUNE_SPELL_LEVEL_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumProt = 0;
		if (pCre->GetDerivedStats().ProtSplLvl[tTemp.i]) {
			nNumProt = UINT_MAX;
		} else {
			if (pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].bOn)
				nNumProt = pCre->GetDerivedStats().ProtSplLvlDec[tTemp.i].nCount;
		}
		bResult = nNumProt < (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_TIME_STOP_COUNTER:
		bResult = g_pChitin->pGame->m_nTimeStopTicksLeft == tTemp.i;
		break;
	case TRIGGER_TIME_STOP_COUNTER_GT:
		bResult = g_pChitin->pGame->m_nTimeStopTicksLeft > tTemp.i;
		break;
	case TRIGGER_TIME_STOP_COUNTER_LT:
		bResult = g_pChitin->pGame->m_nTimeStopTicksLeft < tTemp.i;
		break;
	case TRIGGER_TIME_STOP_OBJECT:
		tTemp.DecodeIdentifiers(*this);
		pSprite = (CGameAIBase*)&tTemp.o.FindTargetOfType(*this, CGAMEOBJECT_TYPE_SPRITE, FALSE);
		if (pSprite == NULL) break;
		bResult = pSprite->id == g_pChitin->pGame->m_eTimeStopExempt;
		break;
	case TRIGGER_NUM_TRAPPING_SPELL_LEVEL:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumTrap = 0;
		if (pCre->GetDerivedStats().SplTrapLvl[tTemp.i].bOn)
			nNumTrap = pCre->GetDerivedStats().SplTrapLvl[tTemp.i].nCount;
		bResult = nNumTrap == tTemp.i2;
		}
		break;
	case TRIGGER_NUM_TRAPPING_SPELL_LEVEL_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumTrap = 0;
		if (pCre->GetDerivedStats().SplTrapLvl[tTemp.i].bOn)
			nNumTrap = pCre->GetDerivedStats().SplTrapLvl[tTemp.i].nCount;
		bResult = nNumTrap > (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_NUM_TRAPPING_SPELL_LEVEL_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		if (tTemp.i < 0) break;
		if (tTemp.i > 9) break;
		unsigned int nNumTrap = 0;
		if (pCre->GetDerivedStats().SplTrapLvl[tTemp.i].bOn)
			nNumTrap = pCre->GetDerivedStats().SplTrapLvl[tTemp.i].nCount;
		bResult = nNumTrap < (unsigned int)tTemp.i2;
		}
		break;
	case TRIGGER_ORIGINAL_CLASS:
		{
		tTemp.DecodeIdentifiers(*this);
		unsigned char nClassOld;
		unsigned char nClassNew;
		tTemp.o.GetDualClasses(&nClassNew, &nClassOld);
		if (nClassNew == nClassOld) break; //not dual-class
		oCriteria.Class = tTemp.i;
		Object oTemp;
		oTemp.Class = nClassOld;
		bResult = oTemp.MatchCriteria(oCriteria, FALSE, FALSE, FALSE);
		}
		break;
	case TRIGGER_HP_LOST:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		int nHurtAmount = pCre->cdsCurrent.maxHP - pCre->BaseStats.currentHP;
		bResult = nHurtAmount == tTemp.i;
		}
		break;
	case TRIGGER_HP_LOST_GT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		int nHurtAmount = pCre->cdsCurrent.maxHP - pCre->BaseStats.currentHP;
		bResult = nHurtAmount > tTemp.i;
		}
		break;
	case TRIGGER_HP_LOST_LT:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		int nHurtAmount = pCre->cdsCurrent.maxHP - pCre->BaseStats.currentHP;
		bResult = nHurtAmount < tTemp.i;
		}
		break;
	case TRIGGER_EQUALS:
		bResult = tTemp.i == tTemp.i2;
		break;
	case TRIGGER_GT:
		bResult = tTemp.i > tTemp.i2;
		break;
	case TRIGGER_LT:
		bResult = tTemp.i < tTemp.i2;
		break;
	case TRIGGER_CHECK_STAT_BAND:
		{
		nFindTargetResult = QuickDecode(tTemp, &pCre);
		if (pCre == NULL) break;
		bResult = (BOOL)(pCre->GetDerivedStats().GetStat(tTemp.i2) & tTemp.i);
		break;
		}
	default:
		bResult = (this->*Tramp_CGameAIBase_EvaluateStatusTrigger)(t);
		break;
	}

	if (pCre != NULL &&
		nFindTargetResult == 0) {
		g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(pCre->id, THREAD_ASYNCH, INFINITE);
	}

	if (pSprite != NULL)
		g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(pSprite->id, THREAD_ASYNCH, INFINITE);

	return bResult;

}

extern void __stdcall CAIGroup_ClearActions_CheckInterrupt(CCreatureObject&);

void DETOUR_CGameAIBase::DETOUR_ClearAllActions(BOOL bLeaveOverrides) {
	POSITION pos = queuedActions.GetHeadPosition();
	if (bLeaveOverrides) {
		while (pos) {
            POSITION posTarget = pos;
			Action* pA = (Action*)queuedActions.GetNext(pos);
            
            // bugged ClearAllActions(1):
            // 1) original "if (pA->dwFlags & 1 == 0)" never executed due priority "==" over "&"

            // 2) if fixed to "if ((pA->dwFlags & 1) == 0)" game loops at one of mod cutscene when imoen change her portrain
            //    (BG1 part of BGT, BGIMOEN.BCS+IMPORTR.BCS)
            /*  BGIMOEN.BCS
            IF
	            Global("ImoenPortraitReplace","GLOBAL",0)
	            Global("EnteredBaldursGate","GLOBAL",1)
	            IfValidForPartyDialog(Myself)
	            PartyRested()
	            !See([ENEMY])
            THEN
	            RESPONSE #100
		            ActionOverride(Player1,SetDialog("Player1"))
		            SetGlobal("ImoenPortraitReplace","GLOBAL",1) <- deleted at ClearAllActions()
		            StartCutSceneMode()
		            StartCutScene("ImPortr")
            END */

            /*  IMPORTR.BCS
            IF
	            True()
            THEN
	            RESPONSE #100
		            CutSceneId("Imoen2")
		            FadeToColor([20.0],0)
		            Wait(2)
		            GiveItemCreate("ImPortr","Imoen2",1,1,0)
		            ActionOverride("Imoen2",UseItem("ImPortr","Imoen2"))
		            FadeFromColor([20.0],0)
		            SmallWait(5)
		            SetGlobal("ImoenPortraitReplace","GLOBAL",2)
		            RealSetGlobalTimer("PortraitReplace","LOCALS",TWO_MINUTES) <- deleted at ClearAllActions()
		            EndCutSceneMode() <- deleted at ClearAllActions()
            END */

            // 3) if DETOUR_ClearAllActions hook disabled, game still loops at cutscene even with original bgmain.exe's code
            
			//if (pA->dwFlags & 1 == 0) {
			//	//BioWare forgets to delete the action here
			//	delete pA;
			//	pA = NULL;

			//	queuedActions.RemoveAt(posTarget);
			//}
		}
	} else {
		while (pos) {
			Action* pA = (Action*)queuedActions.GetNext(pos);
			delete pA;
			pA = NULL;
		}
		queuedActions.RemoveAll();
	}

	//new - forcibly clear all action block variables
	CGameAIBase_ActionClearBlockVars(*this);

	nCurrResponseIdx = -1;
	nCurrScriptBlockIdx = -1;
	nCurrScriptIdx = -1;

    if (pGameOptionsEx) {   // if tobex is alive
        if (pGameOptionsEx->bUI_ExtendedEventText)
            CAIGroup_ClearActions_CheckInterrupt((CCreatureObject&) *this);
    }

	return;
}

ACTIONRESULT DETOUR_CGameAIBase::DETOUR_ExecuteAction() {
	if (0) IECString("DETOUR_CGameAIBase::DETOUR_ExecuteAction");

    //char* ps = & *this->szScriptName;
    //if (strncmp(ps, "KAGAIN", 32) == 0)
    //    int b = 1;

	bInActionExecution = TRUE;
	ACTIONRESULT ar = ACTIONRESULT_ERROR;

	if (currentAction.opcode == ACTION_BREAK_INSTANTS) SetCurrentAction(GetTopAction(g_pActionTemp));

	switch (currentAction.opcode) {
	case ACTION_BREAK_INSTANTS:
		ar = ACTIONRESULT_DONE; //re-implement here to prevent weird behaviour for double BreakInstants()
		break;
	case ACTION_LOSE_GAME:
		g_pChitin->pGame->SetLoseCutscene();
		ar = ACTIONRESULT_DONE;
		break;
	case ACTION_DIALOG_SET_GLOBAL:
		currentAction.opcode = ACTION_SET_GLOBAL;
		return (this->*Tramp_CGameAIBase_ExecuteAction)();
		break;
	case ACTION_DIALOG_INCREMENT_GLOBAL:
		currentAction.opcode = ACTION_INCREMENT_GLOBAL;
		return (this->*Tramp_CGameAIBase_ExecuteAction)();
		break;
	case ACTION_DIALOG_SG:
		currentAction.opcode = ACTION_SG;
		return (this->*Tramp_CGameAIBase_ExecuteAction)();
		break;
	case ACTION_ASSIGN:
		ar = CGameAIBase_ActionAssign(*this, this->currentAction);
		break;
	case ACTION_EVAL:
		ar = CGameAIBase_ActionEval(*this, this->currentAction);
		break;
	case ACTION_CLEAR_BLOCK_VARIABLES:
		ar = CGameAIBase_ActionClearBlockVars(*this);
		break;
	default:
		return (this->*Tramp_CGameAIBase_ExecuteAction)();
		break;
	}

	nLastActionReturn = ar;
	bInActionExecution = FALSE;

	return ar;

	//return (this->*Tramp_CGameAIBase_ExecuteAction)();
}

void DETOUR_CGameAIBase::DETOUR_QueueActions(Response& r, BOOL bSkipIfAlreadyQueued, BOOL bClearActionQueue) {
	if (bSkipIfAlreadyQueued &&
		this->nCurrScriptBlockIdx >= 0 &&
		this->nCurrScriptIdx >= 0 &&
		this->nCurrScriptBlockIdx == r.nScriptBlockIdx &&
		this->nCurrScriptIdx == r.nScriptIdx) {
		return;
	}

    if (bClearActionQueue) {
        ClearAllActions(FALSE);
    }

	this->nCurrResponseIdx = r.nResponseIdx;
	this->nCurrScriptBlockIdx = r.nScriptBlockIdx;
	this->nCurrScriptIdx = r.nScriptIdx;
	this->bInterrupt = TRUE;

	if (this->GetObjectType() != CGAMEOBJECT_TYPE_CREATURE) {
		Action* pAInterruptFalse = new Action();
		pAInterruptFalse->opcode = ACTION_SET_INTERRUPT;
		pAInterruptFalse->m_specificID = 0;
		this->queuedActions.AddTail(pAInterruptFalse);
	}

	//new - add ClearVariables
	Action* pAClearBlockVars = new Action();
	pAClearBlockVars->opcode = ACTION_CLEAR_BLOCK_VARIABLES;
	this->queuedActions.AddTail(pAClearBlockVars);

	POSITION pos = r.m_actions.GetHeadPosition();
	while (pos) {
		Action* aTemp = (Action*)r.m_actions.GetNext(pos);
		Action* pANew = new Action();
		*pANew = *aTemp;
		this->queuedActions.AddTail(pANew);
	}

	if (this->GetObjectType() != CGAMEOBJECT_TYPE_CREATURE) {
		Action* pAInterruptTrue = new Action();
		pAInterruptTrue->opcode = ACTION_SET_INTERRUPT;
		pAInterruptTrue->m_specificID = 1;
		this->queuedActions.AddTail(pAInterruptTrue);
	}
}


ResRef* __stdcall
CGameAIBase_EvaluateStatusTrigger_AreaCheckObject(CArea* pArea, CCreatureObject& Cre) {
    if (pArea == NULL) {
        if (Cre.GetObjectType() == CGAMEOBJECT_TYPE_CREATURE)
            return & Cre.currentArea;
        else
            return NULL;
    } else
        return & pArea->rAreaName;
}


void __declspec(naked)
CGameAIBase_EvaluateStatusTrigger_AreaCheckObject_asm()
{
__asm {
	push    ecx
	push    edx

    push    [ebp-0D4h]  // Cre
    push    ecx         // Area
	call    CGameAIBase_EvaluateStatusTrigger_AreaCheckObject
    // return eax - pResRefArea
    test    eax,eax

	pop     edx
	pop     ecx
    jz      CGameAIBase_EvaluateStatusTrigger_AreaCheckObject_asm_Abort

	ret

CGameAIBase_EvaluateStatusTrigger_AreaCheckObject_asm_Abort:
    add     esp, 4
    push    0489868h    // stop trigger processing, return FALSE
    ret
}
}


void __declspec(naked)
CAIGroup_ClearActions_asm()
{
__asm {

    //cmp     dword ptr [ebp+4], 04CCC51h // called from CGameArea::OnActionButtonClickGround
	//jz      CAIGroup_ClearActions_asm_WALK

    cmp     word ptr [ecx+33ACh], 6     // Cre.wAnimSequenceSimplified = SEQ_HEAD_TURN ?
    jz      CAIGroup_ClearActions_asm_TURNHEAD

//CAIGroup_ClearActions_Orig:
    push    08AD630h    // CGameSprite::SetSequence
    ret

CAIGroup_ClearActions_asm_TURNHEAD:     // skip anything
	ret     4

//CAIGroup_ClearActions_asm_WALK:
    //cmp     word ptr [ecx+33ACh], 10    // Cre.wAnimSequenceSimplified = SEQ_WALK, already walking
    //jz      CAIGroup_ClearActions_Orig  // back to original behaviour

    //mov     word ptr [ecx+33ACh], 7     // Cre.wAnimSequenceSimplified = SEQ_READY, skip SetSequence()
	//ret     4
}
}