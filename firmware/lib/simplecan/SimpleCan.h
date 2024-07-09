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


#pragma once

#include "Arduino.h"
#include <stm32g4xx_hal_fdcan.h>

enum CanSpeed
{
	Mbit1 = 10,
	Kbit500 = 20,
	Kbit250 = 40,
	Kbit125 = 80
};

/**
 CAN wrapper for G431 board.
*/
class SimpleCan
{
public:
	class RxHandler
	{
	public:
		RxHandler(uint16_t dataLength, void (*callback)(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData));
		~RxHandler();

		void notify(FDCAN_HandleTypeDef *hfdcan);

	private:
		FDCAN_RxHeaderTypeDef _rxHeader;
		uint8_t *_rxData;
		void (*_callback)(FDCAN_RxHeaderTypeDef, uint8_t *);
	};

	SimpleCan(int shutdown_pin = -1, int terminate_pin = -1);

	HAL_StatusTypeDef init(CanSpeed speed);
	HAL_StatusTypeDef configFilter(FDCAN_FilterTypeDef *filterDef);
	HAL_StatusTypeDef configGlobalFilter(
		uint32_t nonMatchingStd,
		uint32_t nonMatchingExt,
		uint32_t rejectRemoteStd,
		uint32_t rejectRemoteExt);
	HAL_StatusTypeDef activateNotification(RxHandler *rxHandler);
	HAL_StatusTypeDef deactivateNotification();
	HAL_StatusTypeDef start();
	HAL_StatusTypeDef stop();
	HAL_StatusTypeDef addMessageToTxFifoQ(
		FDCAN_TxHeaderTypeDef *pTxHeader,
		uint8_t *pTxData);
	// private: interrupt handler needs access to it
	static FDCAN_HandleTypeDef _hfdcan1;
	static RxHandler *_rxHandler;
	int shutdown_pin;
};
