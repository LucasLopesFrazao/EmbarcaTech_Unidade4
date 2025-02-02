#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

const uint LED_RED = 13;   // PWM de 1kHz
const uint LED_GREEN = 11; // PWM de 10kHz
const uint LED_BLUE = 12;  // Compartilha slice com o vermelho

const uint16_t PERIOD_1KHZ = 12500;  // 1kHz: (125MHz / 10) / 1000
const uint16_t PERIOD_10KHZ = 1250;  // 10kHz: (125MHz / 10) / 10000

// Duty cycle inicial de 5% para vermelho e azul (mesmo período)
uint16_t duty_red = PERIOD_1KHZ / 20;
uint16_t duty_blue = PERIOD_1KHZ / 20;

uint16_t duty_green = PERIOD_10KHZ / 20;

bool timer_callback(struct repeating_timer *t) {
    static uint32_t ms_counter = 0;
    ms_counter++;
    
    if (ms_counter >= 2000) {
        ms_counter = 0;
        
        // Atualiza LED vermelho
        duty_red += PERIOD_1KHZ / 20;
        if (duty_red >= PERIOD_1KHZ) duty_red = PERIOD_1KHZ / 20;
        pwm_set_gpio_level(LED_RED, duty_red);
        
        // Atualiza LED azul (mesmo período que vermelho)
        duty_blue += PERIOD_1KHZ / 20;
        if (duty_blue >= PERIOD_1KHZ) duty_blue = PERIOD_1KHZ / 20;
        pwm_set_gpio_level(LED_BLUE, duty_blue);
    }
    return true;
}

void setup_pwm() {
    // Configura LED vermelho (1kHz)
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    uint slice_red = pwm_gpio_to_slice_num(LED_RED);
    pwm_set_clkdiv(slice_red, 10.0f);
    pwm_set_wrap(slice_red, PERIOD_1KHZ);
    pwm_set_gpio_level(LED_RED, duty_red);
    pwm_set_enabled(slice_red, true);

    // Configura LED verde (10kHz)
    gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN);
    pwm_set_clkdiv(slice_green, 10.0f);
    pwm_set_wrap(slice_green, PERIOD_10KHZ);
    pwm_set_gpio_level(LED_GREEN, duty_green);
    pwm_set_enabled(slice_green, true);

    // Configura LED azul (mesmo slice que vermelho)
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE);
    pwm_set_clkdiv(slice_blue, 10.0f);
    pwm_set_wrap(slice_blue, PERIOD_1KHZ); // Força mesmo período que vermelho
    pwm_set_gpio_level(LED_BLUE, duty_blue);
    pwm_set_enabled(slice_blue, true);
}

int main() {
    stdio_init_all();
    setup_pwm();
    
    struct repeating_timer timer;
    add_repeating_timer_ms(1, timer_callback, NULL, &timer);
    
    while (true) tight_loop_contents();
}