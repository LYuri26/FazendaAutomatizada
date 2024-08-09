#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

extern bool compressorLigado;
extern const int pinoLigaDesliga;

void setupLigaDesliga(AsyncWebServer& server);
void toggleLuzCasa(String action, AsyncWebServerRequest *request);
void toggleLuzRua(String action, AsyncWebServerRequest *request);
void toggleLuzPasto(String action, AsyncWebServerRequest *request);
void toggleLuzGeral(String action, AsyncWebServerRequest *request);

extern bool luzCasaLigada;
extern bool luzRuaLigada;
extern bool luzPastoLigada;
extern bool luzGeralLigada;

#endif // LIGADESLIGA_H
