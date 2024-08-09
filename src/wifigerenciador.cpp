#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifiinterface.h"

void setupWiFiGerenciamentoPage(AsyncWebServer &server) {
    if (!LittleFS.begin()) {
        Serial.println("Falha ao iniciar o sistema de arquivos LittleFS");
        return;
    }

    server.on("/wifigerenciamento", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", getWiFiGerenciamentoPage());
    });

    server.on("/listadewifi", HTTP_GET, [](AsyncWebServerRequest *request) {
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
        String ssid;
        String password;

        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            ssid = request->getParam("ssid", true)->value();
            password = request->getParam("password", true)->value();

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent = "";
            bool ssidExists = false;
            if (content.length() > 0) {
                unsigned int start = 0;
                while (start < static_cast<unsigned int>(content.length())) {
                    int end = content.indexOf('\n', start);
                    if (end == -1) {
                        end = content.length();
                    }
                    String line = content.substring(start, end);
                    int commaIndex = line.indexOf(',');
                    String savedSSID = line.substring(0, commaIndex);
                    if (savedSSID == ssid) {
                        newContent += ssid + "," + password + "\n";
                        ssidExists = true;
                    } else {
                        newContent += line + "\n";
                    }
                    start = end + 1;
                }
            }

            if (!ssidExists) {
                newContent += ssid + "," + password + "\n";
            }

            file = LittleFS.open("/wifiredes.txt", "w");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita");
                return;
            }
            file.print(newContent);
            file.close();

            connectToWiFi(ssid.c_str(), password.c_str());

            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "Dados ausentes.");
        }
    });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid")) {
            String ssidToDelete = request->getParam("ssid")->value();

            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent = "";
            unsigned int start = 0;
            while (start < static_cast<unsigned int>(content.length())) {
                int end = content.indexOf('\n', start);
                if (end == -1) {
                    end = content.length();
                }
                String line = content.substring(start, end);
                int commaIndex = line.indexOf(',');
                String savedSSID = line.substring(0, commaIndex);
                if (savedSSID != ssidToDelete) {
                    newContent += line + "\n";
                }
                start = end + 1;
            }

            file = LittleFS.open("/wifiredes.txt", "w");
            if (!file) {
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita");
                return;
            }
            file.print(newContent);
            file.close();

            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "SSID ausente.");
        }
    });
}
