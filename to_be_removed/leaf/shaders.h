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
#ifndef LEAF_SHADER_H
#define LEAF_SHADER_H

#include <stdint.h>

uint32_t create_shader          (const char* source, GLenum shader_type);
uint32_t create_shader_program  (const char* vertex_src, const char* fragment_src);
void     delete_shader_program  (uint32_t shader);

void     shader_uniform1f     (uint32_t shader, const char* name, float v);


#endif
