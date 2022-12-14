#include "UserCore.h"

#include <cassert>

#include "stdafx.h"
#include "options.h"
#include "UserChargenKit.h"
#include "UserChargenMageSpell.h"
#include "UserRecMageSpell.h"
#include "UserMageBook.h"
#include "EngineMageBook.h"
#include "UserPriestBook.h"
#include "GreyBackground.h"
#include "uicore.h"
#include "uitext.h"
#include "console.h"
#include "NightMare.h"
#include "Shaman.h"

CUIControl* (__cdecl *Tramp_CreateUIControl)(CPanel&, ChuFileControlInfoBase&) =
    reinterpret_cast<CUIControl* (__cdecl *)(CPanel&, ChuFileControlInfoBase&)> (0x95D5E0);

CUIControl* __cdecl DETOUR_CreateUIControl(CPanel& panel, ChuFileControlInfoBase& controlInfo) {
    CUIControl* pControl = Tramp_CreateUIControl(panel, controlInfo);

    if (pControl == NULL) {
        assert(&controlInfo != NULL);
        ResRef rManager = panel.pManager->name;

        //Chargen
        if (rManager == "GUICG") {
            switch (panel.index) {
            case 0:         // main panel
                switch (controlInfo.id) {
                case 50:    // Nightmare Button
                    if (pGameOptionsEx->bUI_NightmareMode) {
                        pControl = new CUIButtonStartNightmare(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 2:         // class select
                switch (controlInfo.id) {
                case 18:    // Shaman class Button
                    if (pGameOptionsEx->bEngineShamanClass) {
                        pControl = IENew CUIButtonShamanClass(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 7:
                switch (controlInfo.id) {
                case 31:
                    if (pGameOptionsEx->bUserChargenMageSpellScroll) {
                        pControl = new CUIScrollBarChargenMageSpell(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 22:
                switch (controlInfo.id) {
                case 15:
                    if (pGameOptionsEx->bUserChargenKitSelectScroll) {
                        pControl = new CUIScrollBarChargenKit(panel, controlInfo);
                        return pControl;
                    }
                    break;
                case 16:
                    if (pGameOptionsEx->bUserChargenKitSelectScroll) {
                        pControl = IENew CUICheckButtonChargenKit(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        //MageBook
        if (rManager == "GUIMG") {
            switch (panel.index) {
            case 2:
                switch (controlInfo.id) {
                case 65:
                    if (pGameOptionsEx->bUserMageBookScroll) {
                        pControl = new CUIButtonMageBookUp(panel, controlInfo);
                        return pControl;
                    }
                    break;
                case 66:
                    if (pGameOptionsEx->bUserMageBookScroll) {
                        pControl = new CUIButtonMageBookDn(panel, controlInfo);
                        return pControl;
                    }
                    break;
                case 67:    // Sequence Button
                    if (pGameOptionsEx->bUI_SequenceWindowButton) {
                        pControl = new CUIButtonSequence(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 8:
                switch (controlInfo.id) {
                case 67:    // Sequence Button
                    if (pGameOptionsEx->bUI_SequenceWindowButton) {
                        pControl = new CUIButtonSequence(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        //PriestBook
        if (rManager == "GUIPR") {
            switch (panel.index) {
            case 2:
                switch (controlInfo.id) {
                case 62:
                    if (pGameOptionsEx->bUserPriestBookScroll) {
                        pControl = new CUIButtonPriestBookUp(panel, controlInfo);
                        return pControl;
                    }
                    break;
                case 63:
                    if (pGameOptionsEx->bUserPriestBookScroll) {
                        pControl = new CUIButtonPriestBookDn(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        //Record
        if (rManager == "GUIREC") {
            switch (panel.index) {
            case 8:
                switch (controlInfo.id) {
                case 24:
                    if (pGameOptionsEx->bUserRecordMageSpellScroll) {
                        pControl = IENew CUICheckButtonRecMageSpell(panel, controlInfo);
                        return pControl;
                    }
                    break;
                case 30:
                    if (pGameOptionsEx->bUserRecordMageSpellScroll) {
                        pControl = new CUIScrollBarRecMageSpell(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        if (rManager == "GUIW" ||
            rManager == "GUIW08" ||
            rManager == "GUIW10" ||
            rManager == "GUIW12" ||
            rManager == "GUIW16" ||
            rManager == "GUIW20") {
            switch (panel.index) {
            case 1:
                switch (controlInfo.id) {
                case 9: // GreyBackground Switch
                    if (pGameOptionsEx->bUI_GreyBackgroundOnPause) {
                        pControl = new CUIButtonGreyBackground(panel, controlInfo);
                        return pControl;
                    }
                    break;
                default:
                    break;
                }
                break;
            }
        }

    } else {
        assert(&controlInfo != NULL);
        ResRef rManager = panel.pManager->name;

        //World
        if (rManager == "GUIW" ||
            rManager == "GUIW08" ||
            rManager == "GUIW10" ||
            rManager == "GUIW12" ||
            rManager == "GUIW16" ||
            rManager == "GUIW20") {
            switch (panel.index) {
            case 4: //small log bar
                switch (controlInfo.id) {
                case 3:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 7: //large log bar
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 12: //medium log bar
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 13: //dialog3
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 14: //dialog2
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 18: //large log bar (MP)/dialogue panel (MP)
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 22: //medium log bar (MP)
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 29: //dialog panel
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 30: //dialog3 (MP)
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 31: //dialog2 (MP)
                switch (controlInfo.id) {
                case 1:
                    if (pGameOptionsEx->nUserCustomDlgBarSize) {
                        ((CUITextArea*)pControl)->wRowsMax = pGameOptionsEx->nUserCustomDlgBarSize;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

    }



    return pControl;
}