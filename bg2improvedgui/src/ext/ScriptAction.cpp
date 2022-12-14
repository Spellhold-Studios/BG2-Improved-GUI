#include "ScriptAction.h"

#include "stdafx.h"
#include "effcore.h"
#include "msgcore.h"
#include "objcre.h"
#include "chitin.h"
#include "InfGameCommon.h"
#include "ScriptCommon.h"

BOOL CGameAIBase_AtomicSetGlobal(CGameAIBase& sprite, Action& a) {
	IECString sArg = a.GetSName1();
	sArg.MakeUpper();
	IECString sScope = sArg.Left(6);
	IECString sVariable = sArg.Right(sArg.GetLength() - 6);
	CVariable varNew;

	BOOL bIncrement = a.opcode == ACTION_INCREMENT_GLOBAL;
	int nNewValue;

	CGameAIBase* pTarget = NULL;
	Object oOverride = a.oOverride;
	if (!oOverride.MatchCriteria(*poAny, FALSE, FALSE, FALSE) ||
		!oOverride.Name.IsEmpty() ||
		oOverride.oids.id1 != OBJECT_NOTHING) {
		oOverride.DecodeIdentifiers(sprite);
		pTarget = (CGameAIBase*)&oOverride.FindTargetOfType(sprite, CGAMEOBJECT_TYPE_SPRITE, FALSE);
		if (pTarget) {
			a.oOverride = *poAny;
		} else {
			return TRUE;
		}
	} else {
		pTarget = &sprite;
	}

	if (a.opcode == ACTION_SG) {
		sScope = "GLOBAL";
		sVariable = sArg;
	}

	if (!sScope.Compare("GLOBAL")) {
		CVariable& var = g_pChitin->pGame->m_GlobalVariables.Find(sVariable);
		if (&var != NULL) {
			var.nValue = bIncrement ? var.nValue + a.m_specificID : a.m_specificID;
			nNewValue = var.nValue;
		} else {
			varNew.SetName(sVariable);
			varNew.nValue = a.m_specificID;
			nNewValue = varNew.nValue;
			g_pChitin->pGame->m_GlobalVariables.Add(varNew);
		}

		CMessageModifyVariable* pMsg = IENew CMessageModifyVariable();
		pMsg->eSource = pTarget->id;
		pMsg->eTarget = pTarget->id;
		pMsg->sScope = sScope;
		pMsg->sVariable = sVariable;
		pMsg->nValue = nNewValue;
		pMsg->nBehaviour = 0;
		g_pChitin->messages.Send(*pMsg, FALSE);

		return TRUE;
	}

	if (!sScope.Compare("LOCALS")) {
		if (pTarget->GetObjectType() == CGAMEOBJECT_TYPE_CREATURE) {
			CVariable& var = ((CCreatureObject*)pTarget)->pLocalVariables->Find(sVariable);
			if (&var != NULL) {
				var.nValue = bIncrement ? var.nValue + a.m_specificID : a.m_specificID;
				nNewValue = var.nValue;
			} else {
				varNew.SetName(sVariable);
				varNew.nValue = a.m_specificID;
				nNewValue = varNew.nValue;
				((CCreatureObject*)pTarget)->pLocalVariables->Add(varNew); //64A2E8
			}

			CMessageModifyVariable* pMsg = IENew CMessageModifyVariable();
			pMsg->eSource = pTarget->id;
			pMsg->eTarget = pTarget->id;
			pMsg->sScope = sScope;
			pMsg->sVariable = sVariable;
			pMsg->nValue = nNewValue;
			pMsg->nBehaviour = 0;
			g_pChitin->messages.Send(*pMsg, FALSE);

			return TRUE;
		} else {
			LPCTSTR lpsz = "CGameAIBase_AtomicSetGlobal(): pTarget not CGAMEOBJECT_TYPE_CREATURE for setting LOCALS\r\n";
			L.timestamp();
			L.append(lpsz);
			console.write(lpsz);
			return TRUE;
		}
	}

	if (!sScope.Compare("MYAREA")) {
		sScope = pTarget->pArea->rAreaName.GetResRefStr();
	}
	CArea& area = g_pChitin->pGame->GetLoadedArea(sScope);
	if (&area != NULL) {
		CVariable& var = area.m_AreaVariables.Find(sVariable);
		if (&var != NULL) {
			var.nValue = bIncrement ? var.nValue + a.m_specificID : a.m_specificID;
			nNewValue = var.nValue;
		} else {
			varNew.SetName(sVariable);
			varNew.nValue = a.m_specificID;
			nNewValue = varNew.nValue;
			area.m_AreaVariables.Add(varNew);
		}

		CMessageModifyVariable* pMsg = IENew CMessageModifyVariable();
		pMsg->eSource = pTarget->id;
		pMsg->eTarget = pTarget->id;
		pMsg->sScope = sScope;
		pMsg->sVariable = sVariable;
		pMsg->nValue = nNewValue;
		pMsg->nBehaviour = 0;
		g_pChitin->messages.Send(*pMsg, FALSE);

		return TRUE;
	}

	return TRUE;
}

ACTIONRESULT CGameAIBase_ActionAssign(CGameAIBase& sprite, Action& a) {
	IECString sStatement = a.sName1;
	CGameAIBase* pSpriteRef = NULL;
	if (!a.oObject.IsAny()) {
		a.oObject.DecodeIdentifiers(sprite);
		pSpriteRef = (CGameAIBase*)&a.oObject.FindTargetOfType(sprite, CGAMEOBJECT_TYPE_SPRITE, FALSE);
	} else {
		pSpriteRef = &sprite;
	}
	if (pSpriteRef &&
		a.m_specificID2 >= 0 && a.m_specificID2 < BLOCK_VAR_ARRAY_SIZE) {
		std::map<Enum, CBlockVariables*>::iterator it = pRuleEx->m_MapActionVars.find(sprite.id);
		if (it == pRuleEx->m_MapActionVars.end()) pRuleEx->m_MapActionVars[sprite.id] = new CBlockVariables();

		if (a.m_specificID == ARGTYPE_INT) {
			ParseStatement(a.m_specificID2, a.m_specificID, sStatement, *pSpriteRef, *pRuleEx->m_MapActionVars[sprite.id]);
		} else if (a.m_specificID == ARGTYPE_STR) {
			ParseStatement(a.m_specificID2, a.m_specificID, sStatement, *pSpriteRef, *pRuleEx->m_MapActionVars[sprite.id]);
		}
	}

	return ACTIONRESULT_DONE;
}

ACTIONRESULT CGameAIBase_ActionEval(CGameAIBase& sprite, Action& a) {
	IECString sExpression;
	Action* aTarget = NULL;

	std::map<Enum, CBlockVariables*>::iterator it = pRuleEx->m_MapActionVars.find(sprite.id);
	if (it == pRuleEx->m_MapActionVars.end()) {
		sExpression = a.sName1;
	} else {
		sExpression = ParseBlockVariables(a.sName1, a.m_specificID, *pRuleEx->m_MapActionVars[sprite.id]);
	}

	POSITION pos = sprite.queuedActions.GetHeadPosition();
	POSITION posTarget = NULL;
	while (posTarget = pos) {
		Action* aTemp = (Action*)sprite.queuedActions.GetNext(pos);
		if (aTemp->opcode == ACTION_CLEAR_BLOCK_VARIABLES) {
			posTarget = NULL;
			break;
		}
		if (aTemp->opcode != ACTION_EVAL &&
			aTemp->opcode != ACTION_ASSIGN)
			break;
	}
	if (posTarget) {
		aTarget = (Action*)sprite.queuedActions.GetAt(posTarget);
	}

	if (aTarget) {
		if (a.m_specificID == ARGTYPE_INT) {
			int nValue = 0;
			MathPresso::Expression mpExp;
			MathPresso::mresult_t mpResult = mpExp.create(pRuleEx->m_mpContext, (LPCTSTR)sExpression);
			if (mpResult == MathPresso::MRESULT_OK) {
				nValue = (int) mpExp.evaluate(NULL);
			} else {
				LPCTSTR lpsz = "Action Eval(): bad expression \"%s\" (error %d)\r\n";
				L.timestamp();
				L.appendf(lpsz, (LPCTSTR)sExpression, (int)mpResult);
				console.writef(lpsz, (LPCTSTR)sExpression, (int)mpResult);
			}

			switch (a.m_specificID2) {
			case 1: //int1
				aTarget->m_specificID = nValue;
				break;
			case 2: //int2
				aTarget->m_specificID2 = nValue;
				break;
			case 3: //int3
				aTarget->m_specificID3 = nValue;
				break;
			case 4: //pt.x
				aTarget->pt.x = nValue;
				break;
			case 5: //pt.y
				aTarget->pt.y = nValue;
				break;
			default:
				break;
			}

		} else if (a.m_specificID == ARGTYPE_STR) {
			switch (a.m_specificID2) {
			case 1: //int1
				aTarget->sName1 = sExpression;
				break;
			case 2: //int2
				aTarget->sName2 = sExpression;
				break;
			default:
				break;
			}
		}
	}

	return ACTIONRESULT_DONE;
}

ACTIONRESULT CGameAIBase_ActionClearBlockVars(CGameAIBase& sprite) {
	std::map<Enum, CBlockVariables*>::iterator it = pRuleEx->m_MapActionVars.find(sprite.id);
	if (it != pRuleEx->m_MapActionVars.end()) {
		it->second->Empty();
		/*delete pRuleEx->m_MapActionVars[sprite.id];
		pRuleEx->m_MapActionVars[sprite.id] = NULL;
		pRuleEx->m_MapActionVars.erase(it);*/
	}

	return ACTIONRESULT_DONE;
}

void CCreatureObject_ActionChangeAnimation_CopyState(CCreatureObject& creOld, CCreatureObject& creNew) {
	creNew.SetSaveName(ResRef(creOld.currentAction.GetSName1()));
	if (creOld.BaseStats.currentHP <= 0) creNew.BaseStats.stateFlags |= STATE_DEAD;
	return;
}