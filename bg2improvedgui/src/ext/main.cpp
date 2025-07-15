#include "main.h"

#include "stdafx.h"
#include "debug.h"
#include "patch.h"
#include "hook.h"

#include "ExceptionHandler.h"

const DWORD g_nPEAddressDefault = 0x400000;
#define DLL_INTERNAL_NAME "TobEx.dll\0"

void Init() {

	char* lpVersion = NULL;
	DllGetVersion(&lpVersion);
	
	time_t tmTime = time(NULL);
	tm tmLocal;
	localtime_s(&tmLocal, &tmTime);
	const char* buffer = "%sTobEx: Throne of Bhaal Extender%s%s (%s %.2d %s %.4d %.2d:%.2d:%.2d)\r\n";
	const char* bufferD = "%sTobEx Dialogue Log (%s %.2d %s %.4d %.2d:%.2d:%.2d)\r\n";

	//init console
	if (console.Init())
		console.writef(CONSOLEFORECOLOR_HEADER, buffer, "", lpVersion ? " build " : "", lpVersion ? lpVersion : "", days[tmLocal.tm_wday], tmLocal.tm_mday, months[tmLocal.tm_mon], tmLocal.tm_year + 1900, tmLocal.tm_hour, tmLocal.tm_min, tmLocal.tm_sec);

	//init log
	int nDebugLogFileMode = GetCoreIniValue("Debug", "Log File Mode");
	if (L.Init(nDebugLogFileMode))
		L.appendf(buffer, "-----", lpVersion ? " build " : "", lpVersion ? lpVersion : "", days[tmLocal.tm_wday], tmLocal.tm_mday, months[tmLocal.tm_mon], tmLocal.tm_year + 1900, tmLocal.tm_hour, tmLocal.tm_min, tmLocal.tm_sec);

	char lpFile[MAX_PATH];
	if (GetModuleFileName(NULL, lpFile, MAX_PATH)) {
		const int nPEAddress = (int)GetModuleHandle(lpFile);
		if (nPEAddress == g_nPEAddressDefault) {
			InitOptions();

			if (pGameOptionsEx->bDebugLogDialogueBar) {
				if (LD.Init(nDebugLogFileMode))
					LD.appendf(bufferD, "-----", days[tmLocal.tm_wday], tmLocal.tm_mday, months[tmLocal.tm_mon], tmLocal.tm_year + 1900, tmLocal.tm_hour, tmLocal.tm_min, tmLocal.tm_sec);
			}

			InitPatches();
			InitHooks();

#ifdef _DEBUG
			Debug();
#endif //_DEBUG
		} else {
			LPCTSTR lpsz = "Init(): Executable loaded at incompatible address 0x%X. All TobEx features disabled. Please report this error.\r\n";
			console.writef(lpsz, nPEAddress);
			L.timestamp();
			L.appendf(lpsz, nPEAddress);
		}
	} else {
		LPCTSTR lpsz = "Init(): Unable to resolve module name. All TobEx features disabled.\r\n";
		console.write(lpsz);
		L.timestamp();
		L.append(lpsz);
	}

	delete[] lpVersion;

	return;
}

void Deinit() {
  return;
}

BOOL DllGetVersion(char** lplpsz) {
 	LPVOID lpFileVerInfo;
	LPVOID lpFileVer;
	UINT nProdVerSize;
	BOOL bSuccess = FALSE;

	DWORD dwFileVerInfoSize = GetFileVersionInfoSizeA(DLL_INTERNAL_NAME, NULL);

	if (dwFileVerInfoSize) {
  		lpFileVerInfo = (LPVOID) new BYTE[dwFileVerInfoSize];
		if ( GetFileVersionInfoA(DLL_INTERNAL_NAME, 0, dwFileVerInfoSize, lpFileVerInfo) ) {
			//0409 = US English, 04E4 = charset
			if ( VerQueryValueA(lpFileVerInfo, "\\StringFileInfo\\040904E4\\FileVersion", &lpFileVer, &nProdVerSize) ) {
				size_t nLength = strlen((LPCTSTR)lpFileVer);
				char* lpsz = new char[nLength + 1];
				strcpy_s(lpsz, nLength + 1, (LPCTSTR)lpFileVer);
				lpsz[nLength] = '\0';
				*lplpsz = lpsz;

				bSuccess = TRUE;
			}
		}
        if (lpFileVerInfo)
		    delete[] (BYTE *) lpFileVerInfo;
	}
	return bSuccess;
}


char gMSG_MainCrash[] =     "Main() crash";
char gMSG_ThreadCrash[] =   "New thread() crash";
char gMSG_ThreadCrashEx[] = "New threadex() crash";

void __declspec(naked)
RecordExceptionInfoMain_asm() {
__asm
{   // A3A561
    lea     eax, gMSG_MainCrash   // szMessage
    push    eax
    mov     eax, [ebp-14h]
    mov     ecx, [eax]
    mov     ecx, [ecx]
    mov     [ebp-68h], ecx
    push    eax             // ExceptionInfo

    call    RecordExceptionInfo

    add     esp, 8
    ret
}
}

void __declspec(naked)
RecordExceptionInfoThreadStart_asm() {
__asm
{   // A399A9
    lea     eax, gMSG_ThreadCrash   // szMessage
    push    eax
    mov     eax, [ebp-14h]
    mov     ecx, [eax]
    mov     ecx, [ecx]
    mov     [ebp-1Ch], ecx
    push    eax             // ExceptionInfo

    call    RecordExceptionInfo

    add     esp, 8
    ret
}
}

void __declspec(naked)
RecordExceptionInfoThreadStartEx_asm() {
__asm
{   // A3DE64
    lea     eax, gMSG_ThreadCrashEx   // szMessage
    push    eax
    mov     eax, [ebp-14h]
    mov     ecx, [eax]
    mov     ecx, [ecx]
    mov     [ebp-1Ch], ecx
    push    eax             // ExceptionInfo

    call    RecordExceptionInfo

    add     esp, 8
    ret
}
}
