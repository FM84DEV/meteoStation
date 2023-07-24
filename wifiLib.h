#ifndef WIFILIB_H
#define WIFILIB_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//print debug info on serial port
extern bool debugOutput;   //defined in global config, cannot be const (don't know why)
extern const char* ssid;
extern const char* password;
extern const char* updateMD5paswd;

void wifiSetup();
void wifiCheckReboot();
void otaUpdSetup();

#endif
