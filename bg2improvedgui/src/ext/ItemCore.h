#ifndef ITEMCORE_H
#define ITEMCORE_H

#include "itmcore.h"

class DETOUR_CItem : public CItem {
public:
	void DETOUR_Equip(CCreatureObject& cre, int nSlot, BOOL bAnimationOnly);
    void DETOUR_UnEquip(CCreatureObject& Cre, int nSlot, BOOL bRecalculateEffects, BOOL bAnimationOnly);
	CEffect* DETOUR_GetAbilityEffect(int nAbilityIdx, int nEffectIdx, CCreatureObject* creSource);
};

extern void (CItem::*Tramp_CItem_Equip)(CCreatureObject& cre, int nSlot, BOOL bAnimationOnly);
extern void (CItem::*Tramp_CItem_UnEquip)(CCreatureObject& Cre, int nSlot, BOOL bRecalculateEffects, BOOL bAnimationOnly);
extern CEffect* (CItem::*Tramp_CItem_GetAbilityEffect)(int, int, CCreatureObject*);

void CItem_Equip_ResetIdleTimer_Shield_asm();
void CItem_Equip_ResetIdleTimer_OffHand_asm();
void CItem_Equip_ResetIdleTimer_MainHand_asm();

#endif //ITEMCORE_H