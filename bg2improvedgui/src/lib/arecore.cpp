#include "arecore.h"

char (CArea::*CArea_GetSong)(short) =
	SetFP(static_cast<char (CArea::*)(short)>	(&CArea::GetSong),		0x4D40D6);
extern BOOL (CArea::*CArea_CheckPointsAccessible)(POINT&, POINT&, TerrainTable&, BOOL, int) =
	SetFP(static_cast<BOOL (CArea::*)(POINT&, POINT&, TerrainTable&, BOOL, int)>	(&CArea::CheckPointsAccessible),	0x4B8F21);

char CArea::GetSong(short wType) { return (this->*CArea_GetSong)(wType); }
BOOL CArea::CheckPointsAccessible(POINT& pt1, POINT& pt2, TerrainTable& tt, BOOL bCheckVisibility, int nRadius) { return (this->*CArea_CheckPointsAccessible)(pt1, pt2, tt, bCheckVisibility, nRadius); }
_n uchar CInfinity::OutlinePoly(POINT *pPoly, short nVertices, RECT& rClip, unsigned int rgbColor) { _bgmain(0x6C7940) }
_n void CArea::OnActionButtonUp(POINT& pt) { _bgmain(0x4CD908) }
_n int CInfinity::FXPrep(RECT& rFXRect, unsigned long dwFlags, int nSurface, POINT& ptPos, POINT& ptReference) { _bgmain(0x6C6439) }
_n int CInfinity::FXLock(RECT& rFXRect, unsigned long dwFlags) { _bgmain(0x6C6549) }
_n int CInfinity::FXRender(CVidCell* pVidCell, int nRefPointX, int nRefPointY, unsigned long dwFlags, int nTransValue) { _bgmain(0x6C6711) }
_n int CInfinity::FXUnlock(unsigned long dwFlags, RECT* pFxRect, POINT& ptRef) { _bgmain(0x6C7365) }
_n int CInfinity::FXBltFrom(int nSurface, RECT& rFXRect, int x, int y, int nRefPointX, int nRefPointY, unsigned long dwFlags) { _bgmain(0x6C6306) }
_n int CInfinity::FXRenderClippingPolys(int nPosX, int nPosY, int nPosZ, POINT& ptRef, RECT& rGCBounds, bool bDithered, unsigned long dwBlitFlags) { _bgmain(0x6C6B15) }
_n void CArea::OnDeactivation() { _bgmain(0x4CC770) }
_n void CArea::OnActivation()   { _bgmain(0x4CC436) }
