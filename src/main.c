#include "board.h"

#define LOOPDELAY 200

int8_t screen = 0;
int8_t oldscreen = 99;

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
  pfStartMeasure();
  while (1) {
    uint8_t result;
    delay(10);
    checkBootLoaderEntry(false);
    result = pfWaitMeasure();
    if (result == 2) {
      lcdWriteLine(0,"       ERROR!       ");
      lcdWriteLine(0,"");
      pfStartMeasure();
    } else if (result==0) {
      ledSet(2);
      oldscreen = 99; //force redraw
    }
    if (screen != oldscreen) {
      oldscreen = screen;
      if (screen == 0) {
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
      } else if (screen==3) {
        char line[21];
        sprintf(line,"%4d %4d %4d d1",
                (int)maxU[0],
                (int)maxU[1],
                (int)maxU[2]);
        lcdWriteLine(0,line);
        sprintf(line,"%4d %4d %4d",
                (int)minU[0],
                (int)minU[1],
                (int)minU[2]);
        lcdWriteLine(1,line);
      } else if (screen==4) {
        char line[21];
        sprintf(line,"%4d %4d %4d d2",
                (int)maxI[0],
                (int)maxI[1],
                (int)maxI[2]);
        lcdWriteLine(0,line);
        sprintf(line,"%4d %4d %4d",
                (int)minI[0],
                (int)minI[1],
                (int)minI[2]);
        lcdWriteLine(1,line);
      } else if (screen==5) {
        char line[21];
        sprintf(line,"%6d%6d%6dd3",
                (int)sumU[0],
                (int)sumU[1],
                (int)sumU[2]);
        lcdWriteLine(0,line);
        sprintf(line,"%6d %6d %6d",
                (int)sumI[0],
                (int)sumI[1],
                (int)sumI[2]);
        lcdWriteLine(1,line);
      } else if (screen==6) {
        char line[21];
        sprintf(line,"%5d %5d %5d d4",
                sumAbsU[0]/__samples,
                sumAbsU[1]/__samples,
                sumAbsU[2]/__samples);
        lcdWriteLine(0,line);
        sprintf(line,"%5d %5d %5d",
                sumAbsI[0]/__samples,
                sumAbsI[1]/__samples,
                sumAbsI[2]/__samples);
        lcdWriteLine(1,line);
      } else if (screen==7) {
        char line[21];
        sprintf(line,"s=%6u          d5",
                __samples);
        lcdWriteLine(0,line);
        lcdWriteLine(1,"");
      }
    }
    if (result==0)
      pfStartMeasure();

#define MAXSCREEN 7
    uint8_t btn = rotaryGetButtonState();
    if (btn & 0x04) {
      calibrate();
    }
    screen += rotaryGetEncoderState();
    if (screen < 0) screen = 0;
    if (screen > MAXSCREEN) screen = MAXSCREEN;
  }
}
