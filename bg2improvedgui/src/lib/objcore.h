#ifndef OBJCORE_H
#define OBJCORE_H

#include "stdafx.h"
#include "scrcore.h"
#include "arecore.h"
#include "sndcore.h"
#include "objstats.h"

//CGameObject types
//constants from AAA9E1-AAA9ED, having a 1 means scriptable

//CVisualEffectVidCell (0h)
//CGameObject (0h)
//CGameAIBase (1h)
//CSoundObject (10h) (list 1)
//CGameContainer (11h) (list 1)
//CSpawningObject (20h) (list 1)
//CDoorObject (21h)
//CStaticObject (30h) (list 1)
//CCreatureObject (31h) (list 1, if dead?)
//C5EObject (40h)
//CTriggerObject (41h) (list 1)
//CTiledObject (51h) - should be 50h?
//CSmokeObject (60h)
//CAreaObject (61h) -0x3D8, AA6B74 (list 1)
//CBaldurObject (71h) - size: 0x3D8, AA6C28
#define CGAMEOBJECT_TYPE_OBJECT			0x00
#define CGAMEOBJECT_TYPE_SPRITE			0x01
#define CGAMEOBJECT_TYPE_SOUND			0x10
#define CGAMEOBJECT_TYPE_CONTAINER		0x11
#define CGAMEOBJECT_TYPE_SPAWNING		0x20
#define CGAMEOBJECT_TYPE_DOOR			0x21
#define CGAMEOBJECT_TYPE_STATIC			0x30
#define CGAMEOBJECT_TYPE_CREATURE		0x31
#define CGAMEOBJECT_TYPE_OBJECTMARKER	0x40
#define CGAMEOBJECT_TYPE_TRIGGER		0x41
#define CGAMEOBJECT_TYPE_PROJECTILE		0x50
#define CGAMEOBJECT_TYPE_TILE			0x51
#define CGAMEOBJECT_TYPE_SMOKE			0x60
#define CGAMEOBJECT_TYPE_AREA			0x61
#define CGAMEOBJECT_TYPE_BALDUR			0x71
#define CGAMEOBJECT_TYPE_ICONGUI	    0x80 // Tobex new, not AIBase type


//CGameObjectArrayHandler thread indices
const char THREAD_ASYNCH	    = 0;
const char THREAD_RENDER	    = 1;
const char THREAD_SEARCHPATH	= 2;

//CGameObjectArrayHandler return values (0xAAD1C4)
const char OBJECT_SUCCESS		= 0;
const char OBJECT_LOCK_FAILED	= 1;
const char OBJECT_DELETED		= 2; //Index.id does not match Enum.id
const char OBJECT_BAD_ENUM		= 3;
const char OBJECT_SHARING		= 4;
const char OBJECT_DENYING		= 5;

typedef IECPtrList CTimerList; //AA682C

struct CTimeElement {
//For CTimerList
	int  nTime; //0h
	char nId; //4h
	char u5; //pad

};

class CGameSpriteLastUpdate { //Size 50h
//Related to current area sprite is in
//Constructor: 0x008DF1F2, also 0x008DF105
public:
	long* pPath;            //0h, IECString
	short nPath;            //4h
    short currPath;         //6h
	POINT ptDest;           //8h
	uchar nMoveScale;       //10h
	char  u11;
	short nSequence;        //12h  
	POINT ptPosition;       //14h, current position?
	short nFacing;          //1c;
	unsigned long dwState;  //1eh
	short nHitPoints;       //22h
    short nMaxHitPoints;
    short nArmorClass;
    short nACCrushingMod;
    short nACMissileMod;
    short nACPiercingMod;
    short nACSlashingMod;
    short nPortraitIcons;
	long* pPortraitIcons;    //32h
    uchar nEnemyAlly;        //36h
    uchar nEnemyAllyLive;
    uchar bMoraleFailure;
    uchar nGeneral;
    uchar nGeneralLive;
	char  u3b;
	IECString sAreaString;   //3ch current area name?
	bool  bLevelUp;          //40h
	char  u41;
	long  nHPBonusPrev;      //42h, nHPCONBonusTotalOld
	bool  bSummonDisable;    //46h
    bool  bDoNotJump;        //47h
    bool  bSanctuary;        //48h
    bool  bDisableCircle;    //49h
    bool  bHeld;             //4ah
    bool  bFree;             //4bh, bActiveImprisonment
	BOOL  bFullUpdateRequired;//4ch
};

struct CEventMessage { //Size 1Ah
	short     wEventId;
	int       nParam1;
	int       nParam2;
	int       nParam3;
	STRREF    strrefParam4;
	BOOL      bParam5;
	IECString sParam6;
};

class CEventMessageList : public IECPtrList { //Size 1Ch
//Constructor: 0x55E590
public:
	//AA9D28
};

#pragma warning(push)           // for CGameObject and CGameAIBase
#pragma warning(disable:4716)   // function must return a value

class CGameObject { //Size 42h
//Constructor: 0x573470
public:
	//AA6844
    CGameObject();
	virtual ~CGameObject() {} //v0
	virtual uchar       GetObjectType() {}              //v4
	virtual void        AddToArea(CArea *pNewArea, POINT *pt, long zPos, uchar cVertListType) {} //v8
	virtual void        AIUpdate() {}                   //vc
	virtual Object&     GetCurrentObject() {}           //v10 return o;
	virtual long        GetTargetId() {}                //v14 (dw 3738h) - get some enum?
	virtual void        GetNextWaypoint(POINT *pt) {}   //v18 GetCurrentPoint(POINT* ptr)
	virtual POSITION*   GetVertListPos() {}             //v1c , gets 16h
	virtual uchar       GetVertListType() {}            //v20, returns 1ah
	virtual bool        CanSaveGame(STRREF *strError, int restCheck, int combatCheck) {} //v24
	virtual bool        CompressTime(unsigned long deltaTime) {} //v28
	virtual void        DebugDump(IECString& message, bool bEchoToScreen) {} //v2c
	virtual BOOL        IsPointOverMe(POINT& pt) {}     //v30, IsOver()
	virtual BOOL        DoesIntersect(RECT r) {}        //v34, OBJ_as_ARG
	virtual BOOL        OnSearchMap() {}                //v38, InAnArea()
	virtual void        OnActionButton(POINT *pt) {}    //v3c ? to do with modal states
	virtual void        OnFormationButton(POINT *pt) {} //v40 PlaySound? to do with modal states
	virtual void        RemoveFromArea() {}             //v44
	virtual void        Draw(CArea *pArea, CVideoMode *pVidMode, int nSurface) {} //v48, Render()
    virtual bool        DoAIUpdate(bool bRun, long nChitinUpdates) {}             //v4c
    virtual bool        SetObject(Object& o, int dummy) {}  // v50, SetAIType()
    virtual void        SetCursor(long nToolTip) {}         //v54
	virtual void        SetTarget(POINT target, int collisionPath) {} //v58, OBJ_as_ARG
	virtual void        SetVertListPos(POSITION *posVertList) {}      //v5c sets 16h
	virtual BOOL        EvaluateStatusTrigger(Trigger& inTrigger) {}  //v60 EvaluateTrigger()

	Enum    GetEnum();
    void    RemoveFromArea_(); // non-virtual access to RemoveFromArea()

    uchar       nObjType;         //4h, m_objectType, CGAMEOBJECT_TYPE_* type
	char        u5;
	POINT       currentLoc;       //6h,  m_pos
	long        zPos;             //eh,  m_posZ
	CArea*      pArea;            //12h, 
	POSITION*   posVertList;      //16h, ref to pos of vertical list with this enum
	uchar       vertListType;     //1ah, m_listType, which area vertical this is part of
	char        u1b;
	Object      o;                //1ch, m_typeAI, this o (main Object used in script triggers)
	Enum        id;               //30h, e
	short       canBeSeen;        //34h
	int         nPlayerNetworkId; //36h, m_remotePlayerID
	Enum        nObjectNetworkId; //3ah, m_remoteObjectID
	
	//which modulus of the tick depends on nAIUpdateInterval & e == nAIUpdateInterval & nChitinUpdates
	//0: every tick
	//1: every 2nd tick (default)
	//3: every 4th tick
	uchar       nAIUpdateInterval;      //3eh, m_AISpeed

	bool        bIgnoreMessagesToSelf;  //3fh, m_bLocalControl, network
	char        AIInhibitor;            //40h, increment modulo of 3 (range: 0-2), set to 1 when there are no triggers
	char        u41;
};

extern Enum (CGameObject::*CGameObject_GetEnum)();

class CGameAIBase : public CGameObject { //Size 3D8h
//Constructor: 0x476DED
public:
     //AA6778
     CGameAIBase();

     // override
     virtual ~CGameAIBase() {}                                         //v0
     virtual bool          CompressTime(unsigned long deltaTime) {}    //v28
     virtual void          RemoveFromArea() {}                         //v44
     virtual BOOL          EvaluateStatusTrigger(Trigger& inTrigger);  //v60

     // new
     virtual void          ClearAllActions(BOOL bLeaveOverrides);   //v64, ClearActions()
     virtual void          UpdateTarget(CGameObject *target) {}     //v68, void SetTarget(creTarget)
     virtual void          AddActionHead(Action& action) {}         //v6c, AddAction()
     virtual void          AddEffect(CEffect& eff, uchar list, int noSave, int immediateApply) {} //v70 bool bCheckEffectListType, BOOL bDelayFinished, BOOL bUpdateCre
     virtual void          ClearAI(bool setSequence) {}             //v74, update for new round? (BOOL) calls v64, v88
     virtual void          ExecuteOneAction() {}                    //v78, DoAction() calls v7c
     virtual ACTIONRESULT  ExecuteAction();                         //v7c
     virtual void          AddActionTail(Action& a) {}              //v80, InsertAction()
     virtual void          ProcessAI() {}                           //v84, called from AIUpdate
     virtual void          SetCurrentAction(Action& action);        //v88, calls vb0
     virtual void          SetScript(short level, CScript& script) {} //v8c
     virtual short         GetVisualRange() {}                      //v90, returned in pixels
     virtual TerrainTable* GetVisibleTerrainTable() {}              //v94, TerrainTable& GetTerrainTable2()
     virtual TerrainTable* GetTerrainTable() {}                     //v98, TerrainTable& GetTerrainTable()
     virtual int           QuickDecode(Trigger& trigger, CCreatureObject** ppCre) {} //v9c
     virtual short         GetHelpRange() {}                        //va0, m_SightRadius x 48
     virtual void          ApplyTriggers() {}                       //va4
     virtual void          SetAutoPauseInfo(unsigned long nType) {} //va8, AutoPause()
     virtual BOOL          CanSeeInvisible() {}                     //vac, GetCanSeeInvisible BOOL GetSeeInvisible(), returns TRUE if cdsCurrent->seeInivisible is set, or game is in CutSceneMode
     virtual void          OnActionRemoval(Action& curAction) {}    //vb0 void DoPostAction(Action& a), for every ACTIONRESULT except no action taken

     Action& GetTopAction(Action* pAction);
     void    QueueActions(Response& r, BOOL bSkipIfAlreadyQueued, BOOL bClearActionQueue);
		
	/* va8, void SetAutoPauseInfo(int type)
	0x1 WEAPON_UNUSABLE
	0x2 ATTACKED
	0x4 HIT
	0x8 BADLY_WOUNDED
	0x10 UNKNOWN_TRIGGER
	0x20 TARGET_GONE
	0x40 ROUND_END
	0x80 ENEMY_SIGHTED
	0x100 SCRIPTED
	0x200 SPELL_CAST
	0x400 TRAP_FOUND
	default UNKNOWN_REASON */

	Object oAttacker;       //42h, m_lAttacker, for AttackedBy() trigger, for LastAttackerOf triggerid
	long   nAttackedType;   //56h, ASTYLE.IDS from AttackedBy() , DAMAGES.IDS from HitBy()
	Object oCommander;      //5ah, for ReceivedOrder() trigger, for LastCommandedBy triggerid
	Object oProtector;      //6eh, for ProtectedBy triggerid
	Object oProtectee;      //82h, for ProtectorOf triggerid
	Object oTargetor;       //96h, for LastTargetedBy triggerid
	Object oHitter;         //aah, for HitBy() trigger, for LastHitter triggerid
	Object oHelpShouter;    //beh, for Help() trigger, for LastHelp triggerid
	Object oTriggerer;      //d2h, for trigger in SVTRIOBJ.IDS, for LastTrigger triggerid
	Object oSeeer;          //e6h, for LastSeenBy triggerid
	Object oTalker;         //fah, for Said() trigger, for LastTalkedToBy triggerid
	Object oShouter;        //10eh, for Heard() trigger, for LastHeardBy triggerid
	Object oSummoned;       //122h, for Summoned() trigger, for LastSummonerOf triggerid
	Object u136;
	Object u14a;
	Object u15e;
	Object u172;
	Object u186;
	Object u19a;
	Object u1ae;
	Object u1c2;
	Object u1d6;
	Object u1ea;
	Object u1fe;
	Object u212;
	Object u226;
	CScript* pScriptOverride;     //23ah
	CScript* pScriptArea;         //23eh
	CScript* pScriptSpecific;     //242h
	CScript* pScriptClass;        //246h
	CScript* pScriptRace;         //24ah
	CScript* pScriptGeneral;      //24eh
	CScript* pScriptDefault;      //252h
	CActionList  queuedActions;   //256h, checked in ActionListEmpty()
	CTriggerList pendingTriggers; //272h
	//Triggers 0x0*** are checked here with == only
	//Triggers 0x4*** are checked more sophisticatedly

	ulong       PAICallCounter;      //28eh, nTimeFree, countup, time free (i.e. not mazed/imprisoned)
	ulong       PAICallCounterNoMod; //292h, countup timer
	CTimerList  timers;              //296h
	short       nCurrResponseIdx;    //2b2h, gets Response.u2
	short       nCurrScriptBlockIdx; //2b4h, gets Response.u4
	short       nCurrScriptIdx;      //2b6h, gets Response.u6
	BOOL        bInterrupt;          //2b8h, bUseCurrScriptIdx use above three values?
	short       wActionTicksElapsed; //2bch, m_actionCount, how many updates has the current action been running for
	Action      currentAction;       //2beh
	long        nExpectedProcessPendingTriggersCalls; //31ch, used with Delay() trigger
	short       nMissedProcessPendingTriggerCalls;    //320h, random number, 0-15, used with Delay() trigger
	short       nAlertnessPeriod;    //322h
	char        szScriptName[32];    //324h, script name/death var
	BOOL        bInCutScene;         //344h, bEmptyActionList?
	BOOL        bFirstCall;          //348h
	int         forceActionPick;     //34ch
	long        randValue;           //350h random number 0-32767
	uchar       nReactionBase;       //354h m_reactionRoll
	char u355;
	ACTIONRESULT nLastActionReturn;  //356h, arCurrent, the return value of ExecuteAction()
	uchar        nVisualRange;       //358h
	char u359;
	Enum        eGameTextOverHead;   //35ah
	BOOL        bInActionExecution;  //35eh, set during the ExecuteAction() function
	BOOL        bNewTrigger;         //362h  has stored triggers?
	BOOL        bNoInterrupt;        //366h, bInterruptState, SetInterrupt() for sprite
	CSound      forcePauseSound;     //36ah
    long        nServerLastObjectSynchDelay;//3D4h, missed in TobEx v26
};

#pragma warning(pop)

extern BOOL         (CGameAIBase::*CGameAIBase_EvaluateStatusTrigger)(Trigger&);
extern void         (CGameAIBase::*CGameAIBase_ClearAllActions)(BOOL);
extern ACTIONRESULT (CGameAIBase::*CGameAIBase_ExecuteAction)();
extern void         (CGameAIBase::*CGameAIBase_SetCurrentAction)(Action&);
extern Action&      (CGameAIBase::*CCGameAIBase_GetTopAction)(Action*);
extern void         (CGameAIBase::*CCGameAIBase_QueueActions)(Response&, BOOL, BOOL);

struct CGameObjectEntry {     //Size Ch
public:
	char    bShareCounts[3];  //0h
	char    bDenyCounts[3];   //3h
	short   id;               //6h, low short
	CGameObject* pGameObject; //8h
};

struct CGameObjectArrayHandler { //Size 2Eh
//Constructor: 0x675FB0
	char GetGameObjectShare(Enum e, char threadNum, void* pptr, int dwTimeout);
	//char GetGameObject(Enum e, char threadNum, void* pptr, int dwTimeout);
	char GetGameObjectDeny(Enum e, char threadNum, void* pptr, int dwTimeout);
	char FreeGameObjectShare(Enum e, char threadNum, int dwTimeout);
	char FreeGameObjectDeny(Enum e, char threadNum, int dwTimeout);
    char Add(Enum *id, CGameObject *ptr, DWORD dwTimeOut);
    char Delete(Enum id, char THREAD_MODE, int *pGameObject, DWORD dwTimeOut);

#ifdef _DEBUG
	_CCriticalSection ccs; //0h
#else
	CCriticalSection  ccs; //0h
#endif
	CGameObjectEntry *pArray; //20h
	short nArrayCurrentSize;  //24h
	short nArrayGrowSize;     //26h
	short nMaxArrayIndex;     //28h, enum1 total, high short
	short nMaxObjectId;       //2ah, enum2 total, low short
	short deletedListHead;    //2ch, nIdxLink, nRemoteIdx?
};

struct CGameRemoteObjectArrayHandler { //Size 90h
//Constructor: 0x677B5D
	int*    dwArray;     //0h
	short   dwArraySize; //4h, numEntries
	short   u6;
	int     u8;
	int     uc;
	short   u10;
	int     u12;
	int     u16;
	short   u1a;
	ResRef  u1c[6];
	int     u4c[6];
	short   u64[6];
#ifdef _DEBUG
	_CCriticalSection ccs; //70h
#else
	CCriticalSection ccs;  //70h
#endif
};

extern char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObjectShare)(Enum, char, void*, int);
extern char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObject)(Enum, char, void*, int);
extern char (CGameObjectArrayHandler::*CGameObjectArrayHandler_GetGameObjectDeny)(Enum, char, void*, int);
extern char (CGameObjectArrayHandler::*CGameObjectArrayHandler_FreeGameObjectShare)(Enum, char, int);
extern char (CGameObjectArrayHandler::*CGameObjectArrayHandler_FreeGameObjectShare)(Enum, char, int);

#endif //OBJCORE_H