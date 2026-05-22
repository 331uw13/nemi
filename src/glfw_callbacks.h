#ifndef GLFW_CALLBACKS_H
#define GLFW_CALLBACKS_H

#include "leaf/leaf.h"

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_window_resize_callback(GLFWwindow* window, int width, int height);
void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);;
void glfw_char_callback(GLFWwindow* window, uint32_t codepoint);
void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

#endif
