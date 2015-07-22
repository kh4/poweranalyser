#include "board.h"

void handleValuesFromADC(int16_t[6]);
void pfStartMeasure();
uint8_t pfWaitMeasure();

struct pfResults {
  float Upp[3];
  float Ipp[3];
  float Urms[3];
  float Irms[3];

  float powerW[3];
  float powerVA[3];
  float powerFactor[3];

  float powerTotalW;
  float powerTotalVA;
  float powerTotalFactor;

  float frequency;
};
extern struct pfResults pfResults;

// for debugging
extern volatile int16_t  minU[3], maxU[3], minI[3], maxI[3];
extern volatile int32_t  sumU[3], sumI[3], sumAbsU[3], sumAbsI[3];
extern volatile int32_t  sumUI[3]; // integrated power (divided by 256)
extern volatile uint32_t __samples;

