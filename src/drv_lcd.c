#include "board.h"

// HCD pinning
// B3 RS (C/D)
// B4 E
// B5 D4
// B6 D5
// B7 D6
// B8 D7

#define LCD_RS GPIO_Pin_3 // LCD pin 4
#define LCD_RW            // LCD pin 5 tied to GND
#define LCD_E  GPIO_Pin_4 // LCD pin 6
#define LCD_D4 GPIO_Pin_5 // LCD pin 11
#define LCD_D5 GPIO_Pin_6 // LCD pin 12
#define LCD_D6 GPIO_Pin_7 // LCD pin 13
#define LCD_D7 GPIO_Pin_8 // LCD pin 14
#define LCDPINS (LCD_RS|LCD_E|LCD_D4|LCD_D5|LCD_D6|LCD_D7)

#define LCD_IODELAY 2
#define LCD_CMDDELAY 50

void __lcdWriteByte(bool c, uint8_t b)
{
  GPIO_WriteBit(GPIOB, LCD_E, 1);
  delayMicroseconds(LCD_IODELAY);
  GPIO_WriteBit(GPIOB, LCD_RS, c ? 0 : 1);
  GPIO_WriteBit(GPIOB, LCD_D7, b & 0x80);
  GPIO_WriteBit(GPIOB, LCD_D6, b & 0x40);
  GPIO_WriteBit(GPIOB, LCD_D5, b & 0x20);
  GPIO_WriteBit(GPIOB, LCD_D4, b & 0x10);
  delayMicroseconds(LCD_IODELAY);
  GPIO_WriteBit(GPIOB, LCD_E, 0);
  delayMicroseconds(LCD_IODELAY);
  GPIO_WriteBit(GPIOB, LCD_E, 1);
  GPIO_WriteBit(GPIOB, LCD_D7, b & 0x08);
  GPIO_WriteBit(GPIOB, LCD_D6, b & 0x04);
  GPIO_WriteBit(GPIOB, LCD_D5, b & 0x02);
  GPIO_WriteBit(GPIOB, LCD_D4, b & 0x01);
  delayMicroseconds(LCD_IODELAY);
  GPIO_WriteBit(GPIOB, LCD_E, 0);
  delayMicroseconds(LCD_IODELAY);
  GPIO_WriteBit(GPIOB, LCD_E, 1);
  delayMicroseconds(LCD_CMDDELAY);
}

const char __lcdCGRAM[64] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void __lcdLoadCGRAM()
{
  uint8_t i;
  __lcdWriteByte(1, 0x40); // CGRAM start
  for (i = 0; i < sizeof(__lcdCGRAM); i++)
    __lcdWriteByte(0, __lcdCGRAM[i]);
}

void lcdClear()
{
  __lcdWriteByte(1, 0x01); // clear
  __lcdWriteByte(1, 0x02); // return home
  delayMicroseconds(1600);
}

void lcdInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = LCDPINS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOB, LCDPINS, 1);
  delayMicroseconds(100);
  __lcdWriteByte(1, 0x28); // nibble mode, 2 rows
  __lcdWriteByte(1, 0x28); // nibble mode, 2 rows
  lcdClear();
  __lcdWriteByte(1, 0x06); // autoincrement, no scrolling
  __lcdLoadCGRAM();
  __lcdWriteByte(1, 0x0c); // display on
}


void lcdWriteLine(uint8_t row, char *txt) {
  uint8_t i;
  __lcdWriteByte(1, 0x80 | (row ? 0x40 : 0x00));
  delayMicroseconds(100);
  for (i = 0; i < 20; i++) {
    __lcdWriteByte(0, (*txt)?*(txt++):' ');
  }
}
