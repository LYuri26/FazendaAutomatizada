#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "autenticador.h"
#include "ligadesliga.h"

void handleToggleAction(AsyncWebServer &server);
bool readSunriseSunsetTimes(String &sunrise, String &sunset);

const int pinoLuzCasa = 16; // GPIO16 no ESP32
const int pinoLuzRua = 4;   // GPIO4 no ESP32
const int pinoLuzPasto = 5; // GPIO5 no ESP32

const String arquivoEstadoLuz[] = {"/estadoLuzCasa.txt", "/estadoLuzRua.txt", "/estadoLuzPasto.txt", "/estadoLuzGeral.txt"};
bool luzEstado[] = {false, false, false};
bool pinoLuzGeral = false;

void handleToggleAction(AsyncWebServer &server)
{
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Rota '/toggle' acessada.");

        if (!isAuthenticated(request)) {
            Serial.println("Acesso negado! Usuário não autenticado.");
            redirectToAccessDenied(request);
            return;
        }

        String action = request->getParam("action") ? request->getParam("action")->value() : "";
        String idParam = request->getParam("id") ? request->getParam("id")->value() : "";
        int id = idParam.toInt();

        if (action.isEmpty() || id < 0 || id >= 4) {
            Serial.println("Parâmetros inválidos para a ação de toggle.");
            request->send(400, "text/plain", "Parâmetros inválidos!");
            return;
        }

        Serial.printf("Ação: %s, ID: %d\n", action.c_str(), id);
        if (id < 3) {
            bool state = (action == "on");
            luzEstado[id] = state;
            digitalWrite(pinoLuzCasa + id, state ? HIGH : LOW);
            saveEstadoLuz(id, state);
        } else {
            pinoLuzGeral = (action == "on");
            digitalWrite(17, pinoLuzGeral ? HIGH : LOW);
        }

        request->send(200, "text/plain", (action == "on") ? "Luz ligada!" : "Luz desligada!"); });
}

void initLittleFS()
{
    Serial.println("Inicializando LittleFS...");
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao iniciar LittleFS.");
        return;
    }

    Serial.println("LittleFS iniciado com sucesso.");

    for (int i = 0; i < 4; i++)
    {
        if (!LittleFS.exists(arquivoEstadoLuz[i]))
        {
            File file = LittleFS.open(arquivoEstadoLuz[i], "w");
            if (file)
            {
                Serial.printf("Criando arquivo de estado padrão: %s\n", arquivoEstadoLuz[i].c_str());
                file.println("0");
                file.close();
            }
            else
            {
                Serial.printf("Falha ao criar o arquivo: %s\n", arquivoEstadoLuz[i].c_str());
            }
        }
        else
        {
            Serial.printf("Arquivo de estado já existe: %s\n", arquivoEstadoLuz[i].c_str());
        }
    }
}

bool readEstadoLuz(int index)
{
    Serial.printf("Lendo estado da luz do arquivo: %s\n", arquivoEstadoLuz[index].c_str());
    File file = LittleFS.open(arquivoEstadoLuz[index], "r");
    if (!file)
    {
        Serial.printf("Falha ao abrir o arquivo: %s\n", arquivoEstadoLuz[index].c_str());
        return false;
    }

    String state = file.readStringUntil('\n');
    file.close();
    Serial.printf("Estado lido para o arquivo %s: %s\n", arquivoEstadoLuz[index].c_str(), state.c_str());
    return state.toInt() == 1;
}

void saveEstadoLuz(int index, bool state)
{
    Serial.printf("Salvando estado %s para o arquivo: %s\n", state ? "LIGADO" : "DESLIGADO", arquivoEstadoLuz[index].c_str());
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (!file)
    {
        Serial.printf("Falha ao abrir o arquivo para escrita: %s\n", arquivoEstadoLuz[index].c_str());
        return;
    }

    file.println(state ? "1" : "0");
    file.close();
}

void setupLigaDesliga(AsyncWebServer &server)
{
    Serial.println("Configurando função Liga/Desliga...");

    initLittleFS();

    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);
    pinMode(17, OUTPUT);

    Serial.println("Carregando estados iniciais das luzes...");
    luzEstado[0] = readEstadoLuz(0);
    luzEstado[1] = readEstadoLuz(1);
    luzEstado[2] = readEstadoLuz(2);
    pinoLuzGeral = readEstadoLuz(3);

    Serial.println("Aplicando estados iniciais das luzes...");
    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);
    digitalWrite(17, pinoLuzGeral ? HIGH : LOW);

    Serial.println("Estados iniciais aplicados com sucesso.");

    handleToggleAction(server);
}
