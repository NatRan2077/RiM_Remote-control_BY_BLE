//----------------------------------------------------------------------------//
#ifndef INCLCD_H
#define INCLCD_H
//----------------------------------------------------------------------------//
#include "cmt453x.h"
//----------------------------------------------------------------------------//
#define SET 	1
#define RESET	0
//----------------------------------------------------------------------------//
typedef struct{
	uint32_t Year; 
	uint32_t Month;   
	uint32_t Date;  
	uint32_t WeekDay;
	uint32_t Hours;   
	uint32_t Minutes;     
	uint32_t Seconds;     
}RTC_TimeTypeDef;
//----------------------------------------------------------------------------//
enum{
	S1	= 0,
	S2	= 1,
	S3	= 2,
	S4	= 3,
	S5	= 4,
	S6	= 5,
	S7	= 6,
	SS8	= 7,
	
	S9	= 8,
	S10	= 9,
	S11	= 10,
	S12	= 11,
	S13	= 12,
	S14	= 13,
	S15	= 14,
	SS16	= 15,
	S17	= 16,
	S18	= 17,
	S19	= 18,
	S20	= 19,
	
	S21	= 20,
	S22	= 21,
	S23	= 22,
	S24	= 23,
	S25	= 24,
	S26	= 25,
	S27	= 26,
	S28	= 27,
	S29	= 28,
	S30	= 29,
	
	S31	= 30,
	SS32	= 31,
	S33	= 32,
	S34	= 33,
	S35	= 34,
	S36	= 35,
	S37	= 36,
	S38	= 37,
	S39	= 38,
	S40	= 39,
	
	S41	= 40,
	S42	= 41,
	S43	= 42,
	S44	= 43,
	S45	= 44,
	S46	= 45,
	S47	= 46,
	S48	= 47,
	S49	= 48,
	S50	= 49,
	
	S51	= 50,
	S52	= 51,
	S53	= 52,
	S54	= 53,
	S55	= 54,
	S56	= 55,
	S57	= 56,
	S58	= 57,
	
	COLON_1	= 98,
	COLON_2	= 99,
	
	A	= 100,
	t	= 101,
	H	= 104,
	Pr	= 105,
	HC	= 106,
	ANTENNA	= 107,
	PROGRAMM	= 250,
	ERASE	= 251,
	CELSIUS	= 252,
	GRADUS	= 253,
	LINE	= 254,
	SPACE	= 255,
};
//----------------------------------------------------------------------------//
#define	PP1		S21
#define PP2		S19
#define PP2_1	S20
#define PP3		S18
#define PP4		SS16
#define PP4_1	S17
#define PP5		S15
#define PP6		S13
#define PP6_1	S14
#define PP7		S12

#define OP1		S33
#define OP2		S34
#define OP3		S35
#define OP4		S36
#define OP5		S37
#define OP6		S38

#define CP1		S7
#define CP2		SS8
#define CP3		S9
#define CP4		S10
#define CP5		S11

#define M		S22
#define k		S24
#define V		S26
#define r		S31
#define h		SS32
#define Hz		S23
#define procent	S25

#define RELAY_ARR	S43
#define RELAY		S42
#define CLOCK		S45
#define Q_MINUS		S6
#define P_PLUS		S4
#define Q_PLUS		S3
#define P_MINUS		S5
#define WARNING		S39
#define	UHF			S44
#define	MAG			S40
#define	EQI			S53

#define TAMPER_COVER	S50
#define TAMPER_BATTARY	S51
#define TAMPER_METER	S52

#define PHASE_A		S47
#define PHASE_B		S48
#define PHASE_C		S49

#define BATT		S54
#define BATT_1		S55

#define TARIF		S2
//----------------------------------------------------------------------------//
enum{
	HIDE = 0,
	SHOW = 1,
};
//----------------------------------------------------------------------------//
struct LCD_driver{
	u8 (*init)(void);
	u8 (*update)(void);
	void (*all)(void);
	void (*clear)(void);
	void (*show_lines)(void);
	void (*text)(const bool showFlag, u16 num, ...);
	void (*clear_OBIS_data)(void);
	void (*show_val_pos)(const u8 value, u8 position);
	void (*clear_data)(void);
	void (*show_date)(const RTC_TimeTypeDef* TimeStructure);
	void (*show_time)(const RTC_TimeTypeDef* TimeStructure);
	void (*show_long)(s32 data, const bool fShowZeroes, const bool fShowMinusObligatorily, u8 sign);
	void (*showText)(char* str);
	void (*clearText)(void);
};
extern const struct LCD_driver lcd;
//----------------------------------------------------------------------------//
#endif
