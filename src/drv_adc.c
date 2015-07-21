#include "board.h"

/*
    3 x 2 channel ADC
*/

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
__IO uint32_t ADC_DualConvertedValueTab[6];
uint16_t __zero[6];
volatile uint8_t __calibrating = 0;
uint32_t __zeroSums[6];
uint16_t __zeroCount;


#define CAL_SAMPLES 50000

void __handleCalibration(uint16_t *values)
{
  int i;
  if (__zeroCount == 0) {
    for (i=0; i<6; i++)
      __zeroSums[i]=values[i];
    __zeroCount++;
  } else if (__zeroCount >= CAL_SAMPLES) {
    __calibrating = 0;
    for (i=0; i<6; i++)
      __zero[i] = __zeroSums[i] / CAL_SAMPLES;
  } else {
    for (i=0; i<6; i++)
      __zeroSums[i]+=values[i];
    __zeroCount++;
  }
}

void __processADC(bool isFull)
{
  int i;
  uint16_t _values[6];
  for (i = 0; i < 3; i++) {
    uint8_t idx = (isFull ? 3 : 0) + i;
    _values[i * 2] = ADC_DualConvertedValueTab[idx] & 0xfff;
    _values[i * 2 + 1] = (ADC_DualConvertedValueTab[idx]>>16) & 0xfff;
  }
  if (__calibrating)  {
    __handleCalibration(_values);
  } else {
    for (i=0; i<6; i++)
      _values[i]-=__zero[i];
    handleValuesFromADC((int16_t*)_values);
  }
}


void DMA1_Channel1_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_HT1)) {
    DMA_ClearITPendingBit(DMA1_IT_HT1);
    __processADC(0);
  }
  if (DMA_GetITStatus(DMA1_IT_TC1)) {
    DMA_ClearITPendingBit(DMA1_IT_TC1);
    __processADC(1);
  }
}

void adcInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // ADC on PA0 - PA5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* DMA1 channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 6;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    /* Enable DMA1 Channel1 */
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // DMA Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 3;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channels configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_239Cycles5);
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
/* ADC2 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 3;
  ADC_Init(ADC2, &ADC_InitStructure);
  /* ADC2 regular channels configuration */
  ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 2, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 3, ADC_SampleTime_239Cycles5);
  /* Enable ADC2 external trigger conversion */
  ADC_ExternalTrigConvCmd(ADC2, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
/* Enable Vrefint channel17 */
  ADC_TempSensorVrefintCmd(ENABLE);

  /* Enable ADC1 reset calibration register */
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);

  /* Enable ADC2 reset calibration register */
  ADC_ResetCalibration(ADC2);
  /* Check the end of ADC2 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC2));

  /* Start ADC2 calibration */
  ADC_StartCalibration(ADC2);
 /* Check the end of ADC2 calibration */
  while(ADC_GetCalibrationStatus(ADC2));

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void adcCalibrateStart()
{
  __zeroCount=0;
  __calibrating=1;
}

uint8_t adcCalibrateWait()
{
  if (!__calibrating)
    return 255;
  else
    return (uint32_t)__zeroCount * 100 / CAL_SAMPLES;
}
