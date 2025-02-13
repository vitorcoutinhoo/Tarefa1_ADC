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
#define BUTTON 5

#define CENTER_VALUE 2048
#define DEAD_ZONE 110  // Faixa de tolerância (o centro não é 2048 sempre)

uint32_t volatile last_time = 0;
bool volatile leds = true;

void buttons_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Debouncing de 250ms
    if (current_time - last_time > 250) {  
        last_time = current_time;
        
        if (gpio == SW_PIN)
            gpio_put(GREEN_PIN, !gpio_get(GREEN_PIN));
        
        if (gpio == BUTTON)
            leds = !leds;

    }

}

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    
    pwm_set_wrap(slice, wrap);
    pwm_set_enabled(slice, true);
    
    return slice;
}

int map_value(int value, int min_input, int max_input, int min_output, int max_output) {
    return (value - min_input) * (max_output - min_output) / (max_input - min_input) + min_output;
}

int main() {
    stdio_init_all();
    uint wrap = 4095;  // Valor máximo para o ADC (12 bits)

    pwm_init_gpio(BLUE_PIN, wrap);
    pwm_init_gpio(RED_PIN, wrap);

    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &buttons_handler);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &buttons_handler);

    while (true) {
        adc_select_input(0);  
        uint16_t vry_value = adc_read();
        
        adc_select_input(1); 
        uint16_t vrx_value = adc_read();

        int blue_pwm = 0;
        int red_pwm = 0;

        if(leds) {
            // Controle do LED Azul (eixo Y)
            if (vry_value < CENTER_VALUE - DEAD_ZONE) 
            blue_pwm = map_value(vry_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);

            else if (vry_value > CENTER_VALUE + DEAD_ZONE)
                blue_pwm = map_value(vry_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);
            
            // Controle do LED Vermelho (eixo X)
            if (vrx_value < CENTER_VALUE - DEAD_ZONE)
                red_pwm = map_value(vrx_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);

            else if (vrx_value > CENTER_VALUE + DEAD_ZONE) 
                red_pwm = map_value(vrx_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);
        }
        

        pwm_set_gpio_level(BLUE_PIN, blue_pwm);
        pwm_set_gpio_level(RED_PIN, red_pwm);
        
        printf("VRX: %u, VRY: %u, Red PWM: %d, Blue PWM: %d\n", vrx_value, vry_value, red_pwm, blue_pwm);
        
        sleep_ms(100);
    }
}

