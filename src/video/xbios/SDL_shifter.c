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
 *	Atari video hardware driver: the xbios driver with flips and palette
 *	done by its own VBL handler to avoid OS overhead.
 */

#include <mint/cookie.h>
#include <mint/osbind.h>

#include "../ataricommon/SDL_atarimxalloc_c.h"
#include "../ataricommon/SDL_atariprocterm_c.h"
#include "../ataricommon/SDL_atarixbra_c.h"

#include "SDL_shifter.h"

#define SHIFTER_VID_DRIVER_NAME "shifter"

#define VBL_VECTOR	0x70

volatile Uint16 SDL_Shifter_palcount;
Uint16 SDL_Shifter_palfirst;

void (*SDL_Shifter_vbl)(void);

static long cookie_vdo;

static int (*xbios_allocVbuffers)(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize);
static void (*xbios_freeVbuffers)(_THIS);
static void (*xbios_restoreMode)(_THIS);

static void HookVbl(void)
{
	XBRA *xbra = (XBRA *)((Uint32)SDL_Shifter_vbl - sizeof(XBRA));

	xbra->oldvec = *(volatile XbraHandler *)VBL_VECTOR;
	*(volatile XbraHandler *)VBL_VECTOR = SDL_Shifter_vbl;
}

static void UnhookVbl(void)
{
	volatile XbraHandler *place = (volatile XbraHandler *)VBL_VECTOR;

	while (*place != SDL_Shifter_vbl) {
		XBRA *xbra = (XBRA *)((Uint32)*place - sizeof(XBRA));

		if (xbra->xbra_id != XBRA_ID)
			return;
		place = &xbra->oldvec;
	}

	*place = ((XBRA *)((Uint32)SDL_Shifter_vbl - sizeof(XBRA)))->oldvec;
}

static int SHIFTER_Available(void)
{
	if (!XBIOS_bootstrap.available() || Getcookie(C_NOVA, NULL) == C_FOUND) {
		return 0;
	}

	if (Getcookie(C__VDO, &cookie_vdo) != C_FOUND) {
		cookie_vdo = VDO_ST << 16;
	}

	switch (cookie_vdo >> 16) {
		case VDO_ST:
		case VDO_STE:
		case VDO_TT:
			break;
		case VDO_F30:
			if (!SDL_SHIFTER_F30Available()) {
				return 0;
			}
			break;
		default:
			return 0;
	}

	return Atari_CheckAccess((void *) &SDL_Shifter_palcount, sizeof(SDL_Shifter_palcount));
}

static int allocVbuffers(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize)
{
	if (!xbios_allocVbuffers(this, new_video_mode, num_buffers, bufsize)) {
		return 0;
	}

	SDL_Shifter_palcount = 0;

	if (!SDL_Atari_AddProcterm(UnhookVbl)) {
		SDL_SetError("Can not install the VBL handler");
		return 0;
	}

	Supexec(HookVbl);

	return 1;
}

static void StopVbl(void)
{
	if (SDL_Atari_RemoveProcterm(UnhookVbl)) {
		Supexec(UnhookVbl);
	}
}

static void freeVbuffers(_THIS)
{
	StopVbl();

	xbios_freeVbuffers(this);
}

static void restoreMode(_THIS)
{
	StopVbl();

	xbios_restoreMode(this);
}

static SDL_VideoDevice *SHIFTER_CreateDevice(int devindex)
{
	SDL_VideoDevice *this = XBIOS_bootstrap.create(devindex);

	if (this == NULL) {
		return NULL;
	}

	xbios_allocVbuffers = XBIOS_allocVbuffers;
	xbios_freeVbuffers = XBIOS_freeVbuffers;
	xbios_restoreMode = XBIOS_restoreMode;

	XBIOS_allocVbuffers = allocVbuffers;
	XBIOS_freeVbuffers = freeVbuffers;
	XBIOS_restoreMode = restoreMode;

	SDL_SHIFTER_VideoInit_ST(this);

	switch (cookie_vdo >> 16) {
		case VDO_TT:
			SDL_SHIFTER_VideoInit_TT(this);
			break;
		case VDO_F30:
			SDL_SHIFTER_VideoInit_F30(this);
			break;
	}

	return this;
}

VideoBootStrap SHIFTER_bootstrap = {
	SHIFTER_VID_DRIVER_NAME, "Atari video hardware driver",
	SHIFTER_Available, SHIFTER_CreateDevice
};
