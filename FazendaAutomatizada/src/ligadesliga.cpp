#include <FS.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "autenticador.h"
#include "ligadesliga.h"

void toggleLuz(int index, String action, AsyncWebServerRequest *request);

const int pinoLuzCasa = D4;
const int pinoLuzRua = D2;
const int pinoLuzPasto = D1;

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
        toggleLuz(id, action, request); });
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
    pinoLuzGeral = readEstadoLuz(3);

    Serial.println("Aplicando estados iniciais das luzes...");
    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);

    Serial.println("Estados iniciais aplicados com sucesso.");

    handleToggleAction(server);
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
        pinoLuzGeral = estado;
        for (int i = 0; i < 3; i++)
        {
            luzEstado[i] = pinoLuzGeral;
            digitalWrite(i == 0 ? pinoLuzCasa : i == 1 ? pinoLuzRua
                                                       : pinoLuzPasto,
                         pinoLuzGeral ? HIGH : LOW);
            saveEstadoLuz(i, pinoLuzGeral);
        }
        saveEstadoLuz(3, pinoLuzGeral);
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
