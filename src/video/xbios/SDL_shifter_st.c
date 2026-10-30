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

extern void SDL_Shifter_VblST(void);

volatile Uint32 SDL_Shifter_counter;	/* video counter at the last VBL, also for the TT */

static void flipVbuffers(_THIS)
{
	const Uint32 screen = (Uint32) XBIOS_screens[XBIOS_fbnum];
	const Uint32 screensize = XBIOS_pitch * XBIOS_current->height;

	XBIOS_swapVbuffers(this);

	/* The other buffer is free once the video counter lies in this one */
	while (SDL_Shifter_counter - screen >= screensize) {
		XBIOS_vsync(this);
	}
}

void SDL_SHIFTER_VideoInit_ST(_THIS)
{
	SDL_Shifter_vbl = SDL_Shifter_VblST;

	XBIOS_flipVbuffers = flipVbuffers;
}
