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
#include "SDL_atariqueue.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this

#define ATARIBIOS_MAXKEYS 128

#define ATARI_JOY_UP	(1<<0)
#define ATARI_JOY_DOWN	(1<<1)
#define ATARI_JOY_LEFT	(1<<2)
#define ATARI_JOY_RIGHT	(1<<3)
#define ATARI_JOY_FIRE	(1<<7)

/* State filled in by the IKBD or XBIOS interrupt vectors; only one of the
   two drivers can be installed at a time */
extern SDL_bool SDL_Atari_vectors_installed;

extern volatile Uint16 SDL_Atari_mouseb;			/* live button state, for the joystick */
extern volatile Uint8  SDL_Atari_joystick;
extern volatile Uint32 SDL_Atari_queue[ATARI_QUEUE_SIZE];	/* key and mouse events in order */
extern volatile Uint32 SDL_Atari_queue_head;

typedef enum {
	ATARI_EVENTS_INVALID = -1,
	ATARI_EVENTS_IKBD,
	ATARI_EVENTS_XBIOS,
	ATARI_EVENTS_GEM
} SDL_AtariEventsDriver;

/* The driver SDL_ATARI_EVENTSDRIVER asks for, or the default for the video
   driver; sets the SDL error when the machine or the video driver cannot
   provide it */
extern SDL_AtariEventsDriver SDL_Atari_GetEventsDriver(SDL_bool gemVideo);

extern void SDL_Atari_InitializeConsoleSettings(void);
extern void SDL_Atari_RestoreConsoleSettings(void);

extern void SDL_Atari_InitInternalKeymap(_THIS);

extern void SDL_AtariMint_BackgroundTasks(void);

SDL_keysym *SDL_Atari_TranslateKey(int scancode, SDL_keysym *keysym,
	SDL_bool pressed, short kstate);

extern void SDL_Atari_InstallVectors(void (*install)(void), void (*restore)(void));
extern void SDL_Atari_RestoreVectors(void);

/* Drain the queue. Queued motion is posted only when relativeMotion is set,
   mouseFocus (if given) is asked right before a button press is posted */
extern void SDL_Atari_PostEvents(_THIS, SDL_bool relativeMotion, SDL_bool (*mouseFocus)(_THIS));

#endif /* _SDL_ATARI_EVENTS_H_ */
