#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define DS1307_ADDR 0x68

// Registradores do DS1307
#define REG_SECONDS 0x00
#define REG_MINUTES 0x01
#define REG_HOURS   0x02
#define REG_DAY     0x03
#define REG_DATE    0x04
#define REG_MONTH   0x05
#define REG_YEAR    0x06
#define REG_CONTROL 0x07

// Função para converter decimal para BCD
uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Função para converter BCD para decimal
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Função para escrever um byte no RTC
void rtc_write(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, buf, 2, false);
}

// Função para ler um byte do RTC
uint8_t rtc_read(uint8_t reg) {
    uint8_t data;
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, DS1307_ADDR, &data, 1, false);
    return data;
}

// Função para configurar data e hora inicial
void set_initial_time() {
    // Para o relógio (bit 7 do registro de segundos)
    rtc_write(REG_SECONDS, dec_to_bcd(0));
    
    // Configura hora no formato 24h
    rtc_write(REG_HOURS, dec_to_bcd(13) & 0x3F); // 13 horas (formato 24h)
    rtc_write(REG_MINUTES, dec_to_bcd(27));      // 27 minutos
    
    // Configura data
    rtc_write(REG_DATE, dec_to_bcd(24));    // Dia 24
    rtc_write(REG_MONTH, dec_to_bcd(9));    // Mês 9 (Setembro)
    rtc_write(REG_YEAR, dec_to_bcd(24));    // Ano 24 (2024)
    rtc_write(REG_DAY, dec_to_bcd(3));      // Dia da semana (1-7)
    
    // Inicia o relógio (limpa bit 7 do registro de segundos)
    rtc_write(REG_SECONDS, dec_to_bcd(0));
}

// Função para ler data e hora atual
void read_time() {
    // Lê todos os registros individualmente
    uint8_t seconds = bcd_to_dec(rtc_read(REG_SECONDS) & 0x7F);
    uint8_t minutes = bcd_to_dec(rtc_read(REG_MINUTES));
    uint8_t hours = bcd_to_dec(rtc_read(REG_HOURS) & 0x3F);
    uint8_t day = bcd_to_dec(rtc_read(REG_DATE));
    uint8_t month = bcd_to_dec(rtc_read(REG_MONTH));
    uint8_t year = bcd_to_dec(rtc_read(REG_YEAR));
    
    printf("%02d/%02d/20%02d - %02d:%02d:%02d\n", 
           day, month, year, hours, minutes, seconds);
}

int main() {
    stdio_init_all();
    
    // Inicializa I2C com 100KHz
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Aguarda o console serial
    sleep_ms(2000);
    printf("\nIniciando RTC DS1307...\n");
    
    // Configura data e hora inicial
    set_initial_time();
    printf("Data e hora configuradas para 24/09/2024 - 13:27:00\n\n");
    
    while(1) {
        read_time();
        sleep_ms(5000); // Aguarda 5 segundos
    }
    
    return 0;
}