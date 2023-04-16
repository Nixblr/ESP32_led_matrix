#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_idf_version.h>
#include "max7219.h"
#include "stdint.h"
#include "string.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
#define HOST HSPI_HOST
#else
#define HOST SPI2_HOST
#endif

#define CONFIG_EXAMPLE_CASCADE_SIZE 4
#define CONFIG_EXAMPLE_SCROLL_DELAY 100
#define CONFIG_EXAMPLE_PIN_NUM_MOSI 13
#define CONFIG_EXAMPLE_PIN_NUM_CLK 14
#define CONFIG_EXAMPLE_PIN_CS 25

max7219_t dev;

const uint8_t array[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x1b,0xec,
						 0x24,0x12,0x40,0x01,0x48,0x09,0x5a,0x2d,
						 0x6a,0x2b,0x08,0x08,0x09,0xc8,0x04,0x90,
						 0x03,0xe0,0x00,0x00,0x00,0x00,0x00,0x00};

void draw_screen(const uint8_t *frame)
{
	int i;
	uint8_t tmp[8];
	for (i = 0; i < 8; i++)
	{
		tmp[i] = frame[i*2];
	}
	max7219_draw_image_8x8(&dev, 24, tmp);
	
	for (i = 0; i < 8; i++)
	{
		tmp[i] = frame[i*2+1];
	}
	max7219_draw_image_8x8(&dev, 16, tmp);

	for (i = 0; i < 8; i++)
	{
		tmp[i] = frame[i*2 + 16];
	}
	max7219_draw_image_8x8(&dev, 0, tmp);
	for (i = 0; i < 8; i++)
	{
		tmp[i] = frame[i*2+16+1];
	}
	max7219_draw_image_8x8(&dev, 8, tmp);
}

void task(void *pvParameter)
{
	// Configure SPI bus
	spi_bus_config_t cfg = {
		.mosi_io_num = CONFIG_EXAMPLE_PIN_NUM_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = CONFIG_EXAMPLE_PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0,
		.flags = 0};
	ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, 1));

	// Configure device
	dev.cascade_size = CONFIG_EXAMPLE_CASCADE_SIZE;
	dev.digits = 0;
	dev.mirrored = false;

	ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CONFIG_EXAMPLE_PIN_CS));
	ESP_ERROR_CHECK(max7219_init(&dev));

	uint8_t screen[64];
	int8_t offset = 0;

	while (1)
	{
		memset(screen, 0, sizeof(screen));
		memcpy((uint8_t*) screen + offset, array, 32);
		offset -= 2;
		draw_screen(screen);
		offset -= 2;
		if (offset < 0)
		{
			offset = 32;
			vTaskDelay(pdMS_TO_TICKS(500));
		}
		vTaskDelay(pdMS_TO_TICKS(CONFIG_EXAMPLE_SCROLL_DELAY));
	}
}

void app_main(void)
{
	xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}
