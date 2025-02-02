#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

// Definição dos pinos RGB da BitDogLab
#define LED_RED_PIN 16
#define LED_GREEN_PIN 17
#define LED_BLUE_PIN 18

// Constantes para controle do PWM
#define PWM_CLOCK 125000000  // Clock base do RP2040 (125MHz)
#define PWM_MAX_COUNT 65535  // Resolução máxima do PWM (16 bits)
#define DUTY_CYCLE_MIN 5     // Duty cycle mínimo (5%)
#define DUTY_CYCLE_MAX 100   // Duty cycle máximo (100%)
#define DUTY_CYCLE_STEP 5    // Incremento do duty cycle
#define UPDATE_INTERVAL_MS 2000 // Intervalo de atualização (2 segundos)

// Estrutura para controle do LED PWM
typedef struct {
    uint pin;
    uint slice;
    uint channel;
    float freq;
    uint duty_cycle;
} PwmLed;

// Variáveis globais
PwmLed red_led, green_led, blue_led;
struct repeating_timer update_timer;

// Função para configurar o PWM
void pwm_led_init(PwmLed *led, uint pin, float freq) {
    led->pin = pin;
    led->freq = freq;
    led->duty_cycle = DUTY_CYCLE_MIN;
    
    gpio_set_function(pin, GPIO_FUNC_PWM);
    led->slice = pwm_gpio_to_slice_num(pin);
    led->channel = pwm_gpio_to_channel(pin);
    
    // Calcula os valores para a frequência desejada
    float divider = PWM_CLOCK / (freq * PWM_MAX_COUNT);
    pwm_set_clkdiv(led->slice, divider);
    pwm_set_wrap(led->slice, PWM_MAX_COUNT);
    
    // Inicia com duty cycle mínimo
    uint level = (PWM_MAX_COUNT * led->duty_cycle) / 100;
    pwm_set_chan_level(led->slice, led->channel, level);
    pwm_set_enabled(led->slice, true);
}

// Função para atualizar o duty cycle
void update_duty_cycle(PwmLed *led) {
    led->duty_cycle += DUTY_CYCLE_STEP;
    if (led->duty_cycle > DUTY_CYCLE_MAX) {
        led->duty_cycle = DUTY_CYCLE_MIN;
    }
    
    uint level = (PWM_MAX_COUNT * led->duty_cycle) / 100;
    pwm_set_chan_level(led->slice, led->channel, level);
}

// Callback do timer para atualização do duty cycle
bool update_timer_callback(struct repeating_timer *t) {
    update_duty_cycle(&red_led);
    update_duty_cycle(&green_led);
    return true;
}

int main() {
    stdio_init_all();
    
    // Inicializa os LEDs com suas respectivas frequências
    pwm_led_init(&red_led, LED_RED_PIN, 1000);    // 1kHz
    pwm_led_init(&green_led, LED_GREEN_PIN, 10000); // 10kHz
    
    // Configura o LED azul com a mesma frequência do vermelho
    // Nota: Não é possível controlar o LED azul independentemente
    // pois ele compartilha o mesmo slice PWM com outro LED
    pwm_led_init(&blue_led, LED_BLUE_PIN, 1000);
    
    // Configura o timer para atualizar o duty cycle a cada 2 segundos
    add_repeating_timer_ms(UPDATE_INTERVAL_MS, update_timer_callback, NULL, &update_timer);
    
    // Loop principal
    while (true) {
        sleep_ms(100);
    }
    
    return 0;
}