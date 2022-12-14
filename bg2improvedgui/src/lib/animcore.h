#ifndef ANIMCORE_H
#define ANIMCORE_H

#include "stdafx.h"

#include "arecore.h"
#include "vidcore.h"

class CCreatureObject;


class CSequenceSound { // Size=0xc
public:
    ResRef sound;   //0h
    long   offset;  //8h
};

class CAnimationSoundList : public IECPtrList { //Size 28h
//Constructor: 0x87B3E0
public:
    //AACCA0
    BOOL PlaySound(int curFrame, CCreatureObject* pSprite);

    //Elements are 0xc size objects (0x0 ResRef sound [first row], 0x8 frameToPlay [second row])
    POSITION currentSound;  //1ch
    BOOL     bSoundPlaying; //20h
    long     Channel;       //24h
};


struct NECK_POINTS  //Size 4h
{
    short x;    //0
    short y;    //2
};

extern BOOL (CAnimationSoundList::*CAnimationSoundList_PlayPrimedSound)(int, CCreatureObject&);

class CAnimation { //Size 6D8h 
public:
    CAnimation(); //7F9211

    void PlayCurrentSequenceSound(CCreatureObject& cre);
    static CAnimation* CreateAnimation(unsigned short wAnimId, ColorRangeValues& colors, unsigned short wOrient);
    static BOOL IsPlayableAnimation(unsigned short wAnimId);

    //AB6148
    virtual ~CAnimation() {} //v0
    virtual void CalculateFxRect(RECT* pFrame, POINT* pCentre, int zPos) {} //v4 GetCurrentFrameDimensions
    virtual void CalculateGCBoundsRect() {} //v8 GetBoundsRect(RECT* pBounds, POINT* ppt1, POINT* ppt2, zPos, width, height)
    virtual BOOL SetOrientation(short wOrient) {} //vc ChangeDirection
    virtual void EquipArmor(char cArmorLevel, ColorRangeValues& colors) {} //v10
    virtual void EquipHelmet(IECString& sItemAnim, ColorRangeValues& colors) {} //v14
    virtual void EquipShield(IECString& sItemAnim, ColorRangeValues& colors) {} //v18
    virtual void EquipWeapon(IECString& sItemAnim, ColorRangeValues& colors, unsigned int nItemFlags, void* pAttackProb) {} //v1c
    virtual void GetAnimationPalette() {} //v20 VidPal& GetAnimGroupVidPal(nGroupId) (ids use second lowest digit i.e. 0xX0)
    virtual void GetAnimationResRef(IECString* ptr, uchar range) {} //v24, GetAnimGroupPaperdollName ids use as above
    virtual bool CanUseMiddleVertList() {}  //v28 CanLieDown
    virtual void DetectedByInfravision() {} //v2c bool
    virtual short GetCastFrame() {}         //v30 GetNumSeqPerAnimation
    virtual char GetColorBlood() {}         //v34
    virtual void GetColorChunks() {}        //v38
    virtual char GetDefaultVertListType() {} //v3c
    virtual uchar GetCurrentMovementRate() {} //v40, get 7h
    virtual void SetCurrentMovementRate(unsigned char n) {} //v44
    virtual void ResetCurrentMovementRate() {}  //v48, put 6h into 7h
    virtual char GetDefaultMovementRate() {}    //v4c
    virtual void GetNeckOffsets() {}            //v50 
    virtual RECT* GetFootCircle() {}            //v54 GetEllipseRect
    virtual void GetPathSmooth() {}             //v58
    virtual char GetFootCircleSize() {}         //v5c GetPersonalSpace
    virtual void GetSndArmor() {}               //v60 LPCTSTR
    virtual LPCTSTR GetSndDeath() {}            //v64 GetFallingSound
    virtual LPCTSTR GetSndReady() {}            //v68
    virtual LPCTSTR GetWalkingSound(short wTerrainCode); //v6c GetSndWalk
    virtual void GetSndWalkFreq() {}            //v70 get 1ah
    virtual BOOL IsFalseColor() {}              //v74 GetUseColorRange
    virtual bool IsInvulnerable() {}            //v78 IsImmuneToDamage
    virtual BOOL IsMirroring() {}               //v7c IsInMirrorOrientation
    virtual BOOL IsBeginningOfSequence() {}     //v80 IsCurrentFrameFirstFrame
    virtual BOOL IsEndOfSequence() {}           //v84 IsAtEndOfCycle
    virtual void IsEndOfTwitchSequence() {}     //v88 another cycle related thing
    virtual void IncrementFrame() {}            //v8c
    virtual void DecrementFrame() {}            //v90
    virtual BOOL Draw(CInfinity& infinity, CVideoMode& vidmode) {}  //v94 Render
    virtual void ClearColorEffects(char nGroupRangeID) {}           //v98 DeinitColor
    virtual void ClearColorEffectsAll() {}                          //v9c DeinitAllColors
    virtual void SetColorEffect(int nColorGroup, int nGroupRangeId, ABGR rgbColor, char n) {} //va0 InitColor
    virtual void SetColorEffectAll(int nGroupId, ABGR rgbColor, char n) {}  //va4 InitAllColors
    virtual void SetColorRange(int nColorIdx, int nGroupRangeId) {}         //va8
    virtual void SetColorRangeAll(int nGroupRangeId) {} //vac
    virtual short SetAnimationSequence(short wSeq) {}   //vb0, SetSequence
    virtual void CalculateFxRectMax() {}    //vb4
    virtual void SetFootCircle() {}         //vb8 CalculateEllipseRect
    virtual void SetNeckOffsets() {}        //vbc
    virtual char GetAttackFrameType() {}    //vc0 GetInRoundAction
    virtual BOOL GetAboveGround() {}        //vc4, for ankhegs CanBeTargetted
    virtual void GetAwakePlayInReverse() {} //vc8
    virtual void SetBrightest() {}          //vcc, set 3d4h
    virtual void SetBrightestDesired() {}   //vd0  set 3d5h
    virtual void GetCastHeight() {}         //vd4
    virtual void GetCastingOffset() {}      //vd8
    virtual bool GetCurrentSequenceAndFrame(short& wCycle, short& wFrame); //vdc GetCurrentCycleAndFrame
    virtual bool GetCurrentResRef(IECString& s1, IECString& s2, IECString& s3, IECString& s4) {} //ve0, specific animations only?

    ushort wAnimId;                 //4h, animationID
    uchar nMovementRateDefault;     //6h, moveScale
    uchar nMovementRateCurrent;     //7h, moveScaleCurrent, modified by effects
    RECT rFootcircle;               //8h, rEllipse
    uchar colorBlood;               //18h
    uchar colorChunks;              //19h
    ulong nWalkSndFreq;             //1ah, nSndFreq
    LPCTSTR szFallingSound;         //1eh, pSndDeath
    NECK_POINTS neckOffsets[8];     //22h, set by 0x7FA94F proc, either 0, 10, or -10, to do with orientations?
    CVidBitmap combatRounds[5];     //42h, rndbase1-5, These graphics determine what to do during each
    uchar nFootCircleSize;          //3d0h, personalSpace, 3, 5, 7, 9, 13 (in diameter)
    char u3d1;                      //pad
    ushort castFrame;               //3d2h, wCyclesPerAnimation
    bool bBrightest;                //3d4h
    bool bBrightestDesired;         //3d5h
    bool bLightSource;              //3d6h
    ResRef rAniSnd;                 //3d7h, soundRef
    char u3df;                      //pad
    CAnimationSoundList sequencesoundset[19]; //3e0h, corresponds to rows of anim soundset 2DAs (19 in total)
};

extern CAnimation& (CAnimation::*CAnimation_Construct)();
extern void (CAnimation::*CAnimation_PlayCurrentSequenceSound)(CCreatureObject&);
extern CAnimation* (*CAnimation_CreateAnimation)(unsigned short, ColorRangeValues&, unsigned short);
extern BOOL (*CAnimation_IsPlayableAnimation)(unsigned short);
extern LPCTSTR (CAnimation::*CAnimation_GetWalkingSound)(short);
extern bool (CAnimation::*CAnimation_GetCurrentCycleAndFrame)(short&, short&);

struct AnimData { //Size 6h
    CAnimation* pAnimation; //0h
    short wCurrentSequence; //4h
};

#endif //ANIMCORE_H
