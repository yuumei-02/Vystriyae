// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/containers.h>
#include <mcu/memory.h>
#include <mcu/unicode.h>

#include "private.h"
#include "notifications.h"
#include "colors.h"
#include "unicode.h"

Vector G_Notifications;

static inline void check_define_notifications() {
   static bool S_notifications_defined = false;
   if (S_notifications_defined) return;

   G_Notifications = Vector_new(sizeof(Notification));
   S_notifications_defined = true;
}

void Notification_new(NotificationType type, float seconds_duration, nullable const ustr32 title, nullable const ustr32 body) {
   check_define_notifications();

   ustr32 title_copy;
   if (title == nullptr) {
      const ustr32 titleless = U"Titleless";
      title_copy = mcu_malloc(sizeof(titleless) + 1);
      memcpy(title_copy, titleless, sizeof(titleless) + 1);
   } else {
      usize title_bytes = (utf32_strlen(title) * sizeof(u32)) + 1;
      title_copy = mcu_malloc(title_bytes);
      memcpy(title_copy, title, title_bytes);
   }

   ustr32 body_copy = nullptr;
   if (body != nullptr) {
      usize body_bytes = (utf32_strlen(body) * sizeof(u32)) + 1;
      body_copy = mcu_malloc(body_bytes);
      memcpy(body_copy, body, body_bytes);
   }

   Vector_push_create(&G_Notifications, ((Notification) {
      .type = type,
      .sec_duration = seconds_duration,
      .title = title_copy,
      .body = body_copy
   }));
}

[[gnu::always_inline]]
static inline void Notifications_charmap_putc(u32 y, u32 x, u32 c, rgb bg, rgb fg, CellStyle cell_style) {
   u8 data[4];
   u8 byte_len;
   code_point_to_utf8(c, data, &byte_len);

   CoreCharMap_put_cell(y, x, (Cell) {
      .bg = bg,
      .fg = fg,
      .style = cell_style,
      .data = { data[0], data[1], data[2], data[3] },
      .bytes = byte_len,
      .continuation = 0
   });
}

void Notifications_update(bool also_render, float dt, CharMap charmap) {
   for (isize i = G_Notifications.length - 1; i >= 0; --i) {
      Notification* notif = Vector_get(&G_Notifications, (usize) i);
      
      notif->sec_duration -= dt;
      if (notif->sec_duration <= 0.0f) {
         mcu_free(notif->title);
         mcu_free(notif->body);
         Vector_remove(&G_Notifications, i);
      }
   }

   if (!also_render) return;

   foreach (G_Notifications, j) {
      Notification* notif = Vector_get(&G_Notifications, j);

      const u32 button_height = 7;
      const u32 button_width = 32;
      const rgb button_bg = 0x000000ff;
      const rgb button_text_color = 0xffffffff;
      
      rgb button_fg;
      switch (notif->type) {
         case NT_Info:    button_fg = 0x009dffff; break;
         case NT_Warning: button_fg = 0xffed29ff; break;
         case NT_Error:   button_fg = 0xf20b0bff; break;
         default:         button_fg = 0xaa00ffff; break;
      }

      i64 yp = charmap.height - ((button_height - 1) * (j + 1)) - 1;
      i64 xp = charmap.width - button_width - 1;

      if (xp <= 0) continue;
      if (yp <= 0) break;

      u32 y = (u32) yp;
      u32 x = (u32) xp;

      for (usize z = 1; z < button_width - 1; ++z)
         Notifications_charmap_putc(y + 1, x + z, U' ', button_bg, button_text_color, CS_Underline);

      for (usize z = 0; notif->title[z] != '\0' && z < button_width - 2; ++z)
         Notifications_charmap_putc(y + 1, x + 1 + z, notif->title[z], button_bg, button_text_color, CS_Underline);

      if (notif->body != nullptr) {
         u32 z = 0;
         i32 line_breaks = 0;
         bool finished_body = false;

         for (usize n = 2; n < button_height - 1; ++n) {
            for (usize w = 1; w < button_width - 1; ++w) {
               if (finished_body || line_breaks > 0) {
                  Notifications_charmap_putc(y + n, x + w, U' ', button_bg, button_text_color, CS_Normal);
               } else {
               redraw:
                  switch (notif->body[z]) {
                     case '\0': {
                        finished_body = true;
                     } break;
                     
                     case '\n': {
                        if (w == 1) {
                           z++;
                           goto redraw;
                        }
                        line_breaks++;
                     } break;

                     default: {
                        if (line_breaks <= 0)
                           Notifications_charmap_putc(y + n, x + w, notif->body[z], button_bg, button_text_color, CS_Normal);
                     }
                  }
                  z++;
               }
            }

            if (line_breaks > 0)
               line_breaks--;
         }
      }

      Notifications_charmap_putc(y, x, U'┌', button_bg, button_fg, CS_Normal);
      for (u32 i = 1; i < button_width - 1; ++i)
         Notifications_charmap_putc(y, x + i, U'─', button_bg, button_fg, CS_Normal);
      Notifications_charmap_putc(y, x + button_width - 1, U'┐', button_bg, button_fg, CS_Normal);

      for (u32 i = 1; i < button_height - 2; ++i) {
         Notifications_charmap_putc(++y, x, U'│', button_bg, button_fg, CS_Normal);
         Notifications_charmap_putc(y, x + button_width - 1, U'│', button_bg, button_fg, CS_Normal);
      }

      Notifications_charmap_putc(++y, x, U'└', button_bg, button_fg, CS_Normal);
      for (u32 i = 1; i < button_width - 1; ++i)
         Notifications_charmap_putc(y, x + i, U'─', button_bg, button_fg, CS_Normal);
      Notifications_charmap_putc(y, x + button_width - 1, U'┘', button_bg, button_fg, CS_Normal);
   }
}

