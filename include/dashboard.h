#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <ESPAsyncWebServer.h>

void setupDashboardPage(AsyncWebServer& server);
void handleToggleAction(AsyncWebServer& server);

#endif // DASHBOARD_H
