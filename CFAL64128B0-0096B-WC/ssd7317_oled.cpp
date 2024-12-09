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
#include <Arduino.h>

#ifdef OLED_SPI
#include <SPI.h>
#define SSD7317_OLED_DC_CMD			(0)
#define SSD7317_OLED_DC_DATA		(1)
#endif
#ifdef OLED_I2C
#include <Wire.h>
#endif

#include "ssd7317_oled.h"

//////////////////////////////////////////////////////////


static void SSD7317_OLED_Setup();

//TODO Can this be pushed into the .h file?
#ifdef OLED_SPI
static void SSD7317_OLED_DISP_SPI4_WR(unsigned char data, unsigned char DC);
#endif
#ifdef OLED_I2C
static void SSD7317_OLED_WR_CMD(unsigned char command);
#endif

//////////////////////////////////////////////////////////

void SSD7317_OLED_Init(void)
{
	Serial.print("SSD7317_OLED_Init()");
#ifdef OLED_SPI
	Serial.println(" SPI");

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

#endif
#ifdef OLED_I2C
	Serial.println(" I2C");

	//pin setup
	digitalWrite(SSD7317_OLED_RST, HIGH); //reset pin
	pinMode(SSD7317_OLED_RST, OUTPUT);

	//reset
	digitalWrite(SSD7317_OLED_RST, LOW);
	delay(10);
	digitalWrite(SSD7317_OLED_RST, HIGH);
	delay(10);

	//I2C init
	Wire.begin();

#endif
	//run setup commands
	SSD7317_OLED_Setup();

	//bank lcd
	SSD7317_OLED_Blank();
}

void SSD7317_OLED_WriteBuffer(uint8_t *buf)
{
#ifdef OLED_SPI
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
#endif
#ifdef OLED_I2C
	#define SEGS 16
	uint16_t i, j;
	for (i = 0; i < ( SSD7317_OLED_HEIGHT * SSD7317_OLED_WIDTH / 8) / SEGS ; i++)
	{
		//we have to break this up into lots of SEGS due to Arduino I2C limitations
		j = i*SEGS;
		SSD7317_OLED_WR_CMD(0x21);		//col address
		SSD7317_OLED_WR_CMD(j / 8);
		SSD7317_OLED_WR_CMD(0x7F);

		SSD7317_OLED_WR_CMD(0x22);		//page address
		SSD7317_OLED_WR_CMD(j % 8);
		SSD7317_OLED_WR_CMD(0x07);		//128x64

		Wire.beginTransmission(SSD7317_OLED_I2C_ADDR);
		Wire.write(0x40); //control byte, data bit set
		Wire.write(&buf[j], SEGS);
		Wire.endTransmission();
	}
#endif
}

void SSD7317_OLED_Blank(void)
{
#ifdef OLED_SPI
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
#endif

#ifdef OLED_I2C
	//blank the display
	SSD7317_OLED_WR_CMD(0x21);		//col address
	SSD7317_OLED_WR_CMD(0x00);
	SSD7317_OLED_WR_CMD(0x7F);

	SSD7317_OLED_WR_CMD(0x22);		//page address
	SSD7317_OLED_WR_CMD(0x00);
	SSD7317_OLED_WR_CMD(0x07);		//128x64

	Wire.beginTransmission(SSD7317_OLED_I2C_ADDR);
	Wire.write(0x40); //control byte, data bit set
	for (uint16_t i = 0; i < SSD7317_OLED_HEIGHT * SSD7317_OLED_WIDTH / 8; i++)
		Wire.write(0x00);
	Wire.endTransmission();
#endif
}

//////////////////////////////////////////////////////////

//oled manufacturer supplied init commands
const uint8_t SSD7317_128x64_Init[] =
{
	0xFD, 0x12,	// Set command unlock
	0xAE,		// Set display off
	0xD5, 0x80,	// Set display clock divide ration/oscillator frequency
	0x20, 0x01, // Set memory addressing mode
	0xDA, 0x12,	// Set COM pins hardware configuration
	0x81, 0xFF,	// Set contrast control (brightness)
	0xAD, 0x10, // Set internal IREF enable
	0xA0,		// Set segment remap
	0xC8,		// Set COM output scan direction
	0xA2, 0x00,	// Set display start line
	0xD3, 0x10,	// Set display offset
	0xD9, 0x32,	// Set discharge / pre-charge period
	0xDB, 0x30, // Set VCOM deselect level
	0xA8, 0x3F,	// Set multiplex ratio
	0xA4,		// Set entire display off
	0xA6,		// Set normal display
	0xD5, 0x80,	// Touch function set 1
	0x31, 0xD0,	// Touch function set 2
	0x34, 0x08, // Touch function set 3
	0x37, 0x01,	// Touch function set 4
	0x36, 0x0F, // Touch function set 5
#if (SSD7317_TOUCH_I2C_ADDR == 0x53)
	0x35, 0x0A, // Touch function set 6
#elif (SSD7317_TOUCH_I2C_ADDR == 0x5B)
	0x35, 0x0B, // Touch function set 6
#endif
	0xA1, 		// mirror along x axis
	0xAF		// Set display ON
};

static void SSD7317_OLED_Setup(void)
{
	//initialise the oled driver ic
	//send the init commands
	for (uint8_t i = 0; i < sizeof(SSD7317_128x64_Init); i++)
	{
#ifdef OLED_SPI
		SSD7317_OLED_DISP_SPI4_WR(SSD7317_128x64_Init[i], SSD7317_OLED_DC_CMD);
#endif
#ifdef OLED_I2C
		SSD7317_OLED_WR_CMD(SSD7317_128x64_Init[i]);
#endif
	}
}

#ifdef OLED_SPI
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
#ifdef OLED_I2C
static void SSD7317_OLED_WR_CMD(unsigned char command)
{
	//send command
	uint8_t data[2];
	data[0] = 0x00; // control byte - 0 << 6 for command
	data[1] = command;
	Wire.beginTransmission(SSD7317_OLED_I2C_ADDR);
	Wire.write(data, 2);
	Wire.endTransmission();
}
#endif
