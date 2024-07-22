/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/*
	CTPCI Xbios video functions

	Pawel Goralski
	Patrice Mandin
*/

#include <mint/cookie.h>
#include <mint/falcon.h>

#include "SDL_xbios.h"
#include "SDL_xbios_milan.h"

/*
 * Use predefined, hardcoded table if CTPCI_USE_TABLE is defined
 * else enumerate all non virtual video modes
 */
#define CTPCI_USE_TABLE

/*
 * Blitting directly to screen used to be very slow as TT-RAM->CTPCI
 * burst mode was (?) not working. However speed tests on CTPCI TOS
 * 1.01 Beta 10 and CTPCI_1M firmware shows much better framerates
 * without the shadow buffer.
 */
/* #define ENABLE_CTPCI_SHADOWBUF */

typedef struct {
	Uint16 modecode, width, height;
} predefined_mode_t;

static const predefined_mode_t mode_list[]={
	/*{VERTFLAG|PAL|VGA,320,240},*/	/* falls back to 640x480 */
	{PAL|VGA|COL80,640,480},
	{VESA_600|HORFLAG2|PAL|VGA|COL80,800,600},
	{VESA_768|HORFLAG2|PAL|VGA|COL80,1024,768},
	{VERTFLAG2|HORFLAG|PAL|VGA|COL80,1280,960},
	{VERTFLAG2|VESA_600|HORFLAG2|HORFLAG|PAL|VGA|COL80,1600,1200}
};

static const Uint8 mode_bpp[]={
	8, 16, 32
};

/*--- Variables ---*/

#ifndef CTPCI_USE_TABLE
static int enum_actually_add;
static SDL_VideoDevice *enum_this;
#endif

/*--- Functions ---*/

static void listModes(_THIS, int actually_add);
static void saveMode(_THIS, SDL_PixelFormat *vformat);
static void setMode(_THIS, const xbiosmode_t *new_video_mode);
static void restoreMode(_THIS);
static int getLineWidth(_THIS, const xbiosmode_t *new_video_mode, int width, int bpp);
static void swapVbuffers(_THIS);
static int allocVbuffers(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize);
static void freeVbuffers(_THIS);

void SDL_XBIOS_VideoInit_Ctpci(_THIS)
{
	XBIOS_listModes = listModes;
	XBIOS_saveMode = saveMode;
	XBIOS_setMode = setMode;
	XBIOS_restoreMode = restoreMode;
	XBIOS_getLineWidth = getLineWidth;
	XBIOS_swapVbuffers = swapVbuffers;
	XBIOS_allocVbuffers = allocVbuffers;
	XBIOS_freeVbuffers = freeVbuffers;
}

#ifndef CTPCI_USE_TABLE
static unsigned long /*cdecl*/ enumfunc(SCREENINFO *inf, unsigned long flag)
{
	xbiosmode_t modeinfo;

	modeinfo.number = inf->devID;
	modeinfo.width = inf->scrWidth;
	modeinfo.height = inf->scrHeight;
	modeinfo.depth = inf->scrPlanes;
#ifdef ENABLE_CTPCI_SHADOWBUF
	modeinfo.flags = XBIOSMODE_SHADOWCOPY;
#else
	modeinfo.flags = 0;
#endif
	SDL_XBIOS_AddMode(enum_this, enum_actually_add, &modeinfo);

	return ENUMMODE_CONT;
}
#endif

static void listModes(_THIS, int actually_add)
{
#ifdef CTPCI_USE_TABLE
	int i;

	/* Read validated predefined modes */
	for (i=0; i<sizeof(mode_list)/sizeof(predefined_mode_t); i++) {
		int j;
		Uint16 deviceid = mode_list[i].modecode;

		for (j=3; j<5; j++) {
			if (Validmode(deviceid + j)) {
				xbiosmode_t modeinfo;

				modeinfo.number = deviceid + j;
				modeinfo.width = mode_list[i].width;
				modeinfo.height = mode_list[i].height;
				modeinfo.depth = mode_bpp[j-3];
#ifdef ENABLE_CTPCI_SHADOWBUF
				modeinfo.flags = XBIOSMODE_SHADOWCOPY;
#else
				modeinfo.flags = 0;
#endif
				SDL_XBIOS_AddMode(this, actually_add, &modeinfo);
			}
		}
	}
#else
	/* Read custom created modes */
	enum_this = this;
	enum_actually_add = actually_add;
	VsetScreen(-1, &enumfunc, VN_MAGIC, CMD_ENUMMODES);
#endif
}

static void saveMode(_THIS, SDL_PixelFormat *vformat)
{
	SCREENINFO si = { 0 };

	/* Read infos about current mode */
	VsetScreen(-1, &XBIOS_oldvmode, VN_MAGIC, CMD_GETMODE);

	si.size = sizeof(SCREENINFO);
	si.devID = XBIOS_oldvmode;
	si.scrFlags = 0;
	VsetScreen(-1, &si, VN_MAGIC, CMD_GETINFO);

	this->info.current_w = si.scrWidth;
	this->info.current_h = si.scrHeight;

	XBIOS_oldvbase = (void*)si.frameadr;

	vformat->BitsPerPixel = si.scrPlanes;

	XBIOS_oldnumcol = 0;
	if (si.scrFlags & SCRINFO_OK) {
		if (si.scrPlanes <= 8) {
			XBIOS_oldnumcol = 1<<si.scrPlanes;
		}
	}
	if (XBIOS_oldnumcol) {
		VgetRGB(0, XBIOS_oldnumcol, XBIOS_oldpalette);
	}
}

static void setMode(_THIS, const xbiosmode_t *new_video_mode)
{
	VsetScreen(-1, XBIOS_screens[0], VN_MAGIC, CMD_SETADR);

	VsetScreen(-1, new_video_mode->number, VN_MAGIC, CMD_SETMODE);

	/* Set hardware palette to black in True Colour */
	if (new_video_mode->depth > 8) {
		SDL_memset(F30_palette, 0, sizeof(F30_palette));
		VsetRGB(0,256,F30_palette);
	}
}

static void restoreMode(_THIS)
{
	VsetScreen(-1, XBIOS_oldvbase, VN_MAGIC, CMD_SETADR);
	VsetScreen(-1, XBIOS_oldvmode, VN_MAGIC, CMD_SETMODE);
	if (XBIOS_oldnumcol) {
		VsetRGB(0, XBIOS_oldnumcol, XBIOS_oldpalette);
	}
}

static void swapVbuffers(_THIS)
{
	VsetScreen(-1, -1, VN_MAGIC, CMD_FLIPPAGE);
}

static int getLineWidth(_THIS, const xbiosmode_t *new_video_mode, int width, int bpp)
{
	SCREENINFO si;
	int retvalue = width * (((bpp==15) ? 16 : bpp)>>3);

	/* Set pitch of new mode */
	si.size = sizeof(SCREENINFO);
	si.devID = new_video_mode->number;
	si.scrFlags = 0;
	VsetScreen(-1, &si, VN_MAGIC, CMD_GETINFO);
	if (si.scrFlags & SCRINFO_OK) {
		retvalue = si.lineWrap;
	}

	return (retvalue);
}

static int allocVbuffers(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize)
{
	int i;

	for (i=0; i<num_buffers; i++) {
		if (i==0) {
			/* Buffer 0 is current screen */
			XBIOS_screensmem[i] = XBIOS_oldvbase;
		} else {
			VsetScreen(&XBIOS_screensmem[i], new_video_mode->number, VN_MAGIC, CMD_ALLOCPAGE);
		}

		if (!XBIOS_screensmem[i]) {
			SDL_SetError("Can not allocate %d KB for buffer %d", bufsize>>10, i);
			return (0);
		}
		SDL_memset(XBIOS_screensmem[i], 0, bufsize);

		XBIOS_screens[i]=XBIOS_screensmem[i];
	}

	return (1);
}

static void freeVbuffers(_THIS)
{
	int i;

	for (i=0;i<2;i++) {
		if (XBIOS_screensmem[i]) {
			if (i==1) {
				VsetScreen(-1, -1, VN_MAGIC, CMD_FREEPAGE);
			} else {
				/* Do not touch buffer 0 */
			}
			XBIOS_screensmem[i]=NULL;
		}
	}
}
