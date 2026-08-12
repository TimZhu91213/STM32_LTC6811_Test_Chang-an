/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  /* Same as Stm32F103_CAN_BootLoader_test_0811: 36MHz/4/(1+13+4)=500k */
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* Accept-all filter (mask=0), then call HAL_CAN_Start */
HAL_StatusTypeDef Init_Filter(void)
{
  CAN_FilterTypeDef sFilterConfig = {0};

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  return HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
}

/* Placeholder name kept for freertos.c; enable RX pending IRQ for BL enter */
void MyCAN_InitTxIT(void)
{
  (void)HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/**
 * @brief Non-blocking bus health service: ABOM leaves bus-off in hardware;
 *        on recovery clear stuck TX mailboxes and restart HAL if needed.
 */
static void MyCAN_ServiceBus(void)
{
  static uint8_t was_busoff = 0U;
  uint32_t esr = READ_REG(hcan.Instance->ESR);
  uint8_t busoff = ((esr & CAN_ESR_BOFF) != 0U) ? 1U : 0U;

  if (busoff != 0U)
  {
    was_busoff = 1U;
    return;
  }

  if (was_busoff != 0U)
  {
    was_busoff = 0U;
    (void)HAL_CAN_AbortTxRequest(&hcan,
                                 CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
    if ((hcan.State != HAL_CAN_STATE_READY) && (hcan.State != HAL_CAN_STATE_LISTENING))
    {
      (void)HAL_CAN_Stop(&hcan);
      (void)HAL_CAN_Start(&hcan);
    }
    (void)HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    hcan.ErrorCode = HAL_CAN_ERROR_NONE;
  }
  else if (hcan.State == HAL_CAN_STATE_ERROR)
  {
    (void)HAL_CAN_Stop(&hcan);
    (void)HAL_CAN_Start(&hcan);
    (void)HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    hcan.ErrorCode = HAL_CAN_ERROR_NONE;
  }
}

/**
 * @brief Non-blocking TX: never waits on mailbox or TX-complete IRQ.
 *        Returns HAL_BUSY while bus-off or mailboxes full; auto-resumes after bus OK.
 */
HAL_StatusTypeDef MyCAN_Transmit(CAN_TxHeaderTypeDef *TxMessage, uint8_t *Data)
{
  uint32_t pTxMailbox;
  uint32_t now;
  static uint32_t s_last_abort_tick;

  if ((TxMessage == NULL) || (Data == NULL))
  {
    return HAL_ERROR;
  }

  MyCAN_ServiceBus();

  if ((READ_BIT(hcan.Instance->ESR, CAN_ESR_BOFF) != 0U))
  {
    return HAL_BUSY;
  }

  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0U)
  {
    now = HAL_GetTick();
    if ((now - s_last_abort_tick) >= 100U)
    {
      (void)HAL_CAN_AbortTxRequest(&hcan,
                                   CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
      s_last_abort_tick = now;
    }
    return HAL_BUSY;
  }

  return HAL_CAN_AddTxMessage(&hcan, TxMessage, Data, &pTxMailbox);
}

/* USER CODE END 1 */

