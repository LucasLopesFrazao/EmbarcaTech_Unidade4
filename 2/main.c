#include "pico/stdlib.h"
#include "hardware/timer.h"

// Definição dos pinos
#define LED_PIN 12
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Estrutura para controle do LED
typedef struct {
    uint pin;
    volatile bool is_blinking;
    absolute_time_t blink_end_time;
    struct repeating_timer timer;
} Led;

// Estrutura para controle do botão
typedef struct {
    uint pin;
    int press_count;
} Button;

// Callback do temporizador
static bool blink_timer_callback(struct repeating_timer *t) {
    Led *led = (Led *)t->user_data;
    if (led->is_blinking) {
        gpio_put(led->pin, !gpio_get(led->pin));
        
        if (absolute_time_diff_us(get_absolute_time(), led->blink_end_time) <= 0) {
            led->is_blinking = false;
            gpio_put(led->pin, false);
            return false;
        }
    }
    return true;
}

// Inicialização do LED
void led_init(Led *led, uint pin) {
    led->pin = pin;
    led->is_blinking = false;
    gpio_init(led->pin);
    gpio_set_dir(led->pin, GPIO_OUT);
    gpio_put(led->pin, false);
}

// Inicialização do botão
void button_init(Button *button, uint pin) {
    button->pin = pin;
    button->press_count = 0;
    gpio_init(button->pin);
    gpio_set_dir(button->pin, GPIO_IN);
    gpio_pull_up(button->pin);
}

// Função para iniciar/restartar a piscada do LED com intervalo ajustável
void led_start_blinking(Led *led, uint32_t duration_ms, int32_t interval_ms) {
    if (led->is_blinking) {
        cancel_repeating_timer(&led->timer);
    }
    
    led->is_blinking = true;
    led->blink_end_time = make_timeout_time_ms(duration_ms);
    add_repeating_timer_ms(interval_ms, blink_timer_callback, led, &led->timer);
}

// Função para verificar pressionamento do botão com debounce
bool check_button_press(Button *button) {
    if (gpio_get(button->pin) == 0) {
        sleep_ms(50);
        if (gpio_get(button->pin) == 0) {
            // Espera soltar o botão
            while (gpio_get(button->pin) == 0) {
                sleep_ms(10);
            }
            return true;
        }
    }
    return false;
}

int main() {
    stdio_init_all();
    
    Led led;
    Button button_a;
    Button button_b;
    
    led_init(&led, LED_PIN);
    button_init(&button_a, BUTTON_A_PIN);
    button_init(&button_b, BUTTON_B_PIN);

    while (true) {
        // Verifica Botão A (5 presses para 5 Hz)
        if (check_button_press(&button_a)) {
            button_a.press_count++;
            
            if (button_a.press_count >= 5) {
                button_a.press_count = 0;
                led_start_blinking(&led, 10000, 100); // 10s, 100ms (5Hz)
            }
        }

        // Verifica Botão B (1 press para 1 Hz)
        if (check_button_press(&button_b)) {
            if (led.is_blinking) {
                // Calcula tempo restante
                absolute_time_t now = get_absolute_time();
                int64_t remaining_us = absolute_time_diff_us(now, led.blink_end_time);
                if (remaining_us > 0) {
                    led_start_blinking(&led, remaining_us / 1000, 500); // Mantém tempo restante
                }
            } else {
                led_start_blinking(&led, 10000, 500); // 10s, 500ms (1Hz)
            }
        }
        
        sleep_ms(10);
    }
    
    return 0;
}