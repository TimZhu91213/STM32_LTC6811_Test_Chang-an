/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "stm32f4xx_hal_pwr_ex.h"

#define RTC_BKP_MAGIC  0x32F2U
#define RTC_BKP_REG    RTC_BKP_DR0

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekday;
} rtc_cal_t;

static uint32_t s_rtc_preserve_unix;
static uint8_t s_rtc_preserve_valid;

static uint8_t rtc_is_leap_year(uint16_t year)
{
  return (((year % 4U) == 0U) && ((year % 100U) != 0U)) || ((year % 400U) == 0U) ? 1U : 0U;
}

static uint8_t rtc_days_in_month(uint16_t year, uint8_t month)
{
  static const uint8_t dim[] = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

  if ((month < 1U) || (month > 12U))
  {
    return 0U;
  }
  if (month == 2U)
  {
    return (uint8_t)(28U + rtc_is_leap_year(year));
  }
  return dim[month - 1U];
}

static void rtc_unix_to_calendar(uint32_t unix_ts, rtc_cal_t *cal)
{
  uint32_t days;
  uint32_t sod;
  uint16_t year;

  if (cal == NULL)
  {
    return;
  }

  days = unix_ts / 86400U;
  sod = unix_ts % 86400U;
  cal->hour = (uint8_t)(sod / 3600U);
  sod %= 3600U;
  cal->minute = (uint8_t)(sod / 60U);
  cal->second = (uint8_t)(sod % 60U);
  cal->weekday = (uint8_t)(((days + 4U) % 7U) + 1U);

  year = 1970U;
  while (1U)
  {
    uint32_t diy = 365U;
    if (rtc_is_leap_year(year) != 0U)
    {
      diy = 366U;
    }
    if (days < diy)
    {
      break;
    }
    days -= diy;
    year++;
  }

  cal->year = year;
  cal->month = 1U;
  while (cal->month <= 12U)
  {
    uint8_t dim = rtc_days_in_month(year, cal->month);
    if (days < dim)
    {
      break;
    }
    days -= dim;
    cal->month++;
  }
  cal->day = (uint8_t)(days + 1U);
}

static uint32_t rtc_calendar_to_unix(uint16_t year, uint8_t month, uint8_t day,
                                     uint8_t hour, uint8_t minute, uint8_t second)
{
  uint32_t days = 0U;
  uint16_t y;

  for (y = 1970U; y < year; y++)
  {
    days += rtc_is_leap_year(y) ? 366U : 365U;
  }
  for (uint8_t m = 1U; m < month; m++)
  {
    days += rtc_days_in_month(year, m);
  }
  days += (uint32_t)day - 1U;

  return (days * 86400U)
         + ((uint32_t)hour * 3600U)
         + ((uint32_t)minute * 60U)
         + (uint32_t)second;
}

static void rtc_set_from_unix(uint32_t unix_ts)
{
  rtc_cal_t cal;
  RTC_TimeTypeDef s_time = {0};
  RTC_DateTypeDef s_date = {0};

  rtc_unix_to_calendar(unix_ts, &cal);
  if ((cal.year < 2000U) || (cal.year > 2099U))
  {
    return;
  }

  s_time.Hours = cal.hour;
  s_time.Minutes = cal.minute;
  s_time.Seconds = cal.second;
  s_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  s_time.StoreOperation = RTC_STOREOPERATION_RESET;

  s_date.Year = (uint8_t)(cal.year - 2000U);
  s_date.Month = cal.month;
  s_date.Date = cal.day;
  s_date.WeekDay = cal.weekday;

  HAL_PWR_EnableBkUpAccess();
  (void)HAL_RTC_SetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
  (void)HAL_RTC_SetDate(&hrtc, &s_date, RTC_FORMAT_BIN);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_REG, RTC_BKP_MAGIC);
}
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  HAL_PWR_EnableBkUpAccess();
  s_rtc_preserve_valid = 0U;
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_REG) == RTC_BKP_MAGIC)
  {
    RTC_TimeTypeDef now_time = {0};
    RTC_DateTypeDef now_date = {0};

    (void)HAL_RTC_GetTime(&hrtc, &now_time, RTC_FORMAT_BIN);
    (void)HAL_RTC_GetDate(&hrtc, &now_date, RTC_FORMAT_BIN);
    s_rtc_preserve_unix = rtc_calendar_to_unix(
        (uint16_t)(2000U + now_date.Year),
        now_date.Month,
        now_date.Date,
        now_time.Hours,
        now_time.Minutes,
        now_time.Seconds);
    s_rtc_preserve_valid = 1U;
  }
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_REG) != RTC_BKP_MAGIC)
  {
    /** First power-up: set calendar origin and mark backup domain. */
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x1;
    sDate.Year = 0x0;
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_REG, RTC_BKP_MAGIC);
  }
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  /* CubeMX always resets calendar above; restore time saved before that. */
  if (s_rtc_preserve_valid != 0U)
  {
    rtc_set_from_unix(s_rtc_preserve_unix);
    s_rtc_preserve_valid = 0U;
  }
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void RTC_GetDateTime(RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
{
  if ((time == NULL) || (date == NULL))
  {
    return;
  }

  (void)HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN);
  (void)HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN);
}

uint32_t RTC_GetTotalSeconds(void)
{
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};
  static const uint16_t days_before_month[] = {
    0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U
  };

  RTC_GetDateTime(&time, &date);

  return ((uint32_t)date.Year * 365U * 86400U)
         + ((uint32_t)days_before_month[date.Month] * 86400U)
         + (((uint32_t)date.Date - 1U) * 86400U)
         + ((uint32_t)time.Hours * 3600U)
         + ((uint32_t)time.Minutes * 60U)
         + (uint32_t)time.Seconds;
}

/* USER CODE END 1 */

