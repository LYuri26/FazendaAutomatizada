#ifndef LIGADESLIGA_H
#define LIGADESLIGA_H

#include <ESPAsyncWebServer.h>

extern bool luzEstado[3];   // Atualizado para 3 luzes individuais
extern bool luzGeralEstado; // Estado geral das luzes

void setupLigaDesliga(AsyncWebServer &server);
void toggleLuz(int index, String action, AsyncWebServerRequest *request);
bool readEstadoLuz(int index);
void saveEstadoLuz(int index, bool state);
void handleToggleAction(AsyncWebServer &server); // Adicionar a declaração
void checkSunTimes();

#endif // LIGADESLIGA_H
