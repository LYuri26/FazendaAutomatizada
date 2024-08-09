#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

extern bool compressorLigado;
extern const int pinoLigaDesliga;

void setupLigaDesliga(AsyncWebServer& server);
void saveCompressorState(bool state);

#endif // LIGADESLIGA_H
