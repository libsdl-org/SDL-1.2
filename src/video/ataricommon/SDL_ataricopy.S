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

#include "symbols.h"

/*
	Memory copy routines

	Miro Kropacek
*/

	.globl	SYM(SDL_Atari_CopyLong)
	.globl	SYM(SDL_Atari_CopyMove16)

/* move16 (a0)+,(a1)+ encoded by hand as the library is assembled for 68000 */
#define MOVE16_A0_A1	.dc.w	0xf620,0x9000

	.text

| void SDL_Atari_CopyLong(Uint8 *dst, const Uint8 *src, Uint32 w, Uint32 h, Uint32 dstskip, Uint32 srcskip);
| 68020+ only, 4 <= w < 16 MB, the source may be misaligned
SYM(SDL_Atari_CopyLong):
#if !defined(__mcoldfire__)
	movem.l	d2-d6,-(sp)
#ifdef __FASTCALL__
	exg	a0,a1					| a0: src, a1: dst
	move.l	d0,d3					| d3.l: width in bytes
	move.l	d1,d4					| d4.l: height
	move.l	d2,d5					| d5.l: dst skip
	move.l	(5*4+4,sp),d6			| d6.l: src skip
#else
	move.l	(5*4+4,sp),a1			| a1: dst
	move.l	(5*4+8,sp),a0			| a0: src
	move.l	(5*4+12,sp),d3			| d3.l: width in bytes
	move.l	(5*4+16,sp),d4			| d4.l: height
	move.l	(5*4+20,sp),d5			| d5.l: dst skip
	move.l	(5*4+24,sp),d6			| d6.l: src skip
#endif
	bra.w	copylong_row_test

copylong_row:
	move.l	d3,d0					| d0.l: bytes left in this row

	| copy the head up to the next 4-byte boundary of dst
	move.l	a1,d1
	neg.l	d1
	and.l	#3,d1
	sub.l	d1,d0
	lsr.w	#1,d1
	bcc.s	copylong_head2
	move.b	(a0)+,(a1)+
copylong_head2:
	lsr.w	#1,d1
	bcc.s	copylong_body
	move.w	(a0)+,(a1)+

copylong_body:
	move.l	d0,d1
	lsr.l	#8,d1					| d1.l: 256-byte blocks
	bra.w	copylong_256_test
copylong_256:
	.rept	64
	move.l	(a0)+,(a1)+
	.endr
copylong_256_test:
	dbra	d1,copylong_256

	move.w	d0,d1
	and.w	#0xff,d1
	lsr.w	#2,d1					| d1.w: remaining longs
	bra.s	copylong_longs_test
copylong_longs:
	move.l	(a0)+,(a1)+
copylong_longs_test:
	dbra	d1,copylong_longs

	| copy the tail after the last long
	btst	#1,d0
	beq.s	copylong_tail1
	move.w	(a0)+,(a1)+
copylong_tail1:
	btst	#0,d0
	beq.s	copylong_next
	move.b	(a0)+,(a1)+

copylong_next:
	add.l	d5,a1
	add.l	d6,a0
copylong_row_test:
	subq.l	#1,d4
	bpl.w	copylong_row

	movem.l	(sp)+,d2-d6
#endif
	rts

| void SDL_Atari_CopyMove16(Uint8 *dst, const Uint8 *src, Uint32 w, Uint32 h, Uint32 dstskip, Uint32 srcskip);
| 68040+ only, 16 <= w < 16 MB, src and dst must have the same alignment modulo 16 on every row
SYM(SDL_Atari_CopyMove16):
#if !defined(__mcoldfire__)
	movem.l	d2-d6,-(sp)
#ifdef __FASTCALL__
	exg	a0,a1					| a0: src, a1: dst
	move.l	d0,d3					| d3.l: width in bytes
	move.l	d1,d4					| d4.l: height
	move.l	d2,d5					| d5.l: dst skip
	move.l	(5*4+4,sp),d6			| d6.l: src skip
#else
	move.l	(5*4+4,sp),a1			| a1: dst
	move.l	(5*4+8,sp),a0			| a0: src
	move.l	(5*4+12,sp),d3			| d3.l: width in bytes
	move.l	(5*4+16,sp),d4			| d4.l: height
	move.l	(5*4+20,sp),d5			| d5.l: dst skip
	move.l	(5*4+24,sp),d6			| d6.l: src skip
#endif
	bra.w	copymove16_row_test

copymove16_row:
	move.l	d3,d0					| d0.l: bytes left in this row

	| copy the head up to the next 16-byte boundary
	move.l	a1,d1
	neg.l	d1
	and.l	#15,d1
	sub.l	d1,d0
	lsr.w	#1,d1
	bcc.s	copymove16_head2
	move.b	(a0)+,(a1)+
copymove16_head2:
	lsr.w	#1,d1
	bcc.s	copymove16_head4
	move.w	(a0)+,(a1)+
copymove16_head4:
	lsr.w	#1,d1
	bcc.s	copymove16_head8
	move.l	(a0)+,(a1)+
copymove16_head8:
	lsr.w	#1,d1
	bcc.s	copymove16_body
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+

copymove16_body:
	move.l	d0,d1
	lsr.l	#8,d1					| d1.l: 256-byte blocks
	bra.s	copymove16_256_test
copymove16_256:
	.rept	16
	MOVE16_A0_A1
	.endr
copymove16_256_test:
	dbra	d1,copymove16_256

	move.w	d0,d1
	and.w	#0xff,d1
	lsr.w	#4,d1					| d1.w: remaining 16-byte blocks
	bra.s	copymove16_16_test
copymove16_16:
	MOVE16_A0_A1
copymove16_16_test:
	dbra	d1,copymove16_16

	| copy the tail after the last 16-byte boundary
	btst	#3,d0
	beq.s	copymove16_tail4
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
copymove16_tail4:
	btst	#2,d0
	beq.s	copymove16_tail2
	move.l	(a0)+,(a1)+
copymove16_tail2:
	btst	#1,d0
	beq.s	copymove16_tail1
	move.w	(a0)+,(a1)+
copymove16_tail1:
	btst	#0,d0
	beq.s	copymove16_next
	move.b	(a0)+,(a1)+

copymove16_next:
	add.l	d5,a1
	add.l	d6,a0
copymove16_row_test:
	subq.l	#1,d4
	bpl.w	copymove16_row

	movem.l	(sp)+,d2-d6
#endif
	rts
