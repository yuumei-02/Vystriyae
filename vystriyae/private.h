// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include "core.h"
#include "wm.h"
#include "notifications.h"

/* ==== CORE ==== */
typedef struct {
   u32 height;
   u32 width;
} Screen;

typedef struct Cell {
   u32 bg;
   u32 fg;
   CellStyle style;
   u8 data[4];
   u8 bytes;
   u8 continuation;
} Cell;

typedef struct {
   Cell* chars;
   u32 height;
   u32 width;
} CharMap;

u8 utf8_get_byte_length_from_char(u8 c);
void CoreCharMap_put_cell(u32 y, u32 x, Cell cell);

/* ==== WM ==== */
typedef struct {
   Vector Layer_stack;
   Vector InputEvent_queue;
} WindowManager;

typedef struct Layer {
   usize id;

   RenderRegion region;
   CharMapSlice charmap;
   u32 z_order;

   void* context;
   LayerOnCreate  on_create;
   LayerOnDestroy on_destroy;
   LayerOnInput   on_input;
   LayerOnRender  on_render;
} Layer;

extern Screen screen;
extern WindowManager wm;
extern CharMap charmap;

void WindowManager_init();
void WindowManager_destroy();

void WindowManager_queue_input(InputEvent event);
/// Returns [true] on when a [layer] puts the [Shutdown signal] onto the [Layer_stack]
bool WindowManager_on_input();
void WindowManager_on_render(float dt);
void WindowManager_sort_layers();

/* ==== COLORS ==== */
extern float __animation_offset;

/* ==== Notifications ==== */
typedef struct {
   NotificationType type;
   float sec_duration;

   ustr32 title;
   ustr32 body;
} Notification;

extern Vector G_Notifications;

void Notifications_update(bool also_render, float dt, CharMap charmap);

