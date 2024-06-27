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
 *	XBIOS mouse & joystick vectors
 *
 *	Patrice Mandin
 */

	.text

	.globl	SYM(SDL_AtariXbios_Install)
	.globl	SYM(SDL_AtariXbios_Restore)

	.globl	SYM(SDL_AtariXbios_installmousevector)
	.globl	SYM(SDL_AtariXbios_mouselock)
	.globl	SYM(SDL_AtariXbios_mouseb)
	.globl	SYM(SDL_AtariXbios_mousex)
	.globl	SYM(SDL_AtariXbios_mousey)
	.globl	SYM(SDL_AtariXbios_installjoystickvector)
	.globl	SYM(SDL_AtariXbios_joystick)
	.globl	SYM(SDL_AtariXbios_installkeyboardvector)
	.globl	SYM(SDL_AtariXbios_keyboard)

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

	/* Stop interrupts */

	movew	sr,d1
	movew	#0x2700,sr

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

	/* Stop interrupts */

	movew	sr,d1
	movew	#0x2700,sr

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
	lea	sp@(-8),sp
	moveml	d0-d1,sp@
#else
	movel	d0,sp@-
#endif

	/* Mouse buttons */
	moveb	(a0),d0
#if defined(__mcoldfire__)
	andl	#3,d0
#else
	andw	#3,d0
#endif
	movew	d0,SYM(SDL_AtariXbios_mouseb)

	/* X movement */
	moveb	a0@(1),d0
	extw	d0
#if defined(__mcoldfire__)
	movew	SYM(SDL_AtariXbios_mousex),d1
	addl	d1,d0
	movew	d0,SYM(SDL_AtariXbios_mousex)
#else
	addw	d0,SYM(SDL_AtariXbios_mousex)
#endif

	/* Y movement */
	moveb	a0@(2),d0
	extw	d0
#if defined(__mcoldfire__)
	movew	SYM(SDL_AtariXbios_mousey),d1
	addl	d1,d0
	movew	d0,SYM(SDL_AtariXbios_mousey)
#else
	addw	d0,SYM(SDL_AtariXbios_mousey)
#endif

	/* Lock mouse position ? */
	tstw	SYM(SDL_AtariXbios_mouselock)
	beq.s	no_mouse_lock
	clrb	a0@(1)
	clrb	a0@(2)
no_mouse_lock:

	/* Jump through old vector */
#if defined(__mcoldfire__)
	moveml	sp@,d0-d1
	lea	sp@(8),sp
#else
	movel	sp@+,d0
#endif

	movel	oldmousevector,sp@-
	rts

	.data

	.even
SYM(SDL_AtariXbios_installmousevector):
	.dc.w	0
SYM(SDL_AtariXbios_mouselock):
	.dc.w	0
SYM(SDL_AtariXbios_mousex):
	.dc.w	0
SYM(SDL_AtariXbios_mousey):
	.dc.w	0
SYM(SDL_AtariXbios_mouseb):
	.dc.w	0

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
	andw	#0x8f,d0
#endif
	movew	d0,SYM(SDL_AtariXbios_joystick)

	/* Jump through old vector */
	movel	sp@+,d0

	movel	oldjoystickvector,sp@-
	rts

	.data

	.even
SYM(SDL_AtariXbios_installjoystickvector):
	.dc.w	0
SYM(SDL_AtariXbios_joystick):
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
	lea		SYM(SDL_AtariXbios_keyboard),a0
#if defined(__mcoldfire__)
	andl	#0x7f,d1
	btst	#7,d0
	seq	d0
	moveb	d0,a0@(0,d1:l)
#else
	andw	#0x7f,d1
	btst	#7,d0
	seq	a0@(0,d1:w)
#endif

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
SYM(SDL_AtariXbios_keyboard):
	.ds.b	128
