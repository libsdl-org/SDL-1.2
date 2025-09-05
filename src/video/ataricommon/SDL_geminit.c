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

#include <gem.h>

#include "SDL_geminit_c.h"

/* Private display data */

struct SDL_PrivateVideoData {
	short vdi_handle;			/* VDI handle */
	short bpp;					/* Colour depth */
	short old_numcolors;		/* Number of colors in saved palette */
	Uint16 old_palette[256][3];	/* Saved current palette */

	short ap_id;
	GRECT desk;					/* Desktop properties */
	SDL_bool locked;			/* AES locked for fullscreen ? */
	OBJECT *menubar;			/* Menu bar to force desktop to restore its menu bar when going from fullscreen */
};

#define VDI_handle			(this->hidden->vdi_handle)
#define VDI_bpp				(this->hidden->bpp)
#define VDI_oldnumcolors	(this->hidden->old_numcolors)
#define VDI_oldpalette		(this->hidden->old_palette)

#define GEM_ap_id			(this->hidden->ap_id)
#define GEM_desk			(this->hidden->desk)
#define GEM_locked			(this->hidden->locked)
#define GEM_menubar			(this->hidden->menubar)

static OBJECT menu_obj[] = {
	/*
	 * next, head, tail, type,
	 * flags,
	 * state,
	 * spec,
	 * x, y, w,	h
	 */
	{-1, 1, 5, G_IBOX,				/*** 0 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 0L},
	0, 0, 80, 25},

	{5, 2, 2, G_BOX,				/*** 1 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 4352L},
	0, 0, 80, 513},

	{1, 3, 4, G_IBOX,				/*** 2 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 0L},
	2, 0, 12, 769},

	{4, -1, -1, G_TITLE,			/*** 3 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) " Desk"},
	0, 0, 6, 769},

	{2, -1, -1, G_TITLE,			/*** 4 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) " File"},
	6, 0, 6, 769},

	{0, 6, 15, G_IBOX,				/*** 5 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 0L},
	0, 769, 24, 8},

	{15, 7, 14, G_BOX,				/*** 6 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 16716032L},
	2, 0, 22, 8},

	{8, -1, -1, G_STRING,			/*** 7 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  SDL..."},
	0, 0, 22, 1},

	{9, -1, -1, G_STRING,			/*** 8 ***/
	OF_NONE,
	OS_DISABLED,
	{(long) "----------------------"},
	0, 1, 22, 1},

	{10, -1, -1, G_STRING,			/*** 9 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 1"},
	0, 2, 22, 1},

	{11, -1, -1, G_STRING,			/*** 10 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 2"},
	0, 3, 22, 1},

	{12, -1, -1, G_STRING,			/*** 11 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 3"},
	0, 4, 22, 1},

	{13, -1, -1, G_STRING,			/*** 12 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 4"},
	0, 5, 22, 1},

	{14, -1, -1, G_STRING,			/*** 13 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 5"},
	0, 6, 22, 1},

	{6, -1, -1, G_STRING,			/*** 14 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) "  Desk Accessory 6"},
	0, 7, 22, 1},

	{5, 16, 16, G_BOX,				/*** 15 ***/
	OF_NONE,
	OS_NORMAL,
	{(long) 16716032L},
	8, 0, 8, 1},

	{15, -1, -1, G_STRING,			/*** 16 ***/
	OF_LASTOB,
	OS_NORMAL,
	{(long) "  Quit"},
	0, 0, 8, 1}
};

SDL_bool GEM_CommonInit(Sint16 *ap_id, Sint16 *vdi_handle)
{
	int i;
	short work_in[11];
	short work_out[57];

	if (!ap_id || !vdi_handle)
		return SDL_FALSE;

	/* Test if AES available */
	aes_global[0] = 0x0000;
	*ap_id = appl_init();

	work_in[0] = 1;	/* 'Getrez() + 2' is not reliable on TOS 4.x and graphics cards, see https://github.com/pmandin/cleancode/issues/1 */
	for(i = 1; i < 10; i++)
		work_in[i] = 1;
	work_in[10] = 2;

	if (*ap_id == -1) {
		if (aes_global[0] != 0x0000) {
			/* AES present, so it's an error */
			fprintf(stderr, "appl_init() failed.\n");
			return SDL_FALSE;
		}

		/* Open physical VDI workstation */
		v_opnwk(work_in, vdi_handle, work_out);
		if (*vdi_handle == 0) {
			fprintf(stderr, "Can not open VDI physical workstation\n");
			goto GEM_CommonInit_appl_exit;
		}
	} else {
		short dummy;

		/* Ask VDI physical workstation handle opened by AES */
		*vdi_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);
		if (*vdi_handle < 1 ) {
			fprintf(stderr, "Wrong VDI handle %d returned by AES\n", *vdi_handle);
			goto GEM_CommonInit_appl_exit;
		}

		/* Open virtual VDI workstation */
		v_opnvwk(work_in, vdi_handle, work_out);
		if (*vdi_handle == 0) {
			fprintf(stderr, "Can not open VDI virtual workstation\n");
			goto GEM_CommonInit_appl_exit;
		}
	}

	return SDL_TRUE;

GEM_CommonInit_appl_exit:
	appl_exit();
	return SDL_FALSE;
}

void GEM_CommonCreateMenubar(_THIS)
{
	int i;

	if (GEM_ap_id == -1)
		return;

	/* Menu bar to force desktop to restore its menu bar when going from fullscreen */
	for (i = 0; i < sizeof(menu_obj)/sizeof(menu_obj[0]); ++i) {
		rsrc_obfix(menu_obj, i);
	}
	GEM_menubar = &menu_obj[ROOT];

	/* Read desktop size and position */
	wind_get(DESK, WF_WORKXYWH, &GEM_desk.g_x, &GEM_desk.g_y, &GEM_desk.g_w, &GEM_desk.g_h);
}

void GEM_CommonSavePalette(_THIS)
{
	int i;
	short work_out[57];

	if (VDI_handle == -1)
		return;

	/* Read bit depth */
	vq_extnd(VDI_handle, 1, work_out);
	VDI_bpp = work_out[4];

	/* Save current palette */
	if (VDI_bpp>8) {
		VDI_oldnumcolors=1<<8;
	} else {
		VDI_oldnumcolors=1<<VDI_bpp;
	}

	for(i = 0; i < VDI_oldnumcolors; i++) {
		short rgb[3];

		vq_color(VDI_handle, i, 0, rgb);

		VDI_oldpalette[i][0] = rgb[0];
		VDI_oldpalette[i][1] = rgb[1];
		VDI_oldpalette[i][2] = rgb[2];
	}
}

void GEM_CommonRestorePalette(_THIS)
{
	int i;

	if (VDI_handle == -1)
		return;

	for(i = 0; i < VDI_oldnumcolors; i++) {
		vs_color(VDI_handle, i, (short *) VDI_oldpalette[i]);
	}
}

void GEM_CommonQuit(_THIS, SDL_bool restore_cursor)
{
	GEM_UnlockScreen(this, restore_cursor);

	if (GEM_ap_id == -1) {
		if (VDI_handle > 0)
			v_clswk(VDI_handle);
	} else {
		if (VDI_handle > 0)
			v_clsvwk(VDI_handle);

		appl_exit();
		GEM_ap_id = -1;
	}

	VDI_handle = -1;
}

void GEM_LockScreen(_THIS, SDL_bool hide_cursor)
{
	if (GEM_ap_id == -1 || GEM_locked)
		return;

	/* Install menu bar to keep the application in foreground */
	menu_bar(GEM_menubar, MENU_INSTALL);

	/* Lock AES */
	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	if (hide_cursor)
		graf_mouse(M_OFF, NULL);

	/* Reserve memory space, used to be sure of compatibility */
	form_dial(FMD_START, 0,0,0,0, GEM_desk.g_x,GEM_desk.g_y,GEM_desk.g_w,GEM_desk.g_h);

	GEM_locked=SDL_TRUE;
}

void GEM_UnlockScreen(_THIS, SDL_bool restore_cursor)
{
	if (GEM_ap_id == -1 || !GEM_locked)
		return;

	/* Restore screen memory, and send REDRAW to all apps */
	form_dial(FMD_FINISH, 0,0,0,0, GEM_desk.g_x,GEM_desk.g_y,GEM_desk.g_w,GEM_desk.g_h);

	if (restore_cursor) {
		graf_mouse(M_ON, NULL);
		graf_mouse(ARROW, NULL);
	}

	/* Unlock AES */
	wind_update(END_MCTRL);
	wind_update(END_UPDATE);

	/* Restore desktop menu bar */
	menu_bar(GEM_menubar, MENU_REMOVE);

	GEM_locked=SDL_FALSE;
}
