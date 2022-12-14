#define _EXPORTING

#include "TobEx.h"

#include "stdafx.h"
#include "main.h"

//Use as Extension DLL
/*#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static AFX_EXTENSION_MODULE PROJNAMEDLL = { NULL, NULL };*/

//Force inclusion of this module for _USRDLL
//extern "C" { int _afxForceUSRDLL; }

BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {

    switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			Init();
			break;
		case DLL_PROCESS_DETACH:
			Deinit();
			break;
		case DLL_THREAD_ATTACH:
	        break;
		case DLL_THREAD_DETACH:
	        break;
    }

    return true;
}


//////////////////////////////////////////////////////////////
// to avoid linking with full nafxcw.lib(too much cross-linked garbage), link only to minimal required .obj:
// nolib.obj strcore.obj strex.obj winstr.obj mtex.obj fixalloc.obj mtcore.obj objcore.obj afxstate.obj
// plex.obj afxtls.obj afxcrit.obj winhand.obj map_pp.obj afxmem.obj appui1.obj winutil.obj auxdata.obj
// list_p.obj arcobj.obj array_p.obj arccore.obj
//
// Some missed funcs required, just rip from mfc 4.2 srcs :
#ifndef _DEBUG

CWinThread* AFXAPI AfxGetThread()
{
	// check for current thread in module thread state
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	CWinThread* pThread = pState->m_pCurrentWinThread;

	// if no CWinThread for the module, then use the global app
	if (pThread == NULL)
		pThread = AfxGetApp();

	return pThread;
}

void AFXAPI AfxThrowResourceException()
{
	THROW(NULL);
}

void AFXAPI AfxThrowMemoryException()
{
	THROW(NULL);
}

void AFXAPI AfxThrowNotSupportedException()
{
	THROW(NULL);
}

void CException::Delete()
{
	// delete exception if it is auto-deleting
	if (m_bAutoDelete > 0)
	{
#ifdef _DEBUG
		m_bReadyForDelete = TRUE;
#endif
		delete this;
	}
}

void AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName /* = NULL */)
{
	THROW(cause);
}

#endif
/////////////////////////////////////////////////////
