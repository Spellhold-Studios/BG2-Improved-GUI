#ifndef OBJECTCORE_H
#define OBJECTCORE_H

#include "stdafx.h"
#include "objcore.h"

extern BOOL (CGameAIBase::*Tramp_CGameAIBase_EvaluateStatusTrigger)(Trigger&);
extern void (CGameAIBase::*Tramp_CGameAIBase_ClearAllActions)(BOOL);
extern ACTIONRESULT (CGameAIBase::*Tramp_CGameAIBase_ExecuteAction)();
extern void (CGameAIBase::*Tramp_CGameAIBase_QueueActions)(Response&, BOOL, BOOL);

class DETOUR_CGameAIBase : public CGameAIBase {
public:
	BOOL DETOUR_EvaluateStatusTrigger(Trigger& t);
	void DETOUR_ClearAllActions(BOOL bSkipFlagged);
	ACTIONRESULT DETOUR_ExecuteAction();
	void DETOUR_QueueActions(Response& r, BOOL bSkipIfAlreadyQueued, BOOL bClearActionQueue);
};

void CGameAIBase_EvaluateStatusTrigger_AreaCheckObject_asm();
void CAIGroup_ClearActions_asm();

#endif //OBJECTCORE_H