#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

void toggleLuzCasa(String action, AsyncWebServerRequest *request);
void toggleLuzRua(String action, AsyncWebServerRequest *request);
void toggleLuzPasto(String action, AsyncWebServerRequest *request);
void toggleLuzGeral(String action, AsyncWebServerRequest *request);
void setupLigaDesliga(AsyncWebServer &server);

extern const int pinoLuzCasa;
extern const int pinoLuzRua;
extern const int pinoLuzPasto;
extern bool pinoLuzGeral;
extern bool luzEstado[3]; 


void saveEstadoLuz(int index, bool state);

#endif
