#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "autenticador.h"
#include "ligadesliga.h"

const int pinoLuzCasa = 26;  // GPIO16 no ESP32
const int pinoLuzRua = 27;   // GPIO4 no ESP32
const int pinoLuzPasto = 14; // GPIO5 no ESP32

const String arquivoEstadoLuz[] = {
    "/estadoLuzCasa.txt",
    "/estadoLuzRua.txt",
    "/estadoLuzPasto.txt",
    "/estadoLuzGeral.txt"};

bool luzEstado[3] = {false, false, false}; // Estado das luzes individuais
bool luzGeralEstado = false;               // Estado geral que controla todas as luzes

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
    Serial.printf("Salvando estado %s para o arquivo: %s\n", state ? "ligado" : "desligado", arquivoEstadoLuz[index].c_str());
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (!file)
    {
        Serial.printf("Falha ao abrir o arquivo para escrita: %s\n", arquivoEstadoLuz[index].c_str());
        return;
    }

    file.println(state ? "1" : "0");
    file.close();
    Serial.printf("Estado %s salvo para o arquivo: %s\n", state ? "ligado" : "desligado", arquivoEstadoLuz[index].c_str());
}

void toggleLuz(int index, String action, AsyncWebServerRequest *request)
{
    bool estado = (action == "ligar");

    if (index < 0 || index >= 4)
    {
        Serial.println("ID de botão inválido.");
        request->send(400, "text/plain", "Botão inválido!");
        return;
    }

    Serial.printf("Toggling luz %d para %s.\n", index, estado ? "ligar" : "desligar");

    switch (index)
    {
    case 0:
        luzEstado[0] = estado;
        digitalWrite(pinoLuzCasa, estado ? HIGH : LOW);
        saveEstadoLuz(0, estado);
        break;
    case 1:
        luzEstado[1] = estado;
        digitalWrite(pinoLuzRua, estado ? HIGH : LOW);
        saveEstadoLuz(1, estado);
        break;
    case 2:
        luzEstado[2] = estado;
        digitalWrite(pinoLuzPasto, estado ? HIGH : LOW);
        saveEstadoLuz(2, estado);
        break;
    case 3:
        luzGeralEstado = estado;
        for (int i = 0; i < 3; i++)
        {
            luzEstado[i] = luzGeralEstado;
            digitalWrite(i == 0 ? pinoLuzCasa : i == 1 ? pinoLuzRua
                                                       : pinoLuzPasto,
                         luzGeralEstado ? HIGH : LOW);
            saveEstadoLuz(i, luzGeralEstado);
        }
        saveEstadoLuz(3, luzGeralEstado);
        break;
    }

    Serial.print("Luz ");
    Serial.print(index == 0 ? "Casa" : index == 1 ? "Rua"
                                   : index == 2   ? "Pasto"
                                                  : "Geral");
    Serial.print(" foi ");
    Serial.println(estado ? "ligada" : "desligada");

    request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
}

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
        toggleLuz(id, action, request); });
}

void setupLigaDesliga(AsyncWebServer &server)
{
    Serial.println("Configurando função Liga/Desliga...");

    initLittleFS();

    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);

    Serial.println("Carregando estados iniciais das luzes...");
    luzEstado[0] = readEstadoLuz(0);
    luzEstado[1] = readEstadoLuz(1);
    luzEstado[2] = readEstadoLuz(2);
    luzGeralEstado = readEstadoLuz(3);

    Serial.println("Aplicando estados iniciais das luzes...");
    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);

    Serial.println("Estados iniciais aplicados com sucesso.");

    handleToggleAction(server); // Agora deve funcionar
}
