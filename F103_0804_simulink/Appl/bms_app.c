/**
 * @file bms_app.c
 * @brief Simulink C Caller wrappers for LTC6813 sample / open-wire.
 *
 * Return value convention (uint8_t, Simulink-friendly):
 *   Sample*:  0 = OK (no PEC error), 1 = PEC / read fault
 *   Detect*:  0 = no open wire, 1 = at least one open flag set
 * Measurement / flag data are always in the output arrays, not in the return.
 */
#include "bms_app.h"

#ifdef USE_HAL_DRIVER
#include "main.h"
#include "LTC6813_stm32.h"
#include "LTC681x.h"

static cell_asic s_ic[BMS_TOTAL_IC];
static uint8_t s_app_inited = 0U;

static void Bms_EnsureInit(void)
{
  uint8_t i;
  if (s_app_inited != 0U)
  {
    return;
  }
  LTC6813_init_reg_limits(BMS_TOTAL_IC, s_ic);
  LTC6813_init_cfg(BMS_TOTAL_IC, s_ic);
  LTC6813_init_cfgb(BMS_TOTAL_IC, s_ic);
  for (i = 0U; i < BMS_TOTAL_IC; i++)
  {
    s_ic[i].system_open_wire = 0xFFFF;
  }
  s_app_inited = 1U;
}

/* Map library pec (0 / count / negative) to 0=OK, 1=fault */
static uint8_t Bms_PecToStatus(int16_t pec)
{
  return (pec == 0) ? 0U : 1U;
}

static uint8_t Bms_AnyFlagSet(const uint8_t *flags, uint16_t len)
{
  uint16_t i;
  for (i = 0U; i < len; i++)
  {
    if (flags[i] != 0U)
    {
      return 1U;
    }
  }
  return 0U;
}
#endif

void Bms_StartCellAdc(void)
{
#ifdef USE_HAL_DRIVER
  Bms_StartCellAdcEx(MD_7KHZ_3KHZ, DCP_DISABLED, CELL_CH_ALL);
#endif
}

void Bms_StartCellAdcEx(uint8_t MD, uint8_t DCP, uint8_t CH)
{
#ifdef USE_HAL_DRIVER
  Bms_EnsureInit();
  wakeup_sleep(BMS_TOTAL_IC);
  LTC6813_adcv(MD, DCP, CH);
#else
  (void)MD;
  (void)DCP;
  (void)CH;
#endif
}

void Bms_LED_Control(uint8_t on)
{
#ifdef USE_HAL_DRIVER
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,
                    on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
  (void)on;
#endif
}

uint8_t Bms_SampleCellVoltage(uint16_t codes[BMS_CELL_CODE_LEN])
{
  uint16_t i;
#ifndef USE_HAL_DRIVER
  for (i = 0U; i < BMS_CELL_CODE_LEN; i++)
  {
    codes[i] = 0U;
  }
  return 0U;
#else
  uint8_t cic;
  uint8_t ch;
  int16_t pec;
  uint16_t idx = 0U;

  Bms_EnsureInit();
  wakeup_sleep(BMS_TOTAL_IC);
  LTC6813_adcv(MD_7KHZ_3KHZ, DCP_DISABLED, CELL_CH_ALL);
  (void)LTC6813_pollAdc();
  wakeup_idle(BMS_TOTAL_IC);
  pec = (int16_t)LTC6813_rdcv(0U, BMS_TOTAL_IC, s_ic);

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (ch = 0U; ch < BMS_CELLS_PER_IC; ch++)
    {
      codes[idx++] = s_ic[cic].cells.c_codes[ch];
    }
  }
  return Bms_PecToStatus(pec);
#endif
}

uint8_t Bms_SampleTemperature(uint16_t codes[BMS_GPIO_CODE_LEN])
{
  uint16_t i;
#ifndef USE_HAL_DRIVER
  for (i = 0U; i < BMS_GPIO_CODE_LEN; i++)
  {
    codes[i] = 0U;
  }
  return 0U;
#else
  uint8_t cic;
  uint8_t ch;
  int16_t pec;
  uint16_t idx = 0U;

  Bms_EnsureInit();
  wakeup_sleep(BMS_TOTAL_IC);
  LTC6813_adax(MD_7KHZ_3KHZ, AUX_CH_ALL);
  (void)LTC6813_pollAdc();
  wakeup_idle(BMS_TOTAL_IC);
  pec = (int16_t)LTC6813_rdaux(0U, BMS_TOTAL_IC, s_ic);

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (ch = 0U; ch < BMS_GPIO_PER_IC; ch++)
    {
      codes[idx++] = s_ic[cic].aux.a_codes[ch];
    }
  }
  return Bms_PecToStatus(pec);
#endif
}

uint8_t Bms_DetectCellOpenWire(uint8_t flags[BMS_CELL_OW_LEN])
{
  uint16_t i;
#ifndef USE_HAL_DRIVER
  for (i = 0U; i < BMS_CELL_OW_LEN; i++)
  {
    flags[i] = 0U;
  }
  return 0U;
#else
  uint8_t cic;
  uint8_t line;
  uint16_t idx = 0U;

  Bms_EnsureInit();
  LTC6813_run_openwire_multi(BMS_TOTAL_IC, s_ic);

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (line = 0U; line < BMS_CELL_OW_PER_IC; line++)
    {
      flags[idx++] = s_ic[cic].cell_open_wire[line];
    }
  }
  /* 0 = all OK, 1 = at least one open wire */
  return Bms_AnyFlagSet(flags, BMS_CELL_OW_LEN);
#endif
}

uint8_t Bms_DetectTempOpenWire(uint8_t flags[BMS_GPIO_OW_LEN])
{
  uint16_t i;
#ifndef USE_HAL_DRIVER
  for (i = 0U; i < BMS_GPIO_OW_LEN; i++)
  {
    flags[i] = 0U;
  }
  return 0U;
#else
  uint8_t cic;
  uint8_t g;
  uint16_t idx = 0U;

  Bms_EnsureInit();
  LTC6813_run_gpio_openwire(BMS_TOTAL_IC, s_ic);

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (g = 0U; g < BMS_GPIO_OW_PER_IC; g++)
    {
      flags[idx++] = s_ic[cic].gpio_open_wire[g];
    }
  }
  return Bms_AnyFlagSet(flags, BMS_GPIO_OW_LEN);
#endif
}
