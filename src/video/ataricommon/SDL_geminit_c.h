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

#ifndef _SDL_GEMINIT_h
#define _SDL_GEMINIT_h

#include "SDL_stdinc.h"
#include "../SDL_sysvideo.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this

/* Returns SDL_TRUE if
 * - *ap_id has been set to -1 (AES not present) or AES handle (AES present)
 * - *vdi_handle has been set to the result of v_opnvwk() (AES present) or v_opnwk (AES not present)
 * Returns SDL_FALSE if
 * - ap_id or vdi_handle is NULL
 * - AES or VDI initialization has failed
 */
SDL_bool GEM_CommonInit(Sint16 *ap_id, Sint16 *vdi_handle);

void GEM_CommonCreateMenubar(_THIS);

void GEM_CommonSavePalette(_THIS);
void GEM_CommonRestorePalette(_THIS);

void GEM_CommonQuit(_THIS, SDL_bool restore_cursor);

void GEM_LockScreen(_THIS, SDL_bool hide_cursor);
void GEM_UnlockScreen(_THIS, SDL_bool restore_cursor);

#endif /* _SDL_GEMINIT_h */
