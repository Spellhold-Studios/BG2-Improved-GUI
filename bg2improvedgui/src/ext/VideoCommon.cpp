#include "VideoCommon.h"

#include "vidcore.h"
#include "chitin.h"

void *gTextureBuffer = NULL;    
extern bool gX2Render;

void UnpackRGB565(unsigned short color, unsigned char &b, unsigned char &g, unsigned char &r) {
	b = (color >> 8) & 0xF8;
	g = (color >> 3) & 0xFC;
	r = (color << 3) & 0xF8;
	return;
}

short PackRGB565(unsigned char b, unsigned char g, unsigned char r) {
    short color;
    color = (b & 0xF8) << 8;
    color += (g & 0xFC) << 3;
    color += r >> 3;
    return color;
}

void UnpackRGB8888(unsigned int color, unsigned char &a, unsigned char &b, unsigned char &g, unsigned char &r) {
	a = (color >> 24) & 0xFF;
	b = (color >> 16) & 0xFF;
	g = (color >> 8) & 0xFF;
	r = color & 0xFF;
	return;
}

short PackRGB8888(unsigned char a, unsigned char b, unsigned char g, unsigned char r) {
    return ((a << 24) | (b << 16) | (g << 8) | r);
}


BOOL static __stdcall
IsGoodVideoMode(DEVMODE* pMode, uint prevGoodModeIndex) {
    if ((pMode->dmFields & DM_DISPLAYFIXEDOUTPUT) && // interpolation mode for fixed-resolution display, skip it
        prevGoodModeIndex != 0)                      // if we are not first matched mode
        return FALSE;
    else
        return TRUE;
}


void static __stdcall
OverrideVideoMode(DEVMODE* pMode) {
    pMode->dmFields |= DM_DISPLAYFIXEDOUTPUT;
    pMode->dmDisplayFixedOutput = DMDFO_DEFAULT;
 }


bool glfwStringInExtensionString(const char* string, const char* extensions)
{
    const char* start = extensions;

    for (;;)
    {
        const char* where;
        const char* terminator;

        where = strstr(start, string);
        if (!where)
            return false;

        terminator = where + strlen(string);
        if (where == start || *(where - 1) == ' ')
        {
            if (*terminator == ' ' || *terminator == '\0')
                break;
        }

        start = terminator;
    }

    return true;
}


#define GL_EXTENSIONS 0x1F03

void static __stdcall
Set_OpenGL_VSync() {
    const char* wgl_exts_arrayARB = NULL;
    const char* wgl_exts_arrayEXT = NULL;

    DWORD (WINAPI *wglGetProcAddress)(char*);
    HDC   (WINAPI *wglGetCurrentDC)();
    char* (WINAPI *wglGetExtensionsStringEXT)();
    char* (WINAPI *wglGetExtensionsStringARB)(HDC);
    BOOL  (WINAPI *wglSwapIntervalEXT)(int) = NULL;
    int   (WINAPI *wglGetSwapIntervalEXT)() = NULL;

    HMODULE hnd_dll = (HMODULE) *((DWORD *) 0xB8CF94); // opengl32.dll

    wglGetProcAddress = (DWORD (WINAPI *)(char*))       GetProcAddress(hnd_dll, "wglGetProcAddress");
    wglGetCurrentDC   = (HDC   (WINAPI *)())            GetProcAddress(hnd_dll, "wglGetCurrentDC");
    wglGetExtensionsStringEXT = (char* (WINAPI *)())    wglGetProcAddress("wglGetExtensionsStringEXT");
    wglGetExtensionsStringARB = (char* (WINAPI *)(HDC)) wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtensionsStringARB && wglGetCurrentDC)
        wgl_exts_arrayARB = (const char*)  wglGetExtensionsStringARB(wglGetCurrentDC());

    if (wglGetExtensionsStringEXT)
        wgl_exts_arrayEXT = (const char*)  wglGetExtensionsStringEXT();

    if (wglGetProcAddress) {
        wglSwapIntervalEXT    = (BOOL (WINAPI *)(int)) wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (int (WINAPI *)())     wglGetProcAddress("wglGetSwapIntervalEXT");
    }

    //console.writef("prev vsync %d ", wglGetSwapIntervalEXT());

    if  ( wglSwapIntervalEXT &&
          (wgl_exts_arrayEXT && glfwStringInExtensionString("WGL_EXT_swap_control", wgl_exts_arrayEXT)) ||
          (wgl_exts_arrayARB && glfwStringInExtensionString("WGL_EXT_swap_control", wgl_exts_arrayARB))
        ) {
        wglSwapIntervalEXT(1);
    }

}


void static __stdcall
CProjector_Blt8To32_ScaleX2(      // 0x461059
    uchar *src,
    uint pLockedSurface,
    uint MovieWidth,
    uint MovieHeight,
    uint DestPitch,
    uint TopOffsetIn,
    uint LeftOffsetIn) {

/*
      A B C D, ... x (MovieWidth/4)
    +pitch
      Z X Y W, ... x (MovieWidth/4)
*/
    uint TopOffset = TopOffsetIn  - MovieHeight/2;   // normalize
    uint LeftOffset = LeftOffsetIn - MovieWidth/2;   // normalize
    DWORD *dst_newline = (DWORD *) (pLockedSurface + (TopOffset*DestPitch) + LeftOffset*4);
    DWORD *Palette = (DWORD *) 0xB77A20;
    uint DestPitchUINT = DestPitch/4;

    while (MovieHeight) {
        DWORD *dst = dst_newline;
        uint bytecount = MovieWidth;

        while (bytecount) {
            DWORD rgb = Palette[*src];
            // A1A2 BxBx  Line0
            // A3A4 BxBx
            //
            // Z1Z2       Line1 
            // Z3Z4
            dst[0]               = rgb;   // A1
            dst[DestPitchUINT]   = rgb;   // A3
            dst[1]               = rgb;   // A2
            dst[1+DestPitchUINT] = rgb;   // A4

            dst += 2;
            src++;
            bytecount--;
        }

        dst_newline += (DestPitchUINT*2); // go to Z1
        MovieHeight--;
    }
}


void static __stdcall
CProjector_BltPacked16To32_ScaleX2(      // 0x460DCE
    WORD *src,
    uint pLockedSurface,
    uint MovieWidth,
    uint MovieHeight,
    uint DestPitch,
    uint TopOffsetIn,
    uint LeftOffsetIn) {

/*
      A B C D, ... x (MovieWidth/4)
    +pitch
      Z X Y W, ... x (MovieWidth/4)
*/
    uint TopOffset = TopOffsetIn  - MovieHeight/2;   // normalize
    uint LeftOffset = LeftOffsetIn - MovieWidth/2;   // normalize
    DWORD *dst_newline = (DWORD *) (pLockedSurface + (TopOffset*DestPitch) + LeftOffset*4);

    DWORD nShiftLeftRed   = g_pChitin->pEngineActive->pVideoMode->nShiftLeftRed;
    DWORD nShiftLeftGreen = g_pChitin->pEngineActive->pVideoMode->nShiftLeftGreen;
    DWORD nShiftLeftBlue  = g_pChitin->pEngineActive->pVideoMode->nShiftLeftBlue;
    uint DestPitchUINT = DestPitch/4;

    while (MovieHeight) {
        DWORD *dst = dst_newline;
        uint bytecount = MovieWidth;

        while (bytecount) {
            WORD RGB555 = *src;
            DWORD rgb = (((RGB555 & 0x1F)   << 3) << nShiftLeftBlue) |
                        (((RGB555 & 0x3E0)  >> 2) << nShiftLeftGreen) |
                        (((RGB555 & 0x7C00) >> 7) << nShiftLeftRed);
            // A1A2 BxBx  Line0
            // A3A4 BxBx
            //
            // Z1Z2       Line1 
            // Z3Z4
            dst[0]               = rgb;   // A1
            dst[DestPitchUINT]   = rgb;   // A3
            dst[1]               = rgb;   // A2
            dst[1+DestPitchUINT] = rgb;   // A4

            dst += 2;
            src++;
            bytecount--;
        }

        dst_newline += (DestPitchUINT*2); // go to Z1
        MovieHeight--;
    }
}


void static __stdcall
CProjector_Blt8To16_ScaleX2(      // 0x460B1E
    uchar *src,
    uint pLockedSurface,
    uint MovieWidth,
    uint MovieHeight,
    uint DestPitch,
    uint TopOffsetIn,
    uint LeftOffsetIn) {

/*
      A B C D, ... x (MovieWidth/4)
    +pitch
      Z X Y W, ... x (MovieWidth/4)
*/
    uint TopOffset = TopOffsetIn  - MovieHeight/2;   // normalize
    uint LeftOffset = LeftOffsetIn - MovieWidth/2;   // normalize
    WORD *dst_newline = (WORD *) (pLockedSurface + (TopOffset*DestPitch) + LeftOffset*2);
    WORD *Palette = (WORD *) 0xB77A20;
    uint DestPitchUINT = DestPitch/2;

    while (MovieHeight) {
        WORD *dst = dst_newline;
        uint bytecount = MovieWidth;

        while (bytecount) {
            WORD rgb = Palette[*src];
            // A1A2 BxBx  Line0
            // A3A4 BxBx
            //
            // Z1Z2       Line1 
            // Z3Z4
            dst[0]               = rgb;   // A1
            dst[DestPitchUINT]   = rgb;   // A3
            dst[1]               = rgb;   // A2
            dst[1+DestPitchUINT] = rgb;   // A4

            dst += 2;
            src++;
            bytecount--;
        }

        dst_newline += (DestPitchUINT*2); // go to Z1
        MovieHeight--;
    }
}


void static __stdcall
CProjector_ShowFrame3d_RecalcOffsetsX2(      // 0x463F42
    uint MovieWidth,
    uint MovieHeight,
    uint *TopOffsetIn,
    uint *LeftOffsetIn) {

    uint ScreenWidth = *((uint *) 0xBDAE24);
    uint ScreenHeight = *((uint *) 0xBDAE54);
    *TopOffsetIn  = (ScreenHeight/2 - MovieHeight)/2;   // normalize
    *LeftOffsetIn = (ScreenWidth/2  - MovieWidth)/2;   // normalize
}

void static __stdcall
CProjector_BltPacked16To16_ScaleX2(      // 0x460A00
    WORD *src,
    uint pLockedSurface,
    uint MovieWidth,
    uint MovieHeight,
    uint DestPitch,
    uint TopOffsetIn,
    uint LeftOffsetIn) {

/*
      A B C D, ... x (MovieWidth/4)
    +pitch
      Z X Y W, ... x (MovieWidth/4)
*/
    uint TopOffset = TopOffsetIn  - MovieHeight/2;   // normalize
    uint LeftOffset = LeftOffsetIn - MovieWidth/2;   // normalize
    WORD *dst_newline = (WORD *) (pLockedSurface + (TopOffset*DestPitch) + LeftOffset*2);
    uint DestPitchUINT = DestPitch/2;

    while (MovieHeight) {
        WORD *dst = dst_newline;
        uint bytecount = MovieWidth;

        while (bytecount) {
            WORD rgb = *src;
            // A1A2 BxBx  Line0
            // A3A4 BxBx
            //
            // Z1Z2       Line1 
            // Z3Z4
            dst[0]               = rgb;   // A1
            dst[DestPitchUINT]   = rgb;   // A3
            dst[1]               = rgb;   // A2
            dst[1+DestPitchUINT] = rgb;   // A4

            dst += 2;
            src++;
            bytecount--;
        }

        dst_newline += (DestPitchUINT*2); // go to Z1
        MovieHeight--;
    }
}


_n void MVE_MovieMessage::DrawString( uint LeftOffset,  uint TopOffset, uint ScreenWidth, uint ScreenHeight)
                                    { _bgmain(0x462FFA)}


void static __stdcall
CProjector_ShowSubtitles(
    uint ScreenWidth,
    uint ScreenHeight,
    uint MovieWidth,
    uint MovieHeight,
    uint TopOffsetIn,
    uint LeftOffsetIn) {

    MVE_MovieMessage *MovieMessage   = (MVE_MovieMessage *) *((DWORD *)0xB77E30);

    uint LeftOffset = (ScreenWidth  -  (MovieWidth*2))/2;                   // Left X
    uint TopOffset  = (ScreenHeight - (MovieHeight*2))/2 + MovieHeight*2;   // Bottom Y


    MovieMessage->DrawString(LeftOffset, TopOffset, ScreenWidth, ScreenHeight);
}


void static __stdcall
LimitMouseXY(tagPOINT &MousePt) {
    const ushort XRes   = *(( WORD *) (0xB6150C));
    const ushort YRes   = *(( WORD *) (0xB6150E));

    if (gX2Render) {
        if (MousePt.x > XRes) MousePt.x = XRes - 1;
        if (MousePt.y > YRes) MousePt.y = YRes - 1;
    }
}


void  __declspec(naked)
FilterVideoMode_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-10h]           // prev good mode index
    lea     eax,[ebp-0A4h]      // DEVMODE (pre w2000 version)
    push    eax
    call    IsGoodVideoMode

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jnz     PreferDefaultVideoMode_continue

    add     esp,4
    push    09F1868h    ; skip this mode
    ret

PreferDefaultVideoMode_continue:
    mov     [ebp-0B8h], ecx     // Stolen bytes
    ret
}
}


void  __declspec(naked)
OverrideVideoMode_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    lea     eax,[ebp-0A4h]  // DEVMODE (pre w2000 version)
    push    eax
    call    OverrideVideoMode

    pop     edx
    pop     ecx
    pop     eax

    lea     edx, [ebp-0A4h]     // Stolen bytes
    ret
}
}


void  __declspec(naked)
Set_OpenGL_VSync_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    call    Set_OpenGL_VSync

    pop     edx
    pop     ecx
    pop     eax

    mov     eax, 1     // Stolen bytes
    ret
}
}


void  __declspec(naked)
CVidCell_Blt8To32_FixPaletteFetch_asm() {
__asm {
    push    eax

    mov     al, [esi]       // Palette Entry
    cmp     al, [ebp-18h]   // is TransparentColorEntry ?
    jnz     CVidCell_Blt8To32_FixPaletteFetch_Normal

    // skip overwriting already cleared surface (cleared surface filled with transparent color)
    pop     eax
    ret


CVidCell_Blt8To32_FixPaletteFetch_Normal:
    pop     eax
    mov     eax, [eax+ebx]  // Stolen bytes
    mov     [edi], eax      // normal write
    ret
}
}


void  __declspec(naked)
CVidCell_Blt8To32_FixFrameSize_asm() {
__asm {
    mov     ecx, [eax+0CCh]
    mov     dx, [ecx]
    mov     word ptr [ebp-014h], dx // FrameWidth
    mov     ecx, [eax+0CCh]
    mov     dx, [ecx+2]
    mov     word ptr [ebp-8], dx    // FrameHeight
    ret
}
}


void  __declspec(naked)
CVidCell_Blt8To16_FixFrameSize_asm() {
__asm {
    mov     ecx, [eax+0CCh]
    mov     dx, [ecx]
    mov     word ptr [ebp-014h], dx // FrameWidth
    mov     ecx, [eax+0CCh]
    mov     dx, [ecx+2]
    mov     word ptr [ebp-8], dx    // FrameHeight
    ret
}
}


void  __declspec(naked)
WriteColorTwoPixels() {
__asm {
    mov     al, [esi]       // Palette Entry
    cmp     al, [ebp-18h]   // is AlphaColorEntry ?
    jz      CVidCell_Blt8To16_FixPaletteFetch_Skip1

    mov     dx, [ebx+eax*4]
    mov     [edi], dx

CVidCell_Blt8To16_FixPaletteFetch_Skip1:
    inc     esi
    lea     edi, [edi+2]

    mov     al, [esi]       // Palette Entry
    cmp     al, [ebp-18h]   // is AlphaColorEntry ?
    jz      CVidCell_Blt8To16_FixPaletteFetch_Skip2

    mov     dx, [ebx+eax*4]
    mov     [edi], dx

CVidCell_Blt8To16_FixPaletteFetch_Skip2:
    inc     esi
    lea     edi, [edi+2]

    ret
}
}


void  __declspec(naked)
WriteColorPixel() {
__asm {
    mov     al, [esi]       // Palette Entry
    cmp     al, [ebp-18h]   // is AlphaColorEntry ?
    jz      CVidCell_Blt8To16_FixPaletteFetch2_Skip

    mov     [edi], dx       // Stolen bytes

CVidCell_Blt8To16_FixPaletteFetch2_Skip:
    lea     edi, [edi+2]
    ret
}
}


#define arg_0       (+0x8)
#define var_4       (-0x4)
#define FrameHeight (-0x8)
#define var_C       (-0xC)
#define var_10      (-0x10)
#define FrameWidth  (-0x14)


void  __declspec(naked)
CVidCell_Blt8To16_FixPaletteNonCompressed_asm() { // IDA disassembled
__asm {
    push    esi
    push    edi
    mov     esi, [ebp+var_4]
    mov     ebx, [ebp+var_10]
    mov     cx,  [ebp+FrameHeight]
    mov     edi, [ebp+arg_0]
    xor     eax, eax
    test    word ptr [ebp+FrameWidth], 0FFFEh
    jz      OnePixelWidth____l816

    test    word ptr [ebp+FrameWidth], 1
    jnz     OddWidth____l816

    shr     word ptr [ebp+FrameWidth], 1

NextLine____l816:
    push    ecx // FrameHeight
    mov     cx, word ptr [ebp+FrameWidth]

NextPixels____l816:
    call    WriteColorTwoPixels
    dec     cx
    jnz     NextPixels____l816

    pop     ecx // FrameHeight
    add     edi, [ebp+var_C]
    dec     cx
    jnz     NextLine____l816
    jmp     Exit____l816

OddWidth____l816:
    shr     word ptr [ebp+FrameWidth], 1

OddNextLine___l816:
    push    ecx // FrameHeight
    mov     cx, word ptr [ebp+FrameWidth]
    mov     al, [esi]
    inc     esi
    mov     dx, [ebx+eax*4]
    call    WriteColorPixel // mov     [edi], dx
                            // lea     edi, [edi+2]

EvenNextLine___l816:
    call    WriteColorTwoPixels
    dec     cx
    jnz     EvenNextLine___l816

    pop     ecx // FrameHeight
    add     edi, [ebp+var_C]
    dec     cl
    jnz     OddNextLine___l816
    jmp     Exit____l816

OnePixelWidth____l816:
    mov     al, [esi]
    inc     esi
    mov     dx, [ebx+eax*4]
    call    WriteColorPixel // mov     [edi], dx
                            // lea     edi, [edi+2]
    add     edi, [ebp+var_C]
    dec     cx
    jnz     OnePixelWidth____l816

Exit____l816:
    pop     edi
    pop     esi

    mov     eax, 1
    pop     edi
    pop     esi
    pop     ebx
    mov     esp, ebp
    pop     ebp
    retn    0Ch
}
}


void __declspec(naked)
CVidInf_RenderPointerImage_DoubleCursor_asm() {
__asm
{
    mov     eax, [g_pChitin]
    mov     eax, [eax]CBaldurChitin.bDoubleResolution
    mov     dword ptr [ebp-1Ah], eax
    ret
}
}


void __declspec(naked)
CInfCursor_Initialize_SetupCursorVidCell_asm() {
__asm
{
    mov     ecx, [g_pChitin]
    mov     ecx, [ecx]CBaldurChitin.bDoubleResolution
    mov     dword ptr [eax+0D2h], ecx
    ret
}
}


void __declspec(naked)
CVidMode_SetPointer_SetupCursorVidCell_asm() {
__asm
{
    mov     eax, [g_pChitin]
    mov     eax, [eax]CBaldurChitin.bDoubleResolution
    mov     dword ptr [edx+0D2h], eax
    ret
}
}


void __declspec(naked)
CInfCursor_Initialize_SetupTooltipVidCell_asm() {
__asm
{
    mov     eax, [g_pChitin]
    mov     eax, [eax]CBaldurChitin.bDoubleResolution
    mov     dword ptr [ecx+0D2h], eax
    ret
}
}


void __declspec(naked)
CInfToolTip_Initialize_SetupTooltipFont_asm() {
__asm
{
    mov     eax, [esp]
    push    eax             // shift stack

    mov     eax, [g_pChitin]
    mov     eax, [eax]CBaldurChitin.bDoubleResolution
    mov     [esp+4], eax    // emulate "push 0/1"

    lea     ecx, [ebp-20h]  // Stolen bytes
    ret
}
}


void __declspec(naked)
CBaldurChitin_SetupResolution1_asm() {
__asm
{
    movzx   eax, word ptr ds:[0B6150Ch]  // ScreenWidth
    cmp     eax, 1280
    jb      CBaldurChitin_SetupResolution1_LowRes

    mov     dword ptr [ecx+6CF4h], 1
    ret

CBaldurChitin_SetupResolution1_LowRes:
    mov     dword ptr [ecx+6CF4h], 0  // Stolen bytes
    ret
}
}


void __declspec(naked)
CBaldurChitin_SetupResolution2_asm() {
__asm
{
    movzx   eax, word ptr ds:[0B6150Ch]  // ScreenWidth
    cmp     eax, 1280
    jb      CBaldurChitin_SetupResolution2_LowRes

    mov     dword ptr [ecx+6CF8h], 1
    ret

CBaldurChitin_SetupResolution2_LowRes:
    mov     dword ptr [ecx+6CF8h], 0  // Stolen bytes
    ret
}
}


void __declspec(naked)
SetLogicalWidth_asm() {
__asm
{
    mov     cx, word ptr ds:[0B6150Ch] //ScreenWidth
    shr     cx, 1
    ret
}
}



void __declspec(naked)
SetLogicalHeight_asm() {
__asm
{
    mov     dx, word ptr ds:[0B6150Eh] //ScreenHeight
    shr     dx, 1
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame32_ScaleX2_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp-7Ch]   // DestPitch
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-68h]   // pLockedSurface
    push    [ebp-14h]   // src
    call    CProjector_Blt8To32_ScaleX2

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame16_ScaleX2_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp-7Ch]   // DestPitch
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-68h]   // pLockedSurface
    push    [ebp-14h]   // src
    call    CProjector_Blt8To16_ScaleX2

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame3d_RecalcOffsetsX2_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp+24h]   // LeftOffsetIn
    push    eax
    lea     eax, [ebp+28h]   // TopOffsetIn
    push    eax
    push    [ebp+20h]       // MovieHeight
    push    [ebp+1Ch]       // MovieWidth    

    call    CProjector_ShowFrame3d_RecalcOffsetsX2

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp+28h]  // Stolen bytes
    mov     [ebp-18h], ecx

    ret
}
}

float f_x2 = 2.0;

void __declspec(naked)
CProjector_ShowFrame3d_ScaleX2_asm() {
__asm
{
    fmul   dword ptr [f_x2]         // x2
    fadd   dword ptr ds:[0xAA634C]  // 0.2
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame32Packed_ScaleX2_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp-7Ch]   // DestPitch
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-68h]   // pLockedSurface
    push    [ebp-0A8h]  // src
    call    CProjector_BltPacked16To32_ScaleX2

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame16Packed_ScaleX2_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp-7Ch]   // DestPitch
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-68h]   // pLockedSurface
    push    [ebp-94h]   // src
    call    CProjector_BltPacked16To16_ScaleX2

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame32_ShowSubtitles_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-84h]   // ScreenHeight
    push    [ebp-80h]   // ScreenWidth
    call    CProjector_ShowSubtitles

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame16_ShowSubtitles_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1Ch]   // LeftOffsetIn
    push    [ebp-18h]   // TopOffsetIn
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    push    [ebp-84h]   // ScreenHeight
    push    [ebp-80h]   // ScreenWidth
    call    CProjector_ShowSubtitles

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame3d_ShowSubtitles_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp+24h]   // LeftOffsetIn
    push    [ebp+28h]   // TopOffsetIn
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    xor     eax,eax
    mov     ax, word ptr ds:[0B6150Eh] // ScreenHeight word
    push    eax
    mov     ax, word ptr ds:[0B6150Ch] // ScreenWidth word
    push    eax
    call    CProjector_ShowSubtitles

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
CProjector_ShowFrame3dPacked_ShowSubtitles_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp+24h]   // LeftOffsetIn
    push    [ebp+28h]   // TopOffsetIn
    push    [ebp+20h]   // MovieHeight
    push    [ebp+1Ch]   // MovieWidth
    xor     eax,eax
    mov     ax, word ptr ds:[0B6150Eh] // ScreenHeight word
    push    eax
    mov     ax, word ptr ds:[0B6150Ch] // ScreenWidth word
    push    eax
    call    CProjector_ShowSubtitles

    pop     edx
    pop     ecx
    pop     eax
    ret
}
}


void __declspec(naked)
MVE_DrawString_ShiftLine_asm() {
__asm
{
    mov     ecx, [ebp-58h]  // Current Line
    inc     ecx
    inc     ecx             // two lines down
    imul    ecx, eax
    
    ret
}
}


void  __declspec(naked)
GetWindowHeightDX_asm() {
__asm {
    mov     dx, word ptr ds:[0B6150Eh] //ScreenHeight
    shl     edx, 1
    ret
}
}

void  __declspec(naked)
GetWindowHeightAX_asm() {
__asm {
    mov     ax, word ptr ds:[0B6150Eh] //ScreenHeight
    shl     eax, 1
    ret
}
}

void  __declspec(naked)
GetWindowHeightCX_asm() {
__asm {
    mov     cx, word ptr ds:[0B6150Eh] //ScreenHeight
    shl     ecx, 1
    ret
}
}


void  __declspec(naked)
GetWindowWidthAX_asm() {
__asm {
    mov     ax, word ptr ds:[0B6150Ch] //ScreenWidth
    shl     eax, 1
    ret
}
}


void  __declspec(naked)
GetWindowWidthCX_asm() {
__asm {
    mov     cx, word ptr ds:[0B6150Ch] //ScreenWidth
    shl     ecx, 1
    ret
}
}

void  __declspec(naked)
GetWindowWidthDX_asm() {
__asm {
    mov     dx, word ptr ds:[0B6150Ch] //ScreenWidth
    shl     edx, 1
    ret
}
}


void  __declspec(naked)
SetScissorOpenGL_asm() {
__asm {
    // scissor are window coordinates, need x2
    shl     dword ptr [esp+04h], 1
    shl     dword ptr [esp+08h], 1
    shl     dword ptr [esp+0Ch], 1
    shl     dword ptr [esp+10h], 1
    mov     eax, ds:[0B8DF80h] //glScissor
    jmp     eax
}
}


void  __declspec(naked)
LimitMouseXY_asm() {
__asm {
    push    ecx
    push    edx

    lea     eax, [ebp-4Ch]       // CPoint
    push    eax
    call    LimitMouseXY

    pop     edx
    pop     ecx

    mov     edx, [ebp-70h]  // Stolen bytes
    xor     eax, eax
    ret
}
}


void  __declspec(naked)
CVidMode_DrawLine3d_TuneX1_asm() {
__asm {
    mov     dword ptr [ebp-4],   3F800000h  // XTo   +1
    mov     dword ptr [ebp-5Ch], 3F000000h  // YFrom +0.5
    mov     dword ptr [ebp-8],   3F000000h  // YTo   +0.5
    mov     dword ptr [ebp-58h], 00000000h  // XFrom +0
    ret
}
}

void  __declspec(naked)
CVidMode_DrawLine3d_TuneY1_asm() {
__asm {
    mov     dword ptr [ebp-8],   3F800000h  // YTo   +1
    mov     dword ptr [ebp-58h], 3F000000h  // XFrom +0.5
    mov     dword ptr [ebp-4],   3F000000h  // XTo   +0.5
    mov     dword ptr [ebp-5Ch], 00000000h  // YFrom +0
    ret
}
}


void  __declspec(naked)
CVidMode_DrawLine3d_TuneX0_asm() {
__asm {
    fadd    [ebp-58h]               // XFrom Inc
    fadd    dword ptr ds:[0AA634Ch] // 0.2 shift
    ret
}
}


void  __declspec(naked)
CVidMode_DrawLine3d_TuneY0_asm() {
__asm {
    fadd    [ebp-5Ch]               // YFrom Inc
    fadd    dword ptr ds:[0AA634Ch] // 0.2 shift
    ret
}
}


//void  __declspec(naked)
//SetLineWidth_asm() {
//__asm {
//    push   40000000h                // 2.0
//    call   dword ptr ds:[0B8DE00h]  // glLineWidth
//    push   1B01h                    // GL_LINE
//    push   0405h                    // GL_BACK
//    call   dword ptr ds:[0B8DEB4h]  // glPolygonMode
//    ret
//}
//}