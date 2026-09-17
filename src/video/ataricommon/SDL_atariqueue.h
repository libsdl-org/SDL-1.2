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

/*
 *	Input queue written by the IKBD and XBIOS interrupt handlers
 *
 *	Preprocessor definitions only, this file is included from assembler too
 */

#ifndef _SDL_ATARIQUEUE_H_
#define _SDL_ATARIQUEUE_H_

#define ATARI_QUEUE_SIZE	1024
#define ATARI_QUEUE_MASK	(ATARI_QUEUE_SIZE-1)

/* One longword per entry: type in the top byte, then three payload bytes */
#define ATARI_QUEUE_KEY		0	/* scancode (bit 7 set on release), 0, 0 */
#define ATARI_QUEUE_MOUSE	1	/* buttons, dx, dy */

#ifdef __ASSEMBLER__
	/* Append the entry in d0 to the queue; d1 and a0 are clobbered */
	.macro	ATARI_QUEUE_PUSH
	movel	SYM(SDL_Atari_queue_head),d1
	andl	#ATARI_QUEUE_MASK,d1
	lsll	#2,d1
	lea	SYM(SDL_Atari_queue),a0
	movel	d0,a0@(0,d1:l)
	addql	#1,SYM(SDL_Atari_queue_head)
	.endm
#endif

#endif /* _SDL_ATARIQUEUE_H_ */
