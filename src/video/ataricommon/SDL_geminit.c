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

#include "SDL_stdinc.h"

#include "SDL_geminit_c.h"

static GRECT GEM_desk;
static SDL_bool GEM_locked;

static Sint16 app_id = -1;
static SDL_bool aes_present;

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
static OBJECT *GEM_menubar = &menu_obj[ROOT];


Sint16 GEM_CommonInit()
{
	int i;

	/* Allow multiple init calls */
	if (app_id != -1)
		return app_id;

	app_id = appl_init();
	aes_present = aes_global[0] != 0x0000;

	if (app_id == -1) {
		if (aes_present)
			fprintf(stderr, "appl_init() failed.\n");

		return app_id;
	}

	/* Menu bar to force desktop to restore its menu bar when going from fullscreen */
	for (i = 0; i < sizeof(menu_obj)/sizeof(menu_obj[0]); ++i) {
		rsrc_obfix(menu_obj, i);
	}

	wind_get(DESK, WF_WORKXYWH, &GEM_desk.g_x, &GEM_desk.g_y, &GEM_desk.g_w, &GEM_desk.g_h);

	return app_id;
}

void GEM_CommonQuit(SDL_bool restore_cursor)
{
	GEM_UnlockScreen(restore_cursor);

	if (app_id != -1)
		appl_exit();
}

void GEM_LockScreen(SDL_bool hide_cursor)
{
	if (app_id == -1 || GEM_locked)
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

void GEM_UnlockScreen(SDL_bool restore_cursor)
{
	if (app_id == -1 || !GEM_locked)
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
