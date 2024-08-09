#include "autenticador.h"
#include <ESPAsyncWebServer.h>

String sessionId = "";
bool userLoggedIn = false;
String loggedInUser = "";

String generateSessionId() {
    String newSessionId = String(millis(), HEX);
    return newSessionId;
}

bool isAuthenticated(AsyncWebServerRequest *request) {
    if (request->hasHeader("Cookie")) {
        String cookie = request->header("Cookie");
        int sessionIndex = cookie.indexOf("session_id=");
        if (sessionIndex != -1) {
            String sessionValue = cookie.substring(sessionIndex + 11);
            if (sessionValue.equals(sessionId)) {
                return true;
            }
        }
    }
    return false;
}

void handleLogin(AsyncWebServerRequest *request) {
    String username;
    String password;

    if (request->hasParam("username", true)) {
        username = request->getParam("username", true)->value();
    } else {
        request->redirect("/?login_failed=true");
        return;
    }

    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    } else {
        request->redirect("/?login_failed=true");
        return;
    }

    if (username == "admin" && password == "admin123") {
        if (!userLoggedIn) {
            userLoggedIn = true;
            loggedInUser = username;
            sessionId = generateSessionId();
            AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
            response->addHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
            response->addHeader("Location", "/dashboard");
            request->send(response);
        } else {
            request->redirect("/usuario-ja-logado");
        }
    } else {
        request->redirect("/?login_failed=true");
    }
}

void handleLogout(AsyncWebServerRequest *request) {
    if (userLoggedIn) {
        userLoggedIn = false;
        loggedInUser = "";
        sessionId = "";
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/; HttpOnly");
        response->addHeader("Location", "/");
        request->send(response);
    } else {
        request->redirect("/");
    }
}

void notAuthenticated(AsyncWebServerRequest *request) {
    request->redirect("/");
}

void handleDashboard(AsyncWebServerRequest *request) {
    if (isAuthenticated(request)) {
        request->send(200, "text/plain", "Bem-vindo ao Dashboard!");
    } else {
        notAuthenticated(request);
    }
}
