#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifigerenciador.h"
#include "wifiinterface.h"

extern AsyncWebServer server;

void setupWiFiGerenciamentoPage(AsyncWebServer &server)
{
    Serial.println("Configurando rotas de gerenciamento Wi-Fi...");

    server.on("/wifigerenciamento", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /wifigerenciamento acessada.");
        request->send(200, "text/html", getWiFiGerenciamentoPage()); });

    server.on("/listadewifi", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /listadewifi acessada.");

        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file) {
            Serial.println("Erro ao abrir o arquivo de redes Wi-Fi.");
            request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
            return;
        }

        String networks = file.readString();
        file.close();
        request->send(200, "text/plain", networks); });

    server.on("/salvarwifi", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /salvarwifi acessada.");

        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
                Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para leitura.");
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent;
            bool ssidExists = false;
            unsigned int start = 0;
            while (start < content.length()) {
                int end = content.indexOf('\n', start);
                if (end == -1) end = content.length();
                String line = content.substring(start, end);
                int commaIndex = line.indexOf(',');
                if (commaIndex != -1) {
                    String savedSSID = line.substring(0, commaIndex);
                    if (savedSSID == ssid) {
                        newContent += ssid + "," + password + "\n";
                        ssidExists = true;
                    } else {
                        newContent += line + "\n";
                    }
                } else {
                    newContent += line + "\n";
                }
                start = end + 1;
            }
            if (!ssidExists) {
                newContent += ssid + "," + password + "\n";
            }

            file = LittleFS.open("/wifiredes.txt", "w");
            if (!file) {
                Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para escrita.");
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                return;
            }
            file.print(newContent);
            file.close();

            connectToSavedNetworks();  // Implementar a função connectToSavedNetworks para ESP32
            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "Dados ausentes.");
        } });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /excluirwifi acessada.");

        if (request->hasParam("ssid")) {
            String ssidToDelete = request->getParam("ssid")->value();

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
                Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para leitura.");
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent;
            bool ssidFound = false;
            unsigned int start = 0;
            while (start < content.length()) {
                int end = content.indexOf('\n', start);
                if (end == -1) end = content.length();
                String line = content.substring(start, end);
                int commaIndex = line.indexOf(',');
                if (commaIndex != -1) {
                    String savedSSID = line.substring(0, commaIndex);
                    if (savedSSID != ssidToDelete) {
                        newContent += line + "\n";
                    } else {
                        ssidFound = true;
                    }
                } else {
                    newContent += line + "\n";
                }
                start = end + 1;
            }

            if (ssidFound) {
                file = LittleFS.open("/wifiredes.txt", "w");
                if (!file) {
                    Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para escrita.");
                    request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                    return;
                }
                file.print(newContent);
                file.close();
                request->redirect("/wifigerenciamento");
            } else {
                request->send(404, "text/plain", "SSID não encontrado.");
            }
        } else {
            request->send(400, "text/plain", "SSID ausente.");
        } });

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /getip acessada.");
        request->send(200, "text/plain", WiFi.localIP().toString()); });

    Serial.println("Rotas de gerenciamento Wi-Fi configuradas com sucesso.");
}
