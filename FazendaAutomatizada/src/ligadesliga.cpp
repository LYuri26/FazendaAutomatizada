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

const String filePath = "/localizacao.txt";
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
        return false;
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
    int pin;
    switch (index)
    {
    case 0:
        pin = pinoLuzCasa;
        luzEstado[0] = estado;
        Serial.println("Luz da casa foi " + String(estado ? "ligada" : "desligada"));
        break;
    case 1:
        pin = pinoLuzRua;
        luzEstado[1] = estado;
        Serial.println("Luz da rua foi " + String(estado ? "ligada" : "desligada"));
        break;
    case 2:
        pin = pinoLuzPasto;
        luzEstado[2] = estado;
        Serial.println("Luz do pasto foi " + String(estado ? "ligada" : "desligada"));
        break;
    case 3:
        luzGeralEstado = estado;
        for (int i = 0; i < 3; i++)
        {
            luzEstado[i] = luzGeralEstado;
            pin = (i == 0) ? pinoLuzCasa : (i == 1) ? pinoLuzRua
                                                    : pinoLuzPasto;
            digitalWrite(pin, luzGeralEstado ? HIGH : LOW);
            Serial.println("Luz " + String(i) + " foi " + String(luzGeralEstado ? "ligada" : "desligada"));
            saveEstadoLuz(i, luzGeralEstado);
        }
        saveEstadoLuz(3, luzGeralEstado);
        Serial.println("Luzes gerais foram " + String(luzGeralEstado ? "ligadas" : "desligadas"));
        break;
    }
    digitalWrite(pin, estado ? HIGH : LOW);
    saveEstadoLuz(index, estado);
    if (request)
        request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
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
        line.trim();
        if (line.startsWith("sunrise:"))
        {
            sunrise = line.substring(8);
            sunrise.trim();
        }
        else if (line.startsWith("sunset:"))
        {
            sunset = line.substring(7);
            sunset.trim();
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

    String currentTime = getTimeClient();
    String currentHour = currentTime.substring(0, 5);

    if (currentHour == sunset)
    {
        toggleLuz(0, "ligar", nullptr);
        toggleLuz(1, "ligar", nullptr);
        toggleLuz(2, "ligar", nullptr);
        toggleLuz(3, "ligar", nullptr);
    }
    else if (currentHour == sunrise)
    {
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
