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

#include <mint/cookie.h>
#include <mint/osbind.h>

#include "../ataricommon/SDL_atarimch.h"

#include "SDL_shifter.h"

extern void SDL_Shifter_VblF30(void);

volatile Uint32 SDL_Shifter_pending;	/* video base address for the VBL handler to set, 0 if none */
Uint32 SDL_Shifter_paletteF30[256];	/* read by the VBL handler */

/* See SDL_XBIOS_VideoInit_F30() */
SDL_bool SDL_SHIFTER_F30Available(void)
{
	long cookie_dummy;

	if ((SDL_Atari_GetMch() >> 16) == MCH_V4) {
		return SDL_FALSE;
	}

	/* FireBee under FireTOS or EmuTOS */
	if (Getcookie(C__CF_, NULL) == C_FOUND) {
		return SDL_FALSE;
	}

	/* CTPCI booted on the Radeon */
	if ((Getcookie(C_CT60, &cookie_dummy) == C_FOUND)
	    && (Getcookie(C__PCI, &cookie_dummy) == C_FOUND)
	    && ((unsigned long)Physbase()>=0x01000000UL))
	{
		return SDL_FALSE;
	}

	return SDL_TRUE;
}

static void flipVbuffers(_THIS)
{
	SDL_Shifter_pending = (Uint32) XBIOS_screens[XBIOS_fbnum];

	/* The other buffer is free once the handler has written this one */
	while (SDL_Shifter_pending) {
		/* Vsync() prevents the OS from preempting */
		XBIOS_vsync(this);
	}
}

static int setColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	const int count = SDL_Shifter_palcount;
	int first = firstcolor;
	int last = firstcolor + ncolors;
	int i, r,g,b;

	/* Stops the handler from writing a partly copied palette */
	SDL_Shifter_palcount = 0;

	for(i = 0; i < ncolors; i++) {
		r = colors[i].r;
		g = colors[i].g;
		b = colors[i].b;

		/* Videl palette registers: red, green, unused and blue bytes */
		SDL_Shifter_paletteF30[firstcolor+i]=((Uint32)r<<24)|(g<<16)|b;
	}

	if (count) {
		first = SDL_min(first, SDL_Shifter_palfirst);
		last = SDL_max(last, SDL_Shifter_palfirst + count);
	}
	SDL_Shifter_palfirst = first;
	SDL_Shifter_palcount = last - first;

	return (1);
}

void SDL_SHIFTER_VideoInit_F30(_THIS)
{
	SDL_Shifter_vbl = SDL_Shifter_VblF30;

	XBIOS_flipVbuffers = flipVbuffers;
	this->SetColors = setColors;
}
