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

#include "../ataricommon/symbols.h"

	.text

	.globl	SYM(SDL_Shifter_VblST)

	.globl	SYM(SDL_Shifter_counter)

	.even
	.ascii	"XBRA"
	.ascii	"LSDL"
old_vbl:
	.dc.l	0
SYM(SDL_Shifter_VblST):
#if !defined(__mcoldfire__)
	movew	sr,sp@-
	oriw	#0x0700,sr
	moveml	d0/a0,sp@-

	lea	0xffff8203:w,a0
	movepl	a0@(0),d0
	andl	#0x00ffffff,d0		| drops the base middle byte
	movel	d0,SYM(SDL_Shifter_counter)

	moveml	sp@+,d0/a0
	movew	sp@+,sr
#endif
	movel	old_vbl(pc),sp@-
	rts
