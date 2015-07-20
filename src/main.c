#include "board.h"
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

int main(void)
{
    systemInit();
    init_printf(NULL, _putc);
    uartInit(115200);
    ledInit();
    lcdInit();
    adcInit();

    // loopy
    lcdWriteLine(0,"PhaseAngleMeter \0011.0");
    while (1) {
      checkBootLoaderEntry(false);

      printf("%u %d %d:%08x %08x %08x\r\n",micros(), adcReadHTs(), adcReadTCs(),
             adcReadVal(0),adcReadVal(1),adcReadVal(2));
      {
        char line[21];
        sprintf(line,"%03x%03x %03x%03x %03x%03x",
                adcReadVal(0),adcReadVal(1),
                adcReadVal(2),adcReadVal(3),
                adcReadVal(4),adcReadVal(5));
        lcdWriteLine(1,line);
      }
      delay(1000);
      ledSet(2);
    }
}
