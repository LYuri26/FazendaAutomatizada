#ifndef WIFIGERENCIADOR_H
#define WIFIGERENCIADOR_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// Configura a página de gerenciamento de Wi-Fi
void setupWiFiGerenciamentoPage(AsyncWebServer &server);

// Conecta-se a uma rede Wi-Fi usando SSID e senha fornecidos
void connectToWiFi(const char *ssid, const char *password);

// Coloca o ESP32 em modo Access Point (AP)
void enterAPMode();

// Inicia a varredura de redes Wi-Fi de forma assíncrona
void scanWiFiNetworksAsync();

// Processa os resultados da varredura de redes Wi-Fi
void processScanResults();

// Obtém os resultados da varredura de redes Wi-Fi em formato String
String getScanResults();

// Inicia a varredura de redes Wi-Fi
void startScanWiFiNetworks();

// Variáveis externas relacionadas à varredura de redes Wi-Fi
extern bool scanning;
extern unsigned long lastScan;
extern const unsigned long scanInterval;

#endif // WIFIGERENCIADOR_H
