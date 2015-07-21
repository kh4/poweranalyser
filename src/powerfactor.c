#include "board.h"

// inputs:
//  voltage via dividers 400k into 3k3 up and down + 470pF filter
//    output 4.108mV/V; ADC full range (3v3) maps to 803.3v
//      => voltage = 803.3 * adcval / 4096
//  current from ADC758-50B 40mV/A
//     full range maps to 82.5A
//     => current = 82.5 * adcval / 4096
//
//  We use 'raw adc unit' as long as possible ;)
//
//

#define USCALE (803.3 / 4096.0)
#define ISCALE (82.5 / 4096.0)

#define CYCLES 100

volatile int16_t  minU[3], maxU[3], minI[3], maxI[3];
volatile int32_t  sumU[3], sumI[3], sumAbsU[3], sumAbsI[3];
volatile int32_t  sumUI[3]; // integrated power (divided by 256)
volatile uint32_t measurementTime;
volatile uint32_t __samples;
volatile uint8_t  __state = 0;
volatile uint16_t __cycles;


struct pfResults pfResults;

#define ZC_THRESHOLD -100

void handleValuesFromADC(int16_t values[6])
{
  int i;
  if (__state == 1) {
    // wait until U is low enough
    if (values[0] < ZC_THRESHOLD) __state = 2;
    return;
  }
  if (__state == 2) {
    // wait until it crosses positive and go into measurement mode
    if (values[0] >= 0)
      measurementTime = micros();
      __state = 3;
  }
  if (__cycles >= CYCLES) // enough cycles, wait for crossing to stop
    __state = 4;
  if ((__state == 4) && (values[0] < ZC_THRESHOLD)) // negative wait for zero
    __state = 5;
  if ((__state == 5) && (values[0] >= 0)) { // DONE
    __state = 0;
    measurementTime = micros() - measurementTime;
    return;
  }

  __samples++;
  for (i = 0; i < 3; i++) {
    int16_t U = values[i * 2];
    int16_t I = values[i * 2 + 1];
    if (U < minU[i]) minU[i] = U;
    if (U > maxU[i]) maxU[i] = U;
    sumU[i]    += U;
    sumAbsU[i] += abs(U);
    if (I < minI[i]) minI[i] = I;
    if (I > maxI[i]) maxI[i] = I;
    sumI[i]    += I;
    sumAbsI[i] += abs(I);
    sumUI[i]   += ((int32_t)U * (int32_t)I) / 256;
  }
}

#define NOZC_TIME 2000 // 2s
#define TIMEOUT 5000 

uint8_t pfMeasure()
{ // return non zero if in trouble
  int i;
  uint32_t start = millis();
  for (i = 0; i < 3; i++) {
    minU[i] = 0;
    maxU[i] = 0;
    minI[i] = 0;
    maxI[i] = 0;
    sumU[i] = 0;
    sumI[i] = 0;
    sumAbsU[i] = 0;
    sumAbsI[i] = 0;
    sumUI[i] = 0;
  }
  __samples = 0;
  __cycles = 0;
  __state = 1;

  delay(100);

  if ((__state == 1) || (__state == 2)) {
    __state = 6; // sample without ZC
    start = millis();
    while (((millis() - start) < NOZC_TIME));
    __state = 0;
    pfResults.frequency = 0;
  } else {
    // sampling with ZC detection
    while (__state && ((millis() - start) < TIMEOUT));
    if (__state) {
      __state = 0;
      return 1;
    }
    pfResults.frequency = 1000000.0 * (float) CYCLES / (float)measurementTime;
  }

  pfResults.powerTotalW = 0;
  pfResults.powerTotalVA = 0;
  for (i = 0; i < 3; i++) {
    pfResults.Upp[i] = (float)(maxU[i] - minU[i]) * USCALE;
    pfResults.Ipp[i] = (float)(maxI[i] - minI[i]) * USCALE;

    pfResults.Urms[i] = (float)(sumAbsU[i]) / __samples * USCALE;
    pfResults.Irms[i] = (float)(sumAbsI[i]) / __samples * ISCALE;

    pfResults.powerW[i] = pfResults.Urms[i] * pfResults.Irms[i];
    pfResults.powerVA[i] = (float)sumUI[i] / __samples * 256.0 * USCALE * ISCALE;
    pfResults.powerFactor[i] = pfResults.powerVA[i] / pfResults.powerW[i];
    pfResults.powerTotalW += pfResults.powerW[i];
    pfResults.powerTotalVA += pfResults.powerVA[i];
  }
  pfResults.powerTotalFactor = pfResults.powerTotalVA / pfResults.powerTotalW;
  return 0;
}


