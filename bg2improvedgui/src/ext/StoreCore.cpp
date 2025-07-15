#include "StoreCore.h"
#include "ChitinCore.h"
#include "engstore.h"

_n void CScreenStore::GetStoreItem(int nIndex, CScreenStoreItem& cItem) { _bgmain(0x7A2624)}

void __stdcall CStore_AddItem_SetUsages(CStore& sto, CItem& itm, StoFileItem& stoitm, unsigned int nUsageIdx) {
    if (0) IECString("CStore_AddItem_SetUsages");

    if (itm.GetNumCharges(nUsageIdx) > 0) {
        if (
            (
                sto.sto.nType == STORETYPE_BAG &&
                !(sto.sto.dwFlags & STOREFLAG_RECHARGE) //non-rechargable bag
            ) || (
                sto.sto.nType != STORETYPE_BAG &&
                sto.sto.dwFlags & STOREFLAG_RECHARGE //non-rechargable store
            )
        ) {
            stoitm.wUsage[nUsageIdx] = itm.GetNumUsage(nUsageIdx);
        } else {
            stoitm.wUsage[nUsageIdx] = itm.GetNumCharges(nUsageIdx);
        }
    }

    return;
}

void __stdcall CStore_GetBuyPrice_GetChargePercent(CStore& sto, short& nTotalUsages, short& nTotalCharges, CItem& itm, int nUsageIdx) {
    short nCharges = itm.GetNumCharges(nUsageIdx);
    ItmFileAbility* pAbility = NULL;
    
    if (nUsageIdx >= itm.nNumAbilities)
        return;

    if (itm.Demand()) {
        pAbility = itm.GetAbility(nUsageIdx);
        itm.Release();
    }

    if(pAbility != NULL) {
        if (nCharges > 0) {
            nTotalCharges += nCharges;
            nTotalUsages += (pAbility->chargeType == 3 || pAbility->flags & ITEMABILITYFLAG_RECHARGES) ? nCharges : itm.GetNumUsage(nUsageIdx);
        }
    }

}

void __stdcall CStore_GetSellPrice_GetChargePercent(CStore& sto, short& nTotalUsages, short& nTotalCharges, CItem& itm, int nUsageIdx) {
    short nCharges = itm.GetNumCharges(nUsageIdx);
    ItmFileAbility* pAbility = NULL;

    if (nUsageIdx >= itm.nNumAbilities)
        return;

    if (itm.Demand()) {
        pAbility = itm.GetAbility(nUsageIdx);
        itm.Release();
    }

    if(pAbility != NULL) {
        if (nCharges > 0) {
            nTotalCharges += nCharges;
            nTotalUsages += (pAbility->chargeType == 3 || pAbility->flags & ITEMABILITYFLAG_RECHARGES) ? nCharges : itm.GetNumUsage(nUsageIdx);
        }
    }

}

void __stdcall CStore_UnmarshalItem_SetUsages(CItem& itm, StoFileItem& stoitm, unsigned int nUsageIdx) {
    if (itm.GetMaximumStackSize() <= 1) {
        //non-stackable - use modified code
        itm.SetNumUsage(nUsageIdx, stoitm.wUsage[nUsageIdx] < 0 ? 0 : stoitm.wUsage[nUsageIdx]);
    } else {
        //stackable - use original code
        itm.SetNumUsage(nUsageIdx, stoitm.wUsage[nUsageIdx] < 1 ? 1 : stoitm.wUsage[nUsageIdx]);
    }
    return;
}


typedef struct _CNodeStoreItem
{
    struct _CNodeStoreItem*  pNext;
    struct _CNodeStoreItem*  pPrev;
    CScreenStoreItem*        data;
} CNodeStoreItem;


void static __stdcall
ScreenStoreRightPanelButtonClick_CheckShiftKey(CScreenStore& screen, int ItemIndex) {
    if ( ItemIndex >= 0 &&
         ItemIndex < screen.m_cplCustomerItems.GetCount() ) {
        CNodeStoreItem* Node = (CNodeStoreItem*) screen.m_cplCustomerItems.FindIndex(ItemIndex);
        CScreenStoreItem& StoreItem = * Node->data;
        if (StoreItem.bSelected) {      // selection mode
            if (screen.GetShiftKey()) {
                StoreItem.nSelectedCount = StoreItem.nMaxCount; // select all amount
                StoreItem.nTotalPrice = StoreItem.nSelectedCount * StoreItem.nSinglePrice;
            }
        }
    }
}


void static __stdcall
ScreenStoreLeftPanelButtonClick_CheckShiftKey(CScreenStore& screen, int ItemIndex) {
    if ( ItemIndex >= 0 &&
         ItemIndex < screen.m_cplHostItems.GetCount() ) {
        CNodeStoreItem* Node = (CNodeStoreItem*) screen.m_cplHostItems.FindIndex(ItemIndex);
        CScreenStoreItem& StoreItem = * Node->data;
        if (StoreItem.bSelected) {      // selection mode
            if (screen.GetShiftKey()) {
                StoreItem.nSelectedCount = StoreItem.nMaxCount; // select all amount
                StoreItem.nTotalPrice = StoreItem.nSelectedCount * StoreItem.nSinglePrice;
            }
        }
    }
}


int gStoreLeftPanelGoodCount;
int gStoreRightPanelGoodCount;

void static __stdcall
ScreenStoreLeftPanel_FetchItem(CScreenStore& screen,
                                int ItemLine,
                                int nTopHostItem,
                                CScreenStoreItem* StoreItem,
                                int* ItemIndex) {
    CScreenStoreItem StoreItemTmp;

    if (StoreItem == NULL)
        StoreItem = &StoreItemTmp;

    int skipcount = nTopHostItem + ItemLine;
    int count = screen.m_cplHostItems.GetCount();
    int index;

    if (ItemLine == 0) {    // first time
        gStoreLeftPanelGoodCount = 0;

        for (index = 0; index < count; index++) {
            screen.GetStoreItem(index, *StoreItem);
            if (StoreItem->bEnabled) {
                gStoreLeftPanelGoodCount++;
            }
        }
    }

    for (index = 0; index < count; index++) {
        screen.GetStoreItem(index, *StoreItem);
        if (StoreItem->bEnabled) {
            if (skipcount > 0) { 
                skipcount--;    // skip already found good & scroll shifted
                continue;       // find next good
            } else {
                break;          // found good
            }
        }
    }

    if (index == count) { // end of list, clear result
        StoreItem->pItem     = NULL;
        StoreItem->bSelected = FALSE;
        StoreItem->bEnabled  = FALSE;
        StoreItem->nSlot     = -1;
        StoreItem->nTotalPrice    = 0;
        StoreItem->nSinglePrice   = 0;
        StoreItem->nSelectedCount = 0;
        StoreItem->nMaxCount      = 0;
    }

    if (ItemIndex)
        *ItemIndex=index;   // patch index for LeftPanelButtonClick()
}


int static __stdcall
CScreenStore_OnBuyItemButtonClick_IncrementItemIndex(CScreenStore& screen,
                                CScreenStoreItem& StoreItem,
                                int ItemIndex) {
    if (StoreItem.bEnabled)
        return ItemIndex;
    else
        return (ItemIndex - 1); // skip hidden item
}


void __declspec(naked)
ScreenStoreRightPanelButtonClick_CheckShiftKey_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-34h]   // item index
    push    [ebp-4]     // screen

    call    ScreenStoreRightPanelButtonClick_CheckShiftKey

    pop     edx
    pop     ecx
    pop     eax

                        // Stolen bytes
    push    0x7A403A    // Call RecalcPrice()
    ret
}
}


void __declspec(naked)
ScreenStoreLeftPanelButtonClick_CheckShiftKey_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-34h]   // item index
    push    [ebp-4]     // screen

    call    ScreenStoreLeftPanelButtonClick_CheckShiftKey

    pop     edx
    pop     ecx
    pop     eax

                        // Stolen bytes
    push    0x7A40AE    // Call RecalcPrice()
    ret
}
}


void __declspec(naked)
ScreenStoreLeftPanel_FetchItem_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    0
    lea     ecx, [ebp-8Ch]
    push    ecx             // ScreenStoreItem
    push    [ebp-1D8h]      // nTopHostItem
    push    [ebp-104h]      // ItemLine
    push    [ebp-238h]      // screenstore

    call    ScreenStoreLeftPanel_FetchItem

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void __declspec(naked)
ScreenStoreLeftPanelButtonClick_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-34h]  // ItemIndex
    push    eax
    lea     ecx, [ebp-30h]
    push    ecx             // ScreenStoreItem
    push    [ebp-48h]       // nTopHostItem
    mov     eax, [ebp-34h]
    sub     eax, [ebp-48h]
    push    eax             // ItemLine = ItemIndex - nTopHostItem
    push    [ebp-4]         // screenstore

    call    ScreenStoreLeftPanel_FetchItem

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void __declspec(naked)
ScreenStoreLeftPanelDblButtonClick_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-40h]  // ItemIndex
    push    eax
    lea     ecx, [ebp-3Ch]
    push    ecx             // ScreenStoreItem
    push    [ebp-60h]       // nTopHostItem
    mov     eax, [ebp-40h]
    sub     eax, [ebp-60h]
    push    eax             // ItemLine = ItemIndex - nTopHostItem
    push    [ebp-10h]       // screenstore

    call    ScreenStoreLeftPanel_FetchItem

    pop     edx
    pop     ecx
    pop     eax

    mov     eax, [ebp-40h]      // stolen bytes
    mov     [edx+147Ah], eax

    ret
}
}


void __declspec(naked)
ScreenStoreLeftPanelRButtonClick_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [edx+ecx-5] // stolen bytes
    mov     [ebp-24h], eax

    lea     eax, [ebp-24h]  // ItemIndex
    push    eax
    push    0               // ScreenStoreItem
    push    [ebp-38h]       // nTopHostItem
    mov     eax, [ebp-24h]
    sub     eax, [ebp-38h]
    push    eax             // ItemLine = ItemIndex - nTopHostItem
    push    [ebp-10h]       // screenstore

    call    ScreenStoreLeftPanel_FetchItem

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void __declspec(naked)
ScreenStore_GetLeftListCountEAX_asm() {
__asm
{
    mov     eax, gStoreLeftPanelGoodCount

    ret
}
}


void __declspec(naked)
ScreenStore_GetLeftListCountEDX_asm() {
__asm
{
    mov     edx, gStoreLeftPanelGoodCount

    ret
}
}


void __declspec(naked)
ScreenStore_GetLeftListCountECX_asm() {
__asm
{
    mov     ecx, gStoreLeftPanelGoodCount

    ret
}
}


void __declspec(naked)
CScreenStore_OnBuyItemButtonClick_IncrementItemIndex_asm() {
__asm
{
    push    ecx
    push    edx

    push    ecx             // ItemIndex
    push    [ebp-134h]      // ScreenStoreItem
    push    [ebp-2DCh]      // screenstore

    call    CScreenStore_OnBuyItemButtonClick_IncrementItemIndex

    pop     edx
    pop     ecx

    mov     ecx, eax
    mov     [ebp-148h], ecx // Stolen bytes

    ret
}
}
