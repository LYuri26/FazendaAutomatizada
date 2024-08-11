#include <FS.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "autenticador.h"
#include "ligadesliga.h"

// Declaração antecipada da função
void toggleLuz(int index, String action, AsyncWebServerRequest *request);

const int pinoLuzCasa = D4;
const int pinoLuzRua = D2;
const int pinoLuzPasto = D1;

const String arquivoEstadoLuz[] = {"/estadoLuzCasa.txt", "/estadoLuzRua.txt", "/estadoLuzPasto.txt", "/estadoLuzGeral.txt"};
bool luzEstado[] = {false, false, false, false}; // 0: Casa, 1: Rua, 2: Pasto, 3: Geral

void handleToggleAction(AsyncWebServer &server)
{
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
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
        } });
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
    bool estado = (action == "ligar");

    // Atualiza o estado da luz específica
    if (index < 3)
    {
        luzEstado[index] = estado;
        digitalWrite(index == 0 ? pinoLuzCasa : index == 1 ? pinoLuzRua
                                                           : pinoLuzPasto,
                     estado ? HIGH : LOW);
        saveEstadoLuz(index, estado);

        // Adiciona feedback ao Serial
        Serial.print("Luz ");
        Serial.print(index == 0 ? "Casa" : index == 1 ? "Rua" : "Pasto");
        Serial.print(" está ");
        Serial.println(estado ? "ligada" : "desligada");

        request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
    }
    else if (index == 3)
    {
        // Luz Geral
        bool newState = !estado; // Inverte o estado da luz geral
        for (int i = 0; i < 3; i++)
        {
            luzEstado[i] = newState;
            digitalWrite(i == 0 ? pinoLuzCasa : i == 1 ? pinoLuzRua
                                                       : pinoLuzPasto,
                         newState ? HIGH : LOW);
            saveEstadoLuz(i, newState);

            // Adiciona feedback ao Serial
            Serial.print("Luz ");
            Serial.print(i == 0 ? "Casa" : i == 1 ? "Rua" : "Pasto");
            Serial.print(" está ");
            Serial.println(newState ? "ligada" : "desligada");
        }
        luzEstado[3] = newState;

        Serial.print("Luz Geral está ");
        Serial.println(newState ? "ligada" : "desligada");

        request->send(200, "text/plain", "Luz Geral " + String(newState ? "ligada" : "desligada") + "!");
    }
}
