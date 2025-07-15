#ifndef DATATYPES_H
#define DATATYPES_H

#include "stdafx.h"
#include "resref.h"

//Bam, Bah
struct BamFileFrameEntry { //Size Ch
	short width;
	short height;
	short x;
	short y;
	uint  dwFlags;
};

//Bif
struct BifFileHeader {  //Size 14h
	int  dwFileType;    //0h, of mmioFOURCC type
	int  dwVersion;     //4h
	int  dwFiles;       //8h
	int  dwTiles;       //ch
	int* pFiles;        //10h
};

struct BifFileEntry {        //Size 10h
	int   dwResourceLocator; //0h
	int*  pData;             //4h
	int   dwSize;            //8h
	short dwFileType;        //ch
	short eh;                //eh
};

struct BifTileEntry {        //Size 14h
	int   dwResourceLocator; //0h
	int*  pData;             //4h
	int   dwTiles;           //8h
	int   dwSize;            //ch
	short dwFileType;        //10h
	short u12;               //12h
};

//Chu
struct ChuFileControlInfoBase { //Size Eh
	int   id;       //0h
	short posX;     //4h
	short posY;     //6h
	short width;    //8h
	short height;   //ah
	char  type;     //ch
	char  ud;       //dh
};

//Cre
struct ColorRangeValues {       //Size 7h
	unsigned char colorMetal;   //0h
	unsigned char colorMinor;   //1h
	unsigned char colorMajor;   //2h
	unsigned char colorSkin;    //3h
	unsigned char colorLeather; //4h
	unsigned char colorArmor;   //5h
	unsigned char colorHair;    //6h
};

struct CreFileData {            //Size 268h
	STRREF longNameStrRef;      //0h, dialogue
	STRREF shortNameStrRef;     //4h, strref tooltip
	unsigned long dwFlags;      //8h
	unsigned long XPForKill;    //ch
	unsigned long PowerLevel_or_PersonalXP; //10h
	unsigned long gold;         //14h
	unsigned long stateFlags;   //18h
	short          currentHP;   //1ch
	unsigned short maxHP;       //1eh
	unsigned long animId;       //20h
	ColorRangeValues colors;    //24h
	unsigned char effType;      //2bh, 0 = ITEM_EFFECT (Effect), 1 = CEffect
	ResRef smallPortrait;       //2ch
	ResRef largePortrait;       //34h
	unsigned char reputation;   //3ch
	unsigned char hideInShadows;//3dh
	short ArmorClass;               //3eh, ACNatural
	short ArmorClassBase;           //40h, ACEffective
	short ArmorClassCrushingMod;    //42h
	short ArmorClassMissileMod;     //44h
	short ArmorClassPiercingMod;    //46h
	short ArmorClassSlashingMod;    //48h
	char  THAC0;                    //4ah
	unsigned char numAttacks;   //4bh
	unsigned char saveDeath;    //4ch
	unsigned char saveWands;    //4dh
	unsigned char savePoly;     //4eh
	unsigned char saveBreath;   //4fh
	unsigned char saveSpell;    //50h
	char resistFire;
	char resistCold;
	char resistElectricity;
	char resistAcid;
	char resistMagic;
	char resistMagicFire;
	char resistMagicCold;
	char resistSlashing;
	char resistCrushing;
	char resistPiercing;
	char resistMissile;
	unsigned char detectIllusion; //5ch
	unsigned char setTraps;       //5dh
	unsigned char lore;           //5eh
	unsigned char lockpicking;    //5fh
	unsigned char stealth;        //60h
	unsigned char findTraps;      //61h
	unsigned char pickPockets;    //62h
	unsigned char fatigue;        //63h, fatigue above this value results in fatigue
	unsigned char intoxication;   //64h
	char luck;              //65h
	char BG1ProfLSword;     //unused
	char BG1ProfSSword;     //unused
	char BG1ProfBows;       //unused
	char BG1ProfSpears;     //unused
	char BG1ProfBlunt;      //unused
	char BG1ProfSpiked;     //unused
	char BG1ProfAxe;        //unused
	char BG1ProfMissile;    //unused
	char u6e;
	char u6f;
	char u70;
	char u71;
	char u72;
	char u73;
	char u74;
	bool bNightmareModeStats;       //75h
	char u76;
	char u77;
	char u78;
	char u79;
	unsigned char undeadLevel;
	unsigned char tracking;
	char trackingTarget[32];
	STRREF soundset[100];           //9ch
	unsigned char levelPrimary;     //22ch
	unsigned char levelSecondary;   //22dh
	unsigned char levelTertiary;
	unsigned char sex;              //22fh
	unsigned char strength;
	unsigned char strengthEx;
	unsigned char intelligence;
	unsigned char wisdom;
	unsigned char dexterity;
	unsigned char constitution;     //235h
	unsigned char charisma;
	unsigned char morale;           //237h, (Cre+62D) range: 0-20, add -1 when PC dies
	unsigned char moraleBreak;
	unsigned char racialEnemy;      //239h
	unsigned short moraleRecoveryTime;
    union {
        unsigned int dwReversedkit;
        struct {
            unsigned short kitHigh; //23ch, [high WORD.low WORD]
            unsigned short kitLow;
            };
        };
	ResRef scriptOverride;          //240h
	ResRef scriptClass;
	ResRef scriptRace;              //250h
	ResRef scriptGeneral;
	ResRef scriptDefault;           //260h
};

struct CreFileMemorizedSpellLevel { //Size 10h
	unsigned short wLevel;          //0h
	unsigned short wMaxMemSpells;   //2h
	unsigned short wCurrMemSpells;  //4h
	unsigned short magicType;       //6h, 0 = priest, 1 = wizard, 2 = innate
	unsigned long  index;           //8h, of spell table
	unsigned long  nNumSpells;      //ch, count of spells
};

struct CreFileKnownSpell {  //Size Ch
	ResRef name;    //0h
	short level;    //8h
	short type;     //ah
};

struct CreFileMemSpell {    //Size Ch
	ResRef         name;

	//bit0: memorised
	//bit1: is a SPIN107, 108, 109, 110, 111 (a Paladin, Ranger, Druid special innate)
	unsigned short wFlags; //8h, bit0: MEMSPELL_MEMORIZED
	short          ua;     //pad
};

//Dlg
struct DlgFileState {       //Size 10h
	STRREF  strref;         //0h
	int     nResponseIdx;   //4h, first response index
	int     nResponses;     //8h
	int     nTriggerIdx;    //ch
};

struct DlgFileResponse {        //Size 20h
	unsigned int flags;         //0h
	STRREF       strref;        //4h
	STRREF       strrefJournal; //8h
	int          nTriggerIdx;   //ch
	int          nActionIdx;    //10h
	ResRef       NextDialog;    //14h
	int          nNextDialogState; //1ch
};

//Eff
struct EffFileData { //size 108h
//AKA CGameEffectBase
	ResRef       rHeader;       //0h
	unsigned int nOpcode;       //8h
	unsigned int nTargetType;   //ch
	unsigned int nSpellLvl;     //10h
	int          nParam1;       //14h   // 0x4 effect file offset
	unsigned int nParam2;       //18h   // 0x8 effect file offset

	//note, 'set duration' will always set to a gameTime
	//0: convert to 1000, Apply now, set duration [sec]
	//1: Apply now, add to main effect list (permanent until death)
	//2: Apply now, add to equipped effect list (while equipped)
	//3: convert to 6, Apply later, set duration [sec]
	//4: convert to 7, Apply later, set duration [sec]
	//5: convert to 8, Apply later, set duration [sec]
	//6: if gameTime >= duration (ticks), convert to 1000, Apply now set duration [sec]
	//7: if gameTime >= duration (ticks), convert to 1, Apply now
	//8: if gameTime >= duration (ticks), convert to 2, Apply now
	//9: Apply now, will not be removed from CEffectList
	//10: convert to 1000, Apply now, set duration (duration in ticks)
	//1000: if gameTime < duration (ticks), Apply; else purge
	unsigned int nTiming;       //1ch, in seconds (converted to ticks)

	unsigned int   nDuration;   //20h
	unsigned short wProbHigh;   //24h
	unsigned short wProbLow;    //26h
	ResRef         rResource;   //28h
	unsigned int   nDice;       //30h
	unsigned int   nDieSides;   //34h
	unsigned int   nSaveType;   //38h
	int            nSaveBonus;  //3ch
	unsigned int   nSpecial;    //40h
	unsigned int   nSchool;     //44h
	unsigned int   JeremyIsAnIdiot; //48h
	unsigned int   nLevelMin;   //4ch
	unsigned int   nLevelMax;   //50h
		
	//bit0: CEFFECTFLAG_DISPELLABLE
	//bit1: CEFFECTFLAG_IGNORE_RESISTANCE
	//bit2: CEFFECTFLAG_IGNORE_SPELL_LVL_RESISTANCE (only for BounceSplLvl[Dec], ProtSplLvl[Dec], and SplLvlTrap; does not affect ProtSplLvl)
	unsigned int    dwFlags;    //54h, DispelResistFlags
		
	int             nParam3;    //58h
	int             nParam4;    //5ch
	int             nParam5;    //60h
	int             nParam6;    //64h
	ResRef          rResource2; //68h
	ResRef          rResource3; //70h
	POINT           ptSource;   //78h
	POINT           ptDest;     //80h
	unsigned int    nParentResourceType;   //88h, 0 = none, 1 = SPL, 2 = ITM
	ResRef          rParentResource;       //8ch
	unsigned int    dwParentFlags;         //94h, dwFlags of SplFileData or dwFlags of ItmFileAbility
	unsigned int    nParentProjectileType; //98h, PROJECTL.IDS
	int             nParentItemSlot;       //9ch
	char            ScriptName[32];        //a0h
	unsigned int    nSourceCreCasterLevel; //c0h
	BOOL            bFirstCall;            //c4h, all effects start with 1, some opcodes will check this, do once off stuff, and then set to 0
	unsigned int    nTypeSecondary;        //c8h
	unsigned int    pad[15];               //cch
};

//Itm
struct ItmFileAbility { //Size 38h
	ushort attackType;      //0h
	uchar  quickSlotType;   //2h
	uchar  largeDamageDice; //3h
	ResRef useIcon;         //4h
	uchar  targetType;      //ch
	uchar  targetCnt;       //dh
	ushort range;           //eh
    uchar  launcherType;    //10h
    uchar  largeDamageDiceCount;//11h
	uchar  speed;           //12h
    uchar  largeDamageDiceBonus;//13h
	short  toHitBonus;      //14h
	uchar   sizeDice;       //16h
	uchar   nType1;         //17h
	uchar   nDice;          //18h
	uchar   nType2;         //19h
	short  damBonus;        //1ah
	ushort  damType;        //1ch
	ushort  nEffects;       //1eh
	ushort  offEffects;     //20h
	ushort  charges;        //22h
	ushort  chargeType;     //24h
	uint    flags;          //26h
	ushort  missileType;    //2ah
	ushort  meleeIdx[3];    //2ch
	ushort  isArrow;        //32h
	ushort  isBolt;         //34h
	ushort  isMiscProj;     //36h
};

struct ITEM_EFFECT {            //Size 30h, ItmFileEffect, SplFileEffect
	void* operator new(size_t size);
	void operator delete(void* mem);

	short opcode;               //0h
	char  target;               //2h
	char  power;                //3h
	int   param1;               //4h
	int   param2;               //8h
	char  timing;               //ch

	//bit0: Dispellable
	//bit1: Ignore Resistance
	uchar   flags;              //dh

	int     duration;           //eh
	char    highProb;           //12h
	char    lowProb;            //13h
	ResRef  resource;           //14h
	int     numDiceOrMaxLevel;  //1ch
	int     sizeDiceOrMinLevel; //20h
	uint    saveType;           //24h
	int     saveBonus;          //28h
	int     special;            //2ch
};

//Spl
struct SplFileAbility { //Size 28h
	char    type;       //0h
	char    u1;
	char    loc;        //2h
	char    u3;
	ResRef  memIcon;    //4h
	char    targetType; //ch
	char    targetNum;  //dh
	short   range;      //eh
	short   wMinLevel;  //10h
	int     castSpeed;  //12h
	short   sizeDice;   //16h
	short   nDice;      //18h
	short   enchanted;  //1ah
	short   damType;    //1ch
	short   wNumEffects;//1eh
	short   wEffectIdx; //20h
	short   nCharges;   //22h
	short   chargeType; //24h
	short   projIdx;    //26h
};

//Sto
struct StoFileData {            //Size 94h
	int     nType;              //0h
	STRREF  strrefName;         //4h
	uint    dwFlags;            //8h
	int     nSellPercent;       //ch
	int     nBuyPercent;        //10h
	int     nDepreciationRate;  //14h
	short   nStealFailPercent;  //18h
	short   nMaxItems;          //1ah
	long    u1c[2];             //1ch
	int*    pItemTypesBought;   //24h
	int     nItemTypesBought;   //28h
	int*    pItemsSold;         //2ch
	int     nItemsSold;         //30h
	int     nLore;              //34h
	int     nPriceID;           //38h
	ResRef  rRumoursTavern;     //3ch
	int*    pDrinks;            //40h
	int     nDrinks;            //48h
	ResRef  rRumoursTemple;     //4ch
	int     dwFlagsRoom;        //54h
	int     nPriceRoomPeasant;  //58h
	int     nPriceRoomMerchant; //5ch
	int     nPriceRoomNoble;    //60h
	int     nPriceRoomRoyal;    //64h
	int*    pSpells;            //68h
	int     nSpells;            //6ch
	int u70[9];
};

struct StoFileBuyType { //Size 4h
	int nType;
};

struct StoFileDrink {   //Size 14h
//corresponds to 'Drinks for sale' extended header
	ResRef rRumour;     //0h
	STRREF strrefName;  //8h
	int    nPrice;      //ch
	int    nStrength;   //10h
};

struct StoFileItem {    //Size 1Ch
//corresponds to 'Item for sale' extended header
	ResRef rName;       //0h
	short  wWear;       //8h, 0xFF + time until recharge in absolute game hours
	short  wUsage[3];   //ah
	uint   dwFlags;     //10h
	int    nNumInStock; //14h
	BOOL   bInfinite;   //18h
};

struct StoFileSpell {   //Size 10h
//corresponds to 'Cures for sale' extended header
	ResRef  rSpell;     //0h
	int     nPrice;     //8h
};

//Vef
struct VefFileComponent {   //Size E0h
	int     nStartDelay;    //0h, CVisualEffect frame to start
	//the specific AiUpdate to reload the animation
	//i.e. compares to CVisualEffect.u272, when u272 == this, then is reloaded

	int     u4;             //unused
	int     nRestartTime;   //8h, how many ticks to reload
	//if 0, purge this VefComponent; otherwise, u0+=u8

	int     type;           //ch, 1 = load Vvc/Bam, 2 = CreateVisualEffect() from resref (Vef/Vvc/Bam), else = play sound in resref
	ResRef  resource;       //10h, vvc/bam/wav
	uint    dwFlags;        //18h, bit0: multicycle, sets uae of CVefVidCell
	int     u1c[49];        //unmarshalled but unused?
};

//Vvc
struct VvcFileData {            //Size 1E4h
	ResRef  nameAnim1;          //0h
	ResRef  nameAnim2;          //8h
	uint    displayFlags;       //10h
	int     u14;
	uint    seqFlags;           //18h
	int     u1c;
	POINT   ptOffset;           //20h
	BOOL    bUseOrientation;    //28h
	int     nFrameRate;         //2ch
	int     nOrientations;      //30h
	int     nBaseOrientation;   //34h
	uint    posFlags;           //38h
	ResRef  nameBitmapPalette;  //3ch
	int     zPos;               //44h
	POINT   ptCentre;           //48h
	int     glowBrightness;     //50h
	int     duration;           //54h
	ResRef  name;               //58h, unused
	int     animCycleBegin;     //60h
	int     animCycleMiddle;    //64h
	int     currCycle;          //68h
	BOOL    bMultiCycle;        //6ch unused
	ResRef  soundBegin;         //70h
	ResRef  soundMiddle;        //78h
	ResRef  nameAnim3;          //80h
	int     cycleAnimEnd;       //88h
	ResRef  soundEnd;           //8ch
	char    u94[0x150];         //unused
};


struct CAbilityId       // Size=0x14
{
    short  itemType;    //0x0
    short  itemNum;     //0x2
    short  abilityNum;  //0x4
    ResRef ResName;     //0x6
    uchar  targetType;  //0xe
    uchar  targetCount; //0xf
    ulong  toolTip;     //0x10
};


#endif //DATATYPES_H