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

#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"

#include "SDL_mintaudio.h"

/*--- Defines ---*/

#define MINT_AUDIO_DRIVER_NAME "xbios"

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
static void Mint_LockAudio(_THIS);
static void Mint_UnlockAudio(_THIS);
static void Mint_SwapBuffers(Uint8 *nextbuf, int nextsize);

/*--- Format conversion helpers (SDL <-> AtariSoundSetup) ---*/

static int Mint_FormatToAtari(Uint16 sdl_format, AudioFormat *atari_format)
{
	switch (sdl_format) {
		case AUDIO_U8:     *atari_format = AudioFormatUnsigned8;     break;
		case AUDIO_S8:     *atari_format = AudioFormatSigned8;       break;
		case AUDIO_U16LSB: *atari_format = AudioFormatUnsigned16LSB; break;
		case AUDIO_S16LSB: *atari_format = AudioFormatSigned16LSB;   break;
		case AUDIO_U16MSB: *atari_format = AudioFormatUnsigned16MSB; break;
		case AUDIO_S16MSB: *atari_format = AudioFormatSigned16MSB;   break;
		default: return 0;
	}
	return 1;
}

static int Mint_FormatFromAtari(AudioFormat atari_format, Uint16 *sdl_format)
{
	switch (atari_format) {
		case AudioFormatUnsigned8:     *sdl_format = AUDIO_U8;     break;
		case AudioFormatSigned8:       *sdl_format = AUDIO_S8;     break;
		case AudioFormatUnsigned16LSB: *sdl_format = AUDIO_U16LSB; break;
		case AudioFormatSigned16LSB:   *sdl_format = AUDIO_S16LSB; break;
		case AudioFormatUnsigned16MSB: *sdl_format = AUDIO_U16MSB; break;
		case AudioFormatSigned16MSB:   *sdl_format = AUDIO_S16MSB; break;
		default: return 0;
	}
	return 1;
}

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
    this->LockAudio   = Mint_LockAudio;
    this->UnlockAudio = Mint_UnlockAudio;
    this->free        = Audio_DeleteDevice;

    return this;
}

AudioBootStrap MINTAUDIO_XBIOS_bootstrap = {
	MINT_AUDIO_DRIVER_NAME, "MiNT XBIOS audio driver",
	Audio_Available, Audio_CreateDevice
};

static void Mint_LockAudio(_THIS)
{
	/* Stop replay */
	Buffoper(0);
}

static void Mint_UnlockAudio(_THIS)
{
	/* Restart replay */
	Buffoper(SB_PLA_ENA|SB_PLA_RPT);
}

static void Mint_CloseAudio(_THIS)
{
	/* Stop replay */
	Buffoper(0);

	/* Uninstall interrupt */
	Jdisint(MFP_DMASOUND);

	/* Restore and unlock the sound system */
	AtariSoundSetupDeinitXbios();

	SDL_MintAudio_FreeBuffers();

	SDL_MintAudio_num_its = 0;
	SDL_MintAudio_device = NULL;
}

static int Mint_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	AudioSpec desired, obtained;

	DEBUG_PRINT((DEBUG_NAME "asked: %d bits, ",spec->format & 0x00ff));
	DEBUG_PRINT(("signed=%d, ", ((spec->format & 0x8000)!=0)));
	DEBUG_PRINT(("big endian=%d, ", ((spec->format & 0x1000)!=0)));
	DEBUG_PRINT(("channels=%d, ", spec->channels));
	DEBUG_PRINT(("freq=%d\n", spec->freq));

	if (!Mint_FormatToAtari(spec->format, &desired.format)) {
		SDL_SetError("Mint_OpenAudio: Unsupported audio format");
		return(-1);
	}
	desired.frequency = spec->freq;
	desired.channels  = spec->channels;
	desired.samples   = spec->samples;

	/* Lock, select format/frequency and set up the connection matrix.
	   This goes through the standard XBIOS Devconnect(), so it works on
	   real hardware (Falcon, FireBee, ...) instead of poking registers. */
	if (!AtariSoundSetupInitXbios(&desired, &obtained)) {
		SDL_SetError("Mint_OpenAudio: AtariSoundSetupInitXbios() failed");
		return(-1);
	}

	if (!Mint_FormatFromAtari(obtained.format, &spec->format)) {
		SDL_SetError("Mint_OpenAudio: Unsupported audio format");
		AtariSoundSetupDeinitXbios();
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
		AtariSoundSetupDeinitXbios();
		return(-1);
	}

	/* Set buffer */
	MINTAUDIO_swapbuf = Mint_SwapBuffers;
	Mint_SwapBuffers(MINTAUDIO_audiobuf[0], MINTAUDIO_audiosize);

	/* Install interrupt */
	Jdisint(MFP_DMASOUND);
	Xbtimer(XB_TIMERA, 8, 1, SDL_MintAudio_XbiosInterrupt);
	Jenabint(MFP_DMASOUND);

	if (Setinterrupt(SI_TIMERA, SI_PLAY)<0) {
		DEBUG_PRINT((DEBUG_NAME "Setinterrupt() failed\n"));
	}

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
