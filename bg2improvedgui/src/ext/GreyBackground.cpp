#include "GreyBackground.h"

static bool gLastPauseState = false;
bool g_GreyButtonOn = false;

bool IsGreyBackground()
{
    //g_pChitin->pGame->m_WorldTimer.bRun;
    bool State = g_pChitin->pScreenWorld->bPaused;
    if (gLastPauseState != State) {
        gLastPauseState = State;
        
        // trigger to redraw screen, see TimeStopRun 0x05E36FC 
        if (g_GreyButtonOn) {
            EnterCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);
            g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->m_cInfinity.bRefreshVRamRect = 1;
            LeaveCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);
        }
    }

    return (g_GreyButtonOn & State);
}


CUIButtonGreyBackground::CUIButtonGreyBackground(
                                    CPanel& panel,
                                    ChuFileControlInfoBase& controlInfo)
                                    : CUIButton(panel, controlInfo, 1, TRUE)
{
    CUIButton::SetTooltip(47624);
    //if (pGameOptionsEx->bUI_GreyBackgroundEnabled) {
    //    g_GreyButtonOn = true;
    //}
}


CUIButtonGreyBackground::~CUIButtonGreyBackground()
{
}


BOOL CUIButtonGreyBackground::Redraw(BOOL bForceRedraw)
{
    if (g_GreyButtonOn) {
        wFrameUp=1;
        wFrameDn=1;
    } else {
        wFrameUp=0;
        wFrameDn=0;
    }
    vc.nCurrentFrame = wFrameUp;

    return CUIButton::Redraw(bForceRedraw);
}

void CUIButtonGreyBackground::OnLClicked(POINT pt) {
    g_GreyButtonOn = !g_GreyButtonOn;

    if (g_GreyButtonOn) {
        wFrameUp=1;
        wFrameDn=1;
    } else {
        wFrameUp=0;
        wFrameDn=0;
    }

    // trigger to redraw screen
    EnterCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);
    g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->m_cInfinity.bRefreshVRamRect = 1;
    LeaveCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);

    vc.nCurrentFrame = wFrameUp;
    SetRedraw();
}

extern void PlayToggleSound(bool On_Off);

void ToggleGreyBackgroundOnPause() {
    g_GreyButtonOn = !g_GreyButtonOn;

    if (g_GreyButtonOn)
        PlayToggleSound(true);
    else
        PlayToggleSound(false);
    // trigger to redraw screen
    EnterCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);
    g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->m_cInfinity.bRefreshVRamRect = 1;
    LeaveCriticalSection(& g_pChitin->pGame->m_pLoadedAreas[g_pChitin->pGame->m_VisibleAreaIdx]->u200);
}


void __declspec(naked)
GreyBackground_asm() {
__asm
{
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    call    IsGreyBackground

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    test    al,al
    jz      return_

    mov     edx, [ebp-58h]
    or      edx, 80000h        ; grey effect
    mov     [ebp-58h], edx
    

return_:
    mov     [ebp-0d8h], ecx    ; Stolen bytes

    ret
}
}
