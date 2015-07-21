#include "board.h"

/*
    Rotary encoder
*/


volatile int16_t __rotaryState = 0;
volatile int8_t  __buttonState = 0;
uint32_t __buttonDownMillis = 0;

void EXTI1_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line1)) {
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)) {
      // button is up again, process time
      if ((millis() - __buttonDownMillis) > 1000) {
        __buttonState |= 4;
      } else if ((millis() - __buttonDownMillis) > 200) {
        __buttonState |= 2;
      } else {
        __buttonState |= 1;
      }
    } else {
      // record time on button down
      __buttonDownMillis = millis();
    }
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI15_10_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line10)) {
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) {
      __rotaryState++;
    } else {
      __rotaryState--;
    }
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
}

uint8_t rotaryGetEncoderState()
{
  uint8_t t = __rotaryState;
  __rotaryState = 0;
  return t;
}
uint8_t rotaryGetButtonState()
{
  uint8_t t = __buttonState;
  __buttonState = 0;
  return t;
}


void rotaryInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // rotary on PB10 PB11, button PB1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // EXTI init for rotary (PB10)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // EXTI init for button (PB1)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Interrupts
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}

