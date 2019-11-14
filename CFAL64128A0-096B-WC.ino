//==============================================================================
//
//  CRYSTALFONTZ CFAL64128A0-096B-WC EXAMPLE FIRMWARE
//
//  Code written for Seeeduino v4.2 set to 3.3v (important!)
//
//  This code uses interfaces:
//    OLED Display     = 4-wire SPI or I2C (selectable in prefs.h)
//    Touch Controller = I2C
//
//  The OLED/Touch controller is a Sitronix SSD7317
//
//  The OLED and touch controllers are in the same IC, but are for the most
//  part treated like two separate IC's.
//
//  Seeeduino v4.2, an open-source 3.3v capable Arduino clone.
//    https://www.seeedstudio.com/Seeeduino-V4.2-p-2517.html
//    https://github.com/SeeedDocument/SeeeduinoV4/raw/master/resources/Seeeduino_v4.2_sch.pdf
//
//==============================================================================
//
//  2019-10-30 Mark Williams / Crystalfontz
//
//==============================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//==============================================================================

#include <Arduino.h>
#include <Wire.h>
#include "prefs.h"
#include "ssd7317_oled_i2c.h"
#include "ssd7317_touch_i2c.h"
#include "font_8x8x1.h"

//////////////////////////////////////////////////////////

//oled display buffer ram
uint8_t OLEDBuf[(SSD7317_OLED_HEIGHT*SSD7317_OLED_WIDTH)/8]; //1 bit per pixel

//local functions
void PrintChar(uint8_t x, uint8_t y, uint8_t c);
void PrintString(uint8_t x, uint8_t y, const char *str);

//timing
#define SYS_TICK					millis()
#define SYS_TICKSPEED				(1) /* 1mS / tick */
#define TIMER_MAX					UINT32_MAX
#define TIMER_IF_EXPIRED(var)		if ((SYS_TICK - (var)) < (TIMER_MAX/2))
#define TIMER_SET(var,time_ms)		(var) = SYS_TICK + ((time_ms) * SYS_TICKSPEED);

//touch display types
typedef enum
{
	TOUCH_NONE = 0,
	TOUCH_KEY = 1,
	TOUCH_HOLD = 2,
	TOUCH_SWIPE = 4,
} TouchTypes_t;

//////////////////////////////////////////////////////////

//run at power-on / reset
//do inits
void setup()
{
	//startup serial output
	Serial.begin(115200);
	Serial.println("setup()");

	//init the oled controller
	//oled must be initialised before the touch controller
	Serial.println("SSD7317_OLED_Init()");
	//uses hardware i2c
	SSD7317_OLED_Init();
	//black the display buffer, then write status text
	//and send to the display
	memset(OLEDBuf, 0x00, sizeof(OLEDBuf));
	PrintString(1,60, "TOUCH");
	PrintString(1,69, "INIT...");
	SSD7317_OLED_WriteBuffer(OLEDBuf);
#ifdef OLED_I2C
	//disable hardware i2c for now
	Wire.end();
#endif

	//init the capactive touch controller
	//uses software i2c for init as it must ignore I2C NACKs
	Serial.println("SSD7317_Touch_Init()");
	SSD7317_Touch_Init();

	//now use hardware i2c for touch
	Wire.begin();
	SSD7317_Touch_HWI2C(true);
	//kick the i2c speed up to 400kHz
	Wire.setClock(400000);

	//done inits
	Serial.println("setup() done");
}

void UpdateBlankLines(void)
{
	//blank display & draw 4 div key lines
	memset(OLEDBuf, 0x00, sizeof(OLEDBuf));
	memset(OLEDBuf+(32*8), 0xFF, 8);
	memset(OLEDBuf+(64*8), 0xFF, 8);
	memset(OLEDBuf+(96*8), 0xFF, 8);
}

void UpdateForNone(void)
{
	//blank display & draw 4 div key lines
	UpdateBlankLines();
	//print key names on display
	PrintString(1,00+12, "KEY  1");
	PrintString(1,32+12, "KEY  2");
	PrintString(1,64+12, "KEY  3");
	PrintString(1,96+12, "KEY  4");
	//write buffer to oled
	SSD7317_OLED_WriteBuffer(OLEDBuf);
}

void UpdateForKey(uint8_t Key)
{
	uint8_t i;
	//blank display & draw 4 div key lines
	UpdateBlankLines();
	//a key has been pressed
	//highlight the key square on the display
	uint8_t *OLEDBuf_Offset = OLEDBuf + ((Key - 1) * 32 * 8);
	for (i = 0; i < 31; i++)
		if (i % 2)
			memset(OLEDBuf_Offset+(i*8), 0xAA, 8);
		else
			memset(OLEDBuf_Offset+(i*8), 0x55, 8);
	//print key names on display
	PrintString(1,00+12, "KEY  1");
	PrintString(1,32+12, "KEY  2");
	PrintString(1,64+12, "KEY  3");
	PrintString(1,96+12, "KEY  4");
	//write buffer to oled
	SSD7317_OLED_WriteBuffer(OLEDBuf);
}

void UpdateForHold(uint8_t Key)
{
	uint8_t i;
	//blank display & draw 4 div key lines
	UpdateBlankLines();
	//a key has been held
	//highlight the key square on the display
	uint8_t *OLEDBuf_Offset = OLEDBuf + ((Key - 1) * 32 * 8);
	for (i = 0; i < 31; i++)
		if (i % 2)
			memset(OLEDBuf_Offset+(i*8), 0xAA, 8);
		else
			memset(OLEDBuf_Offset+(i*8), 0x55, 8);
	//a key has been held, chnage the text for that row
	if (Key == 1)
		PrintString(1,00+12, "HOLD 1");
	else
		PrintString(1,00+12, "KEY  1");
	if (Key == 2)
		PrintString(1,32+12, "HOLD 2");
	else
		PrintString(1,32+12, "KEY  2");
	if (Key == 3)
		PrintString(1,64+12, "HOLD 3");
	else
		PrintString(1,64+12, "KEY  3");
	if (Key == 4)
		PrintString(1,96+12, "HOLD 4");
	else
		PrintString(1,96+12, "KEY  4");
	//write buffer to oled
	SSD7317_OLED_WriteBuffer(OLEDBuf);
}

void UpdateForSwipe(uint8_t Key)
{
	uint8_t i;
	//blank display & draw 4 div key lines
	UpdateBlankLines();
	//display has been swiped / dragged
	//draw a line down the middle of the display
	memset(OLEDBuf, 0x00, sizeof(OLEDBuf));
	for (i = 10; i < 50; i++)
	{
		OLEDBuf[(i*8)+3] = 0xE0;
		OLEDBuf[(i*8)+4] = 0x07;
	}
	for (i = 54+10+8+4; i < 128-10; i++)
	{
		OLEDBuf[(i*8)+3] = 0xE0;
		OLEDBuf[(i*8)+4] = 0x07;
	}
	//draw swipe text
	PrintString(1,54, "SWIPED");
	if (Key == 1)
		PrintString(2,54+10, "DOWN");
	else
		PrintString(3,54+10, "UP");
	//write buffer to oled
	SSD7317_OLED_WriteBuffer(OLEDBuf);
}

//main program loop
void loop()
{
	//main loop
	Serial.println("loop()");

	uint32_t		DispTimer;
	TouchTypes_t	LastTouchType = TOUCH_NONE;
	uint8_t			LastTouchKey = 0;

	//first display update
	UpdateForNone();

	while(1)
	{
		//this MUST be checked at least every 50mS
		if (SSD7317_TouchData_Waiting == true)
		{
			//flaged for falling touch irq pin
			//handle the touch data
			//this also resets SSD7317_TouchData_Waiting flag var
			SSD7317_Touch_Handle();

			//log the touch to serial
			Serial.print("Touched = type-");
			Serial.print((SSD7317_Gesture_Data >> 8) & 0xFF, 16);
			Serial.print(" key-");
			Serial.println(SSD7317_Gesture_Data & 0xFF, 16);

			//check for a new touch
			if (LastTouchType == TOUCH_NONE)
			{
				//new touch
				LastTouchType = (TouchTypes_t)((SSD7317_Gesture_Data >> 8) & 0xFF);
				LastTouchKey = SSD7317_Gesture_Data & 0xFF;
				//start display timer
				TIMER_SET(DispTimer, 750); //750mS
				//update display with appropriate touch type
				switch (LastTouchType)
				{
					case TOUCH_KEY:
						UpdateForKey(LastTouchKey);
						break;
					case TOUCH_HOLD:
						UpdateForHold(LastTouchKey);
						break;
					case TOUCH_SWIPE:
						UpdateForSwipe(LastTouchKey);
						break;
					case TOUCH_NONE:
					default:
						//shouldnt happen
						LastTouchType = TOUCH_NONE;
						break;
				}
			}
		}

		//check for prev touch & display timer lapsing
		if (LastTouchType != TOUCH_NONE)
		{
			TIMER_IF_EXPIRED(DispTimer)
			{
				//reset touch type/key
				LastTouchType = TOUCH_NONE;
				LastTouchKey = 0;
				//draw untouched normal screen
				UpdateForNone();
			}
		}
	}
}

inline static uint8_t FlipByte(uint8_t c)
{
	c = ((c>>1)&0x55)|((c<<1)&0xAA);
	c = ((c>>2)&0x33)|((c<<2)&0xCC);
	c = (c>>4) | (c<<4);
	return c;
}

void PrintChar(uint8_t x, uint8_t y, uint8_t c)
{
	//print a character to the display buffer
	//aligned on X axis by byte (fast)
	uint8_t	*bufpos;
	uint8_t	i;
	c -= FONT8x8_BASE;
	if (c > FONT8x8_CHARNUM-1)
		return;
	if (x > FONT8x8_WIDTH-1)
		return;
	bufpos = OLEDBuf + x + (y * (SSD7317_OLED_WIDTH / FONT8x8_WIDTH));
	for (i = 0; i < FONT8x8_HEIGHT; i++)
	{
		*bufpos = FlipByte(pgm_read_byte_near(&Font_8x8[(c * FONT8x8_HEIGHT) + i]));
		bufpos += SSD7317_OLED_WIDTH / FONT8x8_WIDTH;
	}
}

void PrintString(uint8_t x, uint8_t y, const char *str)
{
	//render a string to the display buffer
	uint8_t i;
	for (i = 0; i < SSD7317_OLED_WIDTH / FONT8x8_WIDTH; i++)
	{
		if (str[i] == '\0') break; //null char
		PrintChar(x+i, y, (uint8_t)str[i]);
	}
}
