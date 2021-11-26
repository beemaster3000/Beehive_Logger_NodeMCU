
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <DHT.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <MCP23017.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SD.h> 

// Local Libs
#include "DisplayOLED.hpp"
#include "page1.h"
#include "page2.h"
#include "page3.h"

//===========================================================================================
// ------------------- constant class variable and table definitions start ------------------
//===========================================================================================
// General
#define logging_interval 5         // logging interval (averaging period) in mins
#define sampeling_interval 30      // sampeling period in s (CAN NOT BE LESS THAN (nsensors)+1*2sec)
#define nSensor 13                 // Number of T/RH sensors to sample from
#define subsample_delay 2000       // Delay between each RTC sensors reading in ms
#define BATTVOLT_PIN A0            // Analog pin used to measure battery voltage divider from 12v battery to analog input (R1+R2)/R2
#define BATTVOLT_R1 14.29           // Value of R1 in kohm recorded with voltmeter (9.79 original val)
#define BATTVOLT_R2 1.479          // Value of R2 in kohm recorded with voltmeter (0.981 original val)
float T;                           // Variable containing Temperature (degC) sensor readings each loop
float T_cumulTable[nSensor];       // Table containing cumulative Temperature (degC) sensor readings for averageing interval
float RH;                          // Variable containing Relative Humidity (%) in current loop
float RH_cumulTable[nSensor];      // Table containing Relative Humidity (%) sensor readings for averageing interval
float battVolt;                    // Battery Voltage
float battVolt_cumul;              // Cumulative battery Voltage for averageing interval
short int nSamples;                // Counter for averaging values every logging_interval, stored on SD card
short int recordNumber;            // Current record number

// Multiplexer 
//note: could these constants be shifted to the functions that use them to save RAM?)
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

// DHT
// #define DHT_PIN D2     // Digital pin connected to the DHT sensor (same as MX_SIG_PIN)
#define DHT_TYPE DHT11   // DHT 11
DHT dht(MX_SIG_PIN, DHT_TYPE);

// RTC module
RTC_DS3231 rtc;
// const int alarmPin = 8; // The number of the pin for monitoring alarm status on DS3231

// OLED display module
DisplayOLED display;
#define BUTTON_PIN 8        // the number of the pushbutton pin, GPB0 (1) of the MCP23017A Enable pin 
int short buttonState= 0;   // variable for reading the pushbutton status

// SD card
File file;
#define SD_CS_PIN D8 // SD chip select pin.
#define FILE_BASE_NAME "Data"
char fileName[11] = "Data00.csv";
char fileNameWebServer[15]="/";
char HTMLtableBuffer[1024];

// WebServer
const char *ssid = "Beemaster_3000";
const char *password = "12345678";
IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);
int short pageDisplayCounter;
int short filesInCard;

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
// dateTime for file signature
void dateTime(uint16_t* date, uint16_t* time) 
{
  DateTime now = rtc.now();
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

//------------------------------------------------------------------------------
// WebServer
void getFileList(File dir) 
{
  memset(HTMLtableBuffer, '\0', sizeof(HTMLtableBuffer));
  filesInCard=0;
  dir.rewindDirectory();
   while(true) 
   {
    File entry = dir.openNextFile();
    if (! entry) 
    {
      break;
    }
    if(filesInCard==0)
      {
        filesInCard++;
        continue;
      }
      // get file information
      char BuffSize[10];
      itoa (entry.size(),BuffSize,10);
      char buffTime[17];
      struct tm ts;
      time_t epochTimeCreate = entry.getCreationTime();
      time_t epochTimeWrite = entry.getLastWrite();
      // create file list

      // strcat(HTMLtableBuffer,"<tr><td><a href=/" );
      // strcat(HTMLtableBuffer,entry.name());
      // strcat(HTMLtableBuffer,">" );
      
      strcat(HTMLtableBuffer,"<tr><td><a href=""/GetData"">");
      strcat(HTMLtableBuffer,entry.name());
      strcat(HTMLtableBuffer,"</a></td><td>");
      strcat(HTMLtableBuffer,BuffSize);
      strcat(HTMLtableBuffer,"</td><td>");
      ts = *localtime(&epochTimeCreate);
      strftime(buffTime, sizeof(buffTime), "%Y-%m-%d %H:%M", &ts);
      strcat(HTMLtableBuffer,buffTime);
      strcat(HTMLtableBuffer,"</td><td>");
      ts = *localtime(&epochTimeWrite);
      strftime(buffTime, sizeof(buffTime), "%Y-%m-%d %H:%M", &ts);
      strcat(HTMLtableBuffer,buffTime);
      strcat(HTMLtableBuffer,"</td></tr>");
      //// print to terminal for debug
      // Serial.print(entry.name());
      // Serial.print("\t");
      // Serial.print(BuffSize);
      // Serial.print("\t");
      // Serial.println(buffTime);
   }
}

void handleNotFound()
{
  server.send(404,"text/html",htmlPage2);
}
void handleRoot() 
{
  File root;
  root = SD.open("/");
  getFileList(root);
  root.close();
  char charBuffer[2048];
  pageDisplayCounter++;
  sprintf(charBuffer,htmlPage1,fileName,fileName,HTMLtableBuffer,pageDisplayCounter);
  server.send(200, "text/html", charBuffer);
}

void handleDataPage() 
{
  File fileToStream;
  fileToStream = SD.open(fileName, FILE_READ);
  int SDfsize  = fileToStream.size();
  server.sendHeader("Content-Length",(String)(SDfsize));
  server.sendHeader("Cache-Control","max-age=2628000,public");
  server.streamFile(fileToStream,"text/csv");
  fileToStream.close();
  delay(100);
}

void createStreamData()
{
  handleNotFound();
}

void handleLivePage()
{
  char charBuffer[2048];
  memset(charBuffer,  '\0', sizeof(charBuffer));
  memset(HTMLtableBuffer, '\0', sizeof(HTMLtableBuffer));
  char numBuff[4];
  for (uint8_t k = 0; k < nSensor; k++)
  {
    strcat(HTMLtableBuffer,"<tr><td>");
    itoa (k+1,numBuff,10);
    strcat(HTMLtableBuffer,numBuff);
    strcat(HTMLtableBuffer,"</td><td>");
    if (T_cumulTable[k])
    {
      dtostrf(T_cumulTable[k]/nSamples,4,1,numBuff);
      strcat(HTMLtableBuffer,numBuff);
    }else
    {
      strcat(HTMLtableBuffer," NaN ");
    }
    strcat(HTMLtableBuffer,"</td><td>");
    if (RH_cumulTable[k])
    {
      dtostrf(RH_cumulTable[k]/nSamples,4,1,numBuff);
      strcat(HTMLtableBuffer,numBuff);
    }else
    {
      strcat(HTMLtableBuffer," NaN ");
    }
    strcat(HTMLtableBuffer,"</td></tr>");
  }
  sprintf(charBuffer,htmlPage3,HTMLtableBuffer);
  server.send(200, "text/html", charBuffer);
}

//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{
    //// general
  Serial.begin(115200);
  Wire.begin();     // initialize I2C serial bus
  // analogReference(INTERNAL); // kept here for future use with resistances providing no more than 1.1V
  Serial.println(F("Program has started"));
  recordNumber=1;
  pageDisplayCounter=0;


  //// OLED display module (welcome screen)
  display.setup();
  display.drawWelcomeScreen();
  delay(int(1500));

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
    // rtc.adjust(DateTime(2021, 11, 22, 22, 50, 0));
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
  
  //// multiplexer
  configuremcp23017Pins(); 
  mcp23017.digitalWrite(MX_EN_PIN, HIGH); // Disable mx chip

  //// TRH sensor
  dht.begin();
  nSamples=0; 

 //// SD Card 
  
  
  Serial.println();
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) 
  {
    Serial.println("Card failed, or not present");
    exit(0);
  }
  Serial.println("card initialized.");

  sdfat::FsDateTime::setCallback(dateTime);
  // SdFile::dateTimeCallback(dateTime);
  
  if (!SD.exists(fileName)){
  file = SD.open(fileName,FILE_WRITE );
  writeHeader();  
  }else{
  file = SD.open(fileName,FILE_WRITE );
  }
  // Find an unused file name if base name is inaccessible
  if(!file)
  {
    const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
    while(!file)
    { 
      Serial.print("Failed to open ");
      Serial.println(fileName);

      if (fileName[BASE_NAME_SIZE + 1] != '9') {
        fileName[BASE_NAME_SIZE + 1]++;
      } else if (fileName[BASE_NAME_SIZE] != '9') {
        fileName[BASE_NAME_SIZE + 1] = '0';
        fileName[BASE_NAME_SIZE]++;
      } else if (fileName[BASE_NAME_SIZE + 1]=='9'  &&  fileName[BASE_NAME_SIZE]=='9'){
        Serial.println("Can't create or open file");
        exit(0);
      }
      file = SD.open(fileName,FILE_WRITE );
    }
    writeHeader();
  }
  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  file.close();
  recordNumber=0;

   //// Web Server
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  strcat(fileNameWebServer,fileName);  
  
  server.on("/", handleRoot);
  server.on(fileNameWebServer, handleDataPage);
  server.on("/GetData", createStreamData);
  server.on("/GetLiveData",handleLivePage);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Server listening"); 

  // RAM troubleshootiing
  Serial.print(F("kB of free RAM : "));
  Serial.println(ESP.getFreeHeap());
  // Serial.println(freeMemory());
  
  display.clear();
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
  // handle server client
  server.handleClient();

  // Screen On Off subroutine (Main Screen)
  buttonState = mcp23017.digitalRead(BUTTON_PIN);
  delay(10);

  if (buttonState==LOW)
  {
    DateTime now = rtc.now();
    char buff[] = "YYYY-MM-DD hh:mm:ss";
    display.printMainPage(fileName,battVolt,now.toString(buff));
  }
  else{
    display.clear();
  }
  
  // Sample and log alarm control
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
    file = SD.open(fileName,FILE_WRITE);
    writeData(now,nSamples,recordNumber,battVolt_cumul,T_cumulTable,RH_cumulTable); 
    file.close();
    // Reset sample counter and averaging vectors
    nSamples=0;
    battVolt_cumul= 0;
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

      // Switch Chanel to i
      channelControl(i);
      
      // Enable mx chip
      mcp23017.digitalWrite(MX_EN_PIN, LOW); 

      // Measure sensors
      T=dht.readTemperature();
      RH=dht.readHumidity();

      // Send data to terminal
      printSensorData(i, T, RH);

      // Display data on OLED screen
      buttonState = mcp23017.digitalRead(BUTTON_PIN);
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
    // ADC Voltage range in ESP8266 development boards: 0 to 3.3V
    // The ADC pin has a 10-bit resolution, which means you’ll get values between 0 and 1023.
    // Vout=Vin*(R2/(R1+R2)) -> Vin=Vout*(R1+R2)/R2 * (3.3/1023)
    battVolt=analogRead(BATTVOLT_PIN)*((BATTVOLT_R1+BATTVOLT_R2)/BATTVOLT_R2)*(3.3/1023);
    battVolt_cumul=battVolt_cumul+battVolt;
    nSamples++;
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


//  //// SD Card 
//   sdfat::FsDateTime::setCallback(dateTime);
//   // SdFile::dateTimeCallback(dateTime);
//   Serial.println();
//   Serial.print("Initializing SD card...");
//   if (!SD.begin(SD_CS_PIN)) 
//   {
//     Serial.println("Card failed, or not present");
//     exit(0);
//   }
//   Serial.println("card initialized.");
  
//   if (!SD.exists(fileName)){
//   file = SD.open(fileName,FILE_WRITE );
//   writeHeader();  
//   }else{
//   file = SD.open(fileName,FILE_WRITE );
//   }
//   // Find an unused file name if base name is inaccessible
//   if(!file)
//   {
//     const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
//     while(!file)
//     { 
//       Serial.print("Failed to open ");
//       Serial.println(fileName);

//       if (fileName[BASE_NAME_SIZE + 1] != '9') {
//         fileName[BASE_NAME_SIZE + 1]++;
//       } else if (fileName[BASE_NAME_SIZE] != '9') {
//         fileName[BASE_NAME_SIZE + 1] = '0';
//         fileName[BASE_NAME_SIZE]++;
//       } else if (fileName[BASE_NAME_SIZE + 1]=='9'  &&  fileName[BASE_NAME_SIZE]=='9'){
//         Serial.println("Can't create or open file");
//         exit(0);
//       }
//       file = SD.open(fileName,FILE_WRITE );
//     }
//     writeHeader();
//   }
//   Serial.print(F("Logging to: "));
//   Serial.println(fileName);
//   // writeHeader();
//   file.close();
//   recordNumber=0;


