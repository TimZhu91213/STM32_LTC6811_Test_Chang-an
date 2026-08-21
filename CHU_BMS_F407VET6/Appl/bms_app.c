/**
 * @file bms_app.c
 * @brief Simulink C Caller wrappers for LTC6813 sample / open-wire.
 *
 * Return value convention (uint8_t, Simulink-friendly):
 *   Sample*:  0 = OK, 1 = PEC / ADC poll timeout / read fault
 *   Detect*:  0 = no open wire, 1 = at least one open OR ADC poll timeout
 * Measurement / flag data are always in the output arrays, not in the return.
 *
 * Timeout: only here (not LTC681x/LTC6813). Open-wire reimplemented with
 * adow/adax/axow + Bms_PollAdcTimeout so library pollAdc is never called.
 */
#include "bms_app.h"

#ifdef USE_HAL_DRIVER
#include "main.h"
#include "LTC6813_stm32.h"
#include "LTC681x.h"
#include "bms_hardware.h"
#include <string.h>

#ifndef BMS_POLL_ADC_TIMEOUT_MS
#define BMS_POLL_ADC_TIMEOUT_MS  20U
#endif

#define BMS_CELL_OW_THR   4000U
#define BMS_GPIO_OW_THR   150U
#define BMS_ADOW_REPEAT   5
#define BMS_AXOW_REPEAT   3
/* GPIO OW uses aux_channels+1 in datasheet helper */
#define BMS_GPIO_OW_CH    (BMS_GPIO_PER_IC + 1U)

static cell_asic s_ic[BMS_TOTAL_IC];
static uint8_t s_app_inited = 0U;

/* Open-wire scratch (static: avoid large task-stack VLAs) */
static uint16_t s_pull_up[BMS_TOTAL_IC][BMS_CELLS_PER_IC];
static uint16_t s_pull_dn[BMS_TOTAL_IC][BMS_CELLS_PER_IC];
static uint16_t s_aux_nom[BMS_TOTAL_IC][BMS_GPIO_OW_CH];
static uint16_t s_aux_pdn[BMS_TOTAL_IC][BMS_GPIO_OW_CH];

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

/**
 * @brief PLADC poll with wall-clock deadline (CS held low, same as library).
 * @return 0 done; 1 timeout
 */
static uint8_t Bms_PollAdcTimeout(uint32_t timeout_ms)
{
  uint32_t t0;
  uint8_t cmd[4];
  uint16_t cmd_pec;
  uint8_t adc_byte;

  cmd[0] = 0x07U;
  cmd[1] = 0x14U;
  cmd_pec = pec15_calc(2U, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

  cs_low(CS_PIN);
  spi_write_array(4U, cmd);
  t0 = bms_millis();

  for (;;)
  {
    if ((bms_millis() - t0) > timeout_ms)
    {
      cs_high(CS_PIN);
      return 1U;
    }
    adc_byte = spi_read_byte(0xFFU);
    if (adc_byte > 0U)
    {
      cs_high(CS_PIN);
      return 0U;
    }
  }
}

static void Bms_ClearU16(uint16_t *buf, uint16_t len)
{
  uint16_t i;
  for (i = 0U; i < len; i++)
  {
    buf[i] = 0U;
  }
}

static void Bms_ClearCellOwFlags(void)
{
  uint8_t cic;
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    (void)memset(s_ic[cic].cell_open_wire, 0, sizeof(s_ic[cic].cell_open_wire));
    s_ic[cic].system_open_wire = 0xFFFF;
  }
}

static void Bms_ClearGpioOwFlags(void)
{
  uint8_t cic;
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    (void)memset(s_ic[cic].gpio_open_wire, 0, sizeof(s_ic[cic].gpio_open_wire));
    s_ic[cic].system_open_wire = 0xFFFF;
  }
}

/** @return 0 OK; 1 timeout on any ADOW */
static uint8_t Bms_AdowPollRepeat(uint8_t pup, uint8_t repeats)
{
  uint8_t i;
  for (i = 0U; i < repeats; i++)
  {
    wakeup_idle(BMS_TOTAL_IC);
    LTC6813_adow(MD_7KHZ_3KHZ, pup, CELL_CH_ALL, DCP_DISABLED);
    if (Bms_PollAdcTimeout(BMS_POLL_ADC_TIMEOUT_MS) != 0U)
    {
      return 1U;
    }
  }
  return 0U;
}

/**
 * Cell open-wire (multi algorithm), same thresholds as LTC681x_run_openwire_multi.
 * @return 0 OK (flags filled); 1 ADC timeout
 */
static uint8_t Bms_RunCellOpenWire(void)
{
  uint8_t cic;
  uint8_t cell;
  int8_t opencells[BMS_CELLS_PER_IC + 1U];
  int8_t n;
  int8_t i;
  int8_t j;
  int8_t k;
  const uint8_t nch = (uint8_t)BMS_CELLS_PER_IC;

  Bms_ClearCellOwFlags();
  wakeup_sleep(BMS_TOTAL_IC);
  LTC6813_clrcell();

  if (Bms_AdowPollRepeat(PULL_UP_CURRENT, BMS_ADOW_REPEAT) != 0U)
  {
    return 1U;
  }
  wakeup_idle(BMS_TOTAL_IC);
  (void)LTC6813_rdcv(0U, BMS_TOTAL_IC, s_ic);
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (cell = 0U; cell < nch; cell++)
    {
      s_pull_up[cic][cell] = s_ic[cic].cells.c_codes[cell];
    }
  }

  if (Bms_AdowPollRepeat(PULL_DOWN_CURRENT, BMS_ADOW_REPEAT) != 0U)
  {
    return 1U;
  }
  wakeup_idle(BMS_TOTAL_IC);
  (void)LTC6813_rdcv(0U, BMS_TOTAL_IC, s_ic);
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (cell = 0U; cell < nch; cell++)
    {
      s_pull_dn[cic][cell] = s_ic[cic].cells.c_codes[cell];
    }
  }

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    n = 0;
    s_ic[cic].system_open_wire = 0xFFFF;

    for (cell = 0U; cell < nch; cell++)
    {
      uint16_t delta;
      if (s_pull_dn[cic][cell] < s_pull_up[cic][cell])
      {
        delta = (uint16_t)(s_pull_up[cic][cell] - s_pull_dn[cic][cell]);
      }
      else
      {
        delta = 0U;
      }

      if (delta > BMS_CELL_OW_THR)
      {
        opencells[n++] = (int8_t)(cell + 1U);
        for (j = (int8_t)cell; j < (int8_t)(nch - 3U); j++)
        {
          if (s_pull_up[cic][j + 2] == 0U)
          {
            opencells[n++] = (int8_t)(j + 2);
          }
        }
        if ((cell == (nch - 4U)) && (s_pull_dn[cic][nch - 3U] == 0U))
        {
          opencells[n++] = (int8_t)(nch - 2U);
        }
      }
    }

    if (s_pull_dn[cic][0] == 0U)
    {
      opencells[n++] = 0;
    }
    if (s_pull_dn[cic][nch - 1U] == 0U)
    {
      opencells[n++] = (int8_t)nch;
    }
    if (s_pull_dn[cic][nch - 2U] == 0U)
    {
      opencells[n++] = (int8_t)(nch - 1U);
    }

    for (i = 0; i < n; i++)
    {
      for (j = (int8_t)(i + 1); j < n;)
      {
        if (opencells[i] == opencells[j])
        {
          for (k = j; k < n; k++)
          {
            opencells[k] = opencells[k + 1];
          }
          n--;
        }
        else
        {
          j++;
        }
      }
    }

    for (i = 0; i < n; i++)
    {
      for (j = 0; j < (int8_t)(n - 1); j++)
      {
        if (opencells[j] > opencells[j + 1])
        {
          k = opencells[j];
          opencells[j] = opencells[j + 1];
          opencells[j + 1] = k;
        }
      }
    }

    for (i = 0; i < n; i++)
    {
      int8_t line = opencells[i];
      if ((line >= 0) && (line < (int8_t)BMS_CELL_OW_PER_IC))
      {
        s_ic[cic].cell_open_wire[line] = 1U;
      }
    }
    if (n > 0)
    {
      s_ic[cic].system_open_wire = opencells[0];
    }
  }
  return 0U;
}

/**
 * GPIO/temp open-wire (same idea as LTC681x_run_gpio_openwire).
 * @return 0 OK; 1 ADC timeout
 */
static uint8_t Bms_RunGpioOpenWire(void)
{
  uint8_t cic;
  uint8_t ch;
  uint8_t i;
  const uint8_t nch = (uint8_t)BMS_GPIO_OW_CH;

  Bms_ClearGpioOwFlags();
  wakeup_sleep(BMS_TOTAL_IC);
  LTC6813_clraux();

  for (i = 0U; i < BMS_AXOW_REPEAT; i++)
  {
    wakeup_idle(BMS_TOTAL_IC);
    LTC6813_adax(MD_7KHZ_3KHZ, AUX_CH_ALL);
    if (Bms_PollAdcTimeout(BMS_POLL_ADC_TIMEOUT_MS) != 0U)
    {
      return 1U;
    }
  }
  wakeup_idle(BMS_TOTAL_IC);
  (void)LTC6813_rdaux(0U, BMS_TOTAL_IC, s_ic);
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (ch = 0U; ch < nch; ch++)
    {
      s_aux_nom[cic][ch] = (ch < BMS_GPIO_PER_IC) ? s_ic[cic].aux.a_codes[ch] : 0U;
    }
  }

  LTC6813_clraux();
  for (i = 0U; i < BMS_AXOW_REPEAT; i++)
  {
    wakeup_idle(BMS_TOTAL_IC);
    LTC6813_axow(MD_7KHZ_3KHZ, PULL_DOWN_CURRENT);
    if (Bms_PollAdcTimeout(BMS_POLL_ADC_TIMEOUT_MS) != 0U)
    {
      return 1U;
    }
  }
  wakeup_idle(BMS_TOTAL_IC);
  (void)LTC6813_rdaux(0U, BMS_TOTAL_IC, s_ic);
  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (ch = 0U; ch < nch; ch++)
    {
      s_aux_pdn[cic][ch] = (ch < BMS_GPIO_PER_IC) ? s_ic[cic].aux.a_codes[ch] : 0U;
    }
  }

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    s_ic[cic].system_open_wire = 0xFFFF;
    for (ch = 0U; ch < nch; ch++)
    {
      uint16_t delta;
      if (s_aux_pdn[cic][ch] > s_aux_nom[cic][ch])
      {
        delta = (uint16_t)(s_aux_pdn[cic][ch] - s_aux_nom[cic][ch]);
      }
      else
      {
        delta = 0U;
      }

      if (ch < 5U)
      {
        if (delta > BMS_GPIO_OW_THR)
        {
          s_ic[cic].system_open_wire = (long)(ch + 1U);
          s_ic[cic].gpio_open_wire[ch] = 1U;
        }
      }
      else if (ch > 5U)
      {
        if (delta > BMS_GPIO_OW_THR)
        {
          uint8_t gpio_idx = (uint8_t)(ch - 1U);
          s_ic[cic].system_open_wire = (long)ch;
          if (gpio_idx < BMS_GPIO_OW_PER_IC)
          {
            s_ic[cic].gpio_open_wire[gpio_idx] = 1U;
          }
        }
      }
    }
  }
  return 0U;
}
#endif /* USE_HAL_DRIVER */

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
  /* CHU_BMS_F407VET6: LED1/LED2 (PA6/PA7, active high) */
  GPIO_PinState level = on ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, level);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, level);
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
  if (Bms_PollAdcTimeout(BMS_POLL_ADC_TIMEOUT_MS) != 0U)
  {
    Bms_ClearU16(codes, BMS_CELL_CODE_LEN);
    return 1U;
  }
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
  if (Bms_PollAdcTimeout(BMS_POLL_ADC_TIMEOUT_MS) != 0U)
  {
    Bms_ClearU16(codes, BMS_GPIO_CODE_LEN);
    return 1U;
  }
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
  if (Bms_RunCellOpenWire() != 0U)
  {
    for (i = 0U; i < BMS_CELL_OW_LEN; i++)
    {
      flags[i] = 0U;
    }
    return 1U;
  }

  for (cic = 0U; cic < BMS_TOTAL_IC; cic++)
  {
    for (line = 0U; line < BMS_CELL_OW_PER_IC; line++)
    {
      flags[idx++] = s_ic[cic].cell_open_wire[line];
    }
  }
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
  if (Bms_RunGpioOpenWire() != 0U)
  {
    for (i = 0U; i < BMS_GPIO_OW_LEN; i++)
    {
      flags[i] = 0U;
    }
    return 1U;
  }

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
