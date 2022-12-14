#include "EngineWorld.h"

#include "chitin.h"

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

