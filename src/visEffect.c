/*

  WS2812B CPU and memory efficient library

  Date: 28.9.2016

  Author: Martin Hubacek
  	  	  http://www.martinhubacek.cz
  	  	  @hubmartin

  Licence: MIT License

*/

#include <stdint.h>

#include "stm32f3xx_hal.h"
#include "ws2812b/ws2812b.h"
#include <stdlib.h>

// RGB Framebuffers
uint8_t frameBuffer[4*WS2812B_NUMBER_OF_LEDS] = {0};

// Helper defines
#define newColor(r, g, b, w) (((uint32_t)(w) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) <<  8) | (b))
#define White(c) ((uint8_t)((c >> 24) & 0xFF))
#define Red(c) ((uint8_t)((c >> 16) & 0xFF))
#define Green(c) ((uint8_t)((c >> 8) & 0xFF))
#define Blue(c) ((uint8_t)(c & 0xFF))

uint32_t Wheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return newColor(255 - WheelPos * 3, 0, WheelPos * 3, 0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return newColor(0, WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  WheelPos -= 170;
  return newColor(WheelPos * 3, 255 - WheelPos * 3, 0, 0);
}

void visRainbow(uint8_t *frameBuffer, uint32_t frameBufferSize, uint32_t effectLength)
{
	uint32_t i;
	static uint8_t x = 0;

	x += 1;

	if(x == 256*5)
		x = 0;

	for( i = 0; i < frameBufferSize / 4; i++)
	{
		uint32_t color = Wheel(((i * 256) / effectLength + x) & 0xFF);

		frameBuffer[i*4 + 0] = color & 0xFF;
		frameBuffer[i*4 + 1] = color >> 8 & 0xFF;
		frameBuffer[i*4 + 2] = color >> 16 & 0xFF;
		frameBuffer[i*4 + 3] = 0x00;
	}
}

void visDots(uint8_t *frameBuffer, uint32_t frameBufferSize, uint32_t random, uint32_t fadeOutFactor)
{
	uint32_t i;

	for( i = 0; i < frameBufferSize / 3; i++)
	{

		if(rand() % random == 0)
		{
			frameBuffer[i*3 + 0] = 255;
			frameBuffer[i*3 + 1] = 255;
			frameBuffer[i*3 + 2] = 255;
		}


		if(frameBuffer[i*3 + 0] > fadeOutFactor)
			frameBuffer[i*3 + 0] -= frameBuffer[i*3 + 0]/fadeOutFactor;
		else
			frameBuffer[i*3 + 0] = 0;

		if(frameBuffer[i*3 + 1] > fadeOutFactor)
			frameBuffer[i*3 + 1] -= frameBuffer[i*3 + 1]/fadeOutFactor;
		else
			frameBuffer[i*3 + 1] = 0;

		if(frameBuffer[i*3 + 2] > fadeOutFactor)
			frameBuffer[i*3 + 2] -= frameBuffer[i*3 + 2]/fadeOutFactor;
		else
			frameBuffer[i*3 + 2] = 0;
	}
}

void visInit()
{
	// Set output channel/pin, GPIO_PIN_0 = 0, for GPIO_PIN_5 = 5 - this has to correspond to WS2812B_PINS
	// Your RGB framebuffer
	// RAW size of framebuffer

	// GPIO_PIN_7
	ws2812b.item[0].channel = 7;
	ws2812b.item[0].frameBufferPointer = frameBuffer;
	ws2812b.item[0].frameBufferSize = sizeof(frameBuffer);

	ws2812b_init();
	ws2812b.startTransfer = 1;
	ws2812b_handle();
}

void setPixelColor(int n, uint32_t c){

}

void visHandle()
{
	if(ws2812b.transferComplete)
	{
		visRainbow(frameBuffer, sizeof(frameBuffer), 2);

		/*
		frameBuffer[0*4 + 0] = 0xff;
		frameBuffer[0*4 + 1] = 0x00;
		frameBuffer[0*4 + 2] = 0xff;
		frameBuffer[0*4 + 3] = 0x00;  // <<-- is not transmitted !

		frameBuffer[1*4 + 0] = 0xff;  // <<-- this is White for 1st LED ??
		frameBuffer[1*4 + 1] = 0xff;
		frameBuffer[1*4 + 2] = 0x00;
		frameBuffer[1*4 + 3] = 0x00;
*/
		// Signal that buffer is changed and transfer new data
		ws2812b.startTransfer = 1;
		ws2812b_handle();
	}
}


