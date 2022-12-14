#ifndef VIDCORE_H
#define VIDCORE_H

#include "stdafx.h"
#include "datatypes.h"
#include "rescore.h"

#define CVIDIMG_TRANSPARENT				0x00000001
#define CVIDIMG_TRANSLUCENT				0x00000002
#define CVIDCELL_TRANSLUCENT_SHADOW		0x00000004
#define CVIDCELL_BRIGHTEST				0x00000008

struct CVidPolySurface;

typedef unsigned int ABGR; //[ALPHA.RED.GREEN.BLUE]

extern const ABGR* g_pColorRangeArray;
extern const ABGR g_ColorDefaultText;

struct CVidPolySurface { //Size 24h
	CVidPolySurface* pSurfacePrev; //0h
	int x; //4h
	int y; //8h
	int uc; //directionX - 1 = backward from previous x, -1 = fwd or same from previous x
	int u10; //((-(cntx - cnt+1x)) / (cnty - cnt+1y)) * cnt+1y
	int u14; //1 - (cnty - cnt+1y)
	int u18; //(-(cntx - cnt+1x)) % (cnty - cnt+1y)
	int u1c; //cnty - cnt+1y
	int u20; //cnty - cnt+1y
};

struct CVidPoly { //Size 14h?
//Constructor: 0x9F69C0
	int m_pVertexArray; //0h, one element is [WORDx.WORDy]
	int m_nVertices; //4h
	CVidPolySurface* u8; //LinkedList
	CVidPolySurface* uc; //LinkedList, derived from u8 
		
	//void CVidPoly::RenderLine(pSurface, startIdx, endIdx, rgbColor, pRectMirror, pPoint)
	//see CVidPoly::SetLineRenderFunc(dwFlags) for more info 0x9F8563
	int u10;
};

struct CVidBitmap;

class CVidPalette { //Size 24h
public:
//Constructor: 0x9F3640
	void SetFxPaletteNo3d(ABGR* pPalette, int nBitsPerPixel, unsigned int dwFlags, int dwAlpha, BOOL bIgnoreBrighten);
    void SetRange(ushort nRange, ushort nValue, CVidBitmap& bmpMasterPalette);

	unsigned int m_nAUCounter;      // pRgbColor + nChitinUpdates?, gets an element from the palette
	unsigned int m_nAUCounterBase;  // an alternative palette?
		
	struct tagRGBQUAD *m_pPalette;  //8h, m_pPalette
	//TYPE_SET palette is an arrayptr (int size objects - 2 WORDs)

	//TYPE_RANGE palette is...
	//int u0[4]; //special colours
	//int u10[7][12]; //7x12
	//int u160[168]; //generated from the 7x12 colours

	int nColors;                    // m_nEntries ch, nSize
	unsigned int rgbGlobalTint;
	unsigned short m_nType;         //14h, Arg1, m_nType (0 = TYPE_SET, 1 = TYPE_RANGE)
	unsigned char m_bPaletteOwner;
	char pad0;
	BOOL m_bSubRangesCalculated;
	ColorRangeValues colors;        //1ch
	char pad1;
};

extern void (CVidPalette::*CVidPalette_SetFxPaletteNo3d)(ABGR*, int, unsigned int, int, BOOL);


struct CVIDIMG_PALETTEAFFECT            // Size=0x78
{
	// for TYPE_SET palette (also used for TYPE_RANGE)
	// ABGR m_rgbLightMask; //24h
	// int m_brightness; //28h, some brightness value to multiply
	// ABGR m_rgbGamma; //2ch, RGB value based on char 0xAB9AF6 (3)

	// for TYPE_RANGE palette; 7 colors for each range set
	// lightmask
	// ABGR* u30[7];
	// char u4c[7]; //adjustments if not 1
	// char u53; //pad

	// brightness
	// ABGR* u54[7];
	// char u70[7];
	// char u77; //pad

	// gamma
	// ABGR* u78[7];
	// char u94[7];
	// char m_nColorRangeBitfield; //bits 0-7 for each nRangeId set

    unsigned long rgbTintColor;         // Offset=0x0 Size=0x4
    unsigned long rgbAddColor;          // Offset=0x4 Size=0x4
    unsigned long rgbLightColor;        // Offset=0x8 Size=0x4
    // lightmask
    unsigned long *pRangeTints[7];      // Offset=0xc Size=0x1c
    unsigned char aRangeTintPeriods[8]; // Offset=0x28 Size=0x8
    // brightness
    unsigned long *pRangeAdds[7];       // Offset=0x30 Size=0x1c
    unsigned char aRangeAddPeriods[8];  // Offset=0x4c Size=0x8
    // gamma
    unsigned long *pRangeLights[7];     // Offset=0x54 Size=0x1c
    unsigned char aRangeLightPeriods[7];// Offset=0x70 Size=0x7
    unsigned char suppressTints;        // m_nColorRangeBitfield Offset=0x77 Size=0x1
};

//BAM V1 Frame Entries
struct FrameTableEntry   // Size=0xc
{
    unsigned short nWidth;  //0x0
    unsigned short nHeight; //0x2
    short nCenterX;         //0x4
    short nCenterY;         //0x6
    unsigned long nOffset;  //0x8, bit 31: 0=Compressed (RLE), 1=Uncompressed
};

struct CVidImage { //Size 9Ch
//Constructor: 0x9C97F0
//vtable: NULL
//Note: due to virtual table, CVid offsets add 4h
    void SetTintColor(ABGR color);
    CVidPalette m_cPalette;           // Offset=0x0 Size=0x24
    CVIDIMG_PALETTEAFFECT mPaletteAffects;  // Offset=0x24 Size=0x78
};


class CVidCell : public CVidImage { //Size D6h
//Constructor: 0x9C9D57 (0 args; used for creature animations, non-overlapping BAM)
//Constructor: 0x9C9E7D (2 args; used in VVC projectiles, SPMAGMIS, GRNDGLOW, STATES, FLAG1, COLGRAD, STORTINT, BAMs with transparency)
//vtable: 0xAB9890 (aux 0xAB8FF4 ? identical)
//Note: due to virtual table, CVid offsets add 4h
public:
    CVidCell();     //9C9D57
    CVidCell(ResRef cNewResRef, BOOL bDoubleResolution);
    //~CVidCell();    //9CA225
	virtual BOOL IncrementFrame() { return FALSE; } //v0
	virtual void v4() {} //BOOL CVidCell::FXRenderSoftNoPalette(lpSurface, dwLinearSize, x, y, pRect rClip, int, dwFlags, pLoc source)
	virtual void v8() {} //Render, 9CD125
	virtual void vc() {} //9CCA89(lpSurface, dwLinearSize, centreX, centreY, dispflags, 0)
	virtual void v10() {} //9FE56C
	virtual void v14() {}
	virtual void v18() {} //BOOL LoadFrame(nFrame)
    void Render(          // quick call instead v8()
        int nSurface,
        int x,
        int y,
        RECT *rClipping,
        CVidPoly *pClipPoly,
        int nPolys,
        unsigned int dwFlags,
        int nTransVal);
    void SequenceSet(unsigned short nSequence);
    void FrameSet(unsigned short);
    void GetCurrentFrameSize(SIZE *Size, BOOL bReleaseRes);
    void GetCurrentCenterPoint(POINT *p, BOOL bReleaseRes);

	ResCellContainer       bam; //a0h
    ResCellHeaderContainer bah; //b0h
	short nCurrentFrame; //c0h, current frame
	unsigned short nCurrentSequence; //c2h, current cycle, related to ua2 in VisualEffect
	int nAnimType;
	BOOL bPaletteChanged; // bOwnPaletteSet; c8h
    struct FrameTableEntry *pFrameEntry; //cch, from BAM file - via GetFrame()
	unsigned char bShadowOn; //bool bColorOneIsNotBlack; if 0, palette[1] is set to [0.0.0]; inits to 1; is never changed
	char ud1; //padding?
	BOOL bDoubleResolution; //d2h, when need render at x2 resolution
};

struct CVidMosaic : public CVidImage { //Size B0h
//Constructor: 0x9D0601 (0 args)
//Constructor: 0x9D069D (2 args, used for ImeUI)
//vtable: NULL
	struct ResMosContainer {
		BOOL bLoaded;        //9ch (+0)
		Res* pRes;           //a0h (+4h) ResMos
		ResRef name;         //a4h (+8h) MOS files
	} ResHelper;                 
	int bDoubleResolution;   //ach
};

struct CVidTile : public CVidImage { //Size A4h
//Constructor: 0x9D1797
	int *pRes;   // class CResTile *pRes; int u9c; Offset=0x9c Size=0x4
	unsigned long m_dwFlags;// int ua0; Offset=0xa0 Size=0x4
};

struct CInfTileSet { //Size B8h
//Constructor: 0x6C1450
	int u0; //brightness
	ABGR u4; //ABGR lightmask, obtained from CInfinity.u1b8
	CVidTile cVidTile;
	int uac;
	int* pArray; //b0h, ptrs to CObjects (0x72h size; inherits from ResTis, constructor 0x6C283C)
	int nArraySize; //b4h
};

struct CVidBitmap : public CVidImage { //Size B6h
//Constructor: 0x9D3AF8 (0 args; uses area light map, area height map, character small portrait)
//Constructor: 0x9D3C78 (2 args; uses lightmap night)
//Constructor: 0x9D3E55 (3 args; uses mpalette)
//vtable: NULL
    void RenderDirect(int nSurface, int xLeft, int yTop, RECT *rect, int ColorMask, BOOL bLoadResBitmap);
	struct ResBmpContainer {
		BOOL bLoaded; //9ch, hasRes
		Res* pRes; //a0h, pResBmp
		ResRef name; //a4h
	} u9c;
	short m_nBitCount; //ach, CVIDBITMAP_8BIT = 8, CVIDBITMAP_4BIT = 4
	IECString m_szResFileName;
	int ub2; //b8bit (default is 4bit)
};

class CVidFont : public CVidCell { //Size 4FCh
//Constructor: 0x9A94C0
public:
	char ud6[0x400];
	HGDIOBJ hFont; //4d6h
	BOOL bCreatedFont; //4dah
	int u4de;
	short u4e2;
	short wAscent; //4e4h
	short wHeight; //4e6h
	ABGR colGreyText; //4e8h
	ABGR colText; //4ech
	int u4f0;
	int u4f4;
	int u4f8;
};

class CVideoModeBase { //Size F4h
//Constructor: 0x9B3BED
public:
	//AB910C
	int* vtable; //0h
	int u4;
	int u8;
	unsigned int uc; //dwFlags
	int u10;
	int u14;
	int u18; //ff

	//DDCOLORKEY
	int u1c; //dwColorSpaceLowValue, ff00
	int u20; //dwColorSpaceHighValue, ff0000

	ABGR u24; //RgbTriple lightmask for wallgroup obscured animations, normally [0.0.0]

#ifdef _DEBUG
	_CCriticalSection ccs; //28h, for access to 8h, 8ah, 8eh
#else
	CCriticalSection ccs; //28h
#endif
	//Note char 0xB61512 sets the target brightness, 1 = normal, FF for slowest screen fade
	char FadeDirection; //48h, 0 = ToColor, 1 = FromColor
	char FractionalBrightness; //49h, used as a fraction of B61512; in FadeFromColor, decrements
#ifdef _DEBUG
	_CCriticalSection ccs2; //0x4a
#else
	CCriticalSection ccs2; //0x4a
#endif
	IECPtrList u6a; //AB5C50
	int u86;
	CVidCell* pCursorCurrent; //8ah
	char u8e;
	char u8f;
	int u90; //GetTickCount()
	int FrameRate;
	int u98;
	int nSurfaces; //9ch

	int* ppSurfaceArray; //a0h, 20h size, contains pSurface
	//0 = CVIDINF_SURFACE_BACK  +0
	//1 = CVIDINF_SURFACE_FRONT +4
	//2 = SURFACE_FX            +8      256x256
	//3 = SURFACE_MIRROR_FX     +c      256x256
	//4 = CURSOR_SURFACE_BACK   +10     128x64 (see bUserLargerTooltipScroll)
	//5 = CURSOR_SURFACE_FRONT  +14     128x64
	//6                         +18     64x64
	//7                         +1c     64x64

	ABGR ua4; //Color Correction light mask, copied from CInfinity.u1b8
		
	//colors adjusted by 255 - (255-brightness)*(255-color)/256
	//essentially addition with capping at 255
	char m_brightness; //a8h, Brightness Correction (0-40)
		
	//colors adjusted by color * (1+gamma/8)
	char m_gamma; //a9h, Gamma Correction (0-5)

	//bitshifts for colours
	int nShiftLeftRed; //aah, 0 in 32bit, 11 in 16 bit
	int nShiftLeftGreen; //aeh, 8 in 32bit, 5 in 16 bit
	int nShiftLeftBlue; //b2h, 16 in 32bit, 0 in 16 bit
	int ub6; //8
	int uba; //not set
	int ube; //not set
	int nShiftRightRed; //c2h, 0 in 32bit, 3 in 16bit
	int nShiftRightGreen; //c6h, 0 in 32bit, 2 in 16bit
	int nShiftRightBlue; //cah, 0 in 32bit, 3 in 16bit
	int uce; //unused?
	short ud2; //unused?
	int ud4;
	int bShowMouseCursor; //d8h
	int udc;
	int ue0;
	int ue4;
	int ue8;
	int uec;
	int uf0;
};

class CVideoMode : public CVideoModeBase { //Size 732h
//Constructor: 0x9B68C5
public:
	//AB9238
	struct DDSURFACEDESC {
		int dwSize; //f4h
		unsigned int dwFlags; //f8h
		int dwHeight; //fch
		int dwWidth; //100h
		int dwLinearSize; //104h
		int dwBackBufferCount; //108h
		int dwRefreshRate; //10ch
		int dwAlphaBitDepth; //110h
		int dwReserved; //114h
		int lpSurface; //118h, FXSurface for non-3d
		
		int u11c; //ddckCKDestOverlay
		int u120;

		int u124; //ddckCKDestBlt
		int u128;

		int u12c; //ddckCKSrcOverlay
		int u130;

		int u134; //ddckCKSrcBlt
		int u138;

		int u13c; //ddpfPixelFormat
		int u140;
		int u144;
		int u148;
		int u14c;
		int u150;
		int u154;
		int u158;

		int u15c; //ddsCaps
	} m_SurfaceDesc; //f4h

	RECT rFxSurfaceLockedArea; //160h, area of FxSurface that has been locked for drawing
	int u170;
	CVidFont u174; //NORMAL.BAM
	int u670; //p4b00h size object (begins with IDirectDrawSurface array)
	int u674; //nElements in p4b00h size object
	CVidBitmap u678;
	IECString u72e;
};

struct CVideo { //Size 168h
//Constructor: 0x9AD29E
	short nBitsPerPixel; //0h
	short u2;
	bool bIs16Bit; //4h
	bool bIs24Bit; //5h
	bool bIs32Bit; //6h
	char u7; //pad?
	struct CSoftBlt {
		//Size: 0x100
		//Constructor: 0x9FCFA0
		char u0; //SoftSrcKeyBltFast
		char u1; //SoftBltFast
		char u2; //SoftSrcKeyBlt
		char u3; //SoftBlt
		char u4; //SoftMirrorBlt
		char u5[0xd9];
		int ude;
		int ue2;
		char ue6;
		char ue7; //pad?
		int* ue8; //funcptr - ? for Blt()               DDraw/SoftBlt
		int* uec; //funcptr - ? for SrcKeyBlt()         DDraw/SoftSrcBlt
		int* uf0; //funcptr - ? for MirrorBlt()         DDraw/SoftBlt
		int* uf4; //funcptr - ? for SrcKeyMirrorBlt()   DDraw/SoftSrcBlt
		int* uf8; //funcptr - ? for BltFast()           DDraw/SoftBlt
		int* ufc; //funcptr - ? for SrcKeyBltFast()     DDraw/SoftSrcBlt
	} m_SoftBlt; //8h, software blitter
	int dwRefreshRate16Bit; //108h
	int dwRefreshRate24Bit; //10ch
	int dwRefreshRate32Bit; //110h
	ResRef u114;
	short u11c;
	int u11e;
	int m_doubleSizeData; //122h, Double Size Data
	int u126;
	HDC dc; //12ah, HDC GetDC(hWnd) - DeviceContext
	int u12e; //HGLRC wglCreateContext(hdc)
	BOOL bIs3dAccelerated; //132h (3078h)
	int u136; //? glTextureFormat
	BOOL bBackwardsCompatible3d; //13ah, Backwards Compatible 3d (3080h)
	int OpenGL_TextureID; //GLuint texture (for glBindTexture)
	int u142; //pIDirectDraw (DirectDrawCreate onto here)
	int u146; //LPDDSURFACEDESC pIDirectDraw2 (pSurface[1])
	int u14a; //pIDirectDrawClipper
	CVideoMode* VideoModes[4]; //14eh
	int nCurrentVideoModeIdx; //15eh
	char u162;
	bool bFullScreen; //163h
	HWND hBaldurChitin; //164h
};

struct CMarker { //Size 24h
//Constructor: 0x95BFAF, also 0x95D095
    void    Render(CVideoMode& pVidMode, uint ArgX, DWORD *pInfinity, POINT *ptCenter, int nXSize, int nYSize);
	bool    type; //bCircle, 0: targetted pizza look, 1: unbroken circle
	char    u1; //pad
	short   nRecticleCounter; //2h, wHighlights, number of times pizza look is requested
	char    nRecticleForceRender;//4h
	char    nRecticleForceRenderTarget;//5h
	ABGR    rgbCircle; //6h, for unbroken circle
	POINT   ptCenter;//0a
	short   xAxis;
	short   yAxis;
	short   piePiecePtXOffset;
	short   piePiecePtYOffset;
	short   piePieceXOffset;
	short   piePieceYOffset;
	short   xGap;
	short   yGap;
	bool    bTalking;
	char u23; //pad
};

#endif //VIDCORE_H