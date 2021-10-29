
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>

// 74HC595 and Multiplexer
#define MX_SIG_PIN D0  // SIG pin  connected to the DHT sensors
const uint8_t OSRDataPin =  D3;   // connected to 74HC595 SER pin
const uint8_t OSRLatchPin = D4;   // connected to 74HC595 RCLK pin
const uint8_t OSRClockPin = D10;  // connected to 74HC595 SRCLK pin (TX pin ok for output)
const int short s0[16] PROGMEM={0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
const int short s1[16] PROGMEM={0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1};
const int short s2[16] PROGMEM={0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};
const int short s3[16] PROGMEM={0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1};

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

//------------------------------------------------------------------------------
// MX channel selection
void osrWriteRegister(uint8_t outputs) 
{
   // Initiate latching process, next HIGH latches data
   digitalWrite(OSRLatchPin, LOW);
   // Shift output data into the shift register, most significant bit first
   shiftOut(OSRDataPin, OSRClockPin, MSBFIRST, outputs);
   // Latch outputs into the shift register
   digitalWrite(OSRLatchPin, HIGH);
}

void osrDigitalWrite(uint8_t pin, uint8_t value) 
{
   static uint8_t outputs = 0;  // retains shift register output values
   if (value == HIGH) bitSet(outputs, pin);  // set output pin to HIGH
   else if (value == LOW) bitClear(outputs, pin);  // set output pin to LOW
   osrWriteRegister(outputs);  // write all outputs to shift register
}

void channelControl(int MX_Channel)
{
   const uint8_t MX_S0 = 0;
   const uint8_t MX_S1 = 1;
   const uint8_t MX_S2 = 2;
   const uint8_t MX_S3 = 3;
   const uint8_t MX_EN = 4;  // Kept LOW for MX to be ON
   uint8_t outputs     = 0;  // holds shift register output values

  if (pgm_read_word_near(s0 + MX_Channel)==1)
  {
    bitSet(outputs, MX_S0);
  }
  if (pgm_read_word_near(s1 + MX_Channel)==1)
  {
    bitSet(outputs, MX_S1);
  }
  if (pgm_read_word_near(s2 + MX_Channel)==1)
  {
    bitSet(outputs, MX_S2);
  }  
  if (pgm_read_word_near(s3 + MX_Channel)==1)
  {
    bitSet(outputs, MX_S3);
  }
  if (MX_Channel == -1) // if input is -1 set MX_EN to HIGH to turn off MX
  {
    bitSet(outputs, MX_EN);
  }
  osrWriteRegister(outputs); //send new instructions to 74HC595
}

//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{  
  //// multiplexer / 74HC595 shift register
   pinMode(OSRDataPin, OUTPUT);
   pinMode(OSRLatchPin, OUTPUT);
   pinMode(OSRClockPin, OUTPUT);
   channelControl(-1); // set EN in (enable pin) HIGH to disable chip
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
  for(int short i=0; i<7; i++)
  {
    channelControl(i);
    delay(500);
  }
  channelControl(-1);  // disable mx chip
}
