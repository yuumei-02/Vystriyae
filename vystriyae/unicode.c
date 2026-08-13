// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>

usize utf32_strlen(nullable u32* str) {
   usize length = 0;
   while (*str++ != 0) length++;
   return length;
}

/// Turns the unicode [code_point] into a utf8 byte array [data]
/// and sets the [length] of the [data] array
void code_point_to_utf8(u32 code_point, u8* data, u8* length) {
   mcu_assert(data != nullptr, "data can't be null");
   mcu_assert(length != nullptr, "length can't be null");
   
   if (code_point > 0x10ffff || (code_point >= 0xd800 && code_point <= 0xdfff)) {
      data[0] = 0x3f; // '?' for invalid code points
      *length = 1;
      return;
   }

   if (code_point <= 0x7f) {
      data[0] = (u8) code_point;
      *length = 1;
   } else if (code_point <= 0x7ff) {
      data[0] = 0xc0 | (u8) (code_point >> 6);
      data[1] = 0x80 | (u8) (code_point & 0x3f);
      *length = 2;
   } else if (code_point <= 0xffff) {
      data[0] = 0xe0 | (u8) (code_point >> 12);
      data[1] = 0x80 | (u8) ((code_point >> 6) & 0x3f);
      data[2] = 0x80 | (u8) (code_point & 0x3f);
      *length = 3;
   } else {
      data[0] = 0xf0 | (u8) (code_point >> 18);
      data[1] = 0x80 | (u8) ((code_point >> 12) & 0x3f);
      data[2] = 0x80 | (u8) ((code_point >> 6) & 0x3f);
      data[3] = 0x80 | (u8) (code_point & 0x3f);
      *length = 4;
   }
}

// @todo: utf8 function for returning the cell width of multi-cell characters
u8 utf8_get_byte_length_from_char(u8 c) {
   if ((c & 0b10000000) == 0)          return 1;
   if ((c & 0b11100000) == 0b11000000) return 2;
   if ((c & 0b11110000) == 0b11100000) return 3;
   if ((c & 0b11111000) == 0b11110000) return 4;
   return 1;
}

void i64_to_ustr32(i64 self, u32 buffer[]) {
   mcu_assert(buffer != nullptr, "buffer can't be null");

   if (self == 0) {
      buffer[0] = '0';
      buffer[1] = '\0';
      return;
   }

   i32 offset = 0;
   if (self < 0) {
      buffer[offset++] = '-';
      self = -self;
   }

   while (self > 0) {
      buffer[offset++] = '0' + (self % 10);
      self /= 10;
   }

   i32 a = buffer[0] == '-' ? 1 : 0;
   i32 b = offset - 1;

   while (a < b) {
      u32 tmp = buffer[a];
      buffer[a] = buffer[b];
      buffer[b] = tmp;

      a++;
      b--;
   }

   buffer[offset] = '\0';
}

void i32_to_ustr32(i32 self, u32 buffer[]) {
   i64_to_ustr32((i64) self, buffer);
}

void u32_to_ustr32(u32 self, u32 buffer[]) {
   i64_to_ustr32((i64) self, buffer);
}

