/*
MIT License

Copyright (c) 2020 Owen Williams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

From: https://github.com/owennewo/youtube_can/tree/main/canfd_stm32
*/


#include "SimpleCan.h"

// will be called from: HAL_FDCAN_Init
extern "C" void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan);
extern "C" void FDCAN1_IT0_IRQHandler();
extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

FDCAN_HandleTypeDef SimpleCan::_hfdcan1 = { };
SimpleCan::RxHandler* SimpleCan::_rxHandler = NULL;

SimpleCan::SimpleCan(int _shutdown_pin, int _terminate_transceiver_pin)
{
	if (_hfdcan1.Instance != NULL)
	{
		Error_Handler();
	}

	_hfdcan1.Instance = FDCAN1;

	shutdown_pin = _shutdown_pin;

	if (_shutdown_pin > 0) {
		pinMode(_shutdown_pin, OUTPUT);
		// digitalWrite(_shutdown_pin, HIGH);
	}
	if (_terminate_transceiver_pin > 0) {
		pinMode(_terminate_transceiver_pin, OUTPUT);
		digitalWrite(_terminate_transceiver_pin, HIGH);
	}

}

HAL_StatusTypeDef SimpleCan::start(void)
{
	digitalWrite(shutdown_pin, LOW);
	return HAL_FDCAN_Start(&_hfdcan1);
}

HAL_StatusTypeDef SimpleCan::stop(void)
{
	digitalWrite(shutdown_pin, HIGH);
	return HAL_FDCAN_Stop(&_hfdcan1);
}

HAL_StatusTypeDef SimpleCan::init(CanSpeed speed)
{
	FDCAN_InitTypeDef *init = &_hfdcan1.Init;

	init->ClockDivider = FDCAN_CLOCK_DIV1;
	init->FrameFormat = FDCAN_FRAME_FD_BRS;
	init->Mode = FDCAN_MODE_NORMAL;
	init->AutoRetransmission = DISABLE;
	init->TransmitPause = ENABLE;
	init->ProtocolException = DISABLE;

	// 1 MBit: NominalPrescaler = 10
	// see: http://www.bittiming.can-wiki.info/
	// 170MHz / Prescaler / SyncJumpWith / (TimeSeg1 + TimeSeg2 + SyncSeg)
	// SyncSeg = SYNC_SEG = 1, TimeSeg1 = PROP_SEG + PHASE_SEG1, TimeSeg2 = PHASE_SEG2
	init->NominalPrescaler = (uint16_t) speed;
	init->NominalSyncJumpWidth = 1;
	init->NominalTimeSeg1 = 14;
	init->NominalTimeSeg2 = 2;

	init->DataPrescaler = 1;
	init->DataSyncJumpWidth = 4;
	init->DataTimeSeg1 = 5;
	init->DataTimeSeg2 = 4;
	init->StdFiltersNbr = 1;
	init->ExtFiltersNbr = 0;
	init->TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

	return HAL_FDCAN_Init(&_hfdcan1);
}

HAL_StatusTypeDef SimpleCan::configFilter(FDCAN_FilterTypeDef *filterDef)
{
	return HAL_FDCAN_ConfigFilter(&_hfdcan1, filterDef);
}

HAL_StatusTypeDef SimpleCan::configGlobalFilter(
	uint32_t nonMatchingStd,
	uint32_t nonMatchingExt,
	uint32_t rejectRemoteStd,
	uint32_t rejectRemoteExt)
{
	return HAL_FDCAN_ConfigGlobalFilter(&_hfdcan1, nonMatchingStd, nonMatchingExt, rejectRemoteStd, rejectRemoteExt);
}

HAL_StatusTypeDef SimpleCan::activateNotification(RxHandler *rxHandler)
{
	if (_rxHandler != NULL)
	{
		return HAL_ERROR;
	}
#if USE_HAL_FDCAN_REGISTER_CALLBACKS
	// This would make interrupt handling much easier and cleaner.
	// But how to activate it on Arduino platform?
	//_hfdcan1.Instance->RxFifo0Callback = ...;
#endif
	_rxHandler = rxHandler;
	return HAL_FDCAN_ActivateNotification(&_hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

HAL_StatusTypeDef SimpleCan::deactivateNotification()
{
	_rxHandler = NULL;
	return HAL_FDCAN_DeactivateNotification(&_hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE);
}

HAL_StatusTypeDef SimpleCan::addMessageToTxFifoQ(
	FDCAN_TxHeaderTypeDef *pTxHeader,
	uint8_t *pTxData)
{
	return HAL_FDCAN_AddMessageToTxFifoQ(&_hfdcan1, pTxHeader, pTxData);
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
	if (hfdcan == NULL || hfdcan->Instance != FDCAN1)
	{
		return;
	}

	RCC_PeriphCLKInitTypeDef periphClkInit = { };

	HAL_RCCEx_GetPeriphCLKConfig(&periphClkInit);

	// Initializes the peripherals clocks
	periphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_FDCAN;
	periphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
	{
		Error_Handler();
	}

	// Peripheral clock enable
	__HAL_RCC_FDCAN_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// FDCAN1 GPIO Configuration
	// PA11 ------> FDCAN1_RX
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PB9 ------> FDCAN1_TX
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// FDCAN1 interrupt Init
	HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
}

void FDCAN1_IT0_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&SimpleCan::_hfdcan1);
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		if (SimpleCan::_rxHandler == NULL)
		{
			return;
		}
		SimpleCan::_rxHandler->notify(hfdcan);
	}
}

SimpleCan::RxHandler::RxHandler(uint16_t dataLength, void (*callback)(FDCAN_RxHeaderTypeDef, uint8_t *))
{
	_rxData = new byte[dataLength];
	_callback = callback;
}

SimpleCan::RxHandler::~RxHandler()
{
	delete[] _rxData;
}

void SimpleCan::RxHandler::notify(FDCAN_HandleTypeDef *hfdcan)
{
	if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &_rxHeader, _rxData) != HAL_OK)
	{
		Error_Handler();
	}

	if (_callback != NULL)
	{
		_callback(_rxHeader, _rxData);
	}
}
