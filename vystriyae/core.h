// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef struct {
   u8 major;
   u8 minor;
   u8 path;
} VystriyaeVersion;

extern VystriyaeVersion G_vystriyae_version;

typedef enum : u8 {
   CS_Normal = 0,
   CS_Bold,
   CS_Italic,
   CS_Underline,
   CS_StrikeThrough,
   CS_Inverse,
} CellStyle;

typedef enum {
   IET_Key,
   IET_Text,
   IET_Signal,
   IET_Mouse
} InputEventType;

typedef enum : u8 {
   KM_None  = 0,
   KM_Shift = 1 << 0,
   KM_Ctrl  = 1 << 1,
   KM_Meta  = 1 << 2,
   KM_Hypr  = 1 << 3,
} KeyModifiers;

typedef enum {
   KC_None,
   KC_Enter,
   KC_Tab,

   KC_a = 'a',
   KC_b = 'b',
   KC_c = 'c',
   KC_d = 'd',
   KC_e = 'e',
   KC_f = 'f',
   KC_g = 'g',
   KC_h = 'h',
   KC_i = 'i',
   KC_j = 'j',
   KC_k = 'k',
   KC_l = 'l',
   KC_m = 'm',
   KC_n = 'n',
   KC_o = 'o',
   KC_p = 'p',
   KC_q = 'q',
   KC_r = 'r',
   KC_s = 's',
   KC_t = 't',
   KC_u = 'u',
   KC_v = 'v',
   KC_w = 'w',
   KC_x = 'x',
   KC_y = 'y',
   KC_z = 'z',
} KeyCode;

typedef struct {
   KeyCode code;
   u8 modifiers;

   bool pressed;
   bool repeated;
} KeyEvent;

typedef struct {
   u32 code_point;
} TextEvent;

typedef enum {
   SE_Resize,
   SE_Shutdown,
} SignalEvent;

typedef enum {
   MB_Left,
   MB_Right,
   MB_Middle,
} MouseButton;

typedef enum {
   MET_Key,
   MET_Position,
   MET_Scroll
} MouseEventType;

typedef enum {
   MSD_Up,
   MSD_Down
} MouseScrollDirection;

// @reference: https://www.xfree86.org/current/ctlseqs.html#Mouse%20Tracking
// @reference: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking
typedef struct {
   MouseEventType type;

   u32 x;
   u32 y;

   union {
      struct {
         MouseButton key;
         bool released;
      } button;

      struct {
         MouseScrollDirection direction;
      } scroll;
   };
} MouseEvent;

typedef struct {
   InputEventType type;

   union {
      KeyEvent key;
      TextEvent text;
      SignalEvent signal;
      MouseEvent mouse;
   };
} InputEvent;

void System_init();
void System_run();
void System_close();

/// [Input] becomes [blocking] when [fps] is [0].
void set_target_fps(u32 fps);

u32 get_frame_count();
u32 get_target_fps();
float get_fps();

