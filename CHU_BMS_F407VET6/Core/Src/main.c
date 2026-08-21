/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "bms_hardware.h"
#include "F103_0804_simulink_model.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HEARTBEAT_PERIOD_MS  200U
#define SMOKE_STATS_PERIOD_MS 1000U

/* AT24C256: 32KB, 16-bit word address. HAL DevAddress is 8-bit write addr. */
#define AT24C256_DEV_ADDR    0xA0U
#define AT24C256_TEST_ADDR   0x0000U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t s_last_heartbeat_ms;
static uint32_t s_last_stats_ms;
static uint32_t s_prev_can1_ok;
static uint32_t s_prev_can1_fail;
static uint32_t s_prev_can2_ok;
static uint32_t s_prev_can2_fail;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Uart_Print(const char *s)
{
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), 50U);
}

static void SmokeStats_Process(void)
{
  uint32_t now = HAL_GetTick();
  uint32_t can1_ok;
  uint32_t can1_fail;
  uint32_t can2_ok;
  uint32_t can2_fail;
  char line[96];

  if ((now - s_last_stats_ms) < SMOKE_STATS_PERIOD_MS)
  {
    return;
  }
  s_last_stats_ms = now;

  CAN_SmokeTest_GetStats(&can1_ok, &can1_fail, &can2_ok, &can2_fail);
  (void)snprintf(line, sizeof(line),
                 "CAN smoke fps: CAN1 %lu fail+%lu | CAN2 %lu fail+%lu\r\n",
                 (unsigned long)(can1_ok - s_prev_can1_ok),
                 (unsigned long)(can1_fail - s_prev_can1_fail),
                 (unsigned long)(can2_ok - s_prev_can2_ok),
                 (unsigned long)(can2_fail - s_prev_can2_fail));
  s_prev_can1_ok = can1_ok;
  s_prev_can1_fail = can1_fail;
  s_prev_can2_ok = can2_ok;
  s_prev_can2_fail = can2_fail;
  Uart_Print(line);
}

static void Heartbeat_Process(void)
{
  uint32_t now = HAL_GetTick();

  if ((now - s_last_heartbeat_ms) < HEARTBEAT_PERIOD_MS)
  {
    return;
  }
  s_last_heartbeat_ms = now;
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

static void AT24C256_SmokeTest(void)
{
  uint8_t wr = 0xAAU;
  uint8_t rd = 0x00U;
  HAL_StatusTypeDef st;

  st = HAL_I2C_IsDeviceReady(&hi2c3, AT24C256_DEV_ADDR, 8U, 50U);
  if (st != HAL_OK)
  {
    Uart_Print("AT24 NACK (check WP/A2A1A0/pullup)\r\n");
    return;
  }

  st = HAL_I2C_Mem_Write(&hi2c3, AT24C256_DEV_ADDR, AT24C256_TEST_ADDR,
                         I2C_MEMADD_SIZE_16BIT, &wr, 1U, 100U);
  if (st != HAL_OK)
  {
    Uart_Print("AT24 write fail\r\n");
    return;
  }

  /* Internal write cycle ~5ms; poll ACK instead of a blind delay. */
  st = HAL_I2C_IsDeviceReady(&hi2c3, AT24C256_DEV_ADDR, 40U, 5U);
  if (st != HAL_OK)
  {
    Uart_Print("AT24 write cycle timeout\r\n");
    return;
  }

  st = HAL_I2C_Mem_Read(&hi2c3, AT24C256_DEV_ADDR, AT24C256_TEST_ADDR,
                        I2C_MEMADD_SIZE_16BIT, &rd, 1U, 100U);
  if (st != HAL_OK)
  {
    Uart_Print("AT24 read fail\r\n");
    return;
  }

  if (rd == wr)
  {
    Uart_Print("AT24 OK\r\n");
  }
  else
  {
    Uart_Print("AT24 NOT OK\r\n");
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Bootloader jumps with PRIMASK=1; HAL_Delay/SysTick need IRQs enabled. */
  __enable_irq();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_I2C3_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
  /* SPI CS idle high (Cube MX_GPIO_Init leaves RESET) */
  HAL_GPIO_WritePin(SPI_CS1_GPIO_Port, SPI_CS1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI_CS2_GPIO_Port, SPI_CS2_Pin, GPIO_PIN_SET);

  bms_hardware_init();
  CAN_UserInit();
  F103_0804_simulink_model_initialize();

  Uart_Print("CHU_BMS F407: BMS+RTOS; CAN2 0x700 -> BL\r\n");
  AT24C256_SmokeTest();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Scheduler should never return; keep empty if it does. */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
