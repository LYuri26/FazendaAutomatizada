#include "autenticador.h"
#include <ESPAsyncWebServer.h>

String sessionId = "";
unsigned long sessionStartTime = 0;
const unsigned long sessionTimeout = 5 * 60 * 1000; // 5 minutos em milissegundos

bool isAuthenticated(AsyncWebServerRequest *request) {
    if (request->hasHeader("Cookie")) {
        String cookie = request->header("Cookie");
        int sessionIndex = cookie.indexOf("session_id=");
        if (sessionIndex != -1) {
            String sessionValue = cookie.substring(sessionIndex + 11);
            if (sessionValue.equals(sessionId)) {
                if (millis() - sessionStartTime < sessionTimeout) {
                    return true;
                } else {
                    sessionId = "";
                }
            }
        }
    }
    return false;
}

void handleLogin(AsyncWebServerRequest *request) {
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
        String username = request->getParam("username", true)->value();
        String password = request->getParam("password", true)->value();

        if (username == "admin" && password == "admin123") {
            sessionId = String(millis(), HEX);
            sessionStartTime = millis();
            AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
            response->addHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
            response->addHeader("Location", "/dashboard");
            request->send(response);
        } else {
            request->redirect("/?login_failed=true");
        }
    } else {
        request->redirect("/?login_failed=true");
    }
}

void handleLogout(AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
        sessionId = "";
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/; HttpOnly");
        response->addHeader("Location", "/");
        request->send(response);
    } else {
        request->redirect("/");
    }
}

void handleDashboard(AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
        request->send(200, "text/plain", "Bem-vindo ao Dashboard!");
    } else {
        request->redirect("/");
    }
}
