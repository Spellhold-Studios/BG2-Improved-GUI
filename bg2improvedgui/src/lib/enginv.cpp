#include "enginv.h"

void (CScreenInventory::*CScreenInventory_Init)() =
	SetFP(static_cast<void (CScreenInventory::*)()>	(&CScreenInventory::Init),	0x740178);

void CScreenInventory::Init() { return (this->*CScreenInventory_Init)(); }
_n BOOL CScreenInventory::IsCharacterInRange(short) { _bgmain(0x749482) }
_n BOOL CScreenInventory::IsCharacterDead(short)    { _bgmain(0x74972E) }