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
 *	Atari keyboard events manager, using hardware IKBD
 *
 *	Patrice Mandin
 */

/* Mint includes */
#include <mint/cookie.h>
#include <mint/mintbind.h>
#include <mint/osbind.h>

#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"

#include "SDL_atarikeys.h"
#include "SDL_atarievents_c.h"
#include "SDL_ikbdinterrupt_s.h"

#define KEY_PRESSED		0xff
#define KEY_UNDEFINED	0x80
#define KEY_RELEASED	0x00

static Uint16 atari_prevmouseb;	/* save state of mouse buttons */

static int GetButton(int button)
{
	switch(button)
	{
		case 0:
			return SDL_BUTTON_RIGHT;
			break;
		case 1:
		default:
			return SDL_BUTTON_LEFT;
			break;
	}
}

static SDL_bool CheckAccess(const void *addr, size_t length)
{
	Uint32 flags;

	if (Getcookie(C_MiNT, NULL) != C_FOUND)
		return SDL_TRUE;

	if (Mvalidate(0, addr, length, &flags) < 0)
		return SDL_FALSE;

	if (((flags+0x10)&0xf0) != MX_SUPERVISOR && ((flags+0x10)&0xf0) != MX_GLOBAL)
		return SDL_FALSE;

	return SDL_TRUE;
}

void AtariIkbd_InitOSKeymap(_THIS)
{
	SDL_memset((void *) SDL_AtariIkbd_keyboard, KEY_UNDEFINED, sizeof(SDL_AtariIkbd_keyboard));

	/* Now install our handler */
	SDL_AtariIkbd_mouseb = SDL_AtariIkbd_mousex = SDL_AtariIkbd_mousey = 0;
	atari_prevmouseb = 0;

	if (!CheckAccess((void *)&SDL_AtariIkbd_keyboard, sizeof(SDL_AtariIkbd_keyboard))) {
		fprintf(stderr, "Insufficient privileges to install IKBD vectors. Set application's PRGFLAGS to Super.\n");
		return;
	}

	Supexec(SDL_AtariIkbd_Install);
	Setexc(VEC_PROCTERM, SDL_AtariIkbd_Restore);
}

void AtariIkbd_PumpEvents(_THIS)
{
	int i;
	SDL_keysym keysym;
	static short shiftstate;

	SDL_AtariMint_BackgroundTasks();

	if (!SDL_AtariIkbd_enabled)
		return;

	/*--- Send keyboard events ---*/

	for (i=0; i<ATARIBIOS_MAXKEYS; i++) {
		/* Key pressed ? */
		if (SDL_AtariIkbd_keyboard[i]==KEY_PRESSED) {
			switch (i) {
			case SCANCODE_LEFTSHIFT:
				shiftstate |= K_LSHIFT;
				break;
			case SCANCODE_RIGHTSHIFT:
				shiftstate |= K_RSHIFT;
				break;
			case SCANCODE_LEFTCONTROL:
				shiftstate |= K_CTRL;
				break;
			case SCANCODE_LEFTALT:
				shiftstate |= K_ALT;
				break;
			case SCANCODE_CAPSLOCK:
				shiftstate |= K_CAPSLOCK;
				break;
			case SCANCODE_ALTGR:
				shiftstate |= 0x80;
				break;
			}

			SDL_PrivateKeyboard(SDL_PRESSED,
				SDL_Atari_TranslateKey(i, &keysym, SDL_TRUE, shiftstate));
			if (i == SCANCODE_CAPSLOCK) {
				/* Pressed capslock: generate a release event, too because this
				 * is what SDL expects; it handles locking by itself.
				 */
				SDL_PrivateKeyboard(SDL_RELEASED,
					SDL_Atari_TranslateKey(i, &keysym, SDL_FALSE, shiftstate & ~K_CAPSLOCK));
			}
			SDL_AtariIkbd_keyboard[i]=KEY_UNDEFINED;
		}

		/* Key released ? */
		if (SDL_AtariIkbd_keyboard[i]==KEY_RELEASED) {
			switch (i) {
			case SCANCODE_LEFTSHIFT:
				shiftstate &= ~K_LSHIFT;
				break;
			case SCANCODE_RIGHTSHIFT:
				shiftstate &= ~K_RSHIFT;
				break;
			case SCANCODE_LEFTCONTROL:
				shiftstate &= ~K_CTRL;
				break;
			case SCANCODE_LEFTALT:
				shiftstate &= ~K_ALT;
				break;
			case SCANCODE_CAPSLOCK:
				shiftstate &= ~K_CAPSLOCK;
				break;
			case SCANCODE_ALTGR:
				shiftstate &= ~0x80;
				break;
			}

			if (i == SCANCODE_CAPSLOCK) {
				/* Released capslock: generate a pressed event, too because this
				 * is what SDL expects; it handles locking by itself.
				 */
				SDL_PrivateKeyboard(SDL_PRESSED,
					SDL_Atari_TranslateKey(i, &keysym, SDL_TRUE, shiftstate | K_CAPSLOCK));
			}
			SDL_PrivateKeyboard(SDL_RELEASED,
				SDL_Atari_TranslateKey(i, &keysym, SDL_FALSE, shiftstate));
			SDL_AtariIkbd_keyboard[i]=KEY_UNDEFINED;
		}
	}

	/*--- Send mouse events ---*/

	/* Mouse motion ? */
	if (SDL_AtariIkbd_mousex || SDL_AtariIkbd_mousey) {
		SDL_PrivateMouseMotion(0, 1, SDL_AtariIkbd_mousex, SDL_AtariIkbd_mousey);
		SDL_AtariIkbd_mousex = SDL_AtariIkbd_mousey = 0;
	}

	/* Mouse button ? */
	if (SDL_AtariIkbd_mouseb != atari_prevmouseb) {
		for (i=0;i<2;i++) {
			int curbutton, prevbutton;

			curbutton = SDL_AtariIkbd_mouseb & (1<<i);
			prevbutton = atari_prevmouseb & (1<<i);

			if (curbutton && !prevbutton) {
				SDL_PrivateMouseButton(SDL_PRESSED, GetButton(i), 0, 0);
			}
			if (!curbutton && prevbutton) {
				SDL_PrivateMouseButton(SDL_RELEASED, GetButton(i), 0, 0);
			}
		}
		atari_prevmouseb = SDL_AtariIkbd_mouseb;
	}
}

void AtariIkbd_ShutdownEvents(_THIS)
{
	if (!SDL_AtariIkbd_enabled)
		return;

	Supexec(SDL_AtariIkbd_Restore);
}
