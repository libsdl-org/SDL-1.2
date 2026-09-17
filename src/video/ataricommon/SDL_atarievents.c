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
 *
 *	These routines choose what the final event manager will be
 */

#include <mint/cookie.h>
#include <mint/mintbind.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>

#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"
#include "../../timer/SDL_timer_c.h"

#include "SDL_atarikeys.h"
#include "SDL_atarimch.h"
#include "SDL_atarievents_c.h"
#include "SDL_xbiosevents_c.h"

/* from src/audio/mint/SDL_mintaudio.c */
void SDL_AtariMint_UpdateAudio(void);
/* from src/timer/mint/SDL_systimer.c */
#ifdef SDL_TIMER_MINT
void SDL_AtariMint_CheckTimer(void);
#endif

/* Variables */

SDL_bool SDL_Atari_vectors_installed;

/* Written by the interrupt handlers, see SDL_atariqueue.h */
volatile Uint16 SDL_Atari_mouseb;
volatile Uint8  SDL_Atari_joystick;
volatile Uint32 SDL_Atari_queue[ATARI_QUEUE_SIZE];
volatile Uint32 SDL_Atari_queue_head;

/* Local variables */

/* The translation tables from a console scancode to a SDL keysym */
static SDLKey keymap[ATARIBIOS_MAXKEYS];
static const char *keytab_normal;
static const char *keytab_shift;
static const char *keytab_caps;

static SDL_bool conterm_set;
static char old_conterm;

static Uint32 queue_tail;
static Uint16 prev_mouseb;	/* buttons of the previous packet */
static int pending_mousex, pending_mousey;
static short kstate;

static SDL_VideoDevice *pump_device;
static SDL_bool pump_relative_motion;
static SDL_bool (*pump_mouse_focus)(_THIS);
static void (*old_procterm)(void);
static void (*restore_vectors)(void);

/* Functions */

static int GetButton(int button)
{
	switch(button) {
		case 0:
			return SDL_BUTTON_RIGHT;
		case 2:
			return SDL_BUTTON_MIDDLE;
		case 1:
		default:
			return SDL_BUTTON_LEFT;
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

static SDL_bool IsIkbdSupported(void)
{
	long cookie_mch = SDL_Atari_GetMch();

	/* The IKBD driver talks to the keyboard chip directly, which only
	   Atari hardware and its emulators provide */
	return (cookie_mch == MCH_ST<<16) || ((cookie_mch>>16) == MCH_STE) ||
	       (cookie_mch == MCH_TT<<16) || (cookie_mch == MCH_F30<<16) ||
	       (cookie_mch == MCH_ARANYM<<16);
}

SDL_AtariEventsDriver SDL_Atari_GetEventsDriver(SDL_bool gemVideo)
{
	const char *envr = SDL_getenv("SDL_ATARI_EVENTSDRIVER");

	if (!envr) {
		if (gemVideo) {
			return ATARI_EVENTS_GEM;
		}
		if (SDL_AtariXbios_IsKeyboardVectorSupported()) {
			return ATARI_EVENTS_XBIOS;
		}
		if (IsIkbdSupported()) {
			/* TOS 1.x */
			return ATARI_EVENTS_IKBD;
		}
		SDL_SetError("No keyboard driver available for this machine");
		return ATARI_EVENTS_INVALID;
	}

	if (SDL_strcmp(envr, "ikbd") == 0) {
		if (gemVideo) {
			SDL_SetError("IKBD events driver requires the XBIOS video driver");
			return ATARI_EVENTS_INVALID;
		}
		if (!IsIkbdSupported()) {
			SDL_SetError("IKBD events driver requires Atari hardware");
			return ATARI_EVENTS_INVALID;
		}
		return ATARI_EVENTS_IKBD;
	}

	if (SDL_strcmp(envr, "xbios") == 0) {
		if (!SDL_AtariXbios_IsKeyboardVectorSupported()) {
			SDL_SetError("XBIOS events driver requires TOS 2.0 or MagiC");
			return ATARI_EVENTS_INVALID;
		}
		return ATARI_EVENTS_XBIOS;
	}

	if (SDL_strcmp(envr, "gem") == 0) {
		if (!gemVideo) {
			SDL_SetError("GEM events driver requires the GEM video driver");
			return ATARI_EVENTS_INVALID;
		}
		return ATARI_EVENTS_GEM;
	}

	SDL_SetError("Unknown events driver '%s'", envr);
	return ATARI_EVENTS_INVALID;
}

void SDL_Atari_InstallVectors(void (*install)(void), void (*restore)(void))
{
	SDL_Atari_mouseb = 0;
	SDL_Atari_joystick = 0;
	SDL_Atari_queue_head = 0;

	queue_tail = 0;
	prev_mouseb = 0;
	pending_mousex = pending_mousey = 0;

	kstate = Kbshift(-1) & (K_RSHIFT | K_LSHIFT | K_CTRL | K_ALT | K_CAPSLOCK | K_ALTGR);

	/* All the vectors write into this module's variables */
	if (!CheckAccess((void *) SDL_Atari_queue, sizeof(SDL_Atari_queue))) {
		fprintf(stderr, "Insufficient privileges to install interrupt vectors. Set application's PRGFLAGS to Super.\n");
		return;
	}

	Supexec(install);

	restore_vectors = restore;
	old_procterm = Setexc(VEC_PROCTERM, restore);

	SDL_Atari_vectors_installed = SDL_TRUE;
}

void SDL_Atari_RestoreVectors(void)
{
	if (SDL_Atari_vectors_installed) {
		Supexec(restore_vectors);
		SDL_Atari_vectors_installed = SDL_FALSE;
	}

	if (old_procterm != NULL) {
		Setexc(VEC_PROCTERM, old_procterm);
		old_procterm = NULL;
	}
}

/* Motion is posted before the next button or key event so ordering holds */
static void FlushMotion(void)
{
	if (pending_mousex == 0 && pending_mousey == 0) {
		return;
	}

	if (pump_relative_motion && (SDL_GetAppState() & SDL_APPMOUSEFOCUS)) {
		SDL_PrivateMouseMotion(0, 1, pending_mousex, pending_mousey);
	}
	pending_mousex = pending_mousey = 0;
}

static void UpdateModifiers(int scancode, SDL_bool pressed)
{
	short bit;

	switch (scancode) {
		case SCANCODE_LEFTSHIFT:
			bit = K_LSHIFT;
			break;
		case SCANCODE_RIGHTSHIFT:
			bit = K_RSHIFT;
			break;
		case SCANCODE_LEFTCONTROL:
			bit = K_CTRL;
			break;
		case SCANCODE_LEFTALT:
			bit = K_ALT;
			break;
		case SCANCODE_ALTGR:
			bit = K_ALTGR;
			break;
		case SCANCODE_CAPSLOCK:
			if (pressed) {
				kstate ^= K_CAPSLOCK;
			}
			return;
		default:
			return;
	}

	if (pressed) {
		kstate |= bit;
	} else {
		kstate &= ~bit;
	}
}

static void PostKey(int scancode, SDL_bool pressed)
{
	SDL_keysym keysym;

	/* Presses belong to the focused application; a release for a press
	   SDL never saw is dropped by SDL itself */
	if (pressed && !(SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
		return;
	}

	FlushMotion();

	UpdateModifiers(scancode, pressed);
	SDL_PrivateKeyboard(pressed ? SDL_PRESSED : SDL_RELEASED,
		SDL_Atari_TranslateKey(scancode, &keysym, pressed, kstate));
}

static SDL_bool MouseInFocus(void)
{
	if (pump_mouse_focus) {
		return pump_mouse_focus(pump_device);
	}
	return (SDL_GetAppState() & SDL_APPMOUSEFOCUS) != 0;
}

/* Transitions are found against the previous packet but posted against
   what SDL believes: a press skipped for focus produces no release later,
   and a release lost to a queue overrun is caught up on the next change */
static void PostButtons(Uint16 buttons)
{
	Uint8 sdl_buttons;
	int i;

	if (buttons == prev_mouseb) {
		return;
	}

	FlushMotion();

	sdl_buttons = SDL_GetMouseState(NULL, NULL);
	for (i=0; i<3; i++) {
		Uint16 bit = 1<<i;
		int button = GetButton(i);

		if ((buttons & bit) && !(prev_mouseb & bit)) {
			if (!(sdl_buttons & SDL_BUTTON(button)) && MouseInFocus()) {
				SDL_PrivateMouseButton(SDL_PRESSED, button, 0, 0);
			}
		} else if (!(buttons & bit) && (sdl_buttons & SDL_BUTTON(button))) {
			SDL_PrivateMouseButton(SDL_RELEASED, button, 0, 0);
		}
	}

	prev_mouseb = buttons;
}

static void ProcessMouse(Uint32 entry)
{
	Sint8 dx = (Sint8) ((entry >> 8) & 0xff);
	Sint8 dy = (Sint8) (entry & 0xff);

	/* Keep the sum within what a motion event carries */
	if (pending_mousex > 32000 || pending_mousex < -32000
	    || pending_mousey > 32000 || pending_mousey < -32000) {
		FlushMotion();
	}
	pending_mousex += dx;
	pending_mousey += dy;

	PostButtons((entry >> 16) & 0xff);
}

void SDL_Atari_PostEvents(_THIS, SDL_bool relativeMotion, SDL_bool (*mouseFocus)(_THIS))
{
	if (!SDL_Atari_vectors_installed) {
		return;
	}

	pump_device = this;
	pump_relative_motion = relativeMotion;
	pump_mouse_focus = mouseFocus;

	for (;;) {
		Uint32 head = SDL_Atari_queue_head;
		Uint32 entry;

		if (head == queue_tail) {
			break;
		}
		/* The handlers overwrote what was not consumed in time */
		if (head - queue_tail > ATARI_QUEUE_SIZE) {
			queue_tail = head - ATARI_QUEUE_SIZE;
		}

		entry = SDL_Atari_queue[queue_tail & ATARI_QUEUE_MASK];
		/* The slot may have been overwritten while being read */
		if (SDL_Atari_queue_head - queue_tail > ATARI_QUEUE_SIZE) {
			continue;
		}
		queue_tail++;

		switch (entry >> 24) {
			case ATARI_QUEUE_KEY:
				PostKey((entry >> 16) & 0x7f, (entry & (1<<23)) == 0);
				break;
			case ATARI_QUEUE_MOUSE:
				ProcessMouse(entry);
				break;
		}
	}

	FlushMotion();
}

void SDL_Atari_InitializeConsoleSettings(void)
{
	if (!conterm_set) {
		long ssp = Super(SUP_SET);
		old_conterm = *conterm;
		*conterm &= ~((1<<2) | (1<< 0));	/* disable bell and key-click */
		Super(ssp);

		conterm_set = SDL_TRUE;
	}
}
void SDL_Atari_RestoreConsoleSettings(void)
{
	if (conterm_set) {
		long ssp = Super(SUP_SET);
		*conterm = old_conterm;
		Super(ssp);

		conterm_set = SDL_FALSE;
	}
}

/* SDL_Atari_TranslateKey() is used also in GEM events driver */
void SDL_Atari_InitInternalKeymap(_THIS)
{
	int i;
	_KEYTAB *key_tables;

	/* Read system tables for scancode -> ascii translation */
	key_tables = (_KEYTAB *) Keytbl(KT_NOCHANGE, KT_NOCHANGE, KT_NOCHANGE);
	keytab_normal = key_tables->unshift;
	keytab_shift = key_tables->shift;
	keytab_caps = key_tables->caps;

	/* Initialize keymap */
	for ( i=0; i<ATARIBIOS_MAXKEYS; i++ )
		keymap[i] = SDLK_UNKNOWN;

	/* Functions keys */
	for ( i = 0; i<10; i++ ) {
		keymap[SCANCODE_F1 + i] = SDLK_F1+i;
		/* Shift state is handled separately */
		keymap[SCANCODE_SHIFT_F1 + i] = SDLK_F1+i;
	}

	/* Cursor keypad */
	keymap[SCANCODE_HELP] = SDLK_HELP;
	keymap[SCANCODE_UNDO] = SDLK_UNDO;
	keymap[SCANCODE_INSERT] = SDLK_INSERT;
	keymap[SCANCODE_CLRHOME] = SDLK_HOME;
	keymap[SCANCODE_CNTL_HOME] = SDLK_HOME;
	keymap[SCANCODE_UP] = SDLK_UP;
	keymap[SCANCODE_DOWN] = SDLK_DOWN;
	keymap[SCANCODE_RIGHT] = SDLK_RIGHT;
	keymap[SCANCODE_CNTL_RIGHT] = SDLK_RIGHT;
	keymap[SCANCODE_LEFT] = SDLK_LEFT;
	keymap[SCANCODE_CNTL_LEFT] = SDLK_LEFT;

	/* Special keys */
	keymap[SCANCODE_ESCAPE] = SDLK_ESCAPE;
	keymap[SCANCODE_BACKSPACE] = SDLK_BACKSPACE;
	keymap[SCANCODE_TAB] = SDLK_TAB;
	keymap[SCANCODE_ENTER] = SDLK_RETURN;
	keymap[SCANCODE_DELETE] = SDLK_DELETE;
	keymap[SCANCODE_LEFTCONTROL] = SDLK_LCTRL;
	keymap[SCANCODE_LEFTSHIFT] = SDLK_LSHIFT;
	keymap[SCANCODE_RIGHTSHIFT] = SDLK_RSHIFT;
	keymap[SCANCODE_LEFTALT] = SDLK_LALT;
	keymap[SCANCODE_CAPSLOCK] = SDLK_CAPSLOCK;
	keymap[SCANCODE_ALTGR] = SDLK_MODE;
}

/* Atari to Unicode charset translation table */
const static Uint16 SDL_AtariToUnicodeTable[256]={
	/* Standard ASCII characters from 0x00 to 0x7e */
	/* Unicode stuff from 0x7f to 0xff */

	0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
	0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
	0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
	0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
	0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
	0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
	0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
	0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,

	0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
	0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
	0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
	0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
	0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
	0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
	0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
	0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x0394,

	0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,
	0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
	0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,
	0x00FF,0x00D6,0x00DC,0x00A2,0x00A3,0x00A5,0x00DF,0x0192,
	0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,
	0x00BF,0x2310,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
	0x00C3,0x00F5,0x00D8,0x00F8,0x0153,0x0152,0x00C0,0x00C3,
	0x00D5,0x00A8,0x00B4,0x2020,0x00B6,0x00A9,0x00AE,0x2122,

	0x0133,0x0132,0x05D0,0x05D1,0x05D2,0x05D3,0x05D4,0x05D5,
	0x05D6,0x05D7,0x05D8,0x05D9,0x05DB,0x05DC,0x05DE,0x05E0,
	0x05E1,0x05E2,0x05E4,0x05E6,0x05E7,0x05E8,0x05E9,0x05EA,
	0x05DF,0x05DA,0x05DD,0x05E3,0x05E5,0x00A7,0x2038,0x221E,
	0x03B1,0x03B2,0x0393,0x03C0,0x03A3,0x03C3,0x00B5,0x03C4,
	0x03A6,0x0398,0x03A9,0x03B4,0x222E,0x03C6,0x2208,0x2229,
	0x2261,0x00B1,0x2265,0x2264,0x2320,0x2321,0x00F7,0x2248,
	0x00B0,0x2022,0x00B7,0x221A,0x207F,0x00B2,0x00B3,0x00AF
};

SDL_keysym *SDL_Atari_TranslateKey(int scancode, SDL_keysym *keysym,
	SDL_bool pressed, short kstate)
{
	int asciicode = 0;

	/* Set the keysym information */
	keysym->scancode = scancode;
	keysym->mod = KMOD_NONE;	/* set by SDL */
	keysym->sym = keymap[scancode];
	keysym->unicode = 0;

	if (keysym->sym == SDLK_UNKNOWN) {
		const char *keytab;

		if (kstate & (K_LSHIFT | K_RSHIFT))
			keytab = keytab_shift;
		else if (kstate & K_CAPSLOCK)
			keytab = keytab_caps;
		else
			keytab = keytab_normal;

		keysym->sym = (unsigned char) keytab_normal[scancode];

		asciicode = (unsigned char) keytab[scancode];

		if (kstate & K_CTRL) {
			/* This is what TOS does */
			switch (asciicode) {
				case '\r':
					asciicode = 0x0a;	/* ^J */
					break;
				case '2':
					asciicode = 0x00;	/* ^@ */
					break;
				case '6':
					asciicode = 0x1e;	/* ^^ */
					break;
				case '-':
					asciicode = 0x1f;	/* ^_ */
					break;
				default:
					asciicode &= 0x1f;
					break;
			}
		}
	}

	if (SDL_TranslateUNICODE && pressed) {
		keysym->unicode = SDL_AtariToUnicodeTable[asciicode];
	}

	return(keysym);
}

void SDL_AtariMint_BackgroundTasks(void)
{
	SDL_AtariMint_UpdateAudio();
#ifdef SDL_TIMER_MINT
	if (SDL_timer_running) SDL_AtariMint_CheckTimer();
#else
	if (SDL_timer_running) SDL_ThreadedTimerCheck();
#endif
}
