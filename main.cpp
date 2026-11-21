#include <algorithm>
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

static void timer(ledc_timer_t id, unsigned hz, ledc_timer_bit_t bits) {
    ledc_timer_config_t cfg{};
    cfg.speed_mode = LEDC_LOW_SPEED_MODE;
    cfg.timer_num = id;
    cfg.duty_resolution = bits;
    cfg.freq_hz = hz;
    cfg.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&cfg));
}

static void output(int pin, ledc_channel_t channel, ledc_timer_t id) {
    ledc_channel_config_t cfg{};
    cfg.gpio_num = pin;
    cfg.speed_mode = LEDC_LOW_SPEED_MODE;
    cfg.channel = channel;
    cfg.timer_sel = id;
    ESP_ERROR_CHECK(ledc_channel_config(&cfg));
}

static void duty(ledc_channel_t channel, unsigned value) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channel));
}

static adc_oneshot_unit_handle_t adc;
static void init_adc() {
    adc_oneshot_unit_init_cfg_t unit{};
    unit.unit_id = ADC_UNIT_1;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit, &adc));
    adc_oneshot_chan_cfg_t cfg{};
    cfg.atten = ADC_ATTEN_DB_12;
    cfg.bitwidth = ADC_BITWIDTH_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, ADC_CHANNEL_8, &cfg));
}

static int sample() {
    int raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc, ADC_CHANNEL_8, &raw));
    return raw;
}
extern "C" void app_main() {
    init_adc();
    timer(LEDC_TIMER_0, 5000, LEDC_TIMER_10_BIT);
    output(4, LEDC_CHANNEL_0, LEDC_TIMER_0);
    output(5, LEDC_CHANNEL_1, LEDC_TIMER_0);
    while (true) {
        int level = sample() / 4;
        duty(LEDC_CHANNEL_0, level);
        duty(LEDC_CHANNEL_1, level);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
