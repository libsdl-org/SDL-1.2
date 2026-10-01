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
#include "SDL_atariqueue.h"

/*
 *	IKBD 6301 interrupt routine
 *
 *	Patrice Mandin
 */

	.text

	.globl	SYM(SDL_AtariIkbd_Install)
	.globl	SYM(SDL_AtariIkbd_Restore)

	.globl	SYM(SDL_Atari_mouseb)
	.globl	SYM(SDL_Atari_joystick)
	.globl	SYM(SDL_Atari_queue)
	.globl	SYM(SDL_Atari_queue_head)

	.globl	SYM(Atari_UnhookXbra)

/*--- Install our kb_kbdsys vector ---*/

SYM(SDL_AtariIkbd_Install):
	| KBDVBASE *Kbdvbase(void);

#if defined(__mcoldfire__)
	lea	sp@(-8),sp
	moveml	d2/a2,sp@
#else
	moveml	d2/a2,sp@-
#endif
	movew	#34,sp@-
	trap	#14
	addql	#2,sp
#if defined(__mcoldfire__)
	moveml	sp@,d2/a2
	lea	sp@(8),sp
#else
	moveml	sp@+,d2/a2
#endif
	movel	d0,a1
	movel	a1,ikbd_kbdvecs

processing_ikbd_packet_install:

	| Stop interrupts

	movew	sr,d1
	movew	#0x2700,sr

	| Test kbstate

	tstb	a1@(36)
	beqs	no_ikbd_packet_install
	movew	d1,sr
	bras	processing_ikbd_packet_install
no_ikbd_packet_install:

	| Set our routine

	lea	ikbd_packet_start,a0
	movel	a0,ikbd_state

	movel	a1@(32),d0
	movel	d0,old_ikbd
	lea	ikbd,a0
	movel	a0,a1@(32)

	| Set mouse relative mode

	moveql	#8,d0
	moveb	d0,0xfffffc02:w

	| Reenable interrupts

	movew	d1,sr

	rts

/*--- Uninstall our kb_kbdsys vector ---*/

SYM(SDL_AtariIkbd_Restore):
	tstl	old_ikbd
	bnes	ikbd_installed
	rts

ikbd_installed:
	| Disable interrupts

	movew	sr,d1
	movew	#0x2700,sr

	movew	d1,sp@-

#ifdef __FASTCALL__
	movel	ikbd_kbdvecs,d0
	addil	#32,d0
	movel	#0x4c53444c,d1		/* "LSDL" */
	lea	ikbd,a0
	jsr	SYM(Atari_UnhookXbra)
#else
	pea	ikbd
	movel	#0x4c53444c,sp@-	/* "LSDL" */
	movel	ikbd_kbdvecs,a0
	pea	a0@(32)
	jsr	SYM(Atari_UnhookXbra)
	lea	sp@(12),sp
#endif

	movew	sp@+,d1

	| Clear keyboard buffer

	lea	0xfffffc00:w,a0
ikbd_videbuffer:
	btst	#0,a0@
	beqs	ikbd_finbuffer
	tstb	a0@(0x02)
	bras	ikbd_videbuffer
ikbd_finbuffer:

	| Reenable interrupts

	movew	d1,sr

	clrl	old_ikbd
	rts

	.bss

	.even
ikbd_kbdvecs:
	.ds.l	1
ikbd_state:
	.ds.l	1
ikbd_mouse_entry:
	.ds.l	1

/*--- Our custom kb_kbdsys vector ---*/

	.text
	.even
	.ascii	"XBRA"
	.ascii	"LSDL"
old_ikbd:
	.dc.l	0
ikbd:
#if defined(__mcoldfire__)
	lea	sp@(-12),sp
	moveml	d0-d1/a0,sp@
#else
	moveml	d0-d1/a0,sp@-
#endif

	| Interrupt request from the IKBD ACIA?

	moveb	0xfffffc00:w,d1
	bpl	ikbd_end

	| Reading the data register clears both a received byte and an
	| overrun, releasing the ACIA's interrupt line

	moveb	0xfffffc02:w,d0
	btst	#0,d1
	beq	ikbd_end

	| Continue the packet in progress

	movel	ikbd_state,a0
	jmp	a0@

	| First byte of a packet

ikbd_packet_start:

	| Joystick packet ?

	cmpb	#0xff,d0
	beqs	ikbd_yes_joystick

	| Mouse packet ?

	cmpb	#0xf8,d0
	blos	ikbd_no_mouse
	cmpb	#0xfc,d0
	bhss	ikbd_no_mouse

	| Mouse packet, byte #1

ikbd_yes_mouse:
	andl	#3,d0
	movew	d0,SYM(SDL_Atari_mouseb)

	| Start the queue entry, dx and dy follow in the next two bytes

	moveql	#ATARI_QUEUE_MOUSE,d1
	lsll	#8,d1
	moveb	d0,d1
	swap	d1
	movel	d1,ikbd_mouse_entry

	lea	ikbd_mousex,a0
	bras	ikbd_next_state

	| Joystick packet, byte #1

ikbd_yes_joystick:
	lea	ikbd_joystick,a0
	bras	ikbd_next_state

	| Keyboard press/release

ikbd_no_mouse:
	moveb	d0,d1
	moveql	#ATARI_QUEUE_KEY,d0
	lsll	#8,d0
	moveb	d1,d0
	swap	d0
	ATARI_QUEUE_PUSH
	bras	ikbd_end

	| Mouse packet, byte #2

ikbd_mousex:
	moveb	d0,ikbd_mouse_entry+2
	lea	ikbd_mousey,a0
	bras	ikbd_next_state

	| Mouse packet, byte #3

ikbd_mousey:
	moveb	d0,ikbd_mouse_entry+3

	movel	ikbd_mouse_entry,d0
	ATARI_QUEUE_PUSH
	bras	ikbd_packet_done

	| Joystick packet, byte #2

ikbd_joystick:
	moveb	d0,SYM(SDL_Atari_joystick)

ikbd_packet_done:
	lea	ikbd_packet_start,a0

ikbd_next_state:
	movel	a0,ikbd_state

ikbd_end:
#if defined(__mcoldfire__)
	moveml	sp@,d0-d1/a0
	lea	sp@(12),sp
#else
	moveml	sp@+,d0-d1/a0
#endif
	rts
