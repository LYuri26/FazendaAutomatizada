#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifigerenciador.h"
#include "wifiinterface.h"

// Declaração externa do servidor
extern AsyncWebServer server;

void setupWiFiGerenciamentoPage(AsyncWebServer &server) {
    server.on("/wifigerenciamento", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", getWiFiGerenciamentoPage());
    });

    server.on("/listadewifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!LittleFS.exists("/wifiredes.txt")) {
            request->send(404, "text/plain", "Arquivo de redes Wi-Fi não encontrado.");
            return;
        }

        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file) {
            request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
            return;
        }

        String networks = file.readString();
        file.close();
        request->send(200, "text/plain", networks);
    });

    server.on("/salvarwifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

            // Verifica se o arquivo existe, se não existir cria um novo arquivo
            if (!LittleFS.exists("/wifiredes.txt")) {
                File file = LittleFS.open("/wifiredes.txt", "w");
                if (!file) {
                    request->send(500, "text/plain", "Erro ao criar o arquivo de redes Wi-Fi");
                    return;
                }
                file.close();
            }

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
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
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                return;
            }
            file.print(newContent);
            file.close();

            connectToSavedNetworks();

            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "Dados ausentes.");
        }
    });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid")) {
            String ssidToDelete = request->getParam("ssid")->value();

            // Verifica se o arquivo existe, se não existir cria um novo arquivo
            if (!LittleFS.exists("/wifiredes.txt")) {
                File file = LittleFS.open("/wifiredes.txt", "w");
                if (!file) {
                    request->send(500, "text/plain", "Erro ao criar o arquivo de redes Wi-Fi");
                    return;
                }
                file.close();
            }

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent;
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
                    }
                } else {
                    newContent += line + "\n";
                }
                start = end + 1;
            }

            file = LittleFS.open("/wifiredes.txt", "w");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                return;
            }
            file.print(newContent);
            file.close();

            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "SSID ausente.");
        }
    });

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request) {
        String ip = WiFi.localIP().toString();
        request->send(200, "text/plain", ip);
    });
}
