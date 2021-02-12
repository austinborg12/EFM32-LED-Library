#include "em_cmu.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "LED_SPI.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Buffers for converted
uint8_t LEDBuffer[70*8*4 + 50];
//uint8_t offBuffer[50];

//Used to write buffer to serial pin. Stolen from microsd.c in usbcomposite example
int LED_MICROSD_BlockTx(const uint8_t *buff, uint32_t bc)
{
	uint8_t resp;
	uint16_t val;
	uint32_t framectrl, ctrl;
	//Save USART configuration
	framectrl = USART0->FRAME;
	ctrl      = USART0->CTRL;
	//Set frame length to 16bit
	USART0->FRAME = (USART0->FRAME & (~_USART_FRAME_DATABITS_MASK))
	                       | USART_FRAME_DATABITS_SIXTEEN;
	USART0->CTRL |= USART_CTRL_BYTESWAP;

	do {
	    /* Transmit a 512 byte data block to the SD-Card. */

	    val  = *buff++;
	    val |= *buff++ << 8;
	    bc  -= 2;

	    while (!(USART0->STATUS & USART_STATUS_TXBL)) ;

	    USART0->TXDOUBLE = val;
	  } while (bc);

	  while (!(USART0->STATUS & USART_STATUS_TXBL)) ;

	  /* Transmit two dummy CRC bytes. */
	  USART0->TXDOUBLE = 0x0000;

	  while (!(USART0->STATUS & USART_STATUS_TXC)) ;

	  /* Clear send and receive buffers. */
	  USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

	  /* Restore old settings. */
	  USART0->FRAME = framectrl;
	  USART0->CTRL  = ctrl;

	  return 1;
}

//Initializes the USART for serial communication with LED. Pin PC0.
int LED_SPI_Startup(void)
{
	USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

	//Clock Startup and selection
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, false);
	CMU_ClockEnable(cmuClock_USART0, true);
	CMU_ClockEnable(cmuClock_USART1, true); //new
	CMU_ClockEnable(cmuClock_GPIO, true);

	int freq = CMU_ClockFreqGet(cmuClock_HF);

	//Sets Transmission rate for SPI.
	//Sends 8 pulses in 1.25us to
	uint32_t xfersPrMsec   = 6400000;
	init.baudrate = 6400000;
	init.msbf     = true;
	USART_InitSync(USART0, &init);
	USART_InitSync(USART1, &init);  //new

	//Enables Tx Rx Clock and CS pins. Only Tx is needed for LED Ring
	USART0->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
	                | USART_ROUTE_CLKPEN | USART_ROUTE_LOCATION_LOC5;

	//new--for serial commands
	USART0->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
		            | USART_ROUTE_CLKPEN | USART_ROUTE_LOCATION_LOC5;


	/* IO configuration: Configured Specifically for Pin c0 */
	  GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 0);  /* MOSI */
	  GPIO_PinModeSet(gpioPortC, 1, gpioModeInputPull, 1); /* MISO */

}

//Known as LEDSendByte
void LED_Translate(uint8_t data, uint8_t * pBuffer)
{
	for( int x = 0; x < 8; x++)
		{
			uint8_t bit = data & 0x80;
			data = data << 1;

			//uint8_t bit = wordData >> 8;

			uint8_t transmit = 0;
			if( bit == 0)
			{
				transmit = 0xE0;
			}
			else
			{
				transmit = 0xF8;
			}
			pBuffer[x] = transmit;
		}
}

//Writes buffer to USART Tx buffer. Sends data out PinC0
void LED_SetColor()
{
	USART_TxDouble(USART0, 0);

	LED_MICROSD_BlockTx(LEDBuffer, sizeof(LEDBuffer));

}

//Buffer does not reset on call, addresses LED one at a time with RGB color
void LED_SetBufferAll(uint8_t green, uint8_t red, uint8_t blue, uint8_t numLED)
{
	memset( LEDBuffer, 0, sizeof(LEDBuffer));
	for( int x = 0; x < numLED; x++)
	{
		//uint8_t green = 50;
		//uint8_t red  = 0;
		//uint8_t blue  = 0;

		LED_Translate(green,  &LEDBuffer[x*8*4]);
		LED_Translate(red,  &LEDBuffer[x*8*4] + 8);
		LED_Translate(blue,  &LEDBuffer[x*8*4] + 8*2);
		LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
	}
}

//Buffer does not reset on call, addresses LED one at a time with HSV color
void LED_SetBufferHSVAll(uint32_t c, uint8_t numLED)
{
	memset( LEDBuffer, 0, sizeof(LEDBuffer));
	uint8_t green = 0;
		uint8_t red  = 0;
		uint8_t blue  = 0;

		red = (uint8_t)(c >> 16);
		green = (uint8_t)(c >>  8);
		blue = (uint8_t)c;

		for( int x = 0; x < 70; x++)
			{
				//uint8_t green = 50;
				//uint8_t red  = 0;
				//uint8_t blue  = 0;

				LED_Translate(green,  &LEDBuffer[x*8*4]);
				LED_Translate(red,  &LEDBuffer[x*8*4] + 8);
				LED_Translate(blue,  &LEDBuffer[x*8*4] + 8*2);
				LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
			}
}

//Sets one LED to a RGB color
void LED_SetBuffer(uint8_t green, uint8_t red, uint8_t blue, uint8_t numLED)
{
	memset( LEDBuffer, 0, sizeof(LEDBuffer));
	for( int x = 0; x < 70; x++)
	{
		//uint8_t green = 50;
		//uint8_t red  = 0;
		//uint8_t blue  = 0;

		LED_Translate(0x00,  &LEDBuffer[x*8*4]);
		LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8);
		LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*2);
		LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
		if(x == numLED)
		{
			LED_Translate(green,  &LEDBuffer[x*8*4]);
			LED_Translate(red,  &LEDBuffer[x*8*4] + 8);
			LED_Translate(blue,  &LEDBuffer[x*8*4] + 8*2);
			LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
		}
	}
}


void LED_SetBuffer(uint32_t c, uint8_t numLED)
{
	//memset( LEDBuffer, 0, sizeof(LEDBuffer));

	uint8_t green = 0;
	uint8_t red  = 0;
	uint8_t blue  = 0;

	red = (uint8_t)(c >> 16);
	green = (uint8_t)(c >>  8);
	blue = (uint8_t)c;

	for( int x = 0; x < 70; x++)
		{
			//uint8_t green = 50;
			//uint8_t red  = 0;
			//uint8_t blue  = 0;

			//LED_Translate(0x00,  &LEDBuffer[x*8*4]);
			//LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8);
			//LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*2);
			//LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
			if(x == numLED)
			{
				LED_Translate(green,  &LEDBuffer[x*8*4]);
				LED_Translate(red,  &LEDBuffer[x*8*4] + 8);
				LED_Translate(blue,  &LEDBuffer[x*8*4] + 8*2);
				LED_Translate(0x00,  &LEDBuffer[x*8*4] + 8*3);
			}
		}


}

//Will set all buffer values to zero
void LED_ClearBuffer()
{
	memset( LEDBuffer, 0, sizeof(LEDBuffer));
}



