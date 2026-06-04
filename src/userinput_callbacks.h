/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

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
#ifndef USERINPUT_CALLBACKS_H
#define USERINPUT_CALLBACKS_H

//#include "leaf/leaf.h"

#include <stdint.h>

void userinput_key_pressed     (void* user_pointer, int key, int mods);
void userinput_char_pressed    (void* user_pointer, uint32_t codepoint);
void userinput_window_resized  (void* user_pointer, int width, int height);
void userinput_mouse_moved     (void* user_pointer, float new_x, float new_y);
void userinput_mouse_pressed   (void* user_pointer, int button);
void userinput_mouse_scroll    (void* user_pointer, int direction);

#endif
