#ifndef PAGINASERRO_H
#define PAGINASERRO_H

#include <ESPAsyncWebServer.h>

// Função para configurar as páginas de erro
void setupErrorPages(AsyncWebServer &server);

// Função para redirecionar para a página de acesso negado
void redirectToAccessDenied(AsyncWebServerRequest *request);

#endif // PAGINASERRO_H
