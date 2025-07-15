#include "vidcore.h"
#include "infcursor.h"
#include "stdafx.h"

const ABGR* g_pColorRangeArray = (const ABGR*)0xAB9B78;
const ABGR  g_ColorDefaultText = 0xBED7D7;

void (CVidPalette::*CVidPalette_SetFxPaletteNo3d)(ABGR*, int, unsigned int, int, BOOL) =
	SetFP(static_cast<void (CVidPalette::*)(ABGR*, int, unsigned int, int, BOOL)>	(&CVidPalette::SetFxPaletteNo3d),	0x9F5897);

void CVidPalette::SetFxPaletteNo3d(ABGR* pPalette, int nBitsPerPixel, unsigned int dwFlags, int dwAlpha, BOOL bIgnoreBrighten) {
	(this->*CVidPalette_SetFxPaletteNo3d)(pPalette, nBitsPerPixel, dwFlags, dwAlpha, bIgnoreBrighten);
}
_n void CVidBitmap::RenderDirect(int, int, int, RECT *, int, BOOL)     { _bgmain(0x9D4EB8) }
_n void CVidImage::SetTintColor(ABGR)                                  { _bgmain(0x9C9D41) }
_n void CVidCell::SequenceSet(unsigned short nSequence)                { _bgmain(0x9CC538) }
_n void CVidCell::FrameSet(unsigned short)                             { _bgmain(0x9CBEDA) }
_n void CVidCell::GetCurrentFrameSize(SIZE *Size, BOOL bReleaseRes)    { _bgmain(0x9CAFB3) }
_n void CVidCell::GetCurrentCenterPoint(POINT *p, BOOL bReleaseRes)    { _bgmain(0x9CA92A) }
_n void CVidCell::Render(int, int, int y, RECT *, CVidPoly *,
        int, unsigned int, int)                                        { _bgmain(0x9CD125) };

_n void CMarker::Render(CVideoMode& pVidMode, uint ArgX, DWORD *pInfinity, POINT *ptCenter, int nXSize, int nYSize) { _bgmain(0x95CE1C) };

_n void CInfCursor::SetCursor(int nNewCursor, bool bForce, int nPointerNumber) { _bgmain(0x672B29) };
_n void CInfTooltip::SetTextRef(STRREF *textRef, IECString *sExtraText)  { _bgmain(0x6748AA) };
_n void CVidPalette::SetRange(ushort nRange, ushort nValue, CVidBitmap& bmpMasterPalette) { _bgmain(0x9F42C2) };

_n int CVideoMode::OutlinePoly(POINT *pPoly, short nVertices, RECT &rSurface, unsigned int rgbColor, POINT &ptOffset) { _bgmain(0x9B31EE) };
_n int CVideoMode::FillPoly3d( POINT *pPoly, short nVertices, RECT &rSurface, unsigned int rgbColor, POINT &ptOffset) { _bgmain(0x9EFAA3) };
_n int CVideoMode::DrawLine3d(int nXFrom, int nYFrom, int nXTo, int nYTo, const RECT &rSurface, unsigned int rgbColor){ _bgmain(0x9EE7EA) };

//CVidCell::~CVidCell() {
//    if (this->bam.pBam)
//        this->bam.pBam->Release();
//}

