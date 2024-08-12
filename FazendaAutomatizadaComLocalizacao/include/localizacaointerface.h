#ifndef LOCALIZACAO_INTERFACE_H
#define LOCALIZACAO_INTERFACE_H

#include <ESPAsyncWebServer.h>

// HTML da página de localização como uma string C++
extern String localizacaoHtml;

// Funções externas para manipular requisições
extern void handleLocationRequest(const String &location, AsyncWebServerRequest *request);
extern void handleAutocompleteRequest(const String &query, AsyncWebServerRequest *request);

// Função para configurar a página e rotas
void setupLocalizacaoPage(AsyncWebServer &server);

#endif // LOCALIZACAO_INTERFACE_H
