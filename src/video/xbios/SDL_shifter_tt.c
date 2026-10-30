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

#include <mint/osbind.h>

#include "SDL_shifter.h"

extern void SDL_Shifter_VblTT(void);
extern volatile Uint32 SDL_Shifter_counter;

Uint16 SDL_Shifter_paletteTT[256];	/* read by the VBL handler */

static void flipVbuffers(_THIS)
{
	const Uint32 screen = (Uint32) XBIOS_screens[XBIOS_fbnum];
	const int doubleline = (XBIOS_current->flags & XBIOSMODE_DOUBLELINE ? 1 : 0);
	const Uint32 screensize = XBIOS_pitch * (XBIOS_current->height << doubleline);

	XBIOS_swapVbuffers(this);

	/* The other buffer is free once the video counter lies in this one */
	while (SDL_Shifter_counter - screen >= screensize) {
		XBIOS_vsync(this);
	}
}

static int setColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	const int count = SDL_Shifter_palcount;
	int first = firstcolor;
	int last = firstcolor + ncolors;
	int	i, r,g,b;

	/* Stops the handler from writing a partly copied palette */
	SDL_Shifter_palcount = 0;

	for(i = 0; i < ncolors; i++) {
		r = colors[i].r;
		g = colors[i].g;
		b = colors[i].b;

		SDL_Shifter_paletteTT[firstcolor+i]=((r>>4)<<8)|((g>>4)<<4)|(b>>4);
	}

	if (count) {
		first = SDL_min(first, SDL_Shifter_palfirst);
		last = SDL_max(last, SDL_Shifter_palfirst + count);
	}
	SDL_Shifter_palfirst = first;
	SDL_Shifter_palcount = last - first;

	return(1);
}

void SDL_SHIFTER_VideoInit_TT(_THIS)
{
	SDL_Shifter_vbl = SDL_Shifter_VblTT;

	XBIOS_flipVbuffers = flipVbuffers;
	this->SetColors = setColors;
}
