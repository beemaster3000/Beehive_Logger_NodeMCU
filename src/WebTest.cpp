
//===========================================================================================
// ------------------------------- load libraries and classes start -------------------------------
//===========================================================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "Beemaster_3000";
const char *password = "123456";

ESP8266WebServer server(80);

//===========================================================================================
// ------------------------------- program subfunctions start -------------------------------
//===========================================================================================

 
void handleRoot() 
{
  server.send(200, "text/html", "<h1>Hello from Beemaster_3000!</h1>");
}
 

//===========================================================================================
// ------------------------------- Initializing Arduino start -------------------------------
//===========================================================================================
void setup() 
{  
 Serial.begin(115200);
 
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  server.on("/", handleRoot);
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
