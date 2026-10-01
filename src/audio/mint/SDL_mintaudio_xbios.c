/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/*
	MiNT audio driver using uSound

	Patrice Mandin, Didier Mequignon, Miro Kropacek
*/

/* Mint includes */
#include <mint/osbind.h>
#include <mint/falcon.h>
#include <mint/cookie.h>

#include <usound.h>

#if !defined(USOUND_VERSION) || USOUND_VERSION < 2
#error "uSound 2 or newer is required, update usound.h from https://github.com/mikrosk/usound"
#endif

#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"

#include "SDL_mintaudio.h"

/*--- Defines ---*/

#define MINT_AUDIO_DRIVER_NAME "xbios"

/* GSXB extends Setinterrupt() with a third parameter, the address of a
   routine to be called at the end of a frame. It is the only mode GSXB
   supports: the Falcon's Timer A and MFP i7 end of frame interrupts are
   rejected with EBADR because they simply do not exist on some of the
   machines GSXB drives (the Hades, for instance). */
#define SI_GSXB	2	/* Install user End-Of-Frame interrupt function */

/* Debug print info */
#define DEBUG_NAME "audio:xbios: "
#if 0
#define DEBUG_PRINT(what) \
	{ \
		printf what; \
	}
#else
#define DEBUG_PRINT(what)
#endif

/*--- Audio driver functions ---*/

static void Mint_CloseAudio(_THIS);
static int Mint_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void Mint_SwapBuffers(Uint8 *nextbuf, int nextsize);

#if !defined(__mcoldfire__)
/* GSXB callback */
static void Mint_GsxbNullInterrupt(void);
#endif

/*--- Variables ---*/

/* uSound state, shared between Mint_OpenAudio() and Mint_CloseAudio() */
static USoundContext usound_context;

/* Do we use GSXB's own end of frame callback instead of Timer A ?
   ColdFire machines have neither: FireTOS raises no end of frame
   interrupt, so the replay position is polled instead. */
#if !defined(__mcoldfire__)
static int use_gsxb;
#endif

/*--- Format conversion helpers (SDL <-> uSound) ---*/

static int Mint_FormatToUSound(Uint16 sdl_format, USoundFormat *usound_format)
{
	switch (sdl_format) {
		case AUDIO_U8:     *usound_format = USoundFormatUnsigned8;     break;
		case AUDIO_S8:     *usound_format = USoundFormatSigned8;       break;
		case AUDIO_U16LSB: *usound_format = USoundFormatUnsigned16LSB; break;
		case AUDIO_S16LSB: *usound_format = USoundFormatSigned16LSB;   break;
		case AUDIO_U16MSB: *usound_format = USoundFormatUnsigned16MSB; break;
		case AUDIO_S16MSB: *usound_format = USoundFormatSigned16MSB;   break;
		default: return 0;
	}
	return 1;
}

static int Mint_FormatFromUSound(USoundFormat usound_format, Uint16 *sdl_format)
{
	switch (usound_format) {
		case USoundFormatUnsigned8:     *sdl_format = AUDIO_U8;     break;
		case USoundFormatSigned8:       *sdl_format = AUDIO_S8;     break;
		case USoundFormatUnsigned16LSB: *sdl_format = AUDIO_U16LSB; break;
		case USoundFormatSigned16LSB:   *sdl_format = AUDIO_S16LSB; break;
		case USoundFormatUnsigned16MSB: *sdl_format = AUDIO_U16MSB; break;
		case USoundFormatSigned16MSB:   *sdl_format = AUDIO_S16MSB; break;
		default: return 0;
	}
	return 1;
}

/*--- GSXB ---*/

#if !defined(__mcoldfire__)

/* SI_GSXB is documented as available when bits 2 and 5 of the _SND cookie
   are set *and* the GSXB cookie is present. Both halves matter: GSXB sets
   those bits only once a sound card driver has installed itself under it,
   and MilanBlaster/MagicMac set bit 5 as well but do not necessarily
   implement the extended call. */
static int Mint_GsxbAvailable(void)
{
	long cookie;

	if (Getcookie(C_GSXB, &cookie) != C_FOUND) {
		return(0);
	}
	if (Getcookie(C__SND, &cookie) != C_FOUND) {
		return(0);
	}
	return((cookie & (SND_16BIT|SND_EXT)) == (SND_16BIT|SND_EXT));
}

static void Mint_GsxbNullInterrupt(void)
{
}

#endif /* !__mcoldfire__ */

/*--- Audio driver bootstrap functions ---*/

static int Audio_Available(void)
{
	const char *envr = SDL_getenv("SDL_AUDIODRIVER");

	/* Check if user asked a different audio driver */
	if ((envr) && (SDL_strcmp(envr, MINT_AUDIO_DRIVER_NAME)!=0)) {
		DEBUG_PRINT((DEBUG_NAME "user asked a different audio driver\n"));
		return(0);
	}

	if (Locksnd()!=1) {
		DEBUG_PRINT((DEBUG_NAME "no XBIOS sound API, or audio in use\n"));
		return(0);
	}
	Unlocksnd();

	return(1);
}

static void Audio_DeleteDevice(SDL_AudioDevice *device)
{
    SDL_free(device->hidden);
    SDL_free(device);
}

static SDL_AudioDevice *Audio_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
    if ( this ) {
        SDL_memset(this, 0, (sizeof *this));
        this->hidden = (struct SDL_PrivateAudioData *)
                SDL_malloc((sizeof *this->hidden));
    }
    if ( (this == NULL) || (this->hidden == NULL) ) {
        SDL_OutOfMemory();
        if ( this ) {
            SDL_free(this);
        }
        return(0);
    }
    SDL_memset(this->hidden, 0, (sizeof *this->hidden));

    /* Set the function pointers */
    this->OpenAudio   = Mint_OpenAudio;
    this->CloseAudio  = Mint_CloseAudio;
    this->free        = Audio_DeleteDevice;

    return this;
}

AudioBootStrap MINTAUDIO_XBIOS_bootstrap = {
	MINT_AUDIO_DRIVER_NAME, "MiNT XBIOS audio driver",
	Audio_Available, Audio_CreateDevice
};

static void Mint_CloseAudio(_THIS)
{
	/* Stop replay */
	Buffoper(0);

	/* Uninstall interrupt */
#if !defined(__mcoldfire__)
	if (use_gsxb) {
		NSetinterrupt(SI_GSXB, SI_NONE, Mint_GsxbNullInterrupt);
	} else {
		Jdisint(MFP_DMASOUND);
	}
#endif

	/* Restore and unlock the sound system */
	USoundDeinitXbios(&usound_context);

	SDL_MintAudio_FreeBuffers();

	SDL_MintAudio_num_its = 0;
	SDL_MintAudio_device = NULL;
}

static int Mint_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	USoundSpec desired, obtained;

	DEBUG_PRINT((DEBUG_NAME "asked: %d bits, ",spec->format & 0x00ff));
	DEBUG_PRINT(("signed=%d, ", ((spec->format & 0x8000)!=0)));
	DEBUG_PRINT(("big endian=%d, ", ((spec->format & 0x1000)!=0)));
	DEBUG_PRINT(("channels=%d, ", spec->channels));
	DEBUG_PRINT(("freq=%d\n", spec->freq));

	if (!Mint_FormatToUSound(spec->format, &desired.format)) {
		SDL_SetError("Mint_OpenAudio: Unsupported audio format");
		return(-1);
	}
	desired.frequency = spec->freq;
	desired.channels  = spec->channels;
	desired.samples   = spec->samples;

	/* Lock, select format/frequency and set up the connection matrix.
	   This goes through the standard XBIOS Devconnect(), so it works on
	   real hardware (Falcon, FireBee, ...) instead of poking registers. */
	if (!USoundInitXbios(&desired, &obtained, &usound_context)) {
		SDL_SetError("Mint_OpenAudio: USoundInitXbios() failed");
		return(-1);
	}

	if (!Mint_FormatFromUSound(obtained.format, &spec->format)) {
		SDL_SetError("Mint_OpenAudio: Unsupported audio format");
		USoundDeinitXbios(&usound_context);
		return(-1);
	}
	spec->freq     = obtained.frequency;
	spec->channels = obtained.channels;

	DEBUG_PRINT((DEBUG_NAME "obtained: %d bits, ",spec->format & 0x00ff));
	DEBUG_PRINT(("signed=%d, ", ((spec->format & 0x8000)!=0)));
	DEBUG_PRINT(("big endian=%d, ", ((spec->format & 0x1000)!=0)));
	DEBUG_PRINT(("channels=%d, ", spec->channels));
	DEBUG_PRINT(("freq=%d\n", spec->freq));

	SDL_MintAudio_device = this;

	/* Allocate DMA buffers (also computes spec->size) */
	if (!SDL_MintAudio_InitBuffers(spec)) {
		USoundDeinitXbios(&usound_context);
		return(-1);
	}

	/* Set buffer */
	MINTAUDIO_swapbuf = Mint_SwapBuffers;
#if defined(__mcoldfire__)
	/* Both halves of the ring at once, replayed in a loop */
	Mint_SwapBuffers(MINTAUDIO_audiobuf[0], 2 * MINTAUDIO_audiosize);
#else
	Mint_SwapBuffers(MINTAUDIO_audiobuf[0], MINTAUDIO_audiosize);

	/* Install interrupt */
	use_gsxb = Mint_GsxbAvailable();
	if (use_gsxb) {
		if (NSetinterrupt(SI_GSXB, SI_PLAY, SDL_MintAudio_GsxbInterrupt)<0) {
			SDL_SetError("Mint_OpenAudio: GSXB refused the end of frame interrupt");
			SDL_MintAudio_FreeBuffers();
			USoundDeinitXbios(&usound_context);
			SDL_MintAudio_device = NULL;
			return(-1);
		}
	} else {
		Jdisint(MFP_DMASOUND);
		Xbtimer(XB_TIMERA, 8, 1, SDL_MintAudio_XbiosInterrupt);
		Jenabint(MFP_DMASOUND);

		if (Setinterrupt(SI_TIMERA, SI_PLAY)<0) {
			DEBUG_PRINT((DEBUG_NAME "Setinterrupt() failed\n"));
		}
	}
#endif

	/* Go */
	Buffoper(SB_PLA_ENA|SB_PLA_RPT);
	DEBUG_PRINT((DEBUG_NAME "hardware initialized\n"));

    return(1);	/* We don't use SDL threaded audio */
}

static void Mint_SwapBuffers(Uint8 *nextbuf, int nextsize)
{
	unsigned long buffer = (unsigned long) nextbuf;

	Setbuffer(0, buffer, buffer + nextsize);
}
