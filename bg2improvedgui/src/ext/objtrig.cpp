#include "objtrig.h"
#include "objdoor.h"

extern short gTabPressed;

extern CGameObject* GetObjectByID_Fast(DWORD);

typedef struct _ListNode {
	struct _ListNode* pNext;
	struct _ListNode* pPrev;
	DWORD     EnumID;
} ListNode;


bool
IsDoorOverlap(CTriggerObject& TriggerObj, CGameDoor& Door) {
    // Door.BoundsOpen + Door.BoundsClosed vs TriggerObj.rectBounds
    RECT tmpOpen;
    RECT tmpClosed;
    RECT tmp1, tmp2;

    // ex-secret doors
    if ( (Door.dwFlags & 0x01) &&           // already open
         Door.nVerticesOutlineOpen == 0 )   // empty after opening
        return  false;

    if (Door.nVerticesOutlineOpen != 0)
        tmpOpen = Door.BoundsOpen;
    else
        tmpOpen = Door.BoundsClosed;

    if (Door.nVerticesOutlineClosed != 0)
        tmpClosed = Door.BoundsClosed;
    else
        tmpClosed = Door.BoundsOpen;

    UnionRect(&tmp1, &tmpOpen, &tmpClosed);
    IntersectRect(&tmp2, &TriggerObj.rectBounds, &tmp1);

    return !(to_bool(IsRectEmpty(&tmp2)));
}


bool static
NoDoorAround(CTriggerObject& TriggerObj) {
    ListNode* Node;

    Node = (ListNode*) TriggerObj.pArea->m_lVertSortFront.GetHeadPosition();
    while (Node) {
        CGameObject* Obj = GetObjectByID_Fast(Node->EnumID);
        if (Obj != NULL &&
            Obj->nObjType == CGAMEOBJECT_TYPE_DOOR) {
                if (IsDoorOverlap(TriggerObj, (CGameDoor&) *Obj))
                    return false;
        }

        Node = Node->pNext;
    }

    return true;
}


void static __stdcall
HighlightTrigger(CTriggerObject& TriggerObj) {
    if (
        (TriggerObj.nType == 1 &&              // Info   Trigger
            (pGameOptionsEx->bUI_HighlightActiveZones_InfoQuestion ||
             pGameOptionsEx->bUI_HighlightActiveZones_InfoAction)
        )
       ||
        (TriggerObj.nType == 2 &&              // Travel Trigger
            (pGameOptionsEx->bUI_HighlightActiveZones_EnterSteps ||
             pGameOptionsEx->bUI_HighlightActiveZones_EnterDoor)
        )
       ) {
        ushort VisibilityTile  = TriggerObj.pArea->m_VisMap.pMap[
                TriggerObj.pArea->m_VisMap.wWidth * (TriggerObj.currentLoc.y / 32) +    // 0xAACF4B =32 CVisibilityMap::SQUARE_SIZEY
                TriggerObj.currentLoc.x / 32];                                          // 0xAACF4B =32 CVisibilityMap::SQUARE_SIZEX

        if (g_pChitin->pScreenWorld->TabPressedCountDownTime > 0 &&
            (VisibilityTile & 0x8000) != 0 ) {
            POINT Rhomb[4];
            Rhomb[0].x = TriggerObj.currentLoc.x + 20 ; Rhomb[0].y = TriggerObj.currentLoc.y      ;
            Rhomb[1].x = TriggerObj.currentLoc.x      ; Rhomb[1].y = TriggerObj.currentLoc.y + 20 ;
            Rhomb[2].x = TriggerObj.currentLoc.x - 20 ; Rhomb[2].y = TriggerObj.currentLoc.y      ;
            Rhomb[3].x = TriggerObj.currentLoc.x      ; Rhomb[3].y = TriggerObj.currentLoc.y - 20 ;

            if ( (TriggerObj.nCursorIdx  == 8 ||    // Curved Arrows
                  TriggerObj.nCursorIdx  == 34)     // Wheel
                && pGameOptionsEx->bUI_HighlightActiveZones_InfoAction) {
                TriggerObj.pArea->m_cInfinity.OutlinePoly(
                    Rhomb,
                    4,
                    TriggerObj.rectBounds,
                    0xffff40);      // Light Blue
            }

            if (TriggerObj.nCursorIdx  == 22 &&     // Question
                pGameOptionsEx->bUI_HighlightActiveZones_InfoQuestion) {
                TriggerObj.pArea->m_cInfinity.OutlinePoly(
                    Rhomb,
                    4,
                    TriggerObj.rectBounds,
                    0x00FF00);      // Green
            }

            if (
                (TriggerObj.nCursorIdx  == 28 &&    // Steps Up Enter
                 pGameOptionsEx->bUI_HighlightActiveZones_EnterSteps
                )
               ||
                (TriggerObj.nCursorIdx  == 42 &&    // Enter Area
                pGameOptionsEx->bUI_HighlightActiveZones_EnterDoor
                )
               ) {
                if (NoDoorAround(TriggerObj))       // Skip if nearby there is Door
                    TriggerObj.pArea->m_cInfinity.OutlinePoly(
                        Rhomb,
                        4,
                        TriggerObj.rectBounds,
                        0x00FFFF);  // Yellow
            }
        }
    }
}


void  __declspec(naked)
CGameTrigger_Render_Highlight_asm() {
__asm {
    push    eax    
    push    ecx
    push    edx

    push    [ebp-1Ch]       ; TriggerObj
    call    HighlightTrigger

    pop     edx
    pop     ecx
    pop     eax

    mov     cl, [eax+4C34h] ; Stolen bytes
    ret
}
}