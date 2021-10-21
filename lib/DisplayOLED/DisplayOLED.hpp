
#pragma once

#include "U8g2lib.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

class DisplayOLED
{
public:
	DisplayOLED();
	~DisplayOLED();

	void clear();
	void setup();
	void drawWelcomeScreen();
	void printTRHvalues(int short sensorNumber, float t, float h);
	void printMainPage(char filename[13],float battVolt,char dateTime[20]);

private:
	U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2_ = U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
	// U8X8_SSD1306_128X64_NONAME_HW_I2C u8g8_ = U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
};

