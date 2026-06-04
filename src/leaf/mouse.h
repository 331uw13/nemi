#ifndef LEAF_MOUSE_H
#define LEAF_MOUSE_H


// See 'leaf/keyboard.h' for reason why this exists.


#ifdef GRAPHICS_OPENGL

#include <GLFW/glfw3.h>

#define MOUSE_LEFT  GLFW_MOUSE_LEFT_BUTTON
#define MOUSE_RIGHT GLFW_MOUSE_RIGHT_BUTTON
#define MOUSE_MIDDLE GLFW_MOUSE_MIDDLE_BUTTON


#endif


#endif
