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
	Falcon Xbios video functions

	Patrice Mandin
*/

#include <mint/cookie.h>
#include <mint/osbind.h>
#include <mint/falcon.h>

#include "../SDL_sysvideo.h"

#include "../ataricommon/SDL_atarimxalloc_c.h"

#include "SDL_xbios.h"
#include "SDL_xbios_blowup.h"
#include "SDL_xbios_sb3.h"
#include "SDL_xbios_centscreen.h"

/* Use shadow buffer on Supervidel */
/*#define ENABLE_SV_SHADOWBUF 1*/

static const xbiosmode_t rgb_modes[]={
	{BPS16|COL80|OVERSCAN|VERTFLAG,768,480,16,0},
	{BPS16|COL80|OVERSCAN,768,240,16,0},
	{BPS16|COL80|VERTFLAG,640,400,16,0},
	{BPS16|COL80,640,200,16,0},
	{BPS16|OVERSCAN|VERTFLAG,384,480,16,0},
	{BPS16|OVERSCAN,384,240,16,0},
	{BPS16|VERTFLAG,320,400,16,0},
	{BPS16,320,200,16,0},
	{BPS8|COL80|OVERSCAN|VERTFLAG,768,480,8,XBIOSMODE_C2P},
	{BPS8|COL80|OVERSCAN,768,240,8,XBIOSMODE_C2P},
	{BPS8|COL80|VERTFLAG,640,400,8,XBIOSMODE_C2P},
	{BPS8|COL80,640,200,8,XBIOSMODE_C2P},
	{BPS8|OVERSCAN|VERTFLAG,384,480,8,XBIOSMODE_C2P},
	{BPS8|OVERSCAN,384,240,8,XBIOSMODE_C2P},
	{BPS8|VERTFLAG,320,400,8,XBIOSMODE_C2P},
	{BPS8,320,200,8,XBIOSMODE_C2P}
};

static const xbiosmode_t vga_modes[]={
	{BPS16,320,480,16,0},
	{BPS16|VERTFLAG,320,240,16,0},
	{BPS8|COL80,640,480,8,XBIOSMODE_C2P},
	{BPS8|COL80|VERTFLAG,640,240,8,XBIOSMODE_C2P},
	{BPS8,320,480,8,XBIOSMODE_C2P},
	{BPS8|VERTFLAG,320,240,8,XBIOSMODE_C2P}
};

static const xbiosmode_t sv_modes[]={
	{SVEXT|SVEXT_BASERES(4)|BPS32|COL80|OVERSCAN,2560,1440,32,0},	/* 32-bits */
	{SVEXT|SVEXT_BASERES(3)|BPS32|COL80|OVERSCAN,1920,1200,32,0},
	{SVEXT|SVEXT_BASERES(2)|BPS32|COL80|OVERSCAN,1920,1080,32,0},
	{SVEXT|SVEXT_BASERES(1)|BPS32|COL80|OVERSCAN,1680,1050,32,0},
	{SVEXT|SVEXT_BASERES(4)|BPS32|COL80,1600,1200,32,0},
	{SVEXT|SVEXT_BASERES(3)|BPS32|COL80,1280,1024,32,0},
	{SVEXT|SVEXT_BASERES(0)|BPS32|COL80|OVERSCAN,1280,720,32,0},
	{SVEXT|SVEXT_BASERES(2)|BPS32|COL80,1024,768,32,0},
	{SVEXT|SVEXT_BASERES(1)|BPS32|COL80,800,600,32,0},

	{SVEXT|SVEXT_BASERES(0)|BPS32|COL80,640,480,32,0},
	{SVEXT|SVEXT_BASERES(0)|BPS32|COL80|VERTFLAG,640,240,32,0},
	{SVEXT|SVEXT_BASERES(0)|BPS32,320,480,32,0},
	{SVEXT|SVEXT_BASERES(0)|BPS32|VERTFLAG,320,240,32,0},

	{SVEXT|SVEXT_BASERES(4)|BPS16|COL80|OVERSCAN,2560,1440,16,0},	/* 16-bits */
	{SVEXT|SVEXT_BASERES(3)|BPS16|COL80|OVERSCAN,1920,1200,16,0},
	{SVEXT|SVEXT_BASERES(2)|BPS16|COL80|OVERSCAN,1920,1080,16,0},
	{SVEXT|SVEXT_BASERES(1)|BPS16|COL80|OVERSCAN,1680,1050,16,0},
	{SVEXT|SVEXT_BASERES(4)|BPS16|COL80,1600,1200,16,0},
	{SVEXT|SVEXT_BASERES(3)|BPS16|COL80,1280,1024,16,0},
	{SVEXT|SVEXT_BASERES(0)|BPS16|COL80|OVERSCAN,1280,720,16,0},
	{SVEXT|SVEXT_BASERES(2)|BPS16|COL80,1024,768,16,0},
	{SVEXT|SVEXT_BASERES(1)|BPS16|COL80,800,600,16,0},

	{BPS16|COL80,640,480,16,0},
	{BPS16|COL80|VERTFLAG,640,240,16,0},
	{BPS16,320,480,16,0},
	{BPS16|VERTFLAG,320,240,16,0},

	{SVEXT|SVEXT_BASERES(4)|BPS8C|COL80|OVERSCAN,2560,1440,8,0},	/* 8-bits chunky */
	{SVEXT|SVEXT_BASERES(3)|BPS8C|COL80|OVERSCAN,1920,1200,8,0},
	{SVEXT|SVEXT_BASERES(2)|BPS8C|COL80|OVERSCAN,1920,1080,8,0},
	{SVEXT|SVEXT_BASERES(1)|BPS8C|COL80|OVERSCAN,1680,1050,8,0},
	{SVEXT|SVEXT_BASERES(4)|BPS8C|COL80,1600,1200,8,0},
	{SVEXT|SVEXT_BASERES(3)|BPS8C|COL80,1280,1024,8,0},
	{SVEXT|SVEXT_BASERES(0)|BPS8C|COL80|OVERSCAN,1280,720,8,0},
	{SVEXT|SVEXT_BASERES(2)|BPS8C|COL80,1024,768,8,0},
	{SVEXT|SVEXT_BASERES(1)|BPS8C|COL80,800,600,8,0},

	{BPS8C|COL80,640,480,8,0},
	{BPS8C|COL80|VERTFLAG,640,240,8,0},
	{BPS8C,320,480,8,0},
	{BPS8C|VERTFLAG,320,240,8,0}
};

static int has_supervidel;

static void listModes(_THIS, int actually_add);
static void saveMode(_THIS, SDL_PixelFormat *vformat);
static void setMode(_THIS, const xbiosmode_t *new_video_mode);
static void restoreMode(_THIS);
static int setColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);

static int allocVbuffers_SV(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize);
#ifdef ENABLE_SV_SHADOWBUF
static void updateRects_SV(_THIS, int numrects, SDL_Rect *rects);
static int flipHWSurface_SV(_THIS, SDL_Surface *surface);
#endif

void SDL_XBIOS_VideoInit_F30(_THIS)
{
	long cookie_cnts, cookie_scpn, cookie_dummy;

	XBIOS_listModes = listModes;
	XBIOS_saveMode = saveMode;
	XBIOS_setMode = setMode;
	XBIOS_restoreMode = restoreMode;

	this->SetColors = setColors;

	/* Supervidel ? */
	has_supervidel = 0;
	if (Getcookie(C_SupV, &cookie_dummy) == C_FOUND) {
		XBIOS_allocVbuffers = allocVbuffers_SV;
#ifdef ENABLE_SV_SHADOWBUF
		XBIOS_updRects = updateRects_SV;
		this->FlipHWSurface = flipHWSurface_SV;
#endif
		has_supervidel = 1;
	} else
	/* CTPCI ? */
	if ((Getcookie(C_CT60, &cookie_dummy) == C_FOUND)
	    && (Getcookie(C__PCI, &cookie_dummy) == C_FOUND)
	    && ((unsigned long)Physbase()>=0x01000000UL))
	{
		SDL_XBIOS_VideoInit_Ctpci(this);
	} else
	/* ScreenBlaster 3 ? */
	if (Getcookie(C_SCPN, &cookie_scpn) == C_FOUND) {
		SDL_XBIOS_VideoInit_SB3(this);
	} else
	/* Centscreen ? */
	if (Getcookie(C_CNTS, &cookie_cnts) == C_FOUND) {
		SDL_XBIOS_VideoInit_Centscreen(this);
	}
}

static void listModes(_THIS, int actually_add)
{
	long cookie_blow;
	int i, max_modes = 0;
	const xbiosmode_t *f30_modes = NULL;
	xbiosmode_t modeinfo;

	if (has_supervidel) {
		/* SuperVidel specific modes */
		max_modes = sizeof(sv_modes)/sizeof(xbiosmode_t);
		f30_modes = sv_modes;
	} else {
		/* Standard Videl */
		switch (VgetMonitor()) {
			case MONITOR_RGB:
			case MONITOR_TV:
				max_modes = sizeof(rgb_modes)/sizeof(xbiosmode_t);
				f30_modes = rgb_modes;
				break;
			case MONITOR_VGA:
				max_modes = sizeof(vga_modes)/sizeof(xbiosmode_t);
				f30_modes = vga_modes;
				break;
		}
	}

	for (i=0; i<max_modes; i++) {
		SDL_memcpy(&modeinfo, &f30_modes[i], sizeof(xbiosmode_t));
		modeinfo.number &= ~(VGA|PAL);
		modeinfo.number |= XBIOS_oldvmode & (VGA|PAL);
		SDL_XBIOS_AddMode(this, actually_add, &modeinfo);
	}

	if (!has_supervidel && Getcookie(C_BLOW, &cookie_blow) == C_FOUND) {
		SDL_XBIOS_ListBlowupModes(this, actually_add, (blow_cookie_t *)cookie_blow);
	}
}

static void saveMode(_THIS, SDL_PixelFormat *vformat)
{
	XBIOS_oldvbase=Physbase();

	XBIOS_oldvmode=VsetMode(-1);

	XBIOS_oldnumcol= 1<< (1 << (XBIOS_oldvmode & NUMCOLS));
	if (XBIOS_oldnumcol > 256) {
		XBIOS_oldnumcol = 256;
	}
	if (XBIOS_oldnumcol) {
		VgetRGB(0, XBIOS_oldnumcol, XBIOS_oldpalette);
	}
}

static void setMode(_THIS, const xbiosmode_t *new_video_mode)
{
	Setscreen(-1,XBIOS_screens[0],-1);

	(void) VsetMode(new_video_mode->number);

	/* Set hardware palette to black in True Colour */
	if (new_video_mode->depth > 8) {
		SDL_memset(F30_palette, 0, sizeof(F30_palette));
		VsetRGB(0,256,F30_palette);
	}
}

static void restoreMode(_THIS)
{
	Setscreen(-1,XBIOS_oldvbase,-1);

	(void) VsetMode(XBIOS_oldvmode);

	if (XBIOS_oldnumcol) {
		VsetRGB(0, XBIOS_oldnumcol, XBIOS_oldpalette);
	}
}

static int setColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	int i, r,g,b;

	for(i = 0; i < ncolors; i++) {
		r = colors[i].r;
		g = colors[i].g;
		b = colors[i].b;

		F30_palette[i]=(r<<16)|(g<<8)|b;
	}
	VsetRGB(firstcolor,ncolors,F30_palette);

	return 1;
}

static int allocVbuffers_SV(_THIS, const xbiosmode_t *new_video_mode, int num_buffers, int bufsize)
{
	int i;
	Uint32 tmp;

	for (i=0; i<num_buffers; i++) {
		XBIOS_screensmem[i] = Atari_SysMalloc(bufsize, MX_STRAM);

		if (XBIOS_screensmem[i]==NULL) {
			SDL_SetError("Can not allocate %d KB for buffer %d", bufsize>>10, i);
			return (0);
		}
		SDL_memset(XBIOS_screensmem[i], 0, bufsize);

		/* Align on 256byte boundary and map to Supervidel memory */
		tmp = ( (Uint32) XBIOS_screensmem[i]+256) & 0xFFFFFF00UL;
		tmp |= 0xA0000000UL;	/* Map to SV memory */
		XBIOS_screens[i] = (void *) tmp;
	}

#ifdef ENABLE_SV_SHADOWBUF
	/*--- Always use shadow buffer ---*/
	if (XBIOS_shadowscreen) {
		Mfree(XBIOS_shadowscreen);
		XBIOS_shadowscreen=NULL;
	}

	/* allocate shadow buffer in TT-RAM */
	XBIOS_shadowscreen = Atari_SysMalloc(bufsize, MX_PREFTTRAM);

	if (XBIOS_shadowscreen == NULL) {
		SDL_SetError("Can not allocate %d KB for shadow buffer", bufsize>>10);
		return (0);
	}
	SDL_memset(XBIOS_shadowscreen, 0, bufsize);
#endif

	return (1);
}

#ifdef ENABLE_SV_SHADOWBUF
static void updateRects_SV(_THIS, int numrects, SDL_Rect *rects)
{
	SDL_Surface *surface;
	int i;

	surface = this->screen;

	for (i=0;i<numrects;i++) {
		Uint8 *blockSrcStart, *blockDstStart;
		int y;

		blockSrcStart = (Uint8 *) surface->pixels;
		blockSrcStart += surface->pitch*rects[i].y;
		blockSrcStart += surface->format->BytesPerPixel*rects[i].x;

		blockDstStart = ((Uint8 *) XBIOS_screens[XBIOS_fbnum]) + surface->offset;
		blockDstStart += XBIOS_pitch*rects[i].y;
		blockDstStart += surface->format->BytesPerPixel*rects[i].x;

		for(y=0;y<rects[i].h;y++){
			SDL_memcpy(blockDstStart,blockSrcStart,surface->pitch);

			blockSrcStart += surface->pitch;
			blockDstStart += XBIOS_pitch;
		}
	}

	if ((surface->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF) {
		Setscreen(-1,XBIOS_screens[XBIOS_fbnum],-1);
		(*XBIOS_vsync)(this);

		XBIOS_fbnum ^= 1;
	}
}

static int flipHWSurface_SV(_THIS, SDL_Surface *surface)
{
	int i;
	Uint8 *src, *dst;

	src = surface->pixels;
	dst = ((Uint8 *) XBIOS_screens[XBIOS_fbnum]) + surface->offset;

	for (i=0; i<surface->h; i++) {
		SDL_memcpy(dst, src, surface->w * surface->format->BytesPerPixel);
		src += surface->pitch;
		dst += XBIOS_pitch;

	}

 	if ((surface->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF) {
		Setscreen(-1,XBIOS_screens[XBIOS_fbnum],-1);
		(*XBIOS_vsync)(this);

		XBIOS_fbnum ^= 1;
	}

	return(0);
}
#endif
