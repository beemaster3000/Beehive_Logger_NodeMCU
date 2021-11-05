
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "page1.h"
#include "page2.h"

const char *ssid = "Beemaster_3000";
const char *password = "12345678";

ESP8266WebServer server(80);

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

void handleRoot() 
{
  server.send(200, "text/html", htmlPage1);
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
}

//===========================================================================================
// ------------------------------- Main loop Arduino start ----------------------------------
//===========================================================================================
void loop()
{
 server.handleClient();
}
