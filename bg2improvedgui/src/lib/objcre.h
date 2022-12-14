#ifndef OBJCRE_H
#define OBJCRE_H

#include "stdafx.h"
#include "datatypes.h"
#include "effcore.h"
#include "splcore.h"
#include "objcore.h"
#include "scrcore.h"
#include "itmcore.h"
#include "sndcore.h"
#include "vidcore.h"
#include "pathfind.h"
#include "objproj.h"
#include "splcore.h"
#include "dlgcore.h"
#include "animcore.h"
#include "tlkcore.h"
#include "uicore.h"
#include "particle.h"

//Event message ids
#define EVENTMESSAGE_BACKSTAB_SUCCESS			0x01
#define EVENTMESSAGE_BACKSTAB_WEAPON_UNSUITABLE	0x1C
#define EVENTMESSAGE_PICKPOCKET_DISABLED_ARMOR	0x1E
#define EVENTMESSAGE_PICKPOCKET_FAILED_HOSTILE	0x1F
#define EVENTMESSAGE_PICKPOCKET_FAILED			0x20
#define EVENTMESSAGE_PICKPOCKET_NO_ITEMS		0x21
#define EVENTMESSAGE_PICKPOCKET_INV_FULL		0x22
#define EVENTMESSAGE_PICKPOCKET_SUCCESS			0x23
#define EVENTMESSAGE_SPELLFAILED_INTERRUPT      0x3F
#define EVENTMESSAGE_BACKSTAB_FAIL				0x40
#define EVENTMESSAGE_SPELLFAILED_INVISIBLE		0x42


class CGameButtonList : public IECPtrList { //Size 20h
//Constructor: 0x55ECC0
public:
	//AA99DC
	int nIndexMageStart;        //1ch, index at which wizard spells start (since order is priest, mage, innate)
};

struct CButtonData { //Size 30h
//Constructor: 0x532F23
//Constructor: 0x549D30
	ResRef  rUsageIcon;         //0h
	STRREF  strrefName;         //8h
	ResRef  rLauncherIcon;      //ñh
	STRREF  strrefLauncherName; //14h
	short   wAmount;            //18h
	struct CAbilityId {         //1ah, Size 14h
		short   wType;          //0h, 1ah, 1 = SPL, 2 = ITM, 3 = cast spell from list
		short   wItemSlotIdx;   //2h, 1ch
		short   wItemAbilityIdx;//4h, 1eh
		ResRef  rSpellName;     //6h, 20h
		char    nTargetType;    //eh, 28h
		char    nTargets;       //fh, 29h
		STRREF  strrefTooltip;  //10h, 2ah
	}       abilityId;
	bool    bDisabled;          //2eh
	bool    bDisplayCount;      //2fh, bConstructed
};

struct CFavorite {              //Size Eh
//Constructor: 0x579E70
	unsigned int*   vtable;     //0h
	ResRef          name;       //4h
	short           nTimesUsed; //ch
};

struct CProtectedSpl {          //Size 1Ch
	int          nLevel;        //0h, spl level subtracted from amount of immunity/bounces left
	int          nOpcode;       //4h, opcode of the protection effect
	CProjectile* pProj;         //8h
	BOOL         bNoProjectile; //ch, do not make a bounce projectile
	STRREF       strref;        //10h
	BOOL         bStrRefOnly;   //14h, ignores everything except printing a string to console
	BOOL         bSpellTrap;    //18h, applies Restore Lost Spells eff on nPower
};

class CProtectedSplList : public IECPtrList {
public:
	void AddTail(CEffect& effect, int nPower, int nOpcode, CCreatureObject& cre, BOOL bCreateProj, STRREF strref, BOOL bDoNotUpdateEff, BOOL bRestoreLostSpls);
	void Update(CCreatureObject& cre);
};

extern void (CProtectedSplList::*CProtectedSplList_AddTail)(CEffect&, int, int, CCreatureObject&, BOOL, STRREF, BOOL, BOOL);
extern void (CProtectedSplList::*CProtectedSplList_Update)(CCreatureObject&);

class CCreatureObject : public CGameAIBase { //Size 6774h
//Constructor: 0x87FB08
public:
	//AA98A8
    // override
	virtual ~CCreatureObject() {} //v0
	virtual bool         DoAIUpdate(bool bRun, long nChitinUpdates); //v4c
	virtual BOOL         EvaluateStatusTrigger(Trigger& inTrigger);  //v60
	virtual ACTIONRESULT ExecuteAction();                            //v7c
	virtual void         SetCurrentAction(Action& action);           //v88, calls vb0
	//virtual void         SetAutoPauseInfo(unsigned long nType) {}    //va8
	//virtual BOOL         CanSeeInvisible() {}                        //vac

    // new
	//vb8, void CCreatureObject::SetObjects(o, bSetDerived, bSetBase)
	//vc0, void RefreshObjects(), 1c = 3698 (identical), 3684 = 3698 (except general)
	//vc4, void ProcessScriptsOnce(BOOL bOverrideOnly)
	//...
	//etc. to v114

	CCreatureObject(void* pFile, unsigned int dwSize, BOOL bHasSpawned, int nTicksTillRemove, int nMaxMvtDistance, int nMaxMvtDistanceToObject, unsigned int nSchedule, int nDestX, int nDestY, int nFacing);
	CCreatureObject& Construct(void* pFile, unsigned int dwSize, BOOL bHasSpawned, int nTicksTillRemove, int nMaxMvtDistance, int nMaxMvtDistanceToObject, unsigned int nSchedule, int nDestX, int nDestY, int nFacing) { return *this; } //dummy

    CGameObject&    SetTarget(Object& o, char type);
	void            GetSpellIdsName(int nSpellIdx, IECString* ptr);
	CDerivedStats&  GetDerivedStats();
	ACTIONRESULT    CastSpell(ResRef& rResource, CGameObject& cgoTarget, BOOL bPrintStrref, STRREF strref, void* pMod, BOOL bPrintEventMsg, BOOL bDoNotApplySplAbil);
	static void     RemoveItem(CCreatureObject& cre, int nSlot);
	CEffectList&    GetEquippedEffectsList();
	CEffectList&    GetMainEffectsList();
	void            CreateGore(int dwUnused, short wOrient, short wType);
	void            UpdateHPStatusTooltip(CUIControl& control);
	short           GetOrientationTo(POINT& pt);
	static short    CalculateOrientation(POINT& pt1, POINT& pt2);
	void            SetAnimationSequence(short wSeq);
	void            StartSpriteEffect(char nEffectType, char nParticleType, int nParticles);
	CItem*          GetFirstEquippedLauncherOfAbility(ItmFileAbility* ability, short& launcherSlot);
	int             GetSlotOfEquippedLauncherOfAmmo(short wAmmoSlot, short wAbilityIdx);
	void            UnequipAll(BOOL bKeepEffects);
	void            EquipAll(BOOL bDoNotApplyEffects);
	void            AddKnownSpell(ResRef& name, BOOL bPrintEventMessage);
	CreFileKnownSpell*  GetKnownSpellPriest(int nLevel, int nIndex);
	CreFileKnownSpell*  GetKnownSpellMage(int nLevel, int nIndex);
	CreFileKnownSpell*  GetKnownSpellInnate(int nLevel, int nIndex);
	CreFileMemSpell*    GetMemSpellPriest(int nLevel, int nIndex);
	CreFileMemSpell*    GetMemSpellMage(int nLevel, int nIndex);
	CreFileMemSpell*    GetMemSpellInnate(int nLevel, int nIndex);
	BOOL            AddMemSpellPriest(int nLevel, int nIndex, int* pIndex);
	BOOL            AddMemSpellMage(int nLevel, int nIndex, int* pIndex);
	BOOL            AddMemSpellInnate(int nLevel, int nIndex, int* pIndex);
	BOOL            AddKnownSpellPriest(ResRef& name, int nLevel);
	BOOL            AddKnownSpellMage(ResRef& name, int nLevel);
	BOOL            AddKnownSpellInnate(ResRef& name, int nLevel);
	void            ApplyClassAbilities(CDerivedStats& cdsOld, BOOL bPrintMsgForSpecAbil);
	void            RemoveNewSpecialAbilities(CDerivedStats& cdsTarget);
	IECString&      GetLongName();
	STRREF          GetLongNameStrRef();
	void            SetSpellMemorizedState(ResSplContainer& resSpell, BOOL bState);
	void            ValidateAttackSequence(char* pSeq);
	char            SorcererSpellCount(int nLevel, ResRef rTemp);
	BOOL            InDialogAction();
	void            SetSaveName(ResRef& rName);
	unsigned int    GetKitUnusableFlag();
	void            PrintEventMessage(short wEventId, int nParam1, int nParam2, int nParam3, STRREF strrefParam4, BOOL bParam5, IECString& sParam6);
	short           GetProficiencyInItem(CItem& itm);
	ACTIONRESULT    ActionMoveToObject(CGameObject& cgoTarget);
	ACTIONRESULT    ActionPickPockets(CCreatureObject& creTarget);
	short           GetProficiency(int nWeapProfId);
	short           GetSpellCastingLevel(ResSplContainer& resSpell, BOOL bUseWildMagicMod);
	ACTIONRESULT    ActionJumpToAreaEntranceMove(IECString sArea);
	void            UpdateFaceTalkerTimer();
    void            DropPath();
    void            RemoveAllSpellsPriest();
    int             GetSkillValue(unsigned char index, unsigned char otherClass, int bBaseValue);
    void            AddClassAbilities(unsigned char nClass, short numLevels, int bDisplayFeedback);
    void            CheckQuickLists(CAbilityId *ab, short changeAmount, int remove, int removeSpellIfZero);
    void            SorcererSpellDecrement(int nLevel, ResRef *res, int ignoreProvidedRes);
    void            RemoveClassAbilities(unsigned char nClass, short numLevels);
    void            SetModalState(unsigned char modalState, BOOL bUpdateToolbar);
    void            RemoveSpecialAbility(ResRef& cResInnateSpell);
    void            DisplayTextRef(STRREF nameRef, STRREF text, uint nameColor, uint textColor);
    ushort          GetNumSounds(ushort nOffset, ushort nMaxNum);
    uchar           GetCharacterPortraitNumChannel();
    BOOL            Animate();

	ResRef          rSaveName;              //3d8h, CRE name with * prefix
	unsigned short  type;                   //3e0h, bHasSpawned
	unsigned long   nTicksTillRemove;       //3e2h, m_expirationTime, Arg4
	unsigned short  huntingRange;           //3e6h, nMaxMvtDistance, Arg5, Actor3C
	unsigned short  followRange;            //3e8h, nMaxMvtDistanceToObject, Arg6, Actor3E
	POINT           posStart;               //3eah, int destX + int destY; //3eeh
	unsigned long   timeOfDayVisible;       //3f2h, creature schedule
	CreFileData     BaseStats;              //3f6h
	CKnownSpellList KnownSpellsPriest[7];                //65eh
	CKnownSpellList KnownSpellsWizard[9];                //722h
	CKnownSpellList KnownSpellsInnate[1];                //81eh
	CreFileMemorizedSpellLevel* MemorizedLevelPriest[7]; //83ah
	CreFileMemorizedSpellLevel* MemorizedLevelWizard[9]; //856h
	CreFileMemorizedSpellLevel* MemorizedLevelInnate[1]; //87ah
	CMemorizedSpellList MemorizedSpellsPriest[7];        //87eh
	CMemorizedSpellList MemorizedSpellsWizard[9];        //942h
	CMemorizedSpellList MemorizedSpellsInnate[1];        //a3eh

	CCreInventory   Inventory;          //a5ah, m_equipment
	/*CItem* m_pItemAmulet; //a5ah
	CItem* m_pItemArmor;    //a5eh
	CItem* m_pItemBelt;     //a62h
	CItem* m_pItemBoots;    //a66h
	CItem* m_pItemCloak;    //a6ah
	CItem* m_pItemGauntlets;//a6eh
	CItem* m_pItemHelmet;   //a72h
	CItem* m_pItemRingLeft; //a76h
	CItem* m_pItemRingRight;//a7ah
	CItem* m_pItemShield;   //a7eh
	CItem* m_pItemFist;     //a82h
	CItem* m_pItemAmmo0;    //a86h
	CItem* m_pItemAmmo1;    //a8ah
	CItem* m_pItemAmmo2;    //a8eh
	CItem* m_pItemAmmo3;    //a92h
	CItem* m_pItemMisc0;    //a96h, quick item 0
	CItem* m_pItemMisc1;    //a9ah, quick item 1
	CItem* m_pItemMisc2;    //a9eh, quick item 2
	CItem* m_pItemMisc3;    //aa2h
	CItem* m_pItemMisc4;    //aa6h
	CItem* m_pItemMisc5;    //aaah
	CItem* m_pItemMisc6;    //aaeh
	CItem* m_pItemMisc7;    //ab2h
	CItem* m_pItemMisc8;    //ab6h
	CItem* m_pItemMisc9;    //abah
	CItem* m_pItemMisc10;   //abeh
	CItem* m_pItemMisc11;   //ac2h
	CItem* m_pItemMisc12;   //ac6h
	CItem* m_pItemMisc13;   //acah
	CItem* m_pItemMisc14;   //aceh
	CItem* m_pItemMisc15;   //ad2h
	CItem* m_pItemMisc16;   //ad6h
	CItem* m_pItemMisc17;   //adah
	CItem* m_pItemMisc18;   //adeh
	CItem* m_pItemMisc19;   //ae2h, magical weapon slot
	CItem* m_pItemWeapon0;  //ae6h
	CItem* m_pItemWeapon1;  //aeah
	CItem* m_pItemWeapon2;  //aeeh
	CItem* m_pItemWeapon3;  //af2h
	char nSlotSelected;     //af6h
	short nAbilitySelected; //af8h
    CGameObject* pThis;     //afeh */

	unsigned char*  pDialogData;        //b02h
	unsigned long   nDialogData;        //b06h
	CDerivedStats   cdsCurrent;         //b0ah
	CDerivedStats   cdsPrevious;        //13c2h, previous state to restore after effect finishes
	CDerivedStats   cdsDiff;            //1c7ah, difference to add to currentState
    struct CGameSaveCharacter {         //2532h, size 1e4h
	    unsigned long actionMode;
	    CButtonData   quickWeapons[4];  //2536h
	    CButtonData   quickSpells[3];   //25f6h
	    CButtonData   quickItems[3];    //2686h
    } gameSaveData;
	IECString       sLongName;          //2716h
	unsigned char   lastCharacterCount; //271ah
	char u271b;
	struct CGameStatsSprite {           //271ch
		STRREF          strStrongestKillName;   //0h
		unsigned long   nStrongestKillXPValue;  //4h
		unsigned long   nPreviousTimeWithParty; //8h
		unsigned long   nJoinPartyTime;         //ch
		BOOL            bInParty;               //10h
		unsigned long   nChapterKillsXPValue;   //14h
		unsigned long   nChapterKillsNumber;    //18h
		unsigned long   nGameKillsXPValue;      //1ch
		unsigned long   nGameKillsNumber;       //20h
		CFavorite       favSpell[4];            //24h
		CFavorite       favWeapon[4];           //5ch
	} statistics;
	ResRef          currentArea;            //27b0h
	unsigned char   bGlobal;                //27b8h, is this Cre Global
	unsigned char   nModalState;            //27b9h, 0: None, 1: Battle Song, 2: Detecting Traps, 3: Hide in Shadows, 4: Turn Undead
	CSound          sndWalking[2];          //27bah
	unsigned long   nSndWalk;               //288eh
	CSound          sndArmor[2];            //2892h
	unsigned char   CurrentWalkingSoundIdx; //2966h
	unsigned char   CurrentArmorSoundIdx;   //2967h
	CSound          sndReady;               //2968h uses walking sound channel
	CSound          sndDeath;               //29d2h m_sndMissile
	CSound          sndCasting;             //2a3ch
	CSound          sndVoice;               //2aa6h
	CSound          sndSpriteEffect;        //2b10h, associated with overlays
	long            nNumberOfTimesTalkedTo; //2b7ah, for NumTimesTalkedTo() trigger
	BOOL            bSeenPartyBefore;       //2b7eh
	ResRef          areaSpecificScriptName; //2b82h
	long            nNumberOfTimesInteractedWith[24]; //2b8ah, each element associated with NPC.IDS //copied from saved game unknown area under happiness
	short           wHappiness;             //2beah
	Object          oInteractingWith;       //2bech, for InteractingWith() trigger
	Enum            triggerId;              //2c00h
	BOOL            bScheduled;             //2c04h, m_active, not outside 24-hour schedule
	BOOL            bActive;                //2c08h, bActiveAI, not diseased/deactivated
	BOOL            bFree;                  //2c0ch, bActiveImprisonment, not mazed/imprisoned
	BOOL            bSelected;              //2c10h, bSelectionState, as per GAM NPC
	BOOL            bPortraitUpdate;        //2c14h
	bool            bInfravisionOn;         //2c18h
	TerrainTable    terrainTable;           //2c19h tt
	TerrainTable    visibleTerrainTable;    //2c29h tt2
	TerrainTable    flightTerrainTable;     //2c39h tt3
	char u2c49;
	AnimData        animation;                  //2c4ah
	unsigned short* pSpriteEffectArray;         //2c50h, m_GoreParticleArray, elements = (2c5c + 2c5d - 1) * m_nGoreParticleFrames
	POINT*          pSpriteEffectArrayPosition; //2c54h, m_GoreParticleArray2, elements as above
	unsigned char   nTwitches;                  //2c58h
	unsigned char   spriteEffectSequenceNumber; //2c59h, m_nGoreParticleSequences
	unsigned char   spriteEffectDuration;       //2c5ah, m_nNumGoreParticls?
	unsigned char   spriteEffectSequenceLength; //2c5bh, m_nGoreParticleFrames
	char            spriteEffectBaseIntensity;  //2c5ch, assoc gore
	unsigned char   spriteEffectRandomIntensity;//2c5dh, assoc gore
	CVidCell        spriteEffectVidCell;        //2c5eh, e.g. flames/sparks
	CVidPalette     spriteEffectPalette;        //2d34h
	unsigned long   spriteEffectFlags;          //2c58h
	CVidCell        spriteSplashVidCell;        //2d5ch, e.g. blood
	CVidPalette     spriteSplashPalette;        //2e32h
	unsigned long   spriteSplashFlags;          //2e56h
	RECT            rSpriteEffectFXBounds;      //2e5ah,         m_rSpriteEffectFX
	POINT           ptSpriteEffectReferenceCentre; //2e6a,  m_ptSpriteEffectReference
	unsigned char   effectExtendDirection;      //2e72h assoc gore particle
	bool            bEscapingArea;          //2e73h
	BOOL            bIsAnimationMovable;    //2e74h, 0 = static (ANIMATE.IDS values < 0x1000) or HELD, set to 1 on dying
	long            posZDelta;              //2e78h
	unsigned char   doBounce;               //2e7ch 3
	unsigned char   nMirrorImages;          //2e7dh, immune to poison and display special effect icon?
	bool            bBlur;                  //2e7eh
	bool            bInvisible;             //2e7fh, invisibility and improved invisibility
	bool            bSanctuary;             //2e80h
	char u2e81;
	CVidCell        sanctuaryVidCell;       //2e82h
	bool            bEntangle;              //2f58h
	char u2f59;
	CVidCell        entangleVidCell;        //2f5ah
	bool            bMinorGlobe;            //3030h
	char u3031;
	CVidCell        minorGlobeVidCell;      //3032h
	bool            bShieldGlobe;           //3108h
	char u3109;
	CVidCell        shieldGlobeVidCell;     //310ah
	bool            bGreasePool;            //31e0h
	char u31e1;
	CVidCell        greasePoolVidCell;      //31e2h
	bool            bWebHold;               //32b8h
	char u32b9;
	CVidCell        webHoldVidCell;         //32bah

	double          fCircleChange;          //3390h
	double          fCurrCircleChange;      //3398h
	short           radius;                 //33a0h, range?
	char            circleFacing;           //33a2h mirror 33a4 through X axis
	char u33a3;
	double          fDirectionOffset;       //33a4h radians CCW to North

	short           wAnimSequenceSimplified;//33ach, m_nSequence, used in animation sound selection
	POINT           posExact;               //33ach linked to coordinates
	POINT           posDistanceToMove;      //33b6h, m_posDelta, towards target at 0x33beh, set on refresh
	POINT           posTargetToMove;        //33beh, m_posDest, next pathfinding node?
	POINT           posOld;                 //33c6h, copied from 0x6 and 0xa
	POINT           posOldWalk;             //33ceh, copied from above
	POINT           posLastVisMapEntry;     //33d6h int u33d6; int u33da;
	long*           pVisMapExploredArea;    //33deh
	short           skipDeltaDirection;     //33e2h assoc with orientations
	short           deltaDirection;         //33e4h assoc with orientations
	short           wOrientGoal;            //33e6h, nNewDirection, orientation to get to
	short           wOrientInstant;         //33e8h, nDirection, instantaneous orientation
	long*           pPath;                  //33eah, has to do with pathfinding and search requests
	short           nPath;                  //33eeh
	CDwordList      pPathTemp;              //33f0h
	CDwordList      nPathTemp;              //340Ch
	short           currPath;               //3428h
	int             walkBackwards;          //342Ah
	int             turningAbout;           //342Eh
	ABGR            lastRGBColor;           //3432h, =0x8000
	int             pathSearchInvalidDest;  //3436h
	int             pathCollision;          //343ah
	CSearchRequest* currentSearchRequest;   //343eh
	short           wBloodPortraitFlashTimer;   //3442h, countdown by 5, this+78h also gives red intensity
	short           wDamageLocatorArrowTimer;   //3444h, countdown by 1
	ABGR            rgbDamageLocatorArrowColor; //3446h, different colour every 5 ticks
	BOOL            bShowDamageArrow;           //344ah, bBloodFlashOn ?
	int             bBloodFlashOn;              //344eh
	CVidBitmap      smallPortrait;              //3452h
	BOOL            bVisibleMonster;            //3508h
	BOOL            bBumpable;                  //350ch if set, will set search bitmap bits 1, 2, 3 in foot circle area, else 4, 5, 6
	bool            bBumped;                    //3510h
	char u3511;
	POINT           ptBumpedFrom;           //3512h
	int             bInClearBumpPath;       //351ah
	int             followLeader;           //351eh
	int             followLeaderAdditive;   //3522h
	long            followLeaderNext;       //3526h
	int             followStart;            //352ah assoc with action 0x30
	CParticleList   BloodParticles;         //352eh, m_lstBlood
	short           wCastingTimer;          //354ah, how many ticks has creature been casting a spell, counts upwards (-1 is none)
	BOOL            bStartedCasting;        //354ch, assoc ForceSpell[Point], Spell, UseItem, UseItemPoint
	BOOL            bInCasting;             //3550h
	short           selectedSound;          //3554h
	short           moveCount;              //3556h, assoc actions
	short           moveToFrontQueue;       //3558h, countdown to move to front vertical list on resurrect?
	short           moveToBackQueue;        //355ah, countdown to move to middle vertical list on death?
	BOOL            moveToBack;             //355ch, bDead, set by CEffectInstantDeath
	BOOL            moveToFront;            //3560h, bResurrect, set by CEffectResurrect
	CEffectList     EffectsEquipped;        //3564h, for while equipped effects
	CEffectList     EffectsMain;            //3590h, for all other effects
	IECPtrList      persistantEffects;      //35bch, AAA8FC, CPermRepeatingEffList
	CPtrArray       lSelectedLevelUpAbilities; //35d8h
	POINT           curDest;                //35ech, assoc actions, target for pathfinding
	long            nGregsRetryCounter;     //35f4h
	BOOL            bWaitingForAreaLoadInLeaveAreaLUA; //35f8h, assoc actions
	short           userCommandPause;       //35fch, confusion timer?
	short           wTriggerRemovalTimer;   //35feh, ticks until remove all 0x0 triggers
	POINT           curPosition;            //3600h, int u3600; int u3604;
	BOOL            bInterruptSpellcasting; //3608h, m_tookDamage
	POSITION*       groupPosition;          //360ch, of CPartySelection list
	int             groupMove;              //3610h
	int             firstDeadAI;            //3614h
	CProjectile*     currentProjectile;     //3618h, once projectile placed in area list, set to NULL
	ResSplContainer* currentSpell;          //361ch
	CItem*           currentItem;           //3620h, quick item currently being used
	short           currentItemSlot;        //3624h, slot of currentItem
	short           currentItemAbility;     //3626h, ability of currentItem
	short           weaponProficiencyList[40];  //3628h
	uchar           generalWeaponList[8];       //3678h
	short           AttackSpeed;                //3680h, (weaponSpeed - physicalSpeed - 1D6 - luck) / 0.5 * dieSize, range 0-10, determines the y-coordinate of the pixel to select in RNDBASE*.BMP //char u3681;
	short           wPreviousTickActionOpcode;  //3682h, m_lastActionID, action opcode in the last tick, sometimes set to 3 Attack or 22 MoveToObject, never checked
	Object          oDerived;               //3684h, m_liveTypeAI, keeps General, scriptName is of thisCre
	Object          oBase;                  //3698h, m_startTypeAI, base for o and oDerived, used for gender for sounds, scriptName is of thisCre
	int             endOfDamageSeq;         //36ach
	short           playDeadCounter;        //36b0h
	short           turnToStoneCounter;     //36b2h
    short           lightningReactCounter;  //36b4h
	short           sleepCounter;           //36b6h
	short           runCounter;             //36b8h
	short           searchPauseCount;       //36bah incremental timer, assoc actions, and luck, and when a new pathfinding search request gets sent
	int             doneSearch;             //36bch assoc actions, ? has search request?
	short           dieCount;               //36c0h
	short           pauseCount;             //36c2h
    short           recoilFrame;            //36c4h
	short           attackFrame;            //36c6h, wRoundTimer, range 0-100, determines the x-coordinate of the pixel to select in RNDBASE*.BMP

    long            noActionCount;          //36c8h assoc actions (idle time?), ++ with NoAction()
	int             inFormation;            //36cch
	BOOL            bForceRefresh;          //36d0h, m_newEffect, Refresh() despite nTimeFree % 15 == e % 15
	bool            bStatisticalAttack;     //36d4h, m_canDamage, i.e. rolling die to hit, applying damage/effects, depleting charges, etc.
	char u36d5;
	BOOL            bLeftWeaponAttack;        //36d6h
	short           wDoHalfAttack;            //36dah, replaced by TobEx, originally wHalveToHitRolls that was not really used, really a BOOL
	BOOL            bForceReinitAnimColors;   //36dch, m_hasColorEffects, DeInitAllColors() then sends CMessageResetAnimColors during Refresh() 
	BOOL            bForceResetAnimColors;    //36e0h, m_hasColorRangeEffects, resets animation colours directly during Refresh() (e.g. Set Item Color opcode, petrify colouring)
	BOOL            bForceMsgResetAnimColors; //36e4h, m_hasAnimationEffects, sends CMessageResetAnimColors during Refresh() (e.g. Polymorph, Animation Change opcodes)
	BOOL            bRemoveFromArea;          //36e8h
	BOOL            bForceResetAnimation;     //36ech
	CMarker         markerSelf;            //36f0h
	CMarker         markerDest;            //3714h
	Enum            targetId;               //3738h, the target of actions?
	POINT           ptTarget;               //373ch, set by ProtectPoint(), MoveToPoint(), Leader(), cscTarget places here
	short           targetAreaSize;         //3744h
	uchar           nQuickSlotSelected;     //3746h, copied from af6h
	uchar           nQuickAbilitySelected;  //3747h, copied from af8h
	CButtonData     currentUseButton;       //3748h
	ResRef          rDialog;                //3778h, main dialog
	ResRef          rDialogInteraction;     //3780h, interact dialog
	bool            sequenceTest;           //3788h
	char u3789;
	CStrRef         soundset[100];          //378ah
	uchar           cRollSaveDeath;         //6282h, 1D20
	uchar           cRollSaveWand;          //6283h, 1D20
	uchar           cRollSavePolymorph;     //6284h, 1D20
	uchar           cRollSaveBreath;        //6285h, 1D20
	uchar           cRollSaveSpell;         //6286h, 1D20
	uchar           cRollResistMagic;       //6287h, 1D100
	uchar           cRollEffectProbability; //6288h, 1D100, test against effect prob1 and prob2
	uchar           cRollWildMagicLevelMod; //6289h, 1D20, column of LVLMODWM.2DA
	uchar           cRollWildMagicSurge;    //628Ah, 1D20, if 0, do wild magic
	char u628b;
	short           nSelectionCountCommon;  //628ch
    short           nSelectionCountCommonRareCounter;//628eh
    short           nSelectionCountRare;    //629ch
    short           nSelectionCountAction;  //6292h
	long            lastCheckedHitPoints;   //6294h, nLargestCurrentHP, set to largest current HP value obtained
	BOOL            bMoraleBroken;          //6298h, m_moraleFailure, contains char data
	BOOL            bIsAttacking;           //629ch, m_startedSwing, in the middle of attack phase of a round
	short           followCount;            //62a0h  assoc actions
	int             clearAIOnRemoveFromArea;//62a2h
	long            nFaceTalkerTimer;       //62a6h, m_dialogWait, countdown timer
	Enum            nFaceTalkerId;          //62aah, m_dialogWaitTarget, orients cre to person who is about to talk to them
	long            nSoundLength;           //62aeh, in sec
    uchar           talkingRenderCount;     //62b2h
    uchar           inControlLastTime;      //62b3h
    bool            bSecondPass;            //62b4h
	char            u62b5;                  // TobEx AfterLight new, UnselectableType
	CDwordList      PortraitIcons;          //62b6h
	CVidCell        PortraitIconVidCell;    //62d2h
	int             firstActionSound;       //62a8h
	long            nTicksLastRested;       //63ach, base time to calculate fatigue = (nGameTime - 63ach) / (4 hours in ticks) - FATIGUE_BONUS
	BOOL            berserkActive;          //63b0h, updated every AIUpdateActions(), no actual berserk action unless STATE_BERSERK
	short           attackSoundDeadzone;    //63b4h
	long            nHPBonusPrev;           //63b6h, m_nHPCONBonusTotalOld, on previous Refresh()
	BOOL            bConstructing;          //63bah, m_bHPCONBonusTotalUpdate, TRUE only when object is still being constructed
	long            nLastWeightCheck;       //63beh, compared to total item weight
	ulong           modalCounter;           //63c2h
	BOOL            bHiding;                //63c6h
	ulong           lastRegenerationTime;   //63cah, gets nGameTime during refresh
	BOOL            bLevelUpAvailable;      //63ceh
    bool            bAllowDialogInterrupt;  //63d2h
    bool            bHappinessChanged;      //63d3h
	long            nUnselectableCounter;   //63d4h
	ResRef          secondarySounds;        //63d8h, from CRE entry in GAM file
	long            nStealthGreyOut;        //63e0h, m_bStoneSkin?
	int             bMentalStateActing;     //63e4h
	int             nLastLevelUpLevel1;     //63e8h
	int             nLastLevelUpLevel2;     //63ech
	int             nLastLevelUpLevel3;     //63f0h
	int             nLastLevelUpHPRoll;     //63f4h
	BOOL            bForceVisualEffects;    //63f8h, bSendOverlayMessages, associated with sending CMessageCreatureOverlay messages
	short           currentActionId;        //63fch contains an actionOpcode (a dialog or escape area action?)

	BOOL            bPlayedEncumberedStopped;       //63feh assoc actions
	BOOL            bPlayedEncumberedSlowed;        //6402h assoc actions
	long            nHighEncumberanceMessageDelay;  //6406h, decrementing timer to show Encumbered: Can Not Move
	long            nLowEncumbranceMessageDelay;    //640ah, decrementing timer to show Encumbered: Slowed
	short           wPoisonTimer;                   //640eh, m_nPlayedPoisonedDamage, set to 100 on poison damage
	short           nEffectListCalls;               //6410h, wDelayedRefreshCounter, if using previous state, Refresh() will delay and increment this counter, next Refresh() will Refresh() the counter number of times (max 5)
	BOOL            bUseCurrentState;               //6412h, m_bAllowEffectListCall, 0 = uses prevState, 1 = uses currentState, set to 0 when refreshing repeating effects
	char            nPreCutSceneMoveScale;          //6416h
    char            u6417;                          //TobEx AfterLight new, StoredBookType
	int             bUsingCutSceneMovement;     //6418h
	int             bDeleteOnRemove;            //641ch
	CVariableMap*   pLocalVariables;            //6420h
	int             bUnmarshalling;             //6424h, 1 during Unmarshal, 0 when done
	CProtectedSplList ProtectedSpls;            //6428h, m_lBounceList, of CProtectedSpl objects
	Enum            eEntrancePointIndex;        //6444h, which entrance point to use when moving between areas
	long            nBounceDelay;               //6448h, counts down from 25 frames
	long            nMoraleAI;                  //644ch
	Enum            nGeneratedVEFIndex;         //6450h, contains enum of CVisualEffect
	long            nTrackingCounter;           //6454h
	CGameButtonList* pSpellButtonList;          //6458h
	CScript*        pDreamScript;               //645ch
	CGameDialog     dlgCurrent;                 //6460h
	CGameDialog     dlgBanter;                  //64c4h
	bool            bForceVisRangeRedraw;       //6528h
    char            u6529;                      //6529h, TobEx AfterLight new, SELECT_ACTION sound playing
	long            nLastExpiryCheck;           //652ah
	long            nContingencyDelay;          //652eh, m_nLastContingencyCheck, a countdown timer set to 100, delay processing contingency triggers until 0
	IECString       sWeaponLeftToHitBonuses;    //6532h
	IECString       sWeaponRightToHitBonuses;   //6536h
	IECString       sWeaponLeftDamageBonuses;   //653ah
	IECString       sWeaponRightDamageBonuses;  //653eh

	short           effectMovementRate;         //6542h, wMvtRatePrev, on last Refresh()
	ulong           effectStateFlags;           //6544h, nStateFlagsPrev, on last Refresh()
	CreFileMemorizedSpellLevel MemorizedInfoWizPrev[9]; //0x6548, on last Refresh()
	CreFileMemorizedSpellLevel MemorizedInfoPrsPrev[7]; //0x65d8, on last Refresh()
	BOOL            effectStoreInitialized;     //6648h, set to TRUE when the above is saved for the first time during Refresh()

	bool            bLevellingUp;               //664ch, true when loading LevelUpPanel
	char            u664d;                      // used as Greeting toggle
	BOOL            bHasDeathSequence;          //664eh
	CEnumList       lstTargetIds;               //6652h
	IECPtrList      lstTargetPts;               //666eh
	BOOL            bInStore;                   //668ah, prevents party required area transitions
	BOOL            bInDialogue;                //668eh
	long            nDialoguePosition;          //6692h, -1 = none, 1 = listener, 2 = speaker
	long            nWaitingOnDialog;           //6696h assoc actions
	BOOL            bCutSceneOverrideOfState;   //669ah, bLeavingArea
	Enum            ePuppet;                    //669eh
	BOOL            bAffectedByTimeStop;        //66a2h, temporary use during Refresh() to set bForceRefresh
	BOOL            bCopyForAdd;                //66a6h
	long            nCopyParent;                //66aah
	CEventMessageList EventMessages;            //66aeh, feedbackQueue
	BOOL            bOnSearchMap;               //66cah set when search bitmap has set 3 bits
	BOOL            bRemovedFromMap;            //66ceh set when search bitmap has set 2 bits
	CGameSpriteLastUpdate cLastSpriteUpdate;    //66d2h
	BOOL            bSendSpriteUpdate;          //6722h
	long            nLastDamageTaken;           //6726h
	long            nDeadVisualEffectCountDown; //672ah, timer
	BOOL            bForceVisibilityCheck;      //672eh
	uchar           SightRadius;                //6732h, m_nVisualRange, default = 14 (AACF46), one unit = 32 pixels
	char            u6733;                      // TobEx AfterLight new, ApplyCastingEffect stage
	BOOL            bPlayAnimationBattleCry;    //6734h play animSound battle cry?, set to 0 after use
	BOOL            bPlayAnimationSelectionSound;//6738h play animSound selection?, set to 0 after use
	char            LevellUpSpellsToPick[10];   //673ch
	long            nCrossAreaChaseCounter;     //6746h, nAbandonActionTimer, used by AttackReeavluate, TryAttack, MoveToObject; when hits 150, ACTION_FAILED is returned
	BOOL            bDidReEquipAll;             //674ah, bReequipItem
	BOOL            bInEquipItem;               //674eh
	char            cFirstResSlot;              //6752h default = 2a; if CRE name does not start with *, will put first character in here; inherits value from GAM (unknown above killXP(chapter), inherits from Actor2E (char)
	char            u6753;                      //6753h padding, used as shield slot disabled state
	long            nNumColorRanges;            //6754h m_nColorsPrev1, compared to cdsCurrent.ColorListRgb count
	long            nNumColorEffects;           //6758h m_nColorsPrev2, compared to cdsCurrent.ColorListRgb count
	long            nNumWeaponImmunities;       //675ch m_nWeaponProtectionsPrev
	BOOL            bCheckedIfVisiblePause;     //6760h
	BOOL            bInitTOB;                   //6764h, m_bCheckedSpecialAbilities, initialises innates, once per construction
	BOOL            bDroppedTempItem;           //6768h, to do with grabbed item
	long            nSpriteUpdateTimer;         //676ch, m_nSkippedUpdates, increments up to 15, then set to 0 on before CMessageSpriteUpdate
	BOOL            bDelayUpdate;               //6770h, assoc sprite update timer, 0 = always sprite update when ready, other = sprite update, reset to 0 if timer > 2
};

STRREF __cdecl CCreatureObject_GetSkillName(unsigned char index);

extern void (CCreatureObject::*CCreatureObject_GetSpellIdsName)(int, IECString*);
extern CDerivedStats& (CCreatureObject::*CCreatureObject_GetDerivedStats)();
extern ACTIONRESULT (CCreatureObject::*CCreatureObject_CastSpell)(ResRef&, CGameObject&, BOOL, STRREF, void*, BOOL, BOOL);
extern void (*CCreatureObject_RemoveItem)(CCreatureObject&, int);
extern CEffectList& (CCreatureObject::*CCreatureObject_GetEquippedEffectsList)();
extern CEffectList& (CCreatureObject::*CCreatureObject_GetMainEffectsList)();
extern CCreatureObject& (CCreatureObject::*CCreatureObject_Construct_10)(void*, unsigned int, BOOL, int, int, int, unsigned int, int, int, int);
extern void (CCreatureObject::*CCreatureObject_CreateGore)(int, short, short);
extern void (CCreatureObject::*CCreatureObject_UpdateHPStatusTooltip)(CUIControl&);
extern short (CCreatureObject::*CCreatureObject_GetOrientationTo)(POINT&);
extern short (*CCreatureObject_CalculateOrientation)(POINT&, POINT&);
extern void (CCreatureObject::*CCreatureObject_SetAnimationSequence)(short);
extern void (CCreatureObject::*CCreatureObject_StartSpriteEffect)(char, char, int);
extern CItem* (CCreatureObject::*CCreatureObject_GetFirstEquippedLauncherOfAbility)(ItmFileAbility* ability, short& pnSlot);
extern int (CCreatureObject::*CCreatureObject_GetSlotOfEquippedLauncherOfAmmo)(short, short);
extern void (CCreatureObject::*CCreatureObject_UnequipAll)(BOOL);
extern void (CCreatureObject::*CCreatureObject_EquipAll)(BOOL);
extern void (CCreatureObject::*CCreatureObject_AddKnownSpell)(ResRef&, BOOL);
extern CreFileKnownSpell* (CCreatureObject::*CCreatureObject_GetKnownSpellPriest)(int, int);
extern CreFileKnownSpell* (CCreatureObject::*CCreatureObject_GetKnownSpellMage)(int, int);
extern CreFileKnownSpell* (CCreatureObject::*CCreatureObject_GetKnownSpellInnate)(int, int);
extern CreFileMemSpell* (CCreatureObject::*CCreatureObject_GetMemSpellPriest)(int, int);
extern CreFileMemSpell* (CCreatureObject::*CCreatureObject_GetMemSpellMage)(int, int);
extern CreFileMemSpell* (CCreatureObject::*CCreatureObject_GetMemSpellInnate)(int, int);
extern BOOL (CCreatureObject::*CCreatureObject_AddMemSpellPriest)(int, int, int*);
extern BOOL (CCreatureObject::*CCreatureObject_AddMemSpellMage)(int, int, int*);
extern BOOL (CCreatureObject::*CCreatureObject_AddMemSpellInnate)(int, int, int*);
extern BOOL (CCreatureObject::*CCreatureObject_AddKnownSpellPriest)(ResRef&, int);
extern BOOL (CCreatureObject::*CCreatureObject_AddKnownSpellMage)(ResRef&, int);
extern BOOL (CCreatureObject::*CCreatureObject_AddKnownSpellInnate)(ResRef&, int);
extern void (CCreatureObject::*CCreatureObject_ApplyClassAbilities)(CDerivedStats& cdsOld, BOOL bPrintMsgForSpecAbil);
extern void (CCreatureObject::*CCreatureObject_RemoveNewSpecialAbilities)(CDerivedStats& cdsTarget);
extern IECString& (CCreatureObject::*CCreatureObject_GetLongName)();
extern STRREF (CCreatureObject::*CCreatureObject_GetLongNameStrRef)();
extern void (CCreatureObject::*CCreatureObject_SetSpellMemorizedState)(ResSplContainer&, BOOL);
extern void (CCreatureObject::*CCreatureObject_ValidateAttackSequence)(char*);
extern char (CCreatureObject::*CCreatureObject_GetNumUniqueMemSpellMage)(int, ResRef);
extern BOOL (CCreatureObject::*CCreatureObject_InDialogAction)();
extern void (CCreatureObject::*CCreatureObject_SetSaveName)(ResRef&);
extern unsigned int (CCreatureObject::*CCreatureObject_GetKitUnusableFlag)();
extern void (CCreatureObject::*CCreatureObject_PrintEventMessage)(short, int, int, int, STRREF, BOOL, IECString&);
extern short (CCreatureObject::*CCreatureObject_GetProficiencyInItem)(CItem&);
extern ACTIONRESULT (CCreatureObject::*CCreatureObject_ActionMoveToObject)(CGameObject&);
extern ACTIONRESULT (CCreatureObject::*CCreatureObject_ActionPickPockets)(CCreatureObject&);
extern short (CCreatureObject::*CCreatureObject_GetProficiency)(int);
extern short (CCreatureObject::*CCreatureObject_GetSpellCastingLevel)(ResSplContainer&, BOOL);
extern ACTIONRESULT (CCreatureObject::*CCreatureObject_ActionJumpToAreaEntranceMove)(IECString);
extern void (CCreatureObject::*CCreatureObject_UpdateFaceTalkerTimer)();

extern bool (CCreatureObject::*CCreatureObject_NeedsAIUpdate)(bool, int);
extern BOOL (CCreatureObject::*CCreatureObject_EvaluateStatusTrigger)(Trigger&);
extern ACTIONRESULT (CCreatureObject::*CCreatureObject_ExecuteAction)();
extern void (CCreatureObject::*CCreatureObject_SetCurrentAction)(Action&);

#endif //OBJCRE_H
