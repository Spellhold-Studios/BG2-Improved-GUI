#ifndef EFFCORE_H
#define EFFCORE_H

#include "stdafx.h"
#include "datatypes.h"
#include "rescore.h"
#include "sndcore.h"

typedef void* CEffectVirtualTable[10];

#define CEFFECT_DURATION_MAX	0x8888888

class CCreatureObject;
class CEffect;

struct ResEffContainer { //Size 10h
//Constructor: 0x4F2810
	ResEffContainer(ResRef r);
	ResEffContainer& Construct(ResRef r) { return *this; } //dummy

	~ResEffContainer();
	void Deconstruct() {} //dummy

	CEffect& CreateCEffect();

	BOOL bLoaded; //0h
	ResEff* pRes; //4h
	ResRef name; //8h
};

extern ResEffContainer& (ResEffContainer::*ResEffContainer_Construct_1ResRef)(ResRef);
extern void (ResEffContainer::*ResEffContainer_Deconstruct)();
extern CEffect& (ResEffContainer::*ResEffContainer_CreateCEffect)();

class CEffect { //Size 19Ah
public:
	void* operator new(size_t size);
	void operator delete(void* mem);

	CEffect();
	CEffect& Construct() { return *this; } //dummy

	CEffect(ITEM_EFFECT& data, POINT& ptSource, Enum eSource, int destX, int destY, BOOL bUseDice, Enum e2); //4F34C8
	CEffect& Construct(ITEM_EFFECT&, POINT&, Enum, int, int, BOOL, Enum) {return *this;} //dummy

	void operator=(CEffect& eff);
	void OpAssign(CEffect& eff) {} //dummy

	void Unmarshal(EffFileData&);

	CEffect(EffFileData& data);
	CEffect& Construct(EffFileData&) {return *this;} //dummy

	static CEffect& CreateEffect(ITEM_EFFECT& data, POINT& ptSource, Enum eSource, POINT& ptDest, Enum e2);
	static void CEffect::CreateItemEffect(ITEM_EFFECT& ptr, int nOpcode);

	void PlaySound(ResRef& rSound, CCreatureObject& cre);
	BOOL IsExpired();
	BOOL TryApplyEffect(
		CCreatureObject& creTarget,
		unsigned char* pcSaveDeath,
		unsigned char* pcSaveWands,
		unsigned char* pcSavePoly,
		unsigned char* pcSaveBreath,
		unsigned char* pcSaveSpells,
		unsigned char* pcResistMagic,
		unsigned char* pcEffectProbability
	);
	ITEM_EFFECT* CEffect::GetItemEffect(); // creates new ITEM_EFFECT instance, delete after use

    void OnAdd(CCreatureObject& creTarget); // call OnAddSpecific()

	//AA6378
	virtual ~CEffect(); //v0
	void Deconstruct() {} //dummy

	virtual CEffect& Duplicate(); //v4
	virtual BOOL ApplyEffect(CCreatureObject& creTarget); //v8
	virtual BOOL ApplyTiming(CCreatureObject& creTarget); //vc, ResolveEffect(), calls ApplyEffect

	virtual void OnAddSpecific(CCreatureObject& creTarget); //v10, TobEx v26 changed to OnAdd(), same as BG2EE
                                                            // BG2EE has two virtuals: OnAdd() and OnAddSpecific()
                                                            // BG2 has only OnAddSpecific() and nonvirtual OnAdd()
	virtual void OnLoad(CCreatureObject& creTarget); //v14, on addition to CEffectList
	virtual BOOL CheckSave(CCreatureObject& creTarget, char& rollSaveDeath, char& rollSaveWands, char& rollSavePoly, char& rollSaveBreath, char& rollSaveSpells, char& rollMagicResist); //v18
	virtual BOOL UsesDice(); //v1c
	virtual void DisplayString(CCreatureObject& creTarget); //v20
	virtual void OnRemove(CCreatureObject& creTarget); //v24, on removal from CEffectList

	EffFileData effect; //4h
	Enum eSource; //10ch
	BOOL bPurge; //110h - deconstruct and remove effect from CEffectList after application
	BOOL bRefreshStats; //114h - use if you change a m_BaseStats member, sets bApplyEffectsAgain in CEffectList, repeating effect application
	unsigned int nDurationAfterDelay; //118h, broken, no way to calculate

	//used like wildcards when checking against a CEffectList element
	BOOL m_bMatchOpcode; //11ch, match any with same opcode
	BOOL m_bMatchOpcodeParam2; //120h, match any with same opcode and param2
	BOOL m_bMatchOpcodeParam1; //124h, match any with same opcode and param1
	BOOL m_bMatchOpcodeResource; //128h, match any with same opcode and resource
	CSound sound;
	Enum enum2; //196h, Arg7, inherits value of Cre+30h for a global effect of an item equipped by the Cre
};

extern CEffect& (CEffect::*CEffect_Construct_0)();
extern CEffect& (CEffect::*CEffect_Construct_7)(ITEM_EFFECT&, POINT&, Enum, int, int, BOOL, Enum);
extern void (CEffect::*CEffect_OpAssign)(CEffect&);
extern void (CEffect::*CEffect_Unmarshal)(EffFileData&);
extern CEffect& (CEffect::*CEffect_Construct_1)(EffFileData&);
extern CEffect& (*CEffect_CreateEffect)(ITEM_EFFECT&, POINT&, Enum, POINT&, Enum);
extern void (*CEffect_CreateItemEffect)(ITEM_EFFECT&, int);
extern void (CEffect::*CEffect_PlaySound)(ResRef&, CCreatureObject&);
extern BOOL (CEffect::*CEffect_IsExpired)();
extern BOOL (CEffect::*CEffect_TryApplyEffect)(CCreatureObject&, unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned char*);
extern ITEM_EFFECT& (CEffect::*CEffect_ToItemEffect)();
extern void (CEffect::*CEffect_Deconstruct)();
extern CEffect& (CEffect::*CEffect_Duplicate)();
extern BOOL (CEffect::*CEffect_ApplyEffect)(CCreatureObject&);
extern BOOL (CEffect::*CEffect_ApplyTiming)(CCreatureObject&);
extern void (CEffect::*CEffect_OnDelayFinished)(CCreatureObject&);
extern void (CEffect::*CEffect_OnAdd)(CCreatureObject&);
extern BOOL (CEffect::*CEffect_CheckSave)(CCreatureObject&, char&, char&, char&, char&, char&, char&);
extern BOOL (CEffect::*CEffect_IgnoreLevelCheck)();
extern void (CEffect::*CEffect_DisplayString)(CCreatureObject&);
extern void (CEffect::*CEffect_OnRemove)(CCreatureObject&);

class CEffectList : public IECPtrList { //Size 2Ch
public:
	//AAA8A8
	BOOL RemoveOneEffect(CEffect& eff, CCreatureObject& cre, BOOL bMatchSourceAndParent);
	void RemoveEffect(CCreatureObject& creTarget, int nOpcode, POSITION posSkip, int nParam2, ResRef rResource, BOOL bCheckPermTiming);
	void TryDispel(CCreatureObject& creTarget, POSITION posSkip, BOOL bCheckDispellableFlag, BOOL bCheckProbability, char cRand, char cDispelLevel);
	BOOL ApplyEffects(CCreatureObject& cre);
	POSITION GetCurrentPosition();
    void  RemoveAllEffectsFromSourceRes(CCreatureObject& cre, POSITION posLeave, ResRef& pRes);

	POSITION posCurrent; //1ch, posNext
	POSITION posItrPrev; //20h, posCurrent
	BOOL bNewEffect;     //24h
	BOOL bRefreshStats;  //28h
};

extern BOOL (CEffectList::*CEffectList_RemoveOneEffect)(CEffect&, CCreatureObject&, BOOL);
extern void (CEffectList::*CEffectList_RemoveEffect)(CCreatureObject&, int, POSITION, int, ResRef, BOOL);
extern void (CEffectList::*CEffectList_TryDispel)(CCreatureObject&, POSITION, BOOL, BOOL, char, char);
extern BOOL (CEffectList::*CEffectList_ApplyEffects)(CCreatureObject&);
extern POSITION (CEffectList::*CEffectList_GetCurrentPosition)();

#endif //EFFCORE_H