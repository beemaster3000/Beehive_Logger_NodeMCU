
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include "RTClib.h"
#include <Wire.h>
#include "DHT.h"
#include <SPI.h>
#include <avr/pgmspace.h>

// Local Libs
#include "SdFat.h"
#include "DisplayOLED.hpp"

//===========================================================================================
// ------------------- constant class variable and table definitions start ------------------
//===========================================================================================
// General
#define logging_interval 5         // logging interval (averaging period) in mins
#define sampeling_interval 30      // sampeling period in s (CAN NOT BE LESS THAN (nsensors)+1*2sec)
#define nSensor 11                 // Number of T/RH sensors to sample from
#define subsample_delay 2000       // Delay between each RTC sensors reading in ms
#define BATTVOLT_PIN A0            // Analog pin used to measure battery voltage divider from 12v battery to analog input (R1+R2)/R2
#define BATTVOLT_R1 9.79           // Value of R1 in kohm recorded with voltmeter (9.79 original val)
#define BATTVOLT_R2 0.984          // Value of R2 in kohm recorded with voltmeter (0.981 original val)
float T;                           // Variable containing Temperature (degC) sensor readings each loop
float T_cumulTable[nSensor];       // Table containing cumulative Temperature (degC) sensor readings for averageing interval
float RH;                          // Variable containing Relative Humidity (%) in current loop
float RH_cumulTable[nSensor];      // Table containing Relative Humidity (%) sensor readings for averageing interval
float battVolt;                    // Battery Voltage
float battVolt_cumul;              // Cumulative battery Voltage for averageing interval
short int nSamples;                // Counter for averaging values every logging_interval, stored on SD card
short int recordNumber;            // Current record number

// 74HC595 and Multiplexer
#define MX_SIG_PIN D9  // SIG pin  connected to the DHT sensors
const uint8_t OSRDataPin =  D3;   // connected to 74HC595 SER pin (leg 13 OE)
const uint8_t OSRLatchPin = D4;   // connected to 74HC595 RCLK pin (leg 12 RCLK)
const uint8_t OSRClockPin = D10;  // connected to 74HC595 SRCLK pin (leg 11 SRCLK) (TX pin ok for output)
const int short s0[16] PROGMEM={0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
const int short s1[16] PROGMEM={0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1};
const int short s2[16] PROGMEM={0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};
const int short s3[16] PROGMEM={0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1};

// DHT
// #define DHT_PIN 2     // Digital pin connected to the DHT sensor (same as MX_SIG_PIN)
#define DHT_TYPE DHT11   // DHT 11
DHT dht(MX_SIG_PIN, DHT_TYPE);

// RTC module
RTC_DS3231 rtc;
// const int alarmPin = 8; // The number of the pin for monitoring alarm status on DS3231

// OLED display module
DisplayOLED display;
int short setupLine;         // Line number for setup
#define BUTTON_PIN D0        // the number of the pushbutton pin (RX pin ok for input)
int short buttonState= 0;    // variable for reading the pushbutton status


// SD card
SdFat card;   // File system object.
SdFile file;  // Log file.
#define SD_CS_PIN D8 // SD chip select pin.
#define FILE_BASE_NAME "Data"
char fileName[13] = "Data00.csv";

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
   const uint8_t MX_S0 = 1;
   const uint8_t MX_S1 = 2;
   const uint8_t MX_S2 = 3;
   const uint8_t MX_S3 = 4;
   const uint8_t MX_EN = 0;  // Kept LOW for MX to be ON
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



//------------------------------------------------------------------------------
// Serial print sampeling data
void printSensorData(int MX_Channel, float t, float h)
{
  if (isnan(h) || isnan(t)) 
  {
    Serial.print (F("Sensor Number : "));
    Serial.print(MX_Channel+1);
    Serial.println(F(" Failed to read sensor"));
  }
  else
  {
    Serial.print (F("Sensor Number : "));
    Serial.print(MX_Channel+1);
    Serial.print(F(" Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));
  }
}
//------------------------------------------------------------------------------
// Write data header.
void writeHeader() 
{
  file.print(F("DATETIME,nSamples,recordNumber,battVolt"));
  for (uint8_t i = 0; i < nSensor; i++) 
  {
    file.print(F(",temp_"));
    file.print(i+1);
  }
  for (uint8_t i = 0; i < nSensor; i++) 
  {
    file.print(F(",RH_"));
    file.print(i+1);
  }
  file.println();
  file.print(F("yyyy-mm-dd HH:MM, , ,V"));
    for (uint8_t i = 0; i < nSensor; i++) 
  {
    file.print(F(",deg_C"));
  }
  for (uint8_t i = 0; i < nSensor; i++) 
  {
    file.print(F(",%"));
  }
  file.println();
}
//------------------------------------------------------------------------------
// Write data 
void writeData(DateTime now, int nSamples,int recordNumber,float battVolt_cumul, float T_cumulTable[nSensor], float RH_cumulTable[nSensor]) 
{
  char buff[] = "YYYY-MM-DD hh:mm";
  file.print(now.toString(buff));
  file.print(F(","));
  file.print(nSamples);
  file.print(F(","));
  file.print(recordNumber);
  file.print(F(","));
  file.print(battVolt_cumul/nSamples,1);
  for (uint8_t i = 0; i < nSensor; i++)
  {
    file.print(F(","));
    file.print(T_cumulTable[i]/nSamples,1);
  }
    for (uint8_t i = 0; i < nSensor; i++)
  {
    file.print(F(","));
    file.print(RH_cumulTable[i]/nSamples,1);
  }
  file.println();
}
//------------------------------------------------------------------------------
// dateTime for file siignature
void dateTime(uint16_t* date, uint16_t* time) 
{
  DateTime now = rtc.now();
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{
  //// OLED display module (welcome screen)
  display.setup();
  display.drawWelcomeScreen();
  delay(int(2000));
  display.clear();
  // display.println("test line 1",1);
  // display.println("test line 2",2);
  // display.println("test line 3",3);
  // delay(int(6000));

  pinMode(BUTTON_PIN,INPUT_PULLUP); // initialize the pushbutton pin as an input:

  //// general
  Serial.begin(115200);
  // analogReference(INTERNAL); // kept here for future use with resistances providing no more than 1.1V
  Serial.println(F("Program has started"));

  recordNumber=1;

  //// RTC
  if (! rtc.begin()) 
  {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) 
  {
    Serial.println(F("RTC lost power, lets set the time!"));
	  // Comment out below lines once you set the date & time.
    // Following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //// Disable and clear both alarms
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  DateTime now = rtc.now();
  char buff[] = "YYYY-MM-DD hh:mm:ss";
  Serial.println();   
  Serial.println(now.toString(buff));

  //// Set smpeling alarm (alarm 1)
  if (60-now.second() <= 5)
  {
    rtc.setAlarm1(now + TimeSpan(0,0,1,-now.second()), DS3231_A1_Minute);
    Serial.println(F("Sampeling starting next minute"));
  }else
  {
    int short delay = (60-now.second()) % 5 ;
    if (delay == 0)
    {
      delay +=5;
    }
    rtc.setAlarm1(now + TimeSpan(0,0,0,delay), DS3231_A1_Second);
    Serial.print(F("Sampeling starting in: "));
    Serial.print(delay);
    Serial.println(F(" sec"));
  }

  //// Set logging alarm (alarm 2)
  if (60-now.minute() <= logging_interval)
  {
    rtc.setAlarm2(now + TimeSpan(0,1,-now.minute(),0), DS3231_A2_Hour);
    Serial.println(F("First log next hour"));
  }else
  {
    int short delay = (60-now.minute()) % logging_interval ;
    if (delay == 0)
    {
      delay +=logging_interval;
    }
    rtc.setAlarm2(now + TimeSpan(0,0,delay,0), DS3231_A2_Minute);
    Serial.print(F("First log in: "));
    Serial.print(delay);
    Serial.println(F(" min"));
  }
  
  // // multiplexer / 74HC595 shift register
   pinMode(OSRDataPin, OUTPUT);
   pinMode(OSRLatchPin, OUTPUT);
   pinMode(OSRClockPin, OUTPUT);
   channelControl(-1); // set EN in (enable pin) HIGH to disable chip

  //// TRH sensor
  dht.begin();
  nSamples=0; 

  //// SD Card 
	pinMode(SD_CS_PIN, OUTPUT);
  #define error(msg) card.errorHalt(F(msg))
	// card.begin(SD_CS_PIN, SPI_FULL_SPEED);
  card.begin(SD_CS_PIN, SD_SCK_MHZ(25));
  // char fileName[13] = FILE_BASE_NAME "00.csv";
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  // Find an unused file name
  while (card.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  SdFile::dateTimeCallback(dateTime);
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
  }
  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  writeHeader();
  file.close();
  recordNumber=0;

  // RAM troubleshootiing
  Serial.print(F("kB of free RAM : "));
  Serial.println(ESP.getFreeHeap());
  // Serial.println(freeMemory());
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState==LOW)
  {
    DateTime now = rtc.now();
    char buff[] = "YYYY-MM-DD hh:mm:ss";
    display.printMainPage(fileName,battVolt,now.toString(buff));
  }
  else{
    display.clear();
  }
  

  if (rtc.alarmFired(2) == true)
  {
    // Reset Logging and sampeling alarms
    DateTime now = rtc.now();
    rtc.clearAlarm(2); 
    if (60-now.minute() <= logging_interval)
    {
      rtc.setAlarm2(now + TimeSpan(0,1,-now.minute(),0), DS3231_A2_Hour);
      Serial.println();
      Serial.println(F("Logging data, next log next hour"));
    }else
    {
      rtc.setAlarm2(now + TimeSpan(0,0,logging_interval,0), DS3231_A2_Minute);
      Serial.println();
      Serial.print(F("Logging data, next log in: "));
      Serial.print(logging_interval);
      Serial.println(F(" min"));
    }
    file.open(fileName, FILE_WRITE);
    writeData(now,nSamples,recordNumber,battVolt_cumul,T_cumulTable,RH_cumulTable); 
    file.close();
    // Reset sample counter and averaging vectors
    nSamples=0;
    recordNumber++;
    for(int short i=0; i<nSensor; i++)
    {
      T_cumulTable[i]=  0;
      RH_cumulTable[i]= 0;
    }
  } 
  else 
  if (rtc.alarmFired(1) == true)
  {
    DateTime now = rtc.now();
    char buff[] = "YYYY-MM-DD hh:mm:ss";
    Serial.println();   
    Serial.println(now.toString(buff));
    
    // Reset alarm for next sample
    rtc.clearAlarm(1); 
    rtc.setAlarm1(now + TimeSpan(0,0,0,sampeling_interval), DS3231_A1_Second);

    // Multiplexer activation and sensor reading
    for(int short i=0; i<nSensor; i++)
    {
      channelControl(i);
      // Measure sensors
      T=dht.readTemperature();
      RH=dht.readHumidity();

      printSensorData(i, T, RH);

      // Display data on OLED screen
      buttonState = digitalRead(BUTTON_PIN);
      if (buttonState==LOW)
      {
        display.printTRHvalues(i+1,T,RH);
      }
      else{
        display.clear();
      }
      

      // Add values to cumulative arrays
      T_cumulTable[i]=T_cumulTable[i]+T;
      RH_cumulTable[i]=RH_cumulTable[i]+RH;

      delay(subsample_delay); // wait for loopDelay ms
    }
    battVolt=analogRead(BATTVOLT_PIN)*((BATTVOLT_R1+BATTVOLT_R2)/BATTVOLT_R2)*(5.0/1023);
    battVolt_cumul=battVolt_cumul+battVolt;
    nSamples++;
    // display.clear();

    channelControl(-1);  // disable mx chip
  }
}
//===========================================================================================
// ------------------------------- Backup code start ----------------------------------------
//===========================================================================================

// if (!file.open(fileName, FILE_WRITE) )
// {
// error("file.open");
// }
// // file.open(fileName, FILE_WRITE);
// writeData(now,nSamples,battVolt_cumul,T_cumulTable,RH_cumulTable); 
// file.close();

// Serial.print(kB of free RAM : );
// Serial.println(freeMemory());

// const int short MX_CTRL_PIN[4] = {3, 4, 5, 6}; // 4 pins used  for binary output
// bool MX_Table[16][4] = 
// {
//   // s0, s1, s2, s3     channel
//     {0,  0,  0,  0}, // 0
//     {1,  0,  0,  0}, // 1
//     {0,  1,  0,  0}, // 2
//     {1,  1,  0,  0}, // 3
//     {0,  0,  1,  0}, // 4
//     {1,  0,  1,  0}, // 5
//     {0,  1,  1,  0}, // 6
//     {1,  1,  1,  0}, // 7
//     {0,  0,  0,  1}, // 8
//     {1,  0,  0,  1}, // 9
//     {0,  1,  0,  1}, // 10
//     {1,  1,  0,  1}, // 11
//     {0,  0,  1,  1}, // 12
//     {1,  0,  1,  1}, // 13
//     {0,  1,  1,  1}, // 14
//     {1,  1,  1,  1}  // 15
// };
// Serial.print  (MX_Table[MX_Channel][0]);
// Serial.print  (MX_Table[MX_Channel][1]);
// Serial.print  (MX_Table[MX_Channel][2]);
// Serial.println(MX_Table[MX_Channel][3]);
// digitalWrite(MX_CTRL_PIN[0], MX_Table[MX_Channel][0]);
// digitalWrite(MX_CTRL_PIN[1], MX_Table[MX_Channel][1]);
// digitalWrite(MX_CTRL_PIN[2], MX_Table[MX_Channel][2]);
// digitalWrite(MX_CTRL_PIN[3], MX_Table[MX_Channel][3]);
  // for(int short i=0; i<4; i++)
  // {
  //   pinMode(MX_CTRL_PIN[i], OUTPUT);      // set pin as output
  //   digitalWrite(MX_CTRL_PIN[i], HIGH);   // set initial state as HIGH     
  // }