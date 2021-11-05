#pragma once
#include <ESP8266WebServer.h>
#include "page1.h"
#include "page2.h"

class WebServer  
{

public:
	WebServer();
	~WebServer();

	static void handleRoot();
	static void handleClient();
	static void setup();

private:
	static ESP8266WebServer server_;
};

