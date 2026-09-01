//----------------------------------------------------------------------------//
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "cmt453x_usart.h"
#include "USB_Driver.h"
#include "utils.h"
#include "fifo.h"
#include <string.h>
//----------------------------------------------------------------------------//
#define USB_UART_BUFF_SIZE	512
typedef struct {
    u8 buf[USB_UART_BUFF_SIZE];
    uint32_t tail;
    uint32_t head;
}usbUartRing_t;
//----------------------------------------------------------------------------//
typedef struct{
	usbUartRing_t bufTx;			//Кольцевой буфер Tx
	usbUartRing_t bufRx;			//Кольцевой буфер Rx
	bool transmissionComplete;		//Передача завершена
}usbData_t;
usbData_t usbData;
//----------------------------------------------------------------------------//
#define USB_FIFO_RX_PUT(byte)	FIFO_PUT(usbData.bufRx, byte)
#define USB_FIFO_TX_PUT(byte)	FIFO_PUT(usbData.bufTx, byte)
#define USB_FIFO_RX_GET()		FIFO_GET(usbData.bufRx)
#define USB_FIFO_TX_GET()		FIFO_GET(usbData.bufTx)
#define USB_FIFO_RX_DROP()		FIFO_DROP(usbData.bufRx)
#define USB_FIFO_TX_DROP()		FIFO_DROP(usbData.bufTx)
#define USB_FIFO_TX_CNT()		FIFO_COUNT(usbData.bufTx)
#define USB_FIFO_RX_CNT()		FIFO_COUNT(usbData.bufRx);
//----------------------------------------------------------------------------//
#define USB_GPIO			GPIOB
#define USB_RX_Pin			GPIO_PIN_5
#define USB_TX_Pin			GPIO_PIN_4
#define USB_GPIO_AF			GPIO_AF3_USART2
//----------------------------------------------------------------------------//
#define	USB_UART			USART2
#define USB_IRQn			USART2_IRQn
//----------------------------------------------------------------------------//
#define USB_USART_CLK		RCC_APB1_PERIPH_USART2
#define USB_USART_GPIO_CLK	RCC_APB2_PERIPH_GPIOB
//----------------------------------------------------------------------------//
static void USB_UART_Init(void){
	GPIO_InitType GPIO_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	USART_InitType USART_InitStructure;
	
	RCC_EnableAPB1PeriphClk(USB_USART_CLK, ENABLE);
	RCC_EnableAPB2PeriphClk(USB_USART_GPIO_CLK | RCC_APB2_PERIPH_AFIO, ENABLE);
	
	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin				= USB_TX_Pin | USB_RX_Pin;
	GPIO_InitStructure.GPIO_Alternate	= USB_GPIO_AF;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.GPIO_Speed		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.GPIO_Pull		= GPIO_NO_PULL;
	GPIO_InitPeripheral(USB_GPIO, &GPIO_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel			= USB_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority	= 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_InitStructure.BaudRate			= 460800;
	USART_InitStructure.WordLength			= USART_WL_8B;
	USART_InitStructure.StopBits			= USART_STPB_1;
	USART_InitStructure.Parity				= USART_PE_NO;
	USART_InitStructure.HardwareFlowControl	= USART_HFCTRL_NONE;
	USART_InitStructure.Mode				= USART_MODE_RX | USART_MODE_TX;
	USART_Init(USB_UART, &USART_InitStructure);
	
	USB_UART->DAT;	//Для очистки флага
	//USART_ConfigInt(USB_UART, USART_INT_RXDNE, ENABLE);
	
	NVIC_SetPriority(USART2_IRQn, 3);
	NVIC_EnableIRQ(USART2_IRQn);
//	
	USART_Enable(USB_UART, ENABLE);
}
//----------------------------------------------------------------------------//
char str_tmp[SYS_LOG_BUFF_SIZE];
//----------------------------------------------------------------------------//
void sysLogPut(void){
	u16 cnt = strlen(str_tmp);
	for(u16 i = 0; i < cnt; i++){
		if('\r' == str_tmp[i]);			//Возврат каретки пропускаем
		else if('\n' == str_tmp[i]){	//Вместо новой строки вводим символ табуляции
			USB_FIFO_TX_PUT(0x09);
		}
		else	USB_FIFO_TX_PUT(str_tmp[i]);
	}
	USB_FIFO_TX_PUT('\r');
	USB_FIFO_TX_PUT('\n');
	
	usbData.transmissionComplete = false;
	
	USART_ConfigInt(USB_UART, USART_INT_TXDE, ENABLE);
	USART_ConfigInt(USB_UART, USART_INT_TXC, ENABLE);
};
//----------------------------------------------------------------------------//
static bool USB_Get_Transmission_Complete_Flag(void){	//Получить флаг завершенной передачи
	return usbData.transmissionComplete;
}
//----------------------------------------------------------------------------//
void USART2_IRQHandler(void){
	u32 status = USB_UART->STS;	//Читаем статус USART
	
	if(status & USART_FLAG_TXDE){	//Если буфер передачи пустой
		if(USB_FIFO_TX_CNT()){	//Если есть байты для передачи
			USART_SendData(USB_UART, USB_FIFO_TX_GET());
			USB_FIFO_TX_DROP();
		}
		else if(status & USART_FLAG_TXC){	//Если завершили передачу
			USART_ConfigInt(USB_UART, USART_INT_TXDE, DISABLE);
			USART_ConfigInt(USB_UART, USART_INT_TXC, DISABLE);
			usbData.transmissionComplete = true;
		}
	}
}
//----------------------------------------------------------------------------//
const struct USB_Driver_t USB_Driver = {
	.uartInit						= USB_UART_Init,						//Инициализация UART
	.getTransmissionCompleteFlag	= USB_Get_Transmission_Complete_Flag,	//Получить флаг завершенной передачи
};
//----------------------------------------------------------------------------//
