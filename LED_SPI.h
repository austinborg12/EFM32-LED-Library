/*
 * LED_SPI.h
 *
 *  Created on: Jun 27, 2019
 *      Author: Admin
 */

#ifndef LED_SPI
#define LED_SPI
#include <stdint.h>

int LED_SPI_Startup(void);
void LED_Booting();
void LED_Translate(uint8_t data, uint8_t * pBuffer);
void LED_SetColor();
void LED_SetBuffer(uint8_t green, uint8_t red, uint8_t blue, uint8_t numLED);
void LED_SetBufferAll(uint8_t green, uint8_t red, uint8_t blue, uint8_t numLED);
void LED_SetBufferHSVAll(uint32_t c, uint8_t numLED);
void LED_SetBuffer(uint32_t c, uint8_t numLED);
void LED_ClearBuffer();



#endif /* LED_SPI_H_ */
