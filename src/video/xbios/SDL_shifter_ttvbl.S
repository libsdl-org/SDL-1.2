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

	.globl	SYM(SDL_Shifter_VblTT)

	.globl	SYM(SDL_Shifter_vblcount)
	.globl	SYM(SDL_Shifter_counter)
	.globl	SYM(SDL_Shifter_palcount)
	.globl	SYM(SDL_Shifter_palfirst)
	.globl	SYM(SDL_Shifter_paletteTT)

	.even
	.ascii	"XBRA"
	.ascii	"LSDL"
old_vbl:
	.dc.l	0
SYM(SDL_Shifter_VblTT):
#if !defined(__mcoldfire__)
	movew	sr,sp@-
	oriw	#0x0700,sr
	moveml	d0-d1/a0-a1,sp@-

	addql	#1,SYM(SDL_Shifter_vblcount)

	lea	0xffff8203:w,a0
	movepl	a0@(0),d0
	andl	#0x00ffffff,d0		| drops the base middle byte
	movel	d0,SYM(SDL_Shifter_counter)

	movew	SYM(SDL_Shifter_palcount),d0
	beqs	vbl_end
	clrw	SYM(SDL_Shifter_palcount)
	subqw	#1,d0
	movew	SYM(SDL_Shifter_palfirst),d1
	addw	d1,d1
	lea	SYM(SDL_Shifter_paletteTT),a0
	addaw	d1,a0
	lea	0xffff8400:w,a1
	addaw	d1,a1
vbl_palette_loop:
	movew	a0@+,a1@+
	dbra	d0,vbl_palette_loop

vbl_end:
	moveml	sp@+,d0-d1/a0-a1
	movew	sp@+,sr
#endif
	movel	old_vbl(pc),sp@-
	rts
