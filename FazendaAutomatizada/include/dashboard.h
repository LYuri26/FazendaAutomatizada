#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <ESPAsyncWebServer.h>


void handleToggleAction(AsyncWebServer &server);


void setupDashboardPage(AsyncWebServer &server);

#endif 
