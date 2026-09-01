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
 *	XBIOS mouse & joystick vectors
 *
 *	Patrice Mandin
 */

#include <mint/cookie.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>

#include "SDL_atarievents_c.h"
#include "SDL_xbiosevents_c.h"
#include "SDL_xbiosinterrupt_s.h"

SDL_bool SDL_AtariXbios_IsKeyboardVectorSupported()
{
	OSHEADER *tos_header = (OSHEADER *)get_sysvar(_sysbase);

	/* Available only in TOS >= 2.x or in MagiC */
	return tos_header->os_version >= 0x0200 || Getcookie(C_MagX, NULL) == C_FOUND;
}

void AtariXbios_InitOSKeymap(_THIS)
{
	/* All three vectors must be installed together. Only GEM video+events
	 * driver (they are coupled together) has the luxury of being able to
	 * poll keyboard events via GEM and mouse/joystick via XBIOS. */
	int vectors_mask;
	vectors_mask  = ATARI_XBIOS_JOYSTICKEVENTS;	/* XBIOS joystick events */
	vectors_mask |= ATARI_XBIOS_MOUSEEVENTS;	/* XBIOS mouse events */
	vectors_mask |= ATARI_XBIOS_KEYBOARDEVENTS;	/* XBIOS keyboard events */

	SDL_AtariXbios_InstallVectors(vectors_mask);
}

void SDL_AtariXbios_InstallVectors(int vectors_mask)
{
	SDL_AtariXbios_mouselock = 0;

	if (vectors_mask==0)
		return;

	SDL_AtariXbios_installmousevector = (vectors_mask & ATARI_XBIOS_MOUSEEVENTS) != 0;
	SDL_AtariXbios_installjoystickvector = (vectors_mask & ATARI_XBIOS_JOYSTICKEVENTS) != 0;
	SDL_AtariXbios_installkeyboardvector = (vectors_mask & ATARI_XBIOS_KEYBOARDEVENTS) != 0;

	SDL_Atari_InstallVectors(SDL_AtariXbios_Install, SDL_AtariXbios_Restore);
}

void SDL_AtariXbios_LockMousePosition(SDL_bool lockPosition)
{
	SDL_AtariXbios_mouselock = lockPosition;
}
