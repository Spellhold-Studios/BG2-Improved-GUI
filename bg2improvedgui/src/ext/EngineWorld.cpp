#include "EngineWorld.h"

#include "chitin.h"

#include "mmsystem.h"
#include "dsound.h"

extern POSITION (CScreenWorld::*Tramp_CScreenWorld_PrintToConsole_6)(IECString&, IECString&, ABGR, ABGR, int, bool) =
	SetFP(static_cast<POSITION (CScreenWorld::*)(IECString&, IECString&, ABGR, ABGR, int, bool)>	(&CScreenWorld::PrintToConsole),	0x7DDE63);
extern POSITION (CScreenWorld::*Tramp_CScreenWorld_PrintToConsole_4)(IECString&, IECString&, int, bool) =
	SetFP(static_cast<POSITION (CScreenWorld::*)(IECString&, IECString&, int, bool)>				(&CScreenWorld::PrintToConsole),	0x7DE085);

POSITION DETOUR_CScreenWorld::DETOUR_PrintToConsoleColor(IECString& sLeft, IECString& sRight, ABGR colLeft, ABGR colRight, int nUserArg, bool bResetScrollbar) {
	char* buffer;
	if (LD.bFileOpen) {
		if (sLeft.IsEmpty()) {
			buffer = "[%.8X] %s%s\r\n";
		} else {
			buffer = "[%.8X] %s- %s\r\n";
		}
		LD.appendf(buffer, g_pChitin->pGame->m_WorldTimer.nGameTime, (LPCTSTR)sLeft, (LPCTSTR)sRight);
	}
	return (this->*Tramp_CScreenWorld_PrintToConsole_6)(sLeft, sRight, colLeft, colRight, nUserArg, bResetScrollbar);
}

POSITION DETOUR_CScreenWorld::DETOUR_PrintToConsole(IECString& sLeft, IECString& sRight, int nUserArg, bool bResetScrollbar) {
	char* buffer;
	if (LD.bFileOpen) {
		if (sLeft.IsEmpty()) {
			buffer = "[%.8X] %s%s\r\n";
		} else {
			buffer = "[%.8X] %s- %s\r\n";
		}
		LD.appendf(buffer, g_pChitin->pGame->m_WorldTimer.nGameTime, (LPCTSTR)sLeft, (LPCTSTR)sRight);
	}
	return (this->*Tramp_CScreenWorld_PrintToConsole_4)(sLeft, sRight, nUserArg, bResetScrollbar);
}


void static __stdcall
CScreenWorldMap_EngineGameInit(CManager& manager) {
    const ushort XRes    = *(( WORD *) (0xB6150C));
    const ushort YRes    = *(( WORD *) (0xB6150E));
    const ushort XRes640 = *(( WORD *) (0xAB90F0));
    const ushort YRes480 = *(( WORD *) (0xAB90F2));
    const ushort diff_X = (XRes - XRes640*(1 + g_pChitin->bDoubleResolution))/2;

    if (XRes != 640*(1 + g_pChitin->bDoubleResolution) ||
        YRes != 480*(1 + g_pChitin->bDoubleResolution)) {
        CPanel& panel = manager.GetPanel(0);
        panel.height = YRes;
        panel.width =  XRes;
        panel.pt2.x = 0;
        panel.pt2.y = 0;

        for (int i = 0; i <= 14; i++ ) {
	        CUIControl& control = panel.GetUIControl(i);
            if (&control && (i != 4) ) { // exclude main button
                control.pos.x += diff_X;
            }
        }
        CUIControl& controlL = panel.GetUIControl(0x10000002);  // text label
        if (&controlL)
            controlL.pos.x += diff_X;

        ResRef NewMOS;
        if (XRes == 800*(1 + g_pChitin->bDoubleResolution))
            NewMOS = "GUIMAP08";    // 800x600
        else
        if (XRes == 1024*(1 + g_pChitin->bDoubleResolution))
            NewMOS = "GUIMAP10";    // 1024x768
        else
            NewMOS = "GUIMAPWX";    // widescreen custom resolution

        CVidMosaic::ResMosContainer& ResHelper = panel.BackgroundMosaic.ResHelper;
        if (ResHelper.name != NewMOS) {
            if (ResHelper.pRes && !ResHelper.name.IsEmpty()) {
                if (ResHelper.bLoaded) {
                    ResHelper.pRes->RemoveFromHandler();
                    ResHelper.bLoaded = FALSE;
                }
                ResHelper.pRes = NULL;
            }
                
            Res *ResObj = g_pChitin->m_ResHandler.GetResObject(NewMOS, 1004, 1);    // 1004 = MOS type
            if (ResObj) {
                ResHelper.pRes = ResObj;
                ResHelper.name = NewMOS;

            } else {
                ResHelper.pRes = NULL;
                ResHelper.name.Clean();
            }
        }

        CUIButton& BigButton = (CUIButton&) panel.GetUIControl(4);
        //BigButton x = 6
        //BigButton y = 105
        BigButton.height = panel.height - BigButton.pos.y - 5;  // 5 - bottom border
        BigButton.width = XRes - BigButton.pos.x - 5;           // 5 - right border
    }
}


void static __stdcall
CScreenWorld_UnPause(int Mode) {
    POSITION pos;

    //console.write_debug("UnPause=%d \n", Mode);
    CObList& que = g_pChitin->m_mixer.PlayingNow;
    CArea* pArea = g_pChitin->m_mixer.pArea;
    pos = que.GetHeadPosition();
    while (pos != NULL) {
        CVoice *Voice = (CVoice *) que.GetNext(pos);
        bool unmute = false;
        if (Voice) {
            if (Voice->pSound && Voice->pSound->m_pSoundBuffer) { // if CSound exist
                CSound *snd = Voice->pSound;
                if (Mode == 1 && Voice->bPauseMode == 1) {
                    Voice->bPauseMode = 0;
                    //console.write_debug("Unpause static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                }
                if (Mode == 2 && Voice->bTimeStopMode == 1) {
                    Voice->bTimeStopMode = 0;
                    //console.write_debug("UnTimeStop static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                }
                if (Mode == 3 && Voice->bScreenMode == 1) {
                    Voice->bScreenMode = 0;
                    //console.write_debug("UnScreen static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                }

                if (Voice->bPauseMode    == 0 &&
                    Voice->bTimeStopMode == 0 &&
                    Voice->bScreenMode   == 0 &&
                    (snd->pArea == NULL || snd->pArea == pArea)) {   // play again only from current area if it is declared
                    LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)snd->m_pSoundBuffer;
                    DWORD dwPlay = Voice->DSBufPosition;
                    HRESULT hr;
                    hr = IDirectSoundBuffer_SetCurrentPosition(pDSB, dwPlay);
                    if (hr == S_OK) {
                        hr = IDirectSoundBuffer_Play(pDSB, 0, 0, 0); // play DS buffer, never looped
                        if (hr == S_OK) {
                            console.write_debug("Play static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                        }
                    }
                }
            } else
            if (Voice->m_pSoundBuffer) { // transferred from CSound to CVoice
                if (Mode == 1 && Voice->bPauseMode == 1) {
                    Voice->bPauseMode = 0;
                    //console.write_debug("UnPause temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                }
                if (Mode == 2 && Voice->bTimeStopMode == 1) {
                    Voice->bTimeStopMode = 0;
                    //console.write_debug("UnTimestop temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                }
                if (Mode == 3 && Voice->bScreenMode == 1) {
                    Voice->bScreenMode = 0;
                    //console.write_debug("UnScreen temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                }

                if (Voice->bPauseMode    == 0 &&
                    Voice->bTimeStopMode == 0 &&
                    Voice->bScreenMode   == 0 &&
                    (Voice->pArea == NULL || Voice->pArea == pArea)) {    // play again only from current area if it is declared
                    LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Voice->m_pSoundBuffer;
                    DWORD dwPlay = Voice->DSBufPosition;
                    HRESULT hr;
                    hr = IDirectSoundBuffer_SetCurrentPosition(pDSB, dwPlay);
                    if (hr == S_OK) {
                        hr = IDirectSoundBuffer_Play(pDSB, 0, 0, 0); // play DS buffer, never looped
                        if (hr == S_OK) {
                            console.write_debug("Play temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                        }
                    }
                }
            }
        }
    }

    //console.write_debug("PlayingNow=%d \n",         g_pChitin->m_mixer.PlayingNow.GetCount());

    //console.write_debug("AllLoopingQueue=%d \n",    g_pChitin->m_mixer.AllLoopingQueue.GetCount());
    //CObList& que2 = g_pChitin->m_mixer.AllLoopingQueue;
    //pos = que2.GetHeadPosition();
    //while (pos != NULL) {
    //    CSound *snd = (CSound *) que2.GetNext(pos);
    //    if (snd) {
    //        console.write_debug("%s \n", snd->wav.soundName.GetResRefNulled());
    //    }
    //}

    //console.write_debug("Waiting=%d \n", g_pChitin->m_mixer.Waiting.GetCount());
    //CObList& que3 = g_pChitin->m_mixer.Waiting;
    //pos = que3.GetHeadPosition();
    //while (pos != NULL) {
    //    CSound *snd = (CSound *) que3.GetNext(pos);
    //    if (snd) {
    //        console.write_debug("%s \n", snd->wav.soundName.GetResRefNulled());
    //    }
    //}

    g_pChitin->m_mixer.UpdateSoundList(); // Unmute all queues
}


void static __stdcall
CScreenWorld_Pause(int Mode) {
    POSITION pos;
    ResRef PostCastingSnd = "";
    ResRef GUI1 = "PAPER";
    ResRef GUI2 = "GAM_09";

    //console.write_debug("Pause=%d PlayingNow=%d \n", Mode, g_pChitin->m_mixer.PlayingNow.GetCount());

    if (Mode == 2) {    // get playing pos-casting sound
        Enum             Cre_id = g_pChitin->pGame->m_eTimeStopExempt;  // timestop creator
        CCreatureObject* Cre;
        BOOL             result = FALSE;
    
        if (g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(Cre_id, THREAD_ASYNCH, &Cre, -1) == OBJECT_SUCCESS) {
            if (Cre->nObjType == CGAMEOBJECT_TYPE_CREATURE) {
                //Cre->sndCasting.Stop(); // remove post-casting sound from PlayingNow to avoid mess with resident CSound
                if (Cre->sndCasting.wav.pResWav)
                    PostCastingSnd = Cre->sndCasting.wav.soundName;
            }

            g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(Cre_id, THREAD_ASYNCH, INFINITE);
        }
    }

    CObList& que = g_pChitin->m_mixer.PlayingNow;
    pos = que.GetHeadPosition();
    while (pos != NULL) {
        POSITION curpos = pos;
        CVoice *Voice = (CVoice *) que.GetNext(pos);
        if (Voice) {
            if (Voice->pSound && Voice->pSound->m_pSoundBuffer) { // if CSound exist
                CSound *snd = Voice->pSound;
                BOOL isPlay = snd->IsSoundPlaying(false);
                if (isPlay) {
                    // active sound
                    if (snd->bLoop) {   // Remove from PlayingNow & Stop playing
                        console.write_debug("Remove looped %s \n", snd->wav.soundName.GetResRefNulled());
                        THISCALL_1(0xA4F678, &g_pChitin->m_mixer.PlayingNow, curpos ); // CObList.RemoveAt()
                        delete Voice;
                    } else {            // Freeze sound, keep in PlayingNow
                        LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)snd->m_pSoundBuffer;
                        DWORD dwPlay, dwWrite;
                        HRESULT hr;

                        if (
                            (PostCastingSnd.IsValid() && snd->wav.soundName == PostCastingSnd) ||
                            (snd->wav.soundName == GUI1) ||
                            (snd->wav.soundName == GUI2)
                            )
                            ;   // keep playing
                        else {
                            hr = IDirectSoundBuffer_GetCurrentPosition(pDSB, &dwPlay, &dwWrite);
                            if (hr == S_OK) {
                                hr = IDirectSoundBuffer_Stop(pDSB);
                                if (hr == S_OK) {
                                    Voice->nVolume = snd->nVolume;
                                    Voice->DSBufPosition = dwPlay;
                                    if (Mode == 1) {
                                        Voice->bPauseMode = 1;
                                        console.write_debug("Freeze Pause static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                                    }
                                    else
                                    if (Mode == 2) {
                                        Voice->bTimeStopMode = 1;
                                        console.write_debug("Freeze Timestop static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                                    }
                                    else
                                    if (Mode == 3) {
                                        Voice->bScreenMode = 1;
                                        console.write_debug("Freeze Screen static %s \t pos=%d \n", snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // already muted sound
                    if (Mode == 1) {
                        Voice->bPauseMode = 1;
                        //console.write_debug("-> Pause static %s pos=%d \n",  snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                    } else
                    if (Mode == 2) {
                        Voice->bTimeStopMode = 1;
                        //console.write_debug("-> Timestop static %s pos=%d \n",  snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                    } else
                    if (Mode == 3) {
                        Voice->bScreenMode = 1;
                        //console.write_debug("-> Screen static %s pos=%d \n",  snd->wav.soundName.GetResRefNulled(), Voice->DSBufPosition);
                    }
                }
            } else // CVoice without CSound
            if (Voice->m_pSoundBuffer) { // transferred from CSound to CVoice
                LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)Voice->m_pSoundBuffer;
                DWORD dwPlay, dwWrite;
                HRESULT hr;

                if (Voice->bPauseMode    == 0 &&    // active sound
                    Voice->bTimeStopMode == 0 &&
                    Voice->bScreenMode   == 0) {
                    if (
                        (PostCastingSnd.IsValid() && Voice->soundName == PostCastingSnd) ||
                        (Voice->soundName == GUI1) ||
                        (Voice->soundName == GUI2)
                        )
                        ;   // keep playing
                    else {
                        hr = IDirectSoundBuffer_GetCurrentPosition(pDSB, &dwPlay, &dwWrite);
                        if (hr == S_OK) {
                            hr = IDirectSoundBuffer_Stop(pDSB);
                            if (hr == S_OK) {
                                Voice->DSBufPosition = dwPlay;
                                if (Mode == 1) {
                                    Voice->bPauseMode = 1;
                                    console.write_debug("Freeze Pause temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                                }
                                else
                                if (Mode == 2) {
                                    Voice->bTimeStopMode = 1;
                                    console.write_debug("Freeze TimeStop temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                                }
                                else
                                if (Mode == 3) {
                                    Voice->bScreenMode = 1;
                                    console.write_debug("Freeze Screen temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                                }
                            }
                        }
                    }
               } else {
                    // already muted sound
                    if (Mode == 1) {
                        Voice->bPauseMode = 1;
                        //console.write_debug("-> Pause temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                    } else
                    if (Mode == 2) {
                        Voice->bTimeStopMode = 1;
                        //console.write_debug("-> TimeStop temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                    } else
                    if (Mode == 3) {
                        Voice->bScreenMode = 1;
                        //console.write_debug("-> Screen temp %s pos=%d \n", Voice->soundName.GetResRefNulled(), Voice->DSBufPosition);
                    }
                }
            }
        }
    }

    //console.write_debug("AllLoopingQueue=%d \n",    g_pChitin->m_mixer.AllLoopingQueue.GetCount());
    //CObList& que2 = g_pChitin->m_mixer.AllLoopingQueue;
    //pos = que2.GetHeadPosition();
    //while (pos != NULL) {
    //    CSound *snd = (CSound *) que2.GetNext(pos);
    //    if (snd) {
    //        console.write_debug("%s \n", snd->wav.soundName.GetResRefNulled());
    //    }
    //}

 /*   console.write_debug("Waiting=%d \n", g_pChitin->m_mixer.Waiting.GetCount());
    CObList& que3 = g_pChitin->m_mixer.Waiting;
    pos = que3.GetHeadPosition();
    while (pos != NULL) {
        CSound *snd = (CSound *) que3.GetNext(pos);
        if (snd) {
            console.write_debug("%s \n", snd->wav.soundName.GetResRefNulled());
        }
    }*/
}


void static __stdcall
CSoundMixer_TransferBuffer(CVoice& Voice, CSound* snd) {
    if (snd) {
        Voice.soundName = snd->wav.soundName;
        Voice.pArea = snd->pArea;
    }
}


BOOL static __stdcall
UpdateSoundList_RemoveFromPlayingNow(CVoice& Voice) {
    if (Voice.bPauseMode    == 1 ||
        Voice.bTimeStopMode == 1 ||
        Voice.bScreenMode   == 1 )
        return TRUE;    // skip remove
    else {
        /*if (Voice.pSound)
            console.write_debug("RemoveFromPlayingNow %s \n", Voice.pSound->wav.soundName.GetResRefNulled());
        else
            console.write_debug("RemoveFromPlayingNow %s \n", Voice.soundName.GetResRefNulled());*/
        return FALSE;
    }
}


BOOL static __stdcall
CheckPauseAndUpdateWaitingList(CSound* sound) {
    if (g_pChitin->pScreenWorld) {
        if (g_pChitin->pScreenWorld->bPaused) {
            g_pChitin->m_mixer.AddToLoopingList(sound);
            return TRUE;    // skip play
        }
    }

    if (g_pChitin->pGame->m_nTimeStopTicksLeft > 0) {
        g_pChitin->m_mixer.AddToLoopingList(sound);
        return TRUE;    // skip play
    }

    return FALSE;
}


BOOL static __stdcall
CheckPause(CSound& sound) {
    if (g_pChitin->pScreenWorld) {
        if (g_pChitin->pScreenWorld->bPaused)
            return TRUE;    // skip play
    }

    if (g_pChitin->pGame->m_nTimeStopTicksLeft > 0)
        return TRUE;    // skip play

    return FALSE;
}


void __declspec(naked)
CScreenWorldMap_EngineGameInit_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    ecx // Manager
    call    CScreenWorldMap_EngineGameInit

    pop     eax
    pop     edx
    pop     ecx

    ret     4
}
}


//void __declspec(naked)
//CScreenWorld_Unpause_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//    push    eax
//
//    push    1
//    call    CScreenWorld_UnPause
//
//    pop     eax
//    pop     edx
//    pop     ecx
//
//    add     ecx, 1DD0h  // Stolen bytes
//    ret
//}
//}
//
//
//void __declspec(naked)
//CScreenWorld_Pause_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//    push    eax
//
//    push    1
//    call    CScreenWorld_Pause
//
//    pop     eax
//    pop     edx
//    pop     ecx
//
//    add     ecx, 1DD0h  // Stolen bytes
//    ret
//}
//}


void __declspec(naked)
CTimerWorld_HardUnPause_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    1   // Hard UnPause
    call    CScreenWorld_UnPause

    pop     eax
    pop     edx
    pop     ecx

    push    0649F8Eh  // Continue
    ret
}
}

void __declspec(naked)
CTimerWorld_StartTime_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    3   // Screen UnPause
    call    CScreenWorld_UnPause

    pop     eax
    pop     edx
    pop     ecx

    push    0649F8Eh  // Continue
    ret
}
}


void __declspec(naked)
CGameArea_OnActivation_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    3   // Screen Pause
    call    CScreenWorld_UnPause

    pop     eax
    pop     edx
    pop     ecx

    push    09E17D5h  // Continue
    ret
}
}


void __declspec(naked)
CTimerWorld_HardPause_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    1   // Hard Pause
    call    CScreenWorld_Pause

    pop     eax
    pop     edx
    pop     ecx

    push    064A068h  // Continue
    ret
}
}


void __declspec(naked)
CTimerWorld_StopTime_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    3   // Screen Pause
    call    CScreenWorld_Pause

    pop     eax
    pop     edx
    pop     ecx

    push    064A068h  // Continue
    ret
}
}


//void __declspec(naked)
//UpdateSoundList_ActivateFromAllLoopingQueue_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//
//    //push    [ebp-6Ch]   // Node
//    push    [ebp-8]     // CSound
//    call    CheckPause
//    test    eax, eax
//
//    pop     edx
//    pop     ecx
//    jnz     UpdateSoundList_ActivateFromAllLoopingQueue_asm_Skip
//
//    push    09DEAB9h  // CSound::ResetVolume
//    ret
//
//UpdateSoundList_ActivateFromAllLoopingQueue_asm_Skip:
//    ret
//}
//}


void __declspec(naked)
CSound_PlayWaiting_asm() {
__asm
{
    push    ecx
    push    edx

    push    ecx         // CSound
    call    CheckPauseAndUpdateWaitingList
    test    eax, eax

    pop     edx
    pop     ecx
    jnz     CSound_PlayWaiting_asm_Skip

    push    09DEA7Ah  // CSound::PlayWaiting
    ret

CSound_PlayWaiting_asm_Skip:
    ret
}
}


void __declspec(naked)
SoundPlay1CheckPause_asm() {
__asm
{
    push    ecx
    push    edx

    push    ecx // CSound
    call    CheckPause
    test    eax, eax

    pop     edx
    pop     ecx
    jnz     SoundPlay1CheckPause_asm_Skip

    push    09DE123h  // CSound::Play(1)
    ret

SoundPlay1CheckPause_asm_Skip:
    ret     4
}
}


//void __declspec(naked)
//SetVolumeCheckPause_asm() {
//__asm
//{
//    push    edx
//    push    ecx
//
//    push    ecx // CSound
//    call    CheckPause
//    test    eax, eax
//
//    jnz     SetVolumeCheckPause_asm_Skip
//
//    pop     ecx
//    pop     edx
//
//    push    09DF37Ah  // CSound::SetVolume
//    ret
//
//SetVolumeCheckPause_asm_Skip:
//    pop     ecx
//    mov     edx, [esp+8]    // arg0
//    mov     [ecx+069h], dl  // save volume
//
//    pop     edx
//    ret     4
//}
//}

void __declspec(naked)
CSoundMixer_UpdateSoundList_RemoveFromPlayingNow_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp-0Ch] // CVoice
    call    UpdateSoundList_RemoveFromPlayingNow
    test    eax, eax
    pop     ecx
    pop     edx

    jnz     CSoundMixer_UpdateSoundList_RemoveFromPlayingNow_asm_Skip

    mov     ecx, [ebp-90h]  // Stolen bytes
    ret

CSoundMixer_UpdateSoundList_RemoveFromPlayingNow_asm_Skip:

    add     esp,4
    push    09E18A0h  // continue
    ret
}
}


void __declspec(naked)
CSoundMixer_UpdateSoundList_MismatchAreaRemoveFromPlayingNow_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp-0Ch] // CVoice
    call    UpdateSoundList_RemoveFromPlayingNow
    test    eax, eax
    pop     ecx
    pop     edx

    jnz     CSoundMixer_UpdateSoundList_MismatchAreaRemoveFromPlayingNow_asm_Skip

    mov     ecx, [ebp-90h]  // Stolen bytes
    ret

CSoundMixer_UpdateSoundList_MismatchAreaRemoveFromPlayingNow_asm_Skip:

    add     esp,4
    push    09E1986h  // continue
    ret
}
}


void __declspec(naked)
CSoundMixer_UpdateSoundListPriority_RemoveFromPlayingNow_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp-0Ch] // CVoice
    call    UpdateSoundList_RemoveFromPlayingNow
    test    eax, eax
    pop     ecx
    pop     edx

    jnz     CSoundMixer_UpdateSoundListPriority_RemoveFromPlayingNow_asm_Skip

    mov     ecx, [ebp-5Ch]  // Stolen bytes
    add     ecx, 0A4h
    ret

CSoundMixer_UpdateSoundListPriority_RemoveFromPlayingNow_asm_Skip:

    add     esp,4
    push    09E1C5Ch  // continue
    ret
}
}


void __declspec(naked)
CSoundMixer_UpdateSoundListPriority_MismatchAreaRemoveFromPlayingNow_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp-0Ch] // CVoice
    call    UpdateSoundList_RemoveFromPlayingNow
    test    eax, eax
    pop     ecx
    pop     edx

    jnz     CSoundMixer_UpdateSoundListPriority_MismatchAreaRemoveFromPlayingNow_asm_Skip

    mov     ecx, [ebp-5Ch]  // Stolen bytes
    add     ecx, 0A4h
    ret

CSoundMixer_UpdateSoundListPriority_MismatchAreaRemoveFromPlayingNow_asm_Skip:

    add     esp,4
    push    09E1D30h  // continue
    ret
}
}


void __declspec(naked)
CSoundMixer_TransferBuffer_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp+8] // CSound
    push    [ebp-4] // CVoice
    call    CSoundMixer_TransferBuffer

    pop     ecx
    pop     edx

    mov     eax, [ebp+8]  // Stolen bytes
    mov     ecx, [eax+58h]
    ret
}
}


void __declspec(naked)
CMessage_TimeStop_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    2   //  TimeStop 
    call    CScreenWorld_Pause

    pop     eax
    pop     edx
    pop     ecx

    add     edx, 200h  // Stolen bytes
    ret
}
}


void __declspec(naked)
TimeStopEnded_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    2   //  TimeStop 
    call    CScreenWorld_UnPause

    pop     eax
    pop     edx
    pop     ecx

    mov     edx, [ebp-180h]  // Stolen bytes
    ret
}
}


void __declspec(naked)
CVoice_CVoice_asm() {
__asm
{
    ; ecx - CVoice
    mov     byte  ptr [ecx]CVoice.bPauseMode, 0
    mov     byte  ptr [ecx]CVoice.bTimeStopMode, 0
    mov     byte  ptr [ecx]CVoice.bScreenMode, 0
    mov     byte  ptr [ecx]CVoice.pad1, 0
    mov     byte  ptr [ecx]CVoice.pad2, 0
    mov     byte  ptr [ecx]CVoice.pad3, 0
    mov     dword ptr [ecx]CVoice.DSBufPosition, 0
    mov     dword ptr [ecx]CVoice.nVolume, 0
    mov     dword ptr [ecx]CVoice.soundName, 0
    mov     dword ptr [ecx]CVoice.soundName+4, 0
    mov     dword ptr [ecx]CVoice.pArea, 0

    mov     dword ptr [ecx+4], 0  // Stolen bytes
    ret
}
}