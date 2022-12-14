#ifndef INFCURSOR_H
#define INFCURSOR_H

#include "vidcore.h"
#include "uicore.h"
#include "sndcore.h"

#define CURSOR_ARROW			0x64
#define CURSOR_TOOLTIP_SCROLL	0x65

class CInfTooltip : public CVidCell { //Size 656h
//Constructor: 0x67384D
	//AAD08C
public:
	virtual BOOL IncrementFrame() { return FALSE; } //v0
    void SetTextRef(STRREF *textRef, IECString *sExtraText);
    void SetTextRef_(IECString sExtraText, unsigned int *textRef);

	//CVidCell m_Scroll; //0h, TOOLSCRL
	CVidFont m_Font; //d6h, TOOLFONT
	short wFrameLeft; //5d2h
	short wFrameRight; //5d4h
	short wCurrentWidth; //5d6h, gets no larger than L/2+R/2+wTextWidth
	short wCurrentWidth2; //5d8h

	//0: expanding
	//1: contracting
	//2: stationary after expand
	//3: stationary after contract
	char nMode; //5dah

	char u5db; //pad
	short wScrollWidth; //5dch
	short wTextWidth; //5deh, this is the graphical text width in pixels
	short wNumFramesLeft; //5e0h
	short wNumFramesRight; //5e2h
	IECString sTexts[2]; //5e4h, one string for each row
	CSound m_Sound; //5ech, PAPER
};

struct CInfCursor { //Size 8F6h
//Constructor: 0x672310
    void SetCursor(int nNewCursor, bool bForce, int nPointerNumber);
	BOOL bVisible;
	CVidCell vcCursors;    //4h   CURSORS
	CVidCell vcArrow;      //dah  CURSARW
	CVidCell vcCustom;     //1b0h Custom
	CInfTooltip vcToolTip; //286h
	int nAnimationCounter; //8dch
	int nAnimationSpeed;   //8e0h

	//others: CURSORS.BAM sequence
	//0x64: Cursor arrow
	//0x65: Tooltip scroll
	//0x66: u1b0
	int nCurrentCursorIdx; //8e4h
	int nCursorArrowDirection; //8e8h, 0-7 (N-NE-E-SE-S-SW-W-SW)
	BOOL bLeftMouseDown; //8ech, TRUE: will set to cursor index + 1
	CUIControl* pCtrlTarget; //8f0h
	uchar bAnimatingCustom;   //8f4h
	char u8f5; //pad
};

#endif //INFCURSOR_H