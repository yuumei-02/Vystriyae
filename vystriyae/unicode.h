// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

/// Turns the unicode [code_point] into a utf8 byte array [data]
/// and sets the [length] of the [data] array
void code_point_to_utf8(u32 code_point, u8* data, u8* length);
u8 utf8_get_byte_length_from_char(u8 c);
usize utf32_strlen(nullable u32* str);

// @todo: macro for min needed buffer size for each to_ustr32 function
void i64_to_ustr32(i64 self, u32 buffer[]);
void i32_to_ustr32(i32 self, u32 buffer[]);
void u32_to_ustr32(u32 self, u32 buffer[]);

