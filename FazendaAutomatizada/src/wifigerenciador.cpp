#include <ESP8266WiFi.h>
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

        if (!LittleFS.exists("/wifiredes.txt")) {
            Serial.println("Arquivo de redes Wi-Fi não encontrado.");
            request->send(404, "text/plain", "Arquivo de redes Wi-Fi não encontrado.");
            return;
        }

        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file) {
            Serial.println("Erro ao abrir o arquivo de redes Wi-Fi.");
            request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
            return;
        }

        String networks = file.readString();
        file.close();
        Serial.println("Redes Wi-Fi lidas com sucesso.");
        request->send(200, "text/plain", networks); });

    server.on("/salvarwifi", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /salvarwifi acessada.");

        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            Serial.print("SSID recebido: ");
            Serial.println(ssid);
            Serial.print("Password recebido: ");
            Serial.println(password);

            if (!LittleFS.exists("/wifiredes.txt")) {
                Serial.println("Arquivo de redes Wi-Fi não encontrado. Criando novo arquivo.");
                File file = LittleFS.open("/wifiredes.txt", "w");
                if (!file) {
                    Serial.println("Erro ao criar o arquivo de redes Wi-Fi.");
                    request->send(500, "text/plain", "Erro ao criar o arquivo de redes Wi-Fi");
                    return;
                }
                file.close();
            }

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
                        Serial.print("SSID existente atualizado: ");
                        Serial.println(savedSSID);
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
                Serial.print("Novo SSID adicionado: ");
                Serial.println(ssid);
            }

            file = LittleFS.open("/wifiredes.txt", "w");
            if (!file) {
                Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para escrita.");
                request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                return;
            }
            file.print(newContent);
            file.close();

            Serial.println("SSID e senha salvos com sucesso. Tentando conectar às redes salvas...");
            connectToSavedNetworks();

            request->redirect("/wifigerenciamento");
        } else {
            Serial.println("Dados ausentes (SSID ou senha).");
            request->send(400, "text/plain", "Dados ausentes.");
        } });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /excluirwifi acessada.");

        if (request->hasParam("ssid")) {
            String ssidToDelete = request->getParam("ssid")->value();
            Serial.print("SSID a ser excluído: ");
            Serial.println(ssidToDelete);

            if (!LittleFS.exists("/wifiredes.txt")) {
                Serial.println("Arquivo de redes Wi-Fi não encontrado. Criando novo arquivo.");
                File file = LittleFS.open("/wifiredes.txt", "w");
                if (!file) {
                    Serial.println("Erro ao criar o arquivo de redes Wi-Fi.");
                    request->send(500, "text/plain", "Erro ao criar o arquivo de redes Wi-Fi");
                    return;
                }
                file.close();
            }

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
                        Serial.print("SSID encontrado e excluído: ");
                        Serial.println(savedSSID);
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
                Serial.println("SSID excluído com sucesso.");
                request->redirect("/wifigerenciamento");
            } else {
                Serial.println("SSID não encontrado no arquivo de redes Wi-Fi.");
                request->send(404, "text/plain", "SSID não encontrado.");
            }
        } else {
            Serial.println("SSID ausente na solicitação.");
            request->send(400, "text/plain", "SSID ausente.");
        } });

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /getip acessada.");
        String ip = WiFi.localIP().toString();
        Serial.print("IP Local: ");
        Serial.println(ip);
        request->send(200, "text/plain", ip); });

    Serial.println("Rotas de gerenciamento Wi-Fi configuradas com sucesso.");
}
