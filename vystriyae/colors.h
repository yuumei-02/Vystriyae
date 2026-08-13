// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef u32 rgb;

rgb hsv_to_rgb(float h, float s, float v);

rgb animated_rainbow_gradiant(u32 offset, float length);
rgb animated_fade_gradiant(rgb base_color, u32 offset);
rgb animated_blinking_gradiant(rgb base_color, float speed);

