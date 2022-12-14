#include "InfGameCore.h"

#include "stdafx.h"
#include "objcre.h"
#include "chitin.h"
#include "infgame.h"
#include "cstringex.h"
#include "console.h"
#include "log.h"
#include "InfGameCommon.h"
#include "ObjectStats.h"

CRuleTables& (CRuleTables::*Tramp_CRuleTables_Construct)() =
	SetFP(static_cast<CRuleTables& (CRuleTables::*)()>									(&CRuleTables::Construct),					0x6213DC);
void (CRuleTables::*Tramp_CRuleTables_Deconstruct)() =
	SetFP(static_cast<void (CRuleTables::*)()>											(&CRuleTables::Deconstruct),				0x6279D1);
int (CRuleTables::*Tramp_CRuleTables_CalculateNewHPSubclass)(char, char, CDerivedStats&, CDerivedStats&, int, int) =
	SetFP(static_cast<int (CRuleTables::*)(char, char, CDerivedStats&, CDerivedStats&, int, int)>
																						(&CRuleTables::CalculateNewHPSubclass),		0x631055);
ResRef (CRuleTables::*Tramp_CRuleTables_GetMageSpellRef)(int, int) =
	SetFP(static_cast<ResRef (CRuleTables::*)(int, int)>								(&CRuleTables::GetMageSpellRef),			0x633691);
int (CRuleTables::*Tramp_CRuleTables_GetWeapProfMax)(char, char, char, BOOL, int, unsigned int) =
	SetFP(static_cast<int (CRuleTables::*)(char, char, char, BOOL, int, unsigned int)>	(&CRuleTables::GetWeapProfMax),				0x636C57);
BOOL (CRuleTables::*Tramp_CRuleTables_IsMageSchoolAllowed)(unsigned int, unsigned char) =
	SetFP(static_cast<BOOL (CRuleTables::*)(unsigned int, unsigned char)>				(&CRuleTables::IsMageSchoolAllowed),		0x637DEE);
ResRef (CRuleTables::*Tramp_CRuleTables_GetMageSpellRefAutoPick)(char, char) =
	SetFP(static_cast<ResRef (CRuleTables::*)(char, char)>								(&CRuleTables::GetMageSpellRefAutoPick),	0x63AD1A);
unsigned char (CRuleTables::*Tramp_CRuleTables_MapCharacterSpecializationToSchool)(unsigned int) =
	SetFP(static_cast<unsigned char (CRuleTables::*)(unsigned int)>             (&CRuleTables::MapCharacterSpecializationToSchool), 0x639D4A);

void (CMoveAreasList::*Tramp_CMoveAreasList_MoveAllTo)(CArea&) =
	SetFP(static_cast<void (CMoveAreasList::*)(CArea&)>	(&CMoveAreasList::MoveAllTo),	0x5EFA6D);

void (CInfGame::*Tramp_CInfGame_InitGame)(int, int) =
	SetFP(static_cast<void (CInfGame::*)(int, int)>					(&CInfGame::InitGame),				0x67C6C5);
int (CInfGame::*Tramp_CInfGame_GetNumOfItemInBag)(ResRef&, ResRef&, BOOL) =
	SetFP(static_cast<int (CInfGame::*)(ResRef&, ResRef&, BOOL)>	(&CInfGame::GetNumOfItemInBag),		0x68F35C);
void (CInfGame::*Tramp_CInfGame_SetLoseCutscene)() =
	SetFP(static_cast<void (CInfGame::*)()>							(&CInfGame::SetLoseCutscene),		0x6AE3E0);



//CRuleTables
CRuleTables& DETOUR_CRuleTables::DETOUR_Construct() {
	CRuleTables& rule = (this->*Tramp_CRuleTables_Construct)();
	pRuleEx = new CRuleTablesEx(rule);
	return rule;	
}

void DETOUR_CRuleTables::DETOUR_Deconstruct() {
	delete pRuleEx;
	pRuleEx = NULL;
	return (this->*Tramp_CRuleTables_Deconstruct)();
}

int DETOUR_CRuleTables::DETOUR_CalculateNewHPSubclass(char nClass, char nSubclass, CDerivedStats& cdsOld, CDerivedStats& cdsNew, int nMinRoll, int nDivisor) {
	if (0) IECString("DETOUR_CRuleTables::DETOUR_CalculateNewHPSubclass");

	if (!pRuleEx->m_HPClass.m_2da.bLoaded ||
		!pRuleEx->m_HPBarbarian.m_2da.bLoaded) {
		return (this->*Tramp_CRuleTables_CalculateNewHPSubclass)(nClass, nSubclass, cdsOld, cdsNew, nMinRoll, nDivisor);
	}

	char nSubclassLevelOld = cdsOld.GetSubclassLevel(nClass, nSubclass);
	char nSubclassLevelNew = cdsNew.GetSubclassLevel(nClass, nSubclass);

	IECString sSubclass;
	if (nSubclass == CLASS_FIGHTER) {
		unsigned int nKitUnusableFlag = cdsNew.kit;
		if (cdsNew.kit & KIT_TRUECLASS &&
			cdsNew.kit & 0xBFFF) {
			unsigned int dwKitOnly = cdsNew.kit & 0xBFFF;
			int col = 6;
			int row = dwKitOnly;

			IECString sKitUnusableFlag;
			if (col < KITLIST.nCols &&
				row < KITLIST.nRows &&
				col >= 0 &&
				row >= 0) {
				sKitUnusableFlag = *((KITLIST.pDataArray) + (KITLIST.nCols * row + col));
			} else {
				sKitUnusableFlag = KITLIST.defaultVal;
			}

			sscanf_s((LPCTSTR)sKitUnusableFlag, "%d", &nKitUnusableFlag);
		}

		if (nKitUnusableFlag == KIT_BARBARIAN) {
			sSubclass = "BARBARIAN";
		} else {
			sSubclass = GetClassString(nSubclass, KIT_TRUECLASS);
		}
	} else {
		sSubclass = GetClassString(nSubclass, KIT_TRUECLASS);
	}

	IECString sColName = "TABLE";
	IECString sTable = pRuleEx->m_HPClass.GetValue(sColName, sSubclass);
	
	CRuleTable table;
	table.LoadTable(ResRef(sTable));
	if (!table.m_2da.bLoaded) {
		LPCTSTR lpsz = "DETOUR_CRuleTables::DETOUR_CalculateNewHPSubclass(): %s.2DA not found\r\n";
		L.timestamp();
		L.append(lpsz);
		console.write(lpsz);

		return (this->*Tramp_CRuleTables_CalculateNewHPSubclass)(nClass, nSubclass, cdsOld, cdsNew, nMinRoll, nDivisor);
	}

	if (pGameOptionsEx->bDebugVerbose) {
		LPCTSTR lpsz = "DETOUR_CRuleTables::DETOUR_CalculateNewHPSubclass(): Using table %s.2DA\r\n";
		L.timestamp();
		L.appendf(lpsz, (LPCTSTR)sTable);
		console.writef(lpsz, (LPCTSTR)sTable);
	}

	return CalculateNewHPRule(table, nSubclassLevelOld, nSubclassLevelNew, nMinRoll, nDivisor, FALSE, 0, FALSE, 0);
}

ResRef DETOUR_CRuleTables::DETOUR_GetMageSpellRef(int nSpellLevel, int nIndex) {
	if (0) IECString("DETOUR_CRuleTables::DETOUR_GetMageSpellRef");

	ResRef rSpell;
	int nMaxSpells = 50;
	
	if (nSpellLevel <= 0 || nSpellLevel > 9) {
		LPCTSTR lpsz = "DETOUR_CRuleTables::DETOUR_GetMageSpellRef(): Invalid spell level %d\r\n";
		L.timestamp();
		L.appendf(lpsz, nSpellLevel);
		console.writef(lpsz, nSpellLevel);

		return rSpell;
	}

	if (pGameOptionsEx->bEngineExternMageSpellsCap) {
		IECString sCol;
		sCol.Format("%d", nSpellLevel);
		IECString sRow("MAGE");
		IECString sMax(g_pChitin->pGame->SPELLS.GetValue(sCol, sRow));

		if (nSpellLevel == 9 && nIndex >= 99) {
			nMaxSpells = min(99, atoi((LPCTSTR)sMax) - 100);

			nSpellLevel = 0;
			nIndex -= 100;

			//a bit of hackery for index 99
			if (nIndex == -1 && nIndex < nMaxSpells) {
				char szSpell[8] = {0};
				sprintf_s(szSpell, 8, "SPWI%d%02d", nSpellLevel, nIndex + 1);
				rSpell = szSpell;
			}

			//safety for index 199+
			if (nIndex >= 99) {
				return pGameOptionsEx->bUserExternMageSpellHiding ? CRuleTables_TryHideSpell(rSpell): rSpell;
			}
		} else {
			//spell levels 1-8, and level 9 with normal nMaxSpells
			nMaxSpells = min(99, atoi((LPCTSTR)sMax));
		}
	}

	if (nIndex >= 0 && nIndex < nMaxSpells) {
		char szSpell[8] = {0};
		sprintf_s(szSpell, 8, "SPWI%d%02d", nSpellLevel, nIndex + 1);
		rSpell = szSpell;
	}

	return pGameOptionsEx->bUserExternMageSpellHiding ? CRuleTables_TryHideSpell(rSpell): rSpell;
}

int DETOUR_CRuleTables::DETOUR_GetWeapProfMax(char nClassId, char nClassPrimary, char nClassSecondary, BOOL bTwoClasses, int nWeapProfId, unsigned int dwKit) {
	DWORD Eip;
	GetEip(Eip);

	if (0) IECString("DETOUR_CRuleTables::DETOUR_GetWeapProfMax");

	int dwWeapProfMax = (this->*Tramp_CRuleTables_GetWeapProfMax)(nClassId, nClassPrimary, nClassSecondary, bTwoClasses, nWeapProfId, dwKit);
	int dwClassProfsMax = 0;

	if (Eip == 0x6DAEC1 || //CScreenRecord::DualClassProficiencyPanelOnLoad()
		Eip == 0x6DC79A || //CScreenRecord::LevelUpPanelOnLoad()
		Eip == 0x6DC80C || //CScreenRecord::LevelUpPanelOnLoad()
		Eip == 0x6DFAFE || //CScreenRecord::LevelUpPanelOnLoad()
		Eip == 0x6E2196 || //CScreenRecord::DualClassProficiencyPanelOnUpdate()
		Eip == 0x6E28BE || //CScreenRecord::LevelUpPanelOnUpdate()
		Eip == 0x6F082C || //CScreenRecord::DualClass()
		Eip == 0x6F86E9 || //CUIButtonRecordLevelUpProficiency::UpdateCharacter()
		Eip == 0x6FD498) //CUIButtonRecordDualClassProficiency::UpdateCharacter()
		dwClassProfsMax = g_pChitin->pScreenCharacter->dwProfsMax;

	if (Eip == 0x71D9B7 || //CScreenCharGen::ProficienciesPanelOnLoad()
		Eip == 0x71FAC2 || //CScreenCharGen::ProficienciesPanelOnUpdate()
		Eip == 0x71FB11 || //CScreenCharGen::ProficienciesPanelOnUpdate()
		Eip == 0x7305F5 || //CUIButtonCharGenProficiency::UpdateCharacter()
		Eip == 0x730649) //CUIButtonCharGenProficiency::UpdateCharacter()
		dwClassProfsMax = g_pChitin->pScreenCreateChar->dwProfsMax;

	if (dwClassProfsMax) {
		return dwClassProfsMax < dwWeapProfMax ? dwClassProfsMax : dwWeapProfMax;
	} else {
		return dwWeapProfMax;
	}
}

BOOL DETOUR_CRuleTables::DETOUR_IsMageSchoolAllowed(unsigned int dwKit, unsigned char nRace) {
	if (0) IECString("DETOUR_CRuleTables::DETOUR_IsMageSchoolAllowed");

	if (!pRuleEx->m_MageSchoolRaceReq.m_2da.bLoaded) return (this->*Tramp_CRuleTables_IsMageSchoolAllowed)(dwKit, nRace);

	CInfGame* pGame = g_pChitin->pGame;
	IECString sRace = pGame->GetRaceString(nRace);
	IECString sClass = pGame->GetClassString(CLASS_MAGE, dwKit);
	IECString sAllowed = pRuleEx->m_MageSchoolRaceReq.GetValue(sRace, sClass);
	BOOL bAllowed;
	sscanf_s((LPCTSTR)sAllowed, "%d", &bAllowed);

	return bAllowed;
}

ResRef DETOUR_CRuleTables::DETOUR_GetMageSpellRefAutoPick(char nSpellLevel, char nIndex) {
	if (0) IECString("DETOUR_CRuleTables::DETOUR_GetMageSpellRefAutoPick");

	ResRef rSpell = (this->*Tramp_CRuleTables_GetMageSpellRefAutoPick)(nSpellLevel, nIndex);
	return CRuleTables_TryHideSpell(rSpell);
}

//CMoveAreasList
void DETOUR_CMoveAreasList::DETOUR_MoveAllTo(CArea& area) { 
	if (0) IECString("DETOUR_CMoveAreasList::DETOUR_MoveAllTo");

	ITEM_EFFECT* pItmEff = new ITEM_EFFECT;
	CEffect::CreateItemEffect(*pItmEff, CEFFECT_OPCODE_MOVE_TO_AREA);
	IECPtrList cp;

	POSITION pos_i = GetHeadPosition();
	while (pos_i != NULL) {
		MoveAreasElement* pElement = (MoveAreasElement*)GetNext(pos_i);
		MoveAreasComparator* pComp = new MoveAreasComparator;

		pComp->rArea = pElement->rArea;
		pComp->ptDest = pElement->ptDest;

		pItmEff->param2 = pElement->cOrient;
		pItmEff->param1 = pElement->nDelay;
		//pElement->rArea.CopyTo8Chars(pItmEff->resource);
        pItmEff->resource = pElement->rArea;
		pItmEff->timing = 1;
		POINT pt;
		pt.x = -1;
		pt.y = -1;
		CEffect* pEff = &CEffect::CreateEffect(*pItmEff, pElement->ptDest, pElement->eCre, pt, -1);

		CInfGame* pGame = g_pChitin->pGame;
		CCreatureObject* pCre = NULL;
		char threadVal;
		do {
			threadVal = pGame->m_GameObjectArrayHandler.GetGameObjectDeny(pElement->eCre, THREAD_ASYNCH, &pCre, INFINITE);
		} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);
		if (threadVal != OBJECT_SUCCESS) {
			delete pComp;
		} else {
			pComp->sCreLongName = pCre->GetLongName();

			bool bFoundIdentical = false;
			POSITION pos_j = cp.GetHeadPosition();
			while (pos_j != NULL) {
				MoveAreasComparator* pCompElement = (MoveAreasComparator*)cp.GetNext(pos_j);
				if (pCompElement->ptDest.x == pComp->ptDest.x &&
					pCompElement->ptDest.y == pComp->ptDest.y &&
					pCompElement->rArea == pComp->rArea &&
					pCompElement->sCreLongName.Compare(pComp->sCreLongName) == 0) {
					delete pComp;
					bFoundIdentical = true;
					break; //Bioware forgot to break this loop
				}
			}

			if (!bFoundIdentical) {
				cp.AddTail(pComp);
				pCre->BaseStats.dwFlags |= 0x4000; //bit14
				area.m_ObjectsToMarshal.AddTail((void*)pCre->id);
				pCre->AddEffect(*pEff, true, TRUE, FALSE);
			}

            pGame->m_GameObjectArrayHandler.FreeGameObjectDeny(pElement->eCre, THREAD_ASYNCH, INFINITE);
		}
	}

	POSITION pos_k = cp.GetHeadPosition();
	while (pos_k != NULL) {
		MoveAreasComparator* pElement = (MoveAreasComparator*)cp.GetNext(pos_k);
		delete pElement;
	}
	cp.RemoveAll();
	
	delete pItmEff;
	pItmEff = NULL;

	return;
}

//CInfGame
void DETOUR_CInfGame::DETOUR_InitGame(int nUnused, int nUnused2) {
	std::map<Enum, CBlockVariables*>::iterator it;
	for (it = pRuleEx->m_MapActionVars.begin(); it != pRuleEx->m_MapActionVars.end(); it++) {
		delete it->second;
		it->second = NULL;
	}
	pRuleEx->m_MapActionVars.clear();

	return (this->*Tramp_CInfGame_InitGame)(nUnused, nUnused2);
}

int DETOUR_CInfGame::DETOUR_GetNumOfItemInBag(ResRef& rBag, ResRef& rItem, BOOL bIdentifiedOnly) {
	if (0) IECString("DETOUR_CInfGame::DETOUR_GetNumOfItemInBag");

	if (g_pChitin->cNetwork.bSessionOpen) {
		BOOL bRequested = FALSE;
		CStore store;

		if (!g_pChitin->cNetwork.bSessionHosting) {
			store.Unmarshal(rBag);
			BOOL bStoreValid = store.bUnmarshaled && store.m_header == "STORV1.0" ? TRUE : FALSE;
			if (bStoreValid == FALSE) {
				bool bFailed = !g_pChitin->BaldurMessageHandler.RequestHostFile(rBag.GetResRefStr(), CRES_TYPE_STO, TRUE, TRUE, true);
				if (bFailed) {
					g_pChitin->cNetwork.CloseSession(true);
					return 0;
				} else {
					store.Unmarshal(rBag);
					bRequested = TRUE;
				}
			}
		} else {
			DemandServerStore(rBag, TRUE);
			store.Unmarshal(rBag);
		}

		int nItemCount = 0;
		POSITION pos = store.m_items.GetHeadPosition();
		while (pos != NULL) {
			StoFileItem* pItem = (StoFileItem*)store.m_items.GetNext(pos);
			if (pItem) {
				if (pItem->rName == rItem) {
					if (bIdentifiedOnly) {
						if (pItem->dwFlags & CITEMFLAG_IDENTIFIED) {
							int nAdd = pItem->bInfinite ? 1 : pItem->nNumInStock;
							nItemCount += nAdd;
						}
					} else {
						int nAdd = pItem->bInfinite ? 1 : pItem->nNumInStock;
						nItemCount += nAdd;
					}
				}
			}
		}

		if (g_pChitin->cNetwork.bSessionHosting) {
			g_pChitin->pGame->ReleaseServerStore(store.m_filename);
			return nItemCount;
		} else {
			if (bRequested) {
				CMessageHostReleaseServerStore* pMsg = IENew CMessageHostReleaseServerStore();
				pMsg->rStoreName = store.m_filename;
				g_pChitin->messages.Send(*pMsg, FALSE);
			}
			return nItemCount;
		}

	} else {
		CStore store(rBag);
		int nItemCount = 0;
		POSITION pos = store.m_items.GetHeadPosition();

		while (pos != NULL) {
			StoFileItem* pItem = (StoFileItem*)store.m_items.GetNext(pos);
			if (pItem) {
				if (pItem->rName == rItem) {
					if (bIdentifiedOnly) {
						if (pItem->dwFlags & CITEMFLAG_IDENTIFIED) {
							int nAdd = pItem->bInfinite ? 1 : pItem->nNumInStock;
							nItemCount += nAdd;
						}
					} else {
						int nAdd = pItem->bInfinite ? 1 : pItem->nNumInStock;
						nItemCount += nAdd;
					}
				}
			}
		}

		return nItemCount;
	}
}

void DETOUR_CInfGame::DETOUR_SetLoseCutscene() {
	DWORD Eip;
	GetEip(Eip);

	if (0) IECString("DETOUR_CInfGame::DETOUR_SetLoseCutscene");

	if (Eip == 0x5092EB || //CEffectInstantDeath::ApplyEffect()
		Eip == 0x50935F || //CEffectInstantDeath::ApplyEffect()
		Eip == 0x5277E6 || //CEffectPetrification::ApplyEffect()
		Eip == 0x527859 || //CEffectPetrification::ApplyEffect()
		Eip == 0x531E37 || //CEffectImprisonment::ApplyEffect()
		Eip == 0x7C612B || //CScreenWorld::Init()?
		Eip == 0x7D37B8) { //CScreenWorld::Update()
		return;
	}
	
	return (this->*Tramp_CInfGame_SetLoseCutscene)();
}

BOOL __stdcall CRuleTables_HasKnownMageSpells(CCreatureObject& cre) {
	for (int i = 0; i < 9; i++) {
		if (cre.KnownSpellsWizard[i].GetCount()) return TRUE;
	}
	return FALSE;
}

ResRef CRuleTables_TryHideSpell(ResRef& rSpell) {
	if (!pRuleEx->m_HideSpell.m_2da.bLoaded) return rSpell;

	if (pRuleEx->m_HideSpell.nCols && pRuleEx->m_HideSpell.nRows) {
		for (int i = 0; i < pRuleEx->m_HideSpell.nRows; i++) {
			IECString* sSpellHide = pRuleEx->m_HideSpell.pRowHeaderArray + i;
			if (rSpell == *sSpellHide) {
				rSpell = "NO_SPELL";
				break;
			}
		}
	}

	return rSpell;
}

IECString& __stdcall CRuleTables_GetMaxProfs(CCreatureObject& cre, IECString& sRowName) {
	IECString sClass = g_pChitin->pGame->GetClassString(cre.oBase.GetClass(), KIT_TRUECLASS);
	IECString sLevel = "2";
	IECString& sLevelExp = g_pChitin->pGame->XPLEVEL.GetValue(sLevel, sClass);
	int levelExp = atoi((LPCTSTR)sLevelExp);

	IECString sColName;
    (int)cre.BaseStats.PowerLevel_or_PersonalXP < levelExp ? sColName = "FIRST_LEVEL" : sColName = "NONE" /*"OTHER_LEVELS"*/;
		
	return g_pChitin->pGame->PROFSMAX.GetValue(sColName, sRowName); //placed into g_pChitin->pCharacter->dwProfsMax;
}

STRREF __stdcall CInfGame_GetRaceText(unsigned int nRace) {
	int Eip;
	GetEip(Eip);

	STRREF strref;

	if (Eip == 0x734392) {
		if (nRace == 5) {
			nRace = 3;
		} else {
			if (nRace == 3) {
				nRace = 5;
			}
		}
	}

	if (pRuleEx->m_RaceText.m_2da.bLoaded) {
		IECString sStrRef;

		int nCol = 0;
		int nRow = nRace;
		if (nCol < pRuleEx->m_RaceText.nCols &&
			nRow < pRuleEx->m_RaceText.nRows &&
			nCol >= 0 &&
			nRow >= 0) {
			sStrRef = *((pRuleEx->m_RaceText.pDataArray) + (pRuleEx->m_RaceText.nCols * nRow + nCol));
		} else {
			sStrRef = pRuleEx->m_RaceText.defaultVal;
		}

		sscanf_s((LPCTSTR)sStrRef, "%d", &strref);
		return strref;
	} else {
		switch (nRace) {
		case 0:
			strref = 0x1C19; //human
			break;
		case 1:
			strref = 0x1C1A; //elf
			break;
		case 2:
			strref = 0x1C1D; //half-elf
			break;
		case 3:
			strref = 0x1C0E; //dwarf
			break;
		case 4:
			strref = 0x1C1B; //halfling
			break;
		case 5:
			strref = 0x1C1C; //gnome
			break;
		case 6:
			strref = 0xCFC2; //half-orc
			break;
		case 152:
			strref = 0x208C; //tiefling
			break;
		default:
			strref = -1;
			break;
		}

		return strref;
	}

}

BOOL __stdcall CRuleTables_DoesEquipSlotPassCreExclude(CCreatureObject& cre, short wSlot, CItem& itmGrabbed, STRREF* pStrRef) {
	BOOL bPass = TRUE;
	IECString sCreName(cre.szScriptName);
	POINT loc = {0, 0};
	ResRef rSlotItm = cre.Inventory.items[wSlot] ? cre.Inventory.items[wSlot]->itm.name : "";
	ResRef rGrabItm = &itmGrabbed ? itmGrabbed.itm.name : "";

	IECString sItm;
	IECString sUser;
	int dwFlags = 0;

	if (pRuleEx->m_ItemCreExclude.m_2da.bLoaded) {
		for (int row = 0; row < pRuleEx->m_ItemCreExclude.nRows; row++) {
			sItm = pRuleEx->m_ItemCreExclude.GetRowName(row);
			
			if (sItm.CompareNoCase(rSlotItm.GetResRefNulled()) &&	sItm.CompareNoCase(rGrabItm.GetResRefNulled())) continue;

			loc.y = row;

			loc.x = ITEM_USE_COL_USER;
			sUser = pRuleEx->m_ItemCreExclude.GetValue(loc);

			loc.x = ITEM_USE_COL_FLAG;
			dwFlags = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));

			if (!sUser.CompareNoCase((LPCTSTR)sCreName) &&
				!sItm.CompareNoCase(rSlotItm.GetResRefNulled()) &&
				(dwFlags & (ITEM_USE_FLAG_NO_PICKUP | ITEM_USE_FLAG_NO_PICKUP_E))
			) {
				loc.x = (dwFlags & ITEM_USE_FLAG_NO_PICKUP) ? ITEM_USE_COL_STRREF1 : ITEM_USE_COL_STRREF8;
				STRREF strref = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
				*pStrRef = strref;
					
				if (pGameOptionsEx->bDebugVerbose) {
					LPCTSTR lpsz = "CRuleTables_DoesEquipSlotPassCreExclude(): Found %s restricted on %s with FLAG_NO_PICKUP[_E]\r\n";
					L.timestamp();
					L.appendf(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
					console.writef(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
				}
					
				bPass = FALSE;
				break;
			}

			if (sUser.CompareNoCase((LPCTSTR)sCreName) &&
				!sItm.CompareNoCase(rGrabItm.GetResRefNulled()) &&
				(dwFlags & ITEM_USE_FLAG_ONLY_EQUIP)
			) {
				loc.x = ITEM_USE_COL_STRREF2;
				STRREF strref = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
				*pStrRef = strref;
				
				if (pGameOptionsEx->bDebugVerbose) {
					LPCTSTR lpsz = "CRuleTables_DoesEquipSlotPassCreExclude(): Found %s restricted on %s with FLAG_ONLY_EQUIP\r\n";
					L.timestamp();
					L.appendf(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
					console.writef(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
				}
				
				bPass = FALSE;
				break;
			}

			if (!sUser.CompareNoCase((LPCTSTR)sCreName) &&
				!sItm.CompareNoCase(rGrabItm.GetResRefNulled()) &&
				(dwFlags & ITEM_USE_FLAG_NO_DROP_E)
			) {
				loc.x = ITEM_USE_COL_STRREF32;
				STRREF strref = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
				*pStrRef = strref;

				if (pGameOptionsEx->bDebugVerbose) {
					LPCTSTR lpsz = "CRuleTables_DoesEquipSlotPassCreExclude(): Found %s restricted on %s with FLAG_NO_DROP_E\r\n";
					L.timestamp();
					L.appendf(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
					console.writef(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
				}

				bPass = FALSE;
				break;
			}

		}
	} else {
		IECString sName(cre.szScriptName);

		if (cre.Inventory.items[wSlot] != NULL) {
			//fail if target slot has cre-specific item
			if (!sName.CompareNoCase("MINSC") && cre.Inventory.items[wSlot]->itm.name == "MISC84") {
				*pStrRef = 0x27EA;
				bPass = FALSE;
			}
			if (!sName.CompareNoCase("ALORA") && cre.Inventory.items[wSlot]->itm.name == "MISC88") {
				*pStrRef = 0x27EB;
				bPass = FALSE;
			}
			if (!sName.CompareNoCase("EDWIN") && cre.Inventory.items[wSlot]->itm.name == "MISC89") {
				*pStrRef = 0x27EE;
				bPass = FALSE;
			}
		}

		//fail if active item to be equipped is cre-specific
		if (&itmGrabbed != NULL) {
			if (sName.CompareNoCase("XAN") && itmGrabbed.itm.name == "SW1H13") {
				*pStrRef = 0x27EC;
				bPass = FALSE;
			}
			if (sName.CompareNoCase("ELDOTH") && itmGrabbed.itm.name == "AROW14") {
				*pStrRef = 0x27EC;
				bPass = FALSE;
			}
			if (sName.CompareNoCase("MINSC") && itmGrabbed.itm.name == "MISC84") {
				*pStrRef = 0x27EA;
				bPass = FALSE;
			}
			if (sName.CompareNoCase("ALORA") && itmGrabbed.itm.name == "MISC88") {
				*pStrRef = 0x27EB;
				bPass = FALSE;
			}
			if (sName.CompareNoCase("EDWIN") && itmGrabbed.itm.name == "MISC89") {
				*pStrRef = 0x27EE;
				bPass = FALSE;
			}
		}

	}

	return bPass;

}

BOOL __stdcall CRuleTables_DoesInvSlotPassCreExclude(CCreatureObject& cre, short wSlot, CItem& itmGrabbed, STRREF* pStrRef) {
	BOOL bPass = TRUE;
	IECString sCreName(cre.szScriptName);
	POINT loc = {0, 0};
	ResRef rSlotItm = cre.Inventory.items[wSlot] ? cre.Inventory.items[wSlot]->itm.name : "";
	ResRef rGrabItm = &itmGrabbed ? itmGrabbed.itm.name : "";

	IECString sItm;
	IECString sUser;
	int dwFlags = 0;

	if (pRuleEx->m_ItemCreExclude.m_2da.bLoaded) {
		for (int row = 0; row < pRuleEx->m_ItemCreExclude.nRows; row++) {
			sItm = pRuleEx->m_ItemCreExclude.GetRowName(row);
			
			if (sItm.CompareNoCase(rSlotItm.GetResRefNulled()) &&	sItm.CompareNoCase(rGrabItm.GetResRefNulled())) continue;

			loc.y = row;

			loc.x = ITEM_USE_COL_USER;
			sUser = pRuleEx->m_ItemCreExclude.GetValue(loc);

			loc.x = ITEM_USE_COL_FLAG;
			dwFlags = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
			
			if (!sUser.CompareNoCase((LPCTSTR)sCreName) &&
				!sItm.CompareNoCase(rSlotItm.GetResRefNulled()) &&
				(dwFlags & (ITEM_USE_FLAG_NO_PICKUP | ITEM_USE_FLAG_NO_PICKUP_I))
			) {
				loc.x = (dwFlags & ITEM_USE_FLAG_NO_PICKUP) ? ITEM_USE_COL_STRREF1 : ITEM_USE_COL_STRREF16;
				STRREF strref = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
				*pStrRef = strref;

				if (pGameOptionsEx->bDebugVerbose) {
					LPCTSTR lpsz = "CRuleTables_DoesEquipSlotPassCreExclude(): Found %s restricted on %s with FLAG_NO_PICKUP[_I]\r\n";
					L.timestamp();
					L.appendf(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
					console.writef(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
				}

				bPass = FALSE;
				break;
			}

			if (!sUser.CompareNoCase((LPCTSTR)sCreName) &&
				!sItm.CompareNoCase(rGrabItm.GetResRefNulled()) &&
				(dwFlags & ITEM_USE_FLAG_NO_DROP_I)
			) {
				loc.x = ITEM_USE_COL_STRREF64;
				STRREF strref = atoi((LPCTSTR)pRuleEx->m_ItemCreExclude.GetValue(loc));
				*pStrRef = strref;

				if (pGameOptionsEx->bDebugVerbose) {
					LPCTSTR lpsz = "CRuleTables_DoesEquipSlotPassCreExclude(): Found %s restricted on %s with FLAG_NO_DROP_I\r\n";
					L.timestamp();
					L.appendf(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
					console.writef(lpsz, (LPCTSTR)sItm, (LPCTSTR)sUser);
				}

				bPass = FALSE;
				break;
			}

		}
	} else {
		IECString sName(cre.szScriptName);

		if (cre.Inventory.items[wSlot] != NULL) {
			//fail if target slot has cre-specific item
			if (!sName.CompareNoCase("MINSC") && cre.Inventory.items[wSlot]->itm.name == "MISC84") {
				*pStrRef = 0x27EA;
				bPass = FALSE;
			}
			if (!sName.CompareNoCase("ALORA") && cre.Inventory.items[wSlot]->itm.name == "MISC88") {
				*pStrRef = 0x27EB;
				bPass = FALSE;
			}
			if (!sName.CompareNoCase("EDWIN") && cre.Inventory.items[wSlot]->itm.name == "MISC89") {
				*pStrRef = 0x27EE;
				bPass = FALSE;
			}
		}
	}

	return bPass;
}

BOOL __stdcall CRuleTables_IsLowEncumbrance(unsigned int nWeight, unsigned int nWeightAllowance) {
	if (pRuleEx->m_nEncumbranceLowThreshold == 0) return FALSE;
	return nWeight > (nWeightAllowance * pRuleEx->m_nEncumbranceLowThreshold / 100);
}

BOOL __stdcall CRuleTables_IsHighEncumbrance(unsigned int nWeight, unsigned int nWeightAllowance) {
	if (pRuleEx->m_nEncumbranceHighThreshold == 0) return FALSE;
	return nWeight > (nWeightAllowance * pRuleEx->m_nEncumbranceHighThreshold / 100);
}

int __stdcall CRuleTables_GetWeightAllowance(CDerivedStats& cds) {
	int nWeightAllowanceStr;
	int nWeightAllowanceStrEx;

    short nCol = 3; //WEIGHT_ALLOWANCE
	short nRow = cds.strength;
	IECString sWeightAllowanceStr;
	if (nCol < g_pChitin->pGame->STRMOD.nCols &&
		nRow < g_pChitin->pGame->STRMOD.nRows &&
		nCol >= 0 &&
		nRow >= 0) {
		sWeightAllowanceStr = *((g_pChitin->pGame->STRMOD.pDataArray) + (g_pChitin->pGame->STRMOD.nCols * nRow + nCol));
	} else {
		sWeightAllowanceStr = g_pChitin->pGame->STRMOD.defaultVal;
	}
	sscanf_s((LPCTSTR)sWeightAllowanceStr, "%d", &nWeightAllowanceStr);

	nCol = 3; //WEIGHT_ALLOWANCE
	nRow = cds.strengthEx; 
	IECString sWeightAllowanceStrEx;
	if (nCol < g_pChitin->pGame->STRMODEX.nCols &&
		nRow < g_pChitin->pGame->STRMODEX.nRows &&
		nCol >= 0 &&
		nRow >= 0) {
		sWeightAllowanceStrEx = *((g_pChitin->pGame->STRMODEX.pDataArray) + (g_pChitin->pGame->STRMODEX.nCols * nRow + nCol));
	} else {
		sWeightAllowanceStrEx = g_pChitin->pGame->STRMODEX.defaultVal;
	}
	sscanf_s((LPCTSTR)sWeightAllowanceStrEx, "%d", &nWeightAllowanceStrEx);

	return max(0, nWeightAllowanceStr + nWeightAllowanceStrEx + cds.GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_WEIGHTALLOWANCEMOD));
}

void __stdcall CInfGame_SetDifficultyMultiplier(CInfGame* pGame) {
	if (!pRuleEx->m_DiffMod.m_2da.bLoaded) {
		//original code
		switch (pGame->m_GameOptions.m_nDifficultyLevel) {
		case DIFFLEV_EASIEST:
			pGame->m_GameOptions.m_nDifficultyMultiplier = -50;
			break;
		case DIFFLEV_EASY:
			pGame->m_GameOptions.m_nDifficultyMultiplier = -25;
			break;
		case DIFFLEV_NORMAL:
			pGame->m_GameOptions.m_nDifficultyMultiplier = 0;
			break;
		case DIFFLEV_HARD:
			pGame->m_GameOptions.m_nDifficultyMultiplier = 50;
			break;
		case DIFFLEV_HARDEST:
			pGame->m_GameOptions.m_nDifficultyMultiplier = 100;
			break;
		default:
			break;
		}

		return;
	}
	
	int nDifficultyMod = 0;
	int nCol = 0;
	int nRow = pGame->m_GameOptions.m_nDifficultyLevel - 1;
	IECString sDifficultyMod;
	
	if (nCol < pRuleEx->m_DiffMod.nCols &&
		nRow < pRuleEx->m_DiffMod.nRows &&
		nCol >= 0 &&
		nRow >= 0) {
		sDifficultyMod = *((pRuleEx->m_DiffMod.pDataArray) + (pRuleEx->m_DiffMod.nCols * nRow + nCol));
	} else {
		sDifficultyMod = pRuleEx->m_DiffMod.defaultVal;
	}
	sscanf_s((LPCTSTR)sDifficultyMod, "%d", &nDifficultyMod);
	if (nDifficultyMod < -100) nDifficultyMod = -100;
	pGame->m_GameOptions.m_nDifficultyMultiplier = nDifficultyMod;

	//original code handles setting m_nMPDifficultyMultiplier

	return;
}

void __stdcall CInfGame_SetDifficultyMultiplierFeedback(CInfGame* pGame, STRREF* pFeedback) {
	CInfGame_SetDifficultyMultiplier(pGame);

	switch (pGame->m_GameOptions.m_nDifficultyLevel) {
	case DIFFLEV_EASIEST:
		*pFeedback = 11308; //Easy
		break;
	case DIFFLEV_EASY:
		*pFeedback = 11309; //Normal
		break;
	case DIFFLEV_NORMAL:
		*pFeedback = 11311; //Core Rules
		break;
	case DIFFLEV_HARD:
		*pFeedback = 11312; //Hard
		break;
	case DIFFLEV_HARDEST:
		*pFeedback = 11313; //Insane
		break;
	default:
		break;
	}

	return;
}


unsigned char
DETOUR_CRuleTables::DETOUR_MapCharacterSpecializationToSchool(unsigned int reversedKit) {
    unsigned int Kit = reversedKit << 16  | reversedKit >> 16;
    switch (Kit) {
    case KIT_TRUECLASS:
    case KIT_WILDMAGE:
        return SCHOOL_GENERALIST;
        break;
    case MAGESCHOOL_ABJURER:
        return SCHOOL_ABJURER;
        break;
    case MAGESCHOOL_CONJURER:
        return SCHOOL_CONJURER;
        break;
    case MAGESCHOOL_DIVINER:
        return SCHOOL_DIVINER;
        break;
    case MAGESCHOOL_ENCHANTER:
        return SCHOOL_ENCHANTER;
        break;
    case MAGESCHOOL_ILLUSIONIST:
        return SCHOOL_ILLUSIONIST;
        break;
    case MAGESCHOOL_INVOKER:
        return SCHOOL_INVOKER;
        break;
    case MAGESCHOOL_NECROMANCER:
        return SCHOOL_NECROMANCER;
        break;
    case MAGESCHOOL_TRANSMUTER:
        return SCHOOL_TRANSMUTER;
        break;
    default:
        return SCHOOL_NONE;
        break;
    }
}


extern void ToggleStaticIcons();
extern void ToggleShowHPPortrait();
extern void ToggleSpellMenu();
extern void ToggleNPCTooltip();
extern void ToggleGreyBackgroundOnPause();

void static __stdcall
NoBindedKeyHandle(uint keycode) {
    keycode += 0x9; // revert to orig code

    if (pGameOptionsEx) { // if tobex is alive
        if (keycode == 0x6F &&      // Numpad /
            pGameOptionsEx->bUI_HideStaticPortraitIcons) {
            ToggleStaticIcons();
        }

        if (keycode == 0x6A &&      // Numpad *
            pGameOptionsEx->bUI_ShowHitPointsOnPortrait) {
            ToggleShowHPPortrait();
        }

        if (keycode == 0x6D &&      // Numpad -
            pGameOptionsEx->bUI_SpellSelectMenu) {
            ToggleSpellMenu();
        }

        if (keycode == 0x6B &&      // Numpad +
            pGameOptionsEx->bUI_GreyBackgroundOnPause) {
            ToggleGreyBackgroundOnPause();
        }

        if (keycode == 0xA4 &&      // Left ALT
            pGameOptionsEx->bUI_ShowNPCFloatHP) {
            ToggleNPCTooltip();
        }
    }
}


BOOL static __stdcall
CScreenWorld_HandleKeyboard_CheckDialogState(uchar key, CScreenWorld& screen ) {
    if (key == 0x20) {
        if (screen.manager.GetPanel(9).GetUIControl(0).bEnabled)    // Continue button in Dialog panel
            return TRUE;
        else
            return FALSE;
    } else
        return FALSE;
}


BOOL static
CheckMemorizedSpell(CCreatureObject& Cre, char* spellname) {
    POSITION            pos;
    CreFileMemSpell     *pMemSpell;
    CMemorizedSpellList *p = NULL;

    if (!spellname)
        return FALSE;

    ResRef SpellNameRef(spellname);
    ResSplContainer resSpell;
    resSpell.SetResRef(SpellNameRef, TRUE, TRUE);
    resSpell.Demand();
    short CasterType = resSpell.GetSpellType();
    short SpellLevel = resSpell.GetSpellLevel();
    resSpell.Release();

    if (CasterType == SPELLTYPE_MAGE) {
        p = & Cre.MemorizedSpellsWizard[SpellLevel - 1];
    } else 
    if (CasterType == SPELLTYPE_PRIEST) {
        p = & Cre.MemorizedSpellsPriest[SpellLevel - 1];
    } else 
    if (CasterType == SPELLTYPE_INNATE) {
        p = & Cre.MemorizedSpellsInnate[0];
    }

    if (p) {
        pos = p->GetTailPosition();
        while (pos != NULL) {
            pMemSpell = (CreFileMemSpell *) p->GetPrev(pos);
            if (pMemSpell) {
                if (pMemSpell->name == SpellNameRef &&
                    pMemSpell->wFlags & CREMEMSPELL_MEMORIZED) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


BOOL static __stdcall
CScreenWorld_KeyHandle_CheckAvailableSpell(short wBufferIdx) {
    char** KeyboardSpellTable = (char**) 0xB4860C;
    Enum e = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
    CCreatureObject* pCre = NULL;
    char nResult;
    do {
        nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pCre, INFINITE);
    } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

    if (nResult == OBJECT_SUCCESS) {
        char* spellname = KeyboardSpellTable[wBufferIdx];
        BOOL result = CheckMemorizedSpell(*pCre, spellname);
        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
        return result;
    }

    return FALSE;
}



void __declspec(naked)
CScreenWorld_NoBindedKeyHandle_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-1050h]     // key
    call    NoBindedKeyHandle

    pop     edx
    pop     ecx

    mov     word ptr [ebp-60h], 0    // stolen bytes
    ret
}
}


void __declspec(naked)
CScreenCharacter_NoBindedKeyHandle_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-50h]     // key
    call    NoBindedKeyHandle

    pop     edx
    pop     ecx

    push    06E96B3h    // stolen bytes
    ret
}
}


void __declspec(naked)
CScreenInventory_NoBindedKeyHandle_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-54h]     // key
    call    NoBindedKeyHandle

    pop     edx
    pop     ecx

    push    0742FE5h    // stolen bytes
    ret
}
}


void __declspec(naked)
CScreenMap_NoBindedKeyHandle_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-4Ch]     // key
    call    NoBindedKeyHandle

    pop     edx
    pop     ecx

    push    0763646h    // stolen bytes
    ret
}
}


void __declspec(naked)
CScreenJournal_NoBindedKeyHandle_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-4Ch]     // key
    call    NoBindedKeyHandle

    pop     edx
    pop     ecx

    push    07588C5h    // stolen bytes
    ret
}
}


void __declspec(naked)
CScreenWorld_HandleKeyboard_CheckDialogState_asm() {
__asm
{   mov     al, [edx+ecx+0E40h]     // key
    push    ecx
    push    edx

    push    [ebp-0FA0h]     // screen
    push    eax             // key
    call    CScreenWorld_HandleKeyboard_CheckDialogState

    test    eax,eax
    pop     edx
    pop     ecx
    jnz     CScreenWorld_HandleKeyboard_CheckDialogState_EmulateEnterKey

    mov     al, [edx+ecx+0E40h]    // stolen bytes
    ret

CScreenWorld_HandleKeyboard_CheckDialogState_EmulateEnterKey:

    add     esp, 4
    push    07C71F3h    // ENTER handler
    ret
}
}


void __declspec(naked)
CScreenWorld_KeyHandle_CheckSpell_asm() {
__asm
{   push    ecx
    push    edx

    push    [ebp-60h]     // wBufferIdx
    call    CScreenWorld_KeyHandle_CheckAvailableSpell

    test    eax,eax
    pop     edx
    pop     ecx
    jz      CScreenWorld_KeyHandle_CheckSpell_asm_abort

    movsx   edx, word ptr [ebp-60h]    // stolen bytes
    cmp     edx, 016Dh
    ret

CScreenWorld_KeyHandle_CheckSpell_asm_abort:

    add     esp, 4
    push    07CE8E4h    // abort key
    ret
}
}