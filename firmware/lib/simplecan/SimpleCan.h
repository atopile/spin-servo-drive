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
