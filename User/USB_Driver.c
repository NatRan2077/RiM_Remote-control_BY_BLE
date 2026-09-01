//----------------------------------------------------------------------------//
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "cmt453x_dma.h"
#include "cmt453x_usart.h"
#include "USB_Driver.h"
#include "usb.h"
#include "utils.h"
#include "fifo.h"
#include "messages.h"
#include "log.h"
//----------------------------------------------------------------------------//
#define USB_UART_BUFF_TX_SIZE 2048
#define USB_UART_BUFF_RX_SIZE 512
//----------------------------------------------------------------------------//
typedef struct {
	u8 bufTx[USB_UART_BUFF_TX_SIZE]; // Буфер Tx
	u16 cntTx;                       // Длина данных для передачи
	char rxDmaBuff[USB_UART_BUFF_RX_SIZE];

	u8 bufPKT[USB_UART_BUFF_RX_SIZE]; // Буфер принятого пакета
	u16 cntPKT;                       // Длина пакета
	bool fByteRx;                     // Флаг принятого байта
	bool fPktNotRead;                 // Флаг не прочитанного пакета
	bool fTransfer;                   // Флаг для передачи буфера
	bool fTransmitOk;                 // Флаг успешной передачи
	bool fTransmitError;              // Флаг ошибки при передачи
	bool fPendingData;                // Флаг ожидающих обработки данных
	u32 lastTimeOut;                  // Установленный таймаут
} usbData_t;
usbData_t usbData;
//----------------------------------------------------------------------------//
#define USB_FIFO_RX_PUT(byte) FIFO_PUT(usbData.bufRx, byte)
#define USB_FIFO_TX_PUT(byte) FIFO_PUT(usbData.bufTx, byte)
#define USB_FIFO_RX_GET() FIFO_GET(usbData.bufRx)
#define USB_FIFO_TX_GET() FIFO_GET(usbData.bufTx)
#define USB_FIFO_RX_DROP() FIFO_DROP(usbData.bufRx)
#define USB_FIFO_TX_CNT() FIFO_COUNT(usbData.bufTx)
#define USB_FIFO_RX_CNT() FIFO_COUNT(usbData.bufRx);
//----------------------------------------------------------------------------//
#define USB_GPIO GPIOB
#define USB_RX_Pin GPIO_PIN_5
#define USB_TX_Pin GPIO_PIN_4
#define USB_GPIO_AF GPIO_AF3_USART2
//----------------------------------------------------------------------------//
#define USB_UART USART2
#define USB_IRQn USART2_IRQn
//----------------------------------------------------------------------------//
#define USB_USART_CLK RCC_APB1_PERIPH_USART2
#define USB_USART_GPIO_CLK RCC_APB2_PERIPH_GPIOB
#define USB_DMA_CLK RCC_AHB_PERIPH_DMA
//----------------------------------------------------------------------------//
#define USB_DAT_Base (USART2_BASE + 0x04)
#define USB_TX_DMA_Channel DMA_CH3
#define USB_TX_DMA_FLAG_TC DMA_FLAG_TC3
#define USB_TX_DMA_FLAG_TE DMA_FLAG_TE3
#define USB_TX_DMA_REMAP DMA_REMAP_USART2_TX

#define USB_RX_DMA_Channel DMA_CH4
#define USB_RX_DMA_FLAG_TE DMA_FLAG_TE4
#define USB_RX_DMA_REMAP DMA_REMAP_USART2_RX
//----------------------------------------------------------------------------//
static void USB_UART_Init(void) {
	GPIO_InitType GPIO_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	DMA_InitType DMA_InitStructure;
	USART_InitType USART_InitStructure;

	RCC_EnableAHBPeriphClk(USB_DMA_CLK, ENABLE);
	RCC_EnableAPB1PeriphClk(USB_USART_CLK, ENABLE);
	RCC_EnableAPB2PeriphClk(USB_USART_GPIO_CLK | RCC_APB2_PERIPH_AFIO, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);

	GPIO_InitStructure.Pin = USB_TX_Pin | USB_RX_Pin;
	GPIO_InitStructure.GPIO_Alternate = USB_GPIO_AF;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.GPIO_Pull = GPIO_NO_PULL;
	GPIO_InitPeripheral(USB_GPIO, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USB_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_DeInit(USB_TX_DMA_Channel);
	DMA_InitStructure.PeriphAddr = USB_DAT_Base;
	DMA_InitStructure.MemAddr = 1;
	DMA_InitStructure.Direction = DMA_DIR_PERIPH_DST;
	DMA_InitStructure.BufSize = 1;
	DMA_InitStructure.PeriphInc = DMA_PERIPH_INC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
	DMA_InitStructure.MemDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode = DMA_MODE_NORMAL;
	DMA_InitStructure.Priority = DMA_PRIORITY_VERY_HIGH;
	DMA_InitStructure.Mem2Mem = DMA_M2M_DISABLE;
	DMA_Init(USB_TX_DMA_Channel, &DMA_InitStructure);
	DMA_RequestRemap(USB_TX_DMA_REMAP, DMA, USB_TX_DMA_Channel, ENABLE);

	DMA_DeInit(USB_RX_DMA_Channel);
	DMA_InitStructure.PeriphAddr = USB_DAT_Base;
	DMA_InitStructure.MemAddr = 1;
	DMA_InitStructure.Direction = DMA_DIR_PERIPH_SRC;
	DMA_InitStructure.BufSize = 1;
	DMA_InitStructure.PeriphInc = DMA_PERIPH_INC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
	DMA_InitStructure.MemDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.CircularMode = DMA_MODE_NORMAL;
	DMA_InitStructure.Priority = DMA_PRIORITY_VERY_HIGH;
	DMA_InitStructure.Mem2Mem = DMA_M2M_DISABLE;
	DMA_Init(USB_RX_DMA_Channel, &DMA_InitStructure);
	DMA_RequestRemap(USB_RX_DMA_REMAP, DMA, USB_RX_DMA_Channel, ENABLE);

	DMA_SetCurrDataCounter(USB_RX_DMA_Channel, sizeof(usbData.rxDmaBuff));
	USB_RX_DMA_Channel->MADDR = (uint32_t)usbData.rxDmaBuff;

	USART_InitStructure.BaudRate = 460800;
	USART_InitStructure.WordLength = USART_WL_8B;
	USART_InitStructure.StopBits = USART_STPB_1;
	USART_InitStructure.Parity = USART_PE_NO;
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
	USART_InitStructure.Mode = USART_MODE_RX | USART_MODE_TX;
	USART_Init(USB_UART, &USART_InitStructure);

	USB_UART->DAT; // Для очистки флага
	volatile uint32_t temp;
	temp = USB_UART->STS; // Сначала читаем STS
	temp = USB_UART->DAT; // Потом читаем DAT

	USART_ConfigInt(USB_UART, USART_INT_IDLEF, ENABLE);

	NVIC_SetPriority(DMA_Channel1_2_3_4_IRQn, 1);
	NVIC_SetPriority(USART2_IRQn, 2);
	NVIC_EnableIRQ(DMA_Channel1_2_3_4_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);

	DMA_ConfigInt(USB_TX_DMA_Channel, DMA_INT_TXC,
	              ENABLE); // Разрешаем прерывания по завершению передачи
	DMA_ConfigInt(USB_TX_DMA_Channel, DMA_INT_ERR,
	              ENABLE); // Разрешаем прерывания по ошибке при передаче
	DMA_ConfigInt(USB_RX_DMA_Channel, DMA_INT_ERR,
	              ENABLE); // Разрешаем прерывания по ошибке при передаче

	DMA_ClearFlag(USB_TX_DMA_FLAG_TC, DMA);
	DMA_ClearFlag(USB_TX_DMA_FLAG_TE, DMA);
	DMA_ClearFlag(USB_RX_DMA_FLAG_TE, DMA);

	USART_EnableDMA(USB_UART, USART_DMAREQ_TX, ENABLE);
	USART_EnableDMA(USB_UART, USART_DMAREQ_RX, ENABLE);

	USART_Enable(USB_UART, ENABLE);

	DMA_EnableChannel(USB_RX_DMA_Channel, ENABLE);
}
//----------------------------------------------------------------------------//
static void USB_Config_DMA(u32 addr_buff, u16 cnt) {
	DMA_EnableChannel(USB_TX_DMA_Channel, DISABLE);
	usbData.fTransmitOk = false;
	usbData.fTransmitError = false;
	DMA_ClearFlag(USB_TX_DMA_FLAG_TC, DMA);
	DMA_ClearFlag(USB_TX_DMA_FLAG_TE, DMA);
	USB_TX_DMA_Channel->MADDR = addr_buff;
	DMA_SetCurrDataCounter(USB_TX_DMA_Channel, cnt);
	DMA_EnableChannel(USB_TX_DMA_Channel, ENABLE);
}
//----------------------------------------------------------------------------//
static bool USB_Is_Pkt_Not_Read_Flag(void) { // Есть не прочитанный пакет
	return usbData.fPktNotRead;
}
//----------------------------------------------------------------------------//
static void
USB_Clear_Pkt_Not_Read_Flag(void) { // Сбросить флаг не прочитанного пакета
	usbData.fPktNotRead = false;
}
//----------------------------------------------------------------------------//
static void USB_Set_Trasfer_Flag(void) { // Установить флаг для передачи буфера
	usbData.fTransfer = true;
}
//----------------------------------------------------------------------------//
static bool USB_Get_Trasfer_Flag(void) { // Получить флаг передачи буфера
	return usbData.fTransfer;
}
//----------------------------------------------------------------------------//
static void USB_Clear_Trasfer_Flag(void) { // Сбросить флаг передачи буфера
	usbData.fTransfer = false;
}
//----------------------------------------------------------------------------//
static void
USB_Set_Pending_Data_Flag(void) { // Установить флаг ожидающих обработки данных
	usbData.fPendingData = true;
}
//----------------------------------------------------------------------------//
static bool USB_Is_Pending_Data_Flag(void) { // Есть ожидающие обработки данные
	return usbData.fPendingData;
}
//----------------------------------------------------------------------------//
static void
USB_Clear_Pending_Data_Flag(void) { // Сбросить флаг ожидающих обработки данных
	usbData.fPendingData = false;
}
//----------------------------------------------------------------------------//
static void USB_Write_To_BufTX(u8 *buff,
                               u16 len) { // Записать данные в буфер передачи
	memcpy(usbData.bufTx, buff, len);
	usbData.cntTx = len;
}
//----------------------------------------------------------------------------//
static u16 USB_Get_Cnt_Pending_Data(void) { // Получить число байт  данных
	return usbData.cntPKT;
}
//----------------------------------------------------------------------------//
static u8 *
USB_Get_Buf_Pending_Data(void) { // Получить указатель на буфер с данными
	return usbData.bufPKT;
}
//----------------------------------------------------------------------------//
static bool USB_Get_Rx_Byte_Flag(void) { // Получить флаг принятого байта
	return usbData.fByteRx;
}
//----------------------------------------------------------------------------//
static void USB_Clear_Rx_Byte_Flag(void) { // Сбросить флаг принятого байта
	usbData.fByteRx = false;
}
//----------------------------------------------------------------------------//
static bool USB_Get_Transmit_Ok_Flag(void) { // Получить флаг успешной передачи
	return usbData.fTransmitOk;
}
//----------------------------------------------------------------------------//
static void USB_Clear_Transmit_Ok_Flag(void) { // Сбросить флаг успешной
	                                            // передачи
	usbData.fTransmitOk = false;
}
//----------------------------------------------------------------------------//
static bool
USB_Get_Transmit_Error_Flag(void) { // Получить флаг ошибки при передаче
	return usbData.fTransmitError;
}
//----------------------------------------------------------------------------//
static void
USB_Clear_Transmit_Error_Flag(void) { // Сбросить флаг ошибки при передаче
	usbData.fTransmitError = false;
}
//----------------------------------------------------------------------------//
static void USB_Start_Sending_Data(void) { // Начать отправку данных
	DMA_EnableChannel(USB_TX_DMA_Channel, DISABLE);
	USB_TX_DMA_Channel->MADDR = (u32)&usbData.bufTx;
	USB_TX_DMA_Channel->TXNUM = usbData.cntTx;
	DMA_EnableChannel(USB_TX_DMA_Channel, ENABLE);
	usbData.cntTx = 0;
}
//----------------------------------------------------------------------------//
static void USB_IRQ_Handler(void) { // Обработчик прерываний
	u32 status = USB_UART->STS;      // Читаем статус USART

	if (status & USART_FLAG_OREF) { // Ошибка переполнения - сбрасываем и
		                             // игнорируем данные
		volatile u8 t8;
		USB_UART->DAT = t8;
		;
		t8 = USB_UART->DAT;

		return;
	}
	if (status & (USART_FLAG_NEF |
	              USART_FLAG_FEF)) { // Ошибка шума/фрейма - читаем и отбрасываем
		(void)USB_UART->DAT;
		return;
	}
	if (status & USART_FLAG_IDLEF) {
		volatile uint32_t temp;
		temp = USB_UART->STS; // Сначала читаем STS
		temp = USB_UART->DAT; // Потом читаем DAT
		//(void)temp;
		DMA_EnableChannel(USB_RX_DMA_Channel, DISABLE);
		if (!usbData.fPktNotRead) {
			u16 len =
			    USB_UART_BUFF_RX_SIZE - DMA_GetCurrDataCounter(USB_RX_DMA_Channel);
			memcpy(usbData.bufPKT, usbData.rxDmaBuff, len);
			usbData.cntPKT = len;
			usbData.fPktNotRead = true;
		}
		memset(usbData.rxDmaBuff, 0x00, sizeof(usbData.rxDmaBuff));
		DMA_SetCurrDataCounter(USB_RX_DMA_Channel, sizeof(usbData.rxDmaBuff));
		USB_RX_DMA_Channel->MADDR = (uint32_t)usbData.rxDmaBuff;
		DMA_ClearFlag(USB_RX_DMA_FLAG_TE, DMA);
		DMA_EnableChannel(USB_RX_DMA_Channel, ENABLE);
	}
}
//----------------------------------------------------------------------------//
u32 tttt;
static void USB_DMA_IRQ_Handler(void) { // Обработчик прерываний DMA
	tttt++;
	if (DMA_GetFlagStatus(USB_TX_DMA_FLAG_TC, DMA)) {
		DMA_ClearFlag(USB_TX_DMA_FLAG_TC, DMA);
		usbData.fTransmitOk = true;
	} else if (DMA_GetFlagStatus(USB_TX_DMA_FLAG_TE, DMA)) { // если ошибка
		DMA_ClearFlag(USB_TX_DMA_FLAG_TE, DMA);
		usbData.fTransmitError = true;
		DMA_EnableChannel(USB_TX_DMA_Channel, DISABLE); // Отключаем канал
	} else if (DMA_GetFlagStatus(USB_RX_DMA_FLAG_TE, DMA)) {
		DMA_EnableChannel(USB_RX_DMA_Channel, DISABLE);
		memset(usbData.rxDmaBuff, 0x00, sizeof(usbData.rxDmaBuff));
		DMA_SetCurrDataCounter(USB_RX_DMA_Channel, sizeof(usbData.rxDmaBuff));
		USB_RX_DMA_Channel->MADDR = (uint32_t)usbData.rxDmaBuff;
		DMA_ClearFlag(USB_RX_DMA_FLAG_TE, DMA);
		DMA_EnableChannel(USB_RX_DMA_Channel, ENABLE);
	}
}
//----------------------------------------------------------------------------//
const struct USB_Driver_t USB_Driver = {
    .uartInit = USB_UART_Init,   // Инициализация UART
    .configDMA = USB_Config_DMA, // Настраиваем DMA для передачи
    .isPktNotReadFlag = USB_Is_Pkt_Not_Read_Flag, // Есть не прочитанный пакет
    .clearPktNotReadFlag =
        USB_Clear_Pkt_Not_Read_Flag, // Сбросить флаг не прочитанного пакета
    .getTrasferFlag = USB_Get_Trasfer_Flag,     // Получить флаг передачи буфера
    .clearTrasferFlag = USB_Clear_Trasfer_Flag, // Сбросить флаг передачи буфера
    .setPendingDataFlag =
        USB_Set_Pending_Data_Flag, // Установить флаг ожидающих обработки данных
    .isPendingDataFlag =
        USB_Is_Pending_Data_Flag,          // Есть ожидающие обработки данные
    .getRxByteFlag = USB_Get_Rx_Byte_Flag, // Получить флаг принятого байта
    .clearRxByteFlag = USB_Clear_Rx_Byte_Flag, // Сбросить флаг принятого байта
    .getTransmitOkFlag =
        USB_Get_Transmit_Ok_Flag, // Получить флаг успешной передачи
    .clearTransmitOkFlag =
        USB_Clear_Transmit_Ok_Flag, // Сбросить флаг успешной передачи
    .getTransmitErrorFlag =
        USB_Get_Transmit_Error_Flag, // Получить флаг ошибки при передаче
    .clearTransmitErrorFlag =
        USB_Clear_Transmit_Error_Flag, // Сбросить флаг ошибки при передаче
    .startSendingData = USB_Start_Sending_Data, // Начать отправку данных
};
//----------------------------------------------------------------------------//
const struct usb_t usb = {
    .setTransferFlag =
        USB_Set_Trasfer_Flag, // Установить флаг для передачи буфера
    .isPendingDataFlag =
        USB_Is_Pending_Data_Flag, // Есть ожидающие обработки данные
    .clearPendingDataFlag =
        USB_Clear_Pending_Data_Flag, // Сбросить флаг ожидающих обработки данных
    .writeToBufTx = USB_Write_To_BufTX, // Записать данные в буфер передачи
    .getCntPendingData = USB_Get_Cnt_Pending_Data, // Получить число байт данных
    .getBufPendingData =
        USB_Get_Buf_Pending_Data,  // Получить указатель на буфер с данными
    .irqHandler = USB_IRQ_Handler, // Обработчик прерываний
    .DMAirqHandler = USB_DMA_IRQ_Handler, // Обработчик прерываний DMA
};
//----------------------------------------------------------------------------//
