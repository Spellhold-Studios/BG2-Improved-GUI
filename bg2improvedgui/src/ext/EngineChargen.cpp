#include "EngineChargen.h"

#include <cassert>

#include "EngineCommon.h"
#include "UserChargenMageSpell.h"
#include "UserChargenKit.h"
#include "UserCommon.h"
#include "chitin.h"
#include "tlkcore.h"
#include "uibutton.h"
#include "options.h"
#include "console.h"
#include "log.h"
#include "InfGameCommon.h"
#include "AnimationCore.h"

void (CScreenCharGen::*Tramp_CScreenCharGen_KitPanelOnLoad)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::KitPanelOnLoad),			0x71A081);
void (CScreenCharGen::*Tramp_CScreenCharGen_MageBookPanelOnLoad)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::MageBookPanelOnLoad),		0x71B220);
void (CScreenCharGen::*Tramp_CScreenCharGen_KitPanelOnUpdate)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::KitPanelOnUpdate),		0x71E3A5);
void (CScreenCharGen::*Tramp_CScreenCharGen_MageBookPanelOnUpdate)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::MageBookPanelOnUpdate),	0x720425);
void (CScreenCharGen::*Tramp_CScreenCharGen_ClassPanelOnUpdate)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::ClassPanelOnUpdate),		0x720B4B);
void (CScreenCharGen::*Tramp_CScreenCharGen_MulticlassPanelOnUpdate)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::MulticlassPanelOnUpdate),	0x721518);
void (CScreenCharGen::*Tramp_CScreenCharGen_MageSchoolPanelOnUpdate)(CPanel&, CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CPanel&, CCreatureObject&)>	(&CScreenCharGen::MageSchoolPanelOnUpdate),	0x721BA6);
void (CScreenCharGen::*Tramp_CScreenCharGen_InitSoundset)(CCreatureObject&) =
	SetFP(static_cast<void (CScreenCharGen::*)(CCreatureObject&)>			(&CScreenCharGen::InitSoundset),			0x724E37);

void DETOUR_CScreenCharGen::DETOUR_KitPanelOnLoad(CPanel& panel, CCreatureObject& cre) {
	CUIScrollBarChargenKit& scroll = (CUIScrollBarChargenKit&)panel.GetUIControl(15);
	if (&scroll == NULL) {
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_KitPanelOnLoad(): Kit scroll bar not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenCharGen_KitPanelOnLoad)(panel, cre);
	}

	//clear values
	scroll.nCurrentValue = 0;
	scroll.nValues = 0;
	scroll.nRows = 0;

	(this->*Tramp_CScreenCharGen_KitPanelOnLoad)(panel, cre);

	scroll.UpdateKnobPosition(scroll.nCurrentValue, scroll.nValues, scroll.nRows);

	return;
}

void DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnLoad(CPanel& panel, CCreatureObject& cre) {
	CUIScrollBarChargenMageSpell& scroll = (CUIScrollBarChargenMageSpell&)panel.GetUIControl(31);
	if (&scroll == NULL) {
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnLoad(): Chargen mage spell selection scroll bar not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenCharGen_MageBookPanelOnLoad)(panel, cre);
	}

	//clear values
	scroll.nCurrentValue = 0;
	scroll.nValues = 0;
	scroll.nRows = 0;
	scroll.cplTempSpells.RemoveAll();

	(this->*Tramp_CScreenCharGen_MageBookPanelOnLoad)(panel, cre);

	scroll.nValues = MageBookSpells.GetCount() / 6;
	if (MageBookSpells.GetCount() % 6) scroll.nValues++;
	scroll.nRows = 4;

	if (pGameOptionsEx->bDebugVerbose) {
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnLoad(): nMageSpells(%d), nScrollValues(%d)\r\n";
		console.writef(lpsz, MageBookSpells.GetCount(), max(scroll.nValues - scroll.nRows, 0));
		L.timestamp();
		L.appendf(lpsz, MageBookSpells.GetCount(), scroll.nValues);
	}

	scroll.UpdateKnobPosition(scroll.nCurrentValue, scroll.nValues, scroll.nRows);

	return;
}

void DETOUR_CScreenCharGen::DETOUR_KitPanelOnUpdate(CPanel& panel, CCreatureObject& cre) {
	CUIScrollBar& scroll = (CUIScrollBar&)panel.GetUIControl(6);
	pScrollActive = &scroll;
	Object& o = cre.oBase;

	CUIScrollBarChargenKit& scrollKit = (CUIScrollBarChargenKit&)panel.GetUIControl(15);

	CUICheckButtonChargenKit& option1 = (CUICheckButtonChargenKit&)panel.GetUIControl(1);
	CUICheckButtonChargenKit& option2 = (CUICheckButtonChargenKit&)panel.GetUIControl(2);
	CUICheckButtonChargenKit& option3 = (CUICheckButtonChargenKit&)panel.GetUIControl(3);
	CUICheckButtonChargenKit& option4 = (CUICheckButtonChargenKit&)panel.GetUIControl(4);
	CUICheckButtonChargenKit& option5 = (CUICheckButtonChargenKit&)panel.GetUIControl(9);
	CUICheckButtonChargenKit& option6 = (CUICheckButtonChargenKit&)panel.GetUIControl(10);
	CUICheckButtonChargenKit& option7 = (CUICheckButtonChargenKit&)panel.GetUIControl(11);
	CUICheckButtonChargenKit& option8 = (CUICheckButtonChargenKit&)panel.GetUIControl(12);
	CUICheckButtonChargenKit& option9 = (CUICheckButtonChargenKit&)panel.GetUIControl(13);
	CUICheckButtonChargenKit& option10 = (CUICheckButtonChargenKit&)panel.GetUIControl(14);
	CUICheckButtonChargenKit& option11 = (CUICheckButtonChargenKit&)panel.GetUIControl(16);

	CScreenCharGen* pCreateChar = g_pChitin->pScreenCreateChar;
	CInfGame* pGame = g_pChitin->pGame;

	assert(&option1 != NULL);
	assert(&option2 != NULL);
	assert(&option3 != NULL);
	assert(&option4 != NULL);
	assert(&option5 != NULL);
	assert(&option6 != NULL);
	assert(&option7 != NULL);
	assert(&option8 != NULL);
	assert(&option9 != NULL);
	assert(&option10 != NULL);
	assert(pCreateChar != NULL);
	assert(pGame != NULL);

	if (&scrollKit == NULL ||
		&option11 == NULL) {
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_KitPanelOnUpdate(): Kit selection scroll bar or button 11 not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenCharGen_KitPanelOnUpdate)(panel, cre);
	}

	IECString sClass, sRace, sKitName, sAlignment, sRowName, sKitFile;

	if (m_class) {
		sClass = pGame->GetClassString(m_class, KIT_TRUECLASS);
	} else {
		sClass = pGame->GetClassString(o.GetClass(), KIT_TRUECLASS);
	}

	sRace = pGame->GetRaceString(o.Race);
	sKitFile = pGame->KITTABLE.GetValue(sRace, sClass);

	if (Kit_Class_Race.m_2da.name != sKitFile) {
		Kit_Class_Race.LoadTable(ResRef(sKitFile));
	}

	for (int row = 0; row < Kit_Class_Race.nRows; row++) {

		int col = 0;

		IECString sKit;
		if (col < g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols &&
			row < g_pChitin->pScreenCreateChar->Kit_Class_Race.nRows &&
			col >= 0 &&
			row >= 0) {
			sKit = *((g_pChitin->pScreenCreateChar->Kit_Class_Race.pDataArray) + (g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols * row + col));
		} else {
			sKit = g_pChitin->pScreenCreateChar->Kit_Class_Race.defaultVal;
		}
		int nKitId;
		sscanf_s((LPCTSTR)sKit, "%d", &nKitId);

		if (nKitId == 0) { //true class
			if (m_class) {
				pGame->GetDetailedClassString(m_class, KIT_TRUECLASS, 0, sKitName, &cre);
			} else {
				pGame->GetDetailedClassString(o.GetClass(), KIT_TRUECLASS, 0, sKitName, &cre);
			}
		} else {
			int col2 = 1;

			IECString sKitLowerStrRef;
			if (col2 < g_pChitin->pGame->KITLIST.nCols &&
				nKitId < g_pChitin->pGame->KITLIST.nRows &&
				col2 >= 0 &&
				nKitId >= 0) {
				sKitLowerStrRef = *((g_pChitin->pGame->KITLIST.pDataArray) + (g_pChitin->pGame->KITLIST.nCols * nKitId + col2));
			} else {
				sKitLowerStrRef = g_pChitin->pGame->KITLIST.defaultVal;
			}
			STRREF strrefLower;
			sscanf_s((LPCTSTR)sKitLowerStrRef, "%d", &strrefLower);
			sKitName = GetTlkString(strrefLower);
		}

		switch (row - scrollKit.nCurrentValue) {
		case 0:
			option1.SetText(sKitName);
			break;
		case 1:
			option2.SetText(sKitName);
			break;
		case 2:
			option3.SetText(sKitName);
			break;
		case 3:
			option4.SetText(sKitName);
			break;
		case 4:
			option5.SetText(sKitName);
			break;
		case 5:
			option6.SetText(sKitName);
			break;
		case 6:
			option7.SetText(sKitName);
			break;
		case 7:
			option8.SetText(sKitName);
			break;
		case 8:
			option9.SetText(sKitName);
			break;
		case 9:
			option10.SetText(sKitName);
			break;
		case 10:
			option11.SetText(sKitName);
			break;
		}

	} //for (row)

	for (int index = 0; index < 11; index++) {
		CUICheckButton* pButton;
		switch (index) {
		case 0:
			pButton = &option1;
			break;
		case 1:
			pButton = &option2;
			break;
		case 2:
			pButton = &option3;
			break;
		case 3:
			pButton = &option4;
			break;
		case 4:
			pButton = &option5;
			break;
		case 5:
			pButton = &option6;
			break;
		case 6:
			pButton = &option7;
			break;
		case 7:
			pButton = &option8;
			break;
		case 8:
			pButton = &option9;
			break;
		case 9:
			pButton = &option10;
			break;
		case 10:
			pButton = &option11;
			break;
		}

		index += scrollKit.nCurrentValue;

		BOOL bKitAllowedByAlignment = FALSE;
		BOOL bAlignmentChecked = FALSE;
		int nKitId = 0;
		if ((index < Kit_Class_Race.nRows) && (nChargenProgress == 5)) {
			sAlignment = pGame->GetAlignmentString(o.Alignment);

			int col = 0;

			IECString sKit;
			if (col < g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols &&
				index < g_pChitin->pScreenCreateChar->Kit_Class_Race.nRows &&
				col >= 0 &&
				index >= 0) {
				sKit = *((g_pChitin->pScreenCreateChar->Kit_Class_Race.pDataArray) + (g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols * index + col));
			} else {
				sKit = g_pChitin->pScreenCreateChar->Kit_Class_Race.defaultVal;
			}
			sscanf_s((LPCTSTR)sKit, "%d", &nKitId);

			col = 0;

			if (col < g_pChitin->pGame->KITLIST.nCols &&
				nKitId < g_pChitin->pGame->KITLIST.nRows &&
				col >= 0 &&
				nKitId >= 0) {
				sRowName = *((g_pChitin->pGame->KITLIST.pDataArray) + (g_pChitin->pGame->KITLIST.nCols * nKitId + col));
			} else {
				sRowName = g_pChitin->pGame->KITLIST.defaultVal;
			}

			IECString sKitAllowedByAlignment = pGame->ALIGNMNT.GetValue(sAlignment, sRowName);
			sscanf_s((LPCTSTR)sKitAllowedByAlignment, "%d", &bKitAllowedByAlignment);
			bAlignmentChecked = TRUE;
		}

		if (index < Kit_Class_Race.nRows) {
			pButton->SetEnabled(TRUE);
			if (bAlignmentChecked && nKitId) { //not true class
				//if (bAlignmentChecked && bKitAllowedByAlignment) {
                if (bKitAllowedByAlignment) {
					pButton->SetActive(TRUE);
				} else {
					pButton->SetActive(FALSE);
				}
			} else {
				pButton->SetActive(TRUE);
			}

		} else {
			pButton->SetEnabled(FALSE);
			pButton->SetActive(FALSE);
			pButton->SetVisible(FALSE);
		}

		index -= scrollKit.nCurrentValue;

	} //for (index)

	unsigned int dwKit = cre.BaseStats.kitLow | (cre.BaseStats.kitHigh << 16);

	option1.SetToggleState(false);
	option2.SetToggleState(false);
	option3.SetToggleState(false);
	option4.SetToggleState(false);
	option5.SetToggleState(false);
	option6.SetToggleState(false);
	option7.SetToggleState(false);
	option8.SetToggleState(false);
	option9.SetToggleState(false);
	option10.SetToggleState(false);
	option11.SetToggleState(false);

	if ((dwKit & KIT_TRUECLASS) && (dwKit & 0xBFFF)) { //has a kit
		unsigned int dwKitOnly = dwKit & 0xBFFF;

		for (int row = 0; row < Kit_Class_Race.nRows; row++ ) {
			int col = 0;

			IECString sKit;
			if (col < g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols &&
				row < g_pChitin->pScreenCreateChar->Kit_Class_Race.nRows &&
				col >= 0 &&
				row >= 0) {
				sKit = *((g_pChitin->pScreenCreateChar->Kit_Class_Race.pDataArray) + (g_pChitin->pScreenCreateChar->Kit_Class_Race.nCols * row + col));
			} else {
				sKit = g_pChitin->pScreenCreateChar->Kit_Class_Race.defaultVal;
			}
			unsigned int nKitId;
			sscanf_s((LPCTSTR)sKit, "%d", &nKitId);

			if (nKitId == dwKitOnly) {
			switch (row - scrollKit.nCurrentValue) {
				case 0:
					option1.SetToggleState(TRUE);
					break;
				case 1:
					option2.SetToggleState(TRUE);
					break;
				case 2:
					option3.SetToggleState(TRUE);
					break;
				case 3:
					option4.SetToggleState(TRUE);
					break;
				case 4:
					option5.SetToggleState(TRUE);
					break;
				case 5:
					option6.SetToggleState(TRUE);
					break;
				case 6:
					option7.SetToggleState(TRUE);
					break;
				case 7:
					option8.SetToggleState(TRUE);
					break;
				case 8:
					option9.SetToggleState(TRUE);
					break;
				case 9:
					option10.SetToggleState(TRUE);
					break;
				case 10:
					option11.SetToggleState(TRUE);
					break;
				}
			}
		} //for
	} else {
		switch (dwKit) {
			case KIT_TRUECLASS:
				switch (scrollKit.nCurrentValue) {
				case 0:
					option1.SetToggleState(TRUE);
					break;
				default:
					break;
				}
				break;
			case KIT_BERSERKER:
			case KIT_CAVALIER:
			case KIT_FERALAN:
			case KIT_ASSASIN:
			case KIT_BLADE:
			case KIT_GODTALOS:
			case KIT_TOTEMIC:
				switch (scrollKit.nCurrentValue) {
				case 0:
					option2.SetToggleState(TRUE);
					break;
				case 1:
					option1.SetToggleState(TRUE);
					break;
				default:
					break;
				}
				break;
			case KIT_WIZARDSLAYER:
			case KIT_INQUISITOR:
			case KIT_STALKER:
			case KIT_BOUNTYHUNTER:
			case KIT_JESTER:
			case KIT_GODHELM:
			case KIT_SHAPESHIFTER:
				switch (scrollKit.nCurrentValue) {
				case 0:
					option1.SetToggleState(TRUE);
					break;
				case 1:
					option2.SetToggleState(TRUE);
					break;
				case 2:
					option3.SetToggleState(TRUE);
					break;
				default:
					break;
				}
				break;
			case KIT_KENSAI:
			case KIT_UNDEADHUNTER:
			case KIT_BEASTMASTER:
			case KIT_SWASHBUCKLER:
			case KIT_SKALD:
			case KIT_GODLATHANDER:
			case KIT_BEASTFRIEND:
				switch (scrollKit.nCurrentValue) {
				case 0:
					option1.SetToggleState(TRUE);
					break;
				case 1:
					option2.SetToggleState(TRUE);
					break;
				case 2:
					option3.SetToggleState(TRUE);
					break;
				case 3:
					option4.SetToggleState(TRUE);
					break;
				default:
					break;
				}
				break;
			default:
				break;
		}
	} //has a kit

	CUIButton& buttonDone = (CUIButton&)panel.GetUIControl(7);
	buttonDone.SetActive(CanContinue(cre));

	scrollKit.nValues = Kit_Class_Race.nRows;
	scrollKit.nRows = 11;
	if (Kit_Class_Race.nRows > 11) {
		scrollKit.SetEnabled(TRUE);
		scrollKit.SetVisible(TRUE);
	} else {
		scrollKit.SetEnabled(FALSE);
		scrollKit.SetVisible(FALSE);
	}

	if (pGameOptionsEx->bDebugVerbose) {
		
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_KitPanelOnUpdate(): nKits(%d), nScrollValues(%d), wKit(0x%X)\r\n";
		console.writef(lpsz, Kit_Class_Race.nRows, max(scrollKit.nValues - scrollKit.nRows, 0), dwKit);
		L.timestamp();
		L.appendf(lpsz, Kit_Class_Race.nRows, scrollKit.nValues, dwKit);
	}

	return;
};

void DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnUpdate(CPanel& panel, CCreatureObject& cre) {
	CUIScrollBarChargenMageSpell& scroll = (CUIScrollBarChargenMageSpell&)panel.GetUIControl(31);
	if (&scroll == NULL) {
		LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnUpdate(): Chargen mage spell selection scroll bar not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenCharGen_MageBookPanelOnUpdate)(panel, cre);
	}

	//Depending on offset in MageBookSpells, transfer spells to/from temporary spell pile
	int nPileCount = scroll.cplTempSpells.GetCount();
	int nSliderCount = 6 * scroll.nCurrentValue;

	if (nSliderCount > nPileCount) {
		int i = nSliderCount - nPileCount;
		while (i) {
			if (MageBookSpells.IsEmpty()) {
				LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnUpdate(): Mage book is empty\r\n";
				console.write(lpsz);
				L.timestamp();
				L.append(lpsz);
				break;
			}
			scroll.cplTempSpells.AddHead(MageBookSpells.RemoveHead());
			i--;
		}
	}

	if (nSliderCount < nPileCount) {
		int i = nPileCount - nSliderCount;
		while (i) {
			if (scroll.cplTempSpells.IsEmpty()) {
				LPCTSTR lpsz = "DETOUR_CScreenCharGen::DETOUR_MageBookPanelOnUpdate(): Mage spell pile is empty\r\n";
				console.write(lpsz);
				L.timestamp();
				L.append(lpsz);
				break;
			}
			MageBookSpells.AddHead(scroll.cplTempSpells.RemoveHead());
			i--;
		}
	}

	(this->*Tramp_CScreenCharGen_MageBookPanelOnUpdate)(panel, cre);

	//Update on/off status of spell buttons
	for (int i = 2; i <= 25; i++ ) {
		CCreatureObject* pCre = &cre;
		char threadNum = THREAD_ASYNCH;
		char threadVal;
		do {
			threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(g_pChitin->pScreenCreateChar->eChar, threadNum, &pCre, INFINITE);
		} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

		if (threadVal == OBJECT_SUCCESS) {
			CUICheckButtonChargenMageSpell& control = (CUICheckButtonChargenMageSpell&)panel.GetUIControl(i);
			if (UserCommon_HasKnownSpell(&cre, control.m_rSpell, nCurrentMageBookLevel)) {
				control.bToggle = TRUE;
			} else {
				control.bToggle = FALSE;
			}
			control.SetRedraw();
			g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(g_pChitin->pScreenCreateChar->eChar, threadNum, INFINITE);
		}
	}

	if (scroll.cplTempSpells.GetCount() + MageBookSpells.GetCount() > 24) {
		scroll.SetEnabled(TRUE);
		scroll.SetVisible(TRUE);
	} else {
		scroll.SetEnabled(FALSE);
		scroll.SetVisible(FALSE);
	}

	return;
}

void DETOUR_CScreenCharGen::DETOUR_ClassPanelOnUpdate(CPanel& panel, CCreatureObject& cre) {
	if (!pRuleEx->m_ClassRaceReq.m_2da.bLoaded) return (this->*Tramp_CScreenCharGen_ClassPanelOnUpdate)(panel, cre);

	CUIScrollBar& scroll = (CUIScrollBar&)panel.GetUIControl(12);
	pScrollActive = &scroll;
	CInfGame* pGame = g_pChitin->pGame;
	CreFileData& stats = cre.BaseStats;
	Object& o = cre.oBase;
	unsigned int dwKit = stats.kitLow | (stats.kitHigh << 16);

	for (int i = 2; i <= 9; i++) {
		CUICheckButtonChargenClass& button = (CUICheckButtonChargenClass&)panel.GetUIControl(i);
		if (button.GetClass() == o.Class) {
			if (o.Class == CLASS_FIGHTER && dwKit == KIT_BARBARIAN) {
				button.SetToggleState(FALSE);
			} else {
				button.SetToggleState(TRUE);
			}
		} else {
			button.SetToggleState(FALSE);
		}

		IECString sRace = pGame->GetRaceString(o.Race);
		IECString sClass = pGame->GetClassString(button.GetClass(), KIT_TRUECLASS);
		IECString sAllowed = pRuleEx->m_ClassRaceReq.GetValue(sRace, sClass);
		BOOL bAllowed;
		sscanf_s((LPCTSTR)sAllowed, "%d", &bAllowed);
		button.SetActive(bAllowed);
	}

    int maxbutton;
    if (pGameOptionsEx->bEngineShamanClass)
        maxbutton = 18;
    else
        maxbutton = 17;

	for (int i = 15; i <= maxbutton; i++) {
		CUICheckButtonChargenClass& button = (CUICheckButtonChargenClass&)panel.GetUIControl(i);
		if (button.GetClass() == o.Class) {
			if (o.Class == CLASS_FIGHTER && dwKit != KIT_BARBARIAN) {
				button.SetToggleState(FALSE);
			} else {
				button.SetToggleState(TRUE);
			}
		} else {
			button.SetToggleState(FALSE);
		}

		IECString sRace = pGame->GetRaceString(o.Race);
		IECString sClass = pGame->GetClassString(button.GetClass(), KIT_TRUECLASS);
		IECString sAllowed = pRuleEx->m_ClassRaceReq.GetValue(sRace, sClass);
		BOOL bAllowed;
		sscanf_s((LPCTSTR)sAllowed, "%d", &bAllowed);
		button.SetActive(bAllowed);
	}

	//CUIButton& buttonMulticlass = (CUIButton&)panel.GetUIControl(10);
	//buttonMulticlass.SetActive((bool)(o.Race - 1));
	CUIButton& buttonDone = (CUIButton&)panel.GetUIControl(0);
	buttonDone.SetActive(CanContinue(cre));

	return;
}

void DETOUR_CScreenCharGen::DETOUR_MulticlassPanelOnUpdate(CPanel& panel, CCreatureObject& cre) {
	if (!pRuleEx->m_ClassRaceReq.m_2da.bLoaded) return (this->*Tramp_CScreenCharGen_MulticlassPanelOnUpdate)(panel, cre);

	CUIScrollBar& scroll = (CUIScrollBar&)panel.GetUIControl(13);
	pScrollActive = &scroll;
	CInfGame* pGame = g_pChitin->pGame;
	Object& o = cre.oBase;

	for (int i = 2; i <= 11; i++) {
		CUICheckButtonChargenMulticlass& button = (CUICheckButtonChargenMulticlass&)panel.GetUIControl(i);
		button.SetToggleState(button.GetClass() == o.Class);

		IECString sRace = pGame->GetRaceString(o.Race);
		IECString sClass = pGame->GetClassString(button.GetClass(), KIT_TRUECLASS);
		IECString sAllowed = pRuleEx->m_ClassRaceReq.GetValue(sRace, sClass);
		BOOL bAllowed;
		sscanf_s((LPCTSTR)sAllowed, "%d", &bAllowed);
		button.SetActive(bAllowed);
	}

	CUIButton& buttonDone = (CUIButton&)panel.GetUIControl(0);
	buttonDone.SetActive(CanContinue(cre));

	return;
}

void DETOUR_CScreenCharGen::DETOUR_MageSchoolPanelOnUpdate(CPanel& panel, CCreatureObject& cre) {
	if (!pRuleEx->m_MageSchoolRaceReq.m_2da.bLoaded) return (this->*Tramp_CScreenCharGen_MageSchoolPanelOnUpdate)(panel, cre);

	CUIScrollBar& scroll = (CUIScrollBar&)panel.GetUIControl(10);
	pScrollActive = &scroll;
	CInfGame* pGame = g_pChitin->pGame;
	CreFileData& stats = cre.BaseStats;
	Object& o = cre.oBase;
	unsigned int dwKit = stats.kitLow | (stats.kitHigh << 16);
	BOOL bButtonToggled = FALSE;

	BOOL bMageClass = o.GetClass() == CLASS_MAGE || o.GetClass() == CLASS_SORCERER;
	if (!bMageClass) {
		unsigned char nClassNew;
		unsigned char nClassOrg;
		o.GetDualClasses(&nClassNew, &nClassOrg);
		bMageClass = nClassNew == CLASS_MAGE || nClassOrg == CLASS_MAGE;
	}

	for (int i = 2; i <= 9; i++) {
		CUICheckButtonChargenMageSchool& buttonSchool = (CUICheckButtonChargenMageSchool&)panel.GetUIControl(i);
		if (bMageClass &&
			dwKit == buttonSchool.GetKit()) {
			buttonSchool.SetToggleState(TRUE);
			bButtonToggled = TRUE;
		} else {
			buttonSchool.SetToggleState(FALSE);
		}

		buttonSchool.SetActive(pGame->IsMageSchoolAllowed(buttonSchool.GetKit(), o.Race));
	}

	CUICheckButtonChargenMageSchool& buttonWildMage = (CUICheckButtonChargenMageSchool&)panel.GetUIControl(14);
	if (*g_pEnableWildMage) {
		if (bMageClass &&
			dwKit == buttonWildMage.GetKit()) {
			buttonWildMage.SetToggleState(TRUE);
			bButtonToggled = TRUE;
		} else {
			buttonWildMage.SetToggleState(FALSE);
		}
		buttonWildMage.SetActive(pGame->IsMageSchoolAllowed(buttonWildMage.GetKit(), o.Race));
	} else {
		buttonWildMage.SetToggleState(FALSE);
		buttonWildMage.SetActive(FALSE);
		buttonWildMage.SetEnabled(FALSE);
		buttonWildMage.SetVisible(FALSE);
	}

	CUICheckButtonChargenMageSchool& buttonMage = (CUICheckButtonChargenMageSchool&)panel.GetUIControl(13);
	if (bMageClass &&
		dwKit == buttonMage.GetKit()) {
		buttonMage.SetToggleState(TRUE);
		bButtonToggled = TRUE;
	} else {
		buttonMage.SetToggleState(FALSE);
	}
	buttonMage.SetActive(pGame->IsMageSchoolAllowed(buttonMage.GetKit(), o.Race));

	CUIButton& buttonDone = (CUIButton&)panel.GetUIControl(0);
	buttonDone.SetActive(CanContinue(cre) && pGame->IsMageSchoolAllowed(dwKit, o.Race));

	return;
}

void DETOUR_CScreenCharGen::DETOUR_InitSoundset(CCreatureObject& cre) {
	(this->*Tramp_CScreenCharGen_InitSoundset)(cre);
	EngineCommon_ApplySoundset(cre);
	return;
}

BOOL CScreenCharGen_MageSchoolPanelCanContinue(CCreatureObject& cre) {
	return g_pChitin->pGame->IsMageSchoolAllowed(cre.GetKitUnusableFlag(), cre.oBase.Race);
}


unsigned int __stdcall
Transform_IA_AnimID(Object& obj, unsigned int animid) {
    if (!isInfinityAnimationActive) 
        return 0;

    if (animid < 0x6410 || animid > 0x6447)
        return 0;

    animid &= 0xFF0F;           // 64XY -> 640Y
    switch (obj.Class) {
    case CLASS_CLERIC:
    case CLASS_DRUID:
    case CLASS_CLERIC_MAGE:
    case CLASS_CLERIC_THIEF:
    case CLASS_SHAMAN:
        animid = animid | 0x10; // 640Y -> 641Y     CLERIC
        break;
    case CLASS_FIGHTER:
    case CLASS_PALADIN:
    case CLASS_FIGHTER_MAGE:
    case CLASS_FIGHTER_CLERIC:
    case CLASS_FIGHTER_THIEF:
    case CLASS_FIGHTER_MAGE_THIEF:
    case CLASS_RANGER:
    case CLASS_FIGHTER_DRUID:
    case CLASS_FIGHTER_MAGE_CLERIC:
    case CLASS_CLERIC_RANGER:
      animid = animid | 0x20;   // 640Y -> 642Y     FIGHTER
      break;
    case CLASS_MAGE:
    case CLASS_SORCERER:
      animid = animid | 0x30;   // 640Y -> 643Y     MAGE
      if (animid == 0x6436 || animid == 0x6437) {// HALFLING
          animid &= 0xFF0F;
          animid = animid | 0x10;   // 6436 -> 6416 CLERIC HALFLING
      }
      break;
    case CLASS_THIEF:
    case CLASS_BARD:
    case CLASS_MAGE_THIEF:
      animid = animid | 0x40;   // 640Y -> 644Y     THIEF
      break;
    default:
      return 0;
      break;
    }

    return animid;
}


unsigned short __stdcall
CreateNew_IA_AnimID(WORD Gender, WORD Race, WORD Class, WORD animid) {
    WORD Race_Gender;

    if (!isInfinityAnimationActive) 
        return animid;

    WORD NewClass = (Class >> 4) + 0x10;  // BG2 Cleric =  000 > IA 10
                                          //     Fighter = 100 >    20
                                          //     Mage =    200 >    30
                                          //     Thief =   300 >    40
    
    /*
            IA BG1                               BG2
    0x6410 L_CLERIC_MALE_HUMAN          0x6000 CLERIC_MALE_HUMAN
    0x6411 L_CLERIC_FEMALE_HUMAN        0x6010 CLERIC_FEMALE_HUMAN
    0x6412 L_CLERIC_MALE_ELF            0x6001 CLERIC_MALE_ELF
    0x6413 L_CLERIC_FEMALE_ELF          0x6011 CLERIC_FEMALE_ELF
    0x6414 L_CLERIC_MALE_DWARF          0x6002 CLERIC_MALE_DWARF
    0x6415 L_CLERIC_FEMALE_DWARF        0x6012 CLERIC_FEMALE_DWARF
    0x6416 L_CLERIC_MALE_HALFLING       0x6003 CLERIC_MALE_HALFLING
    0x6417 L_CLERIC_FEMALE_HALFLING     0x6013 CLERIC_FEMALE_HALFLING
    *                                   0x6004 CLERIC_MALE_GNOME
    *                                   0x6014 CLERIC_FEMALE_GNOME
    *                                   0x6005 CLERIC_MALE_HALFORC
    *                                   0x6015 CLERIC_FEMALE_HALFORC

    0x6420 L_FIGHTER_MALE_HUMAN         0x6100 FIGHTER_MALE_HUMAN
    0x6421 L_FIGHTER_FEMALE_HUMAN       0x6110 FIGHTER_FEMALE_HUMAN
    0x6422 L_FIGHTER_MALE_ELF           0x6101 FIGHTER_MALE_ELF
    0x6423 L_FIGHTER_FEMALE_ELF         0x6111 FIGHTER_FEMALE_ELF
    0x6424 L_FIGHTER_MALE_DWARF         0x6102 FIGHTER_MALE_DWARF
    0x6425 L_FIGHTER_FEMALE_DWARF       0x6112 FIGHTER_FEMALE_DWARF
    0x6426 L_FIGHTER_MALE_HALFLING      0x6103 FIGHTER_MALE_HALFLING
    0x6427 L_FIGHTER_FEMALE_HALFLING    0x6113 FIGHTER_FEMALE_HALFLING
    *                                   0x6104 FIGHTER_MALE_GNOME
    *                                   0x6114 FIGHTER_FEMALE_GNOME
    *                                   0x6105 FIGHTER_MALE_HALFORC
    *                                   0x6115 FIGHTER_FEMALE_HALFORC

    0x6430 L_MAGE_MALE_HUMAN            0x6200 MAGE_MALE_HUMAN
    0x6431 L_MAGE_FEMALE_HUMAN          0x6210 MAGE_FEMALE_HUMAN
    0x6432 L_MAGE_MALE_ELF              0x6201 MAGE_MALE_ELF
    0x6433 L_MAGE_FEMALE_ELF            0x6211 MAGE_FEMALE_ELF
    0x6434 L_MAGE_MALE_DWARF            0x6202 MAGE_MALE_DWARF
    0x6435 L_MAGE_FEMALE_DWARF          0x6212 MAGE_FEMALE_DWARF
    *                                   0x6204 MAGE_MALE_GNOME
    *                                   0x6214 MAGE_FEMALE_GNOME
    *                                   0x6205 MAGE_MALE_HALFORC
    *                                   0x6215 MAGE_FEMALE_HALFORC

    0x6440 L_THIEF_MALE_HUMAN
    0x6441 L_THIEF_FEMALE_HUMAN
    0x6442 L_THIEF_MALE_ELF
    0x6443 L_THIEF_FEMALE_ELF
    0x6444 L_THIEF_MALE_DWARF
    0x6445 L_THIEF_FEMALE_DWARF
    0x6446 L_THIEF_MALE_HALFLING
    0x6447 L_THIEF_FEMALE_HALFLING

    */

    switch (Gender + Race) {
    case 0x00:
        Race_Gender = 0x0; break;
    case 0x10:
        Race_Gender = 0x1; break;
    case 0x01:
        Race_Gender = 0x2; break;
    case 0x11:
        Race_Gender = 0x3; break;
    case 0x02:
        Race_Gender = 0x4; break;
    case 0x12:
        Race_Gender = 0x5; break;
    case 0x03:
        Race_Gender = 0x6; break;
    case 0x13:
        Race_Gender = 0x7; break;
    case 0x04:
        Race_Gender = 0x4; break;   // GNOME   > DWARF
    case 0x14:
        Race_Gender = 0x5; break;   // GNOME   > DWARF
    case 0x05:
        Race_Gender = 0x0; break;   // HALFORC > HUMAN
    case 0x15:
        Race_Gender = 0x1; break;   // HALFORC > HUMAN
    default:
        Race_Gender = 0x0; break;
    }

    return (0x6400 | NewClass | Race_Gender);
}


void  __declspec(naked)
ScreenCharacter_MakeDualClass_asm() {
__asm {

    push    ecx
    push    edx

    push    ecx             ; anim_id
    lea     eax, [ebp-908h] ; Object
    push    eax
    call    Transform_IA_AnimID

    test    eax,eax
    pop     edx
    pop     ecx
    jz      BG2Type_L

    mov     [ebp-8F4h], eax ; anim_id <- IA anim_id

    add     esp, 4
    push    06F0972h  ; skip orig code
    ret

BG2Type_L:
    and     ecx, 0F0FFh ; stolen bytes
    ret
}
}


void  __declspec(naked)
ScreenCreateChar_CompleteCharacterClass_asm() {
__asm {
    push    ecx
    push    edx

    push    eax             ; orig animid
    push    [ebp-8D0h]      ; Class
    push    [ebp-10h]       ; Race
    push    [ebp-8F0h]      ; Gender
    call    CreateNew_IA_AnimID

    pop     edx
    pop     ecx
    ; eax - animid
    mov     [ebp-8D8h], ax  ; stolen bytes
    ret
}
}