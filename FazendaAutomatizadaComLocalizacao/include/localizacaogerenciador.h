#ifndef LOCALIZACAOGERENCIADOR_H
#define LOCALIZACAOGERENCIADOR_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Função para codificar a URL
String encodeURIComponent(const String &value);

// Função para buscar a localização em latitude e longitude usando um serviço de geocodificação
String geocodeLocation(const String &location);

// Função para buscar o horário do nascer e pôr do sol usando a Sunrise-Sunset API
String fetchSunTimes(float lat, float lng);

// Função para lidar com a requisição de localização
void handleLocationRequest(const String &location, AsyncWebServerRequest *request);

#endif // LOCALIZACAOGERENCIADOR_H
