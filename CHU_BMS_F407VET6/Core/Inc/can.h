/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_UserInit(void);
void MyCAN_InitTxIT(void);
HAL_StatusTypeDef MyCAN_Transmit(CAN_TxHeaderTypeDef *TxMessage, uint8_t *Data);
HAL_StatusTypeDef CAN_SendStdFrame(CAN_HandleTypeDef *hcan, uint32_t std_id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef CAN_SendExtFrame(CAN_HandleTypeDef *hcan, uint32_t ext_id, uint8_t *data, uint8_t len);
void CAN_SendTestMessages(void);
void CAN_SmokeTest_Init(void);
void CAN_SmokeTest_Process(void);
void CAN_SmokeTest_GetStats(uint32_t *can1_ok, uint32_t *can1_fail,
                            uint32_t *can2_ok, uint32_t *can2_fail);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

