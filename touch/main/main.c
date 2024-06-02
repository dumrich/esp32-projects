#include <stdio.h>
#include "driver/ledc.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#define LIGHT_MIN 2000
#define LIGHT_MAX 3000

#define constrain(amt,low,high) ((amt)<(low)? (low):((amt)>(high)? (high):(amt)))

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (12) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LEDC_DUTY               (0) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (1000) // Frequency in Hertz. Set frequency at 4 kHz

#define ANALOG_INPUT_PIN 04
#define ADC2_CHANNEL ADC2_CHANNEL_0

static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void* RWVoltage(void*) {
    uint8_t output_data = 0;
    int read;

    while(1) {
        adc2_get_raw(ADC2_CHANNEL, ADC_BITWIDTH_12, &read);
        output_data = map(constrain(read, LIGHT_MIN, LIGHT_MAX), LIGHT_MIN, LIGHT_MAX, 0, 255);

       
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, output_data);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        printf("ADC_RAW: %d %d\n", read, output_data);

        vTaskDelay( 2 * portTICK_PERIOD_MS );
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    // Setup DAC
    example_ledc_init();
    adc2_config_channel_atten(ADC2_CHANNEL, ADC_ATTEN_DB_11);

    xTaskCreate(&RWVoltage, "read and write from potentiometer", 4096, NULL, 5, NULL);
}