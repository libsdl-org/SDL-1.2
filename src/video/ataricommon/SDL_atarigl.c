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

/* Atari OSMesa implementation of SDL OpenGL support */

/*--- Includes ---*/

#if SDL_VIDEO_OPENGL
#include <GL/osmesa.h>

/* Keep OSMesa out of programs that don't use OpenGL; any gl*() call pulls it in */
#pragma weak OSMesaCreateContextExt
#pragma weak OSMesaDestroyContext
#pragma weak OSMesaMakeCurrent
#pragma weak OSMesaPixelStore
#pragma weak OSMesaGetProcAddress
#pragma weak glGetIntegerv
#pragma weak glFinish
#endif

#include "SDL_endian.h"
#include "SDL_video.h"
#include "SDL_atarigl_c.h"

/*--- Functions prototypes ---*/

#if SDL_VIDEO_OPENGL
static void ConvertNull(_THIS, SDL_Surface *surface);
static void Convert565To555be(_THIS, SDL_Surface *surface);
static void Convert565To555le(_THIS, SDL_Surface *surface);
static void Convert565le(_THIS, SDL_Surface *surface);
static void ConvertBGRAToABGR(_THIS, SDL_Surface *surface);
#endif

/*--- Public functions ---*/

int SDL_AtariGL_Init(_THIS, SDL_Surface *current)
{
#if SDL_VIDEO_OPENGL
	GLenum osmesa_format;
	SDL_PixelFormat *pixel_format;
	Uint32	redmask;
	int recreatecontext;
	GLint newaccumsize;

	gl_active = 0;

	if (OSMesaCreateContextExt == NULL) {
		return 0;
	}

	/* SDL_SetVideoMode() looks up GL functions via SDL_GL_GetProcAddress() */
	this->gl_config.driver_loaded = 1;

	/* Init OpenGL context using OSMesa */
	gl_convert = ConvertNull;

	pixel_format = current->format;
	redmask = pixel_format->Rmask;
	switch (pixel_format->BitsPerPixel) {
		case 15:
			/* 1555, big and little endian, unsupported */
			gl_pixelsize = 2;
			osmesa_format = OSMESA_RGB_565;
			if (redmask == 31<<10) {
				gl_convert = Convert565To555be;
			} else {
				gl_convert = Convert565To555le;
			}
			break;
		case 16:
			gl_pixelsize = 2;
			if (redmask == 31<<11) {
				osmesa_format = OSMESA_RGB_565;
			} else {
				/* 565, little endian, unsupported */
				osmesa_format = OSMESA_RGB_565;
				gl_convert = Convert565le;
			}
			break;
		case 24:
			gl_pixelsize = 3;
			if (redmask == 255<<16) {
				osmesa_format = OSMESA_RGB;
			} else {
				osmesa_format = OSMESA_BGR;
			}
			break;
		case 32:
			gl_pixelsize = 4;
			if (redmask == 255<<16) {
				osmesa_format = OSMESA_ARGB;
			} else if (redmask == 255<<8) {
				osmesa_format = OSMESA_BGRA;
			} else if (redmask == 255<<24) {
				osmesa_format = OSMESA_RGBA;
			} else {
				/* ABGR format unsupported */
				osmesa_format = OSMESA_BGRA;
				gl_convert = ConvertBGRAToABGR;
			}
			break;
		default:
			gl_pixelsize = 1;
			osmesa_format = OSMESA_COLOR_INDEX;
			break;
	}

	/* Try to keep current context if possible */
	newaccumsize =
		this->gl_config.accum_red_size +
		this->gl_config.accum_green_size +
		this->gl_config.accum_blue_size +
		this->gl_config.accum_alpha_size;
	recreatecontext=1;
	if (gl_ctx &&
		(gl_curformat == osmesa_format) &&
		(gl_curdepth == this->gl_config.depth_size) &&
		(gl_curstencil == this->gl_config.stencil_size) &&
		(gl_curaccum == newaccumsize)) {
		recreatecontext = 0;
	}
	if (recreatecontext) {
		SDL_AtariGL_Quit(this);

		gl_ctx = OSMesaCreateContextExt(
			osmesa_format, this->gl_config.depth_size,
			this->gl_config.stencil_size, newaccumsize, NULL );

		if (gl_ctx) {
			gl_curformat = osmesa_format;
			gl_curdepth = this->gl_config.depth_size;
			gl_curstencil = this->gl_config.stencil_size;
			gl_curaccum = newaccumsize;
		} else {
			gl_curformat = 0;
			gl_curdepth = 0;
			gl_curstencil = 0;
			gl_curaccum = 0;
		}
	}

	gl_active = (gl_ctx != NULL);
#endif

	return (gl_active);
}

void SDL_AtariGL_Quit(_THIS)
{
#if SDL_VIDEO_OPENGL
	if (gl_ctx) {
		OSMesaDestroyContext(gl_ctx);
		gl_ctx = NULL;
	}
#endif /* SDL_VIDEO_OPENGL */
	gl_active = 0;
}

int SDL_AtariGL_LoadLibrary(_THIS, const char *path)
{
#if SDL_VIDEO_OPENGL
	if (path) {
		SDL_SetError("Loading a specific OpenGL library is not supported");
		return -1;
	}

	if (OSMesaCreateContextExt == NULL) {
		SDL_SetError("OpenGL support not linked in");
		return -1;
	}

	this->gl_config.driver_loaded = 1;

	return 0;
#else
	return -1;
#endif
}

void *SDL_AtariGL_GetProcAddress(_THIS, const char *proc)
{
	void *func = NULL;
#if SDL_VIDEO_OPENGL

	if (OSMesaGetProcAddress) {
		func = OSMesaGetProcAddress(proc);
	}

#endif
	return func;
}

int SDL_AtariGL_GetAttribute(_THIS, SDL_GLattr attrib, int* value)
{
#if SDL_VIDEO_OPENGL
	GLenum mesa_attrib;
	SDL_Surface *surface;

	if (!gl_active) {
		return -1;
	}

	switch(attrib) {
		case SDL_GL_RED_SIZE:
			mesa_attrib = GL_RED_BITS;
			break;
		case SDL_GL_GREEN_SIZE:
			mesa_attrib = GL_GREEN_BITS;
			break;
		case SDL_GL_BLUE_SIZE:
			mesa_attrib = GL_BLUE_BITS;
			break;
		case SDL_GL_ALPHA_SIZE:
			mesa_attrib = GL_ALPHA_BITS;
			break;
		case SDL_GL_DOUBLEBUFFER:
			surface = this->screen;
			*value = ((surface->flags & SDL_DOUBLEBUF)==SDL_DOUBLEBUF);
			return 0;
		case SDL_GL_DEPTH_SIZE:
			mesa_attrib = GL_DEPTH_BITS;
			break;
		case SDL_GL_STENCIL_SIZE:
			mesa_attrib = GL_STENCIL_BITS;
			break;
		case SDL_GL_ACCUM_RED_SIZE:
			mesa_attrib = GL_ACCUM_RED_BITS;
			break;
		case SDL_GL_ACCUM_GREEN_SIZE:
			mesa_attrib = GL_ACCUM_GREEN_BITS;
			break;
		case SDL_GL_ACCUM_BLUE_SIZE:
			mesa_attrib = GL_ACCUM_BLUE_BITS;
			break;
		case SDL_GL_ACCUM_ALPHA_SIZE:
			mesa_attrib = GL_ACCUM_ALPHA_BITS;
			break;
		default :
			return -1;
	}

	glGetIntegerv(mesa_attrib, value);
	return 0;
#else
	return -1;
#endif
}

int SDL_AtariGL_MakeCurrent(_THIS)
{
#if SDL_VIDEO_OPENGL
	SDL_Surface *surface;
	GLenum type;

	if (!gl_active) {
		SDL_SetError("Invalid OpenGL context");
		return -1;
	}

	surface = this->screen;
	
	if ((surface->format->BitsPerPixel == 15) || (surface->format->BitsPerPixel == 16)) {
		type = GL_UNSIGNED_SHORT_5_6_5;
	} else {
		type = GL_UNSIGNED_BYTE;
	}

	if (!(OSMesaMakeCurrent(gl_ctx, (Uint8 *)surface->pixels + surface->offset, type, surface->w, surface->h))) {
		SDL_SetError("Can not make OpenGL context current");
		return -1;
	}

	/* OSMesa draws upside down */
	OSMesaPixelStore(OSMESA_Y_UP, 0);

	/* The surface can be a part of a larger buffer */
	OSMesaPixelStore(OSMESA_ROW_LENGTH, surface->pitch / gl_pixelsize);

	return 0;
#else
	return -1;
#endif
}

void SDL_AtariGL_SwapBuffers(_THIS)
{
#if SDL_VIDEO_OPENGL
	if (gl_active) {
		glFinish();
		gl_convert(this, this->screen);
	}
#endif
}

#if SDL_VIDEO_OPENGL

/*--- Conversions routines in the screen ---*/

static void ConvertNull(_THIS, SDL_Surface *surface)
{
}

static void Convert565To555be(_THIS, SDL_Surface *surface)
{
	int x,y, pitch;
	unsigned short *line, *pixel;

	line = (void *)((Uint8 *)surface->pixels + surface->offset);
	pitch = surface->pitch >> 1;
	for (y=0; y<surface->h; y++) {
		pixel = line;
		for (x=0; x<surface->w; x++) {
			unsigned short color = *pixel;

			*pixel++ = (color & 0x1f)|((color>>1) & 0xffe0);
		}

		line += pitch;
	}
}

static void Convert565To555le(_THIS, SDL_Surface *surface)
{
	int x,y, pitch;
	unsigned short *line, *pixel;

	line = (void *)((Uint8 *)surface->pixels + surface->offset);
	pitch = surface->pitch >>1;
	for (y=0; y<surface->h; y++) {
		pixel = line;
		for (x=0; x<surface->w; x++) {
			unsigned short color = *pixel;

			color = (color & 0x1f)|((color>>1) & 0xffe0);
			*pixel++ = SDL_Swap16(color);
		}

		line += pitch;
	}
}

static void Convert565le(_THIS, SDL_Surface *surface)
{
	int x,y, pitch;
	unsigned short *line, *pixel;

	line = (void *)((Uint8 *)surface->pixels + surface->offset);
	pitch = surface->pitch >>1;
	for (y=0; y<surface->h; y++) {
		pixel = line;
		for (x=0; x<surface->w; x++) {
			unsigned short color = *pixel;

			*pixel++ = SDL_Swap16(color);
		}

		line += pitch;
	}
}

static void ConvertBGRAToABGR(_THIS, SDL_Surface *surface)
{
	int x,y, pitch;
	unsigned long *line, *pixel;

	line = (void *)((Uint8 *)surface->pixels + surface->offset);
	pitch = surface->pitch >>2;
	for (y=0; y<surface->h; y++) {
		pixel = line;
		for (x=0; x<surface->w; x++) {
			unsigned long color = *pixel;

			*pixel++ = (color<<24)|(color>>8);
		}

		line += pitch;
	}
}

#endif /* SDL_VIDEO_OPENGL */
