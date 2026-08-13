// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/containers.h>
#include <mcu/memory.h>
#include <mcu/unicode.h>

#include "private.h"
#include "unicode.h"
#include "ime.h"

#include <stdint.h>
#include <stdio.h>

static float delta_time = 0.0f;
static usize G_next_layer_id = 0;

float get_delta_time() {
   return delta_time;
}

void CharMapSlice_put_cell(CharMapSlice* self, i32 y, i32 x, Cell cell) {
   mcu_assert(self != nullptr, "self can't be null");
   if (y < 0 || x < 0) return;
   if ((u32) y >= self->height || (u32) x >= self->width) return;

   *(self->chars[(y * self->width) + x]) = cell;
}

void CharMapSlice_putc(CharMapSlice* self, i32 y, i32 x, u32 c, CellStyleOptions styling) {
   mcu_assert(self != nullptr, "self can't be null");
   if (y < 0 || x < 0) return;
   if ((u32) y >= self->height || (u32) x >= self->width) return;

   u8 data[4];
   u8 byte_len;
   code_point_to_utf8(c, data, &byte_len);

   *(self->chars[(y * self->width) + x]) = (Cell) {
      .bg    = styling.bg,
      .fg    = styling.fg,
      .style = styling.style,
      .data  = { data[0], data[1], data[2], data[3] },
      .bytes = byte_len
   };
}

void CharMapSlice_puts(CharMapSlice* self, i32 y, i32 x, nullable const u32* str, CellStyleOptions styling) {
   mcu_assert(self != nullptr, "self can't be null");
   if (str == nullptr) return;
   if (y < 0 || (u32) y >= self->height) return;

   loop {
      u8 data[4];
      u8 byte_len;
      code_point_to_utf8(*str, data, &byte_len);

      if (x >= 0) {
         *(self->chars[(y * self->width) + x]) = (Cell) {
            .bg    = styling.bg,
            .fg    = styling.fg,
            .style = styling.style,
            .data  = { data[0], data[1], data[2], data[3] },
            .bytes = byte_len
         };
      }

      x += 1;
      str += 1;
      if (*str == '\0') return;
   }
}

void CharMapSlice_fill(CharMapSlice* self, char c, CellStyleOptions styling) {
   mcu_assert(self != nullptr, "self can't be null");

   for (u32 y = 0; y < self->height; ++y) {
      u32 yoffset = (y * self->width);
      for (u32 x = 0; x < self->width; ++x) {
         *(self->chars[yoffset + x]) = (Cell) {
            .bg    = styling.bg,
            .fg    = styling.fg,
            .style = styling.style,
            .data  = {c},
            .bytes = 1
         };
      }
   }
}

void CharMapSlice_box(CharMapSlice* self, i32 y, i32 x, u32 height, u32 width, CellStyleOptions styling) {
   if (height < 2 || width < 2) return;

   #define box_cell(...) \
      (Cell) { \
         .bg = styling.bg, \
         .fg = styling.fg, \
         .style = styling.style, \
         .data = { __VA_ARGS__ }, \
         .bytes = 3 \
      }

   CharMapSlice_put_cell(self, y, x, box_cell(0xe2, 0x94, 0x8c)); // ┌
   for (u32 i = 1; i < width - 1; ++i)
      CharMapSlice_put_cell(self, y, x + i, box_cell(0xe2, 0x94, 0x80)); // ─
   CharMapSlice_put_cell(self, y, x + width - 1, box_cell(0xe2, 0x94, 0x90)); // ┐

   for (u32 i = 1; i < height - 1; ++i) {
      CharMapSlice_put_cell(self, y + i, x, box_cell(0xe2, 0x94, 0x82)); // │
      CharMapSlice_put_cell(self, y + i, x + width - 1, box_cell(0xe2, 0x94, 0x82)); // │
   }

   CharMapSlice_put_cell(self, y + height - 1, x, box_cell(0xe2, 0x94, 0x94)); // └
   for (u32 i = 1; i < width - 1; ++i)
      CharMapSlice_put_cell(self, y + height - 1, x + i, box_cell(0xe2, 0x94, 0x80)); // ─
   CharMapSlice_put_cell(self, y + height - 1, x + width - 1, box_cell(0xe2, 0x94, 0x98)); // ┘

   #undef box_cell
}

void CharMapSlice_border(CharMapSlice* self, CellStyleOptions styling) {
   if (self->width < 2 || self->height < 2) return;

   #define box_cell(...) \
      (Cell) { \
         .bg = styling.bg, \
         .fg = styling.fg, \
         .style = styling.style, \
         .data = { __VA_ARGS__ }, \
         .bytes = 3 \
      }

   // top
   *(self->chars[0]) = box_cell(0xe2, 0x94, 0x8c); // ┌
   for (u32 x = 1; x < self->width - 1; ++x) {
      *(self->chars[x]) = box_cell(0xe2, 0x94, 0x80); // ─
   }
   *(self->chars[self->width - 1]) = box_cell(0xe2, 0x94, 0x90); // ┐

   // bottom
   u32 yoffset = (self->height - 1) * self->width;
   *(self->chars[yoffset]) = box_cell(0xe2, 0x94, 0x94); // └
   for (u32 x = 1; x < self->width - 1; ++x) {
      *(self->chars[yoffset + x]) = box_cell(0xe2, 0x94, 0x80); // ─
   }
   *(self->chars[(self->height * self->width) - 1]) = box_cell(0xe2, 0x94, 0x98); // ┘

   // left
   for (u32 y = 1; y < self->height - 1; ++y) {
      *(self->chars[y * self->width]) = box_cell(0xe2, 0x94, 0x82); // │
   }

   // right
   u32 xoffset = (self->width - 1);
   for (u32 y = 1; y < self->height - 1; ++y) {
      *(self->chars[y * self->width + xoffset]) = box_cell(0xe2, 0x94, 0x82); // │
   }

   #undef box_cell
}

bool RenderRegion_clip(RenderRegion* self) {
   mcu_assert(self != nullptr, "self can't be null");

   if (self->width == 0 || self->height == 0) return false;
   if (self->x + self->width < self->x ||
       self->y + self->height < self->y
   ) return false; // Overflow

   // Clip left
   if (self->x >= screen.width) return false; // Entirely out of bounds
   u32 clipped_x_end = (self->x + self->width > screen.width) ? screen.width : self->x + self->width;

   // Clip top
   if (self->y >= screen.height) return false; // Entirely out of bounds
   u32 clipped_y_end = (self->y + self->height > screen.height) ? screen.height : self->y + self->height;

   u32 new_width = clipped_x_end - self->x;
   u32 new_height = clipped_y_end - self->y;

   self->width = new_width;
   self->height = new_height;

   return (new_width > 0 && new_height > 0);
}

void resize_layer(CharMapSlice* charslice, RenderRegion region) {
   mcu_assert(charslice != nullptr, "charslice can't be null");
   mcu_free(charslice->chars);

   RenderRegion clipped = region;
   bool in_bounds = RenderRegion_clip(&clipped);
   if (in_bounds) {
      charslice->height = clipped.height;
      charslice->width = clipped.width;
      charslice->chars  = mcu_malloc(sizeof(Cell*) * (clipped.height * clipped.width));
   } else {
      *charslice = (CharMapSlice) {0};
   }

   for (u32 y = 0; y < charslice->height; ++y) {
      for (u32 x = 0; x < charslice->width; ++x) {
         charslice->chars[(y * charslice->width) + x] = &(charmap.chars[((clipped.y + y) * charmap.width) + (clipped.x + x)]);
      }
   }
}

void resize_layers() {
   foreach (wm.Layer_stack, i) {
      Layer* layer = Vector_get(&wm.Layer_stack, i);
      resize_layer(&layer->charmap, layer->region);
   }
}

usize Layer_push(
   RenderRegion   region, u32 z_order,
   LayerOnCreate  on_create,
   LayerOnDestroy on_destroy,
   LayerOnInput   on_input,
   LayerOnRender  on_render
) {
   CharMapSlice charslice = {0};
   charslice.chars = mcu_malloc(1);
   resize_layer(&charslice, region);

   Layer self = {
      .region = region,
      .charmap = charslice,
      .z_order = z_order,
      
      .context = on_create(),
      .on_create  = on_create,
      .on_destroy = on_destroy,
      .on_input   = on_input,
      .on_render  = on_render
   };

   Vector_push(&wm.Layer_stack, &self);
   return G_next_layer_id++;
}

void Layer_pop() {
   Layer* layer = Vector_pop(&wm.Layer_stack);
   if (layer == nullptr) return;
   layer->on_destroy(layer->context);
   mcu_free(layer->charmap.chars);
}

void Layer_delete(usize layer_id) {
   foreach (wm.Layer_stack, i) {
      Layer* layer = Vector_get(&wm.Layer_stack, i);
      if (layer->id == layer_id) {
         Vector_remove(&wm.Layer_stack, i);
         break;
      }
   }
}

void WindowManager_init() {
   wm = (WindowManager) {
      .Layer_stack = Vector_new(sizeof(Layer)),
      .InputEvent_queue = Vector_new(sizeof(InputEvent))
   };
}

void WindowManager_destroy() {
   for (isize i = (isize) wm.Layer_stack.length - 1; i >= 0; --i) {
      Layer_pop();
   }

   Vector_free(&wm.Layer_stack);
   Vector_free(&wm.InputEvent_queue);
}

void WindowManager_queue_input(InputEvent event) {
   Vector_push(&wm.InputEvent_queue, &event);

   switch (event.type) {
      case IET_Key:   return;
      case IET_Text:  return;
      case IET_Mouse: return;
      
      case IET_Signal: {
         switch (event.signal) {
            case SE_Resize: {
               resize_layers();
            } return;

            case SE_Shutdown: return;
         }

         panic("unreachable");
      }
   }

   panic("unreachable");
}

bool WindowManager_on_input() {
   WindowManager_sort_layers();
   ime_transform(&wm.InputEvent_queue);

   foreach (wm.Layer_stack, i) {
      Layer* layer = Vector_get(&wm.Layer_stack, i);
      layer->on_input(&wm.InputEvent_queue, &layer->region, layer->context);
   }

   foreach (wm.InputEvent_queue, i) {
      InputEvent* event = Vector_get(&wm.InputEvent_queue, i);
      if (event->type == IET_Signal && event->signal == SE_Shutdown) return true;
   }

   resize_layers();
   Vector_clear(&wm.InputEvent_queue);
   return false;
}

void WindowManager_on_render(float dt) {
   WindowManager_sort_layers();
   __animation_offset += 100.0f * dt;
   delta_time = dt;
   foreach (wm.Layer_stack, i) {
      Layer* layer = Vector_get(&wm.Layer_stack, i);
      layer->on_render(layer->charmap, layer->context);
   }
}

void WindowManager_sort_layers() {
   if (wm.Layer_stack.length < 2) return;

   bool sorted = false;
   while (!sorted) {
      u32 previous_z = ((Layer*) Vector_get(&wm.Layer_stack, 0))->z_order;

      u32 swaps = 0;
      foreach (wm.Layer_stack, i) {
         Layer* layer = Vector_get(&wm.Layer_stack, i);
         if (layer->z_order < previous_z) {
            Layer previous = ((Layer*) wm.Layer_stack.buffer)[i - 1];

            ((Layer*) wm.Layer_stack.buffer)[i - 1] = ((Layer*) wm.Layer_stack.buffer)[i];
            ((Layer*) wm.Layer_stack.buffer)[i] = previous;

            previous_z = previous.z_order;
            swaps += 1;
         } else {
            previous_z = layer->z_order;
         }
      }

      if (swaps == 0) sorted = true;
   }
}

u32 get_screen_height() {
   return screen.height;
}

u32 get_screen_width() {
   return screen.width;
}

