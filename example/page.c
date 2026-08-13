// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/containers.h>
#include <mcu/memory.h>
#include <mcu/io.h>
#include <mcu/unicode.h>

#include "vystriyae/core.h"
#include "vystriyae/wm.h"
#include "vystriyae/unicode.h"
#include "vystriyae/notifications.h"
#include "vystriyae/colors.h"

#include "page.h"

typedef struct {
   struct {
      u32 x;
      u32 y;
      MouseScrollDirection scroll_direction;
   } mouse;

   struct {
      bool holding;
      u32 x;
      u32 y;
   } ball;
} Page;

void* Page_on_create() {
   Page* self = mcu_malloc(sizeof(Page));
   *self = (Page) {0};
   return self;
}

void Page_on_destroy(void* context) {
   mcu_free(context);
}

void Page_on_input(Vector* InputEvent_queue, RenderRegion* region, void* context) {
   Page* self = context;

   foreach ((*InputEvent_queue), i) {
      InputEvent event = *(InputEvent*) Vector_get(InputEvent_queue, i);

      switch (event.type) {
         case IET_Key: {
            switch (event.key.code) {
               case KC_q: {
                  Vector_push_create(InputEvent_queue, ((InputEvent) {
                     .type = IET_Signal,
                     .signal = SE_Shutdown,
                  }));
               } break;

               case KC_n: {
                  Notification_new(NT_Unknown, 5.0, U"Hello", U"You pressed the 'n' key");
               } break;

               default: break;
            }
         } break;

         case IET_Mouse: {
            switch (event.mouse.type) {
               case MET_Position: {
                  self->mouse.x = event.mouse.x;
                  self->mouse.y = event.mouse.y;

                  if (self->ball.holding) {
                     self->ball.x = event.mouse.x;
                     self->ball.y = event.mouse.y;
                  }
               } break;

               case MET_Key: {
                  if (event.mouse.button.key != MB_Left) break;
                  self->ball.holding = !event.mouse.button.released;

                  if (self->ball.holding) {
                     self->ball.x = event.mouse.x;
                     self->ball.y = event.mouse.y;
                  }
               } break;

               case MET_Scroll: {
                  self->mouse.scroll_direction = event.mouse.scroll.direction;
               } break;

               default: break;
            }
         } break;

         case IET_Signal: {
            switch (event.signal) {
               case SE_Resize: {
                  region->width = get_screen_width();
                  region->height = get_screen_height();
               } break;
               
               default: break;
            }
         } break;

         default: break;
      }
   }
}

void Page_on_render(CharMapSlice charmap, void* context) {
   Page* self = context;

   u32 row_buffer[16] = {0};
   u32 col_buffer[16] = {0};
   i32_to_ustr32((i32) charmap.height, row_buffer);
   i32_to_ustr32((i32) charmap.width, col_buffer);

   u32 mouse_x[16] = {0};
   u32 mouse_y[16] = {0};
   u32_to_ustr32(self->mouse.x, mouse_x);
   u32_to_ustr32(self->mouse.y, mouse_y);

   fill(' ', .bg = 0x000000ff);

   mvprint(1, 1, U"rows:");
   mvprint(1, 7, row_buffer);
   mvprint(2, 1, U"cols:");
   mvprint(2, 7, col_buffer);

   mvprint(4, 1, U"mouse");
   mvprint(5, 1, U"x:");
   mvprint(5, 4, mouse_x);
   mvprint(6, 1, U"y:");
   mvprint(6, 4, mouse_y);
   mvprint(7, 1, U"scroll:");

   switch (self->mouse.scroll_direction) {
      case MSD_Up:   mvprint(7, 9, U"up");   break;
      case MSD_Down: mvprint(7, 9, U"down"); break;
      default: panic("unreachable");
   }

   mvprintc(self->ball.y, self->ball.x, U'*');

   u32* unicode_msg = U"Unicode and colors are supported! Привет!";
   u32 unicode_msg_len = ustr32_len(unicode_msg);
   // The same color for all characters.
   mvprint(1, charmap.width - unicode_msg_len - 1, unicode_msg, .fg = animated_rainbow_gradiant(1.0f, 0));
   
   // Or a different color per character.
   // for (i32 i = 0; unicode_msg[i] != '\0'; ++i) {
   //   mvprintc(1, charmap.width - unicode_msg_len + i - 1, unicode_msg[i], .fg = animated_rainbow_gradiant(1.0f, i));
   // }

   border();
}

void Page_open() {
   Notification_new(NT_Unknown, 5.0, U"Welcome", U"Welcome to the page.\nHow are you doing?");

   Layer_push(
      (RenderRegion) {
         .y = 0,
         .x = 0,
         .width = get_screen_width(),
         .height = get_screen_height()
      },
      1,
      &Page_on_create,
      &Page_on_destroy,
      &Page_on_input,
      &Page_on_render);
}


