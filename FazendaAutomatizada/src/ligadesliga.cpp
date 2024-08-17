#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "autenticador.h"
#include "ligadesliga.h"
#include "tempo.h"
#include "localizacao.h"

const int pinoLuzCasa = 26;
const int pinoLuzRua = 27;
const int pinoLuzPasto = 14;
const String arquivoEstadoLuz[] = {
    "/estadoLuzCasa.txt",
    "/estadoLuzRua.txt",
    "/estadoLuzPasto.txt",
    "/estadoLuzGeral.txt"};

const String filePath = "/localizacao.txt"; // Arquivo de texto que contém os horários de nascer e pôr do sol
bool luzEstado[3] = {false, false, false};
bool luzGeralEstado = false;

void initLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao iniciar LittleFS.");
        return;
    }
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

bool readEstadoLuz(int index)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "r");
    if (!file)
    {
        return false;
    }
    String state = file.readStringUntil('\n');
    file.close();
    return state.toInt() == 1;
}

void saveEstadoLuz(int index, bool state)
{
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (file)
    {
        file.println(state ? "1" : "0");
        file.close();
    }
}

void toggleLuz(int index, String action, AsyncWebServerRequest *request)
{
    bool estado = (action == "ligar");
    switch (index)
    {
    case 0:
        luzEstado[0] = estado;
        digitalWrite(pinoLuzCasa, estado ? HIGH : LOW);
        Serial.println("Luz da Casa " + String(estado ? "ligada" : "desligada"));
        break;
    case 1:
        luzEstado[1] = estado;
        digitalWrite(pinoLuzRua, estado ? HIGH : LOW);
        Serial.println("Luz da Rua " + String(estado ? "ligada" : "desligada"));
        break;
    case 2:
        luzEstado[2] = estado;
        digitalWrite(pinoLuzPasto, estado ? HIGH : LOW);
        Serial.println("Luz do Pasto " + String(estado ? "ligada" : "desligada"));
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
            Serial.println("Luz " + String(i == 0 ? "da Casa" : i == 1 ? "da Rua"
                                                                       : "do Pasto") +
                           " " + (luzGeralEstado ? "ligada" : "desligada"));
        }
        saveEstadoLuz(3, luzGeralEstado);
        break;
    }
    saveEstadoLuz(index, estado);
    if (request)
        request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
}

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

void checkSunTimes()
{
    if (!LittleFS.exists(filePath))
    {
        Serial.println("Nenhuma localização armazenada. Ignorando a verificação.");
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Falha ao abrir o arquivo de localização.");
        return;
    }

    String sunrise, sunset;
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();                                      // Remove espaços em branco no início e fim da string
        Serial.println("Linha lida do arquivo: " + line); // Depuração
        if (line.startsWith("sunrise:"))
        {
            sunrise = line.substring(8);                                     // Assume formato "sunrise:HH:MM"
            sunrise.trim();                                                  // Remove espaços em branco no início e fim da string
            Serial.println("Horário de nascer do sol extraído: " + sunrise); // Depuração
        }
        else if (line.startsWith("sunset:"))
        {
            sunset = line.substring(7);                                  // Assume formato "sunset:HH:MM"
            sunset.trim();                                               // Remove espaços em branco no início e fim da string
            Serial.println("Horário de pôr do sol extraído: " + sunset); // Depuração
        }
    }
    file.close();

    if (sunrise.isEmpty() || sunset.isEmpty())
    {
        Serial.println("Falha ao encontrar horários de nascer ou pôr do sol no arquivo.");
        return;
    }

    Serial.println("Horário de nascer do sol definido para: " + sunrise);
    Serial.println("Horário de pôr do sol definido para: " + sunset);

    String currentTime = getTimeClient();             // Atualize com a função getTimeClient()
    String currentHour = currentTime.substring(0, 5); // Hora atual no formato HH:MM

    if (currentHour == sunset)
    {
        // Pôr do sol, liga as luzes
        Serial.println("É hora do pôr do sol, ligando as luzes...");
        toggleLuz(0, "ligar", nullptr);
        toggleLuz(1, "ligar", nullptr);
        toggleLuz(2, "ligar", nullptr);
        toggleLuz(3, "ligar", nullptr);
    }
    else if (currentHour == sunrise)
    {
        // Nascer do sol, desliga as luzes
        Serial.println("É hora do nascer do sol, desligando as luzes...");
        toggleLuz(0, "desligar", nullptr);
        toggleLuz(1, "desligar", nullptr);
        toggleLuz(2, "desligar", nullptr);
        toggleLuz(3, "desligar", nullptr);
    }
    else
    {
        Serial.println("Não é hora de ligar ou desligar as luzes.");
    }
}

void setupLigaDesliga(AsyncWebServer &server)
{
    initLittleFS();
    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);
    luzEstado[0] = readEstadoLuz(0);
    luzEstado[1] = readEstadoLuz(1);
    luzEstado[2] = readEstadoLuz(2);
    luzGeralEstado = readEstadoLuz(3);
    digitalWrite(pinoLuzCasa, luzEstado[0] ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzEstado[1] ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzEstado[2] ? HIGH : LOW);
    handleToggleAction(server);
}
