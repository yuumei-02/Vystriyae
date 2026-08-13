// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/containers.h>
#include <mcu/unicode.h>

#include <math.h>

#include "colors.h"
#include "private.h"

float __animation_offset = 0.0f;

void rgb_to_hsv(rgb rgb, float* h, float* s, float* v) {
   mcu_assert(h != nullptr, "h can't be null");
   mcu_assert(s != nullptr, "s can't be null");
   mcu_assert(v != nullptr, "v can't be null");

   float r = ((rgb >> 24) & 0xff) / 255.0f;
   float g = ((rgb >> 16) & 0xff) / 255.0f;
   float b = ((rgb >> 8 ) & 0xff) / 255.0f;

   float max = fmaxf(fmaxf(r, g), b);
   float min = fminf(fminf(r, g), b);
   float delta = max - min;

   *v = max;
   *s = (max == 0.0f) ? 0.0f : (delta / max);

   if (delta == 0.0f) *h = 0.0f;
   else if (max == r) *h = fmodf(((g - b) / delta), 6.0f);
   else if (max == g) *h = ((b - r) / delta) + 2.0f;
   else               *h = ((r - g) / delta) + 4.0f;

   *h *= 60.0f;
   if (*h < 0.0f) *h += 360.0f;
}

rgb hsv_to_rgb(float h, float s, float v) {
   float c = v * s;
   float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
   float m = v - c;

   float rf = 0;
   float gf = 0;
   float bf = 0;

   if (h < 60)       { rf = c; gf = x; bf = 0; }
   else if (h < 120) { rf = x; gf = c; bf = 0; }
   else if (h < 180) { rf = 0; gf = c; bf = x; }
   else if (h < 240) { rf = 0; gf = x; bf = c; }
   else if (h < 300) { rf = x; gf = 0; bf = c; }
   else              { rf = c; gf = 0; bf = x; }

   u8 r = (u8)((rf + m) * 255);
   u8 g = (u8)((gf + m) * 255);
   u8 b = (u8)((bf + m) * 255);

   return (r << 24) | (g << 16) | (b << 8) | (u8) 0;
}

float triangle_wave(float v, float min, float max) {
   float range = max - min;
   float period = 2.0f * range;

   float t = fmod(v - min, period);
   if (t < 0) t += period;
   if (t > range) t = period - t;
   
   return min + t;
}

rgb animated_rainbow_gradiant(u32 offset, float length) {
   float hue = ((__animation_offset) + offset) + (offset * (20.0f * length));
   hue = fmodf(hue, 360.0f);
   return hsv_to_rgb(hue, 1.0f, 1.0f);
}

rgb animated_fade_gradiant(rgb base_color, u32 offset) {
   float h, s, v;
   rgb_to_hsv(base_color, &h, &s, &v);

   v += (__animation_offset - offset * 5.0f) * 0.005f;
   v = fmodf(v, 1.0f);

   return hsv_to_rgb(h, s, v);
}

rgb animated_blinking_gradiant(rgb base_color, float speed) {
   float h, s, v;
   rgb_to_hsv(base_color, &h, &s, &v);

   v = __animation_offset * (0.01f * speed);
   /* v = fmodf(v, 1.0f); */
   v = triangle_wave(v, 0.0f, 1.0f);

   return hsv_to_rgb(h, s, v);
}

