#ifndef SPLCORE_H
#define SPLCORE_H

#include "rescore.h"
#include "effcore.h"

typedef IECPtrList CMemorizedSpellList; //AAA8E4
typedef IECPtrList CKnownSpellList; //AAA8CC

struct ResSplContainer { //Size 10h
	ResSplContainer();

	ResSplContainer(ResRef r);
	ResSplContainer& Construct(ResRef r) { return *this; } //dummy

	~ResSplContainer();
	void Deconstruct() {} //dummy

	void SetResRef(ResRef& r, BOOL bSetAutoRequest, BOOL bWarningIfMissing);
	BOOL Demand();
	BOOL Release();
	int GetNumAbilities();
	SplFileAbility* GetSpellAbility(long abilityNum);
	CEffect* GetAbilityEffect(int nAbilityIdx, int nEffectIdx, CCreatureObject* creSource);
	short GetSpellLevel();
	unsigned int GetExclusionFlags();
	STRREF GetSpellDescription();
	short GetSpellType();
	unsigned int GetSpellFlags();
	unsigned char GetSpellSchoolPrimary();

	BOOL bLoaded; //0h
	ResSpl* pRes; //4h
	ResRef name; //8h
};

extern ResSplContainer& (ResSplContainer::*ResSplContainer_Construct_1ResRef)(ResRef);
extern void (ResSplContainer::*ResSplContainer_Deconstruct)();
extern void (ResSplContainer::*ResSplContainer_LoadResource)(ResRef&, BOOL, BOOL);
extern BOOL (ResSplContainer::*ResSplContainer_Demand)();
extern BOOL (ResSplContainer::*ResSplContainer_Release)();
extern int (ResSplContainer::*ResSplContainer_GetNumAbilities)();
extern SplFileAbility* (ResSplContainer::*ResSplContainer_GetSpellAbility)(long);
extern CEffect* (ResSplContainer::*ResSplContainer_GetAbilityEffect)(int, int, CCreatureObject*);
extern short (ResSplContainer::*ResSplContainer_GetSpellLevel)();
extern unsigned int (ResSplContainer::*ResSplContainer_GetExclusionFlags)();
extern STRREF (ResSplContainer::*ResSplContainer_GetSpellDescription)();
extern short (ResSplContainer::*ResSplContainer_GetSpellType)();
extern unsigned int (ResSplContainer::*ResSplContainer_GetSpellFlags)();
extern unsigned char (ResSplContainer::*ResSplContainer_GetSpellSchoolPrimary)();

/*
struct SpellMod { //Size 1Eh
used in DoWildMagic() on CastSpell()
	short u0; nProjectile
	int u2; nPercentProjSpeed
	int u6; 100
	int ua; nPercentLevel
	BOOL ue; bApplyEffects
	int u12; 1
	BOOL u16; bSetProjTarget
	int u1a;
};
*/

#endif //SPLCORE_H