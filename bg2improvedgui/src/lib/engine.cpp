#include "engine.h"

#include "stdafx.h"

int (CEngine::*CEngine_GetActivePlayerIdx)() =
	SetFP(static_cast<int (CEngine::*)()>		(&CEngine::GetActivePlayerIdx),		0x43AD5C);

int CEngine::GetActivePlayerIdx()	{ return (this->*CEngine_GetActivePlayerIdx)(); }

_n void __cdecl CEngine_UpdateText(CEngine& pScreen, CUITextArea& pTextArea, const char *format, ...) { _bgmain(0x43B22D) }
_n void CEngine::OnLeftPanelButtonClick(unsigned int dwButtonId)    { _bgmain(0x43B67D) }
_n void CEngine::PlayGUISound(ResRef& cResSound)                    { _bgmain(0x43B531) }
