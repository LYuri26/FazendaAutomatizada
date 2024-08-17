#ifndef LOCALIZACAO_H
#define LOCALIZACAO_H

#include <ESPAsyncWebServer.h>

void setupLocationPage(AsyncWebServer &server);
void checkAndUpdateSunTimes();

#endif // LOCALIZACAO_H
