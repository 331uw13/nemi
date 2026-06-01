#ifndef LEAF_COMMON_UTILS_H
#define LEAF_COMMON_UTILS_H

#include "color_type.h"


RGBColor hexrgb_to_color_type (int hexrgb);
RGBColor leaf_color_lerp      (RGBColor x, RGBColor y, float t);
float    leaf_lerp            (float x, float y, float t);




#endif
