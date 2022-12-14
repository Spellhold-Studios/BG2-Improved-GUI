#include "EngineRecord.h"

#include <cassert>

#include "stdafx.h"
#include "objcre.h"
#include "objcore.h"
#include "chitin.h"
#include "uicore.h"
#include "infgame.h"
#include "console.h"
#include "log.h"
#include "EngineCommon.h"
#include "UserRecMageSpell.h"
#include "UserCommon.h"
#include "dbgcore.h"

void (CScreenRecord::*Tramp_CScreenRecord_MageBookPanelOnLoad)(CCreatureObject&) =
	SetFP(static_cast<void (CScreenRecord::*)(CCreatureObject&)>	(&CScreenRecord::MageBookPanelOnLoad),	0x6E0FE6);
void (CScreenRecord::*Tramp_CScreenRecord_MageBookPanelOnUpdate)(CCreatureObject&) =
	SetFP(static_cast<void (CScreenRecord::*)(CCreatureObject&)>	(&CScreenRecord::MageBookPanelOnUpdate),0x6E563F);
void (CScreenRecord::*Tramp_CScreenRecord_UpdateCharacter)() =
	SetFP(static_cast<void (CScreenRecord::*)()>					(&CScreenRecord::UpdateCharacter),		0x6E9F57);
void (CScreenRecord::*Tramp_CScreenRecord_UpdateStyleBonus)(CUITextArea&, uint, int) =
	SetFP(static_cast<void (CScreenRecord::*)(CUITextArea&, uint, int)> (&CScreenRecord::UpdateStyleBonus),	0x6D9672);

void DETOUR_CScreenRecord::DETOUR_MageBookPanelOnLoad(CCreatureObject& cre) {
	CPanel& panel = manager.GetPanel(8);
	assert(&panel != NULL);
	CUIScrollBarRecMageSpell& scroll = (CUIScrollBarRecMageSpell&)panel.GetUIControl(30);
	
	if (&scroll == NULL) {
		LPCTSTR lpsz = "DETOUR_MageBookPanelOnLoad(): Level-up mage spell selection scroll bar not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenRecord_MageBookPanelOnLoad)(cre);
	}

	//clear values
	scroll.nCurrentValue = 0;
	scroll.nValues = 0;
	scroll.nRows = 0;
	scroll.cplTempSpells.RemoveAll();

	(this->*Tramp_CScreenRecord_MageBookPanelOnLoad)(cre);

	if (this->bHighLevelAbility == FALSE) {
		scroll.nValues = MageBookSpells.GetCount() / 5;
		if (MageBookSpells.GetCount() % 5) scroll.nValues++;
		scroll.nRows = 5;
	}

	scroll.UpdateKnobPosition(scroll.nCurrentValue, scroll.nValues, scroll.nRows);

	return;
}

void DETOUR_CScreenRecord::DETOUR_MageBookPanelOnUpdate(CCreatureObject& cre) {
	CPanel& panel = manager.GetPanel(8);
	assert(&panel != NULL);
	CUIScrollBarRecMageSpell& scroll = (CUIScrollBarRecMageSpell&)panel.GetUIControl(30);

	if (&scroll == NULL) {
		LPCTSTR lpsz = "DETOUR_MageBookPanelOnUpdate(): Level-up mage spell selection scroll bar not found\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
		return (this->*Tramp_CScreenRecord_MageBookPanelOnUpdate)(cre);
	}

	//do not apply to selecting high level abilities
	if (this->bHighLevelAbility == TRUE) {
		(this->*Tramp_CScreenRecord_MageBookPanelOnUpdate)(cre);
		scroll.SetEnabled(FALSE);
		scroll.SetVisible(FALSE);
		return;
	}

	//Depending on offset in MageBookSpells, transfer spells to/from temporary spell pile
	int nPileCount = scroll.cplTempSpells.GetCount();
	int nSliderCount = 5 * scroll.nCurrentValue;

	if (nSliderCount > nPileCount) {
		int i = nSliderCount - nPileCount;
		while (i) {
			if (MageBookSpells.IsEmpty()) {
				LPCTSTR lpsz = "DETOUR_MageBookPanelOnUpdate(): Mage book is empty\r\n";
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
				LPCTSTR lpsz = "DETOUR_MageBookPanelOnUpdate(): Mage spell pile is empty\r\n";
				console.write(lpsz);
				L.timestamp();
				L.append(lpsz);
				break;
			}
			MageBookSpells.AddHead(scroll.cplTempSpells.RemoveHead());
			i--;
		}
	}

	(this->*Tramp_CScreenRecord_MageBookPanelOnUpdate)(cre);

	//Update on/off status of spell buttons
	for (int i = 0; i <= 24; i++ ) {
		Enum eChar = ENUM_INVALID_INDEX;
		CInfGame* pGame = g_pChitin->pGame;
		CScreenRecord* pCharacter = g_pChitin->pScreenCharacter;
		int nPlayerIdx = pCharacter->GetActivePlayerIdx();
		if (nPlayerIdx < pGame->numInParty) {
			eChar = pGame->ePlayersPartyOrder[nPlayerIdx];
		}

		CCreatureObject* pCre = &cre;
		char threadNum = THREAD_ASYNCH;
		char threadVal;
		do {
			threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eChar, threadNum, &pCre, INFINITE);
		} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

		if (threadVal == OBJECT_SUCCESS) {
			CUICheckButtonRecMageSpell& control = (CUICheckButtonRecMageSpell&)panel.GetUIControl(i);
			if (&control != NULL) {
				if (UserCommon_HasKnownSpell(&cre, control.m_rSpell, nCurrentMageBookLevel)) {
					control.bToggle = TRUE;
				} else {
					control.bToggle = FALSE;
				}
				control.SetRedraw();
			} else {
				LPCTSTR lpsz = "DETOUR_MageBookPanelOnUpdate(): pButton == NULL\r\n";
				L.timestamp();
				L.append(lpsz);
				console.write(lpsz);
			}
			g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eChar, threadNum, INFINITE);
		}
	}

	if (scroll.cplTempSpells.GetCount() + MageBookSpells.GetCount() > 25) {
		scroll.SetEnabled(TRUE);
		scroll.SetVisible(TRUE);
	} else {
		scroll.SetEnabled(FALSE);
		scroll.SetVisible(FALSE);
	}

	return;
}

void DETOUR_CScreenRecord::DETOUR_UpdateCharacter() {
	CPanel& cp = GetTopPanel();
	assert(&cp != NULL);

	(this->*Tramp_CScreenRecord_UpdateCharacter)();

	if (cp.index - 3 == 17) {

		CInfGame* pGame = g_pChitin->pGame;
		assert(pGame);

		Enum ePlayer1;
		if (u141c) {
			ePlayer1 = g_pChitin->pScreenCreateChar->eChar;
		} else {
			if (nActivePlayerIdx < pGame->numInParty) {
				ePlayer1 = pGame->ePlayersPartyOrder[nActivePlayerIdx];
			} else {
				ePlayer1 = ENUM_INVALID_INDEX;
			}
		}

		CCreatureObject* pCre;
		char threadVal;
		do {
			threadVal = pGame->m_GameObjectArrayHandler.GetGameObjectDeny(ePlayer1, THREAD_ASYNCH, &pCre, INFINITE);
		} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

		if (threadVal == OBJECT_SUCCESS) {
			EngineCommon_ApplySoundset(*pCre);
			pGame->m_GameObjectArrayHandler.FreeGameObjectDeny(ePlayer1, THREAD_ASYNCH, INFINITE);
		} else {
			LPCTSTR lpsz = "DETOUR_UpdateCharacter(): FreeGameObjectDeny returned %d\r\n";
			console.writef(lpsz, threadVal);
			L.timestamp();
			L.appendf(lpsz, threadVal);
			return;
		}

	} //cp.index

	return;
}


void
DETOUR_CScreenRecord::DETOUR_UpdateStyleBonus(CUITextArea& TextArea, uint ActiveWeaponStyle, int nLevel) {
    IECString Str;
    IECString tmpstr;
    bool      someprinted = false;  

    switch ( ActiveWeaponStyle ) {
    case STATS_PROFICIENCY2HANDED:
      if (nLevel != 0) {
          someprinted = true;

          Str = GetTlkString(10336);        // ~Damage~
          CEngine_UpdateText(*this, TextArea, "%s: +1", (LPCTSTR)Str);

          Str = GetTlkString(32140);        // ~Critical Hit~
          CEngine_UpdateText(*this, TextArea, "%s: +1", (LPCTSTR)Str);

          if ( nLevel > 0 && nLevel <= 2 ) {
            Str = GetTlkString(32141);      // ~Weapon Speed~
            CEngine_UpdateText(*this, TextArea, "%s: %d", (LPCTSTR)Str, -2 * nLevel);
          }
      }
      break;
    
    case STATS_PROFICIENCYSWORDANDSHIELD:
      if (nLevel != 0) {
          if ( nLevel <= 0 || nLevel > 2 ) {
            AssertFailedContinue(1561, "D:\\dev\\baldur\\InfScreenCharacter.cpp", "FALSE", 0);
          }
          else {
            someprinted = true;

            Str = GetTlkString(10340);      // ~Missile Adjustment~
            CEngine_UpdateText(*this, TextArea, "%s: %d", (LPCTSTR)Str, -2 * nLevel);
          }
      }
      break;

    case STATS_PROFICIENCYSINGLEWEAPON:
      if (nLevel != 0) {
          someprinted = true;

          Str = GetTlkString(32140);        // ~Critical Hit~
          CEngine_UpdateText(*this, TextArea, "%s: +1", (LPCTSTR)Str);
          if ( nLevel <= 0 || nLevel > 2 ) {
            AssertFailedContinue(1549, "D:\\dev\\baldur\\InfScreenCharacter.cpp", "FALSE", 0);
          }
          else {
            Str = GetTlkString(10339);      // ~Armor Class~
            CEngine_UpdateText(*this, TextArea, "%s: %d", (LPCTSTR)Str, -nLevel);
          }
      }
      break;

    case STATS_PROFICIENCY2WEAPON:
      switch (nLevel) {
        case 0:
          someprinted = true;

          Str = GetTlkString(56911);    // ~Main Hand THAC0~
          CEngine_UpdateText(*this, TextArea, "%s: -4", (LPCTSTR)Str);

          Str = GetTlkString(56910);    // ~Off Hand THAC0~
          CEngine_UpdateText(*this, TextArea, "%s: -8", (LPCTSTR)Str);
          break;
        case 1:
          someprinted = true;

          Str = GetTlkString(56911);
          CEngine_UpdateText(*this, TextArea, "%s: -2", (LPCTSTR)Str);

          Str = GetTlkString(56910);
          CEngine_UpdateText(*this, TextArea, "%s: -6", (LPCTSTR)Str);
          break;
        case 2:
          someprinted = true;

          Str = GetTlkString(56911);
          CEngine_UpdateText(*this, TextArea, "%s:  0", (LPCTSTR)Str);

          Str = GetTlkString(56910);
          CEngine_UpdateText(*this, TextArea, "%s: -4", (LPCTSTR)Str);
          break;
        case 3:
          someprinted = true;

          Str = GetTlkString(56911);
          CEngine_UpdateText(*this, TextArea, "%s:  0", (LPCTSTR)Str);

          Str = GetTlkString(56910);
          CEngine_UpdateText(*this, TextArea, "%s: -2", (LPCTSTR)Str);
          break;
        default:
          AssertFailedContinue(1581, "D:\\dev\\baldur\\InfScreenCharacter.cpp", "FALSE", 0);
          break;
      }
      break;

    case -1:    // ranged/ammo/launcher weapon
      break;

    default:
      AssertFailedContinue(1586, "D:\\dev\\baldur\\InfScreenCharacter.cpp", "FALSE", 0);
      break;
    }

    // Specialization penalty
	Enum eChar = ENUM_INVALID_INDEX;
	CInfGame* pGame = g_pChitin->pGame;
	CScreenRecord* pCharacter = g_pChitin->pScreenCharacter;
	int nPlayerIdx = pCharacter->GetActivePlayerIdx();
	if (nPlayerIdx < pGame->numInParty) {
		eChar = pGame->ePlayersPartyOrder[nPlayerIdx];
	}

	CCreatureObject* Cre;
	char threadNum = THREAD_ASYNCH;
	char threadVal;
	do {
		threadVal = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(eChar, threadNum, &Cre, INFINITE);
	} while (threadVal == OBJECT_SHARING || threadVal == OBJECT_DENYING);

	if (threadVal == OBJECT_SUCCESS) {
        short   launcherSlot;
        short   stars_main, stars_off;
        CItem*  Item = Cre->Inventory.items[Cre->Inventory.nSlotSelected];

        if (Item) {
            Item->Demand();

            CItem*  OffHandWeapon = Cre->Inventory.items[SLOT_SHIELD];
            if (OffHandWeapon && OffHandWeapon->GetType() == 12)  // Shield
                OffHandWeapon = 0;

            ItmFileAbility* ItemAbility = Item->GetAbility(Cre->Inventory.nAbilitySelected);
            if (ItemAbility) {
                CItem* ItemLauncher = Cre->GetFirstEquippedLauncherOfAbility(ItemAbility, launcherSlot);
                if (ItemLauncher) {
                    ItemLauncher->Demand();
                    stars_main = Cre->GetProficiencyInItem(*ItemLauncher);
                    ItemLauncher->Release();
                } else {
                    stars_main = Cre->GetProficiencyInItem(*Item);
                }

                if (OffHandWeapon) {
                    OffHandWeapon->Demand();
                    stars_off = Cre->GetProficiencyInItem(*OffHandWeapon);
                    OffHandWeapon->Release();
                } else {
                    stars_off = 0;
                }


                // WSPECIAL.2da
                short nCol = 0;             // HIT
	            short nRow  = stars_main;
                short nRowO = stars_off;
                int Value, ValueO;
	            IECString sHit, sHitO;
	            if (nCol  < g_pChitin->pGame->WSPECIAL.nCols &&
		            nRow  < g_pChitin->pGame->WSPECIAL.nRows &&
		            nCol  >= 0 &&
		            nRow  >= 0 &&
                    nRowO >= 0) {
		            sHit  = *((g_pChitin->pGame->WSPECIAL.pDataArray) + (g_pChitin->pGame->WSPECIAL.nCols * nRow  + nCol));
                    sHitO = *((g_pChitin->pGame->WSPECIAL.pDataArray) + (g_pChitin->pGame->WSPECIAL.nCols * nRowO + nCol));
	            } else {
		            sHit  = g_pChitin->pGame->WSPECIAL.defaultVal;
                    sHitO = g_pChitin->pGame->WSPECIAL.defaultVal;
	            }
	            sscanf_s((LPCTSTR)sHit,  "%d", &Value);
                sscanf_s((LPCTSTR)sHitO, "%d", &ValueO);


                if (stars_main == 0) {
                    someprinted = true;
                    int penalty = 0;

                    Str = GetTlkString(6551);   // ~Specialization THAC0~
                    if (Cre->o.HasActiveSubclass(CLASS_RANGER,1) ||
                        Cre->o.HasActiveSubclass(CLASS_PALADIN,1) ||
                        Cre->o.HasActiveSubclass(CLASS_FIGHTER,1) ||
                        Cre->o.HasActiveSubclass(CLASS_MONK,1))
                        penalty = 2;
                    else
                    if (Cre->o.HasActiveSubclass(CLASS_CLERIC,1) ||
                        Cre->o.HasActiveSubclass(CLASS_DRUID,1) ||
                        Cre->o.HasActiveSubclass(CLASS_SHAMAN,1))
                        penalty = 3;
                    else
                    if (Cre->o.HasActiveSubclass(CLASS_MAGE,1) ||
                        Cre->o.HasActiveSubclass(CLASS_SORCERER,1))
                        penalty = 5;
                    else
                    if (Cre->o.HasActiveSubclass(CLASS_THIEF,1) ||
                        Cre->o.HasActiveSubclass(CLASS_BARD,1))
                        penalty = 3;

                    CEngine_UpdateText(*this, TextArea, "%s: -%d", (LPCTSTR)Str, penalty);
                } else
                if (Value > 0) {
                    someprinted = true;

                    Str = GetTlkString(6551);   // ~Specialization THAC0~
                    CEngine_UpdateText(*this, TextArea, "%s: +%s", (LPCTSTR)Str, (LPCTSTR)sHit);
                }

                if (ActiveWeaponStyle == STATS_PROFICIENCY2WEAPON) {
                    if (stars_off == 0 ) {
                        someprinted = true;
                        int penalty = 0;

                        Str = GetTlkString(6552);   // ~Off Hand Specialization THAC0~
                        if (Cre->o.HasActiveSubclass(CLASS_RANGER,1) ||
                            Cre->o.HasActiveSubclass(CLASS_PALADIN,1) ||
                            Cre->o.HasActiveSubclass(CLASS_FIGHTER,1) ||
                            Cre->o.HasActiveSubclass(CLASS_MONK,1))
                            penalty = 2;
                        else
                        if (Cre->o.HasActiveSubclass(CLASS_CLERIC,1) ||
                            Cre->o.HasActiveSubclass(CLASS_DRUID,1) ||
                            Cre->o.HasActiveSubclass(CLASS_SHAMAN,1))
                            penalty = 3;
                        else
                        if (Cre->o.HasActiveSubclass(CLASS_MAGE,1) ||
                            Cre->o.HasActiveSubclass(CLASS_SORCERER,1))
                            penalty = 5;
                        else
                        if (Cre->o.HasActiveSubclass(CLASS_THIEF,1) ||
                            Cre->o.HasActiveSubclass(CLASS_BARD,1))
                            penalty = 3;

                        CEngine_UpdateText(*this, TextArea, "%s: -%d", (LPCTSTR)Str, penalty);
                    } else 
                    if (ValueO > 0) {
                        someprinted = true;

                        Str = GetTlkString(6552);   // ~Off Hand Specialization THAC0~
                        CEngine_UpdateText(*this, TextArea, "%s: +%s", (LPCTSTR)Str, (LPCTSTR)sHitO);
                    }
                }
                
                if (someprinted == false) {
                    Str = GetTlkString(61560);      // ~None~
                    CEngine_UpdateText(*this, TextArea, "%s", (LPCTSTR)Str);
                }
            }

            Item->Release();
        }

        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(eChar, threadNum, INFINITE);
    }
}

