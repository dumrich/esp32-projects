#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define BUZZER_PIN 4
#define BUTTON_PIN 14

#define LEDC_CHANNEL 0
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (BUZZER_PIN) // Define the output GPIO
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // Set duty resolution to 8 bits
#define LEDC_FREQUENCY          (2000) // Frequency in Hertz. Set frequency at 1 kHz

void alert(void);
void buzz(void*) {
    while(1) {
        if(gpio_get_level(BUTTON_PIN) == 0) {
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, pow(2, 10) * 0.5));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            alert();
        } else {
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, pow(2, 10) * 0));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
void alert(void) {
    float sinVal;
    int toneVal;

    for(int x = 0; x < 360; x++) {
        // Sin in radians
        sinVal = sin(x * (M_PI/180));
        toneVal = 2000 + sinVal * 500;

        ledc_set_freq(LEDC_MODE, LEDC_CHANNEL_0, toneVal);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
} 

void app_main(void)
{
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
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = pow(2, 10) * 0.5, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    xTaskCreate(buzz, "Buzz on button click", 1024, NULL, 1, NULL);
}