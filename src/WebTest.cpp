
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h> 
// #include <sdFat.h> 

#include "page1.h"
#include "page2.h"
#include "page3.h"

const char *ssid = "Beemaster_3000";
const char *password = "12345678";
IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);

int short pageDisplayCounter;
int short filesInCard;

// SD card
File file;
// File root;
#define SD_CS_PIN D8 // SD chip select pin.
#define FILE_BASE_NAME "Data"
char fileName[11] = "Data04.csv";
char fileNameWebServer[15]="/";
char fileListHTML[1024];



//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================


void getFileList(File dir) 
{
  memset(fileListHTML, '\0', sizeof(fileListHTML));
  filesInCard=0;
   while(true) 
   {
     File entry = dir.openNextFile();
    if (! entry) 
      {
        // no more files
        // return to the first file in the directory
        dir.rewindDirectory();
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

      // strcat(fileListHTML,"<tr><td><a href=/" );
      // strcat(fileListHTML,entry.name());
      // strcat(fileListHTML,">" );
      
      strcat(fileListHTML,"<tr><td><a href=""/GetData"">");
      strcat(fileListHTML,entry.name());
      strcat(fileListHTML,"</a></td><td>");
      strcat(fileListHTML,BuffSize);
      strcat(fileListHTML,"</td><td>");
      ts = *localtime(&epochTimeCreate);
      strftime(buffTime, sizeof(buffTime), "%Y-%m-%d %H:%M", &ts);
      strcat(fileListHTML,buffTime);
      strcat(fileListHTML,"</td><td>");
      ts = *localtime(&epochTimeWrite);
      strftime(buffTime, sizeof(buffTime), "%Y-%m-%d %H:%M", &ts);
      strcat(fileListHTML,buffTime);
      strcat(fileListHTML,"</td></tr>");
      // print to terminal
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
  sprintf(charBuffer,htmlPage1,fileName,fileListHTML,pageDisplayCounter);
  server.send(200, "text/html", charBuffer);
}

void handleDataPage() 
{
  File fileToStream;
  fileToStream = SD.open(fileName, FILE_READ);
  int SDfsize  = fileToStream.size();
  server.sendHeader("Content-Length",(String)(SDfsize));
  server.sendHeader("Cache-Control","max-age=2628000,public");
  size_t fsizeSent = server.streamFile(fileToStream,"text/csv");

  Serial.print("File Size: ");
  Serial.println(SDfsize);
  Serial.print("File Size sent: ");
  Serial.println(fsizeSent);

  fileToStream.close();
  delay(100);
}

void createStreamData()
{
  handleNotFound();
}
void handleLivePage()
{
  server.send(200, "text/html", htmlPage3);
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

// void openReadFile()
// {
//   file = SD.open(fileName, FILE_READ);
// }

// void openWriteFile()
// {
//   file = SD.open(fileName, FILE_WRITE);
// }


  // fileNumber=atoi(&(fileName[4]));




