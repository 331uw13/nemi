#ifndef LEAF_SHADER_H
#define LEAF_SHADER_H

#include <stdint.h>

uint32_t create_shader          (const char* source, GLenum shader_type);
uint32_t create_shader_program  (const char* vertex_src, const char* fragment_src);
void     delete_shader_program  (uint32_t shader);


#endif
