# MIPI DCS HAL for HAGL Graphics Library

HAL for HAGL graphics library for display drivers supporting the [MIPI Display Command Set](https://www.mipi.org/specifications/display-command-set). This covers most displays currently used by hobbyists. Tested with ST7735S (Longan Nano) and ST7789 (TTGO T-Display GD32).

[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat-square)](LICENSE)

## Usage

To use with an Nuclei SDK project you include this HAL and the [HAGL graphics library](https://github.com/tuupola/hagl) itself.

```
$ cd lib
$ git submodule add https://github.com/tuupola/hagl_gd32v_mipi.git hagl_hal
$ git submodule add https://github.com/tuupola/hagl.git
```

Then in your Makefile include the new source and header files.

```
SRCDIRS = . lib/hagl/src lib/hagl_hal/src
INCDIRS = . lib/hagl/include lib/hagl_hal/include
```

By default the HAL uses single buffering. The buffer is the GRAM of the display driver chip. You can enable double buffering with flags.

```
COMMON_FLAGS += -DHAGL_HAL_USE_DOUBLE_BUFFER
```

The default config can be found in `hagl_hal.h`. Defaults are ok for Longan Nano in vertical mode. You can override settings by including an use config file.

```
COMMON_FLAGS += -include ./hagl_user_config.h
```


An example project Makefile which both enables double buffering and overrides default config would look like following:

```
TARGET = firmware
NUCLEI_SDK_ROOT = ../nuclei-sdk
SRCDIRS = . lib/hagl/src lib/hagl_hal/src
INCDIRS = . lib/hagl/include lib/hagl_hal/include
COMMON_FLAGS += -DHAGL_HAL_USE_DOUBLE_BUFFER
COMMON_FLAGS += -include ./hagl_user_config.h

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
```

## Current stats

```
/* Single buffered */
61297 PIXELS per second, FB 0 FPS
962 LINES per second, FB 0 FPS
756 CIRCLES per second, FB 0 FPS
279 FILLED CIRCLES per second, FB 0 FPS
492 ELLIPSES per second, FB 0 FPS
100 FILLED ELLIPSES per second, FB 0 FPS
316 TRIANGLES per second, FB 0 FPS
189 FILLED TRIANGLES per second, FB 0 FPS
2295 RECTANGLES per second, FB 0 FPS
230 FILLED RECTANGLES per second, FB 0 FPS
1360 ROUND RECTANGLES per second, FB 0 FPS
223 FILLED ROUND RECTANGLES per second, FB 0 FPS
193 POLYGONS per second, FB 0 FPS
114 FILLED POLYGONS per second, FB 0 FPS
6394 CHARACTERS per second, FB 0 FPS
480 STRINGS per second, FB 0 FPS
33 RGB BARS per second, FB 0 FPS

/* Double buffered */
397087 PIXELS per second, FB 0 FPS
29462 LINES per second, FB 0 FPS
24498 CIRCLES per second, FB 0 FPS
13028 FILLED CIRCLES per second, FB 0 FPS
13252 ELLIPSES per second, FB 0 FPS
5228 FILLED ELLIPSES per second, FB 0 FPS
10316 TRIANGLES per second, FB 0 FPS
548 FILLED TRIANGLES per second, FB 0 FPS
18202 RECTANGLES per second, FB 0 FPS
13733 FILLED RECTANGLES per second, FB 0 FPS
16931 ROUND RECTANGLES per second, FB 0 FPS
12097 FILLED ROUND RECTANGLES per second, FB 0 FPS
6236 POLYGONS per second, FB 0 FPS
327 FILLED POLYGONS per second, FB 0 FPS
33755 CHARACTERS per second, FB 0 FPS
2632 STRINGS per second, FB 0 FPS
2040 RGB BARS per second, FB 0 FPS

```

## License

The MIT License (MIT). Please see [License File](LICENSE) for more information.
