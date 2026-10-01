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

#ifndef _ATARI_COPY_h
#define _ATARI_COPY_h

#include "SDL_stdinc.h"

/* Copy with move.l, 68020+ only, 4 <= w < 16 MB */
void SDL_Atari_CopyLong(Uint8 *dst, const Uint8 *src, Uint32 w, Uint32 h, Uint32 dstskip, Uint32 srcskip);

/* Copy with move16, 68040+ only, 16 <= w < 16 MB, src and dst must have the same alignment modulo 16 on every row */
void SDL_Atari_CopyMove16(Uint8 *dst, const Uint8 *src, Uint32 w, Uint32 h, Uint32 dstskip, Uint32 srcskip);

#endif /* _ATARI_COPY_h */
