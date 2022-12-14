#include "animcore.h"

#include "stdafx.h"

//CAnimationSoundList
BOOL (CAnimationSoundList::*CAnimationSoundList_PlaySound)(int, CCreatureObject*) =
    SetFP(static_cast<BOOL (CAnimationSoundList::*)(int, CCreatureObject*)> (&CAnimationSoundList::PlaySound),    0x642E3D);

BOOL CAnimationSoundList::PlaySound(int curFrame, CCreatureObject* pSprite) { return (this->*CAnimationSoundList_PlaySound)(curFrame, pSprite); }

//CAnimation
//CAnimation& (CAnimation::*CAnimation_Construct)() =
//    SetFP(static_cast<CAnimation& (CAnimation::*)()>            (&CAnimation::Construct),                   0x7F9211);
void (CAnimation::*CAnimation_PlayCurrentSequenceSound)(CCreatureObject&) =
    SetFP(static_cast<void (CAnimation::*)(CCreatureObject&)>   (&CAnimation::PlayCurrentSequenceSound),    0x7F9DF4);
CAnimation* (*CAnimation_CreateAnimation)(unsigned short, ColorRangeValues&, unsigned short) =
    reinterpret_cast<CAnimation* (*)(unsigned short, ColorRangeValues&, unsigned short)>                    (0x7F9EBC);
BOOL (*CAnimation_IsPlayableAnimation)(unsigned short) =
    reinterpret_cast<BOOL (*)(unsigned short)>                                                              (0x85F364);
LPCTSTR (CAnimation::*CAnimation_GetWalkingSound)(short) =
    SetFP(static_cast<LPCTSTR (CAnimation::*)(short)>           (&CAnimation::GetWalkingSound),             0x87B7B0);
//bool (CAnimation::*CAnimation_GetCurrentCycleAndFrame)(short&, short&) =
//    SetFP(static_cast<bool (CAnimation::*)(short&, short&)>     (&CAnimation::GetCurrentCycleAndFrame),     0x87BA10);

//CAnimation::CAnimation()                                              { (this->*CAnimation_Construct)(); }
//CAnimation::~CAnimation()                                             // 0x87B4C0
void CAnimation::PlayCurrentSequenceSound(CCreatureObject& cre)         { return (this->*CAnimation_PlayCurrentSequenceSound)(cre); }
CAnimation* CAnimation::CreateAnimation(unsigned short wAnimId, ColorRangeValues& colors, unsigned short wOrient)
    { return (*CAnimation_CreateAnimation)(wAnimId, colors, wOrient); }
BOOL CAnimation::IsPlayableAnimation(unsigned short wAnimId)            { return (*CAnimation_IsPlayableAnimation)(wAnimId); }
LPCTSTR CAnimation::GetWalkingSound(short wTerrainCode)                 { return (this->*CAnimation_GetWalkingSound)(wTerrainCode); }
//bool CAnimation::GetCurrentCycleAndFrame(short& wCycle, short& wFrame)  { return (this->*CAnimation_GetCurrentCycleAndFrame)(wCycle, wFrame); }
