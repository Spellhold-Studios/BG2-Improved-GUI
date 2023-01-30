#include "ChitinCore.h"

#include "console.h"

CBaldurChitin& (CBaldurChitin::*Tramp_CBaldurChitin_Construct)(void) =
	SetFP(static_cast<CBaldurChitin& (CBaldurChitin::*)(void)>		(&CBaldurChitin::Construct),	0x432108);

CBaldurChitin& DETOUR_CBaldurChitin::DETOUR_Construct() {
	CBaldurChitin& chitin = (this->*Tramp_CBaldurChitin_Construct)();
	g_pChitin = &chitin;
	return chitin;
}

int __stdcall CBaldurChitin_MessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
	if (0) IECString("CBaldurChitin_MessageBox");

	int dwThreadProcessId = GetWindowThreadProcessId(hWnd, NULL);
	HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, dwThreadProcessId);
	int dwSuspendCount;
	do {
		dwSuspendCount = ResumeThread(hThread);
	} while (dwSuspendCount > 1);
	return MessageBox(hWnd, lpText, lpCaption, uType);
}


//void __declspec(naked)
//CAlloc_asm()
//{
//__asm {
//    push    1               // size= 1 * arg
//    push    [esp+8h]        // arg
//    mov     eax, 0A3B45Ch   // __cdecl calloc()
//    call    eax
//    add     esp, 8          // restore stack
//
//	ret     
//}
//}


void __declspec(naked)
Malloc_asm()
{
__asm {
    push    [esp+4h]        // arg
    mov     eax, 0A3949Fh   // __cdecl _malloc()
    call    eax
    add     esp, 4

    push    eax             // ptr

    push    [esp+8h]        // arg
    push    0               // fill
    push    eax             // ptr
    mov     eax, 0A3C020h   // __cdecl _memset()
    call    eax
    add     esp, 0Ch

    pop     eax             // ptr
	ret     
}
}