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

/* VFC half-lines before VDB from which the base is left for the next VBL */
#define VDB_MARGIN	2

	.text

	.globl	SYM(SDL_Shifter_VblF30)

	.globl	SYM(SDL_Shifter_pending)
	.globl	SYM(SDL_Shifter_palcount)
	.globl	SYM(SDL_Shifter_palfirst)
	.globl	SYM(SDL_Shifter_paletteF30)

	.even
	.ascii	"XBRA"
	.ascii	"LSDL"
old_vbl:
	.dc.l	0
SYM(SDL_Shifter_VblF30):
#if !defined(__mcoldfire__)
	movew	sr,sp@-
	/* Prevent Timer C and others from delaying this VBL */
	oriw	#0x0700,sr
	moveml	d0-d2/a0-a1,sp@-

	movel	SYM(SDL_Shifter_pending),d0
	beqs	vbl_palette

	/* The Videl takes the base at VDB, when the frame's display begins.
	   From VDB_MARGIN half-lines before VDB until VDE a write may miss this
	   frame or be taken half written, so it is left for the next VBL. From
	   VDE on the display is over and the write is taken at the next VDB */
	movew	0xffff82a0:w,d1		| VFC
	cmpw	0xffff82aa:w,d1		| VDE
	bhss	vbl_write
	movew	0xffff82a8:w,d2		| VDB
	subqw	#VDB_MARGIN,d2
	cmpw	d2,d1
	bhss	vbl_palette

vbl_write:
	movel	d0,d1
	swap	d1
	moveb	d1,0xffff8201:w
	movel	d0,d1
	lsrw	#8,d1
	moveb	d1,0xffff8203:w
	moveb	d0,0xffff820d:w		| must follow the high and middle bytes

	clrl	SYM(SDL_Shifter_pending)

vbl_palette:
	movew	SYM(SDL_Shifter_palcount),d0
	beqs	vbl_end
	clrw	SYM(SDL_Shifter_palcount)
	subqw	#1,d0
	movew	SYM(SDL_Shifter_palfirst),d1
	lslw	#2,d1
	lea	SYM(SDL_Shifter_paletteF30),a0
	addaw	d1,a0
	lea	0xffff9800:w,a1
	addaw	d1,a1
vbl_palette_loop:
	movel	a0@+,a1@+
	dbra	d0,vbl_palette_loop

vbl_end:
	moveml	sp@+,d0-d2/a0-a1
	movew	sp@+,sr
#endif
	movel	old_vbl(pc),sp@-
	rts
