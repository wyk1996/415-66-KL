#include <stdio.h>
#include "at32f4xx.h"
#include "at32_board.h"
#include  "main.h"

extern ERTC_InitType ERTC_InitStructure;
extern ERTC_TimeType ERTC_TimeStructure;
extern ERTC_DateType ERTC_DateStructure;
extern ERTC_TimeType ERTC_TimeStampStructure;
extern ERTC_DateType ERTC_TimeStampDateStructure;


extern uint32_t uwAsynchPrediv ;
extern uint32_t uwSynchPrediv ;
extern uint32_t uwSecondfraction ;
void ERTC_TimeShow(void);
void ertc(void);
void System_Init(void);
void bsp_InitDelays(void);
void ERTC_TimeRegulate(void);
void ERTC_TimeStampConfig(void);
void ERTC_Config(void);

