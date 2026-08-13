// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#define _POSIX_C_SOURCE 200809L
#include <mcu/core.h>
#include <mcu/io.h>
#include <mcu/memory.h>
#include <mcu/unicode.h>

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "private.h"
#include "unicode.h"
#include "notifications.h"

#define STDIN 0
#define STDOUT 1

VystriyaeVersion G_vystriyae_version = { 1, 0, 0 };

Screen screen = {0};
WindowManager wm = {0};
CharMap charmap = {0};

typedef struct {
   struct termios saved_state;
   struct termios state;
   cstr color_support;
   bool rgb_support;
   bool mouse_support;
} Terminal;

typedef struct {
   u32 target_fps;
   u32 frame_count;
   float current_fps;
} FrameInfo;

// @todo: query for mouse support
static Terminal terminal = { .mouse_support = true };
static FrameInfo frame_info = {
   .target_fps = 60
};

volatile sig_atomic_t winched = 1;

void handle_winch(i32 signal) {
   unused signal;
   winched = 1;
}

#define milli_to_fps(milliseconds) 1000.0f / milliseconds
#define fps_to_milli(fps) 1000.0f / fps

u32 get_frame_count() {
   return frame_info.frame_count;
}

u32 get_target_fps() {
   return frame_info.target_fps;
}

float get_fps() {
   return frame_info.current_fps;
}

void set_target_fps(u32 new_fps) {
   frame_info.target_fps = new_fps;
}

void CoreCharMap_put_cell(u32 y, u32 x, Cell cell) {
   if (y >= charmap.height || x >= charmap.width) return;
   charmap.chars[(y * charmap.width) + x] = cell;
}

static void CharMap_putc(u32 y, u32 x, char c) {
   if (y >= charmap.height || x >= charmap.width) return;

   charmap.chars[(y * charmap.width) + x] = (Cell) {
      .bg = 0x00000000,
      .fg = 0xffffffff,
      .data = {c},
      .bytes = utf8_get_byte_length_from_char(c)
   };
}

// @todo: Make unicode aware
static void CharMap_puts(u32 y, u32 x, nullable const cstr str) {
   if (str == nullptr) return;
   if (y >= charmap.height) return;

   for (; *str != '\0' && x < charmap.width; ++str) {
      charmap.chars[(y * charmap.width) + x++] = (Cell) {
         .bg = 0x00000000,
         .fg = 0xffffff00,
         .data = {*str},
         .bytes = utf8_get_byte_length_from_char(*str)
      };
   }
}

// @todo: Have it return a string instead so that you can bitwise or styles together to combine them
char CellStyle_to_escape_code(CellStyle style) {
   switch (style) {
      case CS_Normal:        return '0';
      case CS_Bold:          return '1';
      case CS_Italic:        return '3';
      case CS_Underline:     return '4';
      case CS_StrikeThrough: return '9';
      case CS_Inverse:       return '7';
   }
   
   return '0';
}

bool check_for_resize() {
   if (winched == 0) return false;
   winched = 0;

   struct winsize win;
   if (ioctl(STDIN, TIOCGWINSZ, &win)) {
      panic("[!] Failed to get terminal dimensions, reason: \"%s\"", strerror(errno));
   }

   screen.height = (u32) win.ws_row;
   screen.width  = (u32) win.ws_col;

   mcu_free(charmap.chars);
   charmap = (CharMap) {
      .chars = mcu_malloc(sizeof(Cell) * (screen.height * screen.width)),
      .height = screen.height,
      .width = screen.width
   };

   for (u32 y = 0; y < charmap.height; ++y) {
      for (u32 x = 0; x < charmap.width; ++x) {
         charmap.chars[(y * charmap.width) + x] = (Cell) { .bg = 0x00000000, .fg = 0xffffffff, .data = {'.'}, .bytes = 1 };
      }
   }

   return true;
}

void set_cursor(bool hide) {
   if (hide)
      printf("\x1b[?25l");
   else
      printf("\x1b[?25h");
}

void reset_colors() {
   printf("\x1b[0m");
}

bool stdin_has_input() {
   fd_set readfds;
   struct timeval timeout = {0};
   FD_ZERO(&readfds);
   FD_SET(STDIN, &readfds);

   return select(STDIN + 1, &readfds, null, null, &timeout) > 0;
}

void sleep_ms(float ms) {
   struct timespec ts;
   ts.tv_sec  = (long) ms / 1000.0f;
   ts.tv_nsec = (long) ((ms - (ts.tv_sec * 1000)) * 1'000'000);
   nanosleep(&ts, null);
}

typedef struct timeval TimeVal;

TimeVal get_time() {
   TimeVal time;
   gettimeofday(&time, null);
   return time;
}

double time_diff(TimeVal start, TimeVal end) {
   return (end.tv_sec  - start.tv_sec)  +
          (end.tv_usec - start.tv_usec) /
          1'000'000.0f;
}

bool has_true_color_support() {
   return terminal.color_support &&
      (strstr(terminal.color_support, "truecolor") ||
       strstr(terminal.color_support, "24bit"));
}

u32 cstr_to_u32(cstr self) {
   mcu_assert(self != nullptr, "self != nullptr");

   u32 val = 0;

   for (usize i = 0; self[i] != '\0'; ++i) {
      val *= 10;
      val += self[i] - '0';
   }

   return val;
}

void parse_escape_code() {
   if (fgetc(stdin) != '[') return;
   if (fgetc(stdin) != '<') return;

   typedef enum {
      PM_Button,
      PM_X,
      PM_Y
   } ParseMode;

   ParseMode mode = PM_Button;
   char buff[12] = {0};
   u32 buff_off = 0;

   u32 x = 0;
   u32 y = 0;
   u32 mouse_button = 0;

   char c;
   for (c = fgetc(stdin); c != 'm' && c != 'M'; c = fgetc(stdin)) {
      switch (mode) {
         case PM_Button: {
            if (c == ';') {
               buff[buff_off] = '\0';
               mouse_button = cstr_to_u32(buff);
               buff_off = 0;
               mode = PM_X;
            } else {
               buff[buff_off++] = c;
            }
         } continue;

         case PM_X: {
            if (c == ';') {
               buff[buff_off] = '\0';
               x = cstr_to_u32(buff);
               buff_off = 0;
               mode = PM_Y;
            } else {
               buff[buff_off++] = c;
            }
         } continue;

         case PM_Y: {
            buff[buff_off++] = c;
         } continue;
      }

      panic("unreachable");
   }

   bool released = c == 'm';

   buff[buff_off] = '\0';
   y = cstr_to_u32(buff);

   if (x > 0) x--;
   if (y > 0) y--;

   switch (mouse_button) {
      case 0: mouse_button = (u32) MB_Left; goto mb_press;
      case 2: {
         mouse_button = (u32) MB_Right;
      mb_press:
         WindowManager_queue_input((InputEvent) {
            .type = IET_Mouse,
            .mouse = {
               .type = MET_Key,
               .x = x,
               .y = y,
               .button = {
                  .key = (MouseButton) mouse_button,
                  .released = released
               }
            }
         });
      } break;

      case 64: mouse_button = (u32) MSD_Up; goto mb_scroll;
      case 65: {
         mouse_button = (u32) MSD_Down;
      mb_scroll:
         WindowManager_queue_input((InputEvent) {
            .type = IET_Mouse,
            .mouse = {
               .type = MET_Scroll,
               .x = x,
               .y = y,
               .scroll = {
                  .direction = (MouseScrollDirection) mouse_button
               }
            }
         });
      } break;
   
      default: {
         WindowManager_queue_input((InputEvent) {
            .type = IET_Mouse,
            .mouse = {
               .type = MET_Position,
               .x = x,
               .y = y
            }
         });
      }
   }
}

void System_init() {
   tcgetattr(STDIN, &terminal.state);
   terminal.saved_state = terminal.state;

   terminal.state.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
   terminal.state.c_iflag &= ~(IXON | ICRNL);
   terminal.state.c_oflag &= ~(OPOST);
   if (tcsetattr(STDIN, TCSANOW, &terminal.state)) {
      panic("Failed to setup terminal, reason: \"%s\"", strerror(errno));
   }

   terminal.color_support = getenv("COLORTERM");
   terminal.rgb_support = has_true_color_support();

   charmap.chars = mcu_malloc(1);

   struct sigaction sa;
   sa.sa_handler = handle_winch;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sigaction(SIGWINCH, &sa, null);
   check_for_resize();

   WindowManager_init();
   printf("\x1b[?1049h");

   if (terminal.mouse_support) {
      printf("\x1b[?1000h\x1b[?1003h\x1b[?1006h");
   }
}

void System_run() {
   set_cursor(true);

   float delta_time = 0.0f;
   TimeVal last_sample = get_time();
   double render_time = 0.0f;
   double input_time = 0.0f;
   
   loop {
      frame_info.frame_count++;

      /* ==== RENDER ==== */
      TimeVal render_start = get_time();

      printf("\x1b[1;1H");
      WindowManager_on_render(delta_time);
      Notifications_update(true, delta_time, charmap);

      // Blit the charmap to the screen
      u8 continuation = 0;
      char cell_style = 0;
      u32 background = 0;
      u32 foreground = 0;
      
      for (u32 y = 0; y < charmap.height; ++y) {
         for (u32 x = 0; x < charmap.width; ++x) {
            if (continuation > 0) {
               continuation--;
               continue;
            }
            
            Cell cell = charmap.chars[(y * charmap.width) + x];
            if (cell.continuation > 0) {
               continuation = cell.continuation;
               if (x + continuation >= charmap.width) {
                  printf(" ");
                  continue;
               }
            }

            if (terminal.rgb_support) {
               char new_cell_style = CellStyle_to_escape_code(cell.style);
               if (cell_style != new_cell_style) {
                  cell_style = new_cell_style;
                  printf("\x1b[0;%cm", cell_style);
               }

               if (background != cell.bg) {
                  background = cell.bg;

                  u8 br = (cell.bg >> 24) & 0xFF;
                  u8 bg = (cell.bg >> 16) & 0xFF;
                  u8 bb = (cell.bg >> 8 ) & 0xFF;

                  printf("\x1b[48;2;%u;%u;%um", br, bg, bb);
               }

               if (foreground != cell.fg) {
                  foreground = cell.fg;

                  u8 fr = (cell.fg >> 24) & 0xFF;
                  u8 fg = (cell.fg >> 16) & 0xFF;
                  u8 fb = (cell.fg >> 8 ) & 0xFF;

                  printf("\x1b[38;2;%u;%u;%um", fr, fg, fb);
               }
            }

            printf("%.*s", (i32) cell.bytes, (cstr) cell.data);
         }
      }

      fflush(stdout);
      render_time = time_diff(render_start, get_time());
      /* ==== END RENDER ==== */

      /* ==== INPUT ==== */
      TimeVal input_start = get_time();

      bool input_parsing = true;
      while (input_parsing) {
         i32 input;
         if (frame_info.target_fps != 0) {
            errno = 0;
            input = stdin_has_input() ? fgetc(stdin) : null;
         } else {
            input = fgetc(stdin);
         }

         switch (input) {
            case null: {
               input_parsing = false;
            } break;
         
            case EOF: {
               if (ferror(stdin) && errno != EINTR) {
                  panic("[!] Failed to read from stdin, reason: \"%s\"", strerror(errno));
               }
            } break;

            case '\t': {
               WindowManager_queue_input((InputEvent) {
                  .type = IET_Key,
                  .key = {
                     .code = KC_Tab,
                     .modifiers = KM_None
                  }
               });
            } break;

            case '\r': {
               WindowManager_queue_input((InputEvent) {
                  .type = IET_Key,
                  .key = {
                     .code = KC_Enter,
                     .modifiers = KM_None
                  }
               });
            } break;

            case '\x1b': {
               parse_escape_code();
            } break;

            default: {
               bool capatilized = input >= 'A' && input <= 'Z';
               if ((input >= 'a' && input <= 'z') || capatilized) {
                  WindowManager_queue_input((InputEvent) {
                     .type = IET_Key,
                     .key = {
                        .code = capatilized ? input ^ 0x20 : input,
                        .modifiers = capatilized ? KM_Shift : KM_None
                     }
                  });
               }

               WindowManager_queue_input((InputEvent) {
                  .type = IET_Text,
                  .text = {
                     .code_point = input
                  }
               });
            } break;
         }

         if (frame_info.target_fps == 0) {
            break;
         }
      }

      if (check_for_resize()) {
         WindowManager_queue_input((InputEvent) {
            .type = IET_Signal,
            .signal = SE_Resize,
         });
      }

      if (WindowManager_on_input()) return;

      TimeVal now = get_time();
      input_time = time_diff(input_start, now);
      /* ==== END INPUT ==== */

      if (frame_info.target_fps != 0) {
         double sleep_time = fps_to_milli(frame_info.target_fps) - (render_time * 1000.0f) - (input_time * 1000.0f);
         sleep_ms(sleep_time);
      }

      now = get_time();
      float elapsed = time_diff(last_sample, now);
      if (elapsed >= 1.0f) {
         frame_info.current_fps = frame_info.frame_count / elapsed;
         frame_info.frame_count = 0;
         last_sample = now;
      }

      delta_time = time_diff(render_start, now);
   }
}

void System_close_actual(bool from_panic) {
   if (!from_panic) WindowManager_destroy();
   set_cursor(false);
   reset_colors();
   tcsetattr(STDIN, TCSANOW, &terminal.saved_state);
   printf("\x1b[?1049l");
   if (terminal.mouse_support)
      printf("\x1b[?1001l");
}

void System_close() {
   System_close_actual(false);
}

// @todo: Push panic tasks onto a stack
[[noreturn]]
void panic_handler(cstr file, i32 line, const cstr func, cstr format, ...) {
   va_list args;
   va_start(args, format);

   fprintf(stderr, "%s:%d:0: [PANIC] in function \"%s\", ", file, line, func);
   vfprintf(stderr, format, args);
   fputs("\n", stderr);

   System_close_actual(true);
   va_end(args);
   abort();
}

[[noreturn]]
void assert_handler(cstr file, i32 line, const cstr func, cstr format, ...) {
   va_list args;
   va_start(args, format);

   fprintf(stderr, "%s:%d:0: [ASSERT] in function \"%s\", ", file, line, func);
   vfprintf(stderr, format, args);
   fputs("\n", stderr);
   va_end(args);
   
   System_close_actual(true);
   abort();
}

[[noreturn]]
void misc_handler(cstr format, ...) {
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   
   System_close_actual(true);
   abort();
}

