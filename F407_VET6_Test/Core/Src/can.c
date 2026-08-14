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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 4;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = ENABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = ENABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN2_SCE_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(CAN2_SCE_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_SCE_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

#define CAN_SMOKE_CAN1_STD_ID      0x101U
#define CAN_SMOKE_CAN2_STD_ID      0x201U
#define CAN_SMOKE_STALL_MS         20U
#define CAN_ALL_TX_MAILBOXES       (CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2)

typedef struct
{
  uint32_t seq;
  uint32_t last_ok_ms;
  uint32_t ok;
  uint32_t fail;
  uint8_t bus_id;
  uint32_t std_id;
} CAN_SmokeBus_t;

static CAN_SmokeBus_t s_smoke_can1;
static CAN_SmokeBus_t s_smoke_can2;
static uint8_t s_smoke_active = 0U;

/* Counting sem tracks free TX mailboxes (max 3). */
static osSemaphoreId_t s_tx_sem_can1;
static osSemaphoreId_t s_tx_sem_can2;
#define MYCAN_TX_WAIT_MS     5U
#define MYCAN_TX_MBX_COUNT   3U
#define MYCAN_TX_IT_FLAGS    (CAN_IT_TX_MAILBOX_EMPTY)

static void CAN_ConfigFilter(CAN_HandleTypeDef *hcan, uint32_t filter_bank)
{
  CAN_FilterTypeDef filter = {0};

  filter.FilterBank = filter_bank;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterIdHigh = 0x0000;
  filter.FilterIdLow = 0x0000;
  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow = 0x0000;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterActivation = ENABLE;
  filter.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(hcan, &filter) != HAL_OK)
  {
    Error_Handler();
  }
}

void CAN_UserInit(void)
{
  /* CAN1: banks 0..13; CAN2: banks 14..27 (same as F407 BL). */
  CAN_ConfigFilter(&hcan1, 0);
  CAN_ConfigFilter(&hcan2, 14);

  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_CAN_Start(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    Error_Handler();
  }
}

/* Enable TX mailbox-empty IRQ + RX on CAN2; create TX mailbox semaphores. */
void MyCAN_InitTxIT(void)
{
  uint32_t free1;
  uint32_t free2;

  free1 = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
  free2 = HAL_CAN_GetTxMailboxesFreeLevel(&hcan2);
  if (free1 > MYCAN_TX_MBX_COUNT)
  {
    free1 = MYCAN_TX_MBX_COUNT;
  }
  if (free2 > MYCAN_TX_MBX_COUNT)
  {
    free2 = MYCAN_TX_MBX_COUNT;
  }

  if (s_tx_sem_can1 == NULL)
  {
    s_tx_sem_can1 = osSemaphoreNew(MYCAN_TX_MBX_COUNT, free1, NULL);
  }
  if (s_tx_sem_can2 == NULL)
  {
    s_tx_sem_can2 = osSemaphoreNew(MYCAN_TX_MBX_COUNT, free2, NULL);
  }

  (void)HAL_CAN_ActivateNotification(&hcan1, MYCAN_TX_IT_FLAGS);
  (void)HAL_CAN_ActivateNotification(&hcan2, MYCAN_TX_IT_FLAGS | CAN_IT_RX_FIFO0_MSG_PENDING);
}

static osSemaphoreId_t MyCAN_TxSem(CAN_HandleTypeDef *hcan)
{
  return (hcan->Instance == CAN1) ? s_tx_sem_can1 : s_tx_sem_can2;
}

static void MyCAN_TxSemGiveFromIsr(CAN_HandleTypeDef *hcan)
{
  osSemaphoreId_t sem = MyCAN_TxSem(hcan);
  if (sem != NULL)
  {
    (void)osSemaphoreRelease(sem);
  }
}

/**
 * @brief Soft recover after bus-off / HAL error (Abort only on recovery).
 */
static void MyCAN_ServiceBus(CAN_HandleTypeDef *hcan, uint32_t notify)
{
  static uint8_t was_busoff_can1 = 0U;
  static uint8_t was_busoff_can2 = 0U;
  uint8_t *was_busoff = (hcan->Instance == CAN1) ? &was_busoff_can1 : &was_busoff_can2;
  uint32_t esr = READ_REG(hcan->Instance->ESR);
  uint8_t busoff = ((esr & CAN_ESR_BOFF) != 0U) ? 1U : 0U;
  osSemaphoreId_t sem = MyCAN_TxSem(hcan);

  if (busoff != 0U)
  {
    *was_busoff = 1U;
    return;
  }

  if (*was_busoff != 0U)
  {
    *was_busoff = 0U;
    (void)HAL_CAN_AbortTxRequest(hcan, CAN_ALL_TX_MAILBOXES);
    if ((hcan->State != HAL_CAN_STATE_READY) && (hcan->State != HAL_CAN_STATE_LISTENING))
    {
      (void)HAL_CAN_Stop(hcan);
      (void)HAL_CAN_Start(hcan);
    }
    (void)HAL_CAN_ActivateNotification(hcan, notify);
    hcan->ErrorCode = HAL_CAN_ERROR_NONE;
    if (sem != NULL)
    {
      (void)osSemaphoreRelease(sem);
    }
  }
  else if (hcan->State == HAL_CAN_STATE_ERROR)
  {
    (void)HAL_CAN_Stop(hcan);
    (void)HAL_CAN_Start(hcan);
    (void)HAL_CAN_ActivateNotification(hcan, notify);
    hcan->ErrorCode = HAL_CAN_ERROR_NONE;
  }
}

/**
 * @brief Interrupt-assisted TX: free mailbox → fill; IRQ returns credit.
 *        Never Abort on normal path.
 */
static HAL_StatusTypeDef MyCAN_TransmitOne(CAN_HandleTypeDef *hcan,
                                            CAN_TxHeaderTypeDef *TxMessage,
                                            uint8_t *Data,
                                            uint32_t notify)
{
  uint32_t pTxMailbox;
  HAL_StatusTypeDef st;
  osSemaphoreId_t sem = MyCAN_TxSem(hcan);
  uint8_t got_credit = 0U;

  MyCAN_ServiceBus(hcan, notify);

  if ((READ_BIT(hcan->Instance->ESR, CAN_ESR_BOFF) != 0U))
  {
    return HAL_BUSY;
  }

  /* Prefer HW free level so a missed IRQ cannot stall the frame rate. */
  if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U)
  {
    if (sem != NULL)
    {
      (void)osSemaphoreAcquire(sem, MYCAN_TX_WAIT_MS);
      got_credit = 1U;
    }
    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U)
    {
      if ((got_credit != 0U) && (sem != NULL))
      {
        (void)osSemaphoreRelease(sem);
      }
      return HAL_BUSY;
    }
  }
  else if (sem != NULL)
  {
    if (osSemaphoreAcquire(sem, 0U) == osOK)
    {
      got_credit = 1U;
    }
  }

  taskENTER_CRITICAL();
  st = HAL_CAN_AddTxMessage(hcan, TxMessage, Data, &pTxMailbox);
  taskEXIT_CRITICAL();

  if (st != HAL_OK)
  {
    if ((got_credit != 0U) && (sem != NULL))
    {
      (void)osSemaphoreRelease(sem);
    }
    return st;
  }

  return HAL_OK;
}

/**
 * @brief Same frame on CAN1 and CAN2 via interrupt TX (Task03).
 */
HAL_StatusTypeDef MyCAN_Transmit(CAN_TxHeaderTypeDef *TxMessage, uint8_t *Data)
{
  HAL_StatusTypeDef st1;
  HAL_StatusTypeDef st2;

  if ((TxMessage == NULL) || (Data == NULL))
  {
    return HAL_ERROR;
  }

  st1 = MyCAN_TransmitOne(&hcan1, TxMessage, Data, MYCAN_TX_IT_FLAGS);
  st2 = MyCAN_TransmitOne(&hcan2, TxMessage, Data,
                          MYCAN_TX_IT_FLAGS | CAN_IT_RX_FIFO0_MSG_PENDING);

  if ((st1 == HAL_OK) || (st2 == HAL_OK))
  {
    return HAL_OK;
  }
  if (st2 != HAL_OK)
  {
    return st2;
  }
  return st1;
}

static HAL_StatusTypeDef CAN_AddTx(CAN_HandleTypeDef *hcan, uint32_t id, uint32_t ide,
                                   uint8_t *data, uint8_t len)
{
  CAN_TxHeaderTypeDef tx_header = {0};
  uint32_t tx_mailbox;

  if ((hcan == NULL) || (data == NULL))
  {
    return HAL_ERROR;
  }
  if (len > 8U)
  {
    len = 8U;
  }
  if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U)
  {
    return HAL_BUSY;
  }

  if (ide == CAN_ID_EXT)
  {
    tx_header.ExtId = id;
    tx_header.StdId = 0U;
    tx_header.IDE = CAN_ID_EXT;
  }
  else
  {
    tx_header.StdId = id;
    tx_header.ExtId = 0U;
    tx_header.IDE = CAN_ID_STD;
  }
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.DLC = len;
  tx_header.TransmitGlobalTime = DISABLE;

  return HAL_CAN_AddTxMessage(hcan, &tx_header, data, &tx_mailbox);
}

HAL_StatusTypeDef CAN_SendStdFrame(CAN_HandleTypeDef *hcan, uint32_t std_id, uint8_t *data, uint8_t len)
{
  return CAN_AddTx(hcan, std_id, CAN_ID_STD, data, len);
}

HAL_StatusTypeDef CAN_SendExtFrame(CAN_HandleTypeDef *hcan, uint32_t ext_id, uint8_t *data, uint8_t len)
{
  return CAN_AddTx(hcan, ext_id, CAN_ID_EXT, data, len);
}

static HAL_StatusTypeDef CAN_SendStdFrameAndWait(CAN_HandleTypeDef *hcan, uint32_t std_id, uint8_t *data, uint8_t len)
{
  uint32_t tickstart;
  uint32_t pending;

  if (CAN_SendStdFrame(hcan, std_id, data, len) != HAL_OK)
  {
    return HAL_ERROR;
  }

  tickstart = HAL_GetTick();
  pending = CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2;
  while (HAL_CAN_IsTxMessagePending(hcan, pending) != 0U)
  {
    if ((HAL_GetTick() - tickstart) > 10U)
    {
      return HAL_TIMEOUT;
    }
    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) > 0U)
    {
      break;
    }
  }

  return HAL_OK;
}

void CAN_SendTestMessages(void)
{
  static uint8_t counter = 0U;
  uint8_t can1_data[8] = {0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x00U};
  uint8_t can2_data[8] = {0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U, 0x00U};

  can1_data[7] = counter;
  can2_data[7] = counter;
  counter++;

  (void)CAN_SendStdFrameAndWait(&hcan1, 0x101U, can1_data, 8U);
  (void)CAN_SendStdFrameAndWait(&hcan2, 0x201U, can2_data, 8U);
}

static void CAN_DrainRx(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0U)
  {
    (void)HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);
  }
  while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO1) > 0U)
  {
    (void)HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data);
  }
}

static void CAN_RecoverIfStalled(CAN_HandleTypeDef *hcan, CAN_SmokeBus_t *bus)
{
  uint32_t now = HAL_GetTick();

  if ((HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U) &&
      ((now - bus->last_ok_ms) > CAN_SMOKE_STALL_MS))
  {
    (void)HAL_CAN_AbortTxRequest(hcan, CAN_ALL_TX_MAILBOXES);
    (void)HAL_CAN_ResetError(hcan);
    bus->last_ok_ms = now;
    bus->fail++;
  }
}

static HAL_StatusTypeDef CAN_SmokeQueueNext(CAN_HandleTypeDef *hcan, CAN_SmokeBus_t *bus)
{
  CAN_TxHeaderTypeDef tx_header = {0};
  uint32_t tx_mailbox;
  uint8_t data[8];
  uint32_t seq = bus->seq;

  data[0] = bus->bus_id;
  data[1] = 0x00U;
  data[2] = (uint8_t)(seq >> 24);
  data[3] = (uint8_t)(seq >> 16);
  data[4] = (uint8_t)(seq >> 8);
  data[5] = (uint8_t)seq;
  data[6] = (uint8_t)(hcan->Instance == CAN1 ? 0xC1U : 0xC2U);
  data[7] = (uint8_t)(seq ^ 0xA5U);

  if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U)
  {
    return HAL_BUSY;
  }

  tx_header.IDE = CAN_ID_STD;
  tx_header.StdId = bus->std_id;
  tx_header.ExtId = 0U;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.DLC = 8U;
  tx_header.TransmitGlobalTime = DISABLE;

  if (HAL_CAN_AddTxMessage(hcan, &tx_header, data, &tx_mailbox) != HAL_OK)
  {
    bus->fail++;
    return HAL_ERROR;
  }

  bus->seq = seq + 1U;
  bus->ok++;
  bus->last_ok_ms = HAL_GetTick();
  return HAL_OK;
}

static void CAN_SmokeFillBus(CAN_HandleTypeDef *hcan, CAN_SmokeBus_t *bus)
{
  CAN_DrainRx(hcan);
  CAN_RecoverIfStalled(hcan, bus);

  while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) > 0U)
  {
    if (CAN_SmokeQueueNext(hcan, bus) != HAL_OK)
    {
      break;
    }
  }
}

static void CAN_SmokeOnTxEmpty(CAN_HandleTypeDef *hcan)
{
  if (s_smoke_active == 0U)
  {
    return;
  }
  if (hcan->Instance == CAN1)
  {
    (void)CAN_SmokeQueueNext(&hcan1, &s_smoke_can1);
  }
  else if (hcan->Instance == CAN2)
  {
    (void)CAN_SmokeQueueNext(&hcan2, &s_smoke_can2);
  }
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (s_smoke_active != 0U)
  {
    CAN_SmokeOnTxEmpty(hcan);
  }
  else
  {
    MyCAN_TxSemGiveFromIsr(hcan);
  }
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (s_smoke_active != 0U)
  {
    CAN_SmokeOnTxEmpty(hcan);
  }
  else
  {
    MyCAN_TxSemGiveFromIsr(hcan);
  }
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (s_smoke_active != 0U)
  {
    CAN_SmokeOnTxEmpty(hcan);
  }
  else
  {
    MyCAN_TxSemGiveFromIsr(hcan);
  }
}

void CAN_SmokeTest_Init(void)
{
  uint32_t now = HAL_GetTick();

  s_smoke_active = 1U;
  s_smoke_can1.seq = 0U;
  s_smoke_can1.last_ok_ms = now;
  s_smoke_can1.ok = 0U;
  s_smoke_can1.fail = 0U;
  s_smoke_can1.bus_id = 1U;
  s_smoke_can1.std_id = CAN_SMOKE_CAN1_STD_ID;

  s_smoke_can2.seq = 0U;
  s_smoke_can2.last_ok_ms = now;
  s_smoke_can2.ok = 0U;
  s_smoke_can2.fail = 0U;
  s_smoke_can2.bus_id = 2U;
  s_smoke_can2.std_id = CAN_SMOKE_CAN2_STD_ID;
}

void CAN_SmokeTest_Process(void)
{
  CAN_SmokeFillBus(&hcan1, &s_smoke_can1);
  CAN_SmokeFillBus(&hcan2, &s_smoke_can2);
}

void CAN_SmokeTest_GetStats(uint32_t *can1_ok, uint32_t *can1_fail,
                            uint32_t *can2_ok, uint32_t *can2_fail)
{
  if (can1_ok != NULL)
  {
    *can1_ok = s_smoke_can1.ok;
  }
  if (can1_fail != NULL)
  {
    *can1_fail = s_smoke_can1.fail;
  }
  if (can2_ok != NULL)
  {
    *can2_ok = s_smoke_can2.ok;
  }
  if (can2_fail != NULL)
  {
    *can2_fail = s_smoke_can2.fail;
  }
}

/* USER CODE END 1 */

