/*

MIT License

Copyright (c) 2020-2023 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-cut-

This file is part of the GD32V MIPI DCS HAL for the HAGL graphics library:
https://github.com/tuupola/hagl_gd32v_mipi

SPDX-License-Identifier: MIT

-cut-

This is the backend when double buffering is enabled. The GRAM of the
display driver chip is the framebuffer. The memory allocated by the
backend is the back buffer. Total two buffers.

Note that all coordinates are already clipped in the main library itself.
Backend does not need to validate the coordinates, they can always be
assumed to be valid.

*/

#include "hagl_hal.h"

#ifdef HAGL_HAL_USE_DOUBLE_BUFFER

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mipi_display.h>
#include <mipi_dcs.h>

#include <hagl/bitmap.h>
#include <hagl/backend.h>
#include <hagl/color.h>

static hagl_bitmap_t bb;

static size_t
flush(void *self)
{
    /* Flush the whole back buffer. */
    mipi_display_write(0, 0, bb.width, bb.height, (uint8_t *) bb.buffer);
    /* TODO: should be returned above. */
    return DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_DEPTH / 8);
}

static void
put_pixel(void *self, int16_t x0, int16_t y0, hagl_color_t color)
{
    bb.put_pixel(&bb, x0, y0, color);
}

static hagl_color_t
get_pixel(void *self, int16_t x0, int16_t y0)
{
    return bb.get_pixel(&bb, x0, y0);
}

static void
blit(void *self, int16_t x0, int16_t y0, hagl_bitmap_t *src)
{
    bb.blit(&bb, x0, y0, src);
}

static void
scale_blit(void *self, uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, hagl_bitmap_t *src)
{
    bb.scale_blit(&bb, x0, y0, w, h, src);
}

static void
hline(void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color)
{
    bb.hline(&bb, x0, y0, width, color);
}

static void
vline(void *self, int16_t x0, int16_t y0, uint16_t height, hagl_color_t color)
{
    bb.vline(&bb, x0, y0, height, color);
}

void
hagl_hal_init(hagl_backend_t *backend)
{
    mipi_display_init();

    if (!backend->buffer) {
        backend->buffer = calloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_DEPTH / 8), sizeof(uint8_t));
        hagl_hal_debug("Allocated back buffer to address %p.\n", (void *) backend->buffer);
    } else {
        hagl_hal_debug("Using provided back buffer at address %p.\n", (void *) backend->buffer);
    }

    backend->width = MIPI_DISPLAY_WIDTH;
    backend->height = MIPI_DISPLAY_HEIGHT;
    backend->depth = MIPI_DISPLAY_DEPTH;
    backend->put_pixel = put_pixel;
    backend->get_pixel = get_pixel;
    backend->hline = hline;
    backend->vline = vline;
    backend->blit = blit;
    backend->scale_blit = scale_blit;
    backend->flush = flush;

    hagl_bitmap_init(&bb, backend->width, backend->height, backend->depth, backend->buffer);
}

#endif /* HAGL_HAL_USE_DOUBLE_BUFFER */
