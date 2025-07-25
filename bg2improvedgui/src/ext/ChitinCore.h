#ifndef CHITINCORE_H
#define CHITINCORE_H

#include "chitin.h"

extern CBaldurChitin& (CBaldurChitin::*Tramp_CBaldurChitin_Construct)(void);

int __stdcall CBaldurChitin_MessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

class DETOUR_CBaldurChitin : public CBaldurChitin {
public:
	CBaldurChitin& DETOUR_Construct();
};


void CAlloc_asm();
void Malloc_asm();
void CBaldurChitinInit_asm();

#endif //CHITINCORE_H