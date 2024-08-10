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

// Definindo o servidor web
AsyncWebServer server(80);

void setupLittleFS();
void setupServer();
bool isAuthenticated(AsyncWebServerRequest *request);
void redirectToAccessDenied(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    setupLittleFS(); // Corrigido para garantir que o LittleFS esteja configurado antes do uso
    loadSavedWiFiNetworks();

    if (!connectionAttempted)
    {
        enterAPMode();
    }

    setupServer();
}

void loop()
{
    static unsigned long lastCheckTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastCheckTime > 600000)
    { // 10 minutos em milissegundos
        lastCheckTime = currentTime;

        if (isAPMode)
        {
            Serial.println("Modo Access Point ativo.");
            Serial.println("Verificando solicitações no modo AP...");
        }
        else
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("Desconectado da rede Wi-Fi.");
                bool connected = connectToSavedNetworks(); // Corrigido para usar a função atualizada
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

    delay(1000); // Aguarda 1 segundo antes da próxima iteração do loop
}

void setupLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Erro ao montar o sistema de arquivos LittleFS.");
        while (true)
        {
            delay(1000);
        } // Loop infinito em caso de erro
    }
    Serial.println("Sistema de arquivos LittleFS montado com sucesso.");
}

void setupServer()
{

    setupIndexPage(server);
    setupCreditosPage(server);
    setupDashboardPage(server);
    setupLigaDesliga(server);
    setupErrorPages(server);
    setupWiFiGerenciamentoPage(server);

    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", HTTP_GET, handleLogout);
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? request->send(LittleFS, "/dashboard", "text/html") : redirectToAccessDenied(request); });

    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? handleToggleAction(server) : redirectToAccessDenied(request); });

    server.on("/check-auth", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", "{\"authenticated\":" + String(isAuthenticated(request) ? "true" : "false") + "}"); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not found"); });

    server.begin();
}

void redirectToAccessDenied(AsyncWebServerRequest *request)
{
    request->redirect("/acesso-invalido");
}
