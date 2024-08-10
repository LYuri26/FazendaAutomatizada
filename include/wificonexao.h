#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <LittleFS.h>
#include <ESP8266WiFi.h>

extern const char *ap_ssid;
extern const char *ap_password;
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;
extern char ssid[32];
extern char password[32];
extern bool isAPMode;
extern bool connectionAttempted;

void connectToWiFi();
void enterAPMode();
void loadSavedWiFiNetworks();

#endif // WIFICONEXAO_H
