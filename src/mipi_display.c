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

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdatomic.h>

#include "nuclei_sdk_soc.h"

#include "mipi_dcs.h"
#include "mipi_display.h"

static const uint8_t DELAY_BIT = 1 << 7;

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
    /* Set DC low to denote incoming command. */
    gpio_bit_reset(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);

    /* Set CS low to reserve the SPI bus. */
    gpio_bit_reset(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) {};
    spi_i2s_data_transmit(SPI0, command);

    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)) {};
    spi_i2s_data_receive(SPI0);

    /* Set CS high to ignore any traffic on SPI bus. */
    gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);
}

static void mipi_display_write_data(const uint8_t *data, size_t length)
{
    size_t sent = 0;

    if (0 == length) {
        return;
    };

    /* Set DC high to denote incoming data. */
    gpio_bit_set(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);

    while (sent < length) {

        /* Set CS low to reserve the SPI bus. */
        gpio_bit_reset(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)) {};
        spi_i2s_data_transmit(SPI0, data[sent]);

        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)) {};
        spi_i2s_data_receive(SPI0);

        /* Set CS high to ignore any traffic on SPI bus. */
        gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

        sent = sent + 1;
    }
}

static void mipi_display_write_data_dma(const uint8_t *buffer, size_t length)
{
    size_t sent = 0;

    if (0 == length) {
        return;
    };

    /* Set DC high to denote incoming data. */
    gpio_bit_set(MIPI_DISPLAY_PORT_DC, MIPI_DISPLAY_PIN_DC);

    /* Set CS high to ignore any traffic on SPI bus. */
    gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

    dma_channel_disable(DMA0, DMA_CH2);
    dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)(buffer));

    /* Smells like off by one error somewhere? */
    dma_transfer_number_config(DMA0, DMA_CH2, length - 1);

    /* Set CS low to reserve the SPI bus. */
    gpio_bit_reset(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);
    dma_channel_enable(DMA0, DMA_CH2);
}

static void mipi_display_read_data(uint8_t *data, size_t length)
{
    if (0 == length) {
        return;
    };
}

static void mipi_display_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint8_t command;
    uint8_t data[4];

    x1 = x1 + MIPI_DISPLAY_OFFSET_X;
    y1 = y1 + MIPI_DISPLAY_OFFSET_Y;
    x2 = x2 + MIPI_DISPLAY_OFFSET_X;
    y2 = y2 + MIPI_DISPLAY_OFFSET_Y;

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
}

static void mipi_display_dma_init()
{
    dma_parameter_struct dma_config;

    rcu_periph_clock_enable(RCU_DMA0);
    dma_deinit(DMA0, DMA_CH2);

    dma_struct_para_init(&dma_config);
    dma_config.periph_addr = (uint32_t)&SPI_DATA(SPI0);
    dma_config.memory_addr = (uint32_t)NULL;
    dma_config.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_config.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_config.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_config.priority = DMA_PRIORITY_LOW;
    dma_config.number = 0;
    dma_config.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_config.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH2, &dma_config);

    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);

    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
}

static void mipi_display_spi_master_init()
{
    spi_parameter_struct spi_config;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI0);

    /* Enable backlight */
    if (MIPI_DISPLAY_PIN_BL > 0) {
        /* Longan Nano is GPIO_MODE_AF_PP, TTGO T-Display is GPIO_MODE_OUT_PP. */
        gpio_init(MIPI_DISPLAY_PORT_BL, MIPI_DISPLAY_GPIO_MODE_BL, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_BL);
        gpio_bit_set(MIPI_DISPLAY_PORT_BL, MIPI_DISPLAY_PIN_BL);
    }

    gpio_init(MIPI_DISPLAY_PORT_CLK, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_CLK);
    gpio_init(MIPI_DISPLAY_PORT_MOSI, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_MOSI);
    gpio_init(MIPI_DISPLAY_PORT_CS, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_CS);
    gpio_init(MIPI_DISPLAY_PORT_DC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MIPI_DISPLAY_PIN_DC);

    /* Set CS high to ignore any traffic on SPI bus. */
    gpio_bit_set(MIPI_DISPLAY_PORT_CS, MIPI_DISPLAY_PIN_CS);

    spi_struct_para_init(&spi_config);
    spi_config.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_config.device_mode = SPI_MASTER;
    spi_config.frame_size = SPI_FRAMESIZE_8BIT;
    spi_config.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_config.nss = SPI_NSS_SOFT;
    spi_config.prescale = SPI_PSC_8;
    spi_config.endian = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_config);

    spi_crc_polynomial_set(SPI0, 7);
    spi_enable(SPI0);
}

void mipi_display_init()
{
    uint8_t cmd = 0;

    /* Init the spi driver. */
    mipi_display_spi_master_init();
    delay_1ms(100);

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

    /* Set the default viewport to full screen. */
    mipi_display_set_address(0, 0, MIPI_DISPLAY_WIDTH - 1, MIPI_DISPLAY_HEIGHT - 1);

#ifdef HAGL_HAL_USE_DOUBLE_BUFFER
    mipi_display_dma_init();
#endif /* HAGL_HAL_USE_DOUBLE_BUFFER */
}

void mipi_display_write(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t *buffer)
{
    if (0 == w || 0 == h) {
        return;
    }

    int32_t x2 = x1 + w - 1;
    int32_t y2 = y1 + h - 1;
    uint32_t size = w * h;

#ifdef HAGL_HAL_USE_SINGLE_BUFFER
    mipi_display_set_address(x1, y1, x2, y2);
    mipi_display_write_data(buffer, size * DISPLAY_DEPTH / 8);
#endif /* HAGL_HAL_SINGLE_BUFFER */

#ifdef HAGL_HAL_USE_DOUBLE_BUFFER
    mipi_display_write_data_dma(buffer, size * DISPLAY_DEPTH / 8);
#endif /* HAGL_HAL_USE_DOUBLE_BUFFER */
}

/* TODO: This most likely does not work with dma atm. */
void mipi_display_ioctl(const uint8_t command, uint8_t *data, size_t size)
{
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
}

void mipi_display_close()
{
}