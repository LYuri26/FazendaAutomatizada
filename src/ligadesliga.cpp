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
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request);
            return;
        }

        String action = request->getParam("action") ? request->getParam("action")->value() : "";
        String idParam = request->getParam("id") ? request->getParam("id")->value() : "";
        int id = idParam.toInt();

        if (action.isEmpty() || id < 0 || id >= 4) {
            request->send(400, "text/plain", "Parâmetros inválidos!");
            return;
        }

        toggleLuz(id, action, request); });
}

void initLittleFS()
{
    if (!LittleFS.begin())
    {
        return;
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
    if (!file)
    {
        return;
    }

    file.println(state ? "1" : "0");
    file.close();
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
    pinoLuzGeral = readEstadoLuz(3);

    digitalWrite(pinoLuzCasa, pinoLuzGeral ? HIGH : LOW);
    digitalWrite(pinoLuzRua, pinoLuzGeral ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, pinoLuzGeral ? HIGH : LOW);

    handleToggleAction(server);
}

void toggleLuz(int index, String action, AsyncWebServerRequest *request)
{
    bool estado = (action == "ligar");

    if (index < 0 || index >= 4) {
        request->send(400, "text/plain", "Botão inválido!");
        return;
    }

    switch (index) {
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
            for (int i = 0; i < 3; i++) {
                luzEstado[i] = pinoLuzGeral;
                digitalWrite(i == 0 ? pinoLuzCasa : i == 1 ? pinoLuzRua : pinoLuzPasto, pinoLuzGeral ? HIGH : LOW);
                saveEstadoLuz(i, pinoLuzGeral);
            }
            saveEstadoLuz(3, pinoLuzGeral);
            break;
    }

    Serial.print("Luz ");
    Serial.print(index == 0 ? "Casa" : index == 1 ? "Rua" : index == 2 ? "Pasto" : "Geral");
    Serial.print(" está ");
    Serial.println(estado ? "ligada" : "desligada");

    request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
}
