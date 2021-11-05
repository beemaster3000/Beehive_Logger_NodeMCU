#include "WebServer.hpp"  
ESP8266WebServer WebServer::server_= ESP8266WebServer(80);
	
WebServer::WebServer()
{
}
	
WebServer::~WebServer()
{	
}

void WebServer::handleRoot() 
{
  server_.send(200, "text/html", "<h1>Hello from Beemaster_3000!</h1>");
}

void WebServer::handleClient()
{
  server_.handleClient();
}

void WebServer::setup() 
{  
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  server_.on("/", handleRoot);
  server_.begin();
 
  Serial.println("Server listening"); 
}