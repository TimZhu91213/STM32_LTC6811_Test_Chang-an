/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_TS__Pin GPIO_PIN_4
#define ADC_TS__GPIO_Port GPIOA
#define ADC_Precharge__Pin GPIO_PIN_5
#define ADC_Precharge__GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_7
#define LED2_GPIO_Port GPIOA
#define AIR__Control_Pin GPIO_PIN_4
#define AIR__Control_GPIO_Port GPIOC
#define AIR__ControlC5_Pin GPIO_PIN_5
#define AIR__ControlC5_GPIO_Port GPIOC
#define Precharge_Relay_Control_Pin GPIO_PIN_0
#define Precharge_Relay_Control_GPIO_Port GPIOB
#define AIR__Auxiliary_Pin GPIO_PIN_1
#define AIR__Auxiliary_GPIO_Port GPIOB
#define AIR__AuxiliaryB2_Pin GPIO_PIN_2
#define AIR__AuxiliaryB2_GPIO_Port GPIOB
#define Precharge_Relay_Auxiliary_Pin GPIO_PIN_7
#define Precharge_Relay_Auxiliary_GPIO_Port GPIOE
#define ACC_Pin GPIO_PIN_14
#define ACC_GPIO_Port GPIOE
#define DI8_Pin GPIO_PIN_15
#define DI8_GPIO_Port GPIOE
#define BMS_Error_Pin GPIO_PIN_10
#define BMS_Error_GPIO_Port GPIOB
#define IMD_Error_Pin GPIO_PIN_11
#define IMD_Error_GPIO_Port GPIOB
#define BootLoader_Pin GPIO_PIN_14
#define BootLoader_GPIO_Port GPIOB
#define SPI_CS1_Pin GPIO_PIN_0
#define SPI_CS1_GPIO_Port GPIOD
#define SPI_CS2_Pin GPIO_PIN_1
#define SPI_CS2_GPIO_Port GPIOD
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_4
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_5
#define FLASH_MOSI_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
