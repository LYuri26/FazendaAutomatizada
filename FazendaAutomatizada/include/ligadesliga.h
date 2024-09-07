#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

extern bool controleManual[4];
extern bool luzEstado[3];
extern bool luzGeralEstado;
void setupLigaDesliga(AsyncWebServer &server);
void toggleLuz(int index, String action, AsyncWebServerRequest *request);
bool readEstadoLuz(int index);
void saveEstadoLuz(int index, bool state);
void handleToggleAction(AsyncWebServer &server);
void checkSunTimes();
void resetAlteracaoAutomatico();

#endif