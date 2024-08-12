#ifndef PAGINASERRO_H
#define PAGINASERRO_H

#include <ESPAsyncWebServer.h>

void setupErrorPages(AsyncWebServer& server);
void redirectToAccessDenied(AsyncWebServerRequest *request);
#endif
