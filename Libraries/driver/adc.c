/**
  ******************************************************************************
  * File   : ADC/3ADCs_DMA/main.c 
  * Version: V1.2.8
  * Date   : 2020-11-27
  * Brief  : Main program body
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "at32f4xx.h"
#include "at32_board.h"
#include "adc.h"
/** @addtogroup AT32F403A_StdPeriph_Examples
	* @{
	*/

/** @addtogroup ADC_3ADCs_DMA
	* @{
	*/ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ADC_InitType ADC_InitStructure;
DMA_InitType DMA_InitStructure;
__IO uint16_t ADCConvertedValue[100]={0};

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration_(void);
void GPIO_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void adc_init(void)
{
	/* System clocks configuration */
	RCC_Configuration_();

	/* GPIO configuration ------------------------------------------------------*/
	GPIO_Configuration();

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_Reset(DMA1_Channel1);
	DMA_DefaultInitParaConfig(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr    = (uint32_t)&ADC1->RDOR;
	DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)&ADCConvertedValue;
	DMA_InitStructure.DMA_Direction             = DMA_DIR_PERIPHERALSRC;
	DMA_InitStructure.DMA_BufferSize            = 100;
	DMA_InitStructure.DMA_PeripheralInc         = DMA_PERIPHERALINC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc             = DMA_MEMORYINC_ENABLE;
	DMA_InitStructure.DMA_PeripheralDataWidth   = DMA_PERIPHERALDATAWIDTH_HALFWORD;
	DMA_InitStructure.DMA_MemoryDataWidth       = DMA_MEMORYDATAWIDTH_HALFWORD;
	DMA_InitStructure.DMA_Mode                  = DMA_MODE_CIRCULAR;
	DMA_InitStructure.DMA_Priority              = DMA_PRIORITY_HIGH;
	DMA_InitStructure.DMA_MTOM                  = DMA_MEMTOMEM_DISABLE;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	/* Enable DMA1 channel1 */
	DMA_ChannelEnable(DMA1_Channel1, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode              = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanMode          = DISABLE;
	ADC_InitStructure.ADC_ContinuousMode    = ENABLE;
	ADC_InitStructure.ADC_ExternalTrig      = ADC_ExternalTrig_None;
	ADC_InitStructure.ADC_DataAlign         = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NumOfChannel      = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channels configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_28_5);    
	
	/* Enable ADC1 DMA */
	ADC_DMACtrl(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Ctrl(ADC1, ENABLE);

	/* Enable ADC1 reset calibration register */   
	ADC_RstCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCtrl(ADC1, ENABLE);

}

int adc_test(void)
{

	while(1){
		printf("ADCConvertedValue: 0x%x \r\n",ADCConvertedValue[0]& 0xFFFF);
	}
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration_(void)
{
	/* ADCCLK = PCLK2/4 */
	RCC_ADCCLKConfig(RCC_APB2CLK_Div4);
	
	/* Enable peripheral clocks ------------------------------------------------*/
	/* Enable DMA1 clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);

	/* Enable ADC1 and GPIOC clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_ADC1 | RCC_APB2PERIPH_GPIOA, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
	GPIO_InitType GPIO_InitStructure;

	/* Configure PA.0 (ADC Channel0) as analog input -------------------------*/
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pins = GPIO_Pins_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_ANALOG;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
	* @}
	*/ 

/**
	* @}
	*/ 

