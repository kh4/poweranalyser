#include "board.h"

#define LOOPDELAY 200

int8_t screen = 0;

static void _putc(void *p, char c)
{
    uartWrite(c);
}


void checkBootLoaderEntry(bool wait)
{
  uint32_t start = millis();
  do {
    if (uartAvailable() && ('R' == uartRead())) {
      lcdClear();
      lcdWriteLine(0, "Entering bootloader.");
      systemReset(true);
      while (1);
    }
  }  while (wait && ((millis() - start) < 2000));
}

void calibrate()
{
  uint8_t p;
  lcdWriteLine(0,"Calibrating ADC");
  adcCalibrateStart();
  for (p = 0; p!=255 ; p=adcCalibrateWait()) {
    char line[21];
    sprintf(line,"%3d%%",p);
    lcdWriteLine(1, line);
    delay(100);
  }
}

int main(void)
{
  systemInit();
  init_printf(NULL, _putc);
  uartInit(115200);
  checkBootLoaderEntry(true);
  ledInit();
  lcdInit();
  rotaryInit();
  adcInit();
  delay(10);
  calibrate();
  // loop
  while (1) {
    checkBootLoaderEntry(false);
    if (pfMeasure()) {
      lcdWriteLine(0,"       ERROR!       ");
      lcdWriteLine(0,"");
    } else {
      if (screen==0) {
        char line[21];
        sprintf(line," U(R)  U(S)  U(T)");
        lcdWriteLine(0,line);
        sprintf(line,"%4dV %4dV %4dV",
                (int)pfResults.Urms[0],
                (int)pfResults.Urms[1],
                (int)pfResults.Urms[2]);
        lcdWriteLine(1,line);
      } else if (screen==1) {
        char line[21];
        sprintf(line," I(R)  I(S)  I(T)");
        lcdWriteLine(0,line);
        sprintf(line,"%4dA %4dA %4dA",
                (int)pfResults.Irms[0],
                (int)pfResults.Irms[1],
                (int)pfResults.Irms[2]);
        lcdWriteLine(1,line);
    } else if (screen==2) {
        char line[21];
        uint16_t hz = pfResults.frequency * 10.0;
        sprintf(line, "%6dW %6dVA",
                (int)pfResults.powerTotalW,
                (int)pfResults.powerTotalVA);
        lcdWriteLine(0,line);
        {
          if (pfResults.powerTotalFactor < 0) {
            int8_t real = -100.0 * pfResults.powerTotalFactor;
            sprintf(line, "cos(f)=-%d.%02d %2d.%1dHz",
                    real/100, real%100, hz / 10, hz % 10);
            lcdWriteLine(1,line);
          } else {
            int8_t real = 100.0 * pfResults.powerTotalFactor;
            sprintf(line, "cos(f)=%d.%02d %2d.%1dHz",
                    real/100, real%100, hz / 10, hz % 10);
            lcdWriteLine(1,line);
          }
        }
      }
    }
    ledSet(2);
    uint8_t btn = rotaryGetButtonState();
    if (btn & 0x04) {
      calibrate();
    }
    screen += rotaryGetEncoderState();
    if (screen < 0) screen = 0;
    if (screen > 2) screen = 2;
  }
}
