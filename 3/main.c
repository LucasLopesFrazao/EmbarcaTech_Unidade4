#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define BAUD_RATE 9600
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5
#define READ_TIMEOUT_MS 100

// Função para ler linha completa (com espaços)
void read_line(char *buffer, int max_length) {
    int idx = 0;
    while (idx < max_length - 1) {
        int c = getchar();
        if (c == '\n' || c == '\r') { // Finaliza no Enter
            break;
        }
        buffer[idx++] = c;
    }
    buffer[idx] = '\0';
}

int main() {
    stdio_init_all();
    
    uart_init(uart1, BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(uart1, true);

    while(1) {
        char input[100];
        printf("Envie um dado: ");
        read_line(input, sizeof(input)); // Lê linha inteira
        
        uart_puts(uart1, input); // Envia via UART1
        
        // Recebe com timeout (mesma lógica anterior)
        absolute_time_t start_time = get_absolute_time();
        char received[100] = {0};
        int idx = 0;
        
        while(absolute_time_diff_us(start_time, get_absolute_time()) <= READ_TIMEOUT_MS * 1000) {
            while(uart_is_readable(uart1) && idx < sizeof(received)-1) {
                received[idx++] = uart_getc(uart1);
                start_time = get_absolute_time();
            }
            sleep_ms(1);
        }
        
        printf("Dado recebido: %s\n\n", received);
        
        // Limpa buffers
        while(uart_is_readable(uart1)) {
            uart_getc(uart1);
        }
    }
    
    return 0;
}