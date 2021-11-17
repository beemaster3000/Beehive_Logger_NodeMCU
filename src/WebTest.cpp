
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h> 

#include "page1.h"
#include "page2.h"


const char *ssid = "Beemaster_3000";
const char *password = "12345678";

ESP8266WebServer server(80);
int short pageDisplayCounter;

// SD card
File file;
#define SD_CS_PIN D8 // SD chip select pin.
#define FILE_BASE_NAME "Data"
char fileName[11] = "Data04.csv";
// char fileName[13] = "TESTFILE.txt";
char fileList[500];



//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

void openReadFile()
{
  file = SD.open(fileName, FILE_READ);
}

void openWriteFile()
{
  file = SD.open(fileName, FILE_WRITE);
}

void handleRoot() 
{
  char charBuffer[1024];
  pageDisplayCounter++;
  sprintf(charBuffer,htmlPage1,fileList,pageDisplayCounter);
  server.send(200, "text/html", charBuffer);
}

void handleDataPage() 
{
  openReadFile();
  int SDfsize  = file.size();
  server.sendHeader("Content-Length",(String)(SDfsize));
  server.sendHeader("Cache-Control","max-age=2628000,public");
  size_t fsizeSent = server.streamFile(file,"text/csv");

  Serial.print("File Size: ");
  Serial.println(SDfsize);
  Serial.print("File Size sent: ");
  Serial.println(fsizeSent);

  file.close();
  delay(100);
}


  


//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{  
 Serial.begin(115200);
 pageDisplayCounter=0;

  //// SD Card Initialization
  Serial.println();
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) 
  {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");



  //// Web Server
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  server.on("/", handleRoot);
  server.on("/data", handleDataPage);
  server.begin();
 
  Serial.println("Server listening"); 

 
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
 server.handleClient();
}

//===========================================================================================
// ------------------------------- Backup code start ----------------------------------------
//===========================================================================================



  // char fileList[(fileNumber+10)*sizeof(fileName)];
  // char loop_Buffer[11];
  // memset(loop_Buffer, '\0', sizeof(loop_Buffer));
  // memset(fileList, '\0', sizeof(fileList));
  // for (uint8_t i = 0; i < fileNumber; i++)
  // {
  //   strncpy(loop_Buffer, fileName, 11);
  //   strcat(fileList,loop_Buffer);
  //   strcat(fileList," <br>" );
  // }


  // fileNumber=atoi(&(fileName[4]));



  //   //// SD Card 
  // // Initialize at the highest speed supported by the board that is
  // // not over 50 MHz. Try a lower speed if SPI errors occur.
  // pinMode(SD_CS_PIN, OUTPUT);
  // if (!card.begin(SD_CS_PIN, SD_SCK_MHZ(25))) 
  // {
  //   card.initErrorHalt();
  // }
  //   if (!root.open("/")) 
  // {
  //   card.errorHalt("open root failed");
  // }

  // memset(fileList, '\0', sizeof(fileList));
  // char loop_charBuffer[15];
  // while (file.openNext(&root, O_RDONLY)) 
  // {
  //   file.getName(loop_charBuffer, sizeof(loop_charBuffer));
  //   strcat(fileList,loop_charBuffer);
  //   // insert creation date here using file.getCreateDateTime();
  //   strcat(fileList,"<br>" );
  //   file.close();
  // }



//   void handleDataPage() 
// {
//   Serial.println(fileName);
//   if(card.exists(fileName)){
//     char buf[512];
//     FsFile fileStream = card.open(fileName, O_RDONLY);
//     size_t sizeStream = fileStream.size();
//     server.setContentLength(sizeStream);
//     server.send(200,"text/html", "");
//     while(sizeStream) 
//     {
//       size_t nread = fileStream.readBytes(buf, sizeof(buf));
//       server.client().write(buf, nread);
//       sizeStream -= nread;
//     }
//     fileStream.close();
//   }else
//   {
//     server.send(200, "text/html", htmlPage2);
//   }

//   // server.send(200, "text/html", htmlPage2);
// }