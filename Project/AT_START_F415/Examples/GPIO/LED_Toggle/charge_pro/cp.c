/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description cp采样
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <stdlib.h>
#include "adc.h"


/* 3300mv */
#define   CP_ADC_REFER_VOLT     3300
/* 转换位数为12位 */
#define   CP_ADC_CONVERT_BITS   (1ul << 12)
#define   CP_ADC_MIN_CONVERT_V  1300
#define   CP_ADC_DIFF            700

/**
 * @brief 获取cp电压采样值
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint32_t ch_get_cp_volt(uint32_t *puiCpVolt)
{
    uint32_t value     = 0;
    uint32_t uiDValue  = 0;

    uint32_t i = 0;
    uint32_t n = 0;
    uint32_t uiaBuf[8] = {0};
    uint32_t uiSum = 0;
    uint32_t uiVaule = 0;

    for(i = 0; i < 50; i++)
    {
        /* 读取采样值 */
        if(ADCConvertedValue[2*i] < CP_ADC_MIN_CONVERT_V)
        {
            continue;
        }

      
        if(ADCConvertedValue[2*i + 1] < CP_ADC_MIN_CONVERT_V)
        {
            continue;
        }

        uiDValue = ADCConvertedValue[2*i] >= ADCConvertedValue[2*i + 1]  ? ADCConvertedValue[2*i] - ADCConvertedValue[2*i + 1]   : ADCConvertedValue[2*i + 1]  - ADCConvertedValue[2*i]  ;
        if(uiDValue > CP_ADC_DIFF)
        {
            continue;
        }
        value     = (ADCConvertedValue[2*i + 1]  + ADCConvertedValue[2*i]) / 2 ;

        uiaBuf[n] = value * CP_ADC_REFER_VOLT / CP_ADC_CONVERT_BITS;
        uiSum += uiaBuf[n];

        if(++n == 4)
        {
//			rt_adc_disable(pcp_dev,  CP_ADC_CHANNEL);
            //return (uiSum / 8);
            uiVaule = uiSum / 4;
            *puiCpVolt = uiVaule * 4;
            return 0;
        }
    }

    return 0;
}
