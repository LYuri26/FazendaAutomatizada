#ifndef AUTENTICADOR_H
#define AUTENTICADOR_H

#include <ESPAsyncWebServer.h>

extern String sessionId;
extern bool userLoggedIn;
extern String loggedInUser;

void handleLogin(AsyncWebServerRequest *request);
void handleLogout(AsyncWebServerRequest *request);
bool isAuthenticated(AsyncWebServerRequest *request);
void notAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

#endif 