#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define VRX_PIN 27
#define VRY_PIN 26
#define SW_PIN 22

#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13

uint32_t volatile last_time = 0;

void green_led_handler() {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debouncing de 250ms
    if(current_time - last_time > 250000) {
        last_time = current_time;
        gpio_put(GREEN_PIN, !gpio_get(GREEN_PIN));
    }
        
}

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    
    pwm_set_wrap(slice, wrap);
    pwm_set_enabled(slice,true);
    
    return slice;
}

int main() {
    stdio_init_all();
    uint wrap = 4095;

    uint blue_slice = pwm_init_gpio(BLUE_PIN, wrap);
    uint red_slice = pwm_init_gpio(RED_PIN, wrap);
    
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &green_led_handler);
    
    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read();

        adc_select_input(0);
        uint16_t vry_value = adc_read();

        pwm_set_gpio_level(BLUE_PIN, (vry_value * wrap) / 4095);
        pwm_set_gpio_level(RED_PIN, (vrx_value * wrap) / 4095);
        
        printf("VRX: %u, VRY: %u\n", vrx_value, vry_value);
        
        sleep_ms(500);
    } 
}
