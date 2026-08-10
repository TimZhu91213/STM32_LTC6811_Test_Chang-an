/**
 * STM32 HAL port of ADI ltc6811_code/bms_hardware.cpp
 * SPI1: PA5=SCK, PA6=MISO, PA7=MOSI; soft CS=PA4
 */
#include "bms_hardware.h"
#include "spi.h"
#include "stm32f1xx_hal.h"

#define BMS_CS_GPIO_Port  GPIOA
#define BMS_CS_Pin        GPIO_PIN_4

void bms_hardware_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = BMS_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BMS_CS_GPIO_Port, &GPIO_InitStruct);
  cs_high(0);

  if (hspi1.Instance != SPI1 || hspi1.State == HAL_SPI_STATE_RESET)
  {
    MX_SPI1_Init();
  }
}

void cs_low(uint8_t pin)
{
  (void)pin;
  HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_RESET);
}

void cs_high(uint8_t pin)
{
  (void)pin;
  HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
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
