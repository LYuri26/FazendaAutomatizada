#include <WiFi.h>
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
#include "localizacao.h"

AsyncWebServer server(80);

const unsigned long UPDATE_INTERVAL = 300000;
const unsigned long LIGHTS_CHECK_INTERVAL = 300000;
unsigned long lastUpdate = 0;
unsigned long lastLightsCheck = 0;

void setupLittleFS();
void setupServer();
void updateTime();
void setupTimeClient();
void checkSunTimes();
bool isAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    setupLittleFS();
    loadSavedWiFiNetworks();
    enterAPMode(); // Define o modo AP inicialmente
    setupServer();
    setupTimeClient();
    checkAndUpdateSunTimes();
    checkSunTimes();
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL)
    {
        lastUpdate = currentMillis;
        updateTime();
    }

    if (currentMillis - lastLightsCheck >= LIGHTS_CHECK_INTERVAL)
    {
        lastLightsCheck = currentMillis;
        checkAndUpdateSunTimes();
        checkSunTimes();
    }

    static unsigned long lastCheckTime = 0;
    if (currentMillis - lastCheckTime >= 600000) // 10 minutos
    {
        lastCheckTime = currentMillis;

        if (isAPMode)
        {
            Serial.println("Modo Access Point ativo.");
        }
        else
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                loadSavedWiFiNetworks();
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
        Serial.println("Falha ao inicializar o LittleFS. Tentando formatar...");
        if (!LittleFS.format())
        {
            Serial.println("Falha ao formatar o LittleFS.");
            while (true)
                delay(1000);
        }
        if (!LittleFS.begin())
        {
            Serial.println("Falha ao inicializar o LittleFS após formatação.");
            while (true)
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
    setupDefinirHorarios(server);

    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", HTTP_GET, handleLogout);
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? request->send(LittleFS, "/dashboard", "text/html") : redirectToAccessDenied(request); });

    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? handleToggleAction(server) : redirectToAccessDenied(request); });

    server.on("/luzes-estados", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"luzCasaLigada\":" + String(readEstadoLuz(0)) + ",";
        json += "\"luzRuaLigada\":" + String(readEstadoLuz(1)) + ",";
        json += "\"luzPastoLigada\":" + String(readEstadoLuz(2)) + ",";
        json += "\"luzGeralLigada\":" + String(readEstadoLuz(3));
        json += "}";
        request->send(200, "application/json", json); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not found"); });

    server.begin();
}

void redirectToAccessDenied(AsyncWebServerRequest *request)
{
    request->redirect("/acesso-invalido");
}
