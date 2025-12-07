#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "shaders.h"



uint32_t create_shader(const char* source, GLenum shader_type) {
    uint32_t shader = 0;

    if(!source) {
        fprintf(stderr, "%s: source cant be NULL.\n", __func__);
        goto error;
    }

    shader = glCreateShader(shader_type);
    if(!shader) {
        fprintf(stderr, "%s: failed to create shader\n", __func__);
        goto error;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    char* info_log = NULL;
    int info_log_size = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_size);
    if(info_log_size > 1) {
        info_log = malloc(info_log_size);
        if(!info_log) {
            fprintf(stderr, "%s: failed to allocate memory for shader info log.\n", __func__);
            glDeleteShader(shader);
            shader = 0;
            goto error;
        }

        glGetShaderInfoLog(shader, info_log_size, NULL, info_log);
        fprintf(stderr, "%s: %s\n", __func__, info_log);

        free(info_log);
        info_log = NULL;

        glDeleteShader(shader);
        shader = 0;
        goto error;
    }

error:
    return shader;
}


uint32_t create_shader_program(
        const char* vertex_src,
        const char* fragment_src) 
{
    uint32_t prog = 0;
    uint32_t vert_shader = 0;
    uint32_t frag_shader = 0;

    vert_shader = create_shader(vertex_src, GL_VERTEX_SHADER);
    if(!vert_shader) {
        fprintf(stderr, "\033[41m\033[4m(ERROR) vertex shader failed to compile.\033[0m\n");
        goto error;
    }
    //printf("\033[32m+ vertex shader compiled.\033[0m\n");


    frag_shader = create_shader(fragment_src, GL_FRAGMENT_SHADER);
    if(!frag_shader) {
        fprintf(stderr, "\033[41m\033[4m(ERROR) fragment shader failed to compile.\033[0m\n");
        goto error;
    }
    //printf("\033[32m+ fragment shader compiled.\033[0m\n");


    prog = glCreateProgram();
    if(!prog) {
        fprintf(stderr, "failed to create shader program\n");
        goto error;
    }

    glAttachShader(prog, vert_shader);
    glAttachShader(prog, frag_shader);
    glLinkProgram(prog);

    char* info_log = NULL;
    int info_log_size = 0;

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &info_log_size);
    if(info_log_size > 1) {
        info_log = malloc(info_log_size);
        glGetProgramInfoLog(prog, info_log_size, NULL, info_log);

        printf("\033[91m%s\033[0m\n", info_log);

        free(info_log);
        info_log = NULL;
    }


error:
    
    if(vert_shader > 0) {
        glDeleteShader(vert_shader);
    }
    if(frag_shader > 0) {
        glDeleteShader(frag_shader);
    }
    
    return prog;
}

void delete_shader_program(unsigned int shader) {
    if(shader > 0) {
        glDeleteProgram(shader);
    }
}


