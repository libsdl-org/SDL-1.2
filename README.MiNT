==============================================================================
Using the Simple DirectMedia Layer on Atari
==============================================================================


    If you want to build SDL from sources to create SDL programs on Atari:
        see sections I - II.

    If you want to create SDL programs on Atari using SDL binary build,
        download it from my web site (URL at end of this file).

    If you want to configure a program using SDL on Atari,
        see sections IV - VI.


==============================================================================
I.  Building the Simple DirectMedia Layer libraries:
    (This step isn't necessary if you have the SDL binary distribution)

  Do the classic configure, with --disable-shared --enable-static and:

    Tos version (should run everywhere):
      --disable-threads
    Tos does not support threads.

    MiNT version (maybe Magic, only for multitasking OS):
      --disable-pthreads --enable-pth
    Mint and Magic may supports threads, so audio can be used with current
    devices, like Sun audio, or disk-writing support. Like Tos, interrupt
    audio without threads is more suited for Atari machines.

  Then you can make ; make install it.

==============================================================================
II. Building the Simple DirectMedia Layer test programs:

  Do the classic configure, then make.

  Run them !

==============================================================================
III.  Enjoy! :)

  If you have a project you'd like announce, or want to ask questions, go
  ahead and join the discussion forum for SDL, with categories for
  development, announcements, games and repository commits. Sign up at:
  https://discourse.libsdl.org

==============================================================================
IV.  What is supported:

Keyboard (XBIOS, GEM(1), Ikbd)
Mouse (XBIOS, GEM(2), Ikbd)
Video (XBIOS (Fullscreen), GEM (Windowed and Fullscreen))
Timer (Timer C interrupt, GNU pth library)
Joysticks (XBIOS, Ikbd) and joypads (Hardware)
Audio (Hardware, XBIOS, GSXB, MCSN, STFA, /dev/audio if threads enabled)
Threads (Multitasking OS only via GNU pth library)
Shared object loader (using LDG library from http://ldg.atari.org/)
Audio CD (MetaDOS)
OpenGL (using Mesa offscreen rendering driver)

Audio   O/S     Misc
dma8    All     Uses MFP Timer A interrupt
xbios	TOS     Uses MFP Timer A interrupt
xbios   MiNT    Uses MFP Timer A interrupt
xbios   Magic   Uses MFP Timer A interrupt
stfa    All     Uses MFP interrupt
mcsn	TOS     Uses MFP Timer A interrupt
mcsn    MiNT    Uses MiNT thread
mcsn    Magic   Disabled
gsxb    All     Uses GSXB callback

Joypad driver always uses hardware access.
OpenGL driver always uses OSMesa.

(1) GEM does not report AltrGr and CAPSLOCK key states, so Kbshift() (a BIOS
function) is used to report those key events.
(2) GEM does not report relative mouse motion, so xbios mouse driver is used
to report this type event.

==============================================================================
V.  Environment variables:

SDL_VIDEODRIVER:
	Set to 'xbios' to force xbios video driver. Uses XBIOS/Ikbd events driver.
	Set to 'gem' to force gem video driver. Uses an internal events driver.

SDL_VIDEO_GL_DRIVER:
	Set to filename to load as OpenGL library, if you use SDL_GL_LoadLibrary()

SDL_VIDEO_ALIGNED_WINDOWS:
	Set to force aligning of the window's x coordinate to a 16-pixel boundary
	(e.g. for speed reasons)

SDL_AUDIODRIVER:
	Set to 'mint_gsxb' to force Atari GSXB audio driver
	Set to 'mint_mcsn' to force Atari MCSN audio driver
	Set to 'mint_stfa' to force Atari STFA audio driver
	Set to 'mint_xbios' to force Atari Xbios audio driver
	Set to 'mint_dma8' to force Atari 8 bits DMA audio driver
	Set to 'audio' to force Sun /dev/audio audio driver
	Set to 'disk' to force disk-writing audio driver

SDL_ATARI_EVENTSDRIVER
	Set to 'ikbd' to force IKBD 6301 events driver
	Set to 'xbios' to force XBIOS events driver
	GEM events driver is used implicitly if 'gem' video driver is used.

SDL_JOYSTICK_ATARI:
	Use any of these strings in the environment variable to enable or
	disable a joystick:

	'ikbd-joy1-[on|off]' for IKBD joystick on port 1 (hardware access)
	'xbios-joy1-[on|off]' for IKBD joystick on port 1 (XBIOS access)
	'porta-pad-[on|off]' for joypad and/or teamtap on port A
	'porta-joy0-[on|off]' for joystick 0 on port A
	'porta-joy1-[on|off]' for joystick 1 on port A
	'porta-lp-[on|off]' for lightpen on port A
	'porta-anpad-[on|off]' for analog paddle on port A
	'portb-pad-[on|off]' for joypad and/or teamtap on port B
	'portb-joy0-[on|off]' for joystick 0 on port B
	'portb-joy1-[on|off]' for joystick 1 on port B
	'portb-anpad-[on|off]' for analog paddle on port B

	Default configuration is:
		'ikbd-joy1-on' (if IKBD events driver enabled)
		'xbios-joy1-on' (if xbios/gem events driver enabled)
		'porta-pad-on portb-pad-on' (if available on the machine)

	port[a|b]-[pad|joy?|lp|anpad]-* strings are mutually exclusives.
	On such a port, you can only use a joypad OR 1 or 2 joysticks OR
	a lightpen OR an analog paddle. You must disable joypad before
	setting another controller.

	The second joystick port on IKBD is used by the mouse, so not usable.
	Another problem with the IKBD: mouse buttons and joystick fire buttons
	are wired together at the hardware level, it means:
		port 0                port 0           port 1
		mouse left button  = joystick fire 0 = joystick fire 1
		mouse right button = joystick fire 1 = joystick fire 0

	Descriptions of joysticks/joypads:
	- Joypads: 1 hat, 17 buttons (Atari Jaguar console-like).
	- Joysticks: 1 hat, 1 button.
	- Lightpen, analog paddles: 2 axis, 2 buttons. The 2 buttons are those
	  affected to 1 button joysticks on the same port.

==============================================================================
VI.  More informations about drivers:

OpenGL:
	The default is to use the Mesa offscreen driver (osmesa.ldg). If you want
	to use an older OpenGL implementation, like mesa_gl.ldg or tiny_gl.ldg,
	your program must use SDL_GL_LoadLibrary() to do so, and retrieve the
	needed function pointers with SDL_LoadFunction(). In all cases, the OpenGL
	context is taken care of by SDL itself, you just have to use gl* functions.

	However, there is one OpenGL call that has a different prototype in the old
	implementations: glOrtho(). In the old implementations, it has 6 float as
	parameters, in the standard one, it has 6 double parameters. If you want
	to compile testdyngl, or any other SDL program that loads its OpenGL
	library, you must change the glOrtho() prototype used in this program. In
	osmesa.ldg, you can retrieve a glOrtho() with double parameters, by
	searching for the function "glOrtho6d".

Xbios video:
	Video chip is detected using the _VDO cookie.
	Screen enhancers are not supported, but could be if you know how to use
	them.

	ST, STE, Mega ST, Mega STE:
		320x200x4 bits, shades of grey, available only for the purpose of
		testing SDL.
	TT:
		320x480x8 and 320x240x8 (software double-lined mode).
	Falcon:
		All modes supported by the current monitor (RVB or VGA).
		BlowUp and Centscreen: Extended modes.
		ScreenBlaster 3: Only current mode.
		Supervidel: Extended modes.
		CTPCI: Only the modes supported by XBIOS (640x480, 800x600, 1024x768,
			   1280x960, 1600x1200 in 8/16/32 bpp).
	Milan:
		Nearly identical with CTPCI driver. Works only on Milan with S3 video
		card driver (which should be newer than 12.1.2000). To be verified.
	Any machine with NOVA video card:
		All modes with 8 bits or higher depth.
	Clones and any machine with monochrome monitor:
		Not supported.

Gem video:
	Automatically used if xbios not available.

	All machines:
		Only the current resolution, if 8 bits or higher depth.

Keyboard, mouse and joystick driver:
	XBIOS driver is automatically used in TOS >= 2.x and/or in MagiC. Otherwise
	IKBD driver is used.

	As XBIOS video driver is not available on Hades, it is forced to use the
	implicit GEM driver despite the fact that it has both IKBD and XBIOS
	features available.

Joypad driver:
	Available if _MCH cookie is STE or Falcon. Supports teamtap.

Timer driver:
	If you build SDL with threads using the GNU pth library and running on a
	multitasking OS , timers are supported via the pth library. Otherwise
	Timer C is used (which should be available on all machines).

Audio drivers:
	Cookies _SND, MCSN, STFA and GSXB used to detect supported audio
	capabilities.

	STE, Mega STE, TT:
		8 bits DMA (hardware access)
		STFA, MCSN or GSXB driver if installed
	Falcon:
		8 bits DMA (hardware access)
		Xbios functions
		STFA, MCSN or GSXB driver if installed
	Other machines:
		STFA, MCSN or GSXB driver if installed

	STFA driver:
		http://removers.free.fr/softs/stfa.html
	GSXB driver:
		http://assemsoft.atari.org/gsxb/
	MacSound driver:
		http://jf.omnis.ch/software/tos/
	MagicSound driver (MCSN,GSXB compatible):
		http://perso.wanadoo.fr/didierm/
	X-Sound driver (GSXB compatible):
		http://www.uni-ulm.de/~s_thuth/atari/xsound_e.html

--
Patrice Mandin <patmandin@gmail.com>
http://pmandin.atari.org/

Miro Kropacek <miro.kropacek@gmail.com>
http://mikro.atari.org/
