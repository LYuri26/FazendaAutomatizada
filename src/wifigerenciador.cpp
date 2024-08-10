#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifiinterface.h"

// Função para tentar conectar a uma rede Wi-Fi
bool connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempts = 0;
    const int maxAttempts = 20; // Tentar por até 20 tentativas
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado com sucesso!");
        return true;
    } else {
        Serial.println("Falha ao conectar.");
        return false;
    }
}

// Função para ler e conectar às redes Wi-Fi salvas
void connectToSavedNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Erro ao abrir o arquivo de redes Wi-Fi.");
        return;
    }

    String content = file.readString();
    file.close();

    unsigned int start = 0;
    while (start < content.length()) {
        int end = content.indexOf('\n', start);
        if (end == -1) end = content.length();
        String line = content.substring(start, end);
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1) {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            if (connectToWiFi(savedSSID, savedPassword)) {
                return; // Se conectar com sucesso, não continue tentando outras redes
            }
        }
        start = end + 1;
    }
    Serial.println("Nenhuma rede salva foi conectada.");
}

void setupWiFiGerenciamentoPage(AsyncWebServer &server)
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao iniciar o sistema de arquivos LittleFS");
        return;
    }

    server.on("/wifigerenciamento", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", getWiFiGerenciamentoPage());
    });

    server.on("/listadewifi", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file) {
            request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
            return;
        }

        String networks = file.readString();
        file.close();
        request->send(200, "text/plain", networks);
    });

    server.on("/salvarwifi", HTTP_POST, [](AsyncWebServerRequest *request)
    {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

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

            // Conecta às redes salvas
            connectToSavedNetworks();

            request->redirect("/wifigerenciamento");
        } else {
            request->send(400, "text/plain", "Dados ausentes.");
        }
    });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        if (request->hasParam("ssid")) {
            String ssidToDelete = request->getParam("ssid")->value();

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

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        String ip = WiFi.localIP().toString();
        request->send(200, "text/plain", ip);
    });
}
