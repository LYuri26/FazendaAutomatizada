#ifndef WIFICONEXAO_H
#define WIFICONEXAO_H

#include <LittleFS.h>
#include <WiFi.h> // Use a biblioteca WiFi do ESP32

// Configurações do ponto de acesso (AP)
extern const char *ap_ssid;
extern const char *ap_password;
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;

// Dados da rede Wi-Fi
extern char ssid[32];
extern char password[32];
extern bool isAPMode;
extern bool connectionAttempted;

// Funções relacionadas à conexão Wi-Fi
void loadSavedWiFiNetworks();
void enterAPMode();
bool connectToSavedNetworks();
bool connectToWiFi(const String &ssid, const String &password);

#endif // WIFICONEXAO_H
