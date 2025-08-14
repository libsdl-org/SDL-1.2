    /*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

   Written by Troy Davis - GPF (troy_ed@yahoo.com)
*/
#include "SDL_config.h"

#ifdef SDL_AUDIO_DRIVER_DC_STREAM

#include "SDL_audio.h"
#include "../../SDL_audio_c.h"
#include "SDL_dreamcastaudio.h" // Defines SDL_PrivateAudioData struct
#include <dc/sound/stream.h> // Include KOS sound stream library
#include <kos/thread.h>
#include "../../SDL_sysaudio.h"
#include "SDL_timer.h"
#include "kos.h"

static int DCAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void DCAUD_CloseAudio(_THIS);
static void DCAUD_DeleteDevice(SDL_AudioDevice *device);
static Uint8 *DCAUD_GetAudioBuf(_THIS);

static SDL_AudioDevice *audioDevice = NULL; // Pointer to the active audio output device
static unsigned sdl_dc_mixbuffer;

static int DCAUDSTRM_Available(void) {
    return 1;
}


// Stream callback function for SDL 1.2
static void *stream_callback(snd_stream_hnd_t hnd, int req, int *done) {
    SDL_AudioDevice *device = audioDevice;
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)device->hidden;
    void *buffer = hidden->current_buffer; // Use the current buffer for audio processing
    int buffer_size = device->spec.size;
    int bytes_to_copy = SDL_min(req, buffer_size);
    void (SDLCALL *callback)(void*, Uint8*, int);

    SDL_LockMutex(device->mixer_lock);
    hidden->done_mixing=0;
    if (!buffer) {
        SDL_UnlockMutex(device->mixer_lock);
        *done = 0;
        return NULL;
    }

    callback = device->spec.callback;

    if (!device->enabled || device->paused) {
        SDL_memset(buffer, device->spec.silence, bytes_to_copy);
    } else {
        callback(device->spec.userdata, buffer, bytes_to_copy);
        if (bytes_to_copy < req) {
            SDL_memset((Uint8 *)buffer + bytes_to_copy, device->spec.silence, req - bytes_to_copy);
            bytes_to_copy = req;
        }
    }
    hidden->done_mixing=1;
    SDL_UnlockMutex(device->mixer_lock);

    *done = bytes_to_copy;
    return buffer;
}

// Audio playback thread function
static int DREAMCASTAUD_Thread(void *data) {
    SDL_AudioDevice *device = (SDL_AudioDevice *)data;
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)device->hidden;

    // Start playback
    while (device->enabled) {
        // Poll the sound stream
        snd_stream_poll(hidden->stream_handle);

        // Wait until the audio has finished playing
        SDL_Delay(1); // Adjust this delay as necessary for audio timing

        // Toggle buffer after playback is done
        // hidden->current_buffer = (hidden->current_buffer == hidden->buffer1) ? hidden->buffer2 : hidden->buffer1;
    }

    return 0; // Thread complete
}

// Wait for audio to finish playing
static void DCAUD_WaitAudio(_THIS) {
    if (!this->enabled || this->paused) {
        return;
    }        
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)this->hidden;
    snd_stream_poll(hidden->stream_handle);
    while (hidden->done_mixing != 0) {
        SDL_Delay(1); // Polling delay
    }
}

// Play the audio device
static void DCAUD_PlayAudio(_THIS) {
    if (!this->enabled || this->paused) {
        return;
    }        
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)this->hidden;
    snd_stream_poll(hidden->stream_handle);
    while (hidden->done_mixing != 0) {
        SDL_Delay(1); // Polling delay
    }
    if (hidden->done_mixing) {
        // Switch to the other buffer before starting playback
        hidden->current_buffer = (hidden->current_buffer == hidden->buffer1) ? hidden->buffer2 : hidden->buffer1;
        
        // Reset done_mixing flag for the new buffer
        hidden->done_mixing = 0; 
    }
}

// Initialize the SDL2 Dreamcast audio driver
static SDL_AudioDevice *DREAMCASTAUD_Init(int devindex) {
    SDL_AudioDevice *this;

    this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
    if (this) {
        SDL_memset(this, 0, (sizeof *this));
        this->hidden = (struct SDL_PrivateAudioData *)SDL_malloc((sizeof(SDL_PrivateAudioData)));
    }
    if ((this == NULL) || (this->hidden == NULL)) {
        SDL_OutOfMemory();
        if (this) {
            SDL_free(this);
        }
        return(0);
    }
    SDL_memset(this->hidden, 0, (sizeof(SDL_PrivateAudioData)));

    /* Set the function pointers */
    this->OpenAudio = DCAUD_OpenAudio;
    this->WaitAudio = DCAUD_WaitAudio;
    this->PlayAudio = DCAUD_PlayAudio; 
    this->GetAudioBuf = DCAUD_GetAudioBuf;

    this->CloseAudio = DCAUD_CloseAudio;
    this->free = DCAUD_DeleteDevice;

    return this;
}

AudioBootStrap DCAUDSTRM_bootstrap = {
    "dcstreamingaudio", "SDL Dreamcast Streaming Audio Driver",
    DCAUDSTRM_Available, DREAMCASTAUD_Init
};

// Open the audio device
static int DCAUD_OpenAudio(_THIS, SDL_AudioSpec *spec) {
    SDL_PrivateAudioData *hidden;
    Uint16 test_format;
    int channels, frequency;
    audioDevice = this;
    
    hidden = (SDL_PrivateAudioData *)SDL_malloc(sizeof(*hidden));
    if ((this == NULL) || (this->hidden == NULL)) {
        SDL_OutOfMemory();
        if (this) {
            SDL_free(this);
        }
        return(0);
    }
    this->hidden = (struct SDL_PrivateAudioData*)hidden;

    // Initialize sound stream system
    if (snd_stream_init() != 0) {
        SDL_free(hidden);
        return -1;
    }

    // Find a compatible format
    for (test_format = SDL_FirstAudioFormat(this->spec.format); test_format; test_format = SDL_NextAudioFormat()) {
        if ((test_format == AUDIO_S8) || (test_format == AUDIO_S16LSB)) {
            this->spec.format = test_format;
            break;
        }
    }

    if (!test_format) {
        snd_stream_shutdown();
        SDL_free(hidden);
        return -1;
    }

    SDL_CalculateAudioSpec(&this->spec);
    // Allocate the stream with the data callback
    hidden->stream_handle = snd_stream_alloc(stream_callback, this->spec.size);
    if (hidden->stream_handle == SND_STREAM_INVALID) {
        SDL_free(hidden);
        snd_stream_shutdown();
        return -1;
    }
    // Allocate two buffers for double buffering
    hidden->buffer1 = (Uint8 *)memalign(32, this->spec.size); // Ensure 32-byte alignment
    hidden->buffer2 = (Uint8 *)memalign(32, this->spec.size);
    if (!hidden->buffer1 || !hidden->buffer2) {
        SDL_free(hidden);
        snd_stream_shutdown();
        SDL_OutOfMemory();
        return -1;
    }

    hidden->current_buffer = hidden->buffer1; // Initialize current buffer
    sdl_dc_mixbuffer=(unsigned)hidden->current_buffer;
    snd_stream_volume(hidden->stream_handle, 255); // Max volume

    channels = this->spec.channels;
    frequency = this->spec.freq;

    // Initialize the stream based on format
    if (this->spec.format == AUDIO_S16LSB) {
        snd_stream_start(hidden->stream_handle, frequency, (channels == 2) ? 1 : 0);
    } else if (this->spec.format == AUDIO_S8) {
        snd_stream_start_pcm8(hidden->stream_handle, frequency, (channels == 2) ? 1 : 0);
    } else {
        SDL_SetError("Unsupported audio format: %d", this->spec.format);
        return -1;
    }
    
    // Start the playback thread
    this->enabled = 1; // Enable audio
    this->thread = SDL_CreateThread(DREAMCASTAUD_Thread, this);
    if (!this->thread) {
        this->enabled = 0;
        SDL_free(hidden);
        snd_stream_shutdown();
        return 0;
    }

    printf("Dreamcast audio driver initialized: %s\n", DCAUDSTRM_bootstrap.name);
    return 0;
}

// Close the audio device
static void DCAUD_CloseAudio(_THIS) {
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)this->hidden;

    printf("Closing audio device: %s\n", DCAUDSTRM_bootstrap.name);

    // Disable the audio device and wait for the thread to finish
    this->enabled = 0;
    if (this->thread) {
        SDL_WaitThread(this->thread, NULL);
        this->thread = NULL;
    }

    if (hidden) {
        if (hidden->stream_handle != SND_STREAM_INVALID) {
            snd_stream_destroy(hidden->stream_handle);
        }
        if (hidden->buffer1) {
            SDL_free(hidden->buffer1);
        }
        if (hidden->buffer2) {
            SDL_free(hidden->buffer2);
        }
        SDL_free(hidden);
        this->hidden = NULL;
    }

    audioDevice = NULL;

    snd_stream_shutdown();
}

static void DCAUD_DeleteDevice(SDL_AudioDevice *device) {
    SDL_free(device->hidden);
    SDL_free(device);
}

static Uint8 *DCAUD_GetAudioBuf(_THIS) {
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)this->hidden;
    return hidden->current_buffer; // Return the current buffer being used for playback
}

void SDL_DC_RestoreSoundBuffer(void)
{
    SDL_AudioDevice *device = (SDL_AudioDevice *)audioDevice;
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)device->hidden;

    // Lock the mixer to ensure the buffer is not in use
    SDL_LockMutex(device->mixer_lock);

    // Safely restore the current buffer
    hidden->current_buffer = (Uint8 *)sdl_dc_mixbuffer;

    // Unlock the mixer
    SDL_UnlockMutex(device->mixer_lock);
}

void SDL_DC_SetSoundBuffer(void *new_mixbuffer)
{
    SDL_AudioDevice *device = (SDL_AudioDevice *)audioDevice;
    SDL_PrivateAudioData *hidden = (SDL_PrivateAudioData *)device->hidden;

    // Lock the mixer to safely set the buffer
    SDL_LockMutex(device->mixer_lock);

    // Set the new buffer safely
    hidden->current_buffer = (Uint8 *)new_mixbuffer;
    // Reset done_mixing to indicate a new buffer has been set
    // hidden->done_mixing = 0;
    // Unlock the mixer after setting the buffer
    SDL_UnlockMutex(device->mixer_lock);
}

#endif // SDL_AUDIO_DRIVER_DC_STREAM
