//----------------------------------------------------------------------------//
#include "memory.h"
#include "utils.h"
#include <string.h>
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "cmt453x_spi.h"
#include "CRC.h"
//----------------------------------------------------------------------------//
#define MEMORY_SPI			SPI1
#define MEMORY_GPIO_Port 	GPIOA
#define MEMORY_CS_Pin 		GPIO_PIN_0
#define MEMORY_SCK_Pin 		GPIO_PIN_1
#define MEMORY_MISO_Pin 	GPIO_PIN_3
#define MEMORY_MOSI_Pin 	GPIO_PIN_2
#define MEMORY_GPIO_AF		GPIO_AF1_SPI1
//----------------------------------------------------------------------------//
#define MEMORY_SPI_CLK		RCC_APB2_PERIPH_SPI1
#define MEMORY_SPI_GPIO_CLK	RCC_APB2_PERIPH_GPIOA
//----------------------------------------------------------------------------//
#define MEMORY_SELECT()		GPIO_ResetBits(MEMORY_GPIO_Port, MEMORY_CS_Pin)
#define MEMORY_DESELECT()	GPIO_SetBits(MEMORY_GPIO_Port, MEMORY_CS_Pin)
//----------------------------------------------------------------------------//
#define MEMORY_WREN			0x06	//Команда разрешения записи
#define MEMORY_WRDI			0x04	//Команда запрета записи
#define MEMORY_PAGE_PROGRAM	0x02	//Команда записи страницы
#define MEMORY_RDSR			0x05	//Команда чтения статусного регистра
#define MEMORY_READ			0x03	//Команда чтения байт
#define MEMORY_BE			0xD8	//Команда стирания блока 64К
#define MEMORY_CE			0x60	//Команда стирания чипа
#define MEMORY_PE			0x81	//Команда стирания страницы
#define MEMORY_RDID			0x9F	//Команда чтения ID
#define MEMORY_WIP_bit		0x01	//Write in Progress
//----------------------------------------------------------------------------//
#define	P25Q16SH_MANUFACTURE_ID	0x85
#define	P25Q16SH_MEMORY_TYPE	0x60
#define	P25Q16SH_MEMORY_DENSITY	0x15
//----------------------------------------------------------------------------//
#define MEMORY_PAGE_SIZE	memoryData.pageSize
//----------------------------------------------------------------------------//
typedef enum{	//Название памяти
	MEMORY_NAME_UNKNOWN,	//Неизвестная память
	MEMORY_NAME_P25Q16SH,	//P25Q16SH
}memoryName_t;
//----------------------------------------------------------------------------//
typedef struct{	//Структура памяти Flash
	u8 manufactorID;
	u8 type;
	u8 density;
	u16 pageSize;
	memoryName_t name;
}memoryData_t;
//----------------------------------------------------------------------------//
memoryData_t memoryData;			//Память Flash
//----------------------------------------------------------------------------//
static u8 MEMORY_Put(u8 byte){	//Отправить/Принять 1 байт
	*((uint8_t*) & (MEMORY_SPI->DAT)) = byte;
	while(SPI_I2S_RNE_FLAG != (MEMORY_SPI->STS & SPI_I2S_RNE_FLAG));
	return *((uint8_t*) & (MEMORY_SPI->DAT));
}
//----------------------------------------------------------------------------//
//static void MEMORY_Chip_Erase(void);
//----------------------------------------------------------------------------//
static bool MEMORY_Init(void){	//Инициализация памяти
	SPI_InitType SPI_InitStructure;
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(MEMORY_SPI_GPIO_CLK | RCC_APB2_PERIPH_AFIO, ENABLE);
	RCC_EnableAPB2PeriphClk(MEMORY_SPI_CLK, ENABLE);
	
	GPIO_InitStruct(&GPIO_InitStructure);
	
	GPIO_InitStructure.Pin			= MEMORY_CS_Pin;
	GPIO_InitStructure.GPIO_Speed	= GPIO_SPEED_HIGH;
	GPIO_InitStructure.GPIO_Mode	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.GPIO_Pull	= GPIO_NO_PULL;
	GPIO_InitPeripheral(MEMORY_GPIO_Port, &GPIO_InitStructure);
	MEMORY_DESELECT();
	
	GPIO_InitStructure.Pin				= MEMORY_SCK_Pin | MEMORY_MISO_Pin | MEMORY_MOSI_Pin;
	GPIO_InitStructure.GPIO_Mode		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.GPIO_Alternate	= MEMORY_GPIO_AF;
	GPIO_InitPeripheral(MEMORY_GPIO_Port, &GPIO_InitStructure);

	SPI_I2S_DeInit(MEMORY_SPI);
	SPI_InitStructure.DataDirection	= SPI_DIR_DOUBLELINE_FULLDUPLEX;
	SPI_InitStructure.SpiMode		= SPI_MODE_MASTER;
	SPI_InitStructure.DataLen		= SPI_DATA_SIZE_8BITS;
	SPI_InitStructure.CLKPOL		= SPI_CLKPOL_LOW;
	SPI_InitStructure.CLKPHA		= SPI_CLKPHA_FIRST_EDGE;
	SPI_InitStructure.NSS			= SPI_NSS_SOFT;
	SPI_InitStructure.BaudRatePres	= SPI_BR_PRESCALER_4;
	SPI_InitStructure.FirstBit		= SPI_FB_MSB;
	SPI_InitStructure.CRCPoly		= 7;
	SPI_Init(MEMORY_SPI, &SPI_InitStructure);
	
	
//?	SPI_DisableFrameFormatMode(MEMORY_SPI);
	
	SPI_SSOutputEnable(MEMORY_SPI, ENABLE);
	
//	SPI_ConfigFIFOThreshold(MEMORY_SPI, SPI_RXFIFO_QUARTER);
	SPI_Enable(MEMORY_SPI, ENABLE);
	
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_RDID);
	memoryData.manufactorID = MEMORY_Put(0);
	memoryData.type = MEMORY_Put(0);
	memoryData.density = MEMORY_Put(0);
	MEMORY_DESELECT();
	
	if((P25Q16SH_MANUFACTURE_ID == memoryData.manufactorID) && (P25Q16SH_MEMORY_TYPE == memoryData.type) && (P25Q16SH_MEMORY_DENSITY == memoryData.density)){
		memoryData.name = MEMORY_NAME_P25Q16SH;
		memoryData.pageSize = 256;
		return true;
	}
	else{
		memoryData.name = MEMORY_NAME_UNKNOWN;
		memoryData.pageSize = 256;	//Размер страницы по умолчанию
		return false;
	}
	//MEMORY_Chip_Erase();
}
//----------------------------------------------------------------------------//
static void MEMORY_Write_Enable(){	//Разрешить запись в память
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_WREN);
	MEMORY_DESELECT();
}
//----------------------------------------------------------------------------//
static void MEMORY_Write_Disable(){	//Разрешить запись в память
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_WRDI);
	MEMORY_DESELECT();
}
//----------------------------------------------------------------------------//
static bool MEMORY_Is_Busy(void){	//Проверка занятости памяти (идет процесс записи/стирания)
	u8 status = 0;
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_RDSR);
	status = MEMORY_Put(0);
	MEMORY_DESELECT();
	return (MEMORY_WIP_bit == (status & MEMORY_WIP_bit));
}
//----------------------------------------------------------------------------//
static void MEMORY_Read_Bytes(const u32 address, u8* const buffer_ptr, const u16 len){
	while(MEMORY_Is_Busy());
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_READ);
	MEMORY_Put(BYTE_2(address));
	MEMORY_Put(BYTE_1(address));
	MEMORY_Put(BYTE_0(address));
	for(u16 i = 0; i < len; i++){
		buffer_ptr[i] = MEMORY_Put(0);
	}
	MEMORY_DESELECT();
}
//----------------------------------------------------------------------------//
//Запись len байт в память, 24 бита адрес, указатель на буфер с данными, длина(не более 256)
static void MEMORY_Write_Bytes(const u32 address, u8* const buffer_ptr, const u8 len){
	while(MEMORY_Is_Busy());
	MEMORY_Write_Enable();
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_PAGE_PROGRAM);
	MEMORY_Put(BYTE_2(address));
	MEMORY_Put(BYTE_1(address));
	MEMORY_Put(BYTE_0(address));
	for(u8 i = 0; i < len; i++){
		MEMORY_Put(buffer_ptr[i]);
	}
	MEMORY_DESELECT();
	MEMORY_Write_Disable();
}
//----------------------------------------------------------------------------//
static void MEMORY_Write_With_CRC(const u32 address, u8* const buffer_ptr, const u8 len){
	u8 buff[256];
	memcpy(buff, buffer_ptr, len);
	u16 crc = CRC16(buffer_ptr, len);
	buff[len] = BYTE_1(crc);
	buff[len + 1] = BYTE_0(crc);
	MEMORY_Write_Bytes(address, buff, (len + 2));
}
//----------------------------------------------------------------------------//
static void MEMORY_Page_Erase(u32 address){
	while(MEMORY_Is_Busy());
	MEMORY_Write_Enable();
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_PE);
	MEMORY_Put(BYTE_2(address));
	MEMORY_Put(BYTE_1(address));
	MEMORY_Put(BYTE_0(address));
	MEMORY_DESELECT();
	MEMORY_Write_Disable();
}
//----------------------------------------------------------------------------//
static void MEMORY_Block_Erase(u32 address){
	while(MEMORY_Is_Busy());
	MEMORY_Write_Enable();
	MEMORY_SELECT();
	MEMORY_Put(MEMORY_BE);
	MEMORY_Put(BYTE_2(address));
	MEMORY_Put(BYTE_1(address));
	MEMORY_Put(BYTE_0(address));
	MEMORY_DESELECT();
	MEMORY_Write_Disable();
}
//----------------------------------------------------------------------------//
//static void MEMORY_Chip_Erase(void){
//	while(MEMORY_Is_Busy());
//	MEMORY_Write_Enable();
//	MEMORY_SELECT();
//	MEMORY_Put(MEMORY_CE);
//	MEMORY_DESELECT();
//	MEMORY_Write_Disable();
//}
//----------------------------------------------------------------------------//
static result_t MEMORY_Read_With_CRC(u32 address, u8* buffer_ptr, const u8 len){
	u8 buff[256];
	MEMORY_Read_Bytes(address, buff, len + 2);
	u16 crc = CRC16(buff, len);
	if(crc == WORD(buff[len], buff[len + 1])){
		memcpy(buffer_ptr, buff, len);
		return RESULT_SUCCESS;
	}
	else{
		return RESULT_ERROR;
	}
}
//----------------------------------------------------------------------------//
//Запись двух копий по двум адресам с расчетом контрольной суммы, адрес - условный адрес(номер страницы), по которому рассчитывается реальный адрес размещения данных
static result_t MEMORY_Write_Two_Copyes_With_CRC(u32 page, void* const buffer_ptr, u8 len){
	u8 tmpBuff[256];
	
	MEMORY_Page_Erase(page*MEMORY_PAGE_SIZE);
	MEMORY_Page_Erase((page + 1)*MEMORY_PAGE_SIZE);
	MEMORY_Write_With_CRC(page*MEMORY_PAGE_SIZE, buffer_ptr, len);
	MEMORY_Write_With_CRC((page + 1)*MEMORY_PAGE_SIZE, buffer_ptr, len);
	
	if(RESULT_ERROR == MEMORY_Read_With_CRC(page*MEMORY_PAGE_SIZE, tmpBuff, len))	return RESULT_ERROR;
	if(0 != memcmp(buffer_ptr, tmpBuff, len))	return RESULT_ERROR;
	if(RESULT_ERROR == MEMORY_Read_With_CRC((page + 1)*MEMORY_PAGE_SIZE, tmpBuff, len))	return RESULT_ERROR;
	if(0 != memcmp(buffer_ptr, tmpBuff, len))	return RESULT_ERROR;
	return RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
static result_t MEMORY_Read_Two_Copyes_With_CRC(u32 page, void* data_ptr, u8 len){
	u8 data_r[2][256];
	result_t iFlagERROR[2] = {RESULT_ERROR, RESULT_ERROR};
	iFlagERROR[0] = MEMORY_Read_With_CRC(page*MEMORY_PAGE_SIZE, (u8*)&data_r[0], len);					// ** Читаем данные и проверяем их CRC
	iFlagERROR[1] = MEMORY_Read_With_CRC(((page + 1)*MEMORY_PAGE_SIZE), (u8*)&data_r[1], len);	// ** Если ошибки - выставляем флаги
	
	bool both_ok = ((RESULT_SUCCESS == iFlagERROR[0]) && (RESULT_SUCCESS == iFlagERROR[1]));
	bool first_ok = ((RESULT_SUCCESS == iFlagERROR[0]) && (RESULT_ERROR == iFlagERROR[1]));
	bool second_ok = ((RESULT_ERROR == iFlagERROR[0]) && (RESULT_SUCCESS == iFlagERROR[1]));

	if(both_ok){	//Обе копии нормальные
		memcpy(data_ptr,  data_r[0], len);
		return RESULT_SUCCESS;
	}
	else if (first_ok){	//Вторая копия ошибочна, перепишем первую корректную в оба места
		MEMORY_Page_Erase((page + 1)*MEMORY_PAGE_SIZE);
		MEMORY_Write_With_CRC(((page + 1)*MEMORY_PAGE_SIZE), (u8*)&data_r[0], len);
		memcpy(data_ptr,  data_r[0], len);
		return RESULT_SUCCESS;
	}
	else if (second_ok){	//Первая копия ошибочна, перепишем вторую корректную в оба места
		MEMORY_Page_Erase(page*MEMORY_PAGE_SIZE);
		MEMORY_Write_With_CRC(page*MEMORY_PAGE_SIZE, (u8*)&data_r[1], len);
		memcpy(data_ptr,  data_r[1], len);
		return RESULT_SUCCESS;
	}
	return RESULT_ERROR;	//Все неверно
}				
//----------------------------------------------------------------------------//
static void MEMORY_EraseTwo_Copyes(u32 page){
	MEMORY_Page_Erase(page*MEMORY_PAGE_SIZE);
	MEMORY_Page_Erase((page + 1)*MEMORY_PAGE_SIZE);
}
//----------------------------------------------------------------------------//
static u16 MEMORY_Get_Page_Size(void){
	return memoryData.pageSize;
}
static void MEMORY_Erase_Block(u32 page){	//Передаем номер страницы
	MEMORY_Block_Erase(page*MEMORY_PAGE_SIZE);
}
//----------------------------------------------------------------------------//
const struct MEMORY_driver memory = {
	.init					= MEMORY_Init,
	.writeTwoCopyesWithCRC	= MEMORY_Write_Two_Copyes_With_CRC,
	.readTwoCopyesWithCRC	= MEMORY_Read_Two_Copyes_With_CRC,
	.readBytes				= MEMORY_Read_Bytes,
	.writeBytes				= MEMORY_Write_Bytes,
	.getPageSize			= MEMORY_Get_Page_Size,
	.eraseTwoCopyes			= MEMORY_EraseTwo_Copyes,
	.eraseBlock				= MEMORY_Erase_Block,
};
//----------------------------------------------------------------------------//
