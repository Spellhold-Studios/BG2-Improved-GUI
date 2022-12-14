#include "ContainerPanel.h"
#include "chitin.h"
#include "detoursext.h"
#include "uibutton.h" 


#define GAMEOBJ     g_pChitin->pGame->m_GameObjectArrayHandler

typedef unsigned __int64    QWORD;

void (ContainerPanelButton::*Tramp_ContainerPanelButton_OnLClicked)(POINT) =
    SetFP(static_cast<void (ContainerPanelButton::*)(POINT)> (&ContainerPanelButton::OnLClicked),       0x7E582E);

BOOL (ContainerPanelButton::*Tramp_ContainerPanelButton_Redraw)(BOOL) =
    SetFP(static_cast<BOOL (ContainerPanelButton::*)(BOOL)>  (&ContainerPanelButton::Redraw),           0x7E6540);



void ContainerPanelButton::OnLClicked(POINT pt) {
  (this->*Tramp_ContainerPanelButton_OnLClicked)(pt);
}

BOOL ContainerPanelButton::Redraw(BOOL bForceRedraw) {
    return (this->*Tramp_ContainerPanelButton_Redraw)(bForceRedraw);
}



char const ButtonCreateOpcodes[] = {
    // Patched 0x978399 table
    // all codes from GUIW.chu, Panel_ID=8

    // 0 CUIButton
    // 1 CUIPicture
    // 2 CUIButtonMicro
    // 3 CUIScrollBar
    // 4 CUIPicture2
    // 5 CUIButtonHideLeft
    // 6 CUIButtonHideRight
    // 7 CUIClockButton
    // 8 CUIErrorButton
    0,     0,     0,     0, //  0,  1,  2,  3
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     0,     0,
    0,     0,     1,     2, // xx, xx, 50, 51
    3,     3,     4,     8, // 52, 53, 54, xx
    8,     8,     8,     8, // xx  xx  xx  xx
    5,     6,     7         // 60, 61, 62, 63
};


// global vars
static ContainerArray_Head      gContainerArray;
static ItemArray_Head           gItemArray;
static ItemGroupArray_Head      gItemGroupArray;

static unsigned int             gPageIndex;
static unsigned int             gTotalItems;
static bool                     gGroupMode;
static Enum                     gContainerWaitForPurge_ObjID;
static CGameContainer*          g_pContainerWaitForPurgeContainer;
static CMessage*                g_pPurgeMessageType1;
static CMessage*                g_pPurgeMessageType2;
static CRITICAL_SECTION         gCriticalSection;

static unsigned short           gLAST_BUTTONID;
static unsigned short           gBUTTONS_PER_SCREEN;

static unsigned int             gGAMEFIELD_Y_COOR;      // redirected from 0xB84C9C
const char*         const       g_pButtonCreateOpcodes   = ButtonCreateOpcodes;
const unsigned int* const       g_pPointerYCoor          = &gGAMEFIELD_Y_COOR;

static int                      gSort_Buf_MaxIndex;
static char                     gIniSort_Buf [255];
static unsigned short           gSort_Buf    [255];
static bool                     gSortMode_Normal;
static bool                     gSortMode_UnknowFirst;


void
ContainerPanel_ResetStaticVars()
{
    gContainerArray.Head    = NULL;
    gContainerArray.Tail    = NULL;
    gItemArray.Head         = NULL;
    gItemArray.Tail         = NULL;
    gItemGroupArray.Head    = NULL;
    gItemGroupArray.Tail    = NULL;
    g_pContainerWaitForPurgeContainer = NULL;
    gContainerWaitForPurge_ObjID = 0;
    gPageIndex              = 0;
    gTotalItems             = 0;
    g_pPurgeMessageType1      = NULL;
    g_pPurgeMessageType2      = NULL;
}


void __forceinline
ResNameCopy(char* const dst, const char* const src) {
    *(QWORD *)dst = *(QWORD *)src;
}


void
ResNameCopyUpper(char* const dst, const char* const src) {
    *(QWORD *)dst = *(QWORD *)src;

    for (int i = 0; i < 8; i++) {
        if (dst[i] > '`' && dst[i] < '{')   // ranges from orig ResRef::ToUpper
            dst[i] -= 0x20;                 // To UpCase
    }
}


void __forceinline
ResNameZero(char* const dst) {
    *(QWORD *)dst=0;
}


bool __forceinline
ResNameCmp(const char* const dst, const char* const src) {
    if (*(QWORD *)dst == *(QWORD *)src)
        return true;
    else
        return false;
}


#ifdef _DEBUG
void
ContainerPanel_LogGAMEContainerItems(CGameContainer* const pContainer)
{
    int         ContainerItemIndex = 0;
    POSITION    pos;
    CItem*      pItem;

    pos = pContainer->m_items.GetHeadPosition();
    while (pos != NULL) {
        pItem = (CItem *) pContainer->m_items.GetNext(pos);
        if (pItem) {
            console.writef("GAME Item %s ContainerID=%0x ContainerItemIndex=%d Count=%d \n",
                        pItem->itm.name.GetResRefNulled(),
                        pContainer->id,
                        ContainerItemIndex,
                        pItem->wUsage1);

            ContainerItemIndex++;
        } else {
            console.writef("GAME Deleted Itm ContainerID=%0x ContainerItemIndex=%d \n",
                        pContainer->id,
                        ContainerItemIndex);

            ContainerItemIndex++;
        }

    }
}


void
ContainerPanel_LogGroupItems()
{
    ItemGroupArray_Node *pNext,         *pCur;
    ItemArray_Node      *pChildNext,    *pChildCur;
    int Iter    = 0;
    int IterChild;

    pNext= pCur= gItemGroupArray.Head;
    while (pCur) {
        pNext = pCur->Head;
        
        // payload
        console.writef("ItemGroupNode_%0d Name=%s ContainerID=%0x ContainerItemIndex=%d Count=%d Deleted=%d Child=%0x \n",
                        Iter,
                        pCur->Item->itm.name.GetResRefNulled(),
                        (pCur->ContainerNode) ? pCur->ContainerNode->ContainerObjID : 0,
                        pCur->ContainerItemIndex,
                        pCur->Count,
                        pCur->Deleted,
                        pCur->ChildHead);

        pChildNext= pChildCur= pCur->ChildHead;
        IterChild = 0;
        while (pChildCur) {
            pChildNext = pChildCur->Head;

            // payload
            console.writef("ChildItem_%0d Name=%s ContainerID=%0x ContainerItemIndex=%d Count=%d Deleted=%d Pointer=%0x NextChild=%0x \n",
                        IterChild,
                        pChildCur->Item->itm.name.GetResRefNulled(),
                        pChildCur->ContainerNode->ContainerObjID,
                        pChildCur->ContainerItemIndex,
                        pChildCur->Count,
                        pChildCur->Deleted,
                        pChildCur,
                        pChildCur->Head);

            IterChild++;
            //

            pChildCur = pChildNext;
        }

        Iter++;
        //

        pCur = pNext;
    }
    console.writef("gTotalItems= %d  \n", gTotalItems);
}
#endif


void
ContainerPanel_ContainerArrayRemoveAll() 
{
    ContainerArray_Node *pPrev, *pCur;

    #ifdef _DEBUG
        console.writef("ContainerArrayRemoveAll() \n");
    #endif

    pPrev= pCur= gContainerArray.Tail; // end to start
    while (pCur) {
        pPrev = pCur->Tail;

        // payload
        IEFree(pCur);
        //

        pCur = pPrev;
    }

    gContainerArray.Head = NULL;
    gContainerArray.Tail = NULL;
        
}


void
ContainerPanel_ItemGroupArrayRemoveAll() 
{
    ItemGroupArray_Node *pPrev,      *pCur;
    ItemArray_Node      *pChildPrev, *pChildCur;

    #ifdef _DEBUG
        console.writef("ItemGroupArrayRemoveAll() \n");
    #endif

    pPrev= pCur= gItemGroupArray.Tail; // end to start
    while (pCur) {
        pPrev = pCur->Tail;
        
        pChildPrev= pChildCur= pCur->ChildTail;
        while (pChildCur) {
            pChildPrev = pChildCur->Tail;

            // payload
            IEFree(pChildCur);
            //

            pChildCur = pChildPrev;
        }

        // payload
        IEFree(pCur);
        //

        pCur = pPrev;
    }

    gItemGroupArray.Head = NULL;
    gItemGroupArray.Tail = NULL;
    gTotalItems = 0;
        
}



void
ContainerPanel_AddContainerNode(CGameContainer* const pContainer, const Enum ObjID) 
{
    ContainerArray_Node* pNewNode;

    pNewNode= IENew ContainerArray_Node;

    pNewNode->Container         = pContainer;
    pNewNode->ContainerObjID    = ObjID;
    pNewNode->Head              = NULL;
    pNewNode->Tail              = gContainerArray.Tail;

    if (gContainerArray.Head == NULL) {
        gContainerArray.Head = pNewNode;
    }

    if (gContainerArray.Tail != NULL) {
        gContainerArray.Tail->Head = pNewNode;
    }
    
    gContainerArray.Tail = pNewNode;
}


ItemGroupArray_Node*
ContainerPanel_GetExistStackableGroupItem(const CItem* const pItem, unsigned int const dwFlags) {
    ItemGroupArray_Node *pPrev, *pCur;

    pPrev= pCur= gItemGroupArray.Head; // end to start
    while (pCur) {
        pPrev = pCur->Head;
        
        // match by resref name and flags(identified/stolen/...)
        if (ResNameCmp(pCur->Name, (const char *) &pItem->itm.name) && (pCur->dwFlags == dwFlags)) {
            return pCur;
        }
        //

        pCur = pPrev;
    }

    return NULL;
}


void
ContainerPanel_AddChildItemToGroup(
    ItemGroupArray_Node* const pItemGroupNode,
    CItem*               const pItem,
    ContainerArray_Node* const pContainerNode,
    short                const ContainerItemIndex,
    short                const StackSize)
{
    ItemArray_Node  *pNewItemNode;

    pNewItemNode= IENew ItemArray_Node;
    //////////////////////////////////
    // fields
    pNewItemNode->Container             = pContainerNode->Container;
    pNewItemNode->ContainerNode         = pContainerNode;
    pNewItemNode->Item                  = pItem;
    pNewItemNode->ContainerItemIndex    = ContainerItemIndex;
    pNewItemNode->dwFlags               = pItem->dwFlags;
    pNewItemNode->Deleted               = false;

    pItem->itm.pRes->Demand();
        {
            pNewItemNode->ItemType = pItem->itm.pRes->pFile->m_wItemType;
        }
    pItem->itm.pRes->Release();
    //////////////////////////////////

    if (StackSize > 1) {
        pNewItemNode->Count = pItem->GetNumUsage(0);
        pNewItemNode->Stackable = true;
    } else {
        pNewItemNode->Count = 0;
        pNewItemNode->Stackable = false;
    }

    pNewItemNode->Head = NULL;
    pNewItemNode->Tail = pItemGroupNode->ChildTail;

    if (pItemGroupNode->ChildHead == NULL) {
        pItemGroupNode->ChildHead = pNewItemNode;
    }

    if (pItemGroupNode->ChildTail != NULL){
        pItemGroupNode->ChildTail->Head = pNewItemNode;
    }
    
    pItemGroupNode->ChildTail = pNewItemNode;
        
    pItemGroupNode->Count += pNewItemNode->Count;
}



void
ContainerPanel_AddItemGroupNode(
    CItem*               const pItem,
    ContainerArray_Node* const pContainerNode,
    short                const ContainerItemIndex)
{
    short                StackSize;
    bool                 Stackable;
    ItemGroupArray_Node* pNewItemGroupNode;
    ItemGroupArray_Node* pExistItemGroupNode;

    StackSize = pItem->GetMaximumStackSize();
    Stackable = (StackSize > 1) ? true : false;

    if (Stackable && gGroupMode) { 
        pExistItemGroupNode = ContainerPanel_GetExistStackableGroupItem(pItem, pItem->dwFlags);  // may have child
    } else {
        pExistItemGroupNode = NULL;                                                              // single item
    }

    if (pExistItemGroupNode) {
        ContainerPanel_AddChildItemToGroup(pExistItemGroupNode, pItem, pContainerNode, ContainerItemIndex, StackSize);
    }
    else {  // new itemgroup
        gTotalItems++;

        pNewItemGroupNode= IENew ItemGroupArray_Node;
        //////////////////////////////////
        // fields
        pNewItemGroupNode->Container            = pContainerNode->Container;
        pNewItemGroupNode->ContainerNode        = pContainerNode;
        pNewItemGroupNode->Item                 = pItem;
        pNewItemGroupNode->ContainerItemIndex   = ContainerItemIndex;
        pNewItemGroupNode->dwFlags              = pItem->dwFlags;
        pNewItemGroupNode->ChildHead            = NULL;
        pNewItemGroupNode->ChildTail            = NULL;
        pNewItemGroupNode->HasChild             = false;
        pNewItemGroupNode->Deleted              = false;
        ResNameCopy(pNewItemGroupNode->Name,  pItem->itm.name.GetResRef());

        pItem->itm.pRes->Demand();
        {
            pNewItemGroupNode->ItemType = pItem->itm.pRes->pFile->m_wItemType;
        }
        pItem->itm.pRes->Release();
        //////////////////////////////////

        if (Stackable && gGroupMode) {
            pNewItemGroupNode->Count     =  0;      // count will be increased by each child
            pNewItemGroupNode->Stackable = true;
        }
        if (Stackable && (gGroupMode == false)) {
            pNewItemGroupNode->Count     = pItem->GetNumUsage(0);
            pNewItemGroupNode->Stackable = true;
        }
        if (Stackable == false) {
            pNewItemGroupNode->Count     =  0;
            pNewItemGroupNode->Stackable = false;
        }

        pNewItemGroupNode->Head = NULL;
        pNewItemGroupNode->Tail = gItemGroupArray.Tail;

        if (gItemGroupArray.Head == NULL) {
            gItemGroupArray.Head = pNewItemGroupNode;
        }

        if (gItemGroupArray.Tail != NULL) {
            gItemGroupArray.Tail->Head = pNewItemGroupNode;
        }
    
        gItemGroupArray.Tail = pNewItemGroupNode;

        // add first child for stackable items
        if (Stackable && gGroupMode) {
            pNewItemGroupNode->HasChild             = true;
            pNewItemGroupNode->ContainerItemIndex   = -1;
            pNewItemGroupNode->Container            = NULL;
            pNewItemGroupNode->ContainerNode        = NULL;

            ContainerPanel_AddChildItemToGroup(pNewItemGroupNode, pItem, pContainerNode, ContainerItemIndex, StackSize);
        }

    } // new itemgroup

}


void
ContainerPanel_MoveItemGroupNodeToHead(ItemGroupArray_Node* const pItemGroupNode)
{
    ItemGroupArray_Node *Next, *Prev, *OldHead;
    bool    tohead;

    Next = pItemGroupNode->Head;
    Prev = pItemGroupNode->Tail;

    for (;;) {
        if (Next && Prev) {             // have prev & next
            Prev->Head = Next;
            Next->Tail = Prev;
            tohead = true;
            break;
        }

        if (!Next && !Prev) {           // was only one in list
            tohead = false;
            break;
        }

        if (Next && !Prev) {            // was first
            tohead = false;
            break;
        }

        if (!Next && Prev) {            // was last
            gItemGroupArray.Tail = Prev;
            Prev->Head = NULL;
            tohead = true;
            break;
        }
    }

    if (tohead) {
        OldHead = gItemGroupArray.Head;

        gItemGroupArray.Head = pItemGroupNode;
        pItemGroupNode->Tail = NULL;
        pItemGroupNode->Head = OldHead;
        OldHead->Tail = pItemGroupNode;
    }

}


void
ContainerPanel_SortItemGroupArray(int const Mode)
{
    ItemGroupArray_Node *pNext, *pCur;
    int                 i;
    unsigned short      SortType;

    if (Mode == SORT_NORMAL) {
        for (i = gSort_Buf_MaxIndex; i >=0; i--) { // end->start = low priority->high
            SortType = gSort_Buf[i];
            pNext= pCur= gItemGroupArray.Head;
            while (pCur) {
                pNext = pCur->Head;

                // payload
                if (pCur->ItemType == SortType) {
                        ContainerPanel_MoveItemGroupNodeToHead(pCur);
                }
                //
                pCur = pNext;
            }
        }
    }

    if (Mode == SORT_UNKNOWFIRST) {
        pNext= pCur= gItemGroupArray.Head;
        while (pCur) {
            pNext = pCur->Head;

            // payload
            if (!(pCur->dwFlags & CREITEM_IDENTIFIED)) {
                    ContainerPanel_MoveItemGroupNodeToHead(pCur);
            }
            //
            pCur = pNext;
        }
    }

}


void
ContainerPanel_CreateItemGroupArray() 
{
    ContainerArray_Node *pNext,*pCur;
    CGameContainer*      pContainer;
    POSITION             pos;
    CItem*               pItem;
    short                ContainerItemIndex;

    #ifdef _DEBUG
        console.writef("CreateItemGroupArray() \n");
    #endif

    ContainerPanel_ItemGroupArrayRemoveAll();

    pNext= pCur= gContainerArray.Head;
    while (pCur) {
        pNext = pCur->Head;
        {   // Next Container
            pContainer = pCur->Container;
            ContainerItemIndex = 0;
            pos = pContainer->m_items.GetHeadPosition();
            while (pos != NULL) {
                pItem = (CItem *) pContainer->m_items.GetNext(pos);
                if (pItem) {
                    ContainerPanel_AddItemGroupNode(pItem, pCur, ContainerItemIndex);
                    ContainerItemIndex++;
                }
            }

        }
        pCur = pNext;
    }

    if (gSortMode_Normal) {
        ContainerPanel_SortItemGroupArray(SORT_NORMAL);
    }
    if (gSortMode_UnknowFirst) {
        ContainerPanel_SortItemGroupArray(SORT_UNKNOWFIRST);
    }

    #ifdef _DEBUG
        ContainerPanel_LogGroupItems();
    #endif
}


ItemGroupArray_Node*
ContainerPanel_GetItemGroupNodeByIndex(unsigned int const RequestedIndex)
{
    ItemGroupArray_Node *pNext, *pCur;
    int Iter = 0;

    pNext= pCur= gItemGroupArray.Head;
    while (pCur) {
        pNext = pCur->Head;
        
        // payload
        if (Iter == RequestedIndex) {
            return pCur;
        }

        Iter++;
        //

        pCur = pNext;
    }
    return NULL;
}


void
ContainerPanel_GetItemChildNodeByIndex(
        unsigned int          const RequestedIndex,
        ItemGroupArray_Node** const ppItemGroupNode,
        ItemArray_Node**      const ppItemChildNode)
{
    ItemGroupArray_Node *pNext, *pCur;
    ItemArray_Node      *pChildNext, *pChildCur;
    unsigned int Iter = 0;

    pNext= pCur= gItemGroupArray.Head;
    while (pCur) {
        pNext = pCur->Head;
        // if itemgroup is single item
        if ((Iter == RequestedIndex) && (pCur->HasChild == false)) {
            *ppItemGroupNode = pCur;
            *ppItemChildNode = NULL;
            return;
        }

        if (pCur->HasChild == true) {
            pChildNext= pChildCur= pCur->ChildHead;
            while (pChildCur) {
                pChildNext = pChildCur->Head;
                // if child
                if (Iter == RequestedIndex) {
                    *ppItemGroupNode = NULL;
                    *ppItemChildNode = pChildCur;
                    return;
                }
                Iter++;
                //
                pChildCur = pChildNext;
            }
        }
        else {
            Iter++;
        }
        //
        pCur = pNext;
    }

    // empty list
    *ppItemGroupNode = NULL;
    *ppItemChildNode = NULL;
}


void 
ContainerPanel_ShiftContainerItemIndexes(ContainerArray_Node* const pContainerNode, short const DeletedIndex)
{
    ItemGroupArray_Node *pNext, *pCur;
    ItemArray_Node      *pChildNext, *pChildCur;

    pNext= pCur= gItemGroupArray.Head;
    while (pCur) {
        pNext = pCur->Head;
        
        // payload
        // single item
        if ((pCur->HasChild == false) &&
            (pCur->ContainerNode == pContainerNode) &&
            (pCur->Deleted  == false))
        {
            if (pCur->ContainerItemIndex >= DeletedIndex) {
                pCur->ContainerItemIndex--;                                 // shift
            }
        }

        // group
        if (pCur->HasChild)
        {
            pChildNext= pChildCur= pCur->ChildHead;
            while (pChildCur) {
                pChildNext = pChildCur->Head;

                // payload
                if ((pChildCur->ContainerNode == pContainerNode) &&
                    (pChildCur->Deleted  == false))
                {
                    if (pChildCur->ContainerItemIndex >= DeletedIndex) {
                        pChildCur->ContainerItemIndex--;                    // shift
                    }
                }
                //
                pChildCur = pChildNext;
            }
        }
        //

        pCur = pNext;
    }

}



void
ContainerPanel_ConfigButtons()
{
    // get logical X/Y from game
    const short X                = *(( WORD *) (0xB6150C)) / (1 + g_pChitin->bDoubleResolution); // XRes
    const short Y                = *(( WORD *) (0xB6150E)) / (1 + g_pChitin->bDoubleResolution); // YRes
    const int   ContainerPanel_Y = *((DWORD *) (0xB84C9C)); // gamefield Y-Coor while container panel opened

    /*
    widescreenNOPs   = *( (short *) (0x432793));  // widescreen 3.07 detect
    if (widescreenNOPs== 0x9090)
        widescreen=true;
    else
        widescreen=false;
    */

    gGAMEFIELD_Y_COOR   = ContainerPanel_Y - ((135 - 90) * (1 + g_pChitin->bDoubleResolution));

    if (X < 800) {
        gLAST_BUTTONID      = LAST_BUTTONID_640;
        gBUTTONS_PER_SCREEN = (gLAST_BUTTONID - FIRST_BUTTONID + 1);
        return;
    }

    if (X < 1024) {
        gLAST_BUTTONID      = LAST_BUTTONID_800;
        gBUTTONS_PER_SCREEN = (gLAST_BUTTONID - FIRST_BUTTONID + 1);
        return;
    }

    // X >=1024
    if (pGameOptionsEx->bUI_LootPanelGUI_720Hack) {
        gLAST_BUTTONID      = LAST_BUTTONID_800;
        gBUTTONS_PER_SCREEN = (gLAST_BUTTONID - FIRST_BUTTONID + 1);
        return;
    } else {
        gLAST_BUTTONID      = LAST_BUTTONID_1024;
        gBUTTONS_PER_SCREEN = (gLAST_BUTTONID - FIRST_BUTTONID + 1);
        return;
    }

}


void __stdcall
ContainerPanel_CreateArrayOfContainersAround(CCreatureObject& Cre)
{
    int                 cre_x,cre_y;
    CGameContainer*     pContainer;
    CArea*              pArea;
    POSITION            pos;
    Enum                ObjID, OpenedContObjID;

    #ifdef _DEBUG
        console.writef("GetArrayContainersAround() \n");
    #endif

    ContainerPanel_ConfigButtons(); // check resolutions after game configured it

    gPageIndex = 0;
    ContainerPanel_ContainerArrayRemoveAll();

    cre_x = Cre.currentLoc.x;
    cre_y = Cre.currentLoc.y;
    OpenedContObjID =  *( (Enum *) ((unsigned char *)&Cre + 0x2FC) );  // 0x2fc cre.OpenedContainer_ObjID
    pArea = Cre.pArea;

    pos = pArea->m_lVertSortBack.GetHeadPosition();   //TODO: m_lVertSortFront+m_lVertSortBack ?
    while (pos != NULL) {
        ObjID = (Enum) pArea->m_lVertSortBack.GetNext(pos);
        if (GAMEOBJ.GetGameObjectShare(ObjID, THREAD_ASYNCH, &pContainer, -1) != OBJECT_SUCCESS)
            {continue;}

        if (pContainer->nObjType != CGAMEOBJECT_TYPE_CONTAINER)
            {GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
             continue;}

        if (pContainer->wContainerType != 4) // Pile on Ground = 4
            {GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
             continue;}

        if (pContainer->m_items.IsEmpty())   // skip empty piles
            {GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
             continue;}

        if (ObjID == OpenedContObjID )      // Container_ObjID=cre.OpenedContainer_ObjID
            {GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
             continue;}                     // skip items from clicked container    


        /*
        // CIRCLE RANGE
        abs(cont1.x-cont2.x)=side1
        abs(cont1.y-cont2.y)=side2
        range=sqrt(side1^2 + side2^2);
        */

        // SQUARE RANGE
        if (
            abs(pContainer->currentLoc.x - cre_x) > RANGE_CONTAINERS_AROUND || 
            abs(pContainer->currentLoc.y - cre_y) > RANGE_CONTAINERS_AROUND
           )
        {
           GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
           continue;
        }

        ContainerPanel_AddContainerNode(pContainer, ObjID);

        #ifdef _DEBUG
            console.writef("Container added, id=0x%x itemcount=%d x=%d y=%d \n",
                            ObjID,
                            pContainer->m_items.GetCount(),
                            pContainer->currentLoc.x,
                            pContainer->currentLoc.y);
        #endif

        GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
    }

    ContainerPanel_CreateItemGroupArray();
    
}


void
ContainerPanel_StopContainer()
{
    #ifdef _DEBUG
        console.writef("Container closed \n");
    #endif

    ContainerPanel_ItemGroupArrayRemoveAll();
    ContainerPanel_ContainerArrayRemoveAll();

    ContainerPanel_ResetStaticVars();
}


void __stdcall
ContainerPanel_ButtonRedraw(
    short   const ButtonID,
    // out
    CItem** const rItem,
    int*    const rTextRefID,
    ResRef* const rIcon,
    ResRef* const rName,
    short*  const rCount,
    //
    CUIButton& Button)
{
    unsigned int         ButtonIndex, ItemIndex;
    ItemGroupArray_Node* pItemGroupNode;
    CItem*               pItem;
    ItmFileHeader*       pITMFile;


    ButtonIndex = ButtonID - FIRST_BUTTONID;
    ItemIndex = (gPageIndex * gBUTTONS_PER_SCREEN) + ButtonIndex;

    pItemGroupNode = ContainerPanel_GetItemGroupNodeByIndex(ItemIndex);
    if (pItemGroupNode) {
        pItem = pItemGroupNode->Item;

        // rItem
        *rItem = pItem;

        pItem->itm.pRes->Demand();
        {
            pITMFile = pItem->itm.pRes->pFile;

            // rTextRefID
            if (pItem->dwFlags & CREITEM_IDENTIFIED) {
                *rTextRefID =  pITMFile->m_strrefIdentifiedName;        // 0Ch Identified Name Reference ID
            } else {
                *rTextRefID =  pITMFile->m_strrefGenericName;           // 08h General Name Reference ID
            }
            
            // rIcon
            ResNameCopyUpper((char *) rIcon, (char *) &pITMFile->m_rItemIcon);  // 3Ah Icon ResRef
        }
        pItem->itm.pRes->Release();
        
        // rName
        ResNameCopy((char *) rName, (char *) &pItem->itm.name);
        
        // rCount
        *rCount = pItemGroupNode->Count;
        
    }
    else {
        // empty item
        *rItem      = NULL;
        *rTextRefID = -1;
        ResNameZero((char *) rIcon);
        ResNameZero((char *) rName);
        *rCount     = 0;
    }
}


BOOL
DETOUR_ContainerPanelButton::DETOUR_Redraw(BOOL const bForceRedraw)
{
    short        ButtonID;
    CUIButton&   Button = *this;
    unsigned int MaxPageIndex;

    ButtonID = Button.index;

    if (gTotalItems > 1) {
        MaxPageIndex = (gTotalItems-1) / gBUTTONS_PER_SCREEN;
        if (gPageIndex > MaxPageIndex) {
            gPageIndex = MaxPageIndex;

            #ifdef _DEBUG
                console.writef("PageIndex adjusted to %d \n", gPageIndex);
            #endif
        }
    } else {
        MaxPageIndex = 0;
        gPageIndex  = 0;
    }

    if (ButtonID == LEFT_PAGE_BUTTONID) {       // left page button
        #ifdef _DEBUG
            console.writef("LEFT Page Button redraw \n");
        #endif

        if  (gPageIndex == 0) {             // disable button for first page
            #ifdef _DEBUG
                console.writef("LEFT Page Button disabled \n");
            #endif

            Button.SetEnabled(false);
        } else {
            #ifdef _DEBUG
                console.writef("LEFT Page Button enabled \n");
            #endif

            Button.SetEnabled(true);
        }

        return CUIButton::Redraw(bForceRedraw); // this really need ?
    }

    if (ButtonID == RIGHT_PAGE_BUTTONID) {      // right page button
        #ifdef _DEBUG
            console.writef("RIGHT Page Button redraw \n");
        #endif

        if (gPageIndex == MaxPageIndex) {       // disable button for last page
            #ifdef _DEBUG
                console.writef("RIGHT Page Button disabled \n");
            #endif

            Button.SetEnabled(false);
        } else {
            #ifdef _DEBUG
                console.writef("RIGHT Page Button enabled \n");
            #endif

            Button.SetEnabled(true);
        }

        return CUIButton::Redraw(bForceRedraw); // this really need ?
    }

    // any other button
    return ContainerPanelButton::Redraw(bForceRedraw);

}


void
DETOUR_ContainerPanelButton::ScrollClick(short const ButtonID)
{
    unsigned int MaxPageIndex;
    int          needredraw = 0;
    //CUIControl* pControl;

    if (gTotalItems > 1) {
        MaxPageIndex = (gTotalItems-1) / gBUTTONS_PER_SCREEN;
    } else {
        MaxPageIndex = 0;
    }

    if (ButtonID == LEFT_PAGE_BUTTONID) {
        if (gPageIndex > 0) {
                needredraw=1;
                gPageIndex--;
        }
    }

    if (ButtonID == RIGHT_PAGE_BUTTONID) {
        if (gPageIndex < MaxPageIndex) {
                needredraw=1;
                gPageIndex++;
        }
    }

    if (needredraw == 1) {
        #ifdef _DEBUG
            console.writef("New PageIndex= %d,  PanelRedraw() \n", gPageIndex);
        #endif

        /*
        for (i=FIRST_BUTTONID; i<=gLAST_BUTTONID; i++) {
            CUIControl& Control = this->pPanel->GetUIControl(i);
            Control.SetRedraw();
        }

        Control = & this->pPanel->GetUIControl(LEFT_PAGE_BUTTONID);
        Control->SetRedraw();
        Control->Redraw(true);

        Control = & this->pPanel->GetUIControl(RIGHT_PAGE_BUTTONID);
        Control->SetRedraw();
        Control->Redraw(true);
        */
        
        this->pPanel->SetRedraw(NULL); // force redraw

    }
}


bool
InventoryIsFull(
        Enum         const CreObjID,
        CItem*       const pItem,
        unsigned int const dwFlags,
        bool         const Stackable,
        short        const Count)
{
#define NOTFULL false
#define FULL true

    CCreatureObject* pCre;

    if (ResNameCmp((char *) &pItem->itm.name, "MISC07\0\0"))
        return NOTFULL;                             // bypass gold

    if (GAMEOBJ.GetGameObjectShare(CreObjID, THREAD_ASYNCH, &pCre, -1) != OBJECT_SUCCESS)
        return FULL;                                // error accessing creature

    for (int i=INVENTORYSLOT_FIRST; i <= INVENTORYSLOT_LAST; i++)
    {
        if (pCre->Inventory.items[i] == NULL) {
            GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
            return NOTFULL;                         // at least one free slot
        }
    }

    // find existing slot with same stackable item
    /* disabled because game engine always require one free inventory slot even for stackable items

    if (Stackable == false) {
        GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
        return FULL;                                // not stackable item
    }

    StackSize = Item->GetMaximumStackSize();

    for (int i=INVENTORYSLOT_FIRST; i <= INVENTORYSLOT_LAST; i++)
    {
        if (ResNameCmp(
                (char *) &Cre->m_Inventory.items[i]->itm.name,
                (char *) &Item->itm.name)                             &&
            (Cre->m_Inventory.items[i]->dwFlags == Item->dwFlags) ) {
            // found some slot with same item & flags, then calculate enough free stacksize

            InvCount = Cre->m_Inventory.items[i]->m_wUsage1;
            if ((InvCount + Count) <= StackSize) {
                GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
                return NOTFULL;
            }
        }

    }
    */

    GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
    return FULL;
}


void
ContainerPanel_ForceProcessMessages(CGameContainer* const pContainer, Enum const ContainerObjID)
{
    CMessage* pMsg;
    POSITION  pos, current_pos;

    gContainerWaitForPurge_ObjID      = ContainerObjID;
    g_pContainerWaitForPurgeContainer = pContainer;

    EnterCriticalSection(&gCriticalSection);
    pos = g_pChitin->messages.GetHeadPosition();
    while (pos != NULL) {
        current_pos = pos;
        pMsg = (CMessage *) g_pChitin->messages.GetNext(pos);
        if ( (pMsg == g_pPurgeMessageType1)   ||
             (pMsg == g_pPurgeMessageType2) )
        {
            g_pChitin->messages.RemoveAt(current_pos);

            if (pMsg == g_pPurgeMessageType1)    {g_pPurgeMessageType1   = NULL;}
            if (pMsg == g_pPurgeMessageType2)    {g_pPurgeMessageType2   = NULL;}

            pMsg->DoMessage();

            delete pMsg;    // 0x5B0F41
        }

    }
    LeaveCriticalSection(&gCriticalSection);

}


void 
DETOUR_ContainerPanelButton::DETOUR_OnLClicked(POINT const pt)
{
    unsigned int            ButtonIndex, ItemIndex;
    ItemGroupArray_Node*    pItemGroupNode;
    ItemArray_Node          *pChildNext, *pChildCur;

    int*   const            pGAMEContainerObjID = &g_pChitin->pGame->u1e32;
    Enum*  const            pGAMECreatureObjID =  &g_pChitin->pGame->u1e36;
    short* const            pGAMEContainerItemIndex = (short* const) &(this->index);
    bool                    some_child_was_removed = false;

    short const ButtonID                = *pGAMEContainerItemIndex;
    int   const OrigGameContainerObjID  = *pGAMEContainerObjID;
    short const OrigButtonIndex         = ButtonID;

    // Item Buttons
    if ((ButtonID >= FIRST_BUTTONID) &&
        (ButtonID <= gLAST_BUTTONID))
    {
        ButtonIndex = ButtonID - FIRST_BUTTONID;
        ItemIndex = (gPageIndex * gBUTTONS_PER_SCREEN) + ButtonIndex;

        pItemGroupNode = ContainerPanel_GetItemGroupNodeByIndex(ItemIndex);
        if (pItemGroupNode)
        {   // button has item

            if (pItemGroupNode->HasChild)
            {   // items from many containers on same button

                pChildNext= pChildCur= pItemGroupNode->ChildHead;
                while (pChildCur) {
                    pChildNext = pChildCur->Head;

                    // payload
                    if (InventoryIsFull(
                           *pGAMECreatureObjID,
                            pChildCur->Item,
                            pChildCur->dwFlags,
                            pChildCur->Stackable,
                            pChildCur->Count)) {

                        #ifdef _DEBUG
                            console.writef("Inventory Full on child \n");
                        #endif

                        if (some_child_was_removed) {
                            #ifdef _DEBUG
                                console.writef("Inventory Full on child after moving \n");
                            #endif

                            // part of childs was moved to inventory, but no more free slots
                            // reload all items
                            // maybe slow, but fully synced with game
                            ContainerPanel_CreateItemGroupArray();  
                        }
                        return;
                    }

                    #ifdef _DEBUG
                        console.writef("Before: \n");
                        ContainerPanel_LogGAMEContainerItems(pChildCur->Container);
                        //ContainerPanel_LogGroupItems();
                    #endif

                    // override ButtonIndex and ContainerObjID
                    *pGAMEContainerItemIndex    = pChildCur->ContainerItemIndex | 0x8000; // 0x8000 = bypass ContainerItemIndex recalc
                    *pGAMEContainerObjID        = pChildCur->ContainerNode->ContainerObjID;
                    /////////////////////////////////////////////
                    ContainerPanelButton::OnLClicked(pt);
                    /////////////////////////////////////////////
                    pChildCur->Deleted       = true;
                    pItemGroupNode->Count   -= pChildCur->Count;
                    some_child_was_removed   = true;
                    // restore IDs
                    *pGAMEContainerItemIndex    = OrigButtonIndex;
                    *pGAMEContainerObjID        = OrigGameContainerObjID;

                    ContainerPanel_ForceProcessMessages(pChildCur->Container, pChildCur->ContainerNode->ContainerObjID);
                    ContainerPanel_ShiftContainerItemIndexes(pChildCur->ContainerNode, pChildCur->ContainerItemIndex);

                    #ifdef _DEBUG
                        console.writef("After: \n");
                        ContainerPanel_LogGAMEContainerItems(pChildCur->Container);
                        ContainerPanel_LogGroupItems();
                    #endif
                    //
                    
                    pChildCur = pChildNext;
                }


                // reload all items
                // maybe slow, but fully synced with game
                ContainerPanel_CreateItemGroupArray();
                return;

            }
            else
            {   // button has one item

                if (InventoryIsFull(
                           *pGAMECreatureObjID,
                            pItemGroupNode->Item,
                            pItemGroupNode->dwFlags,
                            pItemGroupNode->Stackable,
                            pItemGroupNode->Count)) {

                        #ifdef _DEBUG
                            console.writef("Inventory Full \n");
                        #endif
                    
                        return;
                    }

                #ifdef _DEBUG
                    console.writef("Before: \n");
                    ContainerPanel_LogGAMEContainerItems(pItemGroupNode->Container);
                    //ContainerPanel_LogGroupItems();
                #endif

                // override ButtonIndex and ContainerObjID
                *pGAMEContainerItemIndex    = pItemGroupNode->ContainerItemIndex | 0x8000; // 0x8000 = bypass ContainerItemIndex recalc
                *pGAMEContainerObjID        = pItemGroupNode->ContainerNode->ContainerObjID;
                /////////////////////////////////////////////
                ContainerPanelButton::OnLClicked(pt);
                /////////////////////////////////////////////
                // restore IDs
                *pGAMEContainerItemIndex    = OrigButtonIndex;
                *pGAMEContainerObjID        = OrigGameContainerObjID;

                ContainerPanel_ForceProcessMessages(pItemGroupNode->Container, pItemGroupNode->ContainerNode->ContainerObjID);
                //ContainerPanel_ShiftContainerItemIndexes

                #ifdef _DEBUG
                    console.writef("After: \n");
                    ContainerPanel_LogGAMEContainerItems(g_pContainerWaitForPurgeContainer);
                    ContainerPanel_LogGroupItems();
                #endif

                // reload all items
                // maybe slow, but fully synced with game
                ContainerPanel_CreateItemGroupArray();
                return;
            }

        }
        else
        {   //empty button
            return;
            //pContainerObjID = 0xFFFFFFFFL;
            //pSlotIndex = 0xFFFF;
        }
    }


    // Page Buttons
    if ((ButtonID == LEFT_PAGE_BUTTONID) ||
        (ButtonID == RIGHT_PAGE_BUTTONID))
    {
        ScrollClick(ButtonID);
        return;
    }


    // default buttons
    ContainerPanelButton::OnLClicked(pt);
    return;
    
}


void
ContainerPanel_AddSortElement(int const element_start, int const element_end)
{
    int     n;
    char    Buf[4]; // '255'+ end null

    if ((element_end - element_start + 1) > 3) {
        return;}

    memcpy(Buf, & gIniSort_Buf[element_start], element_end - element_start + 1);
    Buf[element_end - element_start + 1] = '\0';
    sscanf_s(Buf, "%d", &n);
    gSort_Buf[gSort_Buf_MaxIndex]=n;
    gSort_Buf_MaxIndex++;

}


void
ContainerPanel_ParseIniSort()
{
    int i, element_start, element_end;

    element_start = 0;
    for (i = 0; gIniSort_Buf[i] != '\0'; i++ ) {
        if (gIniSort_Buf[i] != ','  &&
            gIniSort_Buf[i] != ';'  &&
            gIniSort_Buf[i] != ' '  &&
            (gIniSort_Buf[i] <  '0' || gIniSort_Buf[i] >  '9')) {
                element_start++;
                continue;
        }

        if (gIniSort_Buf[i] == ','  ||
            gIniSort_Buf[i] == ';'  ||
            gIniSort_Buf[i] == ' ') {
                element_end     = i - 1;
                if (element_start <= element_end) { // no double separators
                    ContainerPanel_AddSortElement(element_start, element_end);
                }
                element_start   = i + 1;
        }
    }

    // last one
    if (gIniSort_Buf[i] == '\0' ||
        gIniSort_Buf[i] == ','  ||
        gIniSort_Buf[i] == ';'  ||
        gIniSort_Buf[i] == ' ') {
            element_end     = i - 1;
            if (element_start <= element_end) {
                ContainerPanel_AddSortElement(element_start, element_end);
            }
    }

    gSort_Buf_MaxIndex--;           // normalize after gSort_Buf_MaxIndex--

}




void __declspec(naked)
ContainerPanel_CreateMesageType1_asm() {
__asm {
    ;0x69B046:
    ; push    10Ah
    ; call    malloc
    ; add     esp, 4
    ; mov     [ebp-30h], eax
    ////////////////////////////////
    mov     g_pPurgeMessageType1, eax
    ////////////////////////////////
    ;0x69B056:
    mov     dword ptr [ebp-4], 1            ; stolen bytes
    ret
}
}


void __declspec(naked)
ContainerPanel_CreateMesageType2_asm() {
__asm {
    ;0x7E5DB6
    ; push    10Ah
    ; call    malloc
    ; add     esp, 4
    ; mov     [ebp-A4h], eax
    /////////////////////////////////
    mov     g_pPurgeMessageType2,  eax
    /////////////////////////////////
    ;0x7E5DC9:
    mov     dword ptr [ebp-4], 0            ; stolen bytes
    ret
}
}



void __declspec(naked)
ContainerPanel_CreateArrayOfContainersAround_asm() {
__asm {
    ; edx = pCre
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    edx
    ; stdcall
    call    ContainerPanel_CreateArrayOfContainersAround

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    add     edx, 1Ch            ; stolen bytes
    ;push   edx                 ; moved to patch.cpp otherwise "ret" will mess stack
    mov     ecx, [ebp-1CH]      
    ret
}
}


void  __declspec(naked)
ContainerPanel_StopContainer_asm() {
__asm {
    ;0x7D9832
    ; push    ebp
    ; mov     ebp, esp
    ; sub     esp, 0

    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    call    ContainerPanel_StopContainer

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ;0x7D983B:
    mov     [ebp-0A8h], ecx     ; stolen bytes
    ret
}
}


void __declspec(naked)
ContainerPanel_ButtonRedraw_asm() {
__asm {
    mov     dword ptr [ebp-20h], 2EEBh  ;   mov     [ebp+Item_TextRefID_Final], 2EEBh
    mov     ecx, [ebp-33Ch]             ;   mov     ecx, [ebp+CButton]
    mov     edx, [ecx+0Ah]              ;   edx =   ButtonID
    mov     [ebp-24h], dx               ;   mov     [ebp+ItemIndex], dx

    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    ; 7 args
    push    ecx                     ;   ecx =               CButton
    lea     eax, [ebp-2Ch]          ;   lea     eax, [ebp+  rItem_Count]
    push    eax
    lea     ecx, [ebp-50h]          ;   lea     ecx, [ebp+  rItem_Name]
    push    ecx
    lea     edx, [ebp-34h]          ;   lea     edx, [ebp+  rItem_Icon]
    push    edx
    lea     eax, [ebp-54h]          ;   lea     eax, [ebp+  rItem_TextRefID]
    push    eax
    lea     ecx, [ebp-28h]          ;   lea     ecx, [ebp+  rItem]
    push    ecx
    mov     dx, [ebp-24h]           ;   mov      dx, [ebp+  ItemIndex]
    push    edx
        
    ; stdcall
    call    ContainerPanel_ButtonRedraw

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ret
}
}


void GetTweakIniStrValue(LPCTSTR szSection, LPCTSTR szKey, LPSTR sResult, DWORD SizeResult) {
    const LPCTSTR s_szFileTweak = "./TobEx_ini/TobExTweak.ini";
    GetPrivateProfileString(szSection, szKey, "", sResult, SizeResult, s_szFileTweak);
}


void
ContainerPanel_Init()
{
    ContainerPanel_ResetStaticVars();
    gGroupMode = to_bool(GetTweakIniValue("Tweak", "UI:Loot Panel Item Grouping"));

    gSort_Buf_MaxIndex    = 0;
    gSortMode_Normal      = false;
    gSortMode_UnknowFirst = false;
    memset(gIniSort_Buf, 0, sizeof(gIniSort_Buf));
    GetTweakIniStrValue("Tweak", "UI:Loot Panel Sorting Order", gIniSort_Buf, sizeof(gIniSort_Buf));
    if (strlen(gIniSort_Buf) != 0)  {
        gSortMode_Normal = true;
        ContainerPanel_ParseIniSort();
    }

    int mode = GetTweakIniValue("Tweak", "UI:Loot Panel Sorting Unidentified First");
    if (mode)  {
        gSortMode_UnknowFirst = true;
    }

    InitializeCriticalSection(&gCriticalSection);

    DetourMemberFunction(Tramp_ContainerPanelButton_OnLClicked, DETOUR_ContainerPanelButton::DETOUR_OnLClicked);
    DetourMemberFunction(Tramp_ContainerPanelButton_Redraw,     DETOUR_ContainerPanelButton::DETOUR_Redraw);

}
