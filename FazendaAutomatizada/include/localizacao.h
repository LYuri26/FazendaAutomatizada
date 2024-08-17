#ifndef LOCALIZACAO_H
#define LOCALIZACAO_H

#include <ESPAsyncWebServer.h>

void checkAndUpdateSunTimes();
void setupDefinirHorarios(AsyncWebServer &server);

#endif // LOCALIZACAO_H
