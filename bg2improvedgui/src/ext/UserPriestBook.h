#ifndef USERPRIESTBOOK_H
#define USERPRIESTBOOK_H

#include "uibutton.h"

extern int CUIButtonPriestBook_KnownSpellOffset;

class CUIButtonPriestBookUp : public CUIButton {
public:
	CUIButtonPriestBookUp(CPanel& panel, ChuFileControlInfoBase& controlInfo);

	virtual ~CUIButtonPriestBookUp(); //v0
	virtual void OnLClicked(POINT pt); //v5c
};

class CUIButtonPriestBookDn : public CUIButton {
public:
	CUIButtonPriestBookDn(CPanel& panel, ChuFileControlInfoBase& controlInfo);

	virtual ~CUIButtonPriestBookDn(); //v0
	virtual void OnLClicked(POINT pt); //v5c
};

void KeepPriestSlotsWhenDrained_asm();
void KeepMageSlotsWhenDrained_asm();
void RemoveSpellsPriest_asm();
void RemoveSpellsPriest2_asm();
void RemoveSpellsMage_asm();
void RemoveSpellsMage2_asm();
void EffectRestorationApply_asm();
void CScreenPriestSpell_OnPortraitLClick_SwitchBook_asm();
void CScreenWizSpell_OnPortraitLClick_SwitchBook_asm();

#endif //USERPRIESTBOOK_H
