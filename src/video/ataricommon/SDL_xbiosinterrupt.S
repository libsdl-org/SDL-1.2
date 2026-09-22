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
 *	XBIOS mouse & joystick vectors
 *
 *	Patrice Mandin
 */

	.text

	.globl	SYM(SDL_AtariXbios_Install)
	.globl	SYM(SDL_AtariXbios_Restore)

	.globl	SYM(SDL_AtariXbios_installmousevector)
	.globl	SYM(SDL_AtariXbios_installjoystickvector)
	.globl	SYM(SDL_AtariXbios_installkeyboardvector)
	.globl	SYM(SDL_AtariXbios_mouselock)
	.globl	SYM(SDL_AtariXbios_MoveMouse)
	.globl	SYM(SDL_AtariXbios_mousedx)
	.globl	SYM(SDL_AtariXbios_mousedy)

	.globl	SYM(SDL_Atari_mouseb)
	.globl	SYM(SDL_Atari_joystick)
	.globl	SYM(SDL_Atari_queue)
	.globl	SYM(SDL_Atari_queue_head)

	.globl	SYM(Atari_UnhookXbra)

/*--- Vector installer ---*/

SYM(SDL_AtariXbios_Install):
#if defined(__mcoldfire__)
	lea	sp@(-8),sp
	moveml	d2/a2,sp@
#else
	moveml	d2/a2,sp@-
#endif
	/* KBDVBASE *Kbdvbase(void); */
	movew	#34,sp@-
	trap	#14
	addql	#2,sp
#if defined(__mcoldfire__)
	moveml	sp@,d2/a2
	lea	sp@(8),sp
#else
	moveml	sp@+,d2/a2
#endif
	movel	d0,a0

processing_ikbd_packet_install:

	/* Stop interrupts */

	movew	sr,d1
	movew	#0x2700,sr

	/* Test kbstate */

	tstb	a0@(36)
	beqs	no_ikbd_packet_install
	movew	d1,sr
	bras	processing_ikbd_packet_install
no_ikbd_packet_install:

	/* Save old mouse vector, set our routine */

	tstw	SYM(SDL_AtariXbios_installmousevector)
	beqs	no_new_mouse_vector
#if defined(__mcoldfire__)
	movel	a0@(16),a1
	movel	a1,oldmousevector
	lea	newmousevector(pc),a1
	movel	a1,a0@(16)
#else
	movel	a0@(16),oldmousevector
	movel	#newmousevector,a0@(16)
#endif
no_new_mouse_vector:

	/* Save old joystick vector, set our routine */

	tstw	SYM(SDL_AtariXbios_installjoystickvector)
	beqs	no_new_joystick_vector
#if defined(__mcoldfire__)
	movel	a0@(24),a1
	movel	a1,oldjoystickvector
	lea	newjoystickvector(pc),a1
	movel	a1,a0@(24)
#else
	movel	a0@(24),oldjoystickvector
	movel	#newjoystickvector,a0@(24)
#endif
no_new_joystick_vector:

	/* Save old keyboard vector, set our routine */

	tstw	SYM(SDL_AtariXbios_installkeyboardvector)
	beqs	no_new_keyboard_vector
#if defined(__mcoldfire__)
	movel	a0@(-4),a1
	movel	a1,oldkeyboardvector
	lea	newkeyboardvector(pc),a1
	movel	a1,a0@(-4)
#else
	movel	a0@(-4),oldkeyboardvector
	movel	#newkeyboardvector,a0@(-4)
#endif
no_new_keyboard_vector:

	/* Restart interrupts */

	movew	d1,sr

	rts

/*--- Vector restorer ---*/

SYM(SDL_AtariXbios_Restore):
#if defined(__mcoldfire__)
	lea	sp@(-8),sp
	moveml	d2/a2,sp@
#else
	moveml	d2/a2,sp@-
#endif
	/* KBDVBASE *Kbdvbase(void); */
	movew	#34,sp@-
	trap	#14
	addql	#2,sp
#if defined(__mcoldfire__)
	moveml	sp@,d2/a2
	lea	sp@(8),sp
#else
	moveml	sp@+,d2/a2
#endif
	movel	d0,a0

processing_ikbd_packet_restore:
	/* Stop interrupts */

	movew	sr,d1
	movew	#0x2700,sr

	/* Test kbstate */

	tstb	a0@(36)
	beqs	no_ikbd_packet_restore
	movew	d1,sr
	bras	processing_ikbd_packet_restore
no_ikbd_packet_restore:

	/* Restore mouse vector */

	tstl	oldmousevector
	beqs	no_restore_mouse

#if defined(__mcoldfire__)
	lea	sp@(-8),sp
	moveml	d1/a0,sp@
#else
	moveml	d1/a0,sp@-
#endif
	pea	newmousevector(pc)
	movel	#0x4c53444c,sp@-	/* "LSDL" */
	pea	a0@(16)
	jsr	SYM(Atari_UnhookXbra)
	lea	sp@(12),sp
	clrl	oldmousevector

#if defined(__mcoldfire__)
	moveml	sp@,d1/a0
	lea	sp@(8),sp
#else
	moveml	sp@+,d1/a0
#endif

no_restore_mouse:

	/* Restore joystick vector */

	tstl	oldjoystickvector
	beqs	no_restore_joystick

	movew	d1,sp@-

	pea	newjoystickvector(pc)
	movel	#0x4c53444c,sp@-	/* "LSDL" */
	pea	a0@(24)
	jsr	SYM(Atari_UnhookXbra)
	lea	sp@(12),sp
	clrl	oldjoystickvector

	movew	sp@+,d1

no_restore_joystick:

	/* Restore keyboard vector */

	tstl	oldkeyboardvector
	beqs	no_restore_keyboard

	movew	d1,sp@-

	pea	newkeyboardvector(pc)
	movel	#0x4c53444c,sp@-	/* "LSDL" */
	pea	a0@(-4)
	jsr	SYM(Atari_UnhookXbra)
	lea	sp@(12),sp
	clrl	oldkeyboardvector

	movew	sp@+,d1

no_restore_keyboard:

	/* Restart interrupts */

	movew	d1,sr
	rts

/*--- Our mouse vector ---*/

	.text
	.even
	.ascii "XBRA"
	.ascii "LSDL"
oldmousevector:
	.dc.l	0
newmousevector:
#if defined(__mcoldfire__)
	lea	sp@(-12),sp
	moveml	d0-d1/a0,sp@
#else
	moveml	d0-d1/a0,sp@-
#endif

	/* Mouse buttons */
	moveb	a0@,d1
	andl	#7,d1
	movew	d1,SYM(SDL_Atari_mouseb)

	/* Queue entry: type, buttons, dx, dy */
	moveql	#ATARI_QUEUE_MOUSE,d0
	lsll	#8,d0
	moveb	d1,d0
	lsll	#8,d0
	moveb	a0@(1),d0
	lsll	#8,d0
	moveb	a0@(2),d0

	/* Lock mouse position ? */
	tstw	SYM(SDL_AtariXbios_mouselock)
	beq.s	no_mouse_lock
	clrb	a0@(1)
	clrb	a0@(2)
no_mouse_lock:

	ATARI_QUEUE_PUSH

	/* Jump through old vector */
#if defined(__mcoldfire__)
	moveml	sp@,d0-d1/a0
	lea	sp@(12),sp
#else
	moveml	sp@+,d0-d1/a0
#endif

	movel	oldmousevector,sp@-
	rts

	.data

	.even
SYM(SDL_AtariXbios_installmousevector):
	.dc.w	0
SYM(SDL_AtariXbios_mouselock):
	.dc.w	0

/*--- Mouse position setter ---*/

/* Reports SDL_AtariXbios_mousedx/dy through the saved mouse vector, as the
   IKBD interrupt would; run it with Supexec() */

	.text

	.even
SYM(SDL_AtariXbios_MoveMouse):
	movel	oldmousevector,d0
	beqs	no_move_mouse

#if defined(__mcoldfire__)
	lea	sp@(-20),sp
	moveml	d1-d2/a0-a2,sp@
#else
	moveml	d1-d2/a0-a2,sp@-
#endif

	movel	d0,a1
	lea	mousepacket,a0

	/* Relative mouse packet: header 0xf8 plus the buttons, then dx, dy */
	movew	SYM(SDL_Atari_mouseb),d0
	orl	#0xf8,d0
	moveb	d0,a0@
	movew	SYM(SDL_AtariXbios_mousedx),d0
	moveb	d0,a0@(1)
	movew	SYM(SDL_AtariXbios_mousedy),d0
	moveb	d0,a0@(2)

	movew	sr,d2
	movew	#0x2700,sr
	jsr	a1@
	movew	d2,sr

#if defined(__mcoldfire__)
	moveml	sp@,d1-d2/a0-a2
	lea	sp@(20),sp
#else
	moveml	sp@+,d1-d2/a0-a2
#endif

no_move_mouse:
	rts

	.data

	.even
SYM(SDL_AtariXbios_mousedx):
	.dc.w	0
SYM(SDL_AtariXbios_mousedy):
	.dc.w	0
mousepacket:
	.dc.b	0,0,0
	.even

/*--- Our joystick vector ---*/

	.text

	.even
	.ascii "XBRA"
	.ascii "LSDL"
oldjoystickvector:
	.dc.l	0
newjoystickvector:
	movel	d0,sp@-

	/* New joystick state */
	moveb	a0@(2),d0
#if defined(__mcoldfire__)
	andl	#0x8f,d0
#else
	andb	#0x8f,d0
#endif
	moveb	d0,SYM(SDL_Atari_joystick)

	/* Jump through old vector */
	movel	sp@+,d0

	movel	oldjoystickvector,sp@-
	rts

	.data

	.even
SYM(SDL_AtariXbios_installjoystickvector):
	.dc.w	0


/*--- Our keyboard vector ---*/

	.text

	.even
	.ascii "XBRA"
	.ascii "LSDL"
oldkeyboardvector:
	.dc.l	0
newkeyboardvector:

#if defined(__mcoldfire__)
	lea	sp@(-12),sp
	moveml	d0-d1/a0,sp@
#else
	moveml	d0-d1/a0,sp@-
#endif

	moveb	d0,d1
	moveql	#ATARI_QUEUE_KEY,d0
	lsll	#8,d0
	moveb	d1,d0
	swap	d0
	ATARI_QUEUE_PUSH

	/* Jump through old vector */
#if defined(__mcoldfire__)
	moveml	sp@,d0-d1/a0
	lea	sp@(12),sp
#else
	moveml	sp@+,d0-d1/a0
#endif

	movel	oldkeyboardvector,sp@-
	rts

	.data

	.even
SYM(SDL_AtariXbios_installkeyboardvector):
	.dc.w	0
