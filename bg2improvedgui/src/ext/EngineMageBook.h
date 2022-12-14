#ifndef ENGINEMAGEBOOK_H
#define ENGINEMAGEBOOK_H

#include "engmagebk.h"

struct MemSpellContingency { //Size 14h
	MemSpellContingency();

	ResRef rName; //0h
	short wMemorizedCount; //8h
	short ua; //ah
	ResRef rParentSpell; //ch
};

extern void (CScreenMageBook::*Tramp_CScreenMageBook_SetLevel)(int);
extern void (CScreenMageBook::*Tramp_CScreenMageBook_ContingencySelectOnLoad)(CPanel&);
extern void (CScreenMageBook::*Tramp_CScreenMageBook_ContingencySelectOnUpdate)(int);
extern bool (CScreenMageBook::*Tramp_CScreenMageBook_ClearContingencySpell)(char index);
extern void (CScreenMageBook::*Tramp_CScreenMageBook_CreateContingencySpellList)(char, char);

class DETOUR_CScreenMageBook : public CScreenMageBook {
public:
	void DETOUR_SetLevel(int nLevel);
	void DETOUR_ContingencySelectOnLoad(CPanel& panel);
	void DETOUR_ContingencySelectOnUpdate(int nPanelIdx);
	bool DETOUR_ClearContingencySpell(char index);
	void DETOUR_CreateContingencySpellList(char nSpellType, char nLevel);
};

class CUIButtonSequence : public CUIButton {
public:
    CUIButtonSequence(CPanel& panel, ChuFileControlInfoBase& controlInfo);

    virtual ~CUIButtonSequence(); //v0
    virtual void SetEnabled(bool b); //v4
    virtual void OnLClicked(POINT pt); //v5c
};


H_MEMBERHOOK_2args(CUIContingencyButton, CUIContingencyButton_OnLClick, uint, uint)


void CScreenWizSpell_UpdateMainPanel_CheckSequenceList_asm();
void GetContingencyPtrEDX_asm();
void GetContingencyPtrEAX_asm();
void GetContingencyPtrCre_EDX_asm();
void CScreenWizSpell_UpdateContingPanel_GetStringRef_asm();
void RemoveContingency_asm();

#endif //ENGINEMAGEBOOK_H