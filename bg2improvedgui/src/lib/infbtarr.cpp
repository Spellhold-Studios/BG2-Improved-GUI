#include "infbtarr.h"

BOOL (CButtonArray::*CButtonArray_CheckButtonEnabled)(int) =
	SetFP(static_cast<BOOL (CButtonArray::*)(int)>	(&CButtonArray::CheckButtonEnabled),	0x671C9E);

BOOL CButtonArray::CheckButtonEnabled(int nIdx) { return (this->*CButtonArray_CheckButtonEnabled)(nIdx); }
_n int  CButtonArray::UpdateButtons() { _bgmain(0x6645EE) }
_n void  CButtonArray::OnRButtonPressed(int) { _bgmain(0x670D83) }
_n int  CButtonArray::SetState(int nState) { _bgmain(0x661A65) }
_n CGameButtonList* CButtonArray_GetData(short, short, uchar)  { _bgmain(0x66097B) }
_n bool CButtonArray_PickSpell(CButtonData* pButtonData, BOOL bInstantUse) { _bgmain(0x6612C6) }
_n void CButtonArray_ClearList() { _bgmain(0x6608E9) }
_n void CButtonArray_SetQuickSlot(CButtonData *pButtonData, int nButton, int nType) { _bgmain(0x66161F) }



