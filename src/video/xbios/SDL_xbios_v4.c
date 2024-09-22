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
	Vampire V4 Xbios video functions

	Peter Persson
*/

#include <mint/osbind.h>
#include <mint/falcon.h>

#include "../ataricommon/SDL_atarimxalloc_c.h"

#include "SDL_xbios.h"

typedef struct {
	Uint16 modecode, width, height;
} predefined_mode_t;

typedef struct {
	Uint16 modecode, depth, flags;
} predefined_fmt_t;

static const predefined_fmt_t fmt_list[] = {
	{ 0x0005, 32, 0 },
	{ 0x0004, 24, 0 },
	{ 0x0002, 16, 0 },
	{ 0x0003, 15, 0 },
	{ 0x0001, 8 , 0 },
};

static const predefined_mode_t mode_list[] = {
	{ 0x0100, 320,  200 },
	{ 0x0200, 320,  240 },
	{ 0x0300, 320,  256 },
	{ 0x0400, 640,  400 },
	{ 0x0500, 640,  480 },
	{ 0x0600, 640,  512 },
	{ 0x0700, 960,  540 },
	{ 0x0800, 480,  270 },
	{ 0x0900, 304,  224 },
	{ 0x0A00, 1280, 720 },
	{ 0x0B00, 640,  360 },
	{ 0x0C00, 800,  600 },
	{ 0x0D00, 1024, 768 },
	{ 0x0E00, 720,  576 },
	{ 0x0F00, 848,  480 },
	{ 0x1000, 640,  200 },
    { 0x1100, 1920,1080 },
    { 0x1200, 1280,1024 },
    { 0x1300, 1280, 800 },
    { 0X1400, 1440, 900 },
};

static void listModes(_THIS, int actually_add);
static void saveMode(_THIS, SDL_PixelFormat *vformat);
static void restoreMode(_THIS);
static int allocVbuffers_V4(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize);

void SDL_XBIOS_VideoInit_V4(_THIS)
{
	XBIOS_listModes     = listModes;
	XBIOS_saveMode      = saveMode;
	XBIOS_allocVbuffers = allocVbuffers_V4;
}

static void listModes(_THIS, int actually_add)
{
	int i, j;

	for(i = 0; i < (sizeof(fmt_list) / sizeof(fmt_list[0])); i++) {
		for(j =  0; j < (sizeof(mode_list) / sizeof(mode_list[0])); j++) {
			xbiosmode_t modeinfo;

			modeinfo.number = 0x4000 | fmt_list[i].modecode | mode_list[j].modecode;
			modeinfo.width = mode_list[j].width;
			modeinfo.height = mode_list[j].height;
			modeinfo.depth = fmt_list[i].depth;
			modeinfo.flags = fmt_list[i].flags;

			SDL_XBIOS_AddMode(this, actually_add, &modeinfo);
		}
	}
}

static void saveMode(_THIS, SDL_PixelFormat *vformat)
{
	XBIOS_oldvbase=Physbase();

	XBIOS_oldvmode=VsetMode(-1);

	if(XBIOS_oldvmode & 0x4000)
		XBIOS_oldnumcol = 256;
	else
		XBIOS_oldnumcol = 1<< (1 << (XBIOS_oldvmode & NUMCOLS));

	if (XBIOS_oldnumcol > 256) {
		XBIOS_oldnumcol = 256;
	}
	if (XBIOS_oldnumcol) {
		VgetRGB(0, XBIOS_oldnumcol, XBIOS_oldpalette);
	}
}

static int allocVbuffers_V4(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize)
{
	int i;
	Uint32 tmp;

	for (i = 0; i < num_buffers; i++) {
		XBIOS_screensmem[i] = Atari_SysMalloc(bufsize, MX_PREFTTRAM);

		if (XBIOS_screensmem[i] == NULL) {
			SDL_SetError("Can not allocate %d KB for buffer %d", bufsize>>10, i);
			return (0);
		}
		SDL_memset(XBIOS_screensmem[i], 0, bufsize);

		/* Align on 256byte boundary */
		tmp = ( (Uint32) XBIOS_screensmem[i] + 255) & 0xFFFFFF00UL;
		XBIOS_screens[i] = (void *) tmp;
	}

	return (1);
}
