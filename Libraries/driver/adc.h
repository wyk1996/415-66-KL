#include "at32f4xx.h"
#include "at32_board.h"
#include "main.h"

void RCC_Configurations(void);
void adc_init(void);
extern __IO uint16_t ADCConvertedValue[100];