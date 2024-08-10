#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <LittleFS.h>
#include <ESP8266WiFi.h>

// Definições de Access Point
extern const char *ap_ssid;
extern const char *ap_password;
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;

extern char ssid[32];
extern char password[32];
extern bool isAPMode;
extern bool connectionAttempted;

// Funções para gerenciamento de Wi-Fi
void loadSavedWiFiNetworks();
void enterAPMode();
bool connectToSavedNetworks(); // Atualizado para retornar bool
bool connectToWiFi(const String& ssid, const String& password);

#endif // WIFICONEXAO_H
