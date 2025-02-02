#include "pico/stdlib.h"
#include "hardware/timer.h"

// Configurações de hardware
#define PINO_LED 12
#define PINO_BOTAO 5

// Parâmetros de operação
#define TEMPO_DEBOUNCE 50    // ms
#define NUM_ACIONAMENTOS 5   // vezes
#define DURACAO_PISCA 10000  // ms
#define INTERVALO_PISCA 100  // ms

// Estados da aplicação
typedef struct {
    volatile bool piscando;
    volatile absolute_time_t fim_pisca;
    uint contagem;
    struct repeating_timer temporizador;
} EstadoApp;

// Protótipos de funções
void configurar_led();
void configurar_botao();
bool verificar_botao();
void iniciar_sequencia(EstadoApp *estado);
bool callback_pisca(struct repeating_timer *t);

void configurar_led() {
    gpio_init(PINO_LED);
    gpio_set_dir(PINO_LED, GPIO_OUT);
}

void configurar_botao() {
    gpio_init(PINO_BOTAO);
    gpio_set_dir(PINO_BOTAO, GPIO_IN);
    gpio_pull_up(PINO_BOTAO);
}

bool verificar_botao() {
    if (!gpio_get(PINO_BOTAO)) {
        sleep_ms(TEMPO_DEBOUNCE);
        if (!gpio_get(PINO_BOTAO)) {
            while (!gpio_get(PINO_BOTAO)) sleep_ms(10);
            return true;
        }
    }
    return false;
}

void iniciar_sequencia(EstadoApp *estado) {
    estado->contagem = 0;
    estado->piscando = true;
    estado->fim_pisca = make_timeout_time_ms(DURACAO_PISCA);
    add_repeating_timer_ms(INTERVALO_PISCA, callback_pisca, estado, &estado->temporizador);
}

bool callback_pisca(struct repeating_timer *t) {
    EstadoApp *estado = (EstadoApp*)t->user_data;
    
    if (estado->piscando) {
        gpio_put(PINO_LED, !gpio_get(PINO_LED));
        
        if (absolute_time_diff_us(get_absolute_time(), estado->fim_pisca) <= 0) {
            estado->piscando = false;
            gpio_put(PINO_LED, false);
            return false;
        }
        return true;
    }
    return false;
}

int main() {
    stdio_init_all();
    configurar_led();
    configurar_botao();

    EstadoApp estado = {
        .piscando = false,
        .fim_pisca = nil_time,
        .contagem = 0
    };

    while (true) {
        if (verificar_botao()) {
            if (++estado.contagem >= NUM_ACIONAMENTOS) {
                iniciar_sequencia(&estado);
            }
        }
        sleep_ms(10);
    }

    return 0;
}