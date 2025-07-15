#ifndef VIDEOCOMMON_H
#define VIDEOCOMMON_H

#include "stdafx.h"

 class MVE_MovieMessage {
 public:
     void DrawString(uint LeftOffset,  uint TopOffset, uint ScreenWidth, uint ScreenHeight);
 };

void UnpackRGB565(unsigned short color, unsigned char& b, unsigned char& g, unsigned char& r);
short PackRGB565(unsigned char b, unsigned char g, unsigned char r);
void UnpackRGB8888(unsigned int color, unsigned char& a, unsigned char& b, unsigned char& g, unsigned char& r);
short PackRGB8888(unsigned char a, unsigned char b, unsigned char g, unsigned char r);

void FilterVideoMode_asm();
void OverrideVideoMode_asm();
void Set_OpenGL_VSync_asm();
void CVidCell_Blt8To32_FixPaletteFetch_asm();
void CVidCell_Blt8To32_FixFrameSize_asm();
void CVidCell_Blt8To16_FixFrameSize_asm();
void CVidCell_Blt8To16_FixPaletteFetch_asm();
void CVidCell_Blt8To16_FixPaletteFetch2_asm();
void CVidInf_RenderPointerImage_DoubleCursor_asm();
void CInfCursor_Initialize_SetupCursorVidCell_asm();
void CVidMode_SetPointer_SetupCursorVidCell_asm();
void CInfCursor_Initialize_SetupTooltipVidCell_asm();
void CInfToolTip_Initialize_SetupTooltipFont_asm();
void CInfToolTip_SetTextRef_DoubleWidth_asm();
void CBaldurChitin_SetupResolution1_asm();
void CBaldurChitin_SetupResolution2_asm();
void CVidCell_Blt8To16_FixPaletteNonCompressed_asm();
void SetLogicalWidth_asm();
void SetLogicalHeight_asm();
void CProjector_ShowFrame32_ScaleX2_asm();
void CProjector_ShowFrame16_ScaleX2_asm();
void CProjector_ShowFrame3d_RecalcOffsetsX2_asm();
void CProjector_ShowFrame3d_ScaleX2_asm();
void CProjector_ShowFrame32Packed_ScaleX2_asm();
void CProjector_ShowFrame16Packed_ScaleX2_asm();
void CProjector_ShowFrame32_ShowSubtitles_asm();
void CProjector_ShowFrame16_ShowSubtitles_asm();
void CProjector_ShowFrame3d_ShowSubtitles_asm();
void CProjector_ShowFrame3dPacked_ShowSubtitles_asm();
void MVE_DrawString_ShiftLine_asm();
void GetWindowHeightDX_asm();
void GetWindowHeightAX_asm();
void GetWindowHeightCX_asm();
void GetWindowWidthAX_asm();
void GetWindowWidthCX_asm();
void GetWindowWidthDX_asm();
void SetScissorOpenGL_asm();
void SetLineWidth_asm();
void LimitMouseXY_asm();
void CVidMode_DrawLine3d_TuneX1_asm();
void CVidMode_DrawLine3d_TuneY1_asm();
void CVidMode_DrawLine3d_TuneX0_asm();
void CVidMode_DrawLine3d_TuneY0_asm();

#endif //VIDEOCOMMON_H
