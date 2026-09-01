//----------------------------------------------------------------------------//
#include <string.h>
#include <math.h>
#include "stdarg.h"
#include "lcd.h"
#include "lcd_def.h"
//----------------------------------------------------------------------------//
#define LCD_I2C				I2C1
#define LCD_I2C_CLK			RCC_APB1_PERIPH_I2C1
#define LCD_I2C_GPIO		GPIOB
#define LCD_I2C_GPIO_AF		GPIO_AF3_I2C
#define LCD_I2C_GPIO_CLK	RCC_APB2_PERIPH_GPIOB
#define LCD_I2C_SDA_PIN		GPIO_PIN_6
#define LCD_I2C_SCL_PIN		GPIO_PIN_7
#define I2C_SLAVE_ADDR		0x7C
#define I2C_MASTER_ADDR		0x7C
//----------------------------------------------------------------------------//
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(20 * I2CT_FLAG_TIMEOUT))  ///((uint32_t)(20 * I2C_FLAG_TIMOUT))
static __IO uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;
//----------------------------------------------------------------------------//
#define LCD_BUFF_SIZE	35
//----------------------------------------------------------------------------//
u8 lcdBuff_Current[LCD_BUFF_SIZE + 1];	//первый байт адрес, всегда равен 0, т.к. передается сразу весь буфер
u8 lcdBuff_New[LCD_BUFF_SIZE];
//----------------------------------------------------------------------------//
void LCD_Bit(u16 index, bool set){
	if(index > (LCD_BUFF_SIZE * 8))	return;
	u8 byteIndex = index / 8;
	u8 bitPos = index%8;
	if(set)	lcdBuff_New[byteIndex] |= (1 << bitPos);
	else	lcdBuff_New[byteIndex] &= ~(1 << bitPos);
}
//----------------------------------------------------------------------------//
static u8 LCD_I2C_Send(u8* data, u8 len){
	u8* sendBufferPtr = data;
	I2CTimeout = I2CT_LONG_TIMEOUT;
	while(I2C_GetFlag(LCD_I2C, I2C_FLAG_BUSY)){
		if((I2CTimeout--) == 0)	return 4;
	};
	
	I2C_ConfigAck(LCD_I2C, ENABLE);
	I2C_GenerateStart(LCD_I2C, ENABLE);        
	I2CTimeout = I2CT_LONG_TIMEOUT;
	while(!I2C_CheckEvent(LCD_I2C, I2C_EVT_MASTER_MODE_FLAG)){ // EV5
		if((I2CTimeout--) == 0)	return 5;
	};

	I2C_SendAddr7bit(LCD_I2C, I2C_SLAVE_ADDR, I2C_DIRECTION_SEND);
	I2CTimeout = I2CT_LONG_TIMEOUT;
	while(!I2C_CheckEvent(LCD_I2C, I2C_EVT_MASTER_TXMODE_FLAG)){ // EV6
		if((I2CTimeout--) == 0)	return 6;
	};

	while(len-- > 0){	//send data
		I2C_SendData(LCD_I2C, *sendBufferPtr++);
		I2CTimeout = I2CT_LONG_TIMEOUT;
		while(!I2C_CheckEvent(LCD_I2C, I2C_EVT_MASTER_DATA_SENDING)){ // EV8
			if((I2CTimeout--) == 0)	return 7;
		};
	};

	I2CTimeout = I2C_FLAG_TIMOUT;
	while(!I2C_CheckEvent(LCD_I2C, I2C_EVT_MASTER_DATA_SENDED)){ // EV8-2
		if((I2CTimeout--) == 0)	return 8;
	};
	I2C_GenerateStop(LCD_I2C, ENABLE);
	return 0;
}
//----------------------------------------------------------------------------//
const u8 lcdInitBuff_1[] = {0xF2, 0xE2, 0xC0, 0x00, 0xFF};
const u8 lcdInitBuff_2[] = {0xF1};
//----------------------------------------------------------------------------//
static u8 LCD_Init(void){
	u8 error = 0;
	I2C_InitType	I2C_InitStruct;
	GPIO_InitType	GPIO_InitStruct;
    
	RCC_EnableAPB1PeriphClk(LCD_I2C_CLK, ENABLE);
	RCC_EnableAPB2PeriphClk(LCD_I2C_GPIO_CLK | RCC_APB2_PERIPH_AFIO, ENABLE);
	
    GPIO_InitStruct.Pin				= LCD_I2C_SDA_PIN | LCD_I2C_SCL_PIN;
    GPIO_InitStruct.GPIO_Speed		= GPIO_SPEED_HIGH; 
    GPIO_InitStruct.GPIO_Mode		= GPIO_MODE_AF_OD;            
    GPIO_InitStruct.GPIO_Alternate	= LCD_I2C_GPIO_AF;
    GPIO_InitStruct.GPIO_Pull		= GPIO_NO_PULL;
	GPIO_InitStruct.GPIO_Current	= GPIO_DC_LOW;
    GPIO_InitPeripheral(LCD_I2C_GPIO, &GPIO_InitStruct);
	
	I2C_DeInit(LCD_I2C);
	I2C_InitStruct.BusMode		= I2C_BUSMODE_I2C;
	I2C_InitStruct.FmDutyCycle	= I2C_FMDUTYCYCLE_1;
	I2C_InitStruct.OwnAddr1		= I2C_MASTER_ADDR;
	I2C_InitStruct.AckEnable	= I2C_ACKEN;
	I2C_InitStruct.AddrMode		= I2C_ADDR_MODE_7BIT;
	I2C_InitStruct.ClkSpeed		= 100000;
	I2C_Init(LCD_I2C, &I2C_InitStruct);
	
	I2C_Enable(LCD_I2C, ENABLE);

	error = LCD_I2C_Send((u8*)lcdInitBuff_1, sizeof(lcdInitBuff_1));
	error = LCD_I2C_Send((u8*)lcdInitBuff_2, sizeof(lcdInitBuff_2));
	
	error = LCD_I2C_Send(lcdBuff_Current, (LCD_BUFF_SIZE + 1));
	
	return error;
}
//----------------------------------------------------------------------------//
u32 upd;
static u8 LCD_Update(void){
	u8 error = 0;
	if(0 != memcmp(lcdBuff_Current + 1, lcdBuff_New, LCD_BUFF_SIZE)){	//Если буфер отлдичается, то обновляем данные на дисплее
		memcpy(lcdBuff_Current + 1, lcdBuff_New, LCD_BUFF_SIZE);
		error = LCD_I2C_Send(lcdBuff_Current, (LCD_BUFF_SIZE + 1));
		upd++;
	}
	return error;
}
//----------------------------------------------------------------------------//
static void LCD_All(void){
	memset(lcdBuff_New, 0xFF, LCD_BUFF_SIZE);
}
//----------------------------------------------------------------------------//
static void LCD_Clear(void){
	memset(lcdBuff_New, 0x00, LCD_BUFF_SIZE);
}
//----------------------------------------------------------------------------//
typedef struct{	//Для цифр (позиции 7-23) - стандартные 7 сегментов
    LCD_Segment A, B, C, D, E, F, G;  //Только основные сегменты
}LCD_Digit;
//----------------------------------------------------------------------------//
typedef struct{	// Для символов (позиции 1-6) - расширенный набор
    LCD_Segment A1, A2, B, C, D1, D2, E, F, G1, G2;
    LCD_Segment H, I, J, K, L, M; // Доп. элементы для символов
} LCD_Character;
//----------------------------------------------------------------------------//
const LCD_Character LCD_CharacterPositions[6] = {
	{.A1 = LCD_1A1,	.A2 = LCD_1A2, .B = LCD_1B, .C = LCD_1C, .D1 = LCD_1D1, .D2 = LCD_1D2, .E = LCD_1E, .F = LCD_1F,.G1 = LCD_1G1, .G2 = LCD_1G2, .H = LCD_1H, .I = LCD_1I, .J = LCD_1J, .K = LCD_1K, .L = LCD_1L, .M = LCD_1M},	//Позиция 1
	{.A1 = LCD_2A1, .A2 = LCD_2A2, .B = LCD_2B, .C = LCD_2C, .D1 = LCD_2D1, .D2 = LCD_2D2, .E = LCD_2E, .F = LCD_2F,.G1 = LCD_2G1, .G2 = LCD_2G2, .H = LCD_2H, .I = LCD_2I, .J = LCD_2J, .K = LCD_2K, .L = LCD_2L, .M = LCD_2M},	//Позиция 2
	{.A1 = LCD_3A1, .A2 = LCD_3A2, .B = LCD_3B, .C = LCD_3C, .D1 = LCD_3D1, .D2 = LCD_3D2, .E = LCD_3E, .F = LCD_3F,.G1 = LCD_3G1, .G2 = LCD_3G2, .H = LCD_3H, .I = LCD_3I, .J = LCD_3J, .K = LCD_3K, .L = LCD_3L, .M = LCD_3M},	//Позиция 3
	{.A1 = LCD_4A1, .A2 = LCD_4A2, .B = LCD_4B, .C = LCD_4C, .D1 = LCD_4D1, .D2 = LCD_4D2, .E = LCD_4E, .F = LCD_4F,.G1 = LCD_4G1, .G2 = LCD_4G2, .H = LCD_4H, .I = LCD_4I, .J = LCD_4J, .K = LCD_4K, .L = LCD_4L, .M = LCD_4M},	//Позиция 4
	{.A1 = LCD_5A1, .A2 = LCD_5A2, .B = LCD_5B, .C = LCD_5C, .D1 = LCD_5D1, .D2 = LCD_5D2, .E = LCD_5E, .F = LCD_5F,.G1 = LCD_5G1, .G2 = LCD_5G2, .H = LCD_5H, .I = LCD_5I, .J = LCD_5J, .K = LCD_5K, .L = LCD_5L, .M = LCD_5M},	//Позиция 5
	{.A1 = LCD_6A1, .A2 = LCD_6A2, .B = LCD_6B, .C = LCD_6C, .D1 = LCD_6D1, .D2 = LCD_6D2, .E = LCD_6E, .F = LCD_6F,.G1 = LCD_6G1, .G2 = LCD_6G2, .H = LCD_6H, .I = LCD_6I, .J = LCD_6J, .K = LCD_6K, .L = LCD_6L, .M = LCD_6M},	//Позиция 6
};
//----------------------------------------------------------------------------//
const LCD_Digit LCD_DigitPositions[23 - 6 - 1] = {
	{.A = LCD_7A,	.B = LCD_7B,	.C = LCD_7C,	.D = LCD_7D,	.E = LCD_7E,	.F = LCD_7F,	.G = LCD_7G},	//Позиция 7
	{.A = LCD_8A,	.B = LCD_8B,	.C = LCD_8C,	.D = LCD_8D,	.E = LCD_8E,	.F = LCD_8F,	.G = LCD_8G},	//Позиция 8
	{.A = LCD_9A,	.B = LCD_9B,	.C = LCD_9C,	.D = LCD_9D,	.E = LCD_9E,	.F = LCD_9F,	.G = LCD_9G},	//Позиция 9
	{.A = LCD_10A,	.B = LCD_10B,	.C = LCD_10C,	.D = LCD_10D,	.E = LCD_10E,	.F = LCD_10F,	.G = LCD_10G},	//Позиция 10
	{.A = LCD_11A,	.B = LCD_11B,	.C = LCD_11C,	.D = LCD_11D,	.E = LCD_11E,	.F = LCD_11F,	.G = LCD_11G},	//Позиция 11
	{.A = LCD_12A,	.B = LCD_12B,	.C = LCD_12C,	.D = LCD_12D,	.E = LCD_12E,	.F = LCD_12F,	.G = LCD_12G},	//Позиция 12
	{.A = LCD_13A,	.B = LCD_13B,	.C = LCD_13C,	.D = LCD_13D,	.E = LCD_13E,	.F = LCD_13F,	.G = LCD_13G},	//Позиция 13
	{.A = LCD_14A,	.B = LCD_14B,	.C = LCD_14C,	.D = LCD_14D,	.E = LCD_14E,	.F = LCD_14F,	.G = LCD_14G},	//Позиция 14
	{.A = LCD_15A,	.B = LCD_15B,	.C = LCD_15C,	.D = LCD_15D,	.E = LCD_15E,	.F = LCD_15F,	.G = LCD_15G},	//Позиция 15
	{.A = LCD_17A,	.B = LCD_17B,	.C = LCD_17C,	.D = LCD_17D,	.E = LCD_17E,	.F = LCD_17F,	.G = LCD_17G},	//Позиция 17
	{.A = LCD_18A,	.B = LCD_18B,	.C = LCD_18C,	.D = LCD_18D,	.E = LCD_18E,	.F = LCD_18F,	.G = LCD_18G},	//Позиция 18
	{.A = LCD_19A,	.B = LCD_19B,	.C = LCD_19C,	.D = LCD_19D,	.E = LCD_19E,	.F = LCD_19F,	.G = LCD_19G},	//Позиция 19
	{.A = LCD_20A,	.B = LCD_20B,	.C = LCD_20C,	.D = LCD_20D,	.E = LCD_20E,	.F = LCD_20F,	.G = LCD_20G},	//Позиция 20
	{.A = LCD_21A,	.B = LCD_21B,	.C = LCD_21C,	.D = LCD_21D,	.E = LCD_21E,	.F = LCD_21F,	.G = LCD_21G},	//Позиция 21
	{.A = LCD_22A,	.B = LCD_22B,	.C = LCD_22C,	.D = LCD_22D,	.E = LCD_22E,	.F = LCD_22F,	.G = LCD_22G},	//Позиция 22
	{.A = LCD_23A,	.B = LCD_23B,	.C = LCD_23C,	.D = LCD_23D,	.E = LCD_23E,	.F = LCD_23F,	.G = LCD_23G},	//Позиция 23
};
//----------------------------------------------------------------------------//
const uint8_t DigitPatterns[] = {
	// GFEDCBA (стандартный порядок битов)
	0x3F,	// 0 (A,B,C,D,E,F)		00111111
	0x06,	// 1 (B,C)				00000110
	0x5B,	// 2 (A,B,G,E,D)		01011011
	0x4F,	// 3 (A,B,G,C,D)		01001111
	0x66,	// 4 (F,G,B,C)			01100110
	0x6D,	// 5 (A,F,G,C,D)		01101101
	0x7D,	// 6 (A,F,G,C,D,E)		01111101
	0x07,	// 7 (A,B,C)			00000111
	0x7F,	// 8 (A,B,C,D,E,F,G)	01111111
	0x6F,	// 9 (A,B,C,D,F,G)		01101111
	0x40,	// LINE (только G)		01000000
	0x63,	// GRADUS				01100011
	0x39,	// CELSIUS				00111001
	0x79,	// ERASE				01111001
	0x73,	// PROGRAMM				01110011
};
//----------------------------------------------------------------------------//
void LCD_Show_Val_Pos(u8 value, u8 pos){
	if((pos < 7) || (pos > 23) || (pos == 16)) return;
	if(pos < 16)	pos -= 6; //смещение
	else			pos -= 7; //смещение
	
	u8 pattern;
	// Получаем паттерн с учетом всех возможных значений
	if(value <= 9)				pattern = DigitPatterns[value];
	else if(value == LINE)		pattern = DigitPatterns[10];
	else if(value == GRADUS)	pattern = DigitPatterns[11];
	else if(value == CELSIUS)	pattern = DigitPatterns[12];
	else if(value == ERASE)		pattern = DigitPatterns[13];
	else if(value == PROGRAMM)	pattern = DigitPatterns[14];
	else						pattern = 0; // Пустой паттерн для неизвестных значений

	const LCD_Digit *seg = &LCD_DigitPositions[pos - 1];	// Получаем сегменты для позиции

	// Устанавливаем состояние сегментов
	LCD_Bit(seg->A, (pattern & 0x01) ? SET : RESET);
	LCD_Bit(seg->B, (pattern & 0x02) ? SET : RESET);
	LCD_Bit(seg->C, (pattern & 0x04) ? SET : RESET);
	LCD_Bit(seg->D, (pattern & 0x08) ? SET : RESET);
	LCD_Bit(seg->E, (pattern & 0x10) ? SET : RESET);
	LCD_Bit(seg->F, (pattern & 0x20) ? SET : RESET);
	LCD_Bit(seg->G, (pattern & 0x40) ? SET : RESET);
}
//----------------------------------------------------------------------------//
const uint8_t CharacterPatterns[] = {
	// GFEDCBA (стандартный порядок битов)
	0x3F,	// 0 (A,B,C,D,E,F)		00111111
	0x06,	// 1 (B,C)				00000110
	0x5B,	// 2 (A,B,G,E,D)		01011011
	0x4F,	// 3 (A,B,G,C,D)		01001111
	0x66,	// 4 (F,G,B,C)			01100110
	0x6D,	// 5 (A,F,G,C,D)		01101101
	0x7D,	// 6 (A,F,G,C,D,E)		01111101
	0x07,	// 7 (A,B,C)			00000111
	0x7F,	// 8 (A,B,C,D,E,F,G)	01111111
	0x6F,	// 9 (A,B,C,D,F,G)		01101111
	0x40,	// LINE (только G)		01000000
	0x63,	// GRADUS				01100011
	0x39,	// CELSIUS				00111001
};

typedef u16 LCD_SymbolMask;

const LCD_SymbolMask LCD_Symbols[] = {
	[0]      = 0x0000, // NULL/пробел
	
    
    // Английские буквы
	['A']    = 0x03CF, // A
	['B']    = 0x13FB, // B
	['C']    = 0x00F3, // C
	['D']    = 0x843F, // D
	['E']    = 0x03F3, // E
	['F']    = 0x03C3, // F
	['G']    = 0x02FB, // G
	['H']    = 0x03CC, // H
	['I']    = 0x4833, // I
	['J']    = 0x007C, // J
	['K']    = 0x31C0, // K
	['L']    = 0x00F0, // L
	['M']    = 0x14CC, // M
	['N']    = 0x24CC, // N
	['O']    = 0x00FF, // O
	['P']    = 0x03C7, // P
	['Q']    = 0x20FF, // Q
	['R']    = 0x23C7, // R
	['S']    = 0x03BB, // S
	['T']    = 0x4803, // T
	['U']    = 0x00FC, // U
	['V']    = 0x90C0, // V
	['W']    = 0xA0CC, // W
	['X']    = 0xB400, // X
	['Y']    = 0x5400, // Y
	['Z']    = 0x9033, // Z

	// Цифры
	['0']    = 0x00FF, // 0
	['1']    = 0x000C, // 1
	['2']    = 0x0377, // 2
	['3']    = 0x033F, // 3
	['4']    = 0x038C, // 4
	['5']    = 0x03BB, // 5
	['6']    = 0x03FB, // 6
	['7']    = 0x000F, // 7
	['8']    = 0x03FF, // 8
	['9']    = 0x03BF, // 9

	// Русские буквы
	['А']    = 0x03CF, // А (как A)
	['Б']    = 0x02FB, // Б (как 6 или B)
	['В']    = 0x13FB, // В (как B)
	['Г']    = 0x00C3, // Г
	['Д']    = 0x883E, // Д
	['Е']    = 0x03F3, // Е (как E)
	['Ж']    = 0x4BCC, // Ж
	['З']    = 0x033F, // З
	['И']    = 0x90CC, // И
	['Й']    = 0x90CD, // Й
	['К']    = 0x31C0, // К (как K)
	['Л']    = 0x880E, // Л
	['М']    = 0x14CC, // М (как M)
	['Н']    = 0x03CC, // Н (как H)
	['О']    = 0x00FF, // О (как O)
	['П']    = 0x00CF, // П
	['Р']    = 0x03C7, // Р (как P)
	['С']    = 0x00F3, // С (как C)
	['Т']    = 0x4803, // Т (как T)
	['У']    = 0x03BC, // У
	['Ф']    = 0x4B87, // Ф
	['Х']    = 0xB400, // Х (как X)
	['Ц']    = 0x20FC, // Ц
	['Ч']    = 0x038C, // Ч
	['Ш']    = 0x48FC, // Ш
	['Щ']    = 0x68FC, // Щ
	['Ъ']    = 0x4A29, // Ъ
	['Ы']    = 0x41DC, // Ы
	['Ь']    = 0x4A28, // Ь
	['Э']    = 0x023F, // Э
	['Ю']    = 0x49EE, // Ю
	['Я']    = 0x838F, // Я

// Специальные символы
[' ']    = 0x0000, // Пробел
};

//----------------------------------------------------------------------------//
void LCD_Show_Character_Pos(s16 symbol, uint8_t pos){
	if (pos < 1 || pos > 6) return;
	
	const u16 pattern = LCD_Symbols[symbol];	// Получаем паттерн
	const LCD_Character *seg = &LCD_CharacterPositions[pos-1];
	
	// Устанавливаем состояние сегментов
	LCD_Bit(seg->A1,	(pattern & 0x0001) ? SET : RESET);
	LCD_Bit(seg->A2,	(pattern & 0x0002) ? SET : RESET);
	LCD_Bit(seg->B,		(pattern & 0x0004) ? SET : RESET);
	LCD_Bit(seg->C,		(pattern & 0x0008) ? SET : RESET);
	LCD_Bit(seg->D1,	(pattern & 0x0010) ? SET : RESET);
	LCD_Bit(seg->D2,	(pattern & 0x0020) ? SET : RESET);
	LCD_Bit(seg->F,		(pattern & 0x0040) ? SET : RESET);
	LCD_Bit(seg->E,		(pattern & 0x0080) ? SET : RESET);
	LCD_Bit(seg->G1,	(pattern & 0x0100) ? SET : RESET);
	LCD_Bit(seg->G2,	(pattern & 0x0200) ? SET : RESET);
	LCD_Bit(seg->H,		(pattern & 0x0400) ? SET : RESET);
	LCD_Bit(seg->I,		(pattern & 0x0800) ? SET : RESET);
	LCD_Bit(seg->J,		(pattern & 0x1000) ? SET : RESET);
	LCD_Bit(seg->K,		(pattern & 0x2000) ? SET : RESET);
	LCD_Bit(seg->L,		(pattern & 0x4000) ? SET : RESET);
	LCD_Bit(seg->M,		(pattern & 0x8000) ? SET : RESET);
}
//----------------------------------------------------------------------------//
static void LCD_Show_Lines(){
	for(u8 i = 8; i < 16; i++)	LCD_Show_Val_Pos(LINE, i);
}
//----------------------------------------------------------------------------//
static void LCD_Symbol(const uint8_t value, bool showFlag){	//Функция вывода симола ЖКИ
	switch(value){
		case S1:	LCD_Bit(LCD_S1, showFlag);	break;
		case S2:	LCD_Bit(LCD_S2, showFlag);	break;
		case S3:	LCD_Bit(LCD_S3, showFlag);	break;
		case S4:	LCD_Bit(LCD_S4, showFlag);	break;
		case S5:	LCD_Bit(LCD_S5, showFlag);	break;
		case S6:	LCD_Bit(LCD_S6, showFlag);	break;
		case S7:	LCD_Bit(LCD_S7, showFlag);	break;
		case SS8:	LCD_Bit(LCD_S8, showFlag);	break;

		case S9:	LCD_Bit(LCD_S9, showFlag);	break;
		case S10:	LCD_Bit(LCD_S10, showFlag);	break;
		case S11:	LCD_Bit(LCD_S11, showFlag);	break;
		case S12:	LCD_Bit(LCD_S12, showFlag);	break;
		case S13:	LCD_Bit(LCD_S13, showFlag);	break;
		case S14:	LCD_Bit(LCD_S14, showFlag);	break;
		case S15:	LCD_Bit(LCD_S15, showFlag);	break;
		case SS16:	LCD_Bit(LCD_S16, showFlag);	break;
		
		case S17:	LCD_Bit(LCD_S17, showFlag);	break;
		case S18:	LCD_Bit(LCD_S18, showFlag);	break;
		case S19:	LCD_Bit(LCD_S19, showFlag);	break;
		case S20:	LCD_Bit(LCD_S20, showFlag);	break;
		case S21:	LCD_Bit(LCD_S21, showFlag);	break;
		case S22:	LCD_Bit(LCD_S22, showFlag);	break;
		case S23:	LCD_Bit(LCD_S23, showFlag);	break;
		case S24:	LCD_Bit(LCD_S24, showFlag);	break;
		
		case S25:	LCD_Bit(LCD_S25, showFlag);	break;
		case S26:	LCD_Bit(LCD_S26, showFlag);	break;
		case S27:	LCD_Bit(LCD_S27, showFlag);	break;
		case S28:	LCD_Bit(LCD_S28, showFlag);	break;
		case S29:	LCD_Bit(LCD_S29, showFlag);	break;
		case S30:	LCD_Bit(LCD_S30, showFlag);	break;
		case S31:	LCD_Bit(LCD_S31, showFlag);	break;
		case SS32:	LCD_Bit(LCD_S32, showFlag);	break;
		
		case S33:	LCD_Bit(LCD_S33, showFlag);	break;
		case S34:	LCD_Bit(LCD_S34, showFlag);	break;
		case S35:	LCD_Bit(LCD_S35, showFlag);	break;
		case S36:	LCD_Bit(LCD_S36, showFlag);	break;
		case S37:	LCD_Bit(LCD_S37, showFlag);	break;
		case S38:	LCD_Bit(LCD_S38, showFlag);	break;
		case S39:	LCD_Bit(LCD_S39, showFlag);	break;
		case S40:	LCD_Bit(LCD_S40, showFlag);	break;
		
		case S41:	LCD_Bit(LCD_S41, showFlag);	break;
		case S42:	LCD_Bit(LCD_S42, showFlag);	break;
		case S43:	LCD_Bit(LCD_S43, showFlag);	break;
		case S44:	LCD_Bit(LCD_S44, showFlag);	break;
		case S45:	LCD_Bit(LCD_S45, showFlag);	break;
		case S46:	LCD_Bit(LCD_S46, showFlag);	break;
		case S47:	LCD_Bit(LCD_S47, showFlag);	break;
		case S48:	LCD_Bit(LCD_S48, showFlag);	break;
		
		case S49:	LCD_Bit(LCD_S49, showFlag);	break;
		case S50:	LCD_Bit(LCD_S50, showFlag);	break;
		case S51:	LCD_Bit(LCD_S51, showFlag);	break;
		case S52:	LCD_Bit(LCD_S52, showFlag);	break;
		case S53:	LCD_Bit(LCD_S53, showFlag);	break;
		case S54:	LCD_Bit(LCD_S54, showFlag);	break;
		case S55:	LCD_Bit(LCD_S55, showFlag);	break;
		case S56:	LCD_Bit(LCD_S56, showFlag);	break;
		case S57:	LCD_Bit(LCD_S57, showFlag);	break;
		case S58:	LCD_Bit(LCD_S58, showFlag);	break;
		
		case COLON_1:	LCD_Bit(PP2, showFlag);
						LCD_Bit(PP2_1, showFlag);	break;
		case COLON_2:	LCD_Bit(PP6, showFlag);
						LCD_Bit(PP6_1, showFlag);	break;
		case A:		LCD_Bit(LCD_S28, showFlag);
					LCD_Bit(LCD_S29, showFlag);		break;
		case t:		LCD_Bit(LCD_S28, showFlag);
					LCD_Bit(LCD_S27, showFlag);		break;
		case ANTENNA:	LCD_Bit(LCD_S56, showFlag);
						LCD_Bit(LCD_S57, showFlag);
						LCD_Bit(LCD_S58, showFlag);	break;
	}
}
//----------------------------------------------------------------------------//
static void LCD_Text(const bool showFlag, u16 num, ...){
	va_list ptr;
	va_start( ptr, num);
	while(num--){
		LCD_Symbol(va_arg(ptr, int), showFlag);
	}
	va_end(ptr);
}
//----------------------------------------------------------------------------//
static void LCD_Show_Zeroes(){	//Показать нули в цифровой панели
	for (u8 i = 8; i < 16; i++){
		LCD_Show_Val_Pos(0, i);
	}
}
//----------------------------------------------------------------------------//
#define CLEAR_NUMBER 20
static void LCD_Clear_Data(void){	//Стереть данные, остальное оставить
	for (u8 i = 8; i < 16; i++){
		LCD_Show_Val_Pos(CLEAR_NUMBER, i);
	}
//	LCD_Text(HIDE, 4, P1, P2, P3, P4);
}
//----------------------------------------------------------------------------//
static void LCD_Clear_OBIS_Data(void){	//Стереть OBIS-код
	for (u8 i = 17; i < 24; i++){
		LCD_Show_Val_Pos(CLEAR_NUMBER, i);
	}
	LCD_Text(HIDE, 6, OP1, OP2, OP3, OP4, OP5, OP6);
}
//----------------------------------------------------------------------------//
static void LCD_Show_Long(s32 data, const bool fShowZeroes, const bool fShowMinusObligatorily, u8 sign){	//Вывод 32 разрядного числа на ЖКИ
	u8 i = 0;
	u8 pos = 15;

	LCD_Clear_Data();

	switch(sign){
		case 1:	lcd.text(SHOW, 1, PP1);	break;
		case 2: lcd.text(SHOW, 1, PP2);	break;
		case 3: lcd.text(SHOW, 1, PP3);	break;
		case 4: lcd.text(SHOW, 1, PP4);	break;
	}
	
	if(0L == data){	// ** Если даныне равны нулю, выводим нули и выходим
		if(true == fShowZeroes)	LCD_Show_Zeroes();
		for (u8 j = 15; j >= (15 - sign); j--)	LCD_Show_Val_Pos(0, j);	// ** Вывод трех нулей 
		if ((fShowMinusObligatorily) && (false == fShowZeroes))	LCD_Show_Val_Pos(LINE, 12);	// ** Обязательно показывать минус
		return;
	}
	if(data > 0){	// ** Число положительное
		if(true == fShowZeroes)	LCD_Show_Zeroes();
		while(data){									// ** Здесь мы вывели все значения входящего числа
			LCD_Show_Val_Pos((u8)(data % 10L),pos--);	// ** подсчитали количество его разрядов
			data /= 10L, i++;
		}
		while(i < 1 + sign){			// ** Если количество разрядов меньше 3 
			pos = 15 - i;				// ** Выводим нули на позициях 6 или (6 и 7) или (6 и 7 и 8)
			LCD_Show_Val_Pos(0, pos);
			i++;
		}
		if((fShowMinusObligatorily) && (false == fShowZeroes)){	// ** Обязательно показывать минус
			pos = 15 - i;
			LCD_Show_Val_Pos(LINE, pos);
			i++;
		}
	}
	else{	// ** Число отрицательное
		data = (s32)fabs((float)data);
		while(data){									// ** Здесь мы вывели все значения входящего числа
			LCD_Show_Val_Pos((u8)(data % 10L),pos--);	// ** подсчитали количество его разрядов
			data /= 10L, i++;
		}
		LCD_Show_Val_Pos(LINE,pos);
		while(i < 1 + sign){				// ** Если количество разрядов меньше 3 
			LCD_Show_Val_Pos(0, 15 - i);		// ** Выводим нули на позициях 6 или (6 и 7) или (6 и 7 и 8)
			LCD_Show_Val_Pos(LINE, 15 - i - 1);
			i++;
		}
	}
}
//----------------------------------------------------------------------------//
static void LCD_Show_time(const RTC_TimeTypeDef* TimeStructure){	//Вывод на ЖКИ времени 
	LCD_Show_Val_Pos(TimeStructure->Hours	/ 10,8);
	LCD_Show_Val_Pos(TimeStructure->Hours	% 10,9);
	LCD_Show_Val_Pos(TimeStructure->Minutes / 10,10);
	LCD_Show_Val_Pos(TimeStructure->Minutes % 10,11);
	LCD_Show_Val_Pos(TimeStructure->Seconds / 10,12);
	LCD_Show_Val_Pos(TimeStructure->Seconds % 10,13);	
	LCD_Text(SHOW, 4, PP6, PP6_1, PP4, PP4_1);
}
//----------------------------------------------------------------------------//
static void LCD_Show_date(const RTC_TimeTypeDef* TimeStructure){	//Вывод на ЖКИ даты
	LCD_Show_Val_Pos(TimeStructure->Year / 1000,	8);
	LCD_Show_Val_Pos(TimeStructure->Year % 1000/100,9);
	LCD_Show_Val_Pos(TimeStructure->Year %100/10,	10);
	LCD_Show_Val_Pos(TimeStructure->Year % 10,		11);
	LCD_Show_Val_Pos(TimeStructure->Month / 10,12);
	LCD_Show_Val_Pos(TimeStructure->Month % 10,13);
	LCD_Show_Val_Pos(TimeStructure->Date / 10,14);
	LCD_Show_Val_Pos(TimeStructure->Date % 10,15);
}
//----------------------------------------------------------------------------//
void LCD_Show_Text(char* str){
	u8 index = 0;	//индекс символа строки
	while(0 != str[index]){
		LCD_Show_Character_Pos(str[index], (index + 1));
		index++;
		if(index > 5) break;
	}
}
//----------------------------------------------------------------------------//
void LCD_Clear_Text(void){
	LCD_Show_Text("      ");
	LCD_Text(HIDE, 5, CP1, CP2, CP3, CP4, CP5);
}
//----------------------------------------------------------------------------//
const struct LCD_driver lcd = {
	.init				= LCD_Init,
	.update				= LCD_Update,
	.all				= LCD_All,
	.clear				= LCD_Clear,
	.show_lines			= LCD_Show_Lines,
	.text				= LCD_Text,
	.clear_OBIS_data	= LCD_Clear_OBIS_Data,
	.show_val_pos		= LCD_Show_Val_Pos,
	.clear_data			= LCD_Clear_Data,
	.show_date 			= LCD_Show_date,
	.show_time 			= LCD_Show_time,
	.show_long			= LCD_Show_Long,
	.showText			= LCD_Show_Text,
	.clearText			= LCD_Clear_Text,
};
//----------------------------------------------------------------------------//
