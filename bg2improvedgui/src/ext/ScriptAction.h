#ifndef SCRIPTACTION_H
#define SCRIPTACTION_H

#include "objcre.h"

BOOL CGameAIBase_AtomicSetGlobal(CGameAIBase& sprite, Action& a);
ACTIONRESULT CGameAIBase_ActionAssign(CGameAIBase& sprite, Action& a);
ACTIONRESULT CGameAIBase_ActionEval(CGameAIBase& sprite, Action& a);
ACTIONRESULT CGameAIBase_ActionClearBlockVars(CGameAIBase& sprite);
void CCreatureObject_ActionChangeAnimation_CopyState(CCreatureObject& creOld, CCreatureObject& creNew);

#endif //SCRIPTACTION_H
