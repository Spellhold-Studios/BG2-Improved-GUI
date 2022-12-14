#include "EngineCommon.h"

#include "objcre.h"
#include "infgame.h"
#include "chitin.h"
#include "tlkcore.h"

#define  ZLIB_WINAPI
#include "zlib.h"

uint gIsGameWindowForeground = 1;   // default is ON

void EngineCommon_ApplySoundset(CCreatureObject& cre) {

	IECString voiceset(cre.secondarySounds.GetResRefNulled());
	if (strcmp(cre.secondarySounds.GetResRefNulled(), "") == 0) { //use default soundset

		char sex = cre.oBase.Gender;

		for (int nRow = 0; nRow < 100; nRow++) {
			STRREF ref = g_pChitin->pGame->GetCharSndStrRef(0, nRow, sex);
			cre.BaseStats.soundset[nRow] = ref;
			g_pChitin->m_TlkTbl.GetTlkString(ref, cre.soundset[nRow]);
		}

	} else {

		CRuleTable& csound = g_pChitin->pGame->CSOUND;

		for (int i = 1; i < csound.nCols; i++) {
			//check if soundset present in CSOUND.2DA
			IECString& colHeader = *(csound.pColHeaderArray + i);

			if (voiceset.Compare((LPCTSTR)colHeader) == 0) { //if present, add StrRefs to cre

				for (int j = 0; j < 100; j++) {
					if (j == 74) continue; //skip biography

					for (int k = 0; k < csound.nRows; k++) {
						IECString& rowHeader = *(csound.pRowHeaderArray + k);

						if ( atoi((LPCTSTR)rowHeader) == j ) {
							IECString value = csound.GetValue(colHeader, rowHeader);
							STRREF ref = atoi((LPCTSTR)value);

							if (ref) {
								cre.BaseStats.soundset[j] = ref;
								g_pChitin->m_TlkTbl.GetTlkString(ref, cre.soundset[j]);
							} else {
								cre.BaseStats.soundset[j] = -1;
								g_pChitin->m_TlkTbl.GetTlkString(-1, cre.soundset[j]);
							}

						} //atoi
					} //for k
				} //for j

				cre.secondarySounds = ""; //set to default soundset
			} //Compare

			//if not present, do nothing
		} //i
	}

}


static HANDLE           gDisplayTrigger     = NULL;
static HANDLE           gDisplayDoneTrigger = NULL;
static HANDLE           gDisplayNotBusy     = NULL;

static unsigned int     gEndWait;
static unsigned int     gAIThread;
static LARGE_INTEGER    gDisplayPayloadStartTicks;
static LARGE_INTEGER    gLastTick;

static bool             gDontWaitNextAIUpdate = false;
static bool             gSkipAIUpdateThisFrame = true;
bool                    gX2Render = false;


void
CreateWinMainSyncEvent() {
    if (gDisplayTrigger == NULL)
        gDisplayTrigger = CreateEventA(NULL, TRUE, TRUE, NULL);

    if (gDisplayDoneTrigger == NULL)
        gDisplayDoneTrigger = CreateEventA(NULL, TRUE, TRUE, NULL);

    if (gDisplayNotBusy == NULL)
        gDisplayNotBusy = CreateEventA(NULL, TRUE, TRUE, NULL);
}


void static
UpdateMousePosition() {
    // update every frame
    POINT MousePt;
    CVideoMode* pVideoMode;
    bool  bUpdateCursorInWindow;

    if ( g_pChitin->pEngineActive )
        pVideoMode = g_pChitin->pEngineActive->pVideoMode;
    else
        pVideoMode = NULL;

    const ushort XRes   = *(( WORD *) (0xB6150C));
    const ushort YRes   = *(( WORD *) (0xB6150E));

    if (pGameOptionsEx->bEngine_RunInBackground)
        bUpdateCursorInWindow = to_bool(gIsGameWindowForeground);
    else
        bUpdateCursorInWindow = true;

    //////////////////////////////////////////////////////////
    // g_pChitin->MainWindowRect.right  = 1280
    // g_pChitin->MainWindowRect.bottom = 960
    // POINT MousePt2;
    //GetCursorPos(&MousePt2);
    //ScreenToClient(g_pChitin->m_CWnd.m_hWnd, &MousePt2);
    //if (MousePt2.x > XRes) MousePt2.x = XRes - 1;
    //if (MousePt2.y > YRes) MousePt2.y = YRes - 1;
    //console.writef("x=%d y=%d \n", MousePt2.x, MousePt2.y);
    //////////////////////////////////////////////////////////

    GetCursorPos(&MousePt);
    if ( g_pChitin->bFullScreen || 
        (PtInRect(&g_pChitin->MainWindowRect, MousePt) && bUpdateCursorInWindow) ) {
        ScreenToClient(g_pChitin->m_CWnd.m_hWnd, &MousePt);
        if (gX2Render) {
            if (MousePt.x > XRes) MousePt.x = XRes - 1;
            if (MousePt.y > YRes) MousePt.y = YRes - 1;
        }

        if ( g_pChitin->bScreenEdgeScroll == true ||
             g_pChitin->bFullScreen ||
            (MousePt.x &&
             MousePt.x != XRes - 1 &&
             MousePt.y &&
             MousePt.y != YRes - 1) )
        {
            if (pVideoMode)
                pVideoMode->bShowMouseCursor = 1;
        }
        else
        {
            MousePt.x = -1;
            MousePt.y = -1;
            if (pVideoMode)
                pVideoMode->bShowMouseCursor = 0;
        }

        //int ForceRefreshCursor = 0;
        if (MousePt.x == g_pChitin->ptCursor.x && MousePt.y == g_pChitin->ptCursor.y) {
          //if ( ForceRefreshCursor > 0 &&
          //     g_pChitin->pEngineActive->v60() )                        // ScreenHasMouse
          //          g_pChitin->pEngineActive->v64(g_pChitin->ptCursor); // UpdateCursor

        }
        else {
          //Lock();
          g_pChitin->ptCursor = MousePt;
          //Unlock();

          g_pChitin->u4026 = 0;
          if ( g_pChitin->pEngineActive->v60() )            // ScreenHasMouse
                    g_pChitin->pEngineActive->v64(MousePt); // UpdateCursor
        }
    }
}


float ToMillisec(LONGLONG difftick) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return ((float)difftick / (float)freq.QuadPart) * 1000 ;
}

#define LOGTICK(x)                                              \
    {LARGE_INTEGER CurTick;                                     \
    QueryPerformanceCounter( &CurTick );                        \
    if (!g_pChitin->bDisableWindow) {                           \
        console.writef("+%.2f\t",                               \
            ToMillisec(CurTick.QuadPart - gLastTick.QuadPart)); \
        x;                                                      \
        console.writef(" period= %.2f \n",                      \
            ToMillisec(CurTick.QuadPart - last_tick.QuadPart)); \
        last_tick = CurTick;                                    \
    }                                                           \
        gLastTick = CurTick;                                    \
    }

#define LOGTICK2(y, x)                                          \
    {LARGE_INTEGER CurTick;                                     \
    QueryPerformanceCounter( &CurTick );                        \
    if (!g_pChitin->bDisableWindow) {                           \
        console.writef("+%.2f\t",                               \
            ToMillisec(CurTick.QuadPart - gLastTick.QuadPart)); \
        x;                                                      \
        console.writef(" period= %.2f \n",                      \
            ToMillisec(CurTick.QuadPart - last_tick.QuadPart)); \
        y = CurTick;                                            \
    }                                                           \
        gLastTick = CurTick;                                    \
    }

#define LOG(x)                                                  \
    {LARGE_INTEGER CurTick;                                     \
    QueryPerformanceCounter( &CurTick );                        \
    if (!g_pChitin->bDisableWindow) {                           \
        console.writef("+%.2f\t",                               \
            ToMillisec(CurTick.QuadPart - gLastTick.QuadPart)); \
        x;                                                      \
    }                                                           \
        gLastTick = CurTick;                                    \
    }



// #define  _LOGEVENTS


void static __stdcall
WinMain_DisplayBegin() {
    int* MaxRefreshRate = (int*) 0xB60628;
    static unsigned int MaxRefreshPeriodMs = 1000 / *MaxRefreshRate;
    int waitstatus = 0;
    static LARGE_INTEGER last_tick = {0};
    static LARGE_INTEGER lastdone_tick = {0};

    #ifdef _LOGEVENTS
        LARGE_INTEGER StartWaitTick, CurTick;
        QueryPerformanceCounter( &StartWaitTick );
    #endif

    if (gDontWaitNextAIUpdate == false) {
        #ifdef _LOGEVENTS
            if (!g_pChitin->bDisableWindow) {
                console.writef("\n");
            }
            LOGTICK(console.writef("> Waiting gDisplayTrigger... request= %d", g_pChitin->bDisplayStale))
        #endif

        if (pGameOptionsEx->bEngine_CpuIdle) {
            // sleep until signal from AI/ProgressBar Threads
            // AI Thread synced with RefreshRate timer
            // ProgressBar is async
            waitstatus = WaitForSingleObject(gDisplayTrigger, MaxRefreshPeriodMs);
        }

    } else {
        #ifdef _LOGEVENTS
            if (!g_pChitin->bDisableWindow) {
                console.writef("\n");
            }
            LOGTICK(console.writef("> No display wait !!! request== %d", g_pChitin->bDisplayStale))
        #endif
    }

    #ifdef _LOGEVENTS
        QueryPerformanceCounter( &CurTick );
        LOGTICK2(lastdone_tick, console.writef(">  waiting is done, request= %d waittime=%.2f waitstatus= %d", g_pChitin->bDisplayStale, ToMillisec(CurTick.QuadPart - StartWaitTick.QuadPart), waitstatus))
    #endif

    gDontWaitNextAIUpdate = false;
    ResetEvent(gDisplayTrigger);

    #ifdef _LOGEVENTS
        QueryPerformanceCounter(&gDisplayPayloadStartTicks);
    #endif

    if (g_pChitin->bDisplayStale)       // if display rendering sheduled
        ResetEvent(gDisplayNotBusy);    // inform waiters about forthcoming rendering busy state
}



void static __stdcall
WinMain_DisplayDoneSync() {
    static LARGE_INTEGER last_tick = {0};

    #ifdef _LOGEVENTS
        LOGTICK(console.writef("DisplayDone"))
        LOG(console.writef(" display payload= %.2f \n", ToMillisec(CurTick.QuadPart - gDisplayPayloadStartTicks.QuadPart)))
    #endif

    SetEvent(gDisplayDoneTrigger);  // resume waiting threads
    SetEvent(gDisplayNotBusy);      // can accept next gDisplayTrigger event
}


void static __stdcall
CBaldurChitin_MainAIThread_TriggerDisplaySync() {
    static LARGE_INTEGER last_tick = {0};

    #ifdef _LOGEVENTS
        LOGTICK(console.writef("MainAIThread request display"))
    #endif

    SetEvent(gDisplayTrigger);        // trigger SynchronousUpdate()
}


void static __stdcall
RenderOneFrame(BOOL bSleepRequested) {
    //static LARGE_INTEGER last_tick ;

    #ifdef _LOGEVENTS
        if (bSleepRequested)
            console.writef("SyncAndWait SleepRequested \n");

        LOG(console.writef("SyncAndWait wait display, request= %d \n", g_pChitin->bInSyncUpdate))
    #endif

    gDontWaitNextAIUpdate = false;  // wait mode
    int waitstatus = WaitForSingleObject(gDisplayNotBusy, INFINITE); // wait if current SynchronousUpdate() is busy

    #ifdef _LOGEVENTS
        LOG(console.writef(" SyncAndWait request display job\n"))

        LARGE_INTEGER WaitedDisplayPayloadStartTicks;
        QueryPerformanceCounter(&WaitedDisplayPayloadStartTicks);
    #endif
    
    ResetEvent(gDisplayDoneTrigger);
    // trigger & wait SynchronousUpdate() for one time
    g_pChitin->bDisplayStale = 1;   // enable  SynchronousUpdate()
    gSkipAIUpdateThisFrame = false; // full render
    waitstatus = SignalObjectAndWait(gDisplayTrigger, gDisplayDoneTrigger, INFINITE, FALSE);

    #ifdef _LOGEVENTS
        LARGE_INTEGER CurTick;
        QueryPerformanceCounter( &CurTick );
        LOG(console.writef("SyncAndWait return, waittime=%.2f status=%d \n", ToMillisec(CurTick.QuadPart - WaitedDisplayPayloadStartTicks.QuadPart), waitstatus))
    #endif
}


void static __stdcall
WaitDisplay() {
    #ifdef _LOGEVENTS
        LARGE_INTEGER StartTick;
        QueryPerformanceCounter( &StartTick );
    #endif

    int displaybusy = g_pChitin->bInSyncUpdate;
    gDontWaitNextAIUpdate = false;                                // wait mode
    int waitstatus = WaitForSingleObject(gDisplayNotBusy, INFINITE);  // wait if SynchronousUpdate() is busy

    #ifdef _LOGEVENTS
        LARGE_INTEGER CurTick;
        QueryPerformanceCounter( &CurTick );
        LOG(console.writef("Async wait return, waittime=%.2f displaystate_before= %d \n", ToMillisec(CurTick.QuadPart - StartTick.QuadPart), displaybusy))
    #endif
}


BOOL static __stdcall
CBaldurChitin_MainAIThread_DoubleRateFrame() {
    static LARGE_INTEGER last_tick = {0};

    if (!g_pChitin->bDisableWindow) {

        #ifdef _LOGEVENTS
            LOGTICK(console.writef("Double rate timer"))
        #endif

        if (g_pChitin->bInSyncUpdate) {     // SynchronousUpdate() not finished yet
            gDontWaitNextAIUpdate = true;   // mark "no-wait" mode

            #ifdef _LOGEVENTS
                LOG(console.writef(" display busy, next frame will be in no wait mode \n"))
            #endif
        }

        if (gSkipAIUpdateThisFrame == true) {   // bounce normal/lite mode
            gSkipAIUpdateThisFrame = false;

            #ifdef _LOGEVENTS
                LOG(console.writef(" request display normal \n"))
            #endif

            return  FALSE;          // normal mode, run AIUpdate(), then trigger SynchronousUpdate()
        } else {
            gSkipAIUpdateThisFrame = true;
            UpdateMousePosition();

            #ifdef _LOGEVENTS
                LOG(console.writef(" request display without ai \n"))
            #endif

            g_pChitin->bDisplayStale = 1;   // enable  SynchronousUpdate()
            SetEvent(gDisplayTrigger);      // trigger SynchronousUpdate()

            return  TRUE;           // skip AIUpdate(), wait next timer event
        }
    } else {
        return  TRUE;           // skip AIUpdate(), wait next timer event
    }
}


BOOL static __stdcall
CheckMouseOnlyMode(CEngine* screen) {
    static LARGE_INTEGER last_tick ;

    // due incremental rendering to backbuffer, Flip()/Swap(), and again to ex-frontbuffer
    // MouseOnlyMode works only in windowed DirectDraw mode (render to backbuffer only)
    if (screen == g_pChitin->pScreenWorld &&    // only for ScreenWorld, other screens has
                                                // enough cpu/display power to full rendering at double rate
        gSkipAIUpdateThisFrame == true &&       // skip normal(AI timer) frames
        !g_pChitin->cVideo.bIs3dAccelerated &&  // no opengl
        !g_pChitin->bFullScreen ) {             // no fullscreen
       
        #ifdef _LOGEVENTS
            LOGTICK(console.writef("LiteSync()"))
        #endif

        return TRUE;
    }
    else {
        #ifdef _LOGEVENTS
            LOGTICK(console.writef("FullSync()"))
        #endif

        return FALSE;
    }
}


bool static __stdcall
IsNeedHandling(uint* pNewStatus) {
    int prevMode = gIsGameWindowForeground;

    gIsGameWindowForeground = *pNewStatus;
    if (g_pChitin->bFullScreen == false) {
        return (prevMode==0 &&               // was inactive
                gIsGameWindowForeground==1); // now active
    } else {
        return  true;   // disable "unpause" for full screen because engine still render graphics to active desktop
                        // after switching to other application
    }
}


void NightMare_SaveGame(CSavedGameHeader& savegame);
void NightMare_LoadGame(CSavedGameHeader& savegame);
extern bool g_GreyButtonOn;

void static __stdcall
CInfGame_Marshal_SaveGame(CSavedGameHeader& savegame) {
    if (pGameOptionsEx->bEngine_AddPauseToSaveGame) {
        if (g_pChitin->pScreenWorld->bPaused)
            savegame.dwFlags |= 0x100;
    }

    if (pGameOptionsEx->bUI_NightmareMode) {
        NightMare_SaveGame(savegame);
    }

    if (pGameOptionsEx->bUI_GreyBackgroundOnPause) {
        if (g_GreyButtonOn)
            savegame.dwFlags |= 0x400;
    }
}

void __stdcall
CInfGame_UnMarshal_LoadGame(CSavedGameHeader& savegame) {
    if (pGameOptionsEx->bEngine_AddPauseToSaveGame) {
        if (savegame.dwFlags & 0x100) {
            //g_pChitin->pGame->m_WorldTimer.PauseGame();
            //g_pChitin->pScreenWorld->bPaused = 1;
            g_pChitin->pScreenWorld->TogglePauseGame(true, TRUE, 0);
        }
    }

    if (pGameOptionsEx->bUI_NightmareMode) {
        NightMare_LoadGame(savegame);
    }

    if (pGameOptionsEx->bUI_GreyBackgroundOnPause) {
        if (savegame.dwFlags & 0x400) 
            g_GreyButtonOn = true;
        else
            g_GreyButtonOn = false;
    }
}


int __cdecl
z_uncompress(uchar *dest, ulong *destLen, uchar *source, ulong sourceLen) {
    return uncompress(dest, destLen, source, sourceLen);
}

int __cdecl
z_compress2(uchar *dest, ulong *destLen, uchar *source, ulong sourceLen, int level) {
    return compress2(dest, destLen, source, sourceLen, level);
}


bool
IsBG1Part() {
    bool       BG1part;
    bool       BG2afterBG1;
    CVariable& bg1done_marker = g_pChitin->pGame->m_GlobalVariables.Find("ENDOFBG1");
    CVariable& bg1part_marker = g_pChitin->pGame->m_GlobalVariables.Find("TETHTORIL");

    if (&bg1part_marker != NULL) {
		BG1part = true;
    } else {
        BG1part = false;
    }

	if (&bg1done_marker != NULL) {
		BG2afterBG1 = bg1done_marker.nValue > 0 ? true : false;
    } else {
        BG2afterBG1 = false;
    }

    if (BG1part && BG2afterBG1 == false)
        return true;
    else
        return false;
}


void  __declspec(naked)
EngineNoActive_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    0           // bAlertable
    push    200         // dwMilliseconds
    mov     eax, [SleepEx]
    call    eax

    pop     edx
    pop     ecx
    pop     eax

    add     esp, 4      // kill retadr
    push    09A64FDh    // Stolen bytes
    ret
}
}


void  __declspec(naked)
CChitin_WinMain_AddWaitForEvent_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    call    WinMain_DisplayBegin

    pop     edx
    pop     ecx
    pop     eax

    cmp     dword ptr [edx+405Eh], 1    // Stolen bytes
    ret
}
}


void  __declspec(naked)
CBaldurChitin_MainAIThread_TriggerDisplaySync_asm() {
__asm {
    mov     dword ptr [ecx+405Eh], 1    // Stolen bytes

    push    eax
    push    ecx
    push    edx

    call    CBaldurChitin_MainAIThread_TriggerDisplaySync

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void  __declspec(naked)
CChitin_WinMain_AddDisplayDoneEvent_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    call    WinMain_DisplayDoneSync

    pop     edx
    pop     ecx
    pop     eax

    mov     dword ptr [edx+405Ah], 1    // Stolen bytes
    ret
}
}


void  __declspec(naked)
CBaldurChitin_MainAIThread_DoubleRateFrame_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    call    CBaldurChitin_MainAIThread_DoubleRateFrame
    test    eax,eax

    pop     edx
    pop     ecx
    pop     eax
    jnz     CBaldurChitin_MainAIThread_DoubleRateFrame_SkipAIUpdate

    cmp     dword ptr [edx+405Ah], 1    // Stolen bytes

CBaldurChitin_MainAIThread_DoubleRateFrame_SkipAIUpdate:
    ret         // return Z flag
                // Z=1 run  AIUpdate()
                // Z=0 skip AIUpdate()
}
}


void  __declspec(naked)
TimeGetTime_WaitCycle_asm() {
__asm {
    lea     eax, [edi+edx*8]    // Stolen bytes
    test    eax, eax
    jl      TimeGetTime_WaitCycle_sleep
    ret

TimeGetTime_WaitCycle_sleep:
    push    eax
    push    ecx
    push    edx

    push    0               // bAlertable
    push    5               // dwMilliseconds, 0x9A4DA9 set minimal timer resolution 5 ms
    mov     eax, [SleepEx]
    call    eax

    pop     edx
    pop     ecx
    pop     eax

    test    eax, eax        // Stolen bytes
    ret
}
}


void  __declspec(naked)
RenderOneFrame_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    0
    call    RenderOneFrame

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void  __declspec(naked)
WaitDisplay_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    call    WaitDisplay

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


#define MaxFrameRate 0B60628h


void  __declspec(naked)
CBaldurChitin_InitXXX_SetKeyboardSpeed_asm() {
__asm {
    mov     eax, ds:MaxFrameRate
    shl     eax, 1  // MaxFrameRate*2
   
    ret
}
}


void  __declspec(naked)
SleepEx_Emu_asm() {
__asm {
    // arg0 - time
    // arg1 - alertable

    push    eax
    push    ecx
    push    edx

    push    1
    call    RenderOneFrame

    pop     edx
    pop     ecx
    pop     eax

    ret     8
}
}


void  __declspec(naked)
CChitin_SynchronousUpdate_CheckScreen_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    edx    ; pEngine
    call    CheckMouseOnlyMode
    test    eax, eax
    jz      CChitin_SynchronousUpdate_Normal

    mov     ecx, [esp+0]    // pEngine
    push    1
    mov     ecx, [ecx+4]    // CVidMode
    mov     eax, [ecx]      // vtable
    call    [eax+0B4h]      // call only CVidInf::Flip()

    pop     edx
    pop     ecx
    pop     eax
    ret

CChitin_SynchronousUpdate_Normal:
    pop     edx
    pop     ecx
    pop     eax

    call    [eax+0C4h]      // Stolen bytes, call full SynchronousUpdate()
    ret
}
}


void  __declspec(naked)
CChitin_OnAltTab_asm() {
__asm {
    mov     [ebp-20h], ecx  // orig code
    push    ecx
    push    edx

    lea     eax, [ebp+0Ch]    ; window status
    push    eax
    call    IsNeedHandling
    pop     edx
    pop     ecx

    cmp     al, 1
    jz      CChitin_OnAltTab_continue

    add     esp, 4   // kill ret addr
    push    09A6FB3h // skip handling
    ret

CChitin_OnAltTab_continue:
    cmp     dword ptr [ebp+0Ch], 1  // Stolen bytes
    ret
}
}


void  __declspec(naked)
CChitin_AsynchronousUpdate_CheckWindowEdge_asm() {
__asm {
    mov     cl, [eax+0F9h]  // orig code
    cmp     dword ptr [gIsGameWindowForeground], 1
    jz      CChitin_AsynchronousUpdateCheckWindowEdge_exit

    xor     cl,cl   // disable Window Edges for mouse

    CChitin_AsynchronousUpdateCheckWindowEdge_exit:
    ret
}
}



void  __declspec(naked)
CChitin_AsynchronousUpdate_CheckCursorPT_asm() {
__asm {
    cmp     dword ptr [gIsGameWindowForeground], 1
    jz      CChitin_AsynchronousUpdate_CheckCursorPT_continue

    xor     eax,eax   // return mouse X,Y out of range
    ret     12

CChitin_AsynchronousUpdate_CheckCursorPT_continue:
    jmp     DWORD ptr [PtInRect]
}
}


void  __declspec(naked)
GetAsyncKeyState_asm() {
__asm {
    cmp     dword ptr [gIsGameWindowForeground], 1
    jz      GetAsyncKeyState_continue

    xor     eax,eax   // no any key pressed
    ret     4

GetAsyncKeyState_continue:
    jmp     DWORD ptr [GetAsyncKeyState]
}
}


void  __declspec(naked)
CInfGame_Marshal_SaveGameInject_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-84Ch]  // CSavedGameHeader
    call    CInfGame_Marshal_SaveGame

    pop     edx
    pop     ecx
    pop     eax

    mov     cl, [eax+4C0Ch]  // Stolen bytes
    ret
}
}


void  __declspec(naked)
CInfGame_UnMarshal_LoadGameInject_asm() {
__asm {
    mov     [ebp-20h], edx      // Stolen bytes
    push    ecx
    push    edx

    push    edx  // CSavedGameHeader
    call    CInfGame_UnMarshal_LoadGame

    pop     edx
    pop     ecx

    mov     eax, [ebp-30h]    // Stolen bytes
    ret
}
}


void  __declspec(naked)
FakeGetDiskFreeSpaceA_asm() {
__asm {
    xor     eax, eax
    ret     5*4
}
}