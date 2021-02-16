#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#define TRUE		1
#define FALSE		0
#define NOT_PRESSED FALSE
#define PRESSED		TRUE

static void prvSetupHardware(void);
static void prvSetupUart(void);
void printmsg(char *msg);

void led_task_handler(void *params);
void button_task_handler(void *params);

uint8_t button_status_flag = NOT_PRESSED;

int main(void)
{
	RCC_DeInit();

	SystemCoreClockUpdate();

	prvSetupHardware();

	xTaskCreate(led_task_handler, "LED-TASK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(button_task_handler, "BUTTON-TASK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	vTaskStartScheduler();

	for(;;);

}

void led_task_handler(void *params)
{
	while(1)
	{
		if(button_status_flag == PRESSED)
		{
			GPIO_WriteBit(GPIOD, GPIO_Pin_15, Bit_SET);
		}
		else
		{
			GPIO_WriteBit(GPIOD, GPIO_Pin_15, Bit_RESET);
		}
	}
}

void button_task_handler(void *params)
{
	while(1)
	{
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
		{
			button_status_flag = PRESSED;
		}
		else
		{
			button_status_flag = NOT_PRESSED;
		}
	}
}

static void prvSetupUart(void)
{
	GPIO_InitTypeDef GPIO_UART_Pin;
	USART_InitTypeDef UART2_Init;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	memset(&GPIO_UART_Pin, 0, sizeof(GPIO_UART_Pin));

	GPIO_UART_Pin.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_UART_Pin.GPIO_Mode = GPIO_Mode_AF;
	GPIO_UART_Pin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_UART_Pin);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	memset(&UART2_Init, 0, sizeof(UART2_Init));

	UART2_Init.USART_BaudRate = 115200;
	UART2_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART2_Init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	UART2_Init.USART_Parity = USART_Parity_No;
	UART2_Init.USART_StopBits = USART_StopBits_1;
	UART2_Init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &UART2_Init);

	USART_Cmd(USART2, ENABLE);
}

void prvSetupGPIO(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef LedInit, ButtonInit;
	LedInit.GPIO_Mode = GPIO_Mode_OUT;
	LedInit.GPIO_OType = GPIO_OType_PP;
	LedInit.GPIO_Pin = GPIO_Pin_15;
	LedInit.GPIO_Speed = GPIO_Low_Speed;
	LedInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &LedInit);

	ButtonInit.GPIO_Mode = GPIO_Mode_IN;
	ButtonInit.GPIO_OType = GPIO_OType_PP;
	ButtonInit.GPIO_Pin = GPIO_Pin_0;
	ButtonInit.GPIO_Speed = GPIO_Low_Speed;
	ButtonInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &ButtonInit);
}

static void prvSetupHardware(void)
{
	prvSetupGPIO();
	prvSetupUart();
}

void printmsg(char *msg)
{
	for(uint32_t i = 0; i < strlen(msg); i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, msg[i]);
	}

	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
}
