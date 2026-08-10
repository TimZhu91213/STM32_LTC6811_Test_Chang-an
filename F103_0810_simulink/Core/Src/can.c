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
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_4TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
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
#include "cmsis_os.h"
#include "task.h"

/* 1 = mailbox free / TX done; Task waits here instead of polling */
static osSemaphoreId_t s_can_tx_sem;

/* Accept-all filter (mask=0), then call HAL_CAN_Start — same order as f103_can_26_2_18 */
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

/**
 * @brief Create TX-done semaphore and enable mailbox-empty IRQ.
 * @note  Call after osKernelInitialize(), before tasks that send CAN.
 */
void MyCAN_InitTxIT(void)
{
  const osSemaphoreAttr_t attr = { .name = "can_tx" };

  if (s_can_tx_sem == NULL)
  {
    /* initial_count=1: allowed to send first frame immediately */
    s_can_tx_sem = osSemaphoreNew(1, 1, &attr);
  }

  /* Idempotent: safe if called from both main and MX_FREERTOS_Init */
  (void)HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY);
}

static void MyCAN_TxDoneFromISR(void)
{
  if (s_can_tx_sem != NULL)
  {
    (void)osSemaphoreRelease(s_can_tx_sem);
  }
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
  (void)hcan_ptr;
  MyCAN_TxDoneFromISR();
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
  (void)hcan_ptr;
  MyCAN_TxDoneFromISR();
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
  (void)hcan_ptr;
  MyCAN_TxDoneFromISR();
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan_ptr)
{
  (void)hcan_ptr;
  /* Avoid deadlock if TX fails (no ACK / bus error) */
  MyCAN_TxDoneFromISR();
}

/**
 * @brief Interrupt-assisted transmit: wait TX-done sem, then AddTxMessage.
 *        Completion is signaled from TxMailbox*CompleteCallback (ISR).
 */
HAL_StatusTypeDef MyCAN_Transmit(CAN_TxHeaderTypeDef *TxMessage, uint8_t *Data)
{
  uint32_t pTxMailbox;
  HAL_StatusTypeDef st;

  if ((TxMessage == NULL) || (Data == NULL) || (s_can_tx_sem == NULL))
  {
    return HAL_ERROR;
  }

  /* Block task until previous frame finished (or error released sem) */
  if (osSemaphoreAcquire(s_can_tx_sem, 20) != osOK)
  {
    return HAL_BUSY;
  }

  /* Keep AddTxMessage atomic vs other tasks (short critical section) */
  taskENTER_CRITICAL();
  st = HAL_CAN_AddTxMessage(&hcan, TxMessage, Data, &pTxMailbox);
  taskEXIT_CRITICAL();

  if (st != HAL_OK)
  {
    (void)osSemaphoreRelease(s_can_tx_sem);
  }
  /* else: sem released in TxMailbox*CompleteCallback / ErrorCallback */
  return st;
}

/* USER CODE END 1 */

