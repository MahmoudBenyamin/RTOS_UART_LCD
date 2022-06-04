/*
 * main.c
 *
 *  Created on: May 23, 2022
 *      Author: mahmo
 */
#include "stdTypes.h"
#include "errorState.h"
#include "AVR_REG.h"

#include "DIO/DIO_int.h"
#include "GIE/GIE_int.h"
#include "UART/UART_Interface.h"
#include "LCD/LCD_int.h"

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/queue.h"
xSemaphoreHandle UART_Semphr = NULL;
xQueueHandle ISR_UART_Handler;
xQueueHandle UART_LCD_Handler;

void UartRecieve (void *pv);
//void DIO_LED (void *pv);
void LCD_Display (void *pv);
void UartNotificationISR (void);

int main()
{
	/****************** init UART **************************************/
	UART_Init();
	UART_Callback(UartNotificationISR);
	/*************************PIN init ********************************/
	DIO_enuSetPinDirection(DIO_u8GROUP_A, PIN0, DIO_u8OUTPUT);
	/********************************* create semaphore and  queues********/
	UART_Semphr = xSemaphoreCreateCounting(10,0);
	ISR_UART_Handler = xQueueCreate(10, 1);
	UART_LCD_Handler = xQueueCreate(10, 1);
	/***************************** LCD init ***************************/
	LCD_enuInit();
	/************************************create your tasks ************************************
	 * 1 - task code
	 * 2- task name
	 * 3- stack size
	 * 4- parameters
	 * 5- priority
	 * 6- handler
	 */
	xTaskCreate(UartRecieve,NULL,100,NULL,3,NULL);
	//xTaskCreate(DIO_LED,NULL,100,NULL,2,NULL);
	xTaskCreate(LCD_Display,NULL,200,NULL,1,NULL);
    /*************************************** start scheduler **************/
	vTaskStartScheduler();
	while(1);
}
void UartRecieve (void *pv)
{
	u8 UartValue = 0;
	while(1)
	{
		if(xSemaphoreTake(UART_Semphr,35) == pdPASS)
		{
			if((xQueueReceive(ISR_UART_Handler,&UartValue,5)) == pdPASS)
			{
				// received  successfully
				if((xQueueSend(UART_LCD_Handler,&UartValue,5)) == pdPASS)
				{
					// sent successfully
				}
				else
				{
					// didn't send
				}
			}
			else
			{
				// didn't receive
			}
		}
		vTaskDelay(10);
	}
}
/*void DIO_LED (void *pv)
{
	u8 Recieved = '0';
	while(1)
	{
		if(Recieved == '1')
			DIO_enuSetPinValue(DIO_u8GROUP_A, PIN0, DIO_u8HIGH);
		else
			DIO_enuSetPinValue(DIO_u8GROUP_A, PIN0, DIO_u8LOW);
		vTaskDelay(15);
	}
}*/
void UartNotificationISR (void)
{
	xSemaphoreGive(UART_Semphr);
	 static u8 valueRecieved = 0;
	 valueRecieved= UART_Receive();
	if((xQueueSend(ISR_UART_Handler,&valueRecieved,5)) == pdPASS)
	{
		// sent successfully
	}
	else
	{
		// didn't send
	}
}
void LCD_Display (void *pv)
{
	LCD_enuWriteString("values Received: ");
	LCD_enuGoToPosition(2, 1);
	u8 counter = 1;
	u8 DisplayValue;
	while(1)
	{
		if((xQueueReceive(UART_LCD_Handler,&DisplayValue,5)) == pdPASS)
		{
			// Received successfully
			LCD_enuWriteData(DisplayValue);
			counter++;
			if(DisplayValue == '1')
				DIO_enuSetPinValue(DIO_u8GROUP_A, PIN0, DIO_u8HIGH);
			else
				DIO_enuSetPinValue(DIO_u8GROUP_A, PIN0, DIO_u8LOW);
		}
		else
		{
			// didn't Receive
		}
		if (counter >= 16)
		{
			LCD_enuGoToPosition(2, 1);
			for (u8 i =0 ; i< 15;i++)
				LCD_enuWriteData(' ');
			counter = 1;
			LCD_enuGoToPosition(2, 1);
		}
		vTaskDelay(15);
	}
}
