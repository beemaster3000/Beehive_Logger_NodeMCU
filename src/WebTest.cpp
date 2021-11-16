
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

#define SD_CS_PIN D8 // SD chip select pin.
#define error(msg) card.errorHalt(F(msg))
#define FILE_BASE_NAME "Data"
char fileName[13] = "Data00.csv";
char fileList[500]; //No specific size for now

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

void handleRoot() 
{
    // Open next file in root.
  // Warning, openNext starts at the current directory position
  // so a rewind of the directory may be required.
  // memset(fileList, '\0', sizeof(fileList));
  // char loop_Buffer[20];
  // while (file.openNext(&root, O_RDONLY)) 
  // {
  //   file.getName(loop_Buffer, sizeof(loop_Buffer));
  //   strcat(fileList,loop_Buffer);
  //   strcat(fileList,"<br>" );
  //   file.close();
  // }
  char charBuffer[1024];
  pageDisplayCounter++;
  sprintf(charBuffer,htmlPage1,fileList,pageDisplayCounter);
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

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  pinMode(SD_CS_PIN, OUTPUT);
  if (!card.begin(SD_CS_PIN, SD_SCK_MHZ(25))) 
  {
    card.initErrorHalt();
  }
    if (!root.open("/")) 
  {
    card.errorHalt("open root failed");
  }
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  // Find an unused file name
  while (card.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') 
    {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') 
    {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else 
    {
      error("Can't create file name");
    }
  }
  int fileNumebr;
  fileNumebr=atoi(&(fileName[4]));

  // char loop_Buffer[13];
  // while (file.openNext(&root, O_RDONLY)) 
  // {
  //   file.getName(loop_Buffer, sizeof(loop_Buffer));
  //   strcat(fileList,loop_Buffer);
  //   strcat(fileList,"<br>" );
  //   file.close();
  // }
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
 server.handleClient();
}
