/*
	normalize.c - main source file for PCM WAV normalizer - v0.253
	(c) 2000-2004 Manuel Kasper <mk@neon1.net>
	smartpeak code by Lapo Luchini <lapo@lapo.it>.
    16Bit version only by Insomniator
    SSE2 optimization by Insomniator

	This file is part of normalize.

	normalize is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	normalize is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"
#include "chitin.h"

#define COPYRIGHT_NOTICE	"normalize v0.253 (c) 2000-2004 Manuel Kasper <mk@neon1.net>.\n" \
							"All rights reserved.\n" \
							"smartpeak code by Lapo Luchini <lapo@lapo.it>."

static signed short    *table16;
static double          g_ratio, g_normpercent, g_peakpercent;
static int             g_smartpeak;
static double          g_mingain = 0;
static int             g_usemingain;

extern double gNormalizeAmplLastValue;
extern ResRef gSoundFileName;

void make_table16(void);
int getpeaks16(short* PCM, ulong Len, signed short *minpeak, signed short *maxpeak);
void amplify16(short* PCM, ulong Len);
void amplify16_v2(short* PCM, ulong Len);


static double ToMillisec(LONGLONG difftick) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return ((double)difftick / (double)freq.QuadPart) * 10000 ;
}

#define LOG                                                     \
    {LARGE_INTEGER CurTick;                                     \
    QueryPerformanceCounter( &CurTick );                        \
    if (!g_pChitin->bDisableWindow) {                           \
        console.write_debug("+%.3g \n",                         \
            ToMillisec(CurTick.QuadPart - gLastTick.QuadPart)); \
    }                                                           \
        gLastTick = CurTick;                                    \
    }

#define ALIGN_NEXT_TO(VA,ALIGNMENT) (((ULONG)VA + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

void static
process_file(short* PCM, short Channels, ulong Len) {
	signed short	mins, maxs;
    //signed short	mins2, maxs2;

    //LARGE_INTEGER gLastTick;
    //QueryPerformanceCounter( &gLastTick );

	if (!getpeaks16(PCM, Len, &mins, &maxs))
	    return;

	//console.write_debug("\rMinimum level found: %d, maximum level found: %d\n", mins, maxs);

	if (mins == -32768)
		mins = -32767;

	if ((-mins) > maxs)
		maxs = -mins;

	if (maxs == 0) {
		console.write_debug("All zero samples found.\n");
		g_ratio = 1;
	} else {
		g_ratio = (32767.0 * g_normpercent) / ((double)maxs * 100.0);
	}

	if (g_ratio == 1) {
        console.write_debug("\tZero attenuationm skipping \n");
		return;
	} else
    if (g_ratio < 1) {
	    //console.write_debug("Performing attenuation of %.03f dB\n", 20.0 * log10(g_ratio));
	} else
    if (g_ratio > 1) {
        if (pGameOptionsEx->bSound_NormalizePrintResname) {
            if (gSoundFileName.IsEmpty())
		        console.writef("\t +%.3f dB", 20.0 * log10(g_ratio));
            else
                gNormalizeAmplLastValue = 20.0 * log10(g_ratio);
        }
	}

    if (g_usemingain) {
        if (fabs(20.0 * log10(g_ratio)) < g_mingain) {
			console.write_debug("Level is smaller than %.03f dB, aborting.\n", g_mingain);
			return;
        }
	}

	//table16 = (signed short*)VirtualAlloc(NULL, 131072, MEM_COMMIT, PAGE_READWRITE);
	//if (table16 == NULL) {
	//    console.write_debug("Cannot allocate translation table in memory.\n");
    //    return;
	//}
	//make_table16();
	//amplify16(PCM, Len);
	//VirtualFree(table16, 0, MEM_RELEASE);

    amplify16_v2(PCM, Len);

    //LOG
}


void static make_table16(void) {
	unsigned short	i = 0;

	do {
		if (((signed short)i * g_ratio) > 32767)
			table16[i] = 32767;
		else if (((signed short)i * g_ratio) < -32767)
			table16[i] = -32767;
		else
			table16[i] = (signed short)(((signed short)i) * g_ratio);
	} while (++i);
}


int static getpeaks16(short* PCM, ulong Len, signed short *minpeak, signed short *maxpeak) {
	unsigned long				i, ndone = 0;
	register signed short		minp = 0, maxp = 0, cur;
	unsigned long				*stats;
	unsigned long				numstat;

	if (g_smartpeak) {
		// allocate memory for the sample statistics
		stats = (unsigned long*)VirtualAlloc(NULL, sizeof(unsigned long) * 65536, MEM_COMMIT, PAGE_READWRITE);
		
		if (stats == NULL) {
			console.write_debug("Cannot allocate getpeaks16() buffer in memory.\n");
			return 0;
		}
		memset(stats, 0, 65536);

		numstat = Len >> 1;
        for (i = 0; i < (Len>>1); i++) {
		    cur = PCM[i];
        	stats[32768 + cur]++;
        }
    } else {
        if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {  // SSE2
            ulong PCM_aligned = ALIGN_NEXT_TO(PCM, 16);
            ulong Len_aligned = Len - (PCM_aligned - (ulong)PCM);
            _MM_ALIGN16 short ssemax[8];
            _MM_ALIGN16 short ssemin[8];

            __asm {
                push    eax
                push    esi

                mov     esi,  [PCM_aligned]
                mov     eax,  [Len_aligned]
                shr     eax,  5              // len/32, avoid out of bound
                pxor    xmm2, xmm2
                pxor    xmm3, xmm3

            getpeaks16_loop:
                movaps  xmm0, [esi]         // 8 words
                movaps  xmm1, [esi+16]      // 8 words, total 16 words per loop
                pminsw  xmm2, xmm0          // xmm2 - array of min
                pminsw  xmm2, xmm1
                pmaxsw  xmm3, xmm0          // xmm3 - array of max
                pmaxsw  xmm3, xmm1
                add     esi,  32
                dec     eax
                ja      getpeaks16_loop

                movaps  [ssemax], xmm3
                movaps  [ssemin], xmm2

                pop     esi
                pop     eax
            }

            for (i = 0; i < 8; i++) {
		        cur = ssemax[i];
                if (cur > maxp)
				    maxp = cur;

                cur = ssemin[i];
        	    if (cur < minp)
				    minp = cur;
            }
        } else {    // legacy x86
            for (i = 0; i < (Len>>1); i++) {
		        cur = PCM[i];
        	    if (cur < minp)
				    minp = cur;
			    if (cur > maxp)
				    maxp = cur;
            }
        }
    }

	if (g_smartpeak) {
		// let's find how many samples is <percent> of the max
		numstat = (unsigned long) (numstat * (1.0 - (g_peakpercent / 100.0)));
		// let's use this to accumulate values
		ndone = 0;
		// let's count the min sample value that has the given percentile
		for (i = 0; (i < 65536) && (ndone <= numstat); i++)
			ndone += stats[i];
		minp = (short) (i - 32769);
		// let's count the max sample value that has the given percentile
		ndone = 0;
		for (i = 65535; (i >= 0) && (ndone <= numstat); i--)
			ndone += stats[i];
		maxp = (short) (i - 32767);
		VirtualFree(stats, 0, MEM_RELEASE);
	}

	*minpeak = minp;
	*maxpeak = maxp;

	return 1;
}


inline void
amplify16(short* PCM, ulong Len) {
	unsigned long	i;

    for (i = 0; i < (Len>>1); i++) {
        PCM[i] = table16[(unsigned short)PCM[i]];
	}
}


inline void
amplify16_v2(short* PCM, ulong Len) {
	unsigned long	i;

    if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {  // SSE2
        __asm {
            push    eax
            push    ecx
            push    edx

            mov     ecx, PCM                        // wav buffer
            mov     edx, Len                        // len in bytes
            shr     edx, 2                          // len/4, avoid out of bound
            movsd   xmm1, g_ratio                   // normalizing multiplier
            shufpd  xmm1, xmm1, 00b                 // copy d0 -> d1
            xor     eax, eax                        // index

        amplify16_v2_loop:
            movd      xmm0, dword ptr [ecx+eax*4]   // xx xx xx xx xx xx w1 w0
            punpcklwd xmm0, xmm0                    // xx xx xx xx w1 w1 w0 w0
            psrad     xmm0, 16                      // xx xx xx xx s1 w1 s0 w0, int16 -> int32

            cvtdq2pd  xmm0, xmm0                    // d1 d1 d1 d1 d0 d0 d0 d0, int32 -> double
            mulpd     xmm0, xmm1                    // * ratio
            cvttpd2dq xmm0, xmm0                    // xx xx xx xx s1 w1 s0 w0, double -> int32

            packssdw  xmm0, xmm0                    // xx xx w1 w0 xx xx w1 w0
            movd      dword ptr [ecx+eax*4], xmm0   // xx xx xx xx xx xx w1 w0
            inc       eax
            cmp       eax, edx
            jb        amplify16_v2_loop

            pop     edx
            pop     ecx
            pop     eax

        }
    } else {    // legacy FPU
        for (i = 0; i < (Len>>1); i++) {
            PCM[i] = (short) (PCM[i] * g_ratio);
	    }
    }
}


void
Normalize(short* PCM, unsigned long Len, short Channels) {
 
    // signed 16bit pcm only
    g_normpercent = 100.0;
    g_peakpercent = 100.0;
    g_smartpeak = 0;      // smartpeak: count as a peak only a signal that has the given percentile (50%%-100%%)
    g_usemingain = 0;     // abort if gain increase is smaller than <mingain> (in dB)
    g_mingain = 0.5;

    if (g_smartpeak) {
        g_peakpercent = 99.99;

	    if (g_peakpercent < 50.0)
		    g_peakpercent = 50.0;
    }

	// this way the percentile peak is amplified to the correct level
	if (g_smartpeak)
		g_normpercent *= g_peakpercent / 100.0;

	process_file(PCM, Channels, Len);
}