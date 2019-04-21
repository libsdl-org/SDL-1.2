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
 *	XBRA protocol
 *
 *	Miro Kropacek
 */

#include <mint/osbind.h>

#include "SDL_atarixbra_c.h"

/* Functions */

XbraHandler Atari_UnhookXbra(Uint32 vecnum, Uint32 app_id, XbraHandler handler)
{
	XBRA *rx;
	XbraHandler vecadr, *stepadr, ret = NULL;

	if (vecnum <= 0x16C) {	/* 0x5B0 is the last official system vector/variable */
		vecadr = Setexc(vecnum, VEC_INQUIRE);
	} else {
		vecadr = *((volatile XbraHandler*)vecnum);
	}

	rx = (XBRA*)((Uint32)vecadr - sizeof(XBRA));

	/* Special Case: Vector to remove is first in chain. */
	if(rx->xbra_id == XBRA_ID && rx->app_id == app_id && vecadr == handler)
	{
		if (vecnum <= 0x16C) {
			return Setexc(vecnum, rx->oldvec);
		} else {
			*((volatile XbraHandler*)vecnum) = rx->oldvec;
			return vecadr;
		}
	}

	stepadr = &rx->oldvec;
	rx = (XBRA*)((Uint32)rx->oldvec - sizeof(XBRA));
	while(rx->xbra_id == XBRA_ID)
	{
		if(rx->app_id == app_id && *stepadr == handler)
		{
			*stepadr = ret = rx->oldvec;
			break;
		}

		stepadr = &rx->oldvec;
		rx = (XBRA*)((Uint32)rx->oldvec - sizeof(XBRA));
	}

	return ret;
}
