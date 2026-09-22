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
 *	GEM Mouse manager
 *
 *	Patrice Mandin
 */

#include <gem.h>

#include "SDL_mouse.h"
#include "../../events/SDL_events_c.h"
#include "../SDL_cursor_c.h"
#include "SDL_gemmouse_c.h"
#include "SDL_gemvideo.h"
#include "../ataricommon/SDL_xbiosevents_c.h"

/* Defines */

/*#define DEBUG_VIDEO_GEM 1*/

#define MAXCURWIDTH 16
#define MAXCURHEIGHT 16

void GEM_FreeWMCursor(_THIS, WMcursor *cursor)
{
#ifdef DEBUG_VIDEO_GEM
	printf("sdl:video:gem: free cursor\n");
#endif

	if (cursor == NULL)
		return;

	graf_mouse(ARROW, NULL);
	GEM_prev_cursor = NULL;

	if (cursor->mform_p != NULL)
		SDL_free(cursor->mform_p);

	SDL_free(cursor);
}

WMcursor *GEM_CreateWMCursor(_THIS,
		Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y)
{
	WMcursor *cursor;
	MFORM *new_mform;
	int i;

#ifdef DEBUG_VIDEO_GEM
	Uint16 *data1, *mask1;

	printf("sdl:video:gem: create cursor\n");
#endif

	/* Check the size */
	if ( (w > MAXCURWIDTH) || (h > MAXCURHEIGHT) ) {
		SDL_SetError("Only cursors of dimension (%dx%d) are allowed",
							MAXCURWIDTH, MAXCURHEIGHT);
		return(NULL);
	}

	/* Allocate the cursor memory */
	cursor = (WMcursor *)SDL_malloc(sizeof(WMcursor));
	if ( cursor == NULL ) {
		SDL_OutOfMemory();
		return(NULL);
	}

	/* Allocate mform */
	new_mform = (MFORM *)SDL_malloc(sizeof(MFORM));
	if (new_mform == NULL) {
		SDL_free(cursor);
		SDL_OutOfMemory();
		return(NULL);
	}

	cursor->mform_p = new_mform;

	new_mform->mf_xhot = hot_x;
	new_mform->mf_yhot = hot_y;
	new_mform->mf_nplanes = 1;
	new_mform->mf_fg = 0;
	new_mform->mf_bg = 1;

	for (i=0;i<MAXCURHEIGHT;i++) {
		new_mform->mf_mask[i]=0;
		new_mform->mf_data[i]=0;
#ifdef DEBUG_VIDEO_GEM
		data1 = (Uint16 *) &data[i<<1];
		mask1 = (Uint16 *) &mask[i<<1];
		printf("sdl:video:gem: source: line %d: data=0x%04x, mask=0x%04x\n",
			i, data1[i], mask1[i]);
#endif
	}

	if (w<=8) {
		for (i=0;i<h;i++) {
			new_mform->mf_mask[i]= mask[i]<<8;
			new_mform->mf_data[i]= data[i]<<8;
		}
	} else {
		for (i=0;i<h;i++) {
			new_mform->mf_mask[i]= (mask[i<<1]<<8) | mask[(i<<1)+1];
			new_mform->mf_data[i]= (data[i<<1]<<8) | data[(i<<1)+1];
		}
	}

#ifdef DEBUG_VIDEO_GEM
	for (i=0; i<h ;i++) {
		printf("sdl:video:gem: cursor: line %d: data=0x%04x, mask=0x%04x\n",
			i, new_mform->mf_data[i], new_mform->mf_mask[i]);
	}

	printf("sdl:video:gem: CreateWMCursor(): done\n");
#endif

	return cursor;
}

int GEM_ShowWMCursor(_THIS, WMcursor *cursor)
{
	GEM_cursor = cursor;

	GEM_CheckMouseMode(this);

#ifdef DEBUG_VIDEO_GEM
	printf("sdl:video:gem: ShowWMCursor(0x%08x)\n", (long) cursor);
#endif

	return 1;
}

/* The pointer belongs to the application as long as it asked for the input
   and its window is the active one */
SDL_bool GEM_MouseGrabbed(_THIS)
{
	const Uint8 focus = (SDL_APPACTIVE|SDL_APPINPUTFOCUS);

	return (this->input_grab != SDL_GRAB_OFF)
		&& ((SDL_GetAppState() & focus) == focus);
}

/* The AES offers no way to place the pointer (appl_tplay() works on AES 3.4
   only), so report the movement to it as the IKBD would. It may scale the
   deltas, therefore approach the target and give up if it stops getting
   closer */
void GEM_SetMousePosition(short x, short y)
{
	int prev_distance = -1;
	int i;

	for (i=0; i<8; i++) {
		short mx, my, mb, ks;
		int distance;

		graf_mkstate(&mx, &my, &mb, &ks);

		distance = SDL_abs(x-mx) + SDL_abs(y-my);
		if (distance == 0 || (prev_distance >= 0 && distance >= prev_distance)) {
			break;
		}
		prev_distance = distance;

		SDL_AtariXbios_MoveMousePosition(x-mx, y-my);
	}
}

void GEM_WarpWMCursor(_THIS, Uint16 x, Uint16 y)
{
	/* The pointer is the AES's unless our window is the active one, and in
	   relative mode it stays parked: only SDL's idea of it moves */
	if (!GEM_mouse_relative && (SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
		short x2 = 0, y2 = 0;

		if ((!GEM_fullscreen) && (GEM_handle>=0)) {
			x2 = GEM_work.g_x;
			y2 = GEM_work.g_y;
		}

		GEM_SetMousePosition(x2+x, y2+y);
	}

	SDL_PrivateMouseMotion(0, 0, x, y);
}

void GEM_CheckMouseMode(_THIS)
{
	const Uint8 full_focus = (SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
	int set_system_cursor = 1;
	SDL_bool hide_system_cursor = SDL_FALSE;

#ifdef DEBUG_VIDEO_GEM
	printf("sdl:video:gem: check mouse mode\n");
#endif

	/* If the mouse is hidden and the pointer is ours, we use relative mode */
	GEM_mouse_relative = (!(SDL_cursorstate & CURSOR_VISIBLE))
		&& GEM_MouseGrabbed(this);
	SDL_AtariXbios_LockMousePosition(GEM_mouse_relative);

	if (SDL_cursorstate & CURSOR_VISIBLE) {
		/* Application defined cursor only over the application window */
		if ((SDL_GetAppState() & full_focus) == full_focus) {
			if (GEM_cursor) {
				if (GEM_cursor != GEM_prev_cursor) {
					graf_mouse(USER_DEF, GEM_cursor->mform_p);
					GEM_prev_cursor = GEM_cursor;
				}
				set_system_cursor = 0;
			} else {
				hide_system_cursor = SDL_TRUE;
			}
		}
	} else {
		/* Mouse cursor hidden only over the application window */
		if ((SDL_GetAppState() & full_focus) == full_focus) {
			set_system_cursor = 0;
			hide_system_cursor = SDL_TRUE;
		}
	}

	if (hide_system_cursor != GEM_cursor_hidden) {
		graf_mouse(hide_system_cursor ? M_OFF : M_ON, NULL);
		GEM_cursor_hidden = hide_system_cursor;
	}

	if (set_system_cursor && GEM_prev_cursor) {
		graf_mouse(ARROW, NULL);
		GEM_prev_cursor = NULL;
	}
}
