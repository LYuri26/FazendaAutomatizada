#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "index.h"
#include "autenticador.h"
#include "dashboard.h"
#include "ligadesliga.h"
#include "creditos.h"
#include "wificonexao.h"
#include "paginaserro.h"
#include "wifigerenciador.h"
#include "wifiinterface.h"

AsyncWebServer server(80);

void setupLittleFS();
void setupServer();
bool isAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

const unsigned long RECONNECT_INTERVAL = 1000;
const int MAX_RECONNECT_ATTEMPTS = 10;

unsigned long lastReconnectAttempt = 0;
int reconnectAttempts = 0;

void setup() {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_STA);
    delay(1000);

    setupLittleFS();
    loadSavedWiFiNetworks();

    setupServer();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = currentMillis;
            connectToWiFi();  // Chame sem parâmetros
            reconnectAttempts++;
            if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
                enterAPMode();
                reconnectAttempts = 0;
            }
        }
    }
}

void setupLittleFS() {
    if (!LittleFS.begin()) {
        Serial.println("Erro ao montar o sistema de arquivos LittleFS.");
        while (true) {
            delay(1000); // Espera em loop infinito para indicar erro.
        }
    } else {
        Serial.println("Sistema de arquivos LittleFS montado com sucesso.");
    }
}

void setupServer() {
    setupIndexPage(server);
    setupCreditosPage(server);
    setupDashboardPage(server);
    setupLigaDesliga(server);

    // Configura páginas de erro
    setupErrorPages(server);

    setupWiFiGerenciamentoPage(server);

    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
        handleLogin(request);
    });

    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleLogout(request);
    });

    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (isAuthenticated(request)) {
            request->send(LittleFS, "/dashboard", "text/html");
        } else {
            redirectToAccessDenied(request);
        }
    });

    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request) {
        if (isAuthenticated(request)) {
            handleToggleAction(server);
        } else {
            redirectToAccessDenied(request);
        }
    });

    server.on("/check-auth", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (isAuthenticated(request)) {
            request->send(200, "application/json", "{\"authenticated\":true}");
        } else {
            request->send(200, "application/json", "{\"authenticated\":false}");
        }
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    server.begin();
}

void redirectToAccessDenied(AsyncWebServerRequest *request) {
    request->redirect("/acesso-invalido");
}
