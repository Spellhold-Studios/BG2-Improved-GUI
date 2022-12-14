#ifndef EFFECTCORE_H
#define EFFECTCORE_H

#include "stdafx.h"
#include "effcore.h"
#include "objcre.h"

//CEffect
extern CEffect& (*Tramp_CEffect_CreateEffect)(ITEM_EFFECT&, POINT&, Enum, POINT&, Enum);
extern BOOL (CEffect::*Tramp_CEffect_ApplyTiming)(CCreatureObject&);
extern BOOL (CEffect::*Tramp_CEffect_CheckSave)(CCreatureObject&, char&, char&, char&, char&, char&, char&);

class DETOUR_CEffect : public CEffect {
public:
	static CEffect& DETOUR_CreateEffect(ITEM_EFFECT& eff, POINT& ptSource, Enum eSource, POINT& ptDest, Enum e2);
	BOOL DETOUR_ApplyTiming(CCreatureObject& creTarget);
	BOOL DETOUR_CheckSave(CCreatureObject& creTarget, char& rollSaveDeath, char& rollSaveWands, char& rollSavePoly, char& rollSaveBreath, char& rollSaveSpells, char& rollMagicResist);
};

//CEffectList
extern void (CEffectList::*Tramp_CEffectList_TryDispel)(CCreatureObject&, POSITION, BOOL, BOOL, char, char);

class DETOUR_CEffectList : public CEffectList {
public:
	void DETOUR_TryDispel(CCreatureObject& creTarget, POSITION posSkip, BOOL bCheckDispellableFlag, BOOL bCheckProbability, char cRand, unsigned char cDispelLevel);
};

H_MEMBERHOOK_1args(CGameEffectSetColor7, CGameEffectSetColor7_Apply, CCreatureObject *)
H_MEMBERHOOK_1args(CGameEffectSetColor8, CGameEffectSetColor8_Apply, CCreatureObject *)
H_MEMBERHOOK_1args(CGameEffectSetColor9, CGameEffectSetColor9_Apply, CCreatureObject *)
H_MEMBERHOOK_1args(CGameEffectSetColor51, CGameEffectSetColor51_Apply, CCreatureObject *)
H_MEMBERHOOK_1args(CGameEffectSetColor52, CGameEffectSetColor52_Apply, CCreatureObject *)

void NonSchoolBonuses_asm();
void CGameEffect_Compare_BeforeRemove_asm();

#endif //EFFECTCOMMON_H