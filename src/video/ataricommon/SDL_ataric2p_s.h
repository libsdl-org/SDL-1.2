/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#ifndef _ATARI_C2P_h
#define _ATARI_C2P_h

#include "SDL_stdinc.h"

/*--- 8 bits functions ---*/

/* Convert a chunky screen to bitplane screen */

/* Optimized for surface-to-surface copy with the same pitch */
void SDL_Atari_C2pConvert8(
	const Uint8 *src,	/* Source screen (one byte=one pixel) */
	const Uint8 *srcend,/* Source screen end (past-the-end position) */
	Uint8 *dest			/* Destination (8 bits planes) */
);

/* Optimized for surface-to-surface copy with the double screen pitch (typically 320x480) */
void SDL_Atari_C2pConvert8_tt(
	const Uint8 *src,	/* Source screen (one byte=one pixel) */
	const Uint8 *srcend,/* Source screen end (past-the-end position) */
	Uint8 *dest,		/* Destination (8 bits planes) */
	Uint32 screenPitch	/* Length of one destination double-line in bytes */
);

/* Optimized for arbitrary rectangle position and dimension (16px aligned) */
void SDL_Atari_C2pConvert8_rect(
	const Uint8 *src,	/* Source screen (one byte=one pixel) at rectangle's [X1, Y1] position */
	const Uint8 *srcend,/* Source screen end at rectangle's [X2, Y2] position (included) */
	Uint32 srcwidth,	/* X2-X1 */
	Uint32 srcpitch,	/* Length of one source line in bytes */
	Uint8 *dest,		/* Destination (8 bits planes) at rectangle's [X1, Y1] position */
	Uint32 dstpitch		/* Length of one destination line in bytes */
);

/*--- 4 bits functions ---*/

/* Convert a chunky screen to bitplane screen */

/* Optimized for surface-to-surface copy with the same pitch */
void SDL_Atari_C2pConvert4(
	const Uint8 *src,	/* Source screen (one byte=one pixel) */
	const Uint8 *srcend,/* Source screen end (past-the-end position) */
	Uint8 *dest			/* Destination (4 bits planes) */
);

/* Optimized for arbitrary rectangle position and dimension (16px aligned) */
void SDL_Atari_C2pConvert4_rect(
	const Uint8 *src,	/* Source screen (one byte=one pixel) at rectangle's [X1, Y1] position */
	const Uint8 *srcend,/* Source screen end at rectangle's [X2, Y2] position (included) */
	Uint32 srcwidth,	/* X2-X1 */
	Uint32 srcpitch,	/* Length of one source line in bytes */
	Uint8 *dest,		/* Destination (4 bits planes) at rectangle's [X1, Y1] position */
	Uint32 dstpitch		/* Length of one destination line in bytes */
);

/*--- Functions ---*/

/* Wrapper for the functions above */
static __inline__ void SDL_Atari_C2pConvert(
	Uint8 *src,			/* Source screen (one byte=one pixel) */
	Uint8 *dest,		/* Destination (4/8 bits planes) */
	Uint32 x,			/* Coordinates of screen to convert */
	Uint32 y,
	Uint32 width,		/* Dimensions of screen to convert */
	Uint32 height,
	Uint32 dblline,		/* Double the lines when converting ? */
	Uint32 bits,		/* Bits per pixel */
	Uint32 srcpitch,	/* Length of one source line in bytes */
	Uint32 dstpitch)	/* Length of one destination line in bytes */
{
	dstpitch <<= dblline;

	const Uint8 *chunkybeg = src + y * srcpitch + x;
	const Uint8 *chunkyend = src + (y + height - 1) * srcpitch + x + width;

	Uint8 *screen = dest + y * dstpitch + x * bits/8;

	if (bits == 8) {
		if (srcpitch == width) {
			if (srcpitch == dstpitch) {
				SDL_Atari_C2pConvert8(chunkybeg, chunkyend, screen);
				return;
			} else if (srcpitch == dstpitch/2) {
				SDL_Atari_C2pConvert8_tt(chunkybeg, chunkyend, screen, dstpitch);
				return;
			}
		}

		SDL_Atari_C2pConvert8_rect(
			chunkybeg, chunkyend,
			width,
			srcpitch,
			screen,
			dstpitch);
	} else {
		if (srcpitch == width && srcpitch/2 == dstpitch) {
			SDL_Atari_C2pConvert4(chunkybeg, chunkyend, screen);
			return;
		}

		SDL_Atari_C2pConvert4_rect(
			chunkybeg, chunkyend,
			width,
			srcpitch,
			screen,
			dstpitch);
	}
}

#endif /* _ATARI_C2P_h */
