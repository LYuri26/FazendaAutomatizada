#ifndef WIFIGERENCIADOR_H
#define WIFIGERENCIADOR_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>


void setupWiFiGerenciamentoPage(AsyncWebServer &server);


void connectToWiFi(const char *ssid, const char *password);


void enterAPMode();


void scanWiFiNetworksAsync();


void processScanResults();


String getScanResults();


void startScanWiFiNetworks();


extern bool scanning;
extern unsigned long lastScan;
extern const unsigned long scanInterval;

#endif
