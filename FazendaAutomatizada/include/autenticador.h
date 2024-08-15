#ifndef AUTENTICADOR_H
#define AUTENTICADOR_H

#include <ESPAsyncWebServer.h>

// Variáveis externas para gerenciamento de sessão e autenticação
extern String sessionId;
extern bool userLoggedIn;
extern String loggedInUser;

// Funções para gerenciamento de autenticação e sessão
void handleLogin(AsyncWebServerRequest *request);
void handleLogout(AsyncWebServerRequest *request);
bool isAuthenticated(AsyncWebServerRequest *request);
void notAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

#endif // AUTENTICADOR_H
