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
*/
#include "SDL_config.h"

#ifndef SDL_dreamcastaudio_h_
#define SDL_dreamcastaudio_h_

#include "SDL_audio.h"
#include "../../SDL_sysaudio.h"
#include "SDL_mutex.h"
#include <stdint.h>  // For uint32_t
#include <dc/sound/stream.h>

/* Hidden "this" pointer for the audio functions */
#define _THIS	SDL_AudioDevice *this

typedef struct {
    snd_stream_hnd_t stream_handle;  // Handle for the audio stream
    void* current_buffer;
    void *buffer1;                    // Buffer for audio data
    void *buffer2;                    // Buffer for audio data
    volatile int done_mixing; // Flag to signal exit
} SDL_PrivateAudioData;


#endif /* SDL_dreamcastaudio_h_ */
/* vi: set ts=4 sw=4 expandtab: */
