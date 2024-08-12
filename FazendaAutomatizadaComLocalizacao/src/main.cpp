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
#include "localizacaointerface.h"

AsyncWebServer server(80);

void setupLittleFS();
void setupServer();
bool isAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    setupLittleFS();
    loadSavedWiFiNetworks();
    enterAPMode();
    setupServer();
}

void loop()
{
    static unsigned long lastCheckTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastCheckTime > 600000)
    {
        lastCheckTime = currentTime;

        if (isAPMode)
        {
            Serial.println("Modo Access Point ativo.");
        }
        else
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                bool connected = connectToSavedNetworks();
                if (connected)
                {
                    Serial.println("Reconectado com sucesso.");
                }
                else
                {
                    Serial.println("Falha ao reconectar.");
                }
            }
            else
            {
                Serial.print("Conectado à rede Wi-Fi: ");
                Serial.println(WiFi.SSID());
                Serial.print("IP Local: ");
                Serial.println(WiFi.localIP());
            }
        }
    }

    delay(1000);
}

void setupLittleFS()
{
    if (!LittleFS.begin())
    {
        while (true)
        {
            delay(1000);
        }
    }
}

void setupServer()
{
    setupIndexPage(server);
    setupCreditosPage(server);
    setupDashboardPage(server);
    setupLigaDesliga(server);
    setupErrorPages(server);
    setupWiFiGerenciamentoPage(server);
    setupLocalizacaoPage(server);

    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", HTTP_GET, handleLogout);
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? request->send(LittleFS, "/dashboard", "text/html") : redirectToAccessDenied(request); });

    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? handleToggleAction(server) : redirectToAccessDenied(request); });

    server.on("/check-auth", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", "{\"authenticated\":" + String(isAuthenticated(request) ? "true" : "false") + "}"); });

    server.on("/localizacao", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Serve the localizacao page, make sure you have this file and route
        request->send(LittleFS, "/localizacao", "text/html"); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not found"); });

    server.begin();
}

void redirectToAccessDenied(AsyncWebServerRequest *request)
{
    request->redirect("/acesso-invalido");
}
