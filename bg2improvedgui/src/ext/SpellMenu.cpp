#include "SpellMenu.h"

#define xStart_MinDlg *( (int *)0xB84C90)
#define yStart_MinDlg *( (int *)0xB84C94)
#define xEnd_MinDlg   *( (int *)0xB84C98)
#define yEnd_MinDlg   *( (int *)0xB84C9C)

#define xStart_MidDlg *( (int *)0xB84CB0)
#define yStart_MidDlg *( (int *)0xB84CB4)
#define xEnd_MidDlg   *( (int *)0xB84CB8)
#define yEnd_MidDlg   *( (int *)0xB84CBC)

#define xStart_MaxDlg *( (int *)0xB84C80)
#define yStart_MaxDlg *( (int *)0xB84C84)
#define xEnd_MaxDlg   *( (int *)0xB84C88)
#define yEnd_MaxDlg   *( (int *)0xB84C8C)

#define QS_PtrLIst          0xB83D0C
#define gButtorArrayPtrList (*( (CGameButtonList **)QS_PtrLIst))

_n void CIcon::RenderIcon(int argX, POINT&  pos, SIZE&   size, RECT&   rBoundingBox, ResRef& ResIcon,
                           BOOL    bDoubleResolutionButtons, ulong   dwFlags, ushort  wCount, BOOL    bForceCount,
                           ushort  wSecondCount, BOOL    bForceSecondCount ) { _bgmain(0x5A5989) }



static BOOL     gSpellMenuEnabled = 0;
static HANDLE   gRenderNotBusy     = NULL;
static int      gMainWindowSizeMode;
static bool     gStopAreaObjects = true;
static IECPtrList gIconObjList;
static int      gButtonArrayTypePreviousIdx;
static bool     gEnableSpellMenu = true;
static bool     gSwitchBookInProgress;
static int      gRequestedToolbarState;
int             gPrev_ToolbarState;
int             gPrev_ToolbarBook;
static BOOL     gLastButtonIDUnderCursor;


_n uchar SpellIcon::GetObjectType()
{ _bgmain(0x494CB0)}

_n void  SpellIcon::AddToArea(CArea *, POINT *, long, uchar)
{ _bgmain(0x573567)}

_n Object& SpellIcon::GetCurrentObject()
{ _bgmain(0x494CE0)}

_n long    SpellIcon::GetTargetId()
{ _bgmain(0x494D20)}

_n void    SpellIcon::GetNextWaypoint(POINT*)
{ _bgmain(0x494D30)}

_n POSITION* SpellIcon::GetVertListPos()
{ _bgmain(0x494D70)}

_n uchar   SpellIcon::GetVertListType()
{ _bgmain(0x494D90)}

_n bool    SpellIcon::CanSaveGame(STRREF*, int, int)
{ _bgmain(0x494DB0)}

_n bool    SpellIcon::CompressTime(unsigned long)
{ _bgmain(0x495500)}    //v28 CGameAIBase::CompressTime
//{ _bgmain(0x5735E9)}   //v28 CGameObject::CompressTime

_n void    SpellIcon::DebugDump(IECString&, bool)
{ _bgmain(0x494DD0)}

_n BOOL    SpellIcon::DoesIntersect(RECT r)
{ _bgmain(0x494DF0)}

_n BOOL    SpellIcon::OnSearchMap()
{ _bgmain(0x494E00)}

_n bool    SpellIcon::SetObject(Object&, int)
{ _bgmain(0x494E90)}

_n void    SpellIcon::SetTarget(POINT target, int)
{ _bgmain(0x494EC0)}

_n void    SpellIcon::SetVertListPos(POSITION*)
{ _bgmain(0x494ED0)}

_n BOOL    SpellIcon::EvaluateStatusTrigger(Trigger&)
{ _bgmain(0x494EF0)}


void
StopAreaIcons() {
    gStopAreaObjects = true;
    int waitstatus = WaitForSingleObject(gRenderNotBusy, INFINITE);
}


SpellIcon::SpellIcon() {
    rect.bottom = 0;
    rect.left = 0;
    rect.right = 0;
    rect.top = 0;

    SpellButton.bDisabled = false;
    SpellButton.bDisplayCount = 0;
    SpellButton.rLauncherIcon = "";
    SpellButton.rUsageIcon = "";
    SpellButton.strrefLauncherName = 0;
    SpellButton.strrefName = 0; 
    SpellButton.wAmount = 0;

    bNumberIcon = false;
    bDisabled = false;
}

SpellIcon::~SpellIcon() {}


#define InfoPanelSpellMenu      4
void SummonPriestInfoPanel(ResRef* Name, int ToolBarType, CButtonData* ButtonData);

void
SpellIcon::OnFormationButton(POINT *pt)
{
    if (pGameOptionsEx->bUI_SpellIconRightClick) {
        if (!bNumberIcon)
            SummonPriestInfoPanel(&SpellButton.abilityId.rSpellName, InfoPanelSpellMenu, NULL);
    }
}


void
SpellIcon::RemoveFromArea() {   // called from CGameArea::ClearMarshal()
    StopAreaIcons();
    gSpellMenuEnabled = 0;
    DeleteAreaObjects(NULL);
}


void
SpellIcon::AIUpdate() {}


bool
SpellIcon::DoAIUpdate(bool bRun, long nChitinUpdates) {
    return false;
}


BOOL static __stdcall
SpellIcon_IsPointOverMe(SpellIcon& obj ) {  // , POINT& pt)
    if (PtInRect(&obj.rect, obj.pArea->mousePos)) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


void
SpellIcon::SetCursor(long nToolTip){
    if (nToolTip < g_pChitin->pGame->m_GameOptions.m_toolTips) {
        if (bNumberIcon)
            g_pChitin->pCursor->SetCursor(6, 0, -1);    // 6 - Out of area
        else
            g_pChitin->pCursor->SetCursor(0, 0, -1);    // 0 - Hand
    } else {    // expired time, tooltip on
        if (!bNumberIcon) {
            IECString empty;
            g_pChitin->pCursor->vcToolTip.SetTextRef(&SpellButton.strrefName, &empty);
            g_pChitin->pCursor->pCtrlTarget=NULL;
            g_pChitin->pGame->tempCursor = 0x65;    // tooltip
        }
    }
}


void
SpellIcon::OnActionButton(POINT *pt) {
    StopAreaIcons();
    gSpellMenuEnabled = 0;
    CloseMenu();
    DeleteAreaObjects(this);

    if (bDisabled) {
        delete this;    // safe delete
        return;
    }

    CButtonArray& Array = g_pChitin->pGame->m_CButtonArray;
    if ( gRequestedToolbarState == 102 || // assign spell to key mode
         gRequestedToolbarState == 114 ) {
        CButtonArray_SetQuickSlot(&SpellButton, Array.nItemSlotIdx, 2);

        CButtonArray_ClearList();
        Array.SetState(gButtonArrayTypePreviousIdx);
    } else {
        bool result = CButtonArray_PickSpell(&SpellButton, 1);

        Enum id = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
        CCreatureObject* Cre = NULL;
        char nResult;
        do {
            nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(id, THREAD_ASYNCH, &Cre, INFINITE);
        } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
        if (nResult == OBJECT_SUCCESS) {
            if (Cre->currentUseButton.abilityId.nTargetType != 5 &&
                Cre->currentUseButton.abilityId.nTargetType != 6 &&
                Cre->currentUseButton.abilityId.nTargetType != 7 ) {
                Array.nActiveButtonIdx = 3;
                Array.UpdateButtons();
            }
            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(id, THREAD_ASYNCH, INFINITE);
        }

        CButtonArray_ClearList();
        Array.SetState(gButtonArrayTypePreviousIdx);

        if (result != true) {   // ?, copy from orig code
            Array.nActiveButtonIdx = 100;
            Array.UpdateButtons();
        }
    }

    delete this;    // safe delete, CButtonArray_PickSpell() make copy of QuickObject to local storage
}

#define ICONGRIDSIZE (32 + 6)*(1 + g_pChitin->bDoubleResolutionButtons)

void
SpellIcon::Draw(CArea *pArea1, CVideoMode *pVidMode, int nSurface) {
    if (gStopAreaObjects)
        return;

    if (!gSpellMenuEnabled) {
        return;
    }

    ResetEvent(gRenderNotBusy);

    //CIcon icon;
    ResRef BackIcon = "SPLBUT44";
    ResRef NumberIcon = "NUMBER";
    POINT  BackLoc;
    BackLoc.x = currentLoc.x - 3*(1 + g_pChitin->bDoubleResolutionButtons); //(ICONPLACESIZE - 32);
    BackLoc.y = currentLoc.y - 3*(1 + g_pChitin->bDoubleResolutionButtons); //(ICONPLACESIZE - 32);

    SIZE  size  =       {32*(1 + g_pChitin->bDoubleResolutionButtons), 32*(1 + g_pChitin->bDoubleResolutionButtons)};                        // icon max size
    SIZE  fullsize =    {ICONGRIDSIZE, ICONGRIDSIZE};  // icon max size
    //RECT  rBoundingBox= {xStart_MinDlg, yStart_MinDlg, xEnd_MinDlg, yEnd_MinDlg};
    RECT  rBoundingBox= {currentLoc.x, currentLoc.y, currentLoc.x + ICONGRIDSIZE, currentLoc.y + ICONGRIDSIZE};
    RECT  rBackBoundingBox= {BackLoc.x, BackLoc.y, BackLoc.x + ICONGRIDSIZE, BackLoc.y + ICONGRIDSIZE};

    // slow render
    //icon.RenderIcon(    // Background icon
    //        0,
    //        BackLoc,
    //        fullsize,
    //        rBoundingBox,
    //        BackIcon,
    //        g_pChitin->bDoubleResolutionButtons,
    //        0,
    //        0,
    //        FALSE,
    //        0,
    //        FALSE);

    //icon.RenderIcon(    // Spell icon
    //        0,
    //        currentLoc,
    //        size,
    //        rBoundingBox,
    //        SpellButton.rUsageIcon,
    //        g_pChitin->bDoubleResolutionButtons,
    //        0,
    //        SpellButton.wAmount,
    //        FALSE,
    //        0,
    //        FALSE);
    //_asm { int 3 }

    POINT  CenterPoint, CenterPointBack;
    SIZE   Size, SizeBack;
    int    Flags =     0x00;
    int    FlagsIcon = 0x00;
    bool   bSkipIcon = false;

    CVidCell IconCell(SpellButton.rUsageIcon, g_pChitin->bDoubleResolutionButtons);
    if (!IconCell.bam.bLoaded) {    // empty/broken icon
        bSkipIcon = true;
    } else {                        // normal icon
        IconCell.SequenceSet(1);
        IconCell.FrameSet(0);
        IconCell.GetCurrentCenterPoint(&CenterPoint, 0);
        IconCell.GetCurrentFrameSize(&Size, 0);

        if (bDisabled) {
            IconCell.SetTintColor(0x0B4B4B4);   // INVALID_TINT_COLOR
            FlagsIcon |= 0x2020000;             // SHADOW
        }
    }

    CVidCell BackCell(BackIcon, g_pChitin->bDoubleResolutionButtons);
    CVidCell NumberCell(NumberIcon, g_pChitin->bDoubleResolutionButtons);

    BackCell.SequenceSet(1);
    BackCell.FrameSet(0);
    BackCell.GetCurrentCenterPoint(&CenterPointBack, 0);
    BackCell.GetCurrentFrameSize(&SizeBack, 0);

    NumberCell.SequenceSet(0);

    RECT    FXRect =     {0, 0, Size.cx, Size.cy};
    RECT    FXRectback = {0, 0, SizeBack.cx, SizeBack.cy};
    POINT   ptRef =      {0, 0};
    int     OffsetX = pArea1->m_cInfinity.m_rWindow.left;
    int     OffsetY = pArea1->m_cInfinity.m_rWindow.top;

    POINT pos = currentLoc;
    POINT posBlt = {currentLoc.x - OffsetX + pArea1->m_cInfinity.nCurrentX,
                    currentLoc.y - OffsetY + pArea1->m_cInfinity.nCurrentY};

    POINT posBack = BackLoc;
    POINT posBltBack = {BackLoc.x - OffsetX + pArea1->m_cInfinity.nCurrentX,
                        BackLoc.y - OffsetY + pArea1->m_cInfinity.nCurrentY};

    if (1) {    // fast render
        // get surface
        pArea1->m_cInfinity.FXPrep(FXRectback, Flags, nSurface, posBack, CenterPointBack);
        pArea1->m_cInfinity.FXLock(FXRectback, Flags);

        // fill
        pArea1->m_cInfinity.FXRender(&BackCell, CenterPointBack.x, CenterPointBack.y, Flags, -1);
        if (!bSkipIcon)
            pArea1->m_cInfinity.FXRender(&IconCell, 3*(1 + g_pChitin->bDoubleResolutionButtons), 3*(1 + g_pChitin->bDoubleResolutionButtons), FlagsIcon, -1);

        if (SpellButton.wAmount) {
            IECString sNumbers;
            sNumbers.Format("%d", SpellButton.wAmount);
            int Strlen = sNumbers.GetLength();
            int Numbers_X = (32 + 3 - Strlen*5) * (1 + g_pChitin->bDoubleResolutionButtons);
            int Numbers_Y = (32 + 3 -      1*7) * (1 + g_pChitin->bDoubleResolutionButtons);

            for (int i = 0; i < Strlen; i++) {
                NumberCell.FrameSet(sNumbers.GetAt(i) - 48);
                pArea1->m_cInfinity.FXRender(&NumberCell, Numbers_X, Numbers_Y, Flags, -1);
                Numbers_X += 5 * (1 + g_pChitin->bDoubleResolutionButtons);
            }
        }

        // commit
        pArea1->m_cInfinity.FXUnlock(Flags, NULL, ptRef);
        pArea1->m_cInfinity.FXBltFrom(nSurface, FXRectback, posBltBack.x, posBltBack.y, CenterPointBack.x, CenterPointBack.y, Flags);
    } else {    // safe but slow render
        BackCell.Render(0, posBack.x, posBack.y, &rBackBoundingBox, 0, 0, 0, -1);
        IconCell.Render(0, pos.x, pos.y, &rBoundingBox, 0, 0, 0, -1);
    }

    SetEvent(gRenderNotBusy);
}


void static
AddNumberIcon(CArea* pArea, int lvl, int X, int Y) {
    IECString NumBAM;

    SpellIcon* obj= new SpellIcon;

    NumBAM.Format("B3NUM%d", lvl);
    obj->SpellButton.rUsageIcon = NumBAM;
    obj->SpellButton.wAmount = 0;
    obj->nObjType = CGAMEOBJECT_TYPE_ICONGUI;
    obj->currentLoc.x = X;
    obj->currentLoc.y = Y;
    obj->rect.left = obj->currentLoc.x;
    obj->rect.top = obj->currentLoc.y;
    obj->rect.right =  obj->rect.left + 32*(1 + g_pChitin->bDoubleResolutionButtons);
    obj->rect.bottom = obj->rect.top  + 32*(1 + g_pChitin->bDoubleResolutionButtons);
    obj->pArea = NULL;
    obj->bNumberIcon = true;
    obj->bDisabled = false;

    char status = g_pChitin->pGame->m_GameObjectArrayHandler.Add(&obj->id, obj, -1);
    if ( status == OBJECT_SUCCESS ) {
        obj->AddToArea(pArea, &obj->currentLoc, 1, LIST_FRONT);
    } else { _asm { int 3 } }

    gIconObjList.AddTail(obj);
}


void static
CreateAreaObjects(int BookType) {
    IECPtrList* pCQuickObjectList = gButtorArrayPtrList;
    if (pCQuickObjectList == NULL) {
        return;
    }
    if (pCQuickObjectList->GetCount() == 0) {
        return;
    }

    CArea* pArea;
    CCreatureObject* Cre = NULL;
    Enum CreID = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
	char nResult;
	do {
		nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(CreID, THREAD_ASYNCH, &Cre, INFINITE);
	} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
	if (nResult == OBJECT_SUCCESS)
        pArea = Cre->pArea;
    else
        return;
    g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(CreID, THREAD_ASYNCH, INFINITE);

    int   startX = 70*(1 + g_pChitin->bDoubleResolutionButtons);
    int   startY = yEnd_MinDlg - 9*ICONGRIDSIZE - 10;   // wiz 9 levels
    //int startY = yEnd_MinDlg - 7*32 - 10;   // priest 9 levels
    int   Line = -(ICONGRIDSIZE);
    int   Row = 0;
    short prev_lvl = 0;

    CButtonData* pButton;
    POSITION pos = pCQuickObjectList->GetHeadPosition();
    while (pos) {
        pButton = (CButtonData*) pCQuickObjectList->GetNext(pos);

        ResSplContainer Spell(pButton->abilityId.rSpellName);
        Spell.Demand();
        short lvl = Spell.GetSpellLevel();
        short type = Spell.GetSpellType();
        Spell.Release();

        if (BookType != 0) {
            if (type != BookType &&
                type != SPELLTYPE_INNATE)
                continue;
        }

        SpellIcon* obj= new SpellIcon;
        obj->nObjType = CGAMEOBJECT_TYPE_ICONGUI;
        obj->SpellButton = *pButton;

        if (type == SPELLTYPE_INNATE)
            lvl = 9;                // to last line for priest

        if (lvl > prev_lvl) {
            while (lvl > prev_lvl) {    // insert number icons
                Row=0;
                prev_lvl++;
                Line += ICONGRIDSIZE;
                AddNumberIcon(pArea, prev_lvl, startX + Row, startY + Line);
            }

            Row += ICONGRIDSIZE;
        }

        obj->currentLoc.x = startX + Row;
        obj->currentLoc.y = startY + Line;
        obj->rect.left = obj->currentLoc.x;
        obj->rect.top = obj->currentLoc.y;
        obj->rect.right =  obj->rect.left + 32*(1 + g_pChitin->bDoubleResolutionButtons);
        obj->rect.bottom = obj->rect.top  + 32*(1 + g_pChitin->bDoubleResolutionButtons);
        obj->pArea = NULL;
        obj->bNumberIcon = false;
        if (pButton->bDisabled)
            obj->bDisabled = true;
        else
            obj->bDisabled = false;

        char status = g_pChitin->pGame->m_GameObjectArrayHandler.Add(&obj->id, obj, -1);
        if ( status == OBJECT_SUCCESS ) {
            obj->AddToArea(pArea, &obj->currentLoc, 1, LIST_FRONT);
        } else { _asm { int 3 } }

        gIconObjList.AddTail(obj);
        Row += ICONGRIDSIZE;
    }
}


void static
DeleteAreaObjects(SpellIcon* caller) {
    if (gIconObjList.GetCount() == 0)
        return;

    POSITION pos = gIconObjList.GetHeadPosition();
    while (pos) {
        POSITION posTarget = pos;
	    SpellIcon* p = (SpellIcon*) gIconObjList.GetNext(pos);
        char status;
        CGameObjectArrayHandler& ObjArray = g_pChitin->pGame->m_GameObjectArrayHandler;
        CGameObject* dummy;

        do {    // sync with CGameArea::Render()
            status = ObjArray.GetGameObjectDeny(p->id, THREAD_ASYNCH, &dummy, INFINITE);
        } while  (status == OBJECT_SHARING || status == OBJECT_DENYING);

        if (status == OBJECT_SUCCESS) {
            p->RemoveFromArea_();

            status = ObjArray.Delete(p->id, 0, 0, -1);
            if ( status != OBJECT_SUCCESS ) { _asm { int 3 } }
                    
            status = ObjArray.FreeGameObjectDeny(p->id, THREAD_ASYNCH, INFINITE);
            if ( status != OBJECT_SUCCESS ) { _asm { int 3 } }
        }

        gIconObjList.RemoveAt(posTarget);
        if (p != caller) {  // don't destroy self yet
            delete p;
        }
    }
}


void static
CloseMenu() {
    CScreenWorld* screen = g_pChitin->pScreenWorld;
    CPanel& panel4  = screen->manager.GetPanel(4);   // min
    CPanel& panel7  = screen->manager.GetPanel(7);   // max
    CPanel& panel12  = screen->manager.GetPanel(12);  // mid

    if (gMainWindowSizeMode == 4 ) {    // min DLG window
        ;   // already minimized
    } else
    if (gMainWindowSizeMode == 12) { // mid
        if (screen->manager.u0 == NULL) { // ?, copied original code
            CUIButton& PageUpButton = (CUIButton&) panel4.GetUIControl(2); // PageUp control
            if (PageUpButton.bEnabled)
                PageUpButton.OnLMouseBtUp(PageUpButton.pos);
        }
    } else
    if (gMainWindowSizeMode == 7) {  // max
        if (screen->manager.u0 == NULL) { // ?, copied original code
            CUIButton& PageUpButton = (CUIButton&) panel4.GetUIControl(2); // PageUp control
            if (PageUpButton.bEnabled)
                PageUpButton.OnLMouseBtUp(PageUpButton.pos);
        }
        if (panel12.bEnabled) { // min -> mid
            if (screen->manager.u0 == NULL) { // ?, copied original code
                CUIButton& PageUpButton = (CUIButton&) panel12.GetUIControl(0); // PageUp control
                if (PageUpButton.bEnabled)
                    PageUpButton.OnLMouseBtUp(PageUpButton.pos);
            }
        }
    }
}


BOOL __stdcall 
OpenSpellToolbar(CButtonArray* Array, int* nState, int SavedBook) {
    StopAreaIcons();

    gPrev_ToolbarState = *nState;    // for right click info panel
    gPrev_ToolbarBook =  SavedBook;

    if (gSpellMenuEnabled) {   // Click again on spell icon, close menu, restore toolbar
        gSpellMenuEnabled = 0;
        CloseMenu();
        DeleteAreaObjects(NULL);

        //recursive mode
        *nState = gButtonArrayTypePreviousIdx;
        Array->nActiveButtonIdx = 100;  // unselect any button
        return TRUE;
    }

    // original bytes
    gButtorArrayPtrList = CButtonArray_GetData(Array->nItemSlotIdx, 2, 0);

    if (g_pChitin->pGame->m_PartySelection.memberList.GetCount() == 0)
        return FALSE;

    Array->nActiveButtonIdx = 100;     // unselect any button
    if (SavedBook == 0) { // Mage or Priest toolbar
        Array->nButtonIdx[0]  = 100;
        Array->nButtonIdx[1]  = 100;
        Array->nButtonIdx[2]  = 100;
        Array->nButtonIdx[3]  = 100;
        Array->nButtonIdx[4]  = 100;
        Array->nButtonIdx[5]  = 100;
        Array->nButtonIdx[6]  =   3;  // show only spell icon
        Array->nButtonIdx[7]  = 100;
        Array->nButtonIdx[8]  = 100;
        Array->nButtonIdx[9]  = 100;
        Array->nButtonIdx[10] = 100;
        Array->nButtonIdx[11] = 100;
        Array->bToggleButtonCleric = FALSE;
    } else
    if (SavedBook == 1) {               // Mage/Cleric mixed book
        Enum id = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
        CCreatureObject* Cre = NULL;
        char nResult;
        do {
            nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(id, THREAD_ASYNCH, &Cre, INFINITE);
        } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
        if (nResult == OBJECT_SUCCESS) {
            if (Cre->u6417 != 0x55)
                Array->bToggleButtonCleric = TRUE;  // cleric book first
            else
                Array->bToggleButtonCleric = FALSE; // magic book first
            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(id, THREAD_ASYNCH, INFINITE);
        }

        Array->nButtonIdx[0]  = 100;
        Array->nButtonIdx[1]  = 100;
        Array->nButtonIdx[2]  = 100;
        Array->nButtonIdx[3]  = 100;
        Array->nButtonIdx[4]  = 100;
        Array->nButtonIdx[5] = (Array->bToggleButtonCleric) ? 67 : 69;   // book icon
        Array->nButtonIdx[6]  =   3;   // show only spell icon
        Array->nButtonIdx[7]  = 100;
        Array->nButtonIdx[8]  = 100;
        Array->nButtonIdx[9]  = 100;
        Array->nButtonIdx[10] = 100;
        Array->nButtonIdx[11] = 100;
        Array->nIndexMageSpellStart = gButtorArrayPtrList->nIndexMageStart; // enable mage book icon
    }
    Array->PageShifted = 0;
    Array->nButtonArrayTypePreviousIdx = Array->nButtonArrayTypeCurrentIdx;
    Array->nButtonArrayTypeCurrentIdx = 0;
    gButtonArrayTypePreviousIdx = Array->nButtonArrayTypePreviousIdx;
    gRequestedToolbarState = *nState;

    CScreenWorld* screen = g_pChitin->pScreenWorld;
    CPanel& panel4  = screen->manager.GetPanel(4);   // min
    CPanel& panel7  = screen->manager.GetPanel(7);   // max
    CPanel& panel12  = screen->manager.GetPanel(12);  // mid

    if (panel7.bEnabled) {  // max
        gMainWindowSizeMode = 7;
        if (screen->manager.u0 == NULL) { // ?, copied original code
            CUIButton& PageDownButton = (CUIButton&) panel7.GetUIControl(0); // PageDown control
            if (PageDownButton.bEnabled)
                PageDownButton.OnLMouseBtUp(PageDownButton.pos);
        }
        if (panel12.bEnabled) { // max -> mid
            if (screen->manager.u0 == NULL) { // ?, copied original code
                CUIButton& PageDownButton = (CUIButton&) panel12.GetUIControl(3); // PageDown control
                if (PageDownButton.bEnabled)
                    PageDownButton.OnLMouseBtUp(PageDownButton.pos);
            }
        }
    } else
    if (panel12.bEnabled) { // mid
        gMainWindowSizeMode = 12;
        if (screen->manager.u0 == NULL) { // ?, copied original code
            CUIButton& PageDownButton = (CUIButton&) panel12.GetUIControl(3); // PageDown control
            if (PageDownButton.bEnabled)
                PageDownButton.OnLMouseBtUp(PageDownButton.pos);
        }
    } else
    if (panel4.bEnabled) {  // min
        gMainWindowSizeMode = 4;
    }

    // final
    DeleteAreaObjects(NULL);
    if (SavedBook == 0)
        CreateAreaObjects(0);
    else
        CreateAreaObjects(Array->bToggleButtonCleric ? 2 : 1);
    gSpellMenuEnabled = 1;
    gStopAreaObjects = false;

    return FALSE;
}


void static __stdcall
VertSortList_PostDraw(CArea& Area, CVideoMode *pVidMode, int nSurface) {
    char nResult;

    CGameObject* Obj;
    POSITION pos = Area.m_lVertSortFront.GetHeadPosition();
    while (pos) {
        Enum ID = (Enum) Area.m_lVertSortFront.GetNext(pos);
        do {
            nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(ID, THREAD_RENDER, &Obj, INFINITE);
        } while (nResult == OBJECT_DENYING);

        if (nResult == OBJECT_SUCCESS ) {
            if (Obj->nObjType == CGAMEOBJECT_TYPE_ICONGUI)
                Obj->Draw(&Area, pVidMode, nSurface);

            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(ID, THREAD_RENDER, INFINITE);
        }
    }
}


void static __stdcall
CheckForGUIIcon(CArea& Area, CGameObject& Obj, uchar* pStatus, Enum ID) {
    if (*pStatus == OBJECT_SUCCESS) {
        if (Obj.nObjType == CGAMEOBJECT_TYPE_ICONGUI) {
            Area.pGame->m_GameObjectArrayHandler.FreeGameObjectShare(ID, THREAD_RENDER, INFINITE);
            *pStatus = OBJECT_BAD_ENUM;
        }
    }
}


//void static __stdcall
//RenderSpellMenu() {
//    if (g_StopRender)
//        return;
//
//    ResetEvent(gRenderNotBusy);
//        //
//        console.writef("ResetEvent \n");
//        //
//
//    if (!g_SpellMenuEnabled) {
//        SetEvent(gRenderNotBusy);
//            //
//            console.writef("SetEvent not enabled \n");
//            //
//        return;
//    }
//
//    CPtrList* g_pCQuickObjectList = (CPtrList*) *( (DWORD *)0xB83D0C);
//    if (g_pCQuickObjectList == NULL) {
//        SetEvent(gRenderNotBusy);
//        return;
//    }
//    if (g_pCQuickObjectList->GetCount() == 0) {
//        SetEvent(gRenderNotBusy);
//            //
//            console.writef("SetEvent count= 0 \n");
//            //
//        return;
//    }
//
//    CIcon icon;
//    POSITION pos;
//    CQuickObject* pButton;
//    int startX = 80;
//    int startY = yEnd_MinDlg - 9*32 - 10;   // wiz 9 levels
//    //int startY = yEnd_MinDlg - 7*32 - 10;   // priest 9 levels
//    int Line = 0;
//    int Row = 0;
//    int prev_lvl = 1;
//    SIZE   size =           {32,32};    // icon max size
//    RECT   rBoundingBox=    {xStart_MinDlg, yStart_MinDlg, xEnd_MinDlg, yEnd_MinDlg};
//
//    pos = g_pCQuickObjectList->GetHeadPosition();
//    while (pos) {
//        pButton = (CQuickObject*) g_pCQuickObjectList->GetNext(pos);
//
//        ////////////
//        ResSplContainer Spell(pButton->m_ObjectInfo.rSpellName);
//        Spell.Demand();
//        short lvl = Spell.GetSpellLevel();
//        Spell.Release();
//
//        if (lvl > prev_lvl) {
//            Row=0;
//            Line += 32;
//            prev_lvl = lvl;
//        }
//
//        POINT  pt = {startX + Row, startY + Line};
//        icon.RenderIcon(
//                0,
//                pt,
//                size,
//                rBoundingBox,
//                pButton->rUsageIcon,
//                g_pChitin->bDoubleResolutionButtons,
//                0,
//                pButton->wAmount,
//                FALSE,
//                0,
//                FALSE);
//        Row += (32 + 10);
//        ////////////
//    }
//
//        //
//            console.writef("SetEvent normal end \n");
//        //
//    SetEvent(gRenderNotBusy);
//}


//void static __stdcall
//CInfButtonArray_SetState_ClearPrevState(CButtonArray* Array, int state, DWORD EIP) {
//    if (gRenderNotBusy == NULL)   // init
//        gRenderNotBusy = CreateEvent(NULL, TRUE, TRUE, NULL);
//
//    //StopAreaIcons();
//    if (state != 103) {     // not Spell toolbar
//        if (Array->nActiveButtonIdx == 20) {   // clicked on spell icon with enabled spell menu
//            //
//            //console.writef("Toolbar: Clicked spell icon at toolbar \n");
//            //
//            return;
//        } 
//
//        if (gSpellMenuEnabled) {   // Close menu
//            //g_SpellMenuEnabled = 0;
//            //CloseMenu();
//            //DeleteAreaObjects(NULL);
//
//            //
//            //console.writef("Toolbar: Abort spell menu, nActiveButtonIdx= %d \n", Array->nActiveButtonIdx);
//            //
//        }
//    }
//}
//
//
//void static __stdcall
//CInfButtonArray_ResetState_Log(DWORD EIP) {
//    //console.writef("ResetState= %X  \n", EIP);
//}


void static __stdcall
CScreenWorld_OnLButtonUp_CheckClick(CScreenWorld& screen, POINT pt) {
    if (gRenderNotBusy == NULL)   // init
        gRenderNotBusy = CreateEvent(NULL, TRUE, TRUE, NULL);

    BOOL bPrev_SpellMenuEnabled = gSpellMenuEnabled;
    CArea* Area = g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx];
    if ((g_pChitin->pGame->m_GameSave.inputMode & 1) == 0) { // non-Area screen
        screen.OnLButtonUp(pt);
        return;
    }

    if (Area == NULL) {
        screen.OnLButtonUp(pt);
        return;
    }

    RECT prevstate = Area->selectSquare;

    //
    //console.writef("\nPre Area Click g_SpellMenuEnabled= %d x0= %d, y0= %d  \n",
    //                    gSpellMenuEnabled,
    //                    prevstate.left, prevstate.top);
    //

    screen.OnLButtonUp(pt);
    RECT newstate  = Area->selectSquare;

    //
    //console.writef("Area Click g_SpellMenuEnabled= %d x1= %d, y1= %d \n",
    //                    gSpellMenuEnabled,
    //                    newstate.left,  newstate.top);
    //

    if (bPrev_SpellMenuEnabled == FALSE && gSpellMenuEnabled == FALSE)  // no spell click
        return;

    if (bPrev_SpellMenuEnabled == FALSE && gSpellMenuEnabled == TRUE)  // opened
        return;

    if (bPrev_SpellMenuEnabled == TRUE && gSpellMenuEnabled == FALSE)  // normal closed
        return;

    if (gSwitchBookInProgress) {
        gSwitchBookInProgress = false;
        return;                                 // skip this click on toolbar
    }

    if (prevstate.left == -1 || newstate.left == -1) {  // outside click
        if (bPrev_SpellMenuEnabled) {   // Close menu
            StopAreaIcons();
            gSpellMenuEnabled = 0;
            CloseMenu();
            DeleteAreaObjects(NULL);

            CButtonArray& Array = g_pChitin->pGame->m_CButtonArray;
            if (Array.nButtonArrayTypeCurrentIdx == 0) {
                Array.SetState(gButtonArrayTypePreviousIdx);
            }

            //
            //  console.writef("outside click, close menu \n");
            //
        }
    }
}


void static __stdcall
CInfButtonArray_ResetState_CloseMenu() {
    if (gRenderNotBusy == NULL)   // init
        gRenderNotBusy = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (gSpellMenuEnabled) {   // Close menu
        StopAreaIcons();
        gSpellMenuEnabled = 0;
        CloseMenu();
        DeleteAreaObjects(NULL);

        //
        //console.writef("reset state, close menu \n");
        //
    }
}

extern void PlayToggleSound(bool On_Off);

void
ToggleSpellMenu() {
    StopAreaIcons();
    gSpellMenuEnabled = 0;
    CloseMenu();
    DeleteAreaObjects(NULL);

    CButtonArray& Array = g_pChitin->pGame->m_CButtonArray;
    if (Array.nButtonArrayTypeCurrentIdx == 0) {
        Array.SetState(gButtonArrayTypePreviousIdx);
    }

    gEnableSpellMenu = !gEnableSpellMenu;

    if (gEnableSpellMenu)
        PlayToggleSound(true);
    else
        PlayToggleSound(false);
}


void static __stdcall
OnLButtonPressed_SwitchBook(CButtonArray* Array, int ButtonId, int ButtonIndex) {
    if (gSpellMenuEnabled) {
        if ((Array->nButtonIdx[5] == 67 ||      // mage book icon
             Array->nButtonIdx[5] == 69) &&
            ButtonIndex == 5) {                 // book icon clicked
            StopAreaIcons();
            gSpellMenuEnabled = 0;
            DeleteAreaObjects(NULL);

            Array->bToggleButtonCleric = !Array->bToggleButtonCleric;

            Enum id = g_pChitin->pGame->m_PartySelection.GetFirstSelected();
            CCreatureObject* Cre = NULL;
            char nResult;
            do {
                nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(id, THREAD_ASYNCH, &Cre, INFINITE);
            } while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
            if (nResult == OBJECT_SUCCESS) {
                Cre->u6417 = (Array->bToggleButtonCleric) ? 0 : 0x55;   // custom marker
                g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(id, THREAD_ASYNCH, INFINITE);
            }

            Array->nButtonIdx[0]  = 100;
            Array->nButtonIdx[1]  = 100;
            Array->nButtonIdx[2]  = 100;
            Array->nButtonIdx[3]  = 100;
            Array->nButtonIdx[4]  = 100;
            Array->nButtonIdx[5] = (Array->bToggleButtonCleric) ? 67 : 69;   // book type icon
            Array->nButtonIdx[6]  =   3;  // show only spell icon
            Array->nButtonIdx[7]  = 100;
            Array->nButtonIdx[8]  = 100;
            Array->nButtonIdx[9]  = 100;
            Array->nButtonIdx[10] = 100;
            Array->nButtonIdx[11] = 100;
            Array->UpdateButtons();

            CreateAreaObjects(Array->bToggleButtonCleric ? SPELLTYPE_PRIEST : SPELLTYPE_MAGE);
            gSpellMenuEnabled = 1;
            gStopAreaObjects = false;
            gSwitchBookInProgress = true;
        }
    
    }
}


void static __stdcall
CGameArea_AIUpdate_FakeVisibleArea(CArea& Area, BOOL* bInVisibleArea) {
    if (Area.iPicked == gLastButtonIDUnderCursor)
        *bInVisibleArea = TRUE;
}


BOOL __declspec(naked)
SpellIcon::IsPointOverMe(POINT& pt) {
__asm
{
    push    ecx
    push    edx

    push    ecx         // SpellIcon
    call    SpellIcon_IsPointOverMe
    test    eax,eax

    pop     edx
    pop     ecx
    jz      SpellIcon_IsPointOverMe_asm_continue

    cmp     [esp], 04BFD49h     // called from CGameArea::AIUpdate()
    jnz     SpellIcon_IsPointOverMe_asm_continue

    mov     eax, [ebp-24h]      // Obj ID under cursor
    mov     [gLastButtonIDUnderCursor], eax
    add     esp, 8              // ret_eip + arg0
    push    04BFDBAh            // FakeVisibleArea
    ret

SpellIcon_IsPointOverMe_asm_continue:
    ret     4
}
}


void __declspec(naked)
CInfButtonArray_SetState_OpenSpellToolbar_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CInfButtonArray_SetState_OpenSpellToolbar_skip

    push    ecx
    push    edx
    push    eax

    push    0               // SavedBook
    lea     eax, [ebp+8]
    push    eax             // nState    
    push    [ebp-0D4h]      // ButtonArray
    call    OpenSpellToolbar
    test    eax, eax

    pop     eax
    pop     edx
    pop     ecx
    jz      CInfButtonArray_SetState_OpenSpellToolbar_continue_asm

    //Recursive Mode
    add     esp, 4
    push    066211Fh    // skip touching toolbar
    ret

CInfButtonArray_SetState_OpenSpellToolbar_continue_asm:
    add     esp, 4
    push    06644EDh    // skip touching toolbar
    ret

CInfButtonArray_SetState_OpenSpellToolbar_skip:
    add     esp,4

    push    0           // Stolen bytes
    push    2
    mov     ecx, [ebp-0D4h]
    push    0662B00h    // back to orig code
    ret
}
}


void __declspec(naked)
CInfButtonArray_SetState_OpenMixedSpellToolbar_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CInfButtonArray_SetState_OpenMixedSpellToolbar_skip

    push    ecx
    push    edx
    push    eax

    push    1               // SavedBook
    lea     eax, [ebp+8]
    push    eax             // nState    
    push    [ebp-0D4h]      // ButtonArray
    call    OpenSpellToolbar
    test    eax, eax

    pop     eax
    pop     edx
    pop     ecx
    jz      CInfButtonArray_SetState_OpenMixedSpellToolbar_continue_asm

    //Recursive Mode
    add     esp, 4
    push    066211Fh    // skip touching toolbar
    ret

CInfButtonArray_SetState_OpenMixedSpellToolbar_continue_asm:
    add     esp, 4
    push    06644EDh    // skip touching toolbar
    ret

CInfButtonArray_SetState_OpenMixedSpellToolbar_skip:
    add     esp,4

    push    0           // Stolen bytes
    push    2
    mov     edx, [ebp-0D4h]
    push    0662D07h    // back to orig code
    ret
}
}


//void __declspec(naked)
//CInfButtonArray_SetState_ClearPrevState_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//
//    push    [ebp+4]     // EIP
//    push    [ebp+8]     // ButtonArray state
//    push    [ebp-0D4h]  // ButtonArray
//    call    CInfButtonArray_SetState_ClearPrevState
//
//    pop     edx
//    pop     ecx
//    
//    push    06608E9h    // Stolen bytes
//    ret
//
//}
//}


//void __declspec(naked)
//CInfButtonArray_ResetState_Log_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//
//    push    [ebp+4] // EIP
//    call    CInfButtonArray_ResetState_Log
//
//    pop     edx
//    pop     ecx
//    
//    mov     [ebp-4], ecx    // Stolen bytes
//    mov     eax, [ebp-4]
//    ret
//
//}
//}


void __declspec(naked)
CScreenWorld_OnLButtonUp_CheckClick_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CScreenWorld_OnLButtonUp_CheckClick_skip

    push    ecx
    push    edx

    push    [esp+10h]   // point.y
    push    [esp+10h]   // point.x
    push    ecx         // screen
    call    CScreenWorld_OnLButtonUp_CheckClick

    pop     edx
    pop     ecx
    ret     8

CScreenWorld_OnLButtonUp_CheckClick_skip:
    push    07CEF98h    // orig code
    ret
}
}


void __declspec(naked)
CInfButtonArray_ResetState_CloseMenu_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CInfButtonArray_ResetState_CloseMenu_skip

    push    ecx
    push    edx

    call    CInfButtonArray_ResetState_CloseMenu

    pop     edx
    pop     ecx

CInfButtonArray_ResetState_CloseMenu_skip:
    push    06619DFh    // original bytes
    ret     
}
}


void __declspec(naked)
CGameArea_Render_PostDraw_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CGameArea_Render_PostDraw_skip

    push    ecx
    push    edx

    push    [ebp+0Ch]   // nSurface
    push    [ebp+8]     // vidmode
    push    [ebp-0A0h]  // Area
    call    VertSortList_PostDraw

    pop     edx
    pop     ecx

CGameArea_Render_PostDraw_skip:
    mov     ecx, [ebp-0A0h]    // original bytes
    ret     
}
}


void __declspec(naked)
CGameArea_Render_InjectCheck_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CGameArea_Render_InjectCheck_skip

    push    ecx
    push    edx
    push    eax

    push    [ebp-14h]       // id
    lea     eax, [ebp-4]
    push    eax             // status
    push    [ebp-18h]       // Obj
    push    [ebp-0A0h]      // Area
    call    CheckForGUIIcon // overwrite [ebp-4] - new status

    pop     eax
    pop     edx
    pop     ecx

CGameArea_Render_InjectCheck_skip:
    movsx   edx, word ptr [ebp-4]// stolen bytes
    xor     eax, eax
    ret

}
}


void __declspec(naked)
CInfButtonArray_OnLButtonPressed_SwitchBook_asm() {
__asm
{   cmp     [gEnableSpellMenu], 0
    jz      CInfButtonArray_OnLButtonPressed_SwitchBook_skip

    push    ecx
    push    edx
    push    eax

    push    [ebp+8]         // ButtonIndex
    push    [ebp-774h]      // ButtonID
    push    [ebp-744h]      // ButtonArray
    call    OnLButtonPressed_SwitchBook

    pop     eax
    pop     edx
    pop     ecx
    //jmp     CInfButtonArray_OnLButtonPressed_SwitchBook_jmp

CInfButtonArray_OnLButtonPressed_SwitchBook_skip:
    cmp     dword ptr [ebp-774h], 1Ch // Stolen bytes
    ja      CInfButtonArray_OnLButtonPressed_SwitchBook_jmp
    ret

CInfButtonArray_OnLButtonPressed_SwitchBook_jmp:
    add     esp, 4
    push    0670C1Eh    // back to orig code
    ret
}
}


void __declspec(naked)
CGameArea_AIUpdate_FakeVisibleArea_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    lea     eax, [ebp-5Ch]  // bInVisibleArea
    push    eax
    push    [ebp-3CCh]      // Area
    call    CGameArea_AIUpdate_FakeVisibleArea

    pop     eax
    pop     edx
    pop     ecx
 
    mov     eax, [ebp-3CCh]    // orig bytes
    ret
}
}


void  __declspec(naked)
CGameArea_Marshal_Skip1_asm() {
__asm {
    cmp     eax, CGAMEOBJECT_TYPE_ICONGUI
    jnz     exit_CGameArea_Marshal_Skip1_asm

    add     esp, 4          // kill ret_addr
    push    04C6469h        // skip error
    ret

exit_CGameArea_Marshal_Skip1_asm:
    mov     eax, 1          // stolen bytes
    ret
}
}