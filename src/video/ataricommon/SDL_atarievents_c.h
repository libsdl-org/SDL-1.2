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

/*
 *	Atari keyboard events manager
 *
 *	Patrice Mandin
 */

#ifndef _SDL_ATARI_EVENTS_H_
#define _SDL_ATARI_EVENTS_H_

#include "SDL_keyboard.h"
#include "../SDL_sysvideo.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this

#define ATARIBIOS_MAXKEYS 128

#define ATARI_KEY_PRESSED	0xff
#define ATARI_KEY_UNDEFINED	0x80
#define ATARI_KEY_RELEASED	0x00

#define ATARI_JOY_UP	(1<<0)
#define ATARI_JOY_DOWN	(1<<1)
#define ATARI_JOY_LEFT	(1<<2)
#define ATARI_JOY_RIGHT	(1<<3)
#define ATARI_JOY_FIRE	(1<<7)

/* State filled in by the IKBD or XBIOS interrupt vectors; only one of the
   two drivers can be installed at a time */
extern SDL_bool SDL_Atari_vectors_installed;

extern volatile Uint8  SDL_Atari_keyboard[ATARIBIOS_MAXKEYS];
extern volatile Uint16 SDL_Atari_mouseb;
extern volatile Sint16 SDL_Atari_mousex;
extern volatile Sint16 SDL_Atari_mousey;
extern volatile Uint8  SDL_Atari_joystick;

typedef enum {
	ATARI_EVENTS_INVALID = -1,
	ATARI_EVENTS_IKBD,
	ATARI_EVENTS_XBIOS
} SDL_AtariEventsDriver;

/* The driver SDL_ATARI_EVENTSDRIVER asks for, or the default for the
   machine; sets the SDL error when the machine cannot provide it */
extern SDL_AtariEventsDriver SDL_Atari_GetEventsDriver(void);

extern void SDL_Atari_InitializeConsoleSettings(void);
extern void SDL_Atari_RestoreConsoleSettings(void);

extern void SDL_Atari_InitInternalKeymap(_THIS);

extern void SDL_AtariMint_BackgroundTasks(void);

SDL_keysym *SDL_Atari_TranslateKey(int scancode, SDL_keysym *keysym,
	SDL_bool pressed, short kstate);

extern void SDL_Atari_InstallVectors(void (*install)(void), void (*restore)(void));
extern void SDL_Atari_RestoreVectors(void);

extern void SDL_Atari_PostKeyboardEvents(_THIS);
extern void SDL_Atari_PostMouseEvents(_THIS, SDL_bool buttonEvents);

#endif /* _SDL_ATARI_EVENTS_H_ */
