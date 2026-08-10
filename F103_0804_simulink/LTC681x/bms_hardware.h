/*!
  LTC681x hardware library — STM32 port of ADI bms_hardware.h
*/
#ifndef BMSHARDWARE_H
#define BMSHARDWARE_H

#include <stdint.h>

/* Call once after MX_SPI1_Init() */
void bms_hardware_init(void);

void cs_low(uint8_t pin);
void cs_high(uint8_t pin);
void delay_u(uint16_t micro);
void delay_m(uint16_t milli);
void set_spi_freq(void);

void spi_write_array(uint8_t len, uint8_t data[]);
void spi_write_read(uint8_t tx_Data[], uint8_t tx_len,
                    uint8_t *rx_data, uint8_t rx_len);
uint8_t spi_read_byte(uint8_t tx_dat);

#endif
