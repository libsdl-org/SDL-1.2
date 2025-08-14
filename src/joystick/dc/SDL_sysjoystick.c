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

    BERO
    bero@geocities.co.jp

    based on win32/SDL_mmjoystick.c

    Sam Lantinga
    slouken@libsdl.org
*/

#include <kos.h>

#include <stdio.h>		/* For the definition of NULL */
#include <stdlib.h>
#include <string.h>

#include "SDL_config.h"

#ifdef SDL_JOYSTICK_DC

#include "SDL_error.h"
#include "SDL_sysevents.h"
#include "SDL_events_c.h"
#include "SDL_joystick.h"
#include "SDL_sysjoystick.h"
#include "SDL_joystick_c.h"

#include "SDL_dreamcast.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

#define MIN_FRAME_UPDATE 16
extern unsigned __sdl_dc_mouse_shift;

#define MAX_JOYSTICKS	8	/* only 2 are supported in the multimedia API */
#define MAX_AXES	6	/* each joystick can have up to 6 axes */
#define MAX_BUTTONS	8	/* and 8 buttons                      */
#define	MAX_HATS	2

#define	JOYNAMELEN	8

static SDLKey _dc_sdl_key[MAX_JOYSTICKS][13]= {
  { SDLK_ESCAPE, SDLK_LALT, SDLK_LCTRL, SDLK_RETURN, SDLK_3, SDLK_SPACE, SDLK_LSHIFT,
	  SDLK_TAB, SDLK_BACKSPACE, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT },
  { SDLK_ESCAPE, SDLK_q,    SDLK_e,     SDLK_z,      SDLK_3, SDLK_x,     SDLK_c,
	  SDLK_1,   SDLK_2,         SDLK_w,  SDLK_s,    SDLK_a,    SDLK_d },
  { SDLK_ESCAPE, SDLK_r,    SDLK_y,     SDLK_v,      SDLK_3, SDLK_b,     SDLK_n,
	  SDLK_4,   SDLK_5,         SDLK_t,  SDLK_g,    SDLK_f,    SDLK_h },
  { SDLK_ESCAPE, SDLK_u,    SDLK_o,     SDLK_m,      SDLK_3, SDLK_COMMA, SDLK_PERIOD,
	  SDLK_8,   SDLK_9,         SDLK_i,  SDLK_k,    SDLK_j,    SDLK_l }
};

void SDL_DC_MapKey(int joy, SDL_DC_button button, SDLKey key)
{
	if ((joy<MAX_JOYSTICKS)&&(joy>=0))
		if (((int)button<13)&&((int)button>=0))
			_dc_sdl_key[joy][(int)button]=key;
}


/* array to hold devices */
static maple_device_t * SYS_Joystick_addr[MAX_JOYSTICKS];

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{
	int prev_buttons;
	int ltrig;
	int rtrig;
	int joyx;
	int joyy;
	int joy2x;
	int joy2y;
};


// Initial keyboard position
int vk_row = 0;
int vk_col = 0;
int virtual_keyboard_visible = 0;
bool shiftActive = false;
int current_row = 0;
int current_col = 0;

/* Function to scan the system for joysticks.
 * This function should set SDL_numjoysticks to the number of available
 * joysticks.  Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */

int __sdl_dc_emulate_keyboard=1, __sdl_dc_emulate_mouse=1,__sdl_dc_emulate_vhkb_keyboard=0;
void toggleVirtualKeyboard();
void SDL_DC_EmulateVirtualKeyboard(SDL_bool value)
{
	__sdl_dc_emulate_vhkb_keyboard=(int)value;
    toggleVirtualKeyboard();
}

void SDL_DC_EmulateKeyboard(SDL_bool value)
{
	__sdl_dc_emulate_keyboard=(int)value;
}

void SDL_DC_EmulateMouse(SDL_bool value)
{
	__sdl_dc_emulate_mouse=(int)value;
}



int SDL_SYS_JoystickInit(void)
{
	int numdevs;
	int i;

	numdevs = 0;
	//Update KOS Maple
	int n = maple_enum_count();
	for (i=0;i<n;i++){
		maple_device_t *cont = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
		if (cont){
			SYS_Joystick_addr[numdevs] = cont;
			numdevs++;
		}
		else{
			maple_device_t *keyb = maple_enum_type(i, MAPLE_FUNC_KEYBOARD); 
			if (keyb) __sdl_dc_emulate_keyboard=0;
			maple_device_t *mouse = maple_enum_type(i, MAPLE_FUNC_MOUSE); 
			if (mouse) __sdl_dc_emulate_mouse=0;
		}
	}
	return(numdevs);
}


/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	maple_device_t *dev = SYS_Joystick_addr[index];
	if (dev && dev->info.functions & MAPLE_FUNC_CONTROLLER) {
		return dev->info.product_name;
	}
	else return NULL;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	/* allocate memory for system specific hardware data */
	joystick->hwdata = (struct joystick_hwdata *) malloc(sizeof(*joystick->hwdata));
	if (joystick->hwdata == NULL)
	{
		SDL_OutOfMemory();
		return(-1);
	}
	memset(joystick->hwdata, 0, sizeof(*joystick->hwdata));

	/* fill nbuttons, naxes, and nhats fields */
	joystick->nbuttons = MAX_BUTTONS;
	joystick->naxes = MAX_AXES;
	joystick->nhats = MAX_HATS;

	return(0);
}


///Virtual keyboard draw
// Virtual keyboard navigation directions
typedef enum {
    VK_UP,
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,
    VK_SELECT
} VKNavigateDir;

#define VK_ROWS 5
#define VK_COLS 14

const char* keyboardLayout[VK_ROWS][VK_COLS] = {
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "BS","↑"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\\","↓"},
    {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "|", "←"},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "", "SH", "→"},
    {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "SPC", "", "","ENTER"}
};

const char* shiftKeyboardLayout[VK_ROWS][VK_COLS] = {
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "BS","↑"},
    {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "|","↓"},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|", "←"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "", "SH", "→"},
    {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "SPC", "", "","ENTER"}
};

void bfont_draw_box(void *buffer, uint32 bufwidth, int x, int y, int width, int height, uint8 opaque, uint32 color) {
    int i, j;
    uint32 old_fg = bfont_set_foreground_color(color);
    uint32 *buf = (uint32*)buffer;

    // Draw top and bottom edges
    for (i = 0; i < width; ++i) {
        bfont_draw(buf + y * bufwidth + x + i, bufwidth, opaque, ' ');
        bfont_draw(buf + (y + height - 1) * bufwidth + x + i, bufwidth, opaque, ' ');
    }

    // Draw left and right edges
    for (j = 1; j < height - 1; ++j) {
        bfont_draw(buf + (y + j) * bufwidth + x, bufwidth, opaque, ' ');
        bfont_draw(buf + (y + j) * bufwidth + x + width - 1, bufwidth, opaque, ' ');
    }

    // Fill the interior
    for (j = 1; j < height - 1; ++j) {
        for (i = 1; i < width - 1; ++i) {
            bfont_draw(buf + (y + j) * bufwidth + x + i, bufwidth, opaque, ' ');
        }
    }

    bfont_set_foreground_color(old_fg);
}

// Function to draw the virtual keyboard
void drawVirtualKeyboard() {
    int x_start = 25;
    int y_start = 325;
    int line_spacing = 20;
    int width = 640;
    int key_spacing = 15;  // Space between keys
    uint8 opaque = 1;      // Set opaque to true for drawing the box
    uint32 normalColor = 0x000FFF; // Color for the highlight box (e.g., white)
    uint32 highlightColor = 0x000000; // Normal key color (e.g., black)

    for (int i = 0; i < VK_ROWS; ++i) {
        const char **row = (shiftActive) ? shiftKeyboardLayout[i] : keyboardLayout[i];
        int x = x_start;
        int y = y_start + i * line_spacing;

        for (int j = 0; j < VK_COLS; ++j) {
            if (row[j] == NULL) break; // Stop if there's no more keys in this row

            int key_width = BFONT_THIN_WIDTH; // Default key width

            // Draw the key's highlight if it's the selected one
            if (i == vk_row && j == vk_col) {
                // Draw background highlight
                bfont_draw_box(vram_s, width, x - 2, y, key_width + 4, line_spacing - 2, opaque, highlightColor);
                bfont_set_foreground_color(0xFFFFFF); // Set the foreground color for the key
            } else {
                bfont_set_foreground_color(normalColor); // White color for normal
            }

            // Draw the key
            const char* key = row[j];
            if (strcmp(key, "↑") == 0) {
                // bfont_set_encoding(BFONT_CODE_RAW);
				// bfont_set_foreground_color(0x000000); // White color for normal
                bfont_draw_str(vram_s + y * width + x, width, !opaque, "^");
				// bfont_set_foreground_color(0xFFFFFF); // Black color for highlight
                // bfont_set_encoding(BFONT_CODE_ISO8859_1);
            } else if (strcmp(key, "↓") == 0) {
                // bfont_set_encoding(BFONT_CODE_RAW);
                bfont_draw_str(vram_s + y * width + x, width, !opaque, "v");
                // bfont_set_encoding(BFONT_CODE_ISO8859_1);
            } else if (strcmp(key, "←") == 0) {
                // bfont_set_encoding(BFONT_CODE_RAW);
                bfont_draw_str(vram_s + y * width + x, width, !opaque, "<");
                // bfont_set_encoding(BFONT_CODE_ISO8859_1);
            } else if (strcmp(key, "→") == 0) {
                // bfont_set_encoding(BFONT_CODE_RAW);
                bfont_draw_str(vram_s + y * width + x, width, !opaque, ">");
                // bfont_set_encoding(BFONT_CODE_ISO8859_1);
            } else {
                bfont_draw_str(vram_s + y * width + x, width, !opaque, key);
            }

            x += key_width + key_spacing; // Move to the next character position

            // Add space after each key
            if (row[j + 1] != NULL) {
                x += key_spacing; // Add extra space between keys
            }
        }
    }
}

unsigned short getVKBD_SDLKeySym(const char* selectedChar) {
    if (strcmp(selectedChar, "SPC") == 0) return SDLK_SPACE;
    if (strcmp(selectedChar, "BS") == 0) return SDLK_BACKSPACE;
    if (strcmp(selectedChar, "ENTER") == 0) return SDLK_RETURN; // Add this line
    if (strcmp(selectedChar, "-") == 0) return SDLK_MINUS;
    if (strcmp(selectedChar, "=") == 0) return SDLK_EQUALS;
    if (strcmp(selectedChar, "_") == 0) return SDLK_UNDERSCORE;
    if (strcmp(selectedChar, "+") == 0) return SDLK_PLUS;
	//	Add mappings for cursor keys
    if (strcmp(selectedChar, "↑") == 0) return SDLK_UP;
    if (strcmp(selectedChar, "←") == 0) return SDLK_LEFT;
    if (strcmp(selectedChar, "→") == 0) return SDLK_RIGHT;
    if (strcmp(selectedChar, "↓") == 0) return SDLK_DOWN;
    // Add mappings for function keys
    if (strcmp(selectedChar, "F1") == 0) return SDLK_F1;
    if (strcmp(selectedChar, "F2") == 0) return SDLK_F2;
    if (strcmp(selectedChar, "F3") == 0) return SDLK_F3;
    if (strcmp(selectedChar, "F4") == 0) return SDLK_F4;
    if (strcmp(selectedChar, "F5") == 0) return SDLK_F5;
    if (strcmp(selectedChar, "F6") == 0) return SDLK_F6;
    if (strcmp(selectedChar, "F7") == 0) return SDLK_F7;
    if (strcmp(selectedChar, "F8") == 0) return SDLK_F8;
    if (strcmp(selectedChar, "F9") == 0) return SDLK_F9;
    if (strcmp(selectedChar, "F10") == 0) return SDLK_F10;
    if (strcmp(selectedChar, "F11") == 0) return SDLK_F11;
    if (strcmp(selectedChar, "F12") == 0) return SDLK_F12;

    // Add more mappings for other special keys as necessary
    if (strcmp(selectedChar, "[") == 0) return SDLK_LEFTBRACKET;
    if (strcmp(selectedChar, "]") == 0) return SDLK_RIGHTBRACKET;
    if (strcmp(selectedChar, "\\") == 0) return SDLK_BACKSLASH;
    if (strcmp(selectedChar, ";") == 0) return SDLK_SEMICOLON;
    if (strcmp(selectedChar, "'") == 0) return SDLK_QUOTE;
    if (strcmp(selectedChar, ",") == 0) return SDLK_COMMA;
    if (strcmp(selectedChar, ".") == 0) return SDLK_PERIOD;
    if (strcmp(selectedChar, "/") == 0) return SDLK_SLASH;

    // For alphabetic characters
    if (selectedChar[0] >= 'a' && selectedChar[0] <= 'z') {
        return selectedChar[0];
    }

    // For uppercase alphabetic characters
    if (selectedChar[0] >= 'A' && selectedChar[0] <= 'Z') {
        return selectedChar[0];
    }

    // For numeric characters
    if (selectedChar[0] >= '0' && selectedChar[0] <= '9') {
        return selectedChar[0];
    }

    // Return 0 if no matching key is found
    return 0;
}


// Function to handle virtual keyboard navigation
char navigateVirtualKeyboard(VKNavigateDir dir) {
    int new_row = vk_row;
    int new_col = vk_col;

    // Update the new row and column based on direction
    switch (dir) {
        case VK_UP:
            new_row = (vk_row > 0) ? vk_row - 1 : VK_ROWS - 1;
            break;
        case VK_DOWN:
            new_row = (vk_row < VK_ROWS - 1) ? vk_row + 1 : 0;
            break;
        case VK_LEFT:
            new_col = (vk_col > 0) ? vk_col - 1 : VK_COLS - 1;
            break;
        case VK_RIGHT:
            new_col = (vk_col < VK_COLS - 1) ? vk_col + 1 : 0;
            break;
		case VK_SELECT: {
			const char* selectedChar = shiftActive ? shiftKeyboardLayout[vk_row][vk_col] : keyboardLayout[vk_row][vk_col];

			// Handle the "SH" key to toggle shift state
			if (strcmp(selectedChar, "SH") == 0) {
				shiftActive = !shiftActive;  // Toggle SHIFT state
				return '\0'; // Do not return the character for "SH"
			} else {
				unsigned short sdlChar = getVKBD_SDLKeySym(selectedChar);

				if (sdlChar != 0) {
					SDL_keysym keysym;
					keysym.sym = sdlChar;

					// Send the SDL key event for the selected character
					SDL_PrivateKeyboard(SDL_PRESSED, &keysym);
					SDL_PrivateKeyboard(SDL_RELEASED, &keysym);
				}

				return selectedChar[0]; // Return the selected character
			}
		}

    }

    // Check for valid keys and skip invalid spaces
    while (keyboardLayout[new_row][new_col][0] == '\0') {
        switch (dir) {
            case VK_UP:
                new_row = (new_row > 0) ? new_row - 1 : VK_ROWS - 1;
                break;
            case VK_DOWN:
                new_row = (new_row < VK_ROWS - 1) ? new_row + 1 : 0;
                break;
            case VK_LEFT:
                new_col = (new_col > 0) ? new_col - 1 : VK_COLS - 1;
                break;
            case VK_RIGHT:
                new_col = (new_col < VK_COLS - 1) ? new_col + 1 : 0;
                break;
            case VK_SELECT:
                break;                
        }

        // Prevent out-of-bounds access
        if (new_row < 0 || new_row >= VK_ROWS || new_col < 0 || new_col >= VK_COLS) {
            break;
        }
    }

    // Update the current position only if it's a valid key
    if (keyboardLayout[new_row][new_col][0] != '\0') {
        vk_row = new_row;
        vk_col = new_col;
    }

    drawVirtualKeyboard(); // Redraw the keyboard with the updated position

    return '\0'; // Return null character if not selecting
}


int state1 = 0; // For keyboard emulation
int state2 = 0; // For mouse emulation

// Virtual keyboard toggle function
void toggleVirtualKeyboard() {
    virtual_keyboard_visible = !virtual_keyboard_visible;
    if (virtual_keyboard_visible) {
        // Preserve the emulation state
        state1 = __sdl_dc_emulate_keyboard;
        state2 = __sdl_dc_emulate_mouse;
        __sdl_dc_emulate_keyboard = 0;
        __sdl_dc_emulate_mouse = 0;		
    } else {
        // Restore the emulation state
        __sdl_dc_emulate_keyboard = state1;
        __sdl_dc_emulate_mouse = state2;
    }
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */

static void joyUpdate(SDL_Joystick *joystick) {
    SDL_keysym keysym;
    int count_cond;
    static int count=1;
    static int mx=2048,my=2048;
    static int escaped=0;
    const int sdl_buttons[] = {
        CONT_C,
        CONT_B,
        CONT_A,
        CONT_START,
        CONT_Z,
        CONT_Y,
        CONT_X,
        CONT_D
    };

    cont_state_t *cond;
	int buttons,prev_buttons,i,max,changed;
    int prev_ltrig = 0, prev_rtrig = 0, prev_joyx = 0, prev_joyy = 0, prev_joy2x = 0, prev_joy2y = 0;


	//Get buttons of the Controller
	maple_device_t * dev = SYS_Joystick_addr[joystick->index];
    cond = (cont_state_t *)maple_dev_status(dev);

    if (!cond) return;

	//Check changes on cont_state_t->buttons
    buttons = cond->buttons;
    prev_buttons = joystick->hwdata->prev_buttons;
	changed = buttons^prev_buttons;

// Handle virtual keyboard navigation if visible
if (virtual_keyboard_visible) {
    static int lastNavState = 0;
    static int navTimer = 0; // Timer for continuous navigation
    static const int navDelay = 10; // Initial delay before continuous navigation starts
    static const int navInterval = 5; // Interval for continuous navigation

    // Check for direction button state changes
    int navState = buttons & (CONT_DPAD_UP | CONT_DPAD_DOWN | CONT_DPAD_LEFT | CONT_DPAD_RIGHT);
    if (navState != lastNavState) {
        lastNavState = navState;
        navTimer = 0; // Reset timer on state change
    }

    // Handle continuous navigation
    if (lastNavState) {
        navTimer++;

        // Check for initial delay
        if (navTimer > navDelay && (navTimer % navInterval == 0)) {
            VKNavigateDir dir = -1;

            if (lastNavState & CONT_DPAD_UP) dir = VK_UP;
            else if (lastNavState & CONT_DPAD_DOWN) dir = VK_DOWN;
            else if (lastNavState & CONT_DPAD_LEFT) dir = VK_LEFT;
            else if (lastNavState & CONT_DPAD_RIGHT) dir = VK_RIGHT;

            // Navigate continuously after the initial delay
            navigateVirtualKeyboard(dir);
        } else if (navTimer == 1) {
            // Immediate navigation on first press
            VKNavigateDir dir = -1;

            if (lastNavState & CONT_DPAD_UP) dir = VK_UP;
            else if (lastNavState & CONT_DPAD_DOWN) dir = VK_DOWN;
            else if (lastNavState & CONT_DPAD_LEFT) dir = VK_LEFT;
            else if (lastNavState & CONT_DPAD_RIGHT) dir = VK_RIGHT;

            navigateVirtualKeyboard(dir);
        }
    } else {
        navTimer = 0; // Reset timer if no direction button is pressed
    }

    // Handle character selection
    if ((changed & CONT_A) && (buttons & CONT_A)) { // Only act on press
        char selectedChar = navigateVirtualKeyboard(VK_SELECT);
        if (selectedChar != '\0') {
            printf("Selected character: %c\n", selectedChar);
        }
    }
            drawVirtualKeyboard(); 
	// return; // dont generate any other inputs when VKB is active
}



    // Check directions for HAT
    if ((changed) & (CONT_DPAD_UP | CONT_DPAD_DOWN | CONT_DPAD_LEFT | CONT_DPAD_RIGHT)) {
        int hat = SDL_HAT_CENTERED;
		/*
    if (!(buttons&CONT_DPAD_UP)) hat|=SDL_HAT_UP;
		if (!(buttons&CONT_DPAD_DOWN)) hat|=SDL_HAT_DOWN;
		if (!(buttons&CONT_DPAD_LEFT)) hat|=SDL_HAT_LEFT;
		if (!(buttons&CONT_DPAD_RIGHT)) hat|=SDL_HAT_RIGHT;
    */
		if ((buttons&CONT_DPAD_UP)) hat|=SDL_HAT_UP;
		if ((buttons&CONT_DPAD_DOWN)) hat|=SDL_HAT_DOWN;
		if ((buttons&CONT_DPAD_LEFT)) hat|=SDL_HAT_LEFT;
		if ((buttons&CONT_DPAD_RIGHT)) hat|=SDL_HAT_RIGHT;
        if (!virtual_keyboard_visible) {
            SDL_PrivateJoystickHat(joystick, 0, hat);
        }
    }
	if ((changed)&(CONT_DPAD2_UP|CONT_DPAD2_DOWN|CONT_DPAD2_LEFT|CONT_DPAD2_RIGHT)) {
        int hat = SDL_HAT_CENTERED;
		if (!(buttons&CONT_DPAD2_UP)) hat|=SDL_HAT_UP;
		if (!(buttons&CONT_DPAD2_DOWN)) hat|=SDL_HAT_DOWN;
		if (!(buttons&CONT_DPAD2_LEFT)) hat|=SDL_HAT_LEFT;
		if (!(buttons&CONT_DPAD2_RIGHT)) hat|=SDL_HAT_RIGHT;
        if (!virtual_keyboard_visible) {
            SDL_PrivateJoystickHat(joystick, 1, hat);
        }
    }

	//Check buttons
	//"buttons" is zero based: so invert the PRESSED/RELEASED way.
	for(i=0,max=0;i<sizeof(sdl_buttons)/sizeof(sdl_buttons[0]);i++) {
        if (changed & sdl_buttons[i]) {
			int act=(buttons & sdl_buttons[i]);
            if (!virtual_keyboard_visible) {
			    SDL_PrivateJoystickButton(joystick, i, act?SDL_PRESSED:SDL_RELEASED);
            }
            if (__sdl_dc_emulate_mouse)
				SDL_PrivateMouseButton(act?SDL_PRESSED:SDL_RELEASED,i,0,0);
			if (__sdl_dc_emulate_keyboard)
			{
                if (act)
                    max++;
				if (max==4) 
				{
                    keysym.sym = SDLK_ESCAPE;
					SDL_PrivateKeyboard(SDL_PRESSED,&keysym);
					max=0;
					escaped=1;
                }
                keysym.sym = _dc_sdl_key[joystick->index][i];
				SDL_PrivateKeyboard(act?SDL_PRESSED:SDL_RELEASED,&keysym);
            }
        }
    }
	if (escaped)
	{
		if (escaped==2)
		{
            keysym.sym = SDLK_ESCAPE;
            SDL_PrivateKeyboard(SDL_RELEASED, &keysym);
            escaped = 0;
        } else
            escaped = 2;
    }

	//If we emulate keyboard, check for SDL_DC DPAD
	//Also here invert the states!
	if (__sdl_dc_emulate_keyboard)
	{
		if (changed & CONT_DPAD_UP)
		{
			keysym.sym=_dc_sdl_key[joystick->index][9];
			SDL_PrivateKeyboard((buttons & CONT_DPAD_UP)?SDL_PRESSED:SDL_RELEASED,&keysym);
        }
		if (changed & CONT_DPAD_DOWN)
		{
			keysym.sym=_dc_sdl_key[joystick->index][10];
			SDL_PrivateKeyboard((buttons & CONT_DPAD_DOWN)?SDL_PRESSED:SDL_RELEASED,&keysym);
        }
		if (changed & CONT_DPAD_LEFT)
		{
			keysym.sym=_dc_sdl_key[joystick->index][11];
			SDL_PrivateKeyboard((buttons & CONT_DPAD_LEFT)?SDL_PRESSED:SDL_RELEASED,&keysym);
        }
		if (changed & CONT_DPAD_RIGHT)
		{
			keysym.sym=_dc_sdl_key[joystick->index][12];
			SDL_PrivateKeyboard((buttons & CONT_DPAD_RIGHT)?SDL_PRESSED:SDL_RELEASED,&keysym);
        }
    }

	//Emulating mouse
	//"joyx", "joyy", "joy2x", and "joy2y" are all zero based
	if ((__sdl_dc_emulate_mouse) && (!joystick->index))
	{
		count_cond=!(count&0x1);
		if (cond->joyx!=0 || cond->joyy!=0 || count_cond)
		{
			{
				register unsigned s=__sdl_dc_mouse_shift+1;
				mx=(cond->joyx)>>s;
				my=(cond->joyy)>>s;
			}
			if (count_cond)
				SDL_PrivateMouseMotion(changed>>1,1,(Sint16)(mx),(Sint16)(my));
			count++;
		}
	}

	//Check changes on cont_state_t: triggers and Axis
	prev_ltrig = joystick->hwdata->ltrig;
	prev_rtrig = joystick->hwdata->rtrig;
	prev_joyx = joystick->hwdata->joyx;
	prev_joyy = joystick->hwdata->joyy;
	prev_joy2x = joystick->hwdata->joy2x;
	prev_joy2y = joystick->hwdata->joy2y;

	//Check Joystick Axis P1
	//"joyx", "joyy", "joy2x", and "joy2y" are all zero based
	if (cond->joyx!=prev_joyx)
		SDL_PrivateJoystickAxis(joystick, 0, cond->joyx);
	if (cond->joyy!=prev_joyy)
		SDL_PrivateJoystickAxis(joystick, 1, cond->joyy);
	
	//Check L and R triggers
	//In this case, do not flip the PRESSED/RELEASED!
	if (cond->rtrig!=prev_rtrig)
	{
        // Toggle virtual keyboard when R trigger is pressed and emulation is enabled
        if (__sdl_dc_emulate_vhkb_keyboard == 1 && cond->rtrig && !prev_rtrig) {
            toggleVirtualKeyboard();
        }

		if (__sdl_dc_emulate_keyboard)
			if (((prev_rtrig) && (!cond->rtrig)) ||
		    	    ((!prev_rtrig) && (cond->rtrig)))
			{
				keysym.sym=_dc_sdl_key[joystick->index][7];
				SDL_PrivateKeyboard((cond->rtrig)?SDL_PRESSED:SDL_RELEASED,&keysym);
			}
		SDL_PrivateJoystickAxis(joystick, 2, cond->rtrig);
	}
    
	if (cond->ltrig!=prev_ltrig)
	{
		if (__sdl_dc_emulate_keyboard)
			if (((prev_ltrig) && (!cond->ltrig)) ||
		    	    ((!prev_ltrig) && (cond->ltrig)))
			{
				keysym.sym=_dc_sdl_key[joystick->index][8];
				SDL_PrivateKeyboard((cond->ltrig)?SDL_PRESSED:SDL_RELEASED,&keysym);
			}
		SDL_PrivateJoystickAxis(joystick, 3, cond->ltrig);
    }
	//Check Joystick Axis P2
	if (cond->joy2x!=prev_joy2x)
		SDL_PrivateJoystickAxis(joystick, 4, cond->joy2x);
	if (cond->joy2y!=prev_joy2y)
		SDL_PrivateJoystickAxis(joystick, 5, cond->joy2y);

	//Update previous state
    joystick->hwdata->prev_buttons = buttons;
	
	joystick->hwdata->ltrig = cond->ltrig;
	joystick->hwdata->rtrig = cond->rtrig;
	joystick->hwdata->joyx = cond->joyx;
	joystick->hwdata->joyy = cond->joyy;
	joystick->hwdata->joy2x = cond->joy2x;
	joystick->hwdata->joy2y = cond->joy2y;

}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	if (joystick->hwdata != NULL) {
		/* free system specific hardware data */
		free(joystick->hwdata);
	}
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	return;
}

static __inline__ Uint32 myGetTicks(void)
{
	return ((timer_us_gettime64()>>10));
}

void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	static Uint32 last_time[MAX_JOYSTICKS]={0,0,0,0,0,0,0,0};
	Uint32 now=myGetTicks();
	if (now-last_time[joystick->index]>MIN_FRAME_UPDATE)
	{
		last_time[joystick->index]=now;
		joyUpdate(joystick);
	}
}
#endif /* SDL_JOYSTICK_DC */
