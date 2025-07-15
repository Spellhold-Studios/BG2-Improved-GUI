#ifndef CONTAINERPANEL_H
#define CONTAINERPANEL_H

#include "objcre.h"
#include "objcont.h"
#include "itmcore.h"
#include "chitin.h"


#define RANGE_CONTAINERS_AROUND 500

#define INVENTORYSLOT_FIRST     0x12
#define INVENTORYSLOT_LAST      (0x12 + 16 - 1)

// ButtonID  <6 [14 15 16 17 18 19 20 21 22 23 24] 7>
#define FIRST_BUTTONID          14
#define LEFT_PAGE_BUTTONID      6
#define RIGHT_PAGE_BUTTONID     7

// 640x480
#define LAST_BUTTONID_640       24
#define MAX_Y_640               480
#define BUTTONS_PER_SCREEN_640  (LAST_BUTTONID_640 - FIRST_BUTTONID + 1)
#define GAMEFIELD_Y_COOR_640    (MAX_Y_640 - 135)

// 800x600
#define LAST_BUTTONID_800       27
#define MAX_Y_800               600
#define BUTTONS_PER_SCREEN_800  (LAST_BUTTONID_800 - FIRST_BUTTONID + 1)
#define GAMEFIELD_Y_COOR_800    (MAX_Y_800 - 135)

// 1024x768
#define LAST_BUTTONID_1024      33
#define MAX_Y_1024              768
#define BUTTONS_PER_SCREEN_1024 (LAST_BUTTONID_1024 - FIRST_BUTTONID + 1)
#define GAMEFIELD_Y_COOR_1024   (MAX_Y_1024 - 135)

#define LAST_ANY_BUTTONID       33

#define SORT_NORMAL         0
#define SORT_UNKNOWFIRST    1

void ContainerPanel_CreateArrayOfContainersAround_asm();
void ContainerPanel_ButtonRedraw_asm();
void ContainerPanel_CreateMesageType1_asm();
void ContainerPanel_CreateMesageType2_asm();
void ContainerPanel_StopContainer_asm();
void ContainerPanel_Init();


extern const char*         const   g_pButtonCreateOpcodes;
extern const unsigned int* const   g_pPointerYCoor;


/////////////////////////////////////////////////////////
class ContainerPanelButton : public CUIButton {
public:
    virtual BOOL Redraw(BOOL bForceRedraw);     //v4c
    virtual void OnLClicked(POINT pt);          //v5c
};

class DETOUR_ContainerPanelButton : public ContainerPanelButton {
public:
    BOOL DETOUR_Redraw(BOOL bForceRedraw);
    void DETOUR_OnLClicked(POINT pt);

    // new
    void ScrollClick(short ButtonID);
};
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
class MyContainerObject : public CGameContainer {
public:
    // new
    void PurgeItems();
};

class DETOUR_Container : public MyContainerObject {
public:
    // new
    void DETOUR_PurgeItems();
};
/////////////////////////////////////////////////////////



typedef struct _ContainerArray_Head {
    struct _ContainerArray_Node *Head;
    struct _ContainerArray_Node *Tail;
} ContainerArray_Head;


typedef struct _ItemArray_Head {
    struct _ItemArray_Node *Head;
    struct _ItemArray_Node *Tail;
} ItemArray_Head;


typedef struct _ItemGroupArray_Head {
    struct _ItemGroupArray_Node *Head;
    struct _ItemGroupArray_Node *Tail;
} ItemGroupArray_Head;


typedef struct _ContainerArray_Node {
    struct _ContainerArray_Node *Head;
    struct _ContainerArray_Node *Tail;

    CGameContainer*     Container;
    Enum                ContainerObjID;
    bool                NeedSync;
} ContainerArray_Node;


typedef struct _ItemArray_Node {
    struct _ItemArray_Node *Head;
    struct _ItemArray_Node *Tail;

    CGameContainer*      Container;
    ContainerArray_Node* ContainerNode;
    CItem*               Item;
    short                Count;
    unsigned int         dwFlags;
    bool                 Stackable;
    short                ContainerItemIndex;
    bool                 Deleted;
    unsigned short       ItemType;
} ItemArray_Node;


typedef struct _ItemGroupArray_Node {
    struct _ItemGroupArray_Node *Head;
    struct _ItemGroupArray_Node *Tail;
    
    struct _ItemArray_Node      *ChildHead;
    struct _ItemArray_Node      *ChildTail;

    CGameContainer*      Container;
    ContainerArray_Node* ContainerNode;
    CItem*               Item;
    char                 Name[8];
    short                Count;
    unsigned int         dwFlags;
    bool                 Stackable;
    short                ContainerItemIndex;
    bool                 HasChild;
    bool                 Deleted;
    unsigned short       ItemType;

} ItemGroupArray_Node;


#endif // CONTAINERPANEL_H