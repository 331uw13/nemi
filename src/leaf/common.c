#include "common.h"

float lerp(float x, float y, float t) {
    return x + t * (y - x);
}

RGBColor leaf_color_lerp(RGBColor x, RGBColor y, float t) {

    float ir = lerp((float)x.r, (float)y.r, t);
    float ig = lerp((float)x.g, (float)y.g, t);
    float ib = lerp((float)x.b, (float)y.b, t);

    // Clamp values.
    ir = (ir < 0.0f) ? 0.0f : (ir > (float)UINT8_MAX) ? (float)UINT8_MAX : ir;
    ig = (ig < 0.0f) ? 0.0f : (ig > (float)UINT8_MAX) ? (float)UINT8_MAX : ig;
    ib = (ib < 0.0f) ? 0.0f : (ib > (float)UINT8_MAX) ? (float)UINT8_MAX : ib;

    return (RGBColor) { (uint8_t)ir, (uint8_t)ig, (uint8_t)ib };
}

RGBColor hexrgb_to_color_type(int hexrgb) {
    return (RGBColor) {
        (hexrgb & 0xFF0000) >> 16,
        (hexrgb & 0x00FF00) >> 8,
        (hexrgb & 0x0000FF)
    };
}
