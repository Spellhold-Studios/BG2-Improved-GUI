#include "objcore.h"

#include "stdafx.h"

//CGameObject
Enum (CGameObject::*CGameObject_GetEnum)() =
	SetFP(static_cast<Enum (CGameObject::*)()>	(&CGameObject::GetEnum),	0x494D00);

Enum CGameObject::GetEnum()	{ return (this->*CGameObject_GetEnum)(); }

//CGameAIBase
BOOL (CGameAIBase::*CGameAIBase_EvaluateStatusTrigger)(Trigger&) =
	SetFP(static_cast<BOOL (CGameAIBase::*)(Trigger&)>				(&CGameAIBase::EvaluateStatusTrigger),	0x47F4F2);
void (CGameAIBase::*CGameAIBase_ClearAllActions)(BOOL) =
	SetFP(static_cast<void (CGameAIBase::*)(BOOL)>					(&CGameAIBase::ClearAllActions),	0x47838F);
ACTIONRESULT (CGameAIBase::*CGameAIBase_ExecuteAction)() =
	SetFP(static_cast<ACTIONRESULT (CGameAIBase::*)()>				(&CGameAIBase::ExecuteAction),		0x47891B);
void (CGameAIBase::*CGameAIBase_SetCurrentAction)(Action&) =
	SetFP(static_cast<void (CGameAIBase::*)(Action&)>				(&CGameAIBase::SetCurrentAction),	0x48F85F);
Action& (CGameAIBase::*CGameAIBase_GetTopAction)(Action*) =
	SetFP(static_cast<Action& (CGameAIBase::*)(Action*)>			(&CGameAIBase::GetTopAction),		0x48CC65);
void (CGameAIBase::*CGameAIBase_QueueActions)(Response&, BOOL, BOOL) =
	SetFP(static_cast<void (CGameAIBase::*)(Response&, BOOL, BOOL)>	(&CGameAIBase::QueueActions),		0x48DA78);

BOOL CGameAIBase::EvaluateStatusTrigger(Trigger& t)	{ return (this->*CGameAIBase_EvaluateStatusTrigger)(t); }
void CGameAIBase::ClearAllActions(BOOL bLeaveOverrides){ return (this->*CGameAIBase_ClearAllActions)(bLeaveOverrides); }
ACTIONRESULT CGameAIBase::ExecuteAction()			{ return (this->*CGameAIBase_ExecuteAction)(); }
void CGameAIBase::SetCurrentAction(Action& a)		{ return (this->*CGameAIBase_SetCurrentAction)(a); }
Action& CGameAIBase::GetTopAction(Action* pAction)	{ return (this->*CGameAIBase_GetTopAction)(pAction); }
void CGameAIBase::QueueActions(Response& r, BOOL bSkipIfAlreadyQueued, BOOL bClearActionQueue) { return (this->*CGameAIBase_QueueActions)(r, bSkipIfAlreadyQueued, bClearActionQueue); }

//CGameObjectArrayHandler
char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObjectShare)(Enum, char, void*, int) =
	SetFP(static_cast<char (CGameObjectArrayHandler::*)(Enum, char, void*, int)>	(&CGameObjectArrayHandler::GetGameObjectShare),		0x67626B);
//char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObject)(Enum, char, void*, int) =
//	SetFP(static_cast<char (CGameObjectArrayHandler::*)(Enum, char, void*, int)>	(&CGameObjectArrayHandler::GetGameObject),			0x6764E5);
char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObjectDeny)(Enum, char, void*, int) =
	SetFP(static_cast<char (CGameObjectArrayHandler::*)(Enum, char, void*, int)>	(&CGameObjectArrayHandler::GetGameObjectDeny),		0x676570);
char (CGameObjectArrayHandler::*CGameObjectArrayHandler_FreeGameObjectShare)(Enum, char, int) =
	SetFP(static_cast<char (CGameObjectArrayHandler::*)(Enum, char, int)>			(&CGameObjectArrayHandler::FreeGameObjectShare),	0x676808);
char (CGameObjectArrayHandler::*CGameObjectArrayHandler_FreeGameObjectDeny)(Enum, char, int) =
	SetFP(static_cast<char (CGameObjectArrayHandler::*)(Enum, char, int)>			(&CGameObjectArrayHandler::FreeGameObjectDeny),		0x676A16);

char CGameObjectArrayHandler::GetGameObjectShare(Enum e, char threadNum, void* pptr, int dwTimeout) {
	return (this->*CGameObjectArrayHandler_GetGameObjectShare)(e, threadNum, pptr, dwTimeout);
}
//char CGameObjectArrayHandler::GetGameObject(Enum e, char threadNum, void* pptr, int dwTimeout) {
//	return (this->*CGameObjectArrayHandler_GetGameObject)(e, threadNum, pptr, dwTimeout);
//}
char CGameObjectArrayHandler::GetGameObjectDeny(Enum e, char threadNum, void* pptr, int dwTimeout) {
	return (this->*CGameObjectArrayHandler_GetGameObjectDeny)(e, threadNum, pptr, dwTimeout);
}
char CGameObjectArrayHandler::FreeGameObjectShare(Enum e, char threadNum, int dwTimeout) {
	return (this->*CGameObjectArrayHandler_FreeGameObjectShare)(e, threadNum, dwTimeout);
}
char CGameObjectArrayHandler::FreeGameObjectDeny(Enum e, char threadNum, int dwTimeout) {
	return (this->*CGameObjectArrayHandler_FreeGameObjectDeny)(e, threadNum, dwTimeout);
}

_n char CGameObjectArrayHandler::Add(Enum *index, CGameObject *ptr, DWORD dwTimeOut) { _bgmain(0x676B76) }
_n char CGameObjectArrayHandler::Delete(Enum id, char THREAD_MODE, int *pGameObject, DWORD dwTimeOut) { _bgmain(0x67726C) }
_n void CGameObject::RemoveFromArea_() { _bgmain(0x573B44) }