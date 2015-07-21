#include "board.h"

void handleValuesFromADC(int16_t[6]);
uint8_t pfMeasure();

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
