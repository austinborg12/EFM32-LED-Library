#ifndef LED_COLORFUNCTIONS
#define LED_COLORFUNCTIONS
#include <stdint.h>


void processingSample(int orbitFraction, int expansionFraction);
void LED_ProgressBar();
uint32_t LED_ColorHSV(uint16_t hue, uint8_t sat, uint8_t val);
uint32_t LED_gamma32(uint32_t x);
#endif
