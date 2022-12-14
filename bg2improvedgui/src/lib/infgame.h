#ifndef INFGAME_H
#define INFGAME_H

#include "stdafx.h"
#include "infbtarr.h"
#include "rescore.h"
#include "objcore.h"
#include "objcre.h"
#include "vidcore.h"
#include "arecore.h"
#include "scrcore.h"
#include "sndcore.h"
#include "stocore.h"
#include "itmcore.h"
#include "uitext.h"

typedef IECPtrList CPartyLocationList; //AAB968

struct CWorldMapArea// Size=0xf0
{
    ResRef m_resCurrentArea;// Offset=0x0 Size=0x8
    ResRef m_resOriginalArea;// Offset=0x8 Size=0x8
    char m_strName[32];// Offset=0x10 Size=0x20
    unsigned long m_dwFlags;// Offset=0x30 Size=0x4
    unsigned long m_sequence;// Offset=0x34 Size=0x4
    unsigned long m_mapLocationX;// Offset=0x38 Size=0x4
    unsigned long m_mapLocationY;// Offset=0x3c Size=0x4
    unsigned long m_strLabel;// Offset=0x40 Size=0x4
    unsigned long m_strAreaName;// Offset=0x44 Size=0x4
    unsigned char m_resAreaLoadMosaic[8];// Offset=0x48 Size=0x8
    unsigned long m_nNorthEdgeStartingLink;// Offset=0x50 Size=0x4
    unsigned long m_nNorthEdgeCount;// Offset=0x54 Size=0x4
    unsigned long m_nWestEdgeStartingLink;// Offset=0x58 Size=0x4
    unsigned long m_nWestEdgeCount;// Offset=0x5c Size=0x4
    unsigned long m_nSouthEdgeStartingLink;// Offset=0x60 Size=0x4
    unsigned long m_nSouthEdgeCount;// Offset=0x64 Size=0x4
    unsigned long m_nEastEdgeStartingLink;// Offset=0x68 Size=0x4
    unsigned long m_nEastEdgeCount;// Offset=0x6c Size=0x4
    unsigned long nUnused[32];// Offset=0x70 Size=0x80
};

struct Res2daContainer { //Size 10h
	BOOL bLoaded; //0h
	ResTxt* pRes; //4h
	ResRef name; //8h
};

struct CWorldTimer { //Size 6h
//Constructor: 0x648F4D
	void UnpauseGame();
	void PauseGame();

	unsigned int nGameTime; //0h (1dd0h), current game time (in ticks)
	char bRun; //4h (1dd4h), 0 = paused, 1 = unpaused
	unsigned char nPartsOfHour; //5h, during dawn/dusk hour, is the part of the hour out of 255 (1dd5h)
};

extern void (CWorldTimer::*CWorldTimer_UnpauseGame)();
extern void (CWorldTimer::*CWorldTimer_PauseGame)();

struct CPartySelection { //Size 22h CAIGroup
	Enum GetFirstSelected();

	short       groupId;        //ffff
	BOOL        groupChanged;   //always set to 1 on remove
	CEnumList   memberList; //6h, party members first in number order, then non-members
};

extern Enum (CPartySelection::*CPartySelection_GetFirstSelected)();

struct CRuleTable { //Size 24h
	CRuleTable(); //63E230
	CRuleTable& Construct() { return *this; } //dummy

	~CRuleTable();

	void LoadTable(ResRef& r);
	IECString& GetValue(IECString& sColName, IECString& sRowName);
	bool FindString(IECString& s, POINT* ppos, BOOL bCheckHeaders);
	void UnloadRes();
	IECString GetDefaultValue();
	IECString& GetValue(POINT& ptColRow);

	//custom
	IECString& GetRowName(int nRow);
	IECString& GetColName(int nCol);

	Res2daContainer m_2da; //0h
	IECString* pColHeaderArray; //10h
	IECString* pRowHeaderArray; //14h
	IECString* pDataArray; //18h
	IECString defaultVal; //1ch
	short nCols; //20h
	short nRows; //22h
};

extern CRuleTable& (CRuleTable::*CRuleTable_Construct)(void);
extern void (CRuleTable::*CRuleTable_LoadRes)(ResRef&);
extern IECString& (CRuleTable::*CRuleTable_GetValue_2)(IECString&, IECString&);
extern bool (CRuleTable::*CRuleTable_FindString)(IECString&, POINT*, BOOL);
extern void (CRuleTable::*CRuleTable_UnloadRes)();
extern IECString (CRuleTable::*CRuleTable_GetDefaultValue)();
extern IECString& (CRuleTable::*CRuleTable_GetValue_1)(POINT&);

struct IdsEntry { //Size Ch
	int nOpcode;
	IECString value; //entire value (i.e. full function description)
	IECString head; //i.e. function name only
};

struct CRuleTables { //Size 1DB0h
	CRuleTables(); //6213DC
	CRuleTables& Construct() { return *this; } //dummy

	~CRuleTables();
	void Deconstruct() {} //dummy

	IECString GetRaceString(unsigned char nRace);
	IECString GetAlignmentString(char align);
	IECString GetClassString(unsigned char nClass, unsigned int dwKit);
	int CalculateNewHPRule(CRuleTable& rule, int nLevelOld, int nLevelNew, int nMinRoll, int nDivisor, BOOL bOverrideSides, int nOverrideSides, BOOL bOverrideModifier, int nOverrideModifier);
	int CalculateNewHPSubclass(char nClass, char nSubclass, CDerivedStats& cdsOld, CDerivedStats& cdsNew, int nMinRoll, int nDivisor);
	int GetMaxMageSpells(int nSpellLevel);
	ResRef GetMageSpellRef(int nSpellLevel, int nIndex);
	STRREF GetCharSndStrRef(int dwCustom, int dwRow, char sex);
	void GetDetailedClassString(char Class, unsigned int dwKit, unsigned int dwFlags, IECString& ptr, CCreatureObject* cre);
	int GetWeapProfMax(char nClassId, char nClassPrimary, char nClassSecondary, BOOL bTwoClasses, int nWeapProfId, unsigned int dwKit);
	BOOL IsMageSchoolAllowed(unsigned int dwKit, unsigned char nRace);
	int GetIntModMaxSpellsPerLevel(CDerivedStats& cds);
	unsigned char MapCharacterSpecializationToSchool(unsigned int dwKit); // orig is (unsigned short)
	bool GetContingencyConditionTexts(STRREF* pStrref, STRREF* pDescription, short wIndex);
	bool GetContingencyTargetTexts(STRREF* pStrref, STRREF* pDescription, short wIndex);
	CRuleTable& GetClassAbilityTable(unsigned char cClass, unsigned int dwKit);
	ResRef GetMageSpellRefAutoPick(char nSpellLevel, char nIndex);
    int GetMaxMemorizedSpellsPriest(Object& Obj, CDerivedStats& DStats, int nSpellLevel, int *nBonus);
    void GetShamanSkillPoints(unsigned char nLevel, unsigned char *nDetect); // TobEx new

	CRuleTable RMODCHR; //0h
	CRuleTable RMODREP; //24h
	CRuleTable REPUTATI; //48h
	Identifiers NODECODE; //6ch
	CRuleTable PROFS; //ach
	CRuleTable PROFSMAX; //d0h
	CRuleTable SKILLRAC; //f4h
	CRuleTable SKILLDEX; //118h
	CRuleTable SKILLBRD; //13Ch
	CRuleTable SKILLRNG; //160h
	CRuleTable ABRACERQ; //184h
	CRuleTable ABRACEAD; //1a8h
	CRuleTable ABCLASRQ; //1cch
	CRuleTable ABCLSMOD; //1f0h
	CRuleTable AbDCScRq; //214h
	CRuleTable AbDCDsRq; //238h
	CRuleTable SAVEPRS; //25ch
	CRuleTable SAVEWAR; //280h
	CRuleTable SAVEWIZ; //2a4h
	CRuleTable SAVEROG; //2c8h
	CRuleTable SAVECNDH; //2ech
	CRuleTable SAVECNG; //310h
	CRuleTable SAVEMONK; //334h
	CRuleTable HPWAR; //358h
	CRuleTable HPWIZ; //37ch
	CRuleTable HPPRS; //3a0h
	CRuleTable HPROG; //3c4h
	CRuleTable HPCONBON; //3e8h
	CRuleTable HPMONK; //40ch
	CRuleTable STRTGOLD; //430h
	CRuleTable THAC0; //454h
	CRuleTable MXSPLPRS; //478h
	CRuleTable MXSPLPAL; //49ch
	CRuleTable MXSPLRAN; //4c0h
	CRuleTable MXSPLWIZ; //4e4h
	CRuleTable MXSPLBRD; //508h
	CRuleTable MXSPLSRC; //52ch
	CRuleTable MXSPLWIS; //550h
	CRuleTable MXSPLDRU; //574h
	CRuleTable SPLSRCKN; //598h
	CRuleTable BACKSTAB; //5bch
	CRuleTable XPLEVEL; //5e0h
	CRuleTable LAYHANDS; //604h
	Identifiers INSTANT; //628h
	Identifiers ACTSLEEP; //668h
	CRuleTable MONSUM01; //6a8h
	CRuleTable MONSUM02; //6cch
	CRuleTable MONSUM03; //6f0h
	CRuleTable ANISUM01; //714h
	CRuleTable ANISUM02; //738h
	CRuleTable STRMOD; //75ch
	CRuleTable STRMODEX; //780h
	CRuleTable INTOXMOD; //7a4h
	CRuleTable FATIGMOD; //7c8h
	CRuleTable EFFTEXT; //7ech
	CRuleTable INTERACT; //810h
	CRuleTable SPELLS; //834h
	CRuleTable TOOLTIP; //858h
	CRuleTable STARTARE; //87ch
	CRuleTable STARTPOS; //8a0h
	CRuleTable SPRKLCLR; //8c4h
	CRuleTable MONTHS; //8e8h
	CRuleTable YEARS; //90ch
	CRuleTable DEXMOD; //930h
	CRuleTable PDIALOG; //954h
	CRuleTable CHARSND; //978h
	CRuleTable RNDEQUIP; //99ch
	CRuleTable RNDTREAS; //9c0h
	CRuleTable RNDMAGIC; //9e4h
	CRuleTable RNDSCROL; //a08h
	CRuleTable RNDWEP; //a2ch
	CRuleTable INTERDIA; //a50h
	CRuleTable INTMOD; //a74h
	CRuleTable DONARUMR; //a98h
	CRuleTable AREALINK; //abch, used for multiplayer
	CRuleTable AREALINS; //ae0h, used for single player
	CRuleTable CACHVALD; //b04h
	CRuleTable SCRPDESC; //b28h
	CRuleTable STATDESC; //b4ch
	CRuleTable MASTAREA; //b70h
	CRuleTable MOVIDESC; //b94h
	CRuleTable SPELDESC; //bb8h
	CRuleTable BNTYCHNC; //bdch
	CRuleTable NPCLEVEL; //c00h
	CRuleTable NPCLVL25; //c24h
	CRuleTable DEATH; //c48h
	CRuleTable ALIGNMNT; //c6ch
	CRuleTable REPMODST; //c90h
	CRuleTable REPSTART; //cb4h
	CRuleTable XPCAP; //cd8h
	CRuleTable WSPECIAL; //cfch
	CRuleTable WSPATCK; //d20h
	CRuleTable LORE; //d44h
	CRuleTable LOREBON; //d68h
	CRuleTable CHRMODST; //d8ch
	CRuleTable RAISDEAD; //db0h
	CRuleTable HAPPY; //dd4h
	CRuleTable REPTXT; //df8h
	CRuleTable PORTRAIT; //e1ch
	CRuleTable INTOXCON; //e40h
	CRuleTable CSOUND; //e64h
	CRuleTable ITEMEXCL; //e88h
	Identifiers CHASE; //each
	Identifiers VEFPROJ; //eech
	Identifiers PROJECTL; //f2ch
	CRuleTable CONTCOND; //f6ch
	CRuleTable CONTTARG; //f90h
	CRuleTable ABSTART; //fb4h
	CRuleTable WEAPPROF; //fd8h
	CRuleTable CLABFI01; //ffch
	CRuleTable CLABFI02; //1020h
	CRuleTable CLABFI03; //1044h
	CRuleTable CLABFI04; //1068h
	CRuleTable CLABPA01; //108ch
	CRuleTable CLABPA02; //10b0h
	CRuleTable CLABPA03; //10d4h
	CRuleTable CLABPA04; //10f8h
	CRuleTable CLABPA05; //111ch
	CRuleTable CLABRN01; //1140h
	CRuleTable CLABRN02; //1164h
	CRuleTable CLABRN03; //1188h
	CRuleTable CLABRN04; //11ach
	CRuleTable CLABRN05; //11d0h
	CRuleTable CLABTH01; //11f4h
	CRuleTable CLABTH02; //1218h
	CRuleTable CLABTH03; //123ch
	CRuleTable CLABTH04; //1260h
	CRuleTable CLABBA01; //1284h
	CRuleTable CLABBA02; //12a8h
	CRuleTable CLABBA03; //12cch
	CRuleTable CLABBA04; //12f0h
	CRuleTable CLABDR01; //1314h
	CRuleTable CLABDR02; //1338h
	CRuleTable CLABDR03; //135ch
	CRuleTable CLABDR04; //1380h
	CRuleTable CLABPR01; //13a4h
	CRuleTable CLABPR02; //13c8h
	CRuleTable CLABPR03; //13ech
	CRuleTable CLABPR04; //1410h
	CRuleTable CLABMA01; //1434h
	CRuleTable CLABMA02; //1458h
	CRuleTable CLABMA03; //147ch
	CRuleTable CLABMA04; //14a0h
	CRuleTable CLABMA05; //14c4h
	CRuleTable CLABMA06; //14e8h
	CRuleTable CLABMA07; //150ch
	CRuleTable CLABMA08; //1530h
	CRuleTable CLABMA09; //1554h
	CRuleTable CLABMO01; //1578h
	CRuleTable CLABFI05; //159ch
	CRuleTable SPLAUTOP; //15c0h
	CRuleTable DUALCLAS; //15e4h
	CRuleTable RANDCOLR; //1608h
	CRuleTable SPAWNGRP; //162ch
	CRuleTable ITEMANIM; //1650h
	CRuleTable HATERACE; //1674h
	CRuleTable XPBONUS; //1698h
	CRuleTable CLEARAIR; //16bch
	CRuleTable SPLASHSC; //16e0h
	CRuleTable LVLMODWM; //1704h
	CRuleTable KITTABLE; //1728h
	CRuleTable KITLIST; //174ch
	CRuleTable* kits[256]; //1770h
	CRuleTable SNDRESRF; //1b70h
	CRuleTable SNDENVRND; //1b94h
	CRuleTable ITEMDIAL; //1bb8h
	CRuleTable LOADHINT; //1bdch
	CRuleTable MSCHOOL; //1c00h
	CRuleTable MSECTYPE; //1c24h
	CRuleTable CONTINGX; //1c48h
	CRuleTable LUABBR; //1c6ch
	CRuleTable LUNUMAB; //1c90h
	CRuleTable XPLIST; //1cb4h
	CRuleTable TRACKING; //1cd8h
	CRuleTable MVESND; //1cfch
	CRuleTable _25STWEAP; //1d20h
	CRuleTable WISH; //1d44h
	CRuleTable ENTRIES; //1d68h
	CRuleTable LOADH25; //1d8ch
};

extern CRuleTables& (CRuleTables::*CRuleTables_Construct)();
extern void (CRuleTables::*CRuleTables_Deconstruct)();
extern IECString (CRuleTables::*CRuleTables_GetRaceString)(unsigned char);
extern IECString (CRuleTables::*CRuleTables_GetAlignmentString)(char);
extern IECString (CRuleTables::*CRuleTables_GetClassString)(unsigned char, unsigned int);
extern int (CRuleTables::*CRuleTables_CalculateNewHPRule)(CRuleTable&, int, int, int, int, BOOL, int, BOOL, int);
extern int (CRuleTables::*CRuleTables_CalculateNewHPSubclass)(char, char, CDerivedStats&, CDerivedStats&, int, int);
extern int (CRuleTables::*CRuleTables_GetMaxMageSpells)(int);
extern ResRef (CRuleTables::*CRuleTables_GetMageSpellRef)(int, int);
extern STRREF (CRuleTables::*CRuleTables_GetCharSndStrRef)(int, int, char);
extern void (CRuleTables::*CRuleTables_GetDetailedClassString)(char, unsigned int, unsigned int, IECString&, CCreatureObject*);
extern int (CRuleTables::*CRuleTables_GetWeapProfMax)(char, char, char, BOOL, int, unsigned int);
extern BOOL (CRuleTables::*CRuleTables_IsMageSchoolAllowed)(unsigned int, unsigned char);
extern int (CRuleTables::*CRuleTables_GetIntModMaxSpellsPerLevel)(CDerivedStats&);
extern unsigned char (CRuleTables::*CRuleTables_MapCharacterSpecializationToSchool)(unsigned int);
extern bool (CRuleTables::*CRuleTables_GetContingencyConditionTexts)(STRREF*, STRREF*, short);
extern bool (CRuleTables::*CRuleTables_GetContingencyTargetTexts)(STRREF*, STRREF*, short);
extern CRuleTable& (CRuleTables::*CRuleTables_GetClassAbilityTable)(unsigned char, unsigned int);
extern ResRef (CRuleTables::*CRuleTables_GetMageSpellRefAutoPick)(char, char);

struct MoveAreasElement { //Size 1Ah
//Constructor: 0x5EF9B0
	Enum eCre; //cre
	ResRef rArea;
	POINT ptDest;
	char cOrient;
	char u15; //pad
	int nDelay;
};

class CMoveAreasList : public IECPtrList { //Size 1Ch
	//AAD240
public:
	void MoveAllTo(CArea& area);
};

extern void (CMoveAreasList::*CMoveAreasList_MoveAllTo)(CArea&);

class CPathNode// Size=0x20
{
    CPathNode *pathNodePrev;// Offset=0x0
    CPathNode *listOpenLeft;// Offset=0x4
    CPathNode *listOpenRight;// Offset=0x8
    CPathNode *listOpenParent;// Offset=0xc
    long gridPosition;// Offset=0x10
    long costStart;// Offset=0x14
    long costTotal;// Offset=0x18
    unsigned char fIsOpen;// Offset=0x1c
    unsigned char pad[3];// Offset=0x1d
};

class CPathSearch// Size=0x18
{
    int PATH_SMOOTH;// Offset=0x0
    class CPathNode **pListGrid;// Offset=0x4
    class CPathNode *openList;// Offset=0x8
    short openListLevel;// Offset=0xc
    short nOpenList;// Offset=0xe
    long *pathBegin;// Offset=0x10
    short nPathNodes;// Offset=0x14
    short pathCurrent;// Offset=0x16
};

struct CInfGame : public CRuleTables { //Size 4DC8h
//Constructor: 0x67AD88
	void InitGame(int nUnused, int nUnused2);
	int GetNumOfItemInBag(ResRef& rBag, ResRef& rItem, BOOL bIdentifiedOnly);
	void DemandServerStore(ResRef& rName, BOOL bAlsoUpdateTemp);
	void ReleaseServerStore(ResRef& rName);
	short GetPartyMemberSlot(Enum e);
	CArea& GetLoadedArea(IECString sAreaName);
	void AddExperienceParty(int n);
	void SetLoseCutscene();
	void StorePartyLocations(BOOL);
	int GetNumControlledSummons();
    void FeedBack(unsigned short feedBackId, int int1);
    BOOL CheckItemUsable(short nPortraitNum, CItem *Item, unsigned int *ErrorCode, BOOL bAsync);
    unsigned int CheckItemNotUsableByClass(unsigned char nClass, unsigned int dwNotUsableBy, unsigned int baseFlags);
	
#ifdef _DEBUG
	_CCriticalSection u1db0;
#else
	CCriticalSection u1db0; //for CSearchRequests
#endif

	CWorldTimer m_WorldTimer; //1dd0h
	BOOL bGameRunning; //1dd6h
	char u1dda;
	char u1ddb;
	int u1ddc[2];
	char u1de4;
	char u1de5;
	char u1de6;
	char u1de7;
	BOOL bStartedDeathSequence; //set when preparing to show end cutscene
	CEnumList lstTargetIds;  //1dec, AA5C50
	IECPtrList lstTargetPts; //1e08, AA9CD0
	short nState;      //1e24, 2: thieving mode
	char iconCount;    //1e26
	char iconIndex;    //1e27 36: thief thieving, 40: bard thieving
	ResRef iconResRef; //1e28
	char   tempCursor; //1e30, 0x65 = tooltip on, 0x04 = tooltip off?
	char u1e31; // pad
	int u1e32;  // iContainer ?
	Enum u1e36; // iContainerSprite ?, used in ObjectActionListEmpty()
	BOOL bAllowTargetDead; //1e3a

	struct CGamePermission { //Size 8h
	//Constructor: 0x573EB0
		bool bPermitBuySell; //0h
		bool bPermitAreaTransition; //1h
		bool bPermitDialog; //2h
		bool bPermitViewCharacter; //3h
		bool bPermitPausing; //4h
		bool u5; //5h
		bool bPermitLeader; //6h
		bool bPermitModifyCharacters; //7h
	} m_CGamePermission; //1e3eh

	struct CMultiplayerSettings { //Size AEh
	//Constructor: 0x5F0150
		CGamePermission u0[6]; //by PlayerIdx
		CGamePermission u30; //SoloServer
		int nPlayerID[6]; //38h
		int u50[6];
		bool bCharacterReady[6]; //68h, by PlayerIdx
		char bCharacterStatus[6]; //6eh, by PlayerIdx
		int u74[6]; //unused?
		char u8c;
		
		//bit1: Experience
		//bit2: Items
		unsigned char bImportFlags; //8dh

		char u8e;
		char u8f;
		char u90;
		char u91;
		int u92[2]; //unused?
		IECString u9a;
		int u9e;
		int ua2;
		bool bDialogueAboutToStart; //a6h
		char ua7;
		int ua8;
		char uac;
		char uad; //padding?
	} m_MultiplayerSettings; //1e46h

	struct CRemoteGameMode { //Size 18h
	//Constructor: 0x5B0C30
		int mode[6]; //0h, init to -1, set to 0x502 or 0x182
	} m_CRemoteGameMode; //1ef4h

	CButtonArray m_CButtonArray; //1f0ch

	struct _6C2B7B { //Size Ch
	//Constructor: 0x6C2B7B
	//ptr to this object copied to CTileset.u18 (a ptr) and to CTileset.uac
		int u0; //gets CVideoMode.u674
		int u4; //gets CVideoMode.u670 (nSize)
		int u8; //array of nSize*0xch objects
	} m_6C2B7B; //372ch

	int bSaveScreen;
	IECString sSaveScreenArea;
	CVidBitmap m_colorRangePalette; //3740h
	CGameObjectArrayHandler m_GameObjectArrayHandler; //37f6h
	CGameRemoteObjectArrayHandler m_GameRemoteObjectArrayHandler; //3824h
	char m_VisibleAreaIdx; //38b4h, index to loaded area array of the currently visible area
	char u38b5; //pad
	CArea* m_pLoadedAreas[12]; //38b6h
	CArea* m_pMasterArea; //38e6h
	Enum ePlayersJoinOrder[6]; //38eah
	Enum ePlayersPartyOrder[6]; //3902h, 0: leader
	short numInParty; //391ah
	CEnumList lstGlobalCreatures; //enums global characters
	CPtrArray characterOverflow;; //3938h
	short nCharacterOverflowCount; // 394ch
	short nReputation; //394eh, 10*that displayed in game
	int* pathSearch; //3950h CPathSearch* (18h size)
	int* listGrid; //3954h ListGrid* (x*y/2 size)
	CPartySelection m_PartySelection; //3958h
	CEnumList celControlledObjects; //397ah
	CEnumList celFamiliars; //3996h, treated as if party-like
	ResRef u39b2[9]; //array of familiar names
	char u39fa[81][0x1c]; //39fah, 0x51 (81) * 0x1c size objects
	int u42d6;
	char* tempPath; //42dah, IECString.GetBuffer()
	char* tempsavePath; //42deh 
	char* scriptsPath; //42e2h
	char* soundsPath; //42e6h
	char* portraitsPath; //42eah
	char* charactersPath; //42eeh
	IECString currentSaveGameName; //42f2h
	char* savePath; //42f6h
	char* mpsavePath; //42fah
	char* arenasPath; //42feh

	struct CGameSave { //4302h
		//bit1: pass key presses onto active ui control
		/*
		-1 None
		0x182 DialogMode
		0x502 MultiDialogMode
		0x802
		0x1000
		0x4000
		0x8000
		0x1016e CutSceneMode
		0x3016e CutSceneLite
		*/
		int inputMode; //0h, essentially a huge bunch of flags

		ResRef cResCurrentWorldArea;
		int nCurrentWorldLink;
		int nGold; //4312h, amount of gold player currently has
	    short curFormation;
	    int quickFormations[5];
	    char groupInventory[390]; // u432c
	    BOOL bArenaMode; //44b2h, arena mode? FX affecting entire area always affect entire party too (even if excluded), when dropping items on death drop inventory only
	    int u44b6;
	    short u44ba;
	    BOOL cutScene; //bInCutscene?
	    int nCutSceneStatusOverride; // 44c0
    } m_GameSave;

	struct GameOptions {
		int m_gore; //44c4h, Memory Level (m_gore)
		int m_goreOption; //Memory Access
		int m_scrollSpeed; //44cch, Mouse Scroll Speed (m_scrollSpeed)
		int m_displayVerbalText; //44d0h, Subtitles (m_displayVerbalText)
		int m_guiFeedBackLevel; //44d4h, GUI Feedback Level (m_guiFeedBackLevel)
		int m_locatorFeedbackLevel; //44d8h, Locator Feedback Level
		int m_soundMusicVolume; //44dch
		int m_soundFXVolume; //44e0h
		int m_soundDialogVolume; //44e4h, all voices
		int m_soundAmbientVolume; //44e8h
		int m_soundMovieVolume; //44ech
		int m_soundFootStepsOn; //44f0h, Footsteps (m_soundFootStepsOn)
		int m_soundVoicesNo; //44f4h
		int m_graphicsTranslucentShadows; //44f8h
		int m_graphicsForceMirroringOff; //44fch
		int m_graphicsTranslucentEffects; //4500h
		int m_toolTips; //4504h, Tooltips (tool tip delay)
		int m_nBoredTime; //4508h, Bored Timeout
		int m_nCommandSoundsFrequency; //450ch, CommandSoundFrequency, 1 = never, 2 = seldom, 3 = always
		int m_nSelectSoundsFrequency; //4510h, SelectionSoundFrequency, 1 = never, 2 = seldom, 3 = always
		BOOL m_bAlwaysDither; //4514h, Always Dither
		int m_nKeyBoardScrollAmount; //4518h, Keyboard Scroll Speed
		int m_nEffectTextLevel; //451ch, Effect Text Level
		int m_nTutorialState; //4520h, Tutorial State
		int m_nAttackSounds; //4524h, Attack Sounds
		int m_nAutoPauseState; //4528h, Auto Pause State
		BOOL m_bAutoPauseCenter; //452ch, Auto Pause Center
		int m_nDifficultyLevel; //4530h, Difficulty Level
		int m_nDifficultyMultiplier; //4534h, % based on difficulty, e.g. -25 means -25%
		int m_nMPDifficultyMultiplier; //4538h, as above for multiplayer
		BOOL m_bInfravision; //453ch, Infravision
		BOOL m_bStupidQuickItemStuff; //4540h, Quick Item Mapping
		BOOL m_bWeatherEnabled; //4544h, Weather
		BOOL m_bCheatsEnabled; //4548h, Cheats
		BOOL m_bEnvironmentalAudio; //454ch, Environmental Audio
		BOOL m_bRestHealParty; //4550h, Heal Party on Rest
		BOOL m_bTerrainHugging; //4554h, Terrain Hugging
		BOOL m_bHPOverHead; //4558h, HP Over Head
		BOOL m_bDebugMode; //455ch, Debug Mode
		BOOL m_bForceDialogPause; //4560h, Force Dialog Pause
		BOOL m_bUse3dAnimations; //4564h, Use 3d Animations
		BOOL m_bCriticalHitScreenShake; //4568h, Critical Hit Screen Shake
		BOOL m_bLowPerformance; //456ch, Low End Machine
		BOOL m_bHotkeysOnTooltips; //4570h, Hotkeys On Tooltips
		BOOL m_bVeryLowPerformance; //4574h, Low End Machine 2
		BOOL m_bDisplayExtraCombatInfo; //4578h, Extra Combat Info
		short m_nAreaEffectsDensity; //457ch, Area Effects Density
		BOOL m_bDuplicateFloatingText; //457eh, Duplicate Floating Text
		char m_nTilesPrecachePercent; //Tiles Precache Percent
		char u4583; //pad?
		BOOL m_bDisableCastingGlows; //4584h, Disable Casting Glows
		BOOL m_bLowMemSounds1; //4588h, Low Mem Sounds 1
		BOOL m_bLowMemSounds2; //458ch, Low Mem Sounds 2
		BOOL m_bDisableVEFVidCells; //4590h, Disable VEFVidCells
		BOOL m_bDisableDisplayText; //4594h, Disable Display Text
		int m_bMidLevelBrighten; //Mid Level Brighten
		int m_bHighLevelBrighten; //High Level Brighten
		short m_nAreaEffectsRefreshProb; //45a0h, Area Effects Refresh Probability
		int u45a2; //Enhanced Path Search/Finding
		int u45a6; //Pausing Map
		int u45aa; //Extra Feedback
		int u45ae; //Filter Games
		int u45b2; //All Learn Spell Info
		int u45b6; //Disable Statics During Combat
		int u45ba; //Disable Foot Steps During Combat
		int u45be; //Disable Placed Sounds During Combat
		int u45c2; //Super Atomic Speed Fighting Action
		int u45c6; //Automated Speed Adjustment
		BOOL u45ca; //1 (bAutoTranslucentShadows) - only if no3d
		BOOL u45ce; //0 (bAutoVeryLowPerformance)
		BOOL u45d2; //0 (bAutoLowPerformance)
		BOOL u45d6; //0 (bAutoDisableBrightest) - only if no3d
		BOOL u45da; //0 (bAutoLimitTransparency) - only if no3d
		BOOL u45de; //0 (bAutoDisableCastingGlows)
		BOOL u45e2; //0 (bAutoDisableVEFVidCells)
		BOOL u45e6; //1 (bAutoMidLevelBrighten) - only if no3d
		BOOL u45ea; //1 (bAutoHighLevelBrighten) - only if no3d
		BOOL u45ee; //1 (bAutoFootSteps)
		BOOL u45f2; //1 (bAutoAttackSounds)
		BOOL u45f6; //1 (bAuto3dAnimations)
		BOOL u45fa; //0 (bAutoDisableVVCSounds)
		BOOL u45fe; //0 (bAutoFasterBlur) - only if no3d
		int u4602; //Automated Translucent Shadows
		int u4606; //Automated Very Low Performance
		int u460a; //Automated Low Performance
		int u460e; //Automated Disable Brightest
		int u4612; //Automated Limit Transparency
		int u4616; //Automated Disable Casting Glows
		int u461a; //Automated Disable VEFVidCells
		int u461e; //Automated Mid Level Brighten
		int u4622; //Automated High Level Brighten
		int u4626; //Automated Foot Steps
		int u462a; //Automated Attack Sounds
		int u462e; //Automated 3D Animations
		int u4632; //Automated Disable VVC Sounds
		int u4636; //Automated Faster Blur		
		BOOL u463a; //1, Conditional Battle Music Change (do not change music if source and target not in party, or if source = target)
		int u463e; //Fully Disable Non Visible During Combat
		int u4642; //Disable VVC Sounds
		int u4646; //Faster Blur
	} m_GameOptions; //44c4h

	struct CGameJournal { //Size 14h
	//Constructor: 0x56FFD9
        void UpdateTextDisplay(int Chapter, CUITextArea *pText, unsigned char SectionType);
		CPtrArray u0; //of IECPtrList AAA9A4s (11 of them)
	} m_CGameJournal; //464ah

	struct CWorldMap { //Size 1Ch
	//Constructor: 0x65CCD0
        BOOL GetAreaIndex(unsigned int nMap, ResRef *cResArea, unsigned int *nArea);
		ResRef m_cResRef;
		struct CWorldMapHeader {  //  0x8 class CWorldMapHeader
		    unsigned long m_nMapCount;
		    unsigned long m_nMapOffset;
	    } m_cHeader; 
		int *m_pData;               //  0x10 class CWorldMapData *
		CWorldMapArea **m_ppAreas;  //  0x14 class CWorldMapArea **
		int *m_ppLinks;             //  0x18 class CWorldMapLinks **
	} m_CWorldMap; //465eh

	//char+20 = hotkey for keymap
	//0-8: [Keymap Page]
	//9-22: [Keymap Actions]
	//23-32: [Keymap Miscellaneous]
	//33-47: [Keymap Quick Slots]
	//48-386:  [Keymap Priest Spells][Keymap Wizard Spells][Keymap High Level Abilities]
	char m_Keymap[387]; //467ah
	bool m_bCtrl[387]; //47fdh, for each array entry as above for keymap, is Ctrl- required?

	CVariableMap m_GlobalVariables; //4980h
	CVariableMap m_GlobalNPCScriptNames; //4988h, values are enums

	CStrRef soundNeedParty; //4990h
	CStrRef soundAreaTransitionRefused; //49feh

	char u4a6c;
	char u4a6d;

	//to do with PathSearch
	CVidCell vcLocator; //4a6eh
	IECPtrList searchRequests;      //4b44h, AAD264 (of CSearchRequests)
	IECPtrList searchRequestsBack;  //4b60h, AAD264
	int searchRequestListEmpty;     //4b7c
	int u4b80;
	int u4b84; //handle to Semaphore for SearchBitmapThread
#ifdef _DEBUG
	_CCriticalSection ccsItems; //4b88h, for access to m_ItemsToPurge
#else
	CCriticalSection ccsItems; //4b88h
#endif
	CItemList m_ItemsToPurge; //4ba8h
	short u4bc4;
	short u4bc6;
	Identifiers SVTRIOBJ; //4bc8h
	int u4c08;
	bool u4c0c; //when set, override script actions take precedence in party members (even if Continue())
	char u4c0d; //pad
	char u4c0e; //random number
	ResRef u4c0f;
	char u4c17;
	int u4c18; //KERNEL32.GetTickCount()
	int u4c1c; //KERNEL32.GetTickCount()
	int u4c20;
	int m_nRealTimer; //4c24h
	POINT u4c28;
	Enum u4c30;
	short u4c34;
	CVidPalette u4c36;
	CVidPalette u4c5a;
	int m_nDeathTimer;   //4c7eh, ticks until ending game on player1 death
	BOOL bNoDreamTonight; //4c82h, to do with dreams
	char u4c86;
	char u4c87;

	CMoveAreasList m_MoveAreasDelay; //4c88h, any MoveBetweenAreas() Cre has entry here
	CMoveAreasList m_MoveAreaNoDelay; //4ca4h
	IECPtrList u4cc0; //AA70B0
	IECString u4cdc;
	int u4ce0;
	int u4ce4;
	BOOL m_bTutorialGame; //4ce8h
	BOOL m_bThroneOfBhaalGame; //4cech - used to determine which song to play at start screen, 1 = use MP in STARTARE.2DA
	Enum m_eBaldurObject; //4cf0h, BALDUR.BCS CBaldurObject
	int m_nTimeStopObjectsTicksLeft; //4cf4h, remaining timestop (area sprites) ticks
	Enum m_eTimeStopExempt; //4cf8h, exempt from timestop (area sprites)
	int m_nTimeStopGreyscaleTicksLeft; //4cfch, remaining timestop (Greyscale) ticks
	int m_nDreamSepiaTicksLeft; //4d00h, remaining dream (Sepia) ticks
	CScriptParser* m_pScriptParser; //4d04h
	CStore* m_aServerStore[12]; //4d08h
	char m_nServerStoreDemands[12]; //4d38h
	IECString u4d44;
	IECString u4d48;
	IECString u4d4c;
	IECString u4d50;
	int u4d54[7];
	int u4d70;
	IECString u4d74;
	int u4d78;
	BOOL m_bThroneOfBhaal; //4d7ch - used to determine if 25-prefixed files are used
	int u4d80;

	CPartyLocationList cplStorePartyLoc; //4d84h, used in Store/RestorePartyLocations(), CPartyLocation0 (0x0 Resref areaname, 0x8 POINT)
	CPartyLocationList cplPrePocketPlane; //4da0h, used in ExitPocketPlane() and Cutscene2 effect, CPartyLocation1 (0x0 Resref areaname, 0x8 POINT)

	struct CVvcMapEntry { //size 12h
		ResRef r;
		short u8;
		int ua;
		int ue;
	};

	struct CVvcMap { //size 8h
	//Constructor: 0x64B3C1
		CVvcMapEntry* pArray; //VvcMapEntry
		int nElements;
	} m_CVvcMap; //4dbch

	int u4dc4; //? compared with CVisualEffectVidCell.u39c*/
};

class CSavedGameHeader { // Size=0xac
public:
    unsigned long  worldTime;           //0
    short          curFormation;        //4
    short          quickFormations[5];  //6
    unsigned long  nPartyGold;          //10
    unsigned short nPCAreaViewed;       //14
    unsigned short wWeatherFlags;       //16
    unsigned long  partyCreatureTableOffset;    //18
    unsigned long  partyCreatureTableCount;     //1c
    unsigned long  partyInventoryTableOffset;   //20
    unsigned long  partyInventoryTableCount;    //24
    unsigned long  globalCreatureTableOffset;   //28
    unsigned long  globalCreatureTableCount;    //2c
    unsigned long  globalVariablesOffset;       //30
    unsigned long  globalVariablesCount;        //34
    unsigned char  currentWorldArea[8];     //38
    unsigned long  currentLink;             //40
    unsigned long  journalEntriesCount;     //44
    unsigned long  journalEntriesOffset;    //48
    long           reputation;              //4c
    unsigned char  masterArea[8];           //50

    // bit0  Party AI Disabled
    // bit1  Larger Text Window
    // bit2  Largest Text Window
    // bit3  *not used*
    // bit4  Full Screen
    // bit5  Left Panel Hidden
    // bit6  Right Panel Hidden
    // bit7  Automap notes hidden

    // TobEx new:
    // bit8  On Pause
    // bit9  Nightmare active
    // bit10 Grey Background On Pause
    unsigned long  dwFlags;                 //58

    unsigned long  versionNumber;           //5c
    unsigned long  familiarsOffset;         //60
    unsigned long  storedLocationsOffset;   //64
    unsigned long  storedLocationsCount;    //68
    unsigned long  gameTime;                //6c
    unsigned long  storedLocationsOffsetPocketPlane; //70
    unsigned long  storedLocationsCountPocketPlane;  //74
    char           unknow[0x34];                     //78
};

class CSavedGamePartyCreature { // Size=0x160
public:
    unsigned short m_wFlags;// Offset=0x0 Size=0x2
    unsigned short m_portraitId;// Offset=0x2 Size=0x2
    unsigned long m_creatureOffset;// Offset=0x4 Size=0x4
    unsigned long m_creatureSize;// Offset=0x8 Size=0x4
    unsigned char m_creatureResRef[8];// Offset=0xc Size=0x8
    unsigned long m_creatureFacing;// Offset=0x14 Size=0x4
    unsigned char m_areaName[8];// Offset=0x18 Size=0x8
    unsigned short m_posX;// Offset=0x20 Size=0x2
    unsigned short m_posY;// Offset=0x22 Size=0x2
    unsigned short m_posViewX;// Offset=0x24 Size=0x2
    unsigned short m_posViewY;// Offset=0x26 Size=0x2
    unsigned short m_nModalState;// Offset=0x28 Size=0x2
    short m_nHappiness;// Offset=0x2a Size=0x2
    long m_nNumberOfTimesInteractedWith[24];// Offset=0x2c Size=0x60
    short m_quickWeaponsItemNum[4];// Offset=0x8c Size=0x8
    short m_quickWeaponsAbilityNum[4];// Offset=0x94 Size=0x8
    unsigned char m_quickSpellsSpellId[3][8];// Offset=0x9c Size=0x18
    short m_quickItemsItemNum[3];// Offset=0xb4 Size=0x6
    short m_quickItemsAbilityNum[3];// Offset=0xba Size=0x6
    char m_name[32];// Offset=0xc0 Size=0x20
    unsigned long m_numberTimesTalkedTo;// Offset=0xe0 Size=0x4
    unsigned long m_strStrongestKillName;// Offset=0xe4 Size=0x4
    unsigned long m_nStrongestKillXPValue;// Offset=0xe8 Size=0x4
    unsigned long m_nPreviousTimeWithParty;// Offset=0xec Size=0x4
    unsigned long m_nJoinPartyTime;// Offset=0xf0 Size=0x4
    unsigned char m_bWithParty;// Offset=0xf4 Size=0x1
    char m_pad1[2];// Offset=0xf5 Size=0x2
    char m_cFirstResSlot;// Offset=0xf7 Size=0x1
    unsigned long m_nChapterKillsXPValue;// Offset=0xf8 Size=0x4
    unsigned long m_nChapterKillsNumber;// Offset=0xfc Size=0x4
    unsigned long m_nGameKillsXPValue;// Offset=0x100 Size=0x4
    unsigned long m_nGameKillsNumber;// Offset=0x104 Size=0x4
    unsigned char m_lSpellStatsName[4][8];// Offset=0x108 Size=0x20
    short m_lSpellStatsCount[4];// Offset=0x128 Size=0x8
    unsigned char m_lWeaponStatsName[4][8];// Offset=0x130 Size=0x20
    short m_lWeaponStatsCount[4];// Offset=0x150 Size=0x8
    unsigned char m_secondarySounds[8];// Offset=0x158 Size=0x8
};


extern void (CInfGame::*CInfGame_InitGame)(int, int);
extern int (CInfGame::*CInfGame_GetNumOfItemInBag)(ResRef&, ResRef&, BOOL);
extern void (CInfGame::*CInfGame_DemandServerStore)(ResRef&, BOOL);
extern void (CInfGame::*CInfGame_ReleaseServerStore)(ResRef&);
extern short (CInfGame::*CInfGame_GetPartyMemberSlot)(Enum);
extern CArea& (CInfGame::*CInfGame_GetLoadedArea)(IECString);
extern void (CInfGame::*CInfGame_AddExperienceParty)(int);
extern void (CInfGame::*CInfGame_SetLoseCutscene)();
extern void (CInfGame::*CInfGame_StorePartyLocations)(BOOL);
extern int (CInfGame::*CInfGame_GetNumControlledSummons)();

#endif //INFGAME_H