#ifndef INFBUTTONARRAY_H
#define INFBUTTONARRAY_H

#include "infbtarr.h"

struct DETOUR_CButtonArray : public CButtonArray {
	BOOL DETOUR_CheckButtonEnabled(int nIdx);
};

extern BOOL (CButtonArray::*Tramp_CButtonArray_CheckButtonEnabled)(int);

void CInfButtonArray_UpdateButtons_CheckInnateList_asm();
void ClericThief_TurnUndeadInnate_asm();
void CUIControlButtonAction_OnRButtonClick_asm();
void CScreenPriestSpell_OnDoneButtonClick_asm();
void CScreenPriestSpell_EscapeKeyDown_asm();
void CScreenPriestSpell_IconAssign_asm();
void CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm();
void CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm();
void CScreenPriestSpell_LoadIconSpell_asm();


#endif //INFBUTTONARRAY_H