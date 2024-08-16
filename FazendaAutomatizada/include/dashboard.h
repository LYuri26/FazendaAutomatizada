#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <ESPAsyncWebServer.h>

// Função para lidar com a ação de alternar o estado das luzes
void handleToggleAction(AsyncWebServer &server);

// Função para configurar a página do dashboard
void setupDashboardPage(AsyncWebServer &server);

#endif // DASHBOARD_H
