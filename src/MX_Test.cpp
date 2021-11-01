
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <MCP23017.h>

// Multiplexer
#define MX_SIG_PIN D4  // SIG pin  connected to the DHT sensors
const int short s0[16] PROGMEM={0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
const int short s1[16] PROGMEM={0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1};
const int short s2[16] PROGMEM={0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};
const int short s3[16] PROGMEM={0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1};

#define MCP23017_I2C_ADDRESS 0x20     // I2C address of the MCP23017 IC
const uint8_t MX_CTRL_PIN_0 = 0;      // GPA0 (21) of the MCP23017
const uint8_t MX_CTRL_PIN_1 = 1;      // GPA1 (22) of the MCP23017
const uint8_t MX_CTRL_PIN_2 = 2;      // GPA2 (23) of the MCP23017
const uint8_t MX_CTRL_PIN_3 = 3;      // GPA3 (24) of the MCP23017
const uint8_t MX_EN_PIN = 4;          // GPA4 (25) of the MCP23017A Enable pin 
MCP23017 mcp23017 = MCP23017(MCP23017_I2C_ADDRESS);  // instance of the connected MCP23017 IC

#define BUTTON_PIN 7        // the number of the pushbutton pin, GPA4 (28) of the MCP23017A Enable pin 
short int delayms;          // delay between MX port switches
int short buttonState= 0;   // variable for reading the pushbutton status

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

//------------------------------------------------------------------------------
// MX channel selection

void configuremcp23017Pins() 
{
   // Configure output pins
   mcp23017.pinMode(MX_CTRL_PIN_0, OUTPUT);
   mcp23017.pinMode(MX_CTRL_PIN_1, OUTPUT);
   mcp23017.pinMode(MX_CTRL_PIN_2, OUTPUT);
   mcp23017.pinMode(MX_CTRL_PIN_3, OUTPUT);
   mcp23017.pinMode(MX_EN_PIN, OUTPUT);
   mcp23017.pinMode(BUTTON_PIN, INPUT_PULLUP, true);   // initialize the pushbutton pin as an input
}

void channelControl(int MX_Channel)
{
    mcp23017.digitalWrite(MX_CTRL_PIN_0, pgm_read_word_near(s0 + MX_Channel));
    mcp23017.digitalWrite(MX_CTRL_PIN_1, pgm_read_word_near(s1 + MX_Channel));
    mcp23017.digitalWrite(MX_CTRL_PIN_2, pgm_read_word_near(s2 + MX_Channel));
    mcp23017.digitalWrite(MX_CTRL_PIN_3, pgm_read_word_near(s3 + MX_Channel));
}
//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{  
  //// multiplexer
  configuremcp23017Pins(); 
  mcp23017.digitalWrite(MX_EN_PIN, LOW); // enable mx chip
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
  buttonState = mcp23017.digitalRead(BUTTON_PIN);
  if (buttonState==LOW)
  {
    delayms = 50;
  }else
  {
    delayms = 500;
  }

  for(int short i=0; i<16; i++)
  {
    channelControl(i);
    delay(delayms);
  }
}
