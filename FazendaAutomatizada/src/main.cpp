// main.cpp
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
#include "tempo.h"

AsyncWebServer server(80);

const unsigned long UPDATE_INTERVAL = 60000; // Atualização a cada minuto (60.000 ms)
unsigned long lastUpdate = 0;
unsigned long lastLightsCheck = 0;
unsigned long lastWiFiCheck = 0;

extern String cidadeSalva;
extern String nascerDoSol;
extern String porDoSol;

void setupLittleFS();
void setupServer();
void updateTime();
void setupTimeClient();
void checkSunTimes();
void checkAndUpdateSunTimes();
void obterDadosLocalizacao();
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
    obterDadosLocalizacao();    // Carrega dados iniciais de localização
    checkAndUpdateSunTimes();   // Atualiza horários de nascer e pôr do sol
    resetAlteracaoAutomatico(); // Reseta o controle de alteração automática
}

void loop()
{
    unsigned long currentMillis = millis();

    // Atualiza a hora a cada minuto
    if (currentMillis - lastUpdate >= UPDATE_INTERVAL)
    {
        lastUpdate = currentMillis;
        updateTime();             // Atualiza a hora a cada minuto
        checkAndUpdateSunTimes(); // Atualiza nascer/pôr do sol a cada 2 horas
        checkSunTimes();          // Atualiza o estado das luzes com base nos horários

        // Exibe a cidade e os horários
        if (cidadeSalva != "")
        {
            Serial.println("Cidade: " + cidadeSalva);
            Serial.println("Nascer do sol: " + nascerDoSol);
            Serial.println("Pôr do sol: " + porDoSol);
        }
    }

    // Verifica o estado do Wi-Fi e do AP a cada minuto, sem feedbacks repetitivos
    if (currentMillis - lastWiFiCheck >= UPDATE_INTERVAL)
    {
        lastWiFiCheck = currentMillis;

        if (isAPMode)
        {
            Serial.println("Modo Access Point ativo.");
        }
        else if (WiFi.status() != WL_CONNECTED)
        {
            loadSavedWiFiNetworks(); // Tenta reconectar se estiver desconectado
        }
    }

    delay(1000); // Loop principal
}

void setupLittleFS()
{
    if (!LittleFS.begin())
    {
        if (!LittleFS.format())
        {
            while (true)
                delay(1000); // Falha na formatação do LittleFS, para o sistema
        }
        if (!LittleFS.begin())
        {
            while (true)
                delay(1000); // Falha ao inicializar o LittleFS após formatação
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
