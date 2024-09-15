#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "autenticador.h"
#include "tempo.h"
#include "localizacao.h"

// Define os pinos para cada luz
const int pinoLuzCasa = 26;
const int pinoLuzRua = 27;
const int pinoLuzPasto = 14;

// Define os nomes das luzes
const String nomeLuzes[] = {
    "Luz Casa",
    "Luz Rua",
    "Luz Pasto",
    "Luz Geral"};

// Define os arquivos de estado das luzes
const String arquivoEstadoLuz[] = {
    "/estadoLuzCasa.txt",
    "/estadoLuzRua.txt",
    "/estadoLuzPasto.txt",
    "/estadoLuzGeral.txt"};

// Arquivo para armazenar localização
const String filePath = "/localizacao.txt";

// Estados das luzes
bool luzEstado[3] = {false, false, false};
bool luzGeralEstado = false;

// Controle manual das luzes
bool controleManual[4] = {false, false, false, false};

// Controle de alteração automática
bool alteracaoAutomaticoRealizada = false;

// Função para inicializar o sistema de arquivos
void initLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao iniciar o sistema de arquivos LittleFS.");
        return;
    }

    // Verifica a existência dos arquivos de estado e os inicializa, se necessário
    for (int i = 0; i < 4; i++)
    {
        if (!LittleFS.exists(arquivoEstadoLuz[i]))
        {
            File file = LittleFS.open(arquivoEstadoLuz[i], "w");
            if (file)
            {
                file.println("0");
                file.close();
            }
        }
    }
}

// Lê o estado de uma luz a partir do arquivo correspondente
bool readEstadoLuz(int index)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "r");
    if (!file)
        return false;

    String state = file.readStringUntil('\n');
    file.close();
    return state.toInt() == 1;
}

// Salva o estado de uma luz no arquivo correspondente
void saveEstadoLuz(int index, bool state)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (file)
    {
        file.println(state ? "1" : "0");
        file.close();
    }
}

// Alterna o estado de uma luz com base na ação (ligar/desligar)
void toggleLuz(int index, String action, AsyncWebServerRequest *request)
{
    bool estado = (action == "ligar");
    int pin;

    // Atualiza o estado das luzes com base no índice
    switch (index)
    {
    case 0:
        pin = pinoLuzCasa;
        luzEstado[0] = estado;
        break;
    case 1:
        pin = pinoLuzRua;
        luzEstado[1] = estado;
        break;
    case 2:
        pin = pinoLuzPasto;
        luzEstado[2] = estado;
        break;
    case 3:
        luzGeralEstado = estado;
        for (int i = 0; i < 3; i++)
        {
            luzEstado[i] = luzGeralEstado;
            pin = (i == 0) ? pinoLuzCasa : (i == 1) ? pinoLuzRua
                                                    : pinoLuzPasto;
            digitalWrite(pin, luzGeralEstado ? HIGH : LOW);
            saveEstadoLuz(i, luzGeralEstado);
        }
        saveEstadoLuz(3, luzGeralEstado);
        break;
    }

    // Ajusta o pino para o estado desejado
    digitalWrite(pin, estado ? HIGH : LOW);
    saveEstadoLuz(index, estado);

    // Atualiza o controle manual
    controleManual[index] = true;

    // Exibe mensagem no Serial
    if (index < 3)
    {
        Serial.printf("%s foi %s\n", nomeLuzes[index].c_str(), estado ? "ligada" : "desligada");
    }
    else
    {
        Serial.println("Todas as luzes foram " + String(estado ? "ligadas" : "desligadas"));
    }

    // Envia resposta para o cliente HTTP, se aplicável
    if (request)
    {
        request->send(200, "text/plain", nomeLuzes[index] + " " + (estado ? "ligada" : "desligada") + "!");
    }
}

// Manipula o endpoint /toggle para alternar as luzes via HTTP
void handleToggleAction(AsyncWebServer &server)
{
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String action = request->getParam("action") ? request->getParam("action")->value() : "";
        String idParam = request->getParam("id") ? request->getParam("id")->value() : "";
        int id = idParam.toInt();
        if (action.isEmpty() || id < 0 || id >= 4) {
            request->send(400, "text/plain", "Parâmetros inválidos!");
            return;
        }
        toggleLuz(id, action, request); });
}

// Verifica se as luzes precisam ser ligadas ou desligadas com base no nascer/pôr do sol
void checkSunTimes()
{
    if (!LittleFS.exists(filePath))
    {
        Serial.println("Arquivo de localização não encontrado.");
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Falha ao abrir o arquivo de localização.");
        return;
    }

    String fileContent = file.readString();
    file.close();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error || !doc.containsKey("sunrise") || !doc.containsKey("sunset"))
    {
        Serial.println("Horários de nascer e pôr do sol não encontrados.");
        return;
    }

    String sunrise = doc["sunrise"].as<String>();
    String sunset = doc["sunset"].as<String>();

    updateTime(); // Função externa que atualiza a hora
    String currentTime = getTimeClient().substring(0, 5);

    // Converte um horário (HH:MM) para minutos totais
    auto timeToMinutes = [](String time) -> int
    {
        int hours = time.substring(0, 2).toInt();
        int minutes = time.substring(3, 5).toInt();
        return hours * 60 + minutes;
    };

    int currentMinutes = timeToMinutes(currentTime);
    int sunriseMinutes = timeToMinutes(sunrise);
    int sunsetMinutes = timeToMinutes(sunset);

    // Lógica para ligar/desligar as luzes com base no horário
    if ((currentMinutes >= sunsetMinutes || currentMinutes < sunriseMinutes) && !alteracaoAutomaticoRealizada)
    {
        if (!luzGeralEstado && !controleManual[3])
        {
            toggleLuz(3, "ligar", nullptr);
            Serial.println("Luzes ligadas automaticamente. Motivo: pôr do sol.");
            alteracaoAutomaticoRealizada = true;
        }
    }
    else if ((currentMinutes >= sunriseMinutes && currentMinutes < sunsetMinutes) && !alteracaoAutomaticoRealizada)
    {
        if (luzGeralEstado && !controleManual[3])
        {
            toggleLuz(3, "desligar", nullptr);
            Serial.println("Luzes desligadas automaticamente. Motivo: nascer do sol.");
            alteracaoAutomaticoRealizada = true;
        }
    }
}

// Reseta o controle de alteração automática
void resetAlteracaoAutomatico()
{
    alteracaoAutomaticoRealizada = false;
}

// Configuração inicial das luzes e servidor
void setupLigaDesliga(AsyncWebServer &server)
{
    initLittleFS();
    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);

    // Inicializa o estado das luzes a partir dos arquivos
    luzEstado[0] = readEstadoLuz(0);
    luzEstado[1] = readEstadoLuz(1);
    luzEstado[2] = readEstadoLuz(2);
    luzGeralEstado = readEstadoLuz(3);

    // Aplica os estados às saídas
    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);

    // Configura o endpoint para alternar as luzes
    handleToggleAction(server);
}