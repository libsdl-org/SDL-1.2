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

#ifndef _SDL_shifter_h
#define _SDL_shifter_h

#include "SDL_xbios.h"

/* Private display data */

extern volatile Uint32 SDL_Shifter_vblcount;	/* VBLs so far, only the handler writes it */
extern volatile Uint32 SDL_Shifter_pending;	/* video base address for the VBL handler to set, 0 if none */
extern volatile Uint16 SDL_Shifter_palcount;	/* colours to write, 0 if none */
extern Uint16 SDL_Shifter_palfirst;

extern void (*SDL_Shifter_vbl)(void);

/* TRAP #12, which tos.hyp lists no users of */
#define SHIFTER_WAIT_VECTOR	(0x80 + 12 * 4)

extern void SDL_Shifter_Wait(void);

/* Returns once SDL_Shifter_vblcount differs from vbl, with its value */
static __inline__ Uint32 SDL_Shifter_WaitVbl(Uint32 vbl)
{
	register Uint32 count __asm__("d0") = vbl;
	register Uint32 caller __asm__("d1") = 0x4c53444cUL;	/* 'LSDL' */

	__asm__ volatile ("trap #12" : "+d" (count) : "d" (caller) : "memory");

	return count;
}

/*--- Functions prototypes ---*/

/* SDL_shifter_st.c */
void SDL_SHIFTER_VideoInit_ST(_THIS);

/* SDL_shifter_tt.c */
void SDL_SHIFTER_VideoInit_TT(_THIS);

/* SDL_shifter_f30.c */
SDL_bool SDL_SHIFTER_F30Available(void);
void SDL_SHIFTER_VideoInit_F30(_THIS);

#endif /* _SDL_shifter_h */
