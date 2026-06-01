#ifndef GLFW_CALLBACKS_H
#define GLFW_CALLBACKS_H

//#include "leaf/leaf.h"

#include <stdint.h>

void userinput_key_pressed(void* user_pointer, int key, int mods);
void userinput_char_pressed(void* user_pointer, uint32_t codepoint);
void userinput_window_resized(void* user_pointer, int width, int height);

/*
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_window_resize_callback(GLFWwindow* window, int width, int height);
void glfw_char_callback(GLFWwindow* window, uint32_t codepoint);
void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
*/

#endif
