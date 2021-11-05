/*

MIT License

Copyright (c) 2020 Mika Tuupola

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

*/

#ifndef _HAGL_HAL_H
#define _HAGL_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint16_t color_t;

#include "nuclei_sdk_soc.h"

#ifdef HAGL_HAL_USE_DOUBLE_BUFFER
#include "hagl_hal_double.h"
#else
#include "hagl_hal_single.h"
#endif

/* Default config is ok for Longan Nano. When compiling you */
/* can override these by including an user config header */
/* file first. */
#ifndef MIPI_DISPLAY_ADDRESS_MODE
//#define MIPI_DISPLAY_ADDRESS_MODE   (MIPI_DCS_ADDRESS_MODE_BGR)
#define MIPI_DISPLAY_ADDRESS_MODE   (MIPI_DCS_ADDRESS_MODE_RGB)
#endif
#ifndef MIPI_DISPLAY_OFFSET_X
//#define MIPI_DISPLAY_OFFSET_X       (26)
#define MIPI_DISPLAY_OFFSET_X       (52)
#endif
#ifndef MIPI_DISPLAY_OFFSET_Y
//#define MIPI_DISPLAY_OFFSET_Y       (1)
#define MIPI_DISPLAY_OFFSET_Y       (40)
#endif
#ifndef MIPI_DISPLAY_PIN_CS
#define MIPI_DISPLAY_PIN_CS         (GPIO_PIN_2)
//#define MIPI_DISPLAY_PIN_CS         (GPIO_PIN_4)
#endif
#ifndef MIPI_DISPLAY_PORT_CS
//#define MIPI_DISPLAY_PORT_CS        (GPIOA)
#define MIPI_DISPLAY_PORT_CS        (GPIOB)
#endif
#ifndef MIPI_DISPLAY_PIN_DC
#define MIPI_DISPLAY_PIN_DC         (GPIO_PIN_0)
#endif
#ifndef MIPI_DISPLAY_PORT_DC
#define MIPI_DISPLAY_PORT_DC        (GPIOB)
#endif
#ifndef MIPI_DISPLAY_PIN_RST
#define MIPI_DISPLAY_PIN_RST        (GPIO_PIN_1)
#endif
#ifndef MIPI_DISPLAY_PORT_RST
#define MIPI_DISPLAY_PORT_RST       (GPIOB)
#endif
#ifndef MIPI_DISPLAY_PIN_BL
//#define MIPI_DISPLAY_PIN_BL         (GPIO_PIN_5)
#define MIPI_DISPLAY_PIN_BL         (GPIO_PIN_10) // TTGO
#endif
#ifndef MIPI_DISPLAY_PORT_BL
//#define MIPI_DISPLAY_PORT_BL        (GPIOA)
#define MIPI_DISPLAY_PORT_BL        (GPIOB) // TTGO
#endif
#ifndef MIPI_DISPLAY_PIN_CLK
//#define MIPI_DISPLAY_PIN_CLK        (GPIO_PIN_6)
#define MIPI_DISPLAY_PIN_CLK        (GPIO_PIN_5) // TTGO
#endif
#ifndef MIPI_DISPLAY_PORT_CLK
#define MIPI_DISPLAY_PORT_CLK       (GPIOA)
#endif
#ifndef MIPI_DISPLAY_PIN_MOSI
#define MIPI_DISPLAY_PIN_MOSI       (GPIO_PIN_7)
#endif
#ifndef MIPI_DISPLAY_PORT_MOSI
#define MIPI_DISPLAY_PORT_MOSI      (GPIOA)
#endif
#ifndef MIPI_DISPLAY_PIXEL_FORMAT
#define MIPI_DISPLAY_PIXEL_FORMAT   (MIPI_DCS_PIXEL_FORMAT_16BIT)
#endif
#ifndef MIPI_DISPLAY_INVERT
#define MIPI_DISPLAY_INVERT
#endif
#ifndef MIPI_DISPLAY_WIDTH
//#define MIPI_DISPLAY_WIDTH          (80)
#define MIPI_DISPLAY_WIDTH          (135)
#endif
#ifndef MIPI_DISPLAY_HEIGHT
//#define MIPI_DISPLAY_HEIGHT         (160)
#define MIPI_DISPLAY_HEIGHT         (240)
#endif
#ifndef MIPI_DISPLAY_DEPTH
#define MIPI_DISPLAY_DEPTH          (16)
#endif

#define DISPLAY_WIDTH               (MIPI_DISPLAY_WIDTH)
#define DISPLAY_HEIGHT              (MIPI_DISPLAY_HEIGHT)
#define DISPLAY_DEPTH               (MIPI_DISPLAY_DEPTH)

#ifdef __cplusplus
}
#endif
#endif /* _HAGL_HAL_H */