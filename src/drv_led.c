#include "board.h"

void ledInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // USART1_TX    PA9
    // USART1_RX    PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static uint8_t __ledstate = 0;

void ledSet(uint8_t state) {

  switch (state) {
  case 0: //OFF
    __ledstate = 0;
    break;
  case 1: //ON
    __ledstate = 1;
    break;
  case 2: //TOGGLE
    __ledstate = !__ledstate;
    break;
  default:
    break;
  }
  GPIO_WriteBit(GPIOC, GPIO_Pin_13, __ledstate);
}
