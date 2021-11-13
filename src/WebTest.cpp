
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SdFat.h>

#include "page1.h"
#include "page2.h"


const char *ssid = "Beemaster_3000";
const char *password = "12345678";

ESP8266WebServer server(80);
int short pageDisplayCounter;

// SD card
SdFat card;   // File system object.
SdFile file;  // Log file.
SdFile root;

#define SD_CS_PIN D8 // SD chip select pin.
#define error(msg) card.errorHalt(F(msg))
#define FILE_BASE_NAME "Data"
char fileName[13];

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

void handleRoot() 
{
  char charBuffer[1024];
  pageDisplayCounter++;
  sprintf(charBuffer,htmlPage1,pageDisplayCounter);
  server.send(200, "text/html", charBuffer);
}

void handlePage2() 
{
  server.send(200, "text/html", htmlPage2);
}

//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{  
 Serial.begin(115200);
 pageDisplayCounter=0;
  
  //// Web Server
  // pageDisplayCounter=0;
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  server.on("/", handleRoot);
  server.on("/page2", handlePage2);
  server.begin();
 
  Serial.println("Server listening"); 

  //// SD Card 

// Wait for USB Serial 
  while (!Serial) 
  {
    SysCall::yield();
  }
  
  Serial.println("Type any character to start");
  while (!Serial.available()) 
  {
    SysCall::yield();
  }

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!card.begin(SD_CS_PIN, SD_SCK_MHZ(25))) 
  {
    card.initErrorHalt();
  }
    if (!root.open("/")) 
  {
    card.errorHalt("open root failed");
  }
  // Open next file in root.
  // Warning, openNext starts at the current directory position
  // so a rewind of the directory may be required.
  String fullString;
  String charBuffer;
  while (file.openNext(&root, O_RDONLY)) 
  {
    // charBuffer file.printFileSize();
    // fullString += charBuffer;
    fullString += ' ';
    // charBuffer file.printModifyDateTime();
    // fullString += charBuffer;
    fullString += ' ';
    // charBuffer file.printName();
    // fullString += charBuffer;
    fullString += ' ';
    fullString += '\n';
    file.close();
  }
  {
    Serial.println(fullString);
  }

}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
 server.handleClient();
}
