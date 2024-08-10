#include <FS.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "autenticador.h"
#include "ligadesliga.h"

// Declaração antecipada da função
void toggleLuz(int index, String action, AsyncWebServerRequest *request);

const int pinoLuzCasa = 10;
const int pinoLuzRua = 11;
const int pinoLuzPasto = 12;

const String arquivoEstadoLuz[] = {"/estadoLuzCasa.txt", "/estadoLuzRua.txt", "/estadoLuzPasto.txt", "/estadoLuzGeral.txt"};
bool luzEstado[] = {false, false, false, false}; // 0: Casa, 1: Rua, 2: Pasto, 3: Geral

void handleToggleAction(AsyncWebServer &server)
{
    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request)
    {
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request);
            return;
        }

        String action = request->getParam("action")->value();
        int id = request->getParam("id")->value().toInt();

        if (id >= 0 && id < 4) {
            toggleLuz(id, action, request);
        } else {
            request->send(400, "text/plain", "Botão inválido!");
        }
    });
}

void initLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Erro ao iniciar LittleFS.");
        return;
    }
    Serial.println("LittleFS inicializado com sucesso.");
}

bool readEstadoLuz(int index)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "r");
    if (!file)
    {
        Serial.println("Arquivo de estado não encontrado, assumindo estado desligado.");
        return false;
    }

    String state = file.readStringUntil('\n');
    file.close();
    return state.toInt() == 1;
}

void saveEstadoLuz(int index, bool state)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (!file)
    {
        Serial.println("Erro ao abrir o arquivo para escrita.");
        return;
    }

    file.println(state ? "1" : "0");
    file.close();
}

void setupLigaDesliga(AsyncWebServer &server)
{
    Serial.println("Configurando o servidor Web para controle das luzes.");

    initLittleFS();

    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);

    luzEstado[0] = readEstadoLuz(0);
    luzEstado[1] = readEstadoLuz(1);
    luzEstado[2] = readEstadoLuz(2);
    luzEstado[3] = readEstadoLuz(3);

    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);

    handleToggleAction(server);

    Serial.println("Configuração concluída.");
}

void toggleLuz(int index, String action, AsyncWebServerRequest *request)
{
    if (action == "ligar")
    {
        luzEstado[index] = true;
        digitalWrite(index == 0 ? pinoLuzCasa : index == 1 ? pinoLuzRua : pinoLuzPasto, HIGH);
        request->send(200, "text/plain", "Luz " + String(index) + " ligada!");
    }
    else if (action == "desligar")
    {
        luzEstado[index] = false;
        digitalWrite(index == 0 ? pinoLuzCasa : index == 1 ? pinoLuzRua : pinoLuzPasto, LOW);
        request->send(200, "text/plain", "Luz " + String(index) + " desligada!");
    }
    else
    {
        request->send(400, "text/plain", "Ação inválida!");
    }
    saveEstadoLuz(index, luzEstado[index]);
}
