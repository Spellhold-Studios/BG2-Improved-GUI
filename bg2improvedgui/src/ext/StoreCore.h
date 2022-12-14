#ifndef STORECORE_H
#define STORECORE_H

#include "stdafx.h"
#include "stocore.h"

void __stdcall CStore_AddItem_SetUsages(CStore& sto, CItem& itm, StoFileItem& stoitm, unsigned int nUsageIdx);
void __stdcall CStore_GetBuyPrice_GetChargePercent(CStore& sto, short& nTotalUsages, short& nTotalCharges, CItem& itm, int nUsageIdx);
void __stdcall CStore_GetSellPrice_GetChargePercent(CStore& sto, short& nTotalUsages, short& nTotalCharges, CItem& itm, int nUsageIdx);
void __stdcall CStore_UnmarshalItem_SetUsages(CItem& itm, StoFileItem& stoitm, unsigned int nUsageIdx);

void ScreenStoreRightPanelButtonClick_CheckShiftKey_asm();
void ScreenStoreLeftPanelButtonClick_CheckShiftKey_asm();
void ScreenStoreLeftPanel_FetchItem_asm();
void ScreenStoreLeftPanelButtonClick_asm();
void ScreenStoreLeftPanelDblButtonClick_asm();
void ScreenStoreLeftPanelRButtonClick_asm();
void ScreenStoreLeftPanel_AdjustScrollBar_asm();
void ScreenStoreRightPanel_AdjustScrollBar_asm();
void ScreenStore_GetLeftListCountEAX_asm();
void ScreenStore_GetLeftListCountEDX_asm();
void ScreenStore_GetLeftListCountECX_asm();
void CScreenStore_OnBuyItemButtonClick_IncrementItemIndex_asm();


#endif //STORECORE_H