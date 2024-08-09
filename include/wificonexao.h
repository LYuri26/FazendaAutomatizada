#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

extern const char *ssid;
extern const char *password;
extern bool isAPMode;

void connectToWiFi(const char *ssid, const char *password);
void enterAPMode();
void loadSavedWiFiNetworks();
void checkScanResults();

#endif // WIFICONEXAO_H
