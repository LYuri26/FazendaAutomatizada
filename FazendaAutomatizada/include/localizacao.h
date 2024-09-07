#ifndef LOCALIZACAO_H
#define LOCALIZACAO_H

#include <ESPAsyncWebServer.h>

extern String cidadeSalva;
extern String nascerDoSol;
extern String porDoSol;

void checkAndUpdateSunTimes();
void setupDefinirHorarios(AsyncWebServer &server);
void obterDadosLocalizacao();

#endif
