#include "uiscroll.h"
#include "chitin.h"

//CUIScrollBar& (CUIScrollBar::*CUIScrollBar_Construct_2CPanel_ChuFileControlInfoBase)(CPanel&, ChuFileControlInfoBase&) =
//    SetFP(static_cast<CUIScrollBar& (CUIScrollBar::*)(CPanel&, ChuFileControlInfoBase&)>
//                                                                        (&CUIScrollBar::Construct),             0x5A1940);
//void (CUIScrollBar::*CUIScrollBar_Deconstruct)() =
//    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::Deconstruct),           0x5A1F51);
void (CUIScrollBar::*CUIScrollBar_UpdateKnobPosition)(short, short, short) =
    SetFP(static_cast<void (CUIScrollBar::*)(short, short, short)>      (&CUIScrollBar::UpdateKnobPosition),    0x5A202F);
void (CUIScrollBar::*CUIScrollBar_OnLoseFocus)() =
    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::OnLoseFocus),           0x5A2476);
void (CUIScrollBar::*CUIScrollBar_OnLMouseDrag)(POINT) =
    SetFP(static_cast<void (CUIScrollBar::*)(POINT)>                    (&CUIScrollBar::OnLMouseDrag),          0x5A2481);
BOOL (CUIScrollBar::*CUIScrollBar_OnLMouseBtDn)(POINT) =
    SetFP(static_cast<BOOL (CUIScrollBar::*)(POINT)>                    (&CUIScrollBar::OnLMouseBtDn),          0x5A21D8);
void (CUIScrollBar::*CUIScrollBar_OnLMouseBtUp)(POINT) =
    SetFP(static_cast<void (CUIScrollBar::*)(POINT)>                    (&CUIScrollBar::OnLMouseBtUp),          0x5A2372);
BOOL (CUIScrollBar::*CUIScrollBar_OnLMouseDblClk)(POINT) =
    SetFP(static_cast<BOOL (CUIScrollBar::*)(POINT)>                    (&CUIScrollBar::OnLMouseDblClk),        0x5A2691);
void (CUIScrollBar::*CUIScrollBar_ShowTooltip)(bool) =
    SetFP(static_cast<void (CUIScrollBar::*)(bool)>                     (&CUIScrollBar::ShowTooltip),           0x5A33E9);
void (CUIScrollBar::*CUIScrollBar_SetRedraw)() =
    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::SetRedraw),             0x5A2111);
BOOL (CUIScrollBar::*CUIScrollBar_Redraw)(BOOL) =
    SetFP(static_cast<BOOL (CUIScrollBar::*)(BOOL)>                     (&CUIScrollBar::Redraw),                0x5A2893);
BOOL (CUIScrollBar::*CUIScrollBar_NeedRedraw)() =
    SetFP(static_cast<BOOL (CUIScrollBar::*)()>                         (&CUIScrollBar::NeedRedraw),           0x5A2829);
void (CUIScrollBar::*CUIScrollBar_OnLClicked)(POINT) =
    SetFP(static_cast<void (CUIScrollBar::*)(POINT)>                    (&CUIScrollBar::OnLClicked),            0x5A2622);
void (CUIScrollBar::*CUIScrollBar_OnMouseDragKnob)() =
    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::OnMouseDragKnob),       0x5A2771);
void (CUIScrollBar::*CUIScrollBar_OnLMouseBtnDnArrowUp)() =
    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::OnLMouseBtnDnArrowUp),  0x5A2717);
void (CUIScrollBar::*CUIScrollBar_OnLMouseBtnDnArrowDn)() =
    SetFP(static_cast<void (CUIScrollBar::*)()>                         (&CUIScrollBar::OnLMouseBtnDnArrowDn),  0x5A2744);
void (CUIScrollBar::*CUIScrollBar_OnLMouseBtnUpArrow)(char) =
    SetFP(static_cast<void (CUIScrollBar::*)(char)>                     (&CUIScrollBar::OnLMouseBtnUpArrow),    0x5A27B4);
void (CUIScrollBar::*CUIScrollBar_OnLClickedAboveKnob)(short) =
    SetFP(static_cast<void (CUIScrollBar::*)(short)>                    (&CUIScrollBar::OnLClickedAboveKnob),   0x5A26B1);
void (CUIScrollBar::*CUIScrollBar_OnLClickedBelowKnob)(short) =
    SetFP(static_cast<void (CUIScrollBar::*)(short)>                    (&CUIScrollBar::OnLClickedBelowKnob),   0x5A26E4);

//CUIScrollBar::CUIScrollBar(CPanel& panel, ChuFileControlInfoBase& controlInfo)
//                                                              { (this->*CUIScrollBar_Construct_2CPanel_ChuFileControlInfoBase)(panel, controlInfo); }
//CUIScrollBar::~CUIScrollBar()                                 { (this->*CUIScrollBar_Deconstruct)(); }
void CUIScrollBar::UpdateKnobPosition(short wValueCurrent, short wValues, short wRows)
                                                                { return (this->*CUIScrollBar_UpdateKnobPosition)(wValueCurrent, wValues, wRows); }
void CUIScrollBar::OnLoseFocus()                                { return (this->*CUIScrollBar_OnLoseFocus)(); }
void CUIScrollBar::OnLMouseDrag(POINT pt)                       { return (this->*CUIScrollBar_OnLMouseDrag)(pt); }
BOOL CUIScrollBar::OnLMouseBtDn(POINT pt)                       { return (this->*CUIScrollBar_OnLMouseBtDn)(pt); }
void CUIScrollBar::OnLMouseBtUp(POINT pt)                       { return (this->*CUIScrollBar_OnLMouseBtUp)(pt); }
BOOL CUIScrollBar::OnLMouseDblClk(POINT pt)                     { return (this->*CUIScrollBar_OnLMouseDblClk)(pt); }
void CUIScrollBar::ShowTooltip(bool b)                          { return (this->*CUIScrollBar_ShowTooltip)(b); }
void CUIScrollBar::SetRedraw()                                  { return (this->*CUIScrollBar_SetRedraw)(); }
BOOL CUIScrollBar::Redraw(BOOL bForceRedraw)                    { return (this->*CUIScrollBar_Redraw)(bForceRedraw); }
BOOL CUIScrollBar::NeedRedraw()                                 { return (this->*CUIScrollBar_NeedRedraw)(); }
void CUIScrollBar::OnLClicked(POINT pt)                         { return (this->*CUIScrollBar_OnLClicked)(pt); }
void CUIScrollBar::OnMouseDragKnob()                            { return (this->*CUIScrollBar_OnMouseDragKnob)(); }
void CUIScrollBar::OnLMouseBtnDnArrowUp()                       { return (this->*CUIScrollBar_OnLMouseBtnDnArrowUp)(); }
void CUIScrollBar::OnLMouseBtnDnArrowDn()                       { return (this->*CUIScrollBar_OnLMouseBtnDnArrowDn)(); }
void CUIScrollBar::OnLMouseBtnUpArrow(char b)                   { return (this->*CUIScrollBar_OnLMouseBtnUpArrow)(b); }
void CUIScrollBar::OnLClickedAboveKnob(short interval)          { return (this->*CUIScrollBar_OnLClickedAboveKnob)(interval); }
void CUIScrollBar::OnLClickedBelowKnob(short interval)          { return (this->*CUIScrollBar_OnLClickedBelowKnob)(interval); }


void static __stdcall
CChitin_AsynchronousUpdate_SwitchScrollBar() {
    POINT MousePt;

    GetCursorPos(&MousePt);
    if ( g_pChitin->bFullScreen || PtInRect(&g_pChitin->MainWindowRect, MousePt) ) {
        ScreenToClient(g_pChitin->m_CWnd.m_hWnd, &MousePt);

        // Store
        if (g_pChitin->pEngineActive == g_pChitin->pScreenStore) {
            CPanel& panelMain =     g_pChitin->pScreenStore->manager.GetPanel(2);   // Buy/Sell panel
            CPanel& panelIdentify = g_pChitin->pScreenStore->manager.GetPanel(4);   // Identify panel
            CPanel& panelHeal =     g_pChitin->pScreenStore->manager.GetPanel(5);   // Heal panel
            CPanel& panelSteal =    g_pChitin->pScreenStore->manager.GetPanel(6);   // Steal panel
            CPanel& panelDrink =    g_pChitin->pScreenStore->manager.GetPanel(8);   // Drink panel

            // Buy/Sell panel
            if (g_pChitin->pScreenStore->m_pMainPanel == &panelMain &&
                g_pChitin->pScreenStore->lPopupStack.GetCount() == 0) {
                CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;

                RECT BuyScroll  = { 24,  129, 245+16, 129+182};
                OffsetRect(&BuyScroll, panelMain.pt2.x, panelMain.pt2.y);

                RECT SellScroll = { 269, 129, 490+16, 129+182};
                OffsetRect(&SellScroll, panelMain.pt2.x, panelMain.pt2.y);

                if (PtInRect(&BuyScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelMain.GetUIControl(11);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }

                if (PtInRect(&SellScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelMain.GetUIControl(12);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }

            // Identify panel
            if (g_pChitin->pScreenStore->m_pMainPanel == &panelIdentify &&
                g_pChitin->pScreenStore->lPopupStack.GetCount() == 0) {
                CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;

                RECT LeftScroll  = { 24,  129, 245+16, 129+182};
                OffsetRect(&LeftScroll, panelIdentify.pt2.x, panelIdentify.pt2.y);

                RECT RightScroll = { 284, 120, 469+16, 120+240};
                OffsetRect(&RightScroll, panelIdentify.pt2.x, panelIdentify.pt2.y);

                if (PtInRect(&LeftScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelIdentify.GetUIControl(7);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }

                if (PtInRect(&RightScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelIdentify.GetUIControl(24);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }

            // Heal panel
            if (g_pChitin->pScreenStore->m_pMainPanel == &panelHeal &&
                g_pChitin->pScreenStore->lPopupStack.GetCount() == 0) {
                CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;

                RECT LeftScroll  = { 24,  129, 245+16, 129+182};
                OffsetRect(&LeftScroll, panelHeal.pt2.x, panelHeal.pt2.y);

                RECT RightScroll = { 284, 120, 469+16, 120+240};
                OffsetRect(&RightScroll, panelHeal.pt2.x, panelHeal.pt2.y);

                if (PtInRect(&LeftScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelHeal.GetUIControl(7);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }

                if (PtInRect(&RightScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelHeal.GetUIControl(24);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }

            // Steal panel
            if (g_pChitin->pScreenStore->m_pMainPanel == &panelSteal &&
                g_pChitin->pScreenStore->lPopupStack.GetCount() == 0) {
                CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;

                RECT LeftScroll  = { 24,  129, 245+16, 129+182};
                OffsetRect(&LeftScroll, panelSteal.pt2.x, panelSteal.pt2.y);

                RECT RightScroll = { 269, 128, 490+16, 128+182};
                OffsetRect(&RightScroll, panelSteal.pt2.x, panelSteal.pt2.y);

                if (PtInRect(&LeftScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelSteal.GetUIControl(9);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }

                if (PtInRect(&RightScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelSteal.GetUIControl(10);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }

            // Drink panel
            if (g_pChitin->pScreenStore->m_pMainPanel == &panelDrink &&
                g_pChitin->pScreenStore->lPopupStack.GetCount() == 0) {
                CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;

                RECT LeftScroll  = { 25,  130, 241+16, 131+252};
                OffsetRect(&LeftScroll, panelDrink.pt2.x, panelDrink.pt2.y);

                RECT RightScroll = { 290, 202, 469+16, 202+156};
                OffsetRect(&RightScroll, panelDrink.pt2.x, panelDrink.pt2.y);

                if (PtInRect(&LeftScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelDrink.GetUIControl(5);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }

                if (PtInRect(&RightScroll, MousePt)) {
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelDrink.GetUIControl(14);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }


            // Item description panel
            if (g_pChitin->pScreenStore->lPopupStack.GetCount() > 0) { // child panel
                CPanel& panelDesc = g_pChitin->pScreenStore->manager.GetPanel(12);
                CPanel *ActivePanel = (CPanel *) g_pChitin->pScreenStore->lPopupStack.GetTail();

                if (ActivePanel == &panelDesc) {
                    CUIScrollBar** ppScrollActive = & g_pChitin->pScreenStore->pScrollActive;
                    CUIScrollBar& Scrollbar = (CUIScrollBar &) panelDesc.GetUIControl(6);
                    if (*ppScrollActive != &Scrollbar) {
                        *ppScrollActive = &Scrollbar;
                    }
                }
            }
        }

        // Custom Portraits Panel
        if (g_pChitin->pEngineActive == g_pChitin->pScreenCharacter) {
            CPanel& panel = g_pChitin->pScreenCharacter->manager.GetPanel(19);  // Custom Portraits
            if (g_pChitin->pScreenCharacter->cplOpenPanels.GetCount()) {
                if ((CPanel *) g_pChitin->pScreenCharacter->cplOpenPanels.GetTail() == &panel) {
                    CUIScrollBar** ppScrollActive = & g_pChitin->pScreenCharacter->pScrollActive;

                    RECT BigScroll   = { 171, 139, 470+16, 139+145};
                    OffsetRect(&BigScroll, panel.pt2.x, panel.pt2.y);

                    RECT SmallScroll = { 26,  330, 424+16, 390    };
                    OffsetRect(&SmallScroll, panel.pt2.x, panel.pt2.y);

                    if (PtInRect(&BigScroll, MousePt)) {
                        CUIScrollBar& Scrollbar = (CUIScrollBar &) panel.GetUIControl(4);
                        if (*ppScrollActive != &Scrollbar)
                            *ppScrollActive = &Scrollbar;
                    }

                    if (PtInRect(&SmallScroll, MousePt)) {
                        CUIScrollBar& Scrollbar = (CUIScrollBar &) panel.GetUIControl(5);
                        if (*ppScrollActive != &Scrollbar)
                            *ppScrollActive = &Scrollbar;
                    }
                }
            }
        }

        // LevelUp
        if (g_pChitin->pEngineActive == g_pChitin->pScreenCharacter) {
            CPanel& panel = g_pChitin->pScreenCharacter->manager.GetPanel(3);   // levelup panel

            if (g_pChitin->pScreenCharacter->cplOpenPanels.GetCount()) {
                if ((CPanel *) g_pChitin->pScreenCharacter->cplOpenPanels.GetTail() == &panel) {
                    CUIScrollBar** ppScrollActive = & g_pChitin->pScreenCharacter->pScrollActive;

                    RECT WeaponScroll = { 23,  163, 329+19, 167+262};
                    OffsetRect(&WeaponScroll, panel.pt2.x, panel.pt2.y);

                    RECT ThiefScroll =  { 361, 165, 603+16, 165+126};
                    OffsetRect(&ThiefScroll, panel.pt2.x, panel.pt2.y);

                    RECT DescScroll =   { 356, 306, 603+16, 306+120};
                    OffsetRect(&DescScroll, panel.pt2.x, panel.pt2.y);

                    if (PtInRect(&WeaponScroll, MousePt)) {
                        CUIScrollBar& Scrollbar = (CUIScrollBar &) panel.GetUIControl(108);
                        if (*ppScrollActive != &Scrollbar)
                            *ppScrollActive = &Scrollbar;
                    }

                    if (PtInRect(&ThiefScroll, MousePt)) {
                        CUIScrollBar& Scrollbar = (CUIScrollBar &) panel.GetUIControl(109);
                        if (*ppScrollActive != &Scrollbar)
                            *ppScrollActive = &Scrollbar;
                    }

                    if (PtInRect(&DescScroll, MousePt)) {
                        CUIScrollBar& Scrollbar = (CUIScrollBar &) panel.GetUIControl(111);
                        if (*ppScrollActive != &Scrollbar)
                            *ppScrollActive = &Scrollbar;
                    }
                }
            }
        }
    }
}


void __declspec(naked)
CChitin_AsynchronousUpdate_SwitchScrollBar_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    call    CChitin_AsynchronousUpdate_SwitchScrollBar

    pop     edx
    pop     ecx
    pop     eax

    call    dword ptr [eax+98h]
    ret
}
}