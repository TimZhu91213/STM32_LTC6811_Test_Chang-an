/**
 * STM32F407 HAL port of ADI bms_hardware
 * SPI1: PB3=SCK, PB4=MISO, PB5=MOSI; soft CS=SPI_CS1 (PB6)
 */
#include "bms_hardware.h"
#include "spi.h"
#include "main.h"
#include "stm32f4xx_hal.h"

void bms_hardware_init(void)
{
  /* CS already configured in MX_GPIO_Init as SPI_CS1; idle high */
  cs_high(0);

  if (hspi1.Instance != SPI1 || hspi1.State == HAL_SPI_STATE_RESET)
  {
    MX_SPI1_Init();
  }
}

void cs_low(uint8_t pin)
{
  (void)pin;
  HAL_GPIO_WritePin(SPI_CS1_GPIO_Port, SPI_CS1_Pin, GPIO_PIN_RESET);
}

void cs_high(uint8_t pin)
{
  (void)pin;
  HAL_GPIO_WritePin(SPI_CS1_GPIO_Port, SPI_CS1_Pin, GPIO_PIN_SET);
}

/* Busy-wait microseconds (approx @ SystemCoreClock) */
void delay_u(uint16_t micro)
{
  uint32_t cycles = ((SystemCoreClock / 1000000U) * (uint32_t)micro) / 5U;
  while (cycles--)
  {
    __NOP();
  }
}

void delay_m(uint16_t milli)
{
  HAL_Delay(milli);
}

uint32_t bms_millis(void)
{
  return HAL_GetTick();
}

void set_spi_freq(void)
{
  /* SPI baud set in MX_SPI1_Init */
}

static uint8_t spi_xfer(uint8_t tx)
{
  uint8_t rx = 0xFF;
  if (HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100) != HAL_OK)
  {
    __HAL_SPI_CLEAR_OVRFLAG(&hspi1);
    hspi1.State = HAL_SPI_STATE_READY;
    return 0xFF;
  }
  return rx;
}

void spi_write_array(uint8_t len, uint8_t data[])
{
  uint8_t i;
  for (i = 0; i < len; i++)
  {
    (void)spi_xfer(data[i]);
  }
}

void spi_write_read(uint8_t tx_Data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len)
{
  uint8_t i;
  for (i = 0; i < tx_len; i++)
  {
    (void)spi_xfer(tx_Data[i]);
  }
  for (i = 0; i < rx_len; i++)
  {
    rx_data[i] = spi_xfer(0xFF);
  }
}

uint8_t spi_read_byte(uint8_t tx_dat)
{
  (void)tx_dat;
  return spi_xfer(0xFF);
}
