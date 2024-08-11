#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

void toggleLuzCasa(String action, AsyncWebServerRequest *request);
void toggleLuzRua(String action, AsyncWebServerRequest *request);
void toggleLuzPasto(String action, AsyncWebServerRequest *request);
void toggleLuzGeral(String action, AsyncWebServerRequest *request);
void setupLigaDesliga(AsyncWebServer &server);
void handleToggleAction(AsyncWebServerRequest *request);


extern bool luzesLigadas[4];
extern bool luzCasaLigada;
extern bool luzRuaLigada;
extern bool luzPastoLigada;
extern bool luzGeralLigada;

#endif // LIGADESLIGA_H
