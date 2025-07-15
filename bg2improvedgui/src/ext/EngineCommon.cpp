#include "EngineCommon.h"

#include "objcre.h"
#include "infgame.h"
#include "chitin.h"
#include "tlkcore.h"
#include "animext.h"

#define  ZLIB_WINAPI
#include "zlib.h"

uint gIsGameWindowForeground = 1;   // default is ON

DWORD orig_SleepEx;

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


//#define  _LOGTICKEVENTS
//#define  _LOGTICKEVENTS2


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



void static
UpdateMousePosition() {
    // update every frame
    static LARGE_INTEGER last_tick = {0};

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


void static __stdcall
WinMain_DisplayBegin() {
    int* MaxRefreshRate = (int*) 0xB60628;
    static unsigned int MaxRefreshPeriodMs = 1000 / *MaxRefreshRate;
    int waitstatus = 0;
    static LARGE_INTEGER last_tick = {0};
    static LARGE_INTEGER lastdone_tick = {0};

    #ifdef _LOGTICKEVENTS
        LARGE_INTEGER StartWaitTick, CurTick;
        QueryPerformanceCounter( &StartWaitTick );
    #endif

    //#ifdef _LOGTICKEVENTS2
    //    LOGTICK(console.writef("DisplayBegin"))
    //#endif

    if (gDontWaitNextAIUpdate == false) {
        #ifdef _LOGTICKEVENTS
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
        #ifdef _LOGTICKEVENTS
            if (!g_pChitin->bDisableWindow) {
                console.writef("\n");
            }
            LOGTICK(console.writef("> No display wait !!! request== %d", g_pChitin->bDisplayStale))
        #endif
    }

    #ifdef _LOGTICKEVENTS
        QueryPerformanceCounter( &CurTick );
        LOGTICK2(lastdone_tick, console.writef(">  waiting is done, request= %d waittime=%.2f waitstatus= %d", g_pChitin->bDisplayStale, ToMillisec(CurTick.QuadPart - StartWaitTick.QuadPart), waitstatus))
    #endif

    gDontWaitNextAIUpdate = false;
    ResetEvent(gDisplayTrigger);

    #ifdef _LOGTICKEVENTS
        QueryPerformanceCounter(&gDisplayPayloadStartTicks);
    #endif

    if (g_pChitin->bDisplayStale)       // if display rendering sheduled
        ResetEvent(gDisplayNotBusy);    // inform waiters about forthcoming rendering busy state
}



void static __stdcall
WinMain_DisplayDoneSync() {
    static LARGE_INTEGER last_tick = {0};

    #ifdef _LOGTICKEVENTS
        LOGTICK(console.writef("DisplayDone"))
        LOG(console.writef(" display payload= %.2f \n", ToMillisec(CurTick.QuadPart - gDisplayPayloadStartTicks.QuadPart)))
    #endif

    //#ifdef _LOGTICKEVENTS2
    //    LOGTICK(console.writef("DisplayDone"))
    //    console.writef("\n");
    //#endif

    SetEvent(gDisplayDoneTrigger);  // resume waiting threads
    SetEvent(gDisplayNotBusy);      // can accept next gDisplayTrigger event
}


void static __stdcall
CBaldurChitin_MainAIThread_TriggerDisplaySync() {
    static LARGE_INTEGER last_tick = {0};

    #ifdef _LOGTICKEVENTS2
        LOGTICK(console.writef("MainAIThread() done"))
    #endif

    SetEvent(gDisplayTrigger);        // trigger SynchronousUpdate()
}


void static __stdcall
RenderOneFrame(BOOL bSleepRequested) {
    //static LARGE_INTEGER last_tick ;

    #ifdef _LOGTICKEVENTS2
        console.writef("RenderOneFrame \n");
    #endif

    #ifdef _LOGTICKEVENTS
        if (bSleepRequested)
            console.writef("SyncAndWait SleepRequested \n");

        LOG(console.writef("SyncAndWait wait display, request= %d \n", g_pChitin->bInSyncUpdate))
    #endif

    gDontWaitNextAIUpdate = false;  // wait mode
    int waitstatus = WaitForSingleObject(gDisplayNotBusy, INFINITE); // wait if current SynchronousUpdate() is busy

    #ifdef _LOGTICKEVENTS
        LOG(console.writef(" SyncAndWait request display job\n"))

        LARGE_INTEGER WaitedDisplayPayloadStartTicks;
        QueryPerformanceCounter(&WaitedDisplayPayloadStartTicks);
    #endif
    
    ResetEvent(gDisplayDoneTrigger);
    // trigger & wait SynchronousUpdate() for one time
    g_pChitin->bDisplayStale = 1;   // enable  SynchronousUpdate()
    gSkipAIUpdateThisFrame = false; // full render
    waitstatus = SignalObjectAndWait(gDisplayTrigger, gDisplayDoneTrigger, INFINITE, FALSE);

    #ifdef _LOGTICKEVENTS
        LARGE_INTEGER CurTick;
        QueryPerformanceCounter( &CurTick );
        LOG(console.writef("SyncAndWait return, waittime=%.2f status=%d \n", ToMillisec(CurTick.QuadPart - WaitedDisplayPayloadStartTicks.QuadPart), waitstatus))
    #endif
}


void static __stdcall
WaitDisplay() {
    #ifdef _LOGTICKEVENTS
        LARGE_INTEGER StartTick;
        QueryPerformanceCounter( &StartTick );
    #endif

    int displaybusy = g_pChitin->bInSyncUpdate;
    gDontWaitNextAIUpdate = false;                                // wait mode
    int waitstatus = WaitForSingleObject(gDisplayNotBusy, INFINITE);  // wait if SynchronousUpdate() is busy

    #ifdef _LOGTICKEVENTS
        LARGE_INTEGER CurTick;
        QueryPerformanceCounter( &CurTick );
        LOG(console.writef("Async wait return, waittime=%.2f displaystate_before= %d \n", ToMillisec(CurTick.QuadPart - StartTick.QuadPart), displaybusy))
    #endif
}


void static __stdcall
CGameArea_Render_Log() {
    //console.writef("CGameArea:Render() \n");
}


//#define  _LOGTICKEVENTS3

static long lastScrollState = 0;

void static __stdcall
AdjustViewPosition_ContinueScroll(unsigned char *nScrollState) {
    //int dummy;

    #ifdef _LOGTICKEVENTS3
        console.writef("\nAdjustViewPosition(%d) \n", *nScrollState);
    #endif

    CEngine*   pCurrentScreen = g_pChitin->pEngineActive;
    CScreenWorld* screen = g_pChitin->pScreenWorld;

    if (pCurrentScreen  == (CEngine*) g_pChitin->pScreenWorld) {
        CArea* pArea = g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx];

        if (gSkipAIUpdateThisFrame) { // No AI mode
            #ifdef _LOGTICKEVENTS3
                console.writef(" no AI mode, prev = %d \n", lastScrollState);
            #endif

            //if (*nScrollState != lastScrollState) 
            //    dummy = 1;

            *nScrollState = (unsigned char) lastScrollState;
            lastScrollState = pArea->nKeyScrollState;
        } else { // normal mode
            lastScrollState = pArea->nKeyScrollState;
        }

        #ifdef _LOGTICKEVENTS3
            console.writef("Area.nScrollState=%d Area.nKeyScrollState=%d nScrollState=%d \n",
            pArea->nScrollState, 
            pArea->nKeyScrollState,
            *nScrollState);
        #endif
    }
}


uint static __stdcall
AdjustViewPosition_GetScrollSpeed(unsigned char *nScrollState) {
    unsigned int result;
    CInfGame *pGame= g_pChitin->pGame;
    //int dummy;

#ifdef _LOGTICKEVENTS3
    //if (gSkipAIUpdateThisFrame)
    //    console.writef("GetScrollSpeed no AI  mode nScrollState=%d \n", *nScrollState);
    //else
    //    console.writef("GetScrollSpeed normal mode \n");
#endif

    if (pGame->m_pLoadedAreas[pGame->m_VisibleAreaIdx]->nKeyScrollState ||
        (gSkipAIUpdateThisFrame && *nScrollState))
        result = pGame->m_GameOptions.m_nKeyBoardScrollAmount;
    else
        result = pGame->m_GameOptions.m_scrollSpeed;

#ifdef _LOGTICKEVENTS3
    //console.writef("GetScrollSpeed()=%d \n", result);
#endif

    return result;
}


void static __stdcall
AdjustViewPosition_Log(int ScrollSpeed) {
    //console.writef("scrollSpeed=%d \n", ScrollSpeed);
}


void static __stdcall
Scroll_Log() {
    console.writef("CInfinity::Scroll() \n");
}


void static __stdcall
CInfinity_Render_Log(CInfinity& inf) {
    console.writef("CInfinity:Render() \n");
}



void static __stdcall
SleepEx_Log(DWORD EIP, DWORD time) {
    if (EIP != 0x436312 &&
        1)
        console.writef("SleepEx() ip=%X time=%d \n", EIP, time);
}


BOOL static __stdcall
CBaldurChitin_MainAIThread_DoubleRateFrame() {
    static LARGE_INTEGER last_tick = {0};

    if (!g_pChitin->bDisableWindow) {

        #ifdef _LOGTICKEVENTS2
            console.writef("\nDouble rate timer \n");

            CEngine*   pCurrentScreen = g_pChitin->pEngineActive;
            CScreenWorld* screen = g_pChitin->pScreenWorld;
            if (pCurrentScreen  == (CEngine*) g_pChitin->pScreenWorld) {
                CArea* pArea = g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx];
                console.writef("ptCurrentPosExact X=%d Y=%d %d \n",
                    pArea->m_cInfinity.ptCurrentPosExact.x/10000,
                    pArea->m_cInfinity.ptCurrentPosExact.y/10000,
                    pArea->m_cInfinity.LastTickCount);
            }
            


        #endif

        if (g_pChitin->bInSyncUpdate) {     // SynchronousUpdate() not finished yet
            gDontWaitNextAIUpdate = true;   // mark "no-wait" mode

            #ifdef _LOGTICKEVENTS
                LOG(console.writef(" display busy, next frame will be in no wait mode \n"))
            #endif
        }

        if (gSkipAIUpdateThisFrame == true) {   // bounce normal/lite mode
            gSkipAIUpdateThisFrame = false;

            #ifdef _LOGTICKEVENTS2
                LOG(console.writef(" request display normal \n"))
            #endif

            return  FALSE;          // normal mode, run AIUpdate(), then trigger SynchronousUpdate()
        } else {
            gSkipAIUpdateThisFrame = true;
            UpdateMousePosition();

            #ifdef _LOGTICKEVENTS2
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
       
        #ifdef _LOGTICKEVENTS
            LOGTICK(console.writef("LiteSync()"))
        #endif

        return TRUE;
    }
    else {
        #ifdef _LOGTICKEVENTS
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


CVariable *
LoadBG1Var(CSavedGameHeader* savegame, int mode) {
    uint i;
    char *saved_str;
    static CVariable *BG1part;
    static CVariable *BG2afterBG1;

    if (mode == 0) {
        BG1part = NULL;
        BG2afterBG1 = NULL;

        for (i = 0; i < savegame->globalVariablesCount; ++i ) {
            saved_str = (char *) savegame - 8 + savegame->globalVariablesOffset + 84 * i;
            if (strcmp(saved_str,"ENDOFBG1") == 0)
                BG2afterBG1 = (CVariable *) saved_str;
            if (strcmp(saved_str,"TETHTORIL") == 0)
                BG1part = (CVariable *) saved_str;
        }

        return NULL;
    }
    else 
    if (mode == 1) {    // BG1part
        return (CVariable *) BG1part;
    }
    else {              // BG2afterBG1
        return (CVariable *) BG2afterBG1;
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

    if (pGameOptionsEx->bEngine_LimitXP) {
        if(IsBG1Part() == 1)
            g_pChitin->pGame->bBG2 = FALSE; // clear bg2 flag
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

    if (pGameOptionsEx->bEngine_BGTMovementSpeed) {
        LoadBG1Var(&savegame, 0);
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
    CVariable *bg1done_marker = & g_pChitin->pGame->m_GlobalVariables.Find("ENDOFBG1");
    CVariable *bg1part_marker = & g_pChitin->pGame->m_GlobalVariables.Find("TETHTORIL");


    if (bg1done_marker == NULL &&  // called during loading game, no any VARS loaded yet
        bg1part_marker == NULL) {
        bg1part_marker =  LoadBG1Var(NULL, 1);
        bg1done_marker =  LoadBG1Var(NULL, 2);
    }


    if (bg1part_marker != NULL) {
		BG1part = true;
    } else {
        BG1part = false;
    }

	if (bg1done_marker != NULL) {
		BG2afterBG1 = bg1done_marker->intValue > 0 ? true : false;
    } else {
        BG2afterBG1 = false;
    }

    if (BG1part && BG2afterBG1 == false)
        return true;
    else
        return false;
}


void __stdcall
CProjectileBAM_AIUpdate_LogTick() {
    static uint i =0;

    console.writef("AIUpdate=%d \n", i++);
}


void __stdcall
CGameSprite_AIUpdateFly_LogTick(CCreatureObject &cre) {
    static uint i =0;

    if (cre.animation.pAnimation->wAnimId == 0xD000)
        console.writef("AIUpdate=%d \n", i++);
}

void __stdcall
CProjectileBAM_Render_LogTick(CProjectileBAM &pro) {
    static uint i =0;
    uint x = pro.currentLoc.x;
    uint y = pro.currentLoc.y;

    console.writef("Render=%d x=%d y=%d \n", i++, x, y);
}


void __stdcall
CAnimation_Render_LogTick(CAnimation &anim, POINT &pt) {
    static uint i =0;
    //uint x = pro.currentLoc.x;
    //uint y = pro.currentLoc.y;

    console.writef("Render=%d x=%d y=%d \n", i++, pt.x, pt.y);
}


void __stdcall
CProjectileBAM_CProjectileBAM_PatchTickRate(CProjectileBAM &pro, ResRef &resBAM, DWORD EIP, DWORD *Stack) {
    if (pro.nAIUpdateInterval == 1) {    //  every 2nd tick
        IECString *str = NULL;
        ResRef res;

        if (EIP == 0x602D39) {  //CProjectile::DecodeProjectile()
            str = (IECString *) (*(DWORD *)(Stack[0] - 0x38) + 4);  // see 0x602ACE
            res = *str;
        } else
        if (EIP == 0x608CE0) {  //CProjectileArea::CProjectileArea()
            Stack = (DWORD*) Stack[0];
            if (Stack[1] == 0x602E68) {  //CProjectile::DecodeProjectile()
                str = (IECString *) (*(DWORD *)(Stack[0] - 0x38) + 4);  // see 0x602E3C
                res = *str;

                Stack = (DWORD*) Stack[0];
                if (Stack[1] == 0x4ADF21) {  //CGameAIBase::FireSpell()
                    Stack = (DWORD*) Stack[0];
                    if (Stack[1] == 0x46C3BC ||  //CContingencyList::TriggerSequencer()
                        Stack[1] == 0x46C3F0 ||
                        Stack[1] == 0x46C424 ||

                        Stack[1] == 0x46BAE2 ||  //CContingencyList::ProcessTrigger()
                        Stack[1] == 0x46BB16 ||
                        Stack[1] == 0x46BB4A ||

                        Stack[1] == 0x46BFA9 ||  //CContingencyList::Process()
                        Stack[1] == 0x46BFDD ||
                        Stack[1] == 0x46C011 ) {
                            return;              // skip Contingency/Sequencer to keep child Projectile::AIUpdate() in sync
                    }

                }
            }
        }

        if (str) {
            /*if (res == "ACIDBLGR"   ||
                res == "ACIDBLMU"   ||
                res == "ACIDBLOB"   ||
                res == "ACIDBLOC"   ||
                res == "ARROW"      ||
                res == "ARROWEX"    ||
                res == "ARROWFLB"   ||
                res == "ARROWFLG"   ||
                res == "ARROWFLI"   ||
                res == "ARROWFLM"   ||
                res == "ARROWHVY"   ||
                res == "AXE"        ||
                res == "AXEEX"      ||
                res == "BOLT"       ||
                res == "BOLT2"      ||
                res == "BOLTEX"     ||
                res == "BULLET"     ||
                res == "BULLETEX"   ||
                res == "cdbehbla"   ||
                res == "CHROMORB"   ||
                res == "CLOUD"      ||
                res == "CLOUDKIL"   ||
                res == "CLOUDPC"    ||
                res == "COMET"      ||
                res == "DAGGER"     ||
                res == "DAGGEREX"   ||
                res == "DART"       ||
                res == "DARTEX"     ||
                res == "DBFIREBL"   ||
                res == "DFIREBL"    ||
                res == "DRAGBLCK"   ||
                res == "DRAGGREE"   ||
                res == "DRAGRED"    ||
                res == "DRAGSILV"   ||
                res == "ENTANG2"    ||
                res == "FIREBALL"   ||
                res == "FIREBLGR"   ||
                res == "FIREBLIC"   ||
                res == "FIREBLNS"   ||
                res == "FIREBOLT"   ||
                res == "FIREBTBL"   ||
                res == "FIRESTOR"   ||
                res == "GOLCLOUD"   ||
                res == "GREASE"     ||
                res == "ICEGLYP"    ||
                res == "ICESTORM"   ||
                res == "INSEC1"     ||
                res == "INSEC2"     ||
                res == "INSEC3"     ||
                res == "INSEC4"     ||
                res == "LIGHTBLT"   ||
                res == "LIGHTSTO"   ||
                res == "MAGICMIS"   ||
                res == "METSWARM"   ||
                res == "PCOMETT"    ||
                res == "PFIELD"     ||
                res == "PFIRE2"     ||
                res == "PFIRE3"     ||
                res == "PSKULLT"    ||
                res == "PSPEART"    ||
                res == "PWILT"      ||
                res == "SAFEFIRE"   ||
                res == "SPATTCK2"   ||
                res == "SPBEHBLA"   ||
                res == "SPEAR"      ||
                res == "SPEAREX"    ||
                res == "SPENBLD"    ||
                res == "SPGREORB"   ||
                res == "STONE"      ||
                res == "STRMVENG"   ||
                res == "TRAPDART"   ||
                res == "TRAPGLPN"   ||
                res == "TRAPGLYP"   ||
                res == "TRAPSKUL"   ||
                res == "TSKULLNP"   ||
                res == "WEB"        ||
                res == "WEB1P"      ||*/
            if (resBAM == "ACIDBLOB"    ||
                resBAM == "ARARROW"     ||
                resBAM == "SPFLMARR"    ||
                resBAM == "AXE"         ||
                resBAM == "BOLT"        ||
                resBAM == "SPFIRSED"    ||
                resBAM == "MAGICSTN"    ||
                resBAM == "SPBEHBLA"    ||
                resBAM == "SPFIREBL"    ||
                resBAM == "SPCHRORB"    ||
                resBAM == "STNKCLDT"    ||
                resBAM == "DAGGER"      ||
                resBAM == "DART"        ||
                resBAM == "SPFIRET"     ||
                resBAM == "SPGLYPTI"    ||
                resBAM == "SPLIGHTB"    ||
                resBAM == "SPMAGMIS"    ||
                resBAM == "SPCOMETT"    ||
                resBAM == "SPSKULLT"    ||
                resBAM == "SPSPEART"    ||
                resBAM == "SPATTCK2"    ||
                resBAM == "SPEAR"       ||
                resBAM == "SPENBLD"     ||
                resBAM == "SPGREORB"    ||
                resBAM == "TRDARTST"    ||
                resBAM == "GLPHWRDT"    ||
                resBAM == "SKLT"        ||
                resBAM == "WEBENTT"     ||
                0 ){
                    pro.nAIUpdateInterval = 0;  //  every tick
                    pro.nSpeed /= 2;            //  adjust speed
            }
        }
    }
}


void __stdcall
CProjectileBAM_CProjectileBAM_PatchSpeed(CProjectileBAM &pro, DWORD EIP, DWORD *Stack) {
static short firstspeed = 30;

    if (EIP == 0x608CE0) {  //CProjectileArea::CProjectileArea()
        Stack = (DWORD*) Stack[0];
        if (Stack[1] == 0x602E68) {  //CProjectile::DecodeProjectile()
            Stack = (DWORD*) Stack[0];
            if (Stack[1] == 0x4ADF21) {  //CGameAIBase::FireSpell()
                Stack = (DWORD*) Stack[0];

                //CContingencyList::TriggerSequencer
                if (Stack[1] == 0x46C3BC) { // first
                        firstspeed = pro.nSpeed > 15 ? pro.nSpeed : 15;        //  fastest
                } else
                if (Stack[1] == 0x46C3F0) {  // second
                        pro.nSpeed = firstspeed - 5;    //  mid
                } else
                if (Stack[1] == 0x46C424) { //  last
                        pro.nSpeed = firstspeed - 10;   //  slowest
                }

            }
        }
    }
}

void __stdcall
CAnimationD000_CAnimationD000_PatchTickRate(CAnimationD000 &anim, ushort AnimID, char* pColorRanges) {
    CCreatureObject *cre  =  (CCreatureObject *) (pColorRanges - 0x41A);
    if ((AnimID & 0xF000) == 0xD000 ){    // eagle, seagull, vulture, bird, bird inside
                cre->nAIUpdateInterval = 0;     //  every tick
                anim.nMovementRateDefault /= 2; //  adjust speed
                anim.nMovementRateCurrent /= 2;
        }
}


DWORD __stdcall
CGameSprite_ProcessEffectList_IgnoreFly(CCreatureObject &cre) {
    ushort AnimID = cre.animation.pAnimation->wAnimId;
    if ((AnimID & 0xF000) == 0xD000 )    // eagle, seagull, vulture, bird, bird inside
        return 1;  // fake nAIUpdateInterval = 1
    else
        return cre.nAIUpdateInterval;
}



void __stdcall
CGameSprite_AIUpdate_IncrementFrame(CCreatureObject &cre) {
    ushort AnimID = cre.animation.pAnimation->wAnimId;
    if ((AnimID & 0xF000) == 0xD000 ){    // eagle, seagull, vulture, bird, bird inside
        if (cre.animation.pAnimation->u3df == 1) {
            cre.animation.pAnimation->u3df = 0;
            cre.animation.pAnimation->IncrementFrame();
        } else
            cre.animation.pAnimation->u3df = 1;
            // skip even IncrementFrame() because we have x2 nAIUpdateInterval
        }
    else
        cre.animation.pAnimation->IncrementFrame();
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
    xor     eax, eax    // force return error
    ret     5*4
}
}


void  __declspec(naked)
CProjectileBAM_AIUpdate_LogTick_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    call    CProjectileBAM_AIUpdate_LogTick

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [eax+42BAh]  // stolen bytes
    ret
}
}

void  __declspec(naked)
CProjectileBAM_Render_LogTick_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    ecx
    call    CProjectileBAM_Render_LogTick

    pop     edx
    pop     ecx
    pop     eax

    mov     [ebp-0B0h], ecx  // stolen bytes
    ret
}
}


void  __declspec(naked)
CProjectileBAM_CProjectileBAM_asm() {
__asm {

    push    eax
    add     ecx, 0FCh   // stolen bytes
    push    ecx
    push    edx

    push    ebp         // Parent EBP
	push    [ebp+4]     // EIP
    push    ecx         // ResRef
    push    [ebp-0A0h]  // CProjectileBAM
    call    CProjectileBAM_CProjectileBAM_PatchTickRate

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void  __declspec(naked)
CAnimationD000_PatchTickRate_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+0Ch]   // colorranges
    push    [ebp+8]     // AnimID
    push    [ebp-38h]   // CAnimation
    call    CAnimationD000_CAnimationD000_PatchTickRate

    pop     edx
    pop     ecx
    pop     eax

    mov     dword ptr [ebp-4], 0FFFFFFFFh// stolen bytes

    ret
}
}

void  __declspec(naked)
CAnimation_Render_LogTick_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp+18h]   // CPoint *ptNewPos
    push    [ebp-68h]   // CAnimation
    call    CAnimation_Render_LogTick

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp+18h]  // stolen bytes
    mov     edx, [ecx]
    ret
}
}


void  __declspec(naked)
CGameSprite_AIUpdateFly_LogTick_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-490h] // Cre
    call    CGameSprite_AIUpdateFly_LogTick

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp-490h]  // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_ProcessEffectList_IgnoreFly_asm() {
__asm {

    push    ecx
    push    edx

    push    [ebp-880h] // Cre
    call    CGameSprite_ProcessEffectList_IgnoreFly
    ; eax - return

    pop     edx
    pop     ecx

    and     eax, 0FFh  // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameSprite_AIUpdate_IncrementFrame_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-490h] // Cre
    call    CGameSprite_AIUpdate_IncrementFrame

    pop     edx
    pop     ecx
    pop     eax

    ret
}
}


void  __declspec(naked)
CProjectileBAM_CProjectileBAM_PatchSpeed_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    ebp         // Parent EBP
	push    [ebp+4]     // EIP
    push    [ebp-0A0h]  // CProjectileBAM
    call    CProjectileBAM_CProjectileBAM_PatchSpeed

    pop     edx
    pop     ecx
    pop     eax

    add     edx, 0B6h   // stolen bytes
    ret
}
}


void  __declspec(naked)
CInfinity_Render_Log_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    push    [ebp-138h]  // CInfinity
    call    CInfinity_Render_Log

    pop     edx
    pop     ecx
    pop     eax

    mov     edx, [ecx+42BAh]   // stolen bytes
    ret
}
}


void  __declspec(naked)
CGameArea_Render_Log_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    call    CGameArea_Render_Log

    pop     edx
    pop     ecx
    pop     eax

    mov     eax, [ebp-0A0h]   // stolen bytes
    ret
}
}


void  __declspec(naked)
AdjustViewPosition_ContinueScroll_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp+8]
    push    eax
    call    AdjustViewPosition_ContinueScroll

    pop     edx
    pop     ecx
    pop     eax

    mov     cl, [eax+1A0h]  // stolen bytes
    ret
}
}


void  __declspec(naked)
AdjustViewPosition_Dividex2_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    //push    [ebp-34h]       // scrollSpeed
    //call    AdjustViewPosition_Log

    pop     edx
    pop     ecx
    pop     eax


    sub     eax, [edx+166h] // stolen bytes
    shr     eax, 1          // Tick_Diff/2
    ret
}
}


void  __declspec(naked)
Scroll_Log_asm() {
__asm {

    push    eax
    push    ecx
    push    edx

    call    Scroll_Log

    pop     edx
    pop     ecx
    pop     eax

    mov     [eax+166h], ecx  // stolen bytes
    ret
}
}


void  __declspec(naked)
AdjustViewPosition_GetScrollSpeed_asm() {
__asm {

    push    ecx
    push    edx

    lea     eax, [ebp+8]
    push    eax
    call    AdjustViewPosition_GetScrollSpeed

    pop     edx
    pop     ecx

    mov     [ebp-34h], eax
    mov     [ebp-1Ch], eax  // stolen bytes, eax
    ret
}
}


void  __declspec(naked)
SleepEx_Log_asm() {
__asm {
    
    push    ecx
    push    edx

    mov     eax, [esp+12]   // time
    mov     ecx, [esp+8]    // IP

    push    eax
    push    ecx
    call    SleepEx_Log

    //mov     eax, [esp+16]   // alertable
    //push    eax
    //mov     eax, [esp+8]    // time
    //push    eax
    //call    orig_SleepEx

    pop     edx
    pop     ecx

    ret     8
}
}
