#ifndef ANIMEXT_H
#define ANIMEXT_H

#include "animcore.h"
#include "datatypes.h"
#include "cstringex.h"
#include "arecore.h"

class CAnimation0000 : public CAnimation { //Size 976h CGameAnimationTypeEffect
//Constructor: 0x7FAA53
public:
    //AB622C
    virtual ~CAnimation0000() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 { return true; }

    CVidCell* pCurrentVidCell; //6d8h
    CVidCell* pCurrentVidCellShadow; //6dch
    CVidCell g1VidCell; //6e0h
    CVidCell g1VidCellShadow; //7b6h
    CVidPalette pColorRange; //88ch
    bool bTranslucent;   //b80h
    bool bRender;       //8b1h
    ushort posZ;        //8b2h
    ushort deltaZ;      //8b4h
    short wCurrentAnimationIdx; //8b6h
    short wCurrentOrientation; //8b8h
    uchar nOrientations; //8bah
    uchar u8bb; //pad
    BOOL  bNewPalette;   //8bch
    CVidBitmap newPalette; //8c0h
};

class CAnimation1000 : public CAnimation { //Size 73Ah CGameAnimationTypeMonsterQuadrant
//0x10**, 0x11**
//Constructor: 0x8356F3
public:
    //AB6C00
    virtual ~CAnimation1000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h

    //each contains nVidCells
    CVidCell* pG1VidCellBase; //6e4h
    CVidCell* pG2VidCellBase; //6e8h
    CVidCell* pG3VidCellBase; //6ech

    CVidCell* currentVidCellExtend; //6f0h
    //each contains nVidCells
    CVidCell* pG1VidCellExtend; //6f4h
    CVidCell* pG2VidCellExtend; //6f8h
    CVidCell* pG3VidCellExtend; //6fch

    CVidPalette pColorRange; //700h
    short wCurrentAnimationIdx; //724h
    short wCurrentOrientation; //726h
    BOOL bUseColorRange; //728h
    char nOrientations; //72ch, if wOrient > nOrient, use Extended VidCell
    char nParts; //72d, # parts per vidcell (e.g. MTANG1[1-4])
    BOOL bCaster; //72eh
    BOOL bUseExtendedVidCells; //732h
    BOOL bPathSmooth; //736h
};

extern LPCTSTR (CAnimation1000::*CAnimation1000_GetWalkingSound)(short);

class CAnimation1200 : public CAnimation { //Size AC6h CGameAnimationTypeMonsterMulti
//0x12**, 0x1X**
//Constructor: 0x82C8AE
public:
    //AB6B1C
    virtual ~CAnimation1200() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h

    CVidCell* pG1VidCellBase; //6e4h
    CVidCell* pG2VidCellBase; //6e8h
    CVidCell* pG3VidCellBase; //6ech
    CVidCell* pG4VidCellBase; //6f0h
    CVidCell* pG5VidCellBase; //6f4h

    CVidPalette pColorRange; //6f8h
    short wCurrentAnimationIdx; //71ch
    short wCurrentOrientation; //71eh
    BOOL bUseNewBitmaps;   //720h
    CVidBitmap* currentNewBitmap; //724h
    BOOL bUseColorRange; //728h
    CVidBitmap NewBitmap1; //72ch
    CVidBitmap NewBitmap2; //7e2h
    CVidBitmap NewBitmap3; //898h
    CVidBitmap NewBitmap4; //94eh
    CVidBitmap NewBitmap5; //a04h
    char nOrientations; //abah, orientations until mirror
    char nParts; //abbh, nQuadrants
    BOOL bDoubleBlit; //abch
    BOOL bSplitBams; //ac0h
    char splitBamChar; //ac4h
    char splitDirectionBamChar; //ac5h
};

extern LPCTSTR (CAnimation1200::*CAnimation1200_GetWalkingSound)(short);

class CAnimation1300 : public CAnimation { //Size 7F4h CGameAnimationTypeMonsterMultiNew
//0x13**
//Constructor: 0x8746B1
public:
    //AB7158
    virtual ~CAnimation1300() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28  return to_bool(bCanUseMiddleVertList); 
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch

    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pG1VidCellBase; //6e4h
    CVidCell* pG2VidCellBase; //6e8h

    CVidPalette pColorRange; //6ech
    short wCurrentAnimationIdx; //710h
    short wCurrentOrientation; //712h
    short wShootAnimationIdx; //714h, = 3
    int u716;
    BOOL bUseColorRange; //71ah
    CVidBitmap u71e;
    char nOrientations; //7d4h
    char nParts; //7d5h
    int u7d6;
    char u7da;
    char u7db;
    int u7dc;
    int u7e0;
    int u7e4;
    uchar bDetectedByInfravision; //7e8h
    char u7e9;
    BOOL bCanUseMiddleVertList; //7eah, retrieved as char (set to 1)
    int u7ee;
    char u7f2;
    char u7f3;
};

extern LPCTSTR (CAnimation1300::*CAnimation1300_GetWalkingSound)(short);

class CAnimation2000 : public CAnimation { //Size E0Ch CGameAnimationTypeMonsterLayeredSpell
//Constructor: 0x83D12A
public:
    //AB6DC8
    virtual ~CAnimation2000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch

    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h
    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidCell G2VidCellBase; //894h
    CVidCell G2VidCellExtend; //96ah
    CVidPalette pColorRangeBase; //a40h

    CVidCell* pCurrentVidCellShadow; //a64h

    CVidCell* pCurrentVidCellShadowBase; //a68h
    CVidCell* pCurrentVidCellShadowExtend; //a6ch
    CVidCell G1VidCellShadowBase; //a70h
    CVidCell G1VidCellShadowExtend; //b46h
    CVidCell G2VidCellShadowBase; //c1ch
    CVidCell G2VidCellShadowExtend; //cf2h
    CVidPalette pColorRangeExtend; //dc8h

    short wCurrentAnimationIdx; //dech
    short wCurrentOrientation; //deeh
    BOOL  bRenderWeapons; //df0h
    int udf4;
    BOOL bUseColorRange; //df8h
    IECString udfc;
    IECString ue00;
    bool bInvulnerable; //e04h
    char ue05; //pad
    BOOL bDualAttack; //e06h
    char nOrientations; //e0ah
    char ue0a; //pad
};

extern LPCTSTR (CAnimation2000::*CAnimation2000_GetWalkingSound)(short);

class CAnimation3000 : public CAnimation { //Size 1106h CGameAnimationTypeMonsterAnkheg
//Constructor: 0x841063
public:
    //AB6EAC
    virtual ~CAnimation3000() {} //v0
    virtual BOOL GetAboveGround() {} //vc4 { return bAboveGround; } CanBeTargetted

    IECString sPrefix;          //6d8h
    CVidCell* pCurrentVidCell;  //6dch

    CVidCell* pCurrentVidCellBase;   //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4

    CVidCell G1VidCellBase;         //6e8h
    CVidCell G1VidCellExtend;       //7beh
    CVidCell G2VidCellBase;         //894h
    CVidCell G2VidCellExtend;       //96ah
    CVidCell G3VidCellBase;         //a40h
    CVidCell G3VidCellExtend;       //b16h

    CVidCell* pCurrentVidCellHole;       //bech
    CVidCell* pCurrentVidCellHoleBase;   //bf0h
    CVidCell* pCurrentVidCellHoleExtend; //bf4h

    CVidCell G1VidCellHoleBase;         //bf8h  *DG1
    CVidCell G1VidCellHoleExtend;       //cceh  *DG1E
    CVidCell G2VidCellHoleBase;         //da4h  *DG2
    CVidCell G2VidCellHoleExtend;       //e7ah  *DG2E
    CVidCell G3VidCellHoleBase;         //f50h  *DG3
    CVidCell G3VidCellHoleExtend;       //1026h *DG3E

    short wCurrentAnimationIdx; //10fch     m_currentBamSequence
    short wCurrentOrientation;  //10feh     m_currentBamDirection
    BOOL bAboveGround;          //1100h     bCanBeTargetted
    uchar nOrientations;        //1104h
    char u1105;                 //pad
};

class CAnimation4000 : public CAnimation { //Size 7E4h CGameAnimationTypeTownStatic
//Constructor: 0x801EDE
public:
    //AB63F4
    virtual ~CAnimation4000() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 return bCanUseMiddleVertList;

    CVidCell* pCurrentVidCellBase; //6d8h
    CVidCell* pCurrentVidCellExtend; //6dch
    
    CVidCell VidCell; //6e0h
    CVidPalette pColorRangeBase; //7b6h
    short wCurrentAnimationIdx; //7dah
    short wCurrentOrientation; //7dch
    BOOL bUseColorRange; //7deh
    bool bCanUseMiddleVertList; //7e2h
    char u7e3; //pad
};

class CAnimation5000 : public CAnimation { //Size 16E0h CGameAnimationTypeCharacter
//Playable humanoids
//0x5***, 0x6***, except 0x6400-0x6405
//Constructor: 0x843678
public:
    CAnimation5000(unsigned short wAnimId, ColorRangeValues& colors, int nOrientation);
    CAnimation5000& Construct(unsigned short, ColorRangeValues&, int) { return *this; }

    //AB6F90
    virtual ~CAnimation5000() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 return m_bCanUseMiddleVertList; 
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefixPaperDoll; //6d8h, C<RACE><SEX>B
    IECString sPrefix;          //6dch, C<RACE><SEX>B - main
    IECString sHeightCode;                //6e0h WQ* (single weapon prefix?)
    IECString sHeightCodeHelmet;          //6e4h WQ*
    IECString sHeightCodeShieldPaperDoll; //6e8h WQ*
    char cArmorCode;    //6ech, '1' - anim type of equipped armour
    char cArmorMaxCode; //6edh, '4'

    //main creature
    CVidCell* pvcMainCurrent; //6eeh
    CVidCell* pvcMainBase;    //6f2h
    CVidCell  vcMainGen;       //6f6h, G1
    CVidCell  vcMainCast;      //7cch
    CVidCell  vcMainAttack1;   //8a2h, A1/A2/A7 [1H/2H/dual-wield]
    CVidCell  vcMainAttack2;   //978h, A3/A4/A8
    CVidCell  vcMainAttack3;   //a4eh, A5/A6/A9
    CVidPalette charPalette;    //b24h
    
    //Weapon
    IECString sWeaponPrefix;        //b48h
    CVidCell* pvcWeaponCurrent;     //b4ch
    CVidCell* pvcWeaponCurrentBase; //b50h
    CVidCell  vcWeaponGen;          //b54h G1
    CVidCell  vcWeaponAttack1;      //c2ah A1
    CVidCell  vcWeaponAttack2;      //d00h A3
    CVidCell  vcWeaponAttack3;      //dd6h A5
    CVidPalette weaponPalette;      //each
    
    //Shield
    IECString sShieldPrefix;        //ed0h
    CVidCell* pvcShieldCurrent;     //ed4h
    CVidCell* pvcShieldCurrentBase; //ed8h
    CVidCell  vcShieldGen;          //edch  OG1
    CVidCell  vcShieldAttack1;      //fb2h  OA7
    CVidCell  vcShieldAttack2;      //1088h OA8
    CVidCell  vcShieldAttack3;      //115eh OA9
    CVidPalette shieldPalette;      //1234h

    //Helmet
    IECString sHelmetPrefix;        //1258h
    CVidCell* pvcHelmetCurrent;     //125ch
    CVidCell* pvcHelmetCurrentBase; //1260h
    CVidCell u1264;
    CVidCell u133a;
    CVidCell u1410;
    CVidCell u14e6;
    CVidCell u15bc;
    CVidPalette u1692;

    short wCurrentAnimation; //16b6h
    short wCurrentOrientation; //16b8h
    BOOL bRenderWeapons;  //16bah, bUseAuxiliaryVidCells;  group indices 1 and 2
    BOOL bRenderHelmet;   //16beh, bUseAuxiliaryVidCells2; group index 3
    BOOL bEquipHelmet;    //16c2h
    BOOL bWeaponLeftHand; //16c6h, set to 1 when ITEMFLAG_BOW is set
    BOOL bUseColorRange;  //16cah, for first group of VidCells, other groups automatically use VidPals
    
    //1: bow/arrow
    //2: xbow/bolt
    //3: sling/bullet
    //4: main weapon or no weapon
    //5: 2-handed weapon
    //11h: offhand only
    //13h: dual-wield
    //10h (bit4): MELEE_1HLR_MASK (16)
    char cWeaponCode;           //16ceh, controls which animations to load up into first group
    char nOrientations;         //16cfh
    uchar bDetectedByInfravision; //16d0
    bool bInvulnerable;         //16d1h
    bool bCanUseMiddleVertList; //16d2h, only SAREVOK (X404) is false
    char ArmorSuffix;           // 16d3h, armor level suffix char
    char ArmorSpecificSuffix;   // 16d4h, armor level suffix char
    char u16d5; //pad
    int  bDoubleBlit;       // 16d6h
    int  bSplitBams;        // 16dah
    char SplitBamSuffix;    // 16deh, final suffix (after armor level)
    char u16df; //pad, tobex new, bit0 W=1, bit1 W=0, mage marker for quick access without 2da search
};

extern CAnimation5000& (CAnimation5000::*CAnimation5000_Construct)(unsigned short, ColorRangeValues&, int);
extern LPCTSTR (CAnimation5000::*CAnimation5000_GetWalkingSound)(short);

class CAnimation6400 : public CAnimation { //Size 36B0h CGameAnimationTypeCharacterOld
//0x6400-0x6405
//Constructor: 0x8586AC
public:
    //AB7074
    virtual ~CAnimation6400() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 return m_bCanUseMiddleVertList;
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix;   //6d8h, C<RACE><SEX>C, group1
    IECString sShadowPrefix; //6dc, CSHD, group5
    IECString sHeightCode;       //6e0h WP*, WPL, group2 group3
    IECString sHeightCodeHelmet; //6e4h WP*, WPL, group4?
    char cArmorCode;    // 6e8h, '1' - anim type of equipped armour
    char cArmorMaxCode; // 6e9h, '4'

    //main creature animation (main prefix)
    CVidCell* pvcMainCurrent; //6eah
    CVidCell* pvcMainBase; //6eeh
    CVidCell* pvcMainExtend; //6f2h
    CVidCell  vcMainGenBase; //6f6h,        G1
    CVidCell  vcMainGenExtend; //7cch
    CVidCell  vcMainWalkBase; //8a2h,       W2
    CVidCell  vcMainWalkExtend; //978h
    CVidCell  vcMainCastBase; //a4eh
    CVidCell  vcMainCastExtend; //b24h
    CVidCell  vcMainAttack1Base; //bfah,    A1 (overhand)
    CVidCell  vcMainAttack1Extend; //cd0h
    CVidCell  vcMainAttack2Base; //da6h,    A3 (backhand)
    CVidCell  vcMainAttack2Extend; //e7ch
    CVidCell  vcMainAttack3Base; //f52h,    A5 (thrust)
    CVidCell  vcMainAttack3Extend; //1028h
    CVidPalette vpMainColorRanges; //10feh

    //common to all weapons, uses 6e0 prefix, WP*<itemanim>[O]... (O if offhand)
    IECString sWeaponPrefix; //1122h
    CVidCell* pvcWeaponCurrent; //1126h
    CVidCell* pvcWeaponBase; //112ah
    CVidCell* pvcWeaponExtend; //112eh
    CVidCell vcWeaponGenBase; //1132h
    CVidCell vcWeaponGenExtend; //1208h
    CVidCell vcWeaponWalkBase; //12deh
    CVidCell vcWeaponWalkExtend; //13b4h
    CVidCell vcWeaponAttack1Base; //148ah
    CVidCell vcWeaponAttack1Extend; //1560h
    CVidCell vcWeaponAttack2Base; //1636h
    CVidCell vcWeaponAttack2Extend; //170ch
    CVidCell vcWeaponAttack3Base; //17e2h
    CVidCell vcWeaponAttack3Extend; //18b8h
    CVidPalette vpWeaponColorRanges; //198eh

    //shield, uses 6e0 prefix
    IECString sShieldPrefix; //19b2h
    CVidCell* pvcShieldCurrent; //19b6h
    CVidCell* pvcShieldBase; //19bah
    CVidCell* pvcShieldExtend; //19beh
    CVidCell vcShieldGenBase; //19c2h
    CVidCell vcShieldGenExtend; //1a98h
    CVidCell vcShieldWalkBase; //1b6eh
    CVidCell vcShieldWalkExtend; //1c44h
    CVidCell vcShieldAttack1Base; //1d1ah
    CVidCell vcShieldAttack1Extend; //1df0h
    CVidCell vcShieldAttack2Base; //1ec6h
    CVidCell vcShieldAttack2Extend; //1f9ch
    CVidCell vcShieldAttack3Base; //1ec6h
    CVidCell vcShieldAttack3Extend; //2148h
    CVidPalette vpShieldColorRanges; //221eh

    //helmet, uses 6e4 prefix
    IECString sHelmetPrefix; //u2242
    CVidCell* pvcHelmetCurrent; //u2246
    CVidCell* pvcHelmetBase; //u224a
    CVidCell* pvcHelmetExtend; //u224e
    CVidCell vcHelmGenBase; //u2252
    CVidCell vcHelmGenExtend; //u2328
    CVidCell vcHelmWalkBase; //u23fe
    CVidCell vcHelmWalkExtend; //u24d4
    CVidCell vcHelmCastBase; //u25aa
    CVidCell vcHelmCastExtend; //u2680
    CVidCell vcHelmAttack1Base; //u2756
    CVidCell vcHelmAttack1Extend; //u282c
    CVidCell vcHelmAttack2Base; //u2902
    CVidCell vcHelmAttack2Extend; //u29d8
    CVidCell vcHelmAttack3Base; //u2aae
    CVidCell vcHelmAttack3Extend; //u2b84
    CVidPalette vpHelmColorRanges; //u2c5a

    //CSHD Shadow (if sprite mirror, Extends get X suffix)
    CVidCell* pvcShadowCurrent; //2c7eh
    CVidCell* pvcShadowBase; //2c82h
    CVidCell* pvcShadowExtend; //2c86h
    CVidCell vcShadowGenBase; //2c8ah
    CVidCell vcShadowGenExtend; //2d60h
    CVidCell vcShadowWalkBase; //2e36h
    CVidCell vcShadowWalkExtend; //2f0ch
    CVidCell vcShadowCastBase; //2fe2h
    CVidCell vcShadowCastExtend; //30b8h
    CVidCell vcShadowAttack1Base; //318eh
    CVidCell vcShadowAttack1Extend; //3264h
    CVidCell vcShadowAttack2Base; //333ah
    CVidCell vcShadowAttack2Extend; //3410h
    CVidCell vcShadowAttack3Base; //34e6h
    CVidCell vcShadowAttack3Extend; //35bch

    short wCurrentAnimation; //3692h
    short wCurrentOrientation; //3694h

    BOOL bRenderWeapons;  //3696h
    BOOL bRenderHelmet;   //369ah
    BOOL bEquipHelmet;    //369eh
    BOOL bWeaponLeftHand; //36a2h, set to 1 when ITEMFLAG_BOW is set
    BOOL bUseColorRange; //36a6h (toggle for first set only, others are always on)
    
    //local attack prob (size 3)
    //0: 1 = overhand, bow/arrow, xbow/bolt, sling/bullet
    //1: 1 = backhand
    //2: 1 = thrust

    //bit0+1: load A1 (sling/bullet)
    //bit0 only: load SA into A1 slot (bow/arrow)
    //bit1 only: load SX into A1 slot (xbow/bolt)
    //bit2: load A1 (one hand overhand)
    //bit3: load A3 (one hand backhand)
    //bit4: load A5 (one hand thrust)
    //bit5: load A2 into A1 slot (two hand overhand)
    //bit6: load A4 into A2 slot (two hand backhand)
    //bit7: load A6 into A5 slot (two hand thrust)
    char cWeaponCode; //36aah
    char nOrientations; //36abh
    uchar bDetectedByInfravision; //36ach
    char bInvulnerablel; //36adh
    bool bCanUseMiddleVertList; //36aeh, only SAREVOK (X404) is false
    char u36af; //pad, tobex new, mage marker for quick access without 2da search
};

extern LPCTSTR (CAnimation6400::*CAnimation6400_GetWalkingSound)(short);

class CAnimation7000 : public CAnimation { //Size B2Ch CGameAnimationTypeMonsterOld
//0x7[015CE]*[01], 0x72*[0-3], 0x7[47]*[0-2], 0x7[68D]*0, 0x7[9A]*[0-4], 0x7B*[0-6]
//Constructor: 0x8176A5
public:
    //AB678C
    virtual ~CAnimation7000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch

    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h
    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidCell G2VidCellBase; //894h
    CVidCell G2VidCellExtend; //96ah
    CVidPalette pColorRangeBase; //a40h

    short wCurrentAnimation; //a64h
    short wCurrentOrientation; //a66h
    BOOL bUseColorRange;    //a68h
    BOOL bTranslucent;      //a6ch
    BOOL bNewPalette;       //a70h
    CVidBitmap newPalette;  //a74h
    uchar bDetectedByInfravision; //ub2ah
    uchar nOrientations; //b2bh
};

extern LPCTSTR (CAnimation7000::*CAnimation7000_GetWalkingSound)(short);

class CAnimation7300 : public CAnimation { //Size D3Ch CGameAnimationTypeMonster
//0x7300, 0x7703, 0x7F00, all outliers of above
//Constructor: 0x807184
public:
    //AB66A0
    virtual ~CAnimation7300() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 return to_bool(bCanUseMiddleVertList);
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell *pCurrentVidCell; //6dch
    CVidCell *pCurrentVidCellBase; //6e0h
    CVidCell  G1VidCellBase; //6e4h
    CVidCell  G2VidCellBase; //7bah
    CVidPalette pColorRangeBase; //890h

    CVidCell* pCurrentVidCellWeapon; //8b4h
    CVidCell* pCurrentVidCellWeaponBase; //8b8h
    CVidCell  G1VidCellWeaponBase; //8bch
    CVidCell  G2VidCellWeaponBase; //992h
    CVidPalette vpWeaponPalette; //a68h

    short   wCurrentAnimation; //a8ch
    short   wCurrentOrientation; //a8eh
    short   wShootAnimationIdx; //a90h, = 3 or 4
    BOOL    bUseColorRange; //a92h
    BOOL    bTranslucent; //a96h
    BOOL    bNewPalette; //a9ah
    CVidBitmap ua9e;
    BOOL    bTwoPalettes; //b54h
    ResRef  cResRefPalette1; //b58h
    ResRef  cResRefPalette2; //b60h
    BOOL    bRenderWeapons; //b68h
    int     ub6c;  // bHideWeapons/weaponLeftHand
    uchar   bDetectedByInfravision; //b70h
    uchar   nOrientations; //b71h
    BOOL    bCanUseMiddleVertList; //b72h, retrieved as char (always 1)
    BOOL    bPathSmooth;   //b76h
    BOOL    bSplitBams;    //b7ah
    char    splitBamChar;  //b7eh
    char    currentShootSplit; //b7fh

    BOOL      bGlowLayer; //b80h, bUseSecondAnim, refers to below
    IECString sGlowPrefix; //b84h, if DisableBrightest == 1
    CVidCell* pGlowCurrentVidCell; //b88h
    CVidCell* pGlowCurrentVidCellBase; //b8ch
    CVidCell  GlowG1VidCellBase; //b90h
    CVidCell  GlowG2VidCellBase; //c66h
};

extern LPCTSTR (CAnimation7300::*CAnimation7300_GetWalkingSound)(short);

class CAnimation8000 : public CAnimation { //Size E02h CGameAnimationTypeMonsterLayered
//Constructor: 0x839BE6
public:
    //AB6CE4
    virtual ~CAnimation8000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h

    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidCell G2VidCellBase; //894h
    CVidCell G2VidCellExtend; //96ah
    CVidPalette pColorRange; //a40h

    CVidCell* ua64;
    CVidCell* ua68;
    CVidCell* ua6c;
    CVidCell ua70;
    CVidCell ub46;
    CVidCell uc1c;
    CVidCell ucf2;
    CVidPalette udc8;

    short wCurrentAnimation; //dech
    short wCurrentOrientation; //deeh
    int udf0;
    int udf4;
    char udf8;
    char nOrientations; //df9h
    IECString udfa; //S1/SS
    IECString udfe; //HB/BW
};

extern LPCTSTR (CAnimation8000::*CAnimation8000_GetWalkingSound)(short);

class CAnimation9000 : public CAnimation { //Size C1Ah CGameAnimationTypeMonsterLarge
//Constructor: 0x828165
public:
    //AB6954
    virtual ~CAnimation9000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h

    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidCell G2VidCellBase; //894h
    CVidCell G2VidCellExtend; //96ah
    CVidCell G3VidCellBase; //a40h
    CVidCell G3VidCellExtend; //b16h
    CVidPalette pColorRange; //bech

    short wCurrentAnimation; //c10h
    short wCurrentOrientation; //c12h
    BOOL bUseColorRange; //c14h
    char nOrientations; //c18h
    char uc19; //pad
};

extern LPCTSTR (CAnimation9000::*CAnimation9000_GetWalkingSound)(short);

class CAnimationA000 : public CAnimation { //Size C1Ah CGameAnimationTypeMonsterLarge16
//Constructor: 0x82A22F
public:
    //AB6A38
    virtual ~CAnimationA000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h

    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidCell G2VidCellBase; //894h
    CVidCell G2VidCellExtend; //96ah
    CVidCell G3VidCellBase; //a40h
    CVidCell G3VidCellExtend; //b16h
    CVidPalette pColorRange; //bech

    short wCurrentAnimation; //c10h
    short wCurrentOrientation; //c12h
    BOOL bUseColorRange; //c14h
    char nOrientations; //c18h
    char uc19; //pad
};

extern LPCTSTR (CAnimationA000::*CAnimationA000_GetWalkingSound)(short);

class CAnimationB000 : public CAnimation { //Size 8C2h CGameAnimationTypeAmbientStatic
//Constructor: 0x803352
public:
    //AB64D8
    virtual ~CAnimationB000() {} //v0

    CVidCell* pCurrentVidCell; //6d8h
    CVidCell* pCurrentVidCellBase; //6dch
    CVidCell* pCurrentVidCellExtend; //6e0h

    CVidCell G1VidCellBase; //6e4h
    CVidCell G1VidCellExtend; //7bah
    CVidPalette pColorRange; //890h

    short wCurrentAnimation; //8b4h
    short wCurrentOrientation; //8b6h
    BOOL bUseColorRange; //8b8h
    BOOL bInvulnerable; //8bch
    char nOrientations; //8c0h
    char u8c1; //pad
};


class CAnimationC000 : public CAnimation { //Size 8CAh CGameAnimationTypeAmbient
//Constructor: 0x804D2B
public:
    //AB65BC
    virtual ~CAnimationC000() {} //v0
    virtual char GetDefaultVertListType() {} //v3c { return nDefaultVertListType; } 
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h

    CVidCell G1VidCellBase; //6e8h
    CVidCell G1VidCellExtend; //7beh
    CVidPalette pColorRange; //894h

    short wCurrentAnimation; //8b8h
    short wCurrentOrientation; //8bah
    BOOL bUseColorRange; //8bch
    int u8c0;
    int u8c4;
    char nDefaultVertListType; //8c8h, always LIST_FRONT except for C500 is LIST_BACK
    char nOrientations; //8c9h
};

extern LPCTSTR (CAnimationC000::*CAnimationC000_GetWalkingSound)(short);

class CAnimationD000 : public CAnimation { //Size 7E4h CGameAnimationTypeFlying
//Constructor: 0x800D4F
public:
    //AB6310
    virtual ~CAnimationD000() {} //v0
    virtual bool CanUseMiddleVertList() {} //v28 { return false; }
    virtual char GetDefaultVertListType() {} //v3c { return LIST_BACK; }

    CVidCell* pCurrentVidCell; //6d8h
    CVidCell* pCurrentVidCellBase; //6dch
    CVidCell G1VidCellBase; //6e0h
    CVidPalette pColorRange; //7b6h

    short wCurrentAnimation; //7dah
    short wCurrentOrientation; //7dch
    BOOL bUseColorRange;; //7deh
    char nOrientations; //7e2h
    char u7e3; //pad
};

class CAnimationE000 : public CAnimation { //Size 3278h CGameAnimationTypeMonsterIcewind
//Constructor: 0x81C723
public:
    //AB6870
    virtual ~CAnimationE000() {} //v0
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c

    IECString sPrefix; //6d8h
    CVidCell* pCurrentVidCell; //6dch
    CVidCell* pCurrentVidCellBase; //6e0h
    CVidCell* pCurrentVidCellExtend; //6e4h

    CVidCell u6e8; //A1
    CVidCell u7be; //A1E
    CVidCell u894; //A2
    CVidCell u96a; //A2E
    CVidCell ua40; //A3
    CVidCell ub16; //A2E (bugged, should be A3E)
    CVidCell ubec; //A4
    CVidCell ucc2; //A4E
    CVidCell ud98; //Gu
    CVidCell ue6e; //GuE
    CVidCell uf44; //Sl
    CVidCell u101a; //SlE
    CVidCell u10f0; //De
    CVidCell u11c6; //DeE
    CVidCell u129c; //GH
    CVidCell u1372; //GHE
    CVidCell u1448; //Sd
    CVidCell u151e; //SdE
    CVidCell u15f4; //Sc
    CVidCell u16ca; //ScE
    CVidCell u17a0; //Sp
    CVidCell u1876; //SpE
    CVidCell u194c; //Ca
    CVidCell u1a22; //CaE
    CVidCell u1af8; //Tw
    CVidCell u1bce; //TwE
    CVidCell u1ca4; //Wk
    CVidCell u1d7a; //WkE

    CVidCell* pCurrentVidCell2; //1e50h
    CVidCell* pCurrentVidCellBase2; //1e54h
    CVidCell* pCurrentVidCellExtend2; //1e58h

    CVidCell u1e5c;
    CVidCell u1f32;
    CVidCell u2008;
    CVidCell u20de;
    CVidCell u21b4;
    CVidCell u228a;
    CVidCell u2360;
    CVidCell u2436;
    CVidCell u250c;
    CVidCell u25e2;
    CVidCell u26b8;
    CVidCell u278e;
    CVidCell u2864;
    CVidCell u293a;
    CVidCell u2a10;
    CVidCell u2ae6;
    CVidCell u2bbc;
    CVidCell u2c92;
    CVidCell u2d68;
    CVidCell u2e3e;
    CVidCell u2f14;
    CVidCell u2fea;
    CVidCell u30c0;
    CVidCell u3196;

    short wCurrentOrientation; //326ch
    BOOL bRenderWeapons; //326eh
    BOOL bWeaponLeftHand; //3272h
    uchar bDetectedByInfravision; //3276h
    uchar nOrientations; //3277h
};

extern LPCTSTR (CAnimationE000::*CAnimationE000_GetWalkingSound)(short);

#endif //ANIMEXT_H