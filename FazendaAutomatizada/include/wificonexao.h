#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <LittleFS.h>
#include <WiFi.h>

extern const char *ap_ssid;
extern const char *ap_password;
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;

extern String ssid;
extern String password;
extern bool isAPMode;
extern bool connectionAttempted;

void loadSavedWiFiNetworks();
void enterAPMode();
bool connectToSavedNetworks();
bool connectToWiFi(const String &ssid, const String &password);
void attemptConnection(const String &ssid, const String &password);

#endif
