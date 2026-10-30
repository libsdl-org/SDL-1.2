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

extern volatile Uint16 SDL_Shifter_palcount;	/* colours to write, 0 if none */
extern Uint16 SDL_Shifter_palfirst;

extern void (*SDL_Shifter_vbl)(void);

/*--- Functions prototypes ---*/

/* SDL_shifter_st.c */
void SDL_SHIFTER_VideoInit_ST(_THIS);

/* SDL_shifter_tt.c */
void SDL_SHIFTER_VideoInit_TT(_THIS);

/* SDL_shifter_f30.c */
SDL_bool SDL_SHIFTER_F30Available(void);
void SDL_SHIFTER_VideoInit_F30(_THIS);

#endif /* _SDL_shifter_h */
