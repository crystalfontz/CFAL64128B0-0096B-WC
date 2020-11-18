//==============================================================================
//
//  CRYSTALFONTZ CFAL64128B0-0096B-WC EXAMPLE FIRMWARE
//
//  OLED DISPLAY SPI INTERFACE FIRMWARE
//
//  Code written for Seeeduino v4.2 set to 3.3v (important!)
//
//  The controller is a Sitronix SSD7317.
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

#include "prefs.h"

#ifdef OLED_SPI

#include <Arduino.h>
#include <SPI.h>
#include "ssd7317_oled_spi.h"

//////////////////////////////////////////////////////////

#define SSD7317_OLED_DC_CMD			(0)
#define SSD7317_OLED_DC_DATA		(1)

static void SSD7317_OLED_Setup();
static void SSD7317_OLED_DISP_SPI4_WR(unsigned char data, unsigned char DC);

//////////////////////////////////////////////////////////

void SSD7317_OLED_Init(void)
{
	//pin setup
	digitalWrite(SSD7317_OLED_SPI_CS, HIGH);
	pinMode(SSD7317_OLED_SPI_CS, OUTPUT);
	digitalWrite(SSD7317_OLED_SPI_DC, HIGH);
	pinMode(SSD7317_OLED_SPI_DC, OUTPUT);
	digitalWrite(SSD7317_OLED_RST, HIGH);
	pinMode(SSD7317_OLED_RST, OUTPUT);

	//spi setup
	SPI.begin();
	SPI.beginTransaction(SPISettings(SSD7317_OLED_SPI_FREQ, MSBFIRST, SPI_MODE0));

	//reset
	digitalWrite(SSD7317_OLED_RST, LOW);
	delay(10);
	digitalWrite(SSD7317_OLED_RST, HIGH);
	delay(10);

	//run setup commands
	SSD7317_OLED_Setup();

	//bank lcd
	SSD7317_OLED_Blank();
}

void SSD7317_OLED_WriteBuffer(uint8_t *buf)
{
	SSD7317_OLED_DISP_SPI4_WR(0x21, SSD7317_OLED_DC_CMD);		//col address
	SSD7317_OLED_DISP_SPI4_WR(0x00, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0x7F, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0x22, SSD7317_OLED_DC_CMD);		//page address
	SSD7317_OLED_DISP_SPI4_WR(0x00, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0x07, SSD7317_OLED_DC_CMD);		//128x64

	//setup D/C# bit
	digitalWrite(SSD7317_OLED_SPI_DC, SSD7317_OLED_DC_DATA);
	//setup CS
	digitalWrite(SSD7317_OLED_SPI_CS, LOW);
	//send data
	SPI.transfer(buf, SSD7317_OLED_HEIGHT * SSD7317_OLED_WIDTH / 8);
	//done
	digitalWrite(SSD7317_OLED_SPI_CS, HIGH);
}

void SSD7317_OLED_Blank(void)
{
	//blank the display
	SSD7317_OLED_DISP_SPI4_WR(0x21, SSD7317_OLED_DC_CMD);		//col address
	SSD7317_OLED_DISP_SPI4_WR(0x00, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0x7F, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0x22, SSD7317_OLED_DC_CMD);		//page address
	SSD7317_OLED_DISP_SPI4_WR(0x00, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0x07, SSD7317_OLED_DC_CMD);		//128x64

	//setup D/C# bit
	digitalWrite(SSD7317_OLED_SPI_DC, SSD7317_OLED_DC_DATA);
	//setup CS
	digitalWrite(SSD7317_OLED_SPI_CS, LOW);
	//send data
	for (uint16_t i = 0; i < SSD7317_OLED_HEIGHT * SSD7317_OLED_WIDTH / 8; i++)
		SPI.transfer(0x00);
	//done
	digitalWrite(SSD7317_OLED_SPI_CS, HIGH);
}

//////////////////////////////////////////////////////////

static void SSD7317_OLED_Setup()
{
	//initialise the oled driver ic
	SSD7317_OLED_DISP_SPI4_WR(0XFD, SSD7317_OLED_DC_CMD); // Command Lock Main
	SSD7317_OLED_DISP_SPI4_WR(0X12, SSD7317_OLED_DC_CMD); // 12H:Unlock 16:Lock

	SSD7317_OLED_DISP_SPI4_WR(0XAE, SSD7317_OLED_DC_CMD); // Display OFF (sleep mode)

	SSD7317_OLED_DISP_SPI4_WR(0XAD, SSD7317_OLED_DC_CMD); // External or internal IREF Selection
	SSD7317_OLED_DISP_SPI4_WR(0X00, SSD7317_OLED_DC_CMD); //10 in , 00 ext

	SSD7317_OLED_DISP_SPI4_WR(0XA8, SSD7317_OLED_DC_CMD); // Set Multiplex Ratio Main
	SSD7317_OLED_DISP_SPI4_WR(0X3f, SSD7317_OLED_DC_CMD); // Set 64 duty -

	SSD7317_OLED_DISP_SPI4_WR(0XD3, SSD7317_OLED_DC_CMD); // Set Display Offset
	SSD7317_OLED_DISP_SPI4_WR(0X10, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0XA2, SSD7317_OLED_DC_CMD); // Set Display Start Line
	SSD7317_OLED_DISP_SPI4_WR(0X00, SSD7317_OLED_DC_CMD); // Start Line 00

	SSD7317_OLED_DISP_SPI4_WR(0XA1, SSD7317_OLED_DC_CMD); // Set Segment Remap

	SSD7317_OLED_DISP_SPI4_WR(0XC8, SSD7317_OLED_DC_CMD); // Set COM Output Scan Direction

	SSD7317_OLED_DISP_SPI4_WR(0XDA, SSD7317_OLED_DC_CMD); //Com Pins Hardware
	SSD7317_OLED_DISP_SPI4_WR(0X12, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0X81, SSD7317_OLED_DC_CMD); // Set Contrast Control
	SSD7317_OLED_DISP_SPI4_WR(0X8f, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0XA4, SSD7317_OLED_DC_CMD); // A4:Display Ram ;A5:Display All Pixel

	SSD7317_OLED_DISP_SPI4_WR(0XA6, SSD7317_OLED_DC_CMD); // A6:0 is Pixel off; A7:0 is Pixel on

	SSD7317_OLED_DISP_SPI4_WR(0XD5, SSD7317_OLED_DC_CMD); // Set Front Clock Divider /Oscillator Frequency
	SSD7317_OLED_DISP_SPI4_WR(0X80, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0XD9, SSD7317_OLED_DC_CMD); // Set Pre-charge Period
	SSD7317_OLED_DISP_SPI4_WR(0X32, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0XDB, SSD7317_OLED_DC_CMD); //Set VcomH
	SSD7317_OLED_DISP_SPI4_WR(0X30, SSD7317_OLED_DC_CMD);

	//==============================================
	SSD7317_OLED_DISP_SPI4_WR(0X31, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0Xd0, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0X34, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0X0f, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0X37, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0X01, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0X36, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0X0f, SSD7317_OLED_DC_CMD);

	SSD7317_OLED_DISP_SPI4_WR(0X35, SSD7317_OLED_DC_CMD);
	SSD7317_OLED_DISP_SPI4_WR(0X0b, SSD7317_OLED_DC_CMD);
	//=============================================

	SSD7317_OLED_DISP_SPI4_WR(0x20, SSD7317_OLED_DC_CMD); //mem adressing mode
	SSD7317_OLED_DISP_SPI4_WR(0x01, SSD7317_OLED_DC_CMD); //com-page h-mode

	SSD7317_OLED_DISP_SPI4_WR(0XAF, SSD7317_OLED_DC_CMD); //display on
}

static void SSD7317_OLED_DISP_SPI4_WR(unsigned char data, unsigned char DC)
{
	//write a byte of data to the oled controller ic
	//setup D/C# bit
	digitalWrite(SSD7317_OLED_SPI_DC, SSD7317_OLED_DC_CMD);
	//setup CS
	digitalWrite(SSD7317_OLED_SPI_CS, LOW);
	//send data (also controls CS)
	SPI.transfer(data);
	//done
	digitalWrite(SSD7317_OLED_SPI_CS, HIGH);
}

#endif
