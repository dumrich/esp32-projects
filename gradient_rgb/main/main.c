#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LEDC_FREQUENCY          (1000) // Frequency in Hertz. Set frequency at 1 kHz

const gpio_num_t ledPins[] = {GPIO_NUM_15, GPIO_NUM_0, GPIO_NUM_4};
const ledc_channel_t chns[] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};

uint32_t wheel[256];

void gen_wheel(uint32_t* wheelx, size_t len) {
    // Red to Green
    uint32_t red = 255;
    uint32_t green = 0;
    uint32_t blue = 0;

    for(int i = 0; i < 85; i++) {
        wheelx[i] = (red << 16) | (green << 8) | blue;
        red -= 3;
        green += 3;
    }

    for(int i = 85; i < 170; i++) {
        wheelx[i] = (red << 16) | (green << 8) | blue;
        green -= 3;
        blue += 3;
    }

    for(int i = 170; i < 255; i++) {
        wheelx[i] = (red << 16) | (green << 8) | blue;
        blue -= 3;
        red += 3;
    }
}

// Each RGB ranges from 0-255
void setColor(uint8_t B, uint8_t G, uint8_t R) {
    // Red
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, chns[0], R));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, chns[0]));
    
    // Green
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, chns[1], G));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, chns[1]));

    // Blue
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, chns[2], B));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, chns[2]));
}

void rgb(void*) {
    // Generate color wheel
    size_t nums = sizeof(wheel)/sizeof(uint32_t);
    gen_wheel(wheel, nums);
    for(int i = 0; i < nums; i++) {
        setColor((wheel[i] >> 0xF) & 0xFF, (wheel[i] >> 0x8) & 0xFF, wheel[i] & 0xFF);
        vTaskDelay(40 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    for(int i = 0; i < sizeof(ledPins)/sizeof(gpio_num_t); i++) {
        gpio_set_direction(ledPins[i], GPIO_MODE_OUTPUT);
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_MODE,
            .timer_num        = LEDC_TIMER,
            .duty_resolution  = LEDC_DUTY_RES,
            .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_MODE,
            .channel        = chns[i],
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = ledPins[i],
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }

    xTaskCreate(rgb, "Random RGB lights", 1024, NULL, 1, NULL);
}