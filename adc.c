#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// Pinos do joystick
#define VRX_PIN 27
#define VRY_PIN 26
#define SW_PIN 22

// Pinos dos leds RGB e do botão A
#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13
#define BUTTON 5

// Pinos e constantes do display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define CENTER_VALUE 2048 // Centro do joystick
#define DEAD_ZONE 200  // Faixa de tolerância (o centro não é 2048 sempre)

uint32_t volatile last_time = 0; // variável de tempo pra debouncing

bool volatile leds = true; // Flag pra ligar e desligar os LEDs PWM
bool volatile border = false; // Flag para alterar a borda do display

ssd1306_t ssd; // Estrutura do display

// Função de interrupção
void buttons_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Debouncing de 250ms
    if (current_time - last_time > 250) {  
        last_time = current_time; 
        
        // Alterna o estado do LED Verde
        if (gpio == SW_PIN){
            gpio_put(GREEN_PIN, !gpio_get(GREEN_PIN));
            border = !border; // Alterna a borda do display
        }
        
        // Alterna a Flag de PWM dos LEDs
        if (gpio == BUTTON)
            leds = !leds;

    }

}

// Inicializa um pino PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    
    pwm_set_wrap(slice, wrap);
    pwm_set_enabled(slice, true);
    
    return slice;
}

// Mapeia a posição do joystick para um valor determinado
int map_value(int value, int min_input, int max_input, int min_output, int max_output) {
    return (value - min_input) * (max_output - min_output) / (max_input - min_input) + min_output;
}

int main() {
    stdio_init_all();
    uint wrap = 4095;  // Valor máximo para o ADC (12 bits)

    // Configura os leds azul e vermelho como pwm
    pwm_init_gpio(BLUE_PIN, wrap);
    pwm_init_gpio(RED_PIN, wrap);

    // Inicia o adc e os pinos vrx e vry do joystick
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    // Inicia e configura o botão do joystick
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    // Inicia e configura o botão A
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    // Inicia e seta a direção do led verde
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    // Configura a função de interrupção
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &buttons_handler);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &buttons_handler);

    // Taxa de atualização do display em 400khz
    i2c_init(I2C_PORT, 400 * 1000);

    // Seta a função i²c e coloca os pinos do display em pull up
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL);

    // Inicializa e configura o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd);

    while (true) {
        // Pega o valor de vry
        adc_select_input(0);  
        uint16_t vry_value = adc_read();
        
        // Pega o valor de vrx
        adc_select_input(1); 
        uint16_t vrx_value = adc_read();

        // Seta o pwm como zero
        uint blue_pwm = 0;
        uint red_pwm = 0;
        
        // Caso PWM estiver ativo
        if(leds) {
            // Mapeia o valor do vry do joystick para um valor pwm para o led azul
            if (vry_value < CENTER_VALUE - DEAD_ZONE) 
                blue_pwm = map_value(vry_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);

            else if (vry_value > CENTER_VALUE + DEAD_ZONE)
                blue_pwm = map_value(vry_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);
            
            // Mapeia o valor do vrx do joystick para um valor pwm para o led vermelho
            if (vrx_value < CENTER_VALUE - DEAD_ZONE)
                red_pwm = map_value(vrx_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);

            else if (vrx_value > CENTER_VALUE + DEAD_ZONE) 
                red_pwm = map_value(vrx_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);
        }

        // Mapeia os valores do joystick para a posição no display
        uint x = map_value(vrx_value, 0, 4095, 0, 119);
        uint y = map_value(vry_value, 0, 4095, 55, 0);

        // Conifura o valor minimo e o máximo de x
        if (x == 118) x = 116;
        if (x == 0) x = 4;

        // Configura o valor minimo e máximo de y
        if (y == 55) y = 52;
        if (y == 1) y = 4;

        ssd1306_fill(&ssd, false); // Apaga o display

        // Borda quando o led verde esta ligado
        if (border) {
            // superior esquerdo
            ssd1306_hline(&ssd, 0, 14, 0, true);
            ssd1306_vline(&ssd, 0, 0, 14, true);
            
            // superior direito
            ssd1306_hline(&ssd, 111, 127, 0, true);
            ssd1306_vline(&ssd, 127, 0, 14, true);

            // inferior esquerdo
            ssd1306_hline(&ssd, 0, 14, 63, true);
            ssd1306_vline(&ssd, 0, 47, 63, true);

            // inferior direito
            ssd1306_hline(&ssd, 111, 127, 63, true);
            ssd1306_vline(&ssd, 127, 47, 63, true);

            // traçado superior
            ssd1306_hline(&ssd, 41, 84, 0, true);

            // traçado inferior
            ssd1306_hline(&ssd, 41, 84, 63, true);
        }
        else
            // Borda quando o led esta desligado
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);

        // Desenha o quadrado na posição y, x
        ssd1306_rect(&ssd, y, x, 8, 8, true, true);
        ssd1306_send_data(&ssd);
        
        // Seta o duty cicle dos pinos pwm
        pwm_set_gpio_level(BLUE_PIN, blue_pwm);
        pwm_set_gpio_level(RED_PIN, red_pwm);
        
        printf("VRX: %u, VRY: %u, Red PWM: %d, Blue PWM: %d, X: %d, Y: %d\n", vrx_value, vry_value, red_pwm, blue_pwm, x, y);
        sleep_ms(100);
    }
}

