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
#include "SDL_config.h"

#ifdef SDL_LOADSO_OS2

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* System dependent library loading routines                           */

#include <stdio.h>
#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#include <os2.h>

#include "SDL_loadso.h"

void *SDL_LoadObject(const char *sofile)
{
    HMODULE handle = NULLHANDLE;
    char buf[256];
    APIRET ulrc = DosLoadModule(buf, sizeof (buf), (char *) sofile, &handle);

    if (ulrc != NO_ERROR && !SDL_strrchr(sofile, '\\') && !SDL_strrchr(sofile, '/')) {
        /* strip .dll extension and retry only if name has no path. */
        size_t len = SDL_strlen(sofile);
        if (len > 4 && SDL_strcasecmp(&sofile[len - 4], ".dll") == 0) {
            char *ptr = SDL_stack_alloc(char, len);
            SDL_memcpy(ptr, sofile, len);
            ptr[len-4] = '\0';
            ulrc = DosLoadModule(buf, sizeof (buf), ptr, &handle);
            SDL_stack_free(ptr);
        }
    }

    /* Generate an error message if all loads failed */
    if (ulrc != NO_ERROR) {
        SDL_SetError("Failed loading %s: %s", sofile, buf);
        handle = NULLHANDLE;
    }

    return (void *)handle;
}

void *SDL_LoadFunction(void *handle, const char *name)
{
    PFN symbol = NULL;
    APIRET ulrc = DosQueryProcAddr((HMODULE)handle, 0, name, &symbol);

    if (ulrc != NO_ERROR) {
        /* retry with an underscore prepended, e.g. for gcc-built dlls. */
        size_t len = SDL_strlen(name) + 1;
        char *_name = SDL_stack_alloc(char, len + 1);
        _name[0] = '_';
        SDL_memcpy(&_name[1], name, len);
        ulrc = DosQueryProcAddr((HMODULE)handle, 0, _name, &symbol);
        SDL_stack_free(_name);
    }

    if (ulrc != NO_ERROR) {
        SDL_SetError("Failed loading procedure %s (E%u)", name, ulrc);
        symbol = NULL;
    }

    return (void *)symbol;
}

void SDL_UnloadObject(void *handle)
{
    if (handle != NULL) {
        DosFreeModule((HMODULE)handle);
    }
}

#endif /* SDL_LOADSO_OS2 */
