// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef struct Cell Cell;

typedef struct {
   u32 y;
   u32 x;
   u32 height;
   u32 width;
} RenderRegion;

typedef struct {
   Cell** chars;
   u32 height;
   u32 width;
} CharMapSlice;

typedef void* (*LayerOnCreate)();
typedef void  (*LayerOnDestroy)(void* context);
typedef void  (*LayerOnInput)(Vector* InputEvent_queue, RenderRegion* region, void* context);
typedef void  (*LayerOnRender)(CharMapSlice charmap, void* context);

typedef struct Layer Layer;

/// Returns the [id] of the layer
usize Layer_push(RenderRegion  region, u32 z_order,
                 LayerOnCreate  on_create,
                 LayerOnDestroy on_destroy,
                 LayerOnInput   on_input,
                 LayerOnRender  on_render);

void Layer_pop();
void Layer_delete(usize layer_id);

typedef struct {
   u32 bg;
   u32 fg;
   CellStyle style;
} CellStyleOptions;

void CharMapSlice_putc(CharMapSlice* self, i32 y, i32 x, u32 c, CellStyleOptions styling);
void CharMapSlice_puts(CharMapSlice* self, i32 y, i32 x, nullable const u32* str, CellStyleOptions styling);
void CharMapSlice_fill(CharMapSlice* self, char c, CellStyleOptions styling);
void CharMapSlice_border(CharMapSlice* self, CellStyleOptions styling);
void CharMapSlice_box(CharMapSlice* self, i32 y, i32 x, u32 height, u32 width, CellStyleOptions styling);

#define border(...) \
   CharMapSlice_border(&charmap, (CellStyleOptions) { \
      .bg = 0x00000000, \
      .fg = 0xffffffff, \
      .style = CS_Normal \
      __VA_OPT__(,) __VA_ARGS__})

#define box(y, x, height, width, ...) \
   CharMapSlice_box(&charmap, y, x, height, width, (CellStyleOptions) { \
      .bg = 0x00000000, \
      .fg = 0xffffffff, \
      .style = CS_Normal \
      __VA_OPT__(,) __VA_ARGS__})

#define mvprintc(y, x, c, ...) \
   CharMapSlice_putc(&charmap, y, x, c, (CellStyleOptions) { \
      .bg = 0x00000000, \
      .fg = 0xffffffff, \
      .style = CS_Normal \
      __VA_OPT__(,) __VA_ARGS__})

#define mvprint(y, x, str, ...) \
   CharMapSlice_puts(&charmap, y, x, str, (CellStyleOptions) { \
      .bg = 0x00000000, \
      .fg = 0xffffffff, \
      .style = CS_Normal \
      __VA_OPT__(,) __VA_ARGS__})

#define fill(str, ...) \
   CharMapSlice_fill(&charmap, str, (CellStyleOptions) { \
      .bg = 0x00000000, \
      .fg = 0xffffffff, \
      .style = CS_Normal \
      __VA_OPT__(,) __VA_ARGS__})

u32 get_screen_height();
u32 get_screen_width();

float get_delta_time();

