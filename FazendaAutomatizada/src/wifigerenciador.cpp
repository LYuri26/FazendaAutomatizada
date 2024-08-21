#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifigerenciador.h"
#include "wifiinterface.h"

extern AsyncWebServer server;

void initializeFile()
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao montar o sistema de arquivos.");
        return;
    }

    if (!LittleFS.exists("/wifiredes.txt"))
    {
        File file = LittleFS.open("/wifiredes.txt", "w");
        if (!file)
        {
            Serial.println("Erro ao criar o arquivo wifiredes.txt.");
            return;
        }
        file.print("");
        file.close();
        Serial.println("Arquivo wifiredes.txt criado com sucesso.");
    }
}

void setupWiFiGerenciamentoPage(AsyncWebServer &server)
{
    Serial.println("Configurando rotas de gerenciamento Wi-Fi...");

    initializeFile();

    server.on("/wifigerenciamento", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /wifigerenciamento acessada.");
        request->send(200, "text/html", getWiFiGerenciamentoPage()); });

    server.on("/listadewifi", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /listadewifi acessada.");
        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file)
        {
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

    if (request->hasParam("ssid", true) && request->hasParam("password", true))
    {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();
        
        // Adicionando as redes ao arquivo
        File file = LittleFS.open("/wifiredes.txt", "a"); // Abrindo em modo de adição
        if (!file)
        {
            Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para escrita.");
            request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
            return;
        }

        // Escrevendo no arquivo no formato ssid,password\n
        file.printf("%s,%s\n", ssid.c_str(), password.c_str());
        file.close();

        Serial.println("Rede salva com sucesso.");
        request->send(200, "text/plain", "Rede salva com sucesso."); 
    }
    else
    {
        request->send(400, "text/plain", "Dados ausentes.");
    } });

    server.on("/excluirwifi", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /excluirwifi acessada.");

        if (request->hasParam("ssid"))
        {
            String ssidToDelete = request->getParam("ssid")->value();
            File file = LittleFS.open("/wifiredes.txt", "r");
            if (!file)
            {
                Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para leitura.");
                request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
                return;
            }

            String content = file.readString();
            file.close();

            String newContent;
            bool ssidFound = false;
            unsigned int start = 0;
            while (start < content.length())
            {
                int end = content.indexOf('\n', start);
                if (end == -1)
                    end = content.length();
                String line = content.substring(start, end);
                int commaIndex = line.indexOf(',');
                if (commaIndex != -1)
                {
                    String savedSSID = line.substring(0, commaIndex);
                    if (savedSSID != ssidToDelete)
                    {
                        newContent += line + "\n";
                    }
                    else
                    {
                        ssidFound = true;
                    }
                }
                else
                {
                    newContent += line + "\n";
                }
                start = end + 1;
            }

            if (ssidFound)
            {
                file = LittleFS.open("/wifiredes.txt", "w");
                if (!file)
                {
                    Serial.println("Erro ao abrir o arquivo de redes Wi-Fi para escrita.");
                    request->send(500, "text/plain", "Erro ao abrir o arquivo para escrita.");
                    return;
                }
                file.print(newContent);
                file.close();
                request->redirect("/wifigerenciamento");
            }
            else
            {
                request->send(404, "text/plain", "SSID não encontrado.");
            }
        }
        else
        {
            request->send(400, "text/plain", "SSID ausente.");
        } });

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /getip acessada.");
        request->send(200, "text/plain", WiFi.localIP().toString()); });

    server.on("/filecontents", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota /filecontents acessada.");
        File file = LittleFS.open("/wifiredes.txt", "r");
        if (!file)
        {
            Serial.println("Erro ao abrir o arquivo de redes Wi-Fi.");
            request->send(500, "text/plain", "Erro ao abrir o arquivo de redes Wi-Fi");
            return;
        }
        String content = file.readString();
        file.close();
        request->send(200, "text/plain", content); });

    Serial.println("Rotas de gerenciamento Wi-Fi configuradas com sucesso.");
}