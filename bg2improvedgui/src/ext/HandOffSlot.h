#ifndef HandOffSlot_H
#define HandOffSlot_H

#include "chitin.h"


class ResContainer  {
public:
    void ResBamContainer_SetRes(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing);
    BOOL FindKey(ResRef& NewResRef, int a1, int a2);
    //void ResBahContainer_SetRes(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing);
    int* buf1[4];
};

extern void (ResContainer::*Tramp_ResBamContainer_SetRes)(ResRef&,int,int);
extern BOOL (ResContainer::*Tramp_FindKey)(ResRef&,int,int);


class DETOUR_ResContainer : public ResContainer {
public:
	void DETOUR_ResBamContainer_SetRes(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing);
    BOOL DETOUR_FindKey(ResRef& NewResRef, int a1, int a2);
};

//(pAnimation->wAnimId >= 0x5000 && pAnimation->wAnimId <  0x5400) ||
// pAnimation->wAnimId & 0x6000 ||
// Polymorph XXX

#define IsAnim5000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB6F90 ? true : false )  // ~CAnimation5000()

#define IsAnim6400(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB7074 ? true : false )  // ~CAnimation6400()

#define IsAnim7000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB678C ? true : false )  // ~CAnimation7000()

#define IsAnim7300(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB66A0 ? true : false )  // ~CAnimation7300()

#define IsAnim8000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB6CE4 ? true : false )  // ~CAnimation8000()

#define IsAnim9000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB6954 ? true : false )  // ~CAnimation9000()

#define IsAnimA000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB6A38 ? true : false )  // ~CAnimationA000()

#define IsAnimE000(pAnimation)  \
    ( *((unsigned int *)pAnimation) == 0xAB6870 ? true : false )  // ~CAnimationE000()




void ScreenRecordThacInfo_asm();
void ApplyDamage_asm();
void CCreatureObject__Hit_asm();
void CombatInfo_asm();
void WeaponStyleBonuses_asm();
void WeaponStyleBonuses2_asm();
void ApplyLevelProgressTableSub_asm();
void ApplyLevelProgressTableSub2_asm();
void Sub_8C29E6_asm();
void Sub_8C4E09_asm();
void EquipRanged_asm();
void EquipRanged2_asm();
void CCreatureObject_CCreatureObject_InitU6753_asm();
void CInfGame_SwapItemPersonal_CheckShieldSlot_asm();

bool
isOffHandDisabled(const CCreatureObject& Cre);

bool
IsOffHandAllowed(CCreatureObject& Cre, CItem* const pOffHandItem);

void
ReConfigOffHand(CCreatureObject& Cre, CItem* const pItem, int const nSlot, BOOL bAnimationOnly, bool bSpecialCaller);

void CreatureRMClick_asm();


#define OffHand_OFF         0
#define OffHand_Effects     1
#define OffHand_Anim        2
#define OffHand_Full        3

#define write_shield 0

extern uint    debug_line;
extern uint    debug_callnum;

#define DEBUG_hand(x, ...)                      \
    if (write_shield) {                         \
        console.writef("%d ", debug_line++);    \
        console.writef(x, __VA_ARGS__);         \
    }


#endif //HandOffSlot_H