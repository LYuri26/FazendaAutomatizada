#ifndef PAGINASERRO_H
#define PAGINASERRO_H

#include <ESPAsyncWebServer.h>

void setupAcessoInvalidoPage(AsyncWebServer& server);
void setupNotFoundPage(AsyncWebServer& server);
void setupUsuarioJaLogadoPage(AsyncWebServer& server);
void setupCredenciaisInvalidasPage(AsyncWebServer& server);

#endif // PAGINASERRO_H
