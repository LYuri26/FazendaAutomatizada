#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <ESP8266WiFi.h>

extern char ssid[32];        // Buffer para armazenar o SSID
extern char password[32];    // Buffer para armazenar a senha
extern bool isAPMode;
extern unsigned long lastMessageTime; // Declaração externa para acesso global

void connectToWiFi();
void enterAPMode();
void loadSavedWiFiNetworks();
void checkScanResults();

#endif // WIFICONEXAO_H
