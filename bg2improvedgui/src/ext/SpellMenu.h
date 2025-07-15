#include "chitin.h"
#include "utils.h"

class CIcon {
public:
    void RenderIcon(
        int     argX,
        POINT&  pos,
        SIZE&   size,
        RECT&   rBoundingBox,
        ResRef& ResIcon,
        BOOL    bDoubleResolutionButtons,
        ulong   dwFlags,
        ushort  wCount,
        BOOL    bForceCount,
        ushort  wSecondCount,
        BOOL    bForceSecondCount );
};

class SpellIcon : public CGameObject {
public:
    SpellIcon();
    virtual ~SpellIcon();

    virtual uchar   GetObjectType();                            //v4  CGameObject::GetObjectType
    virtual void    AddToArea(CArea *, POINT *, long, uchar);   //v8  CGameObject::AddToArea
    virtual void    AIUpdate();                                 //vc
    virtual Object& GetCurrentObject();                         //v10 CGameObject::GetCurrentObject
	virtual long    GetTargetId();                              //v14 CGameObject::GetTargetId
	virtual void    GetNextWaypoint(POINT*);                    //v18 CGameObject::GetNextWaypoint
	virtual POSITION* GetVertListPos();                         //v1c CGameObject::GetVerticalListPos
	virtual uchar   GetVertListType();                          //v20 CGameObject::GetVertListType
	virtual bool    CanSaveGame(STRREF*, int, int);             //v24 CGameObject::CanSaveGame
	virtual bool    CompressTime(unsigned long);                //v28 CGameAIBase::CompressTime
	virtual void    DebugDump(IECString&, bool);                //v2c CGameObject::DebugDump
    virtual BOOL    IsPointOverMe(POINT& pt);                   //v30
    virtual BOOL    DoesIntersect(RECT r);                      //v34 CGameObject::DoesIntersect 
	virtual BOOL    OnSearchMap();                              //v38 CGameObject::OnSearchMap
    virtual void    OnActionButton(POINT *pt);                  //v3c
    virtual void    OnFormationButton(POINT *pt);               //v40 MouseRClick
    virtual void    RemoveFromArea();                           //v44
    virtual void    Draw(CArea *pArea, CVideoMode *pVidMode, int nSurface);//v48
    virtual bool    DoAIUpdate(bool bRun, long nChitinUpdates); //v4c
    virtual bool    SetObject(Object&, int);                    //v50 CGameObject::SetAIType
    virtual void    SetCursor(long nToolTip);                   //v54
    virtual void    SetTarget(POINT target, int);               //v58 CGameObject::SetTarget
	virtual void    SetVertListPos(POSITION*);                  //v5c CGameObject::SetVerticalListPos
	virtual BOOL    EvaluateStatusTrigger(Trigger&);            //v60 CGameObject::EvaluateStatusTrigger

    RECT            rect;
    CButtonData     SpellButton;
    bool            bNumberIcon;
    bool            bDisabled;
};

void CloseMenu();
void DeleteAreaObjects(SpellIcon*);

void CGameArea_Render_ShowSpellMenu_asm();
void CInfButtonArray_SetState_OpenSpellToolbar_asm();
void CInfButtonArray_SetState_OpenMixedSpellToolbar_asm();
void CInfButtonArray_SetState_ClearPrevState_asm();
void CInfButtonArray_ResetState_Log_asm();
void CScreenWorld_OnLButtonUp_CheckClick_asm();
void CInfButtonArray_ResetState_CloseMenu_asm();
void CGameArea_Render_PostDraw_asm();
void CGameArea_Render_InjectCheck_asm();
void CInfButtonArray_OnLButtonPressed_SwitchBook_asm();
void CGameArea_AIUpdate_FakeVisibleArea_asm();
void CGameArea_Marshal_Skip1_asm();
void CGameArea_Marshal_Skip2_asm();
void CGameArea_Marshal_Skip3_asm();