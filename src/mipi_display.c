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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdatomic.h>

#include "nuclei_sdk_soc.h"

#include "mipi_dcs.h"
#include "mipi_display.h"

static const uint8_t DELAY_BIT = 1 << 7;
// static volatile atomic_flag lock = ATOMIC_FLAG_INIT;

static const mipi_init_command_t init_commands[] = {
    {MIPI_DCS_SOFT_RESET, {0}, 0 | DELAY_BIT},
    {MIPI_DCS_SET_ADDRESS_MODE, {MIPI_DISPLAY_ADDRESS_MODE}, 1},
    {MIPI_DCS_SET_PIXEL_FORMAT, {MIPI_DISPLAY_PIXEL_FORMAT}, 1},
#ifdef MIPI_DISPLAY_INVERT
    {MIPI_DCS_ENTER_INVERT_MODE, {0}, 0},
#else
    {MIPI_DCS_EXIT_INVERT_MODE, {0}, 0},
#endif
    {MIPI_DCS_EXIT_SLEEP_MODE, {0}, 0 | DELAY_BIT},
    {MIPI_DCS_SET_DISPLAY_ON, {0}, 0 | DELAY_BIT},
    /* End of commands . */
    {0, {0}, 0xff},
};

static void mipi_display_write_command(const uint8_t command)
{
    // ESP_LOGD(TAG, "Sending command 0x%02x", (uint8_t)command);

    gpio_bit_reset(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);
    gpio_bit_reset(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) {
        /* NOP */
    };
    spi_i2s_data_transmit(SPI0, command);

    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)) {
        /* NOP */
    };
    spi_i2s_data_receive(SPI0);

    gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);
}

static void mipi_display_write_data(const uint8_t *data, size_t length)
{
    size_t sent = 0;

    if (0 == length) {
        return;
    };

    // ESP_LOG_BUFFER_HEX_LEVEL(TAG, data, length, ESP_LOG_DEBUG);

    /* Set DC */
    gpio_bit_set(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);

    while (sent < length) {
        gpio_bit_reset(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) {
            /* NOP */
        };
        spi_i2s_data_transmit(SPI0, data[sent]);

        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)) {
            /* NOP */
        };
        spi_i2s_data_receive(SPI0);

        gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

        sent = sent + 1;
    }
}

static void mipi_display_read_data(uint8_t *data, size_t length)
{
    if (0 == length) {
        return;
    };
}

static void mipi_display_spi_master_init()
{
    spi_parameter_struct spi_init_struct;

    gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_8;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

	spi_crc_polynomial_set(SPI0, 7);
	spi_enable(SPI0);
}

void mipi_display_init()
{
    uint8_t cmd = 0;

	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
 	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_SPI0);

    /* Init pins. */
    gpio_init(MIPI_DISPLAY_PORT_BL, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_BL);
    gpio_init(MIPI_DISPLAY_PORT_CLK, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_CLK);
    gpio_init(MIPI_DISPLAY_PORT_MOSI, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_MOSI);
	gpio_init(MIPI_DISPLAY_PORT_CS, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_CS);
	gpio_init(MIPI_DISPLAY_PORT_DC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_DC);

    /* Init spi driver. */
    mipi_display_spi_master_init();
    delay_1ms(100);

	gpio_bit_reset(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);

    /* Reset the display. */
    if (MIPI_DISPLAY_PIN_RST > 0) {
        gpio_init(MIPI_DISPLAY_PORT_RST, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_RST);
        gpio_bit_reset(MIPI_DISPLAY_PORT_RST, MIPI_DISPLAY_PIN_RST);
        delay_1ms(100);
        gpio_bit_set(MIPI_DISPLAY_PORT_RST, MIPI_DISPLAY_PIN_RST);
        delay_1ms(100);
    }

    /* Send all the commands. */
    while (init_commands[cmd].count != 0xff) {
        mipi_display_write_command(init_commands[cmd].command);
        mipi_display_write_data(init_commands[cmd].data, init_commands[cmd].count & 0x1F);
        if (init_commands[cmd].count & DELAY_BIT) {
            delay_1ms(200);
        }
        cmd++;
    }

    /* Enable backlight */
    if (MIPI_DISPLAY_PIN_BL > 0) {
        gpio_bit_set(MIPI_DISPLAY_PORT_BL, MIPI_DISPLAY_PIN_BL);
    }

    // ESP_LOGI(TAG, "Display initialized.");
}

void mipi_display_write(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t *buffer)
{
    if (0 == w || 0 == h) {
        return;
    }

    x1 = x1 + MIPI_DISPLAY_OFFSET_X;
    y1 = y1 + MIPI_DISPLAY_OFFSET_Y;

    int32_t x2 = x1 + w - 1;
    int32_t y2 = y1 + h - 1;
    uint32_t size = w * h;

    uint8_t command;
    uint8_t data[4];

    // while (atomic_flag_test_and_set(&lock)) {
    //     /* NOP */
    // }

    mipi_display_write_command(MIPI_DCS_SET_COLUMN_ADDRESS);
    data[0] = x1 >> 8;
    data[1] = x1 & 0xff;
    data[2] = x2 >> 8;
    data[3] = x2 & 0xff;
    mipi_display_write_data(data, 4);

    mipi_display_write_command(MIPI_DCS_SET_PAGE_ADDRESS);
    data[0] = y1 >> 8;
    data[1] = y1 & 0xff;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xff;
    mipi_display_write_data(data, 4);

    mipi_display_write_command(MIPI_DCS_WRITE_MEMORY_START);
    mipi_display_write_data(buffer, size * DISPLAY_DEPTH / 8);

    // atomic_flag_clear(&lock);
}

void mipi_display_ioctl(const uint8_t command, uint8_t *data, size_t size)
{
    // while (atomic_flag_test_and_set(&lock)) {
    //     /* NOP */
    // };

    switch (command) {
        case MIPI_DCS_GET_COMPRESSION_MODE:
        case MIPI_DCS_GET_DISPLAY_ID:
        case MIPI_DCS_GET_RED_CHANNEL:
        case MIPI_DCS_GET_GREEN_CHANNEL:
        case MIPI_DCS_GET_BLUE_CHANNEL:
        case MIPI_DCS_GET_DISPLAY_STATUS:
        case MIPI_DCS_GET_POWER_MODE:
        case MIPI_DCS_GET_ADDRESS_MODE:
        case MIPI_DCS_GET_PIXEL_FORMAT:
        case MIPI_DCS_GET_DISPLAY_MODE:
        case MIPI_DCS_GET_SIGNAL_MODE:
        case MIPI_DCS_GET_DIAGNOSTIC_RESULT:
        case MIPI_DCS_GET_SCANLINE:
        case MIPI_DCS_GET_DISPLAY_BRIGHTNESS:
        case MIPI_DCS_GET_CONTROL_DISPLAY:
        case MIPI_DCS_GET_POWER_SAVE:
        case MIPI_DCS_READ_DDB_START:
        case MIPI_DCS_READ_DDB_CONTINUE:
            mipi_display_write_command(command);
            mipi_display_read_data(data, size);
            break;
        default:
            mipi_display_write_command(command);
            mipi_display_write_data(data, size);
    }

    // atomic_flag_clear(&lock);
}

void mipi_display_close()
{
    // spi_device_release_bus(spi);
}