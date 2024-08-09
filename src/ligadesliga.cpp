#include <FS.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "autenticador.h"
#include "ligadesliga.h"

const int pinoLuzCasa = 2;
const int pinoLuzRua = 3;
const int pinoLuzPasto = 4;

const String arquivoEstadoCasa = "/estadoLuzCasa.txt";
const String arquivoEstadoRua = "/estadoLuzRua.txt";
const String arquivoEstadoPasto = "/estadoLuzPasto.txt";
const String arquivoEstadoGeral = "/estadoLuzGeral.txt";

bool luzCasaLigada = false;
bool luzRuaLigada = false;
bool luzPastoLigada = false;
bool luzGeralLigada = false;

void handleToggleAction(AsyncWebServer &server) {
    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request) {
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request);
            return;
        }

        String action = request->getParam("action")->value();
        String id = request->getParam("id")->value();

        if (id == "toggleButton1") { // Luz da Casa
            toggleLuzCasa(action, request);
        } else if (id == "toggleButton2") { // Luz da Rua
            toggleLuzRua(action, request);
        } else if (id == "toggleButton3") { // Luz do Pasto
            toggleLuzPasto(action, request);
        } else if (id == "toggleButton4") { // Luz Geral
            toggleLuzGeral(action, request);
        } else {
            request->send(400, "text/plain", "Botão inválido!");
        }
    });
}


void initLittleFS() {
    if (!LittleFS.begin()) {
        Serial.println("Erro ao iniciar LittleFS. O sistema de arquivos não pôde ser montado.");
        return;
    }
    Serial.println("LittleFS inicializado com sucesso.");
}

bool readEstadoLuz(const String &arquivoEstado) {
    File file = LittleFS.open(arquivoEstado, "r");
    if (!file) {
        Serial.println("Arquivo de estado não encontrado, assumindo estado desligado.");
        return false;
    }

    String state = file.readStringUntil('\n');
    file.close();
    bool estado = state.toInt() == 1;
    return estado;
}

void saveEstadoLuz(const String &arquivoEstado, bool state) {
    File file = LittleFS.open(arquivoEstado, "w");
    if (!file) {
        Serial.println("Erro ao abrir o arquivo para escrita.");
        return;
    }

    file.println(state ? "1" : "0");
    file.close();
}

void setupLigaDesliga(AsyncWebServer &server) {
    Serial.println("Configurando o servidor Web para controle das luzes.");

    initLittleFS();

    pinMode(pinoLuzCasa, OUTPUT);
    pinMode(pinoLuzRua, OUTPUT);
    pinMode(pinoLuzPasto, OUTPUT);

    luzCasaLigada = readEstadoLuz(arquivoEstadoCasa);
    luzRuaLigada = readEstadoLuz(arquivoEstadoRua);
    luzPastoLigada = readEstadoLuz(arquivoEstadoPasto);
    luzGeralLigada = readEstadoLuz(arquivoEstadoGeral);

    digitalWrite(pinoLuzCasa, luzCasaLigada ? HIGH : LOW);
    digitalWrite(pinoLuzRua, luzRuaLigada ? HIGH : LOW);
    digitalWrite(pinoLuzPasto, luzPastoLigada ? HIGH : LOW);

    handleToggleAction(server);

    Serial.println("Configuração concluída.");
}

void toggleLuzCasa(String action, AsyncWebServerRequest *request) {
    if (action == "ligar") {
        luzCasaLigada = true;
        digitalWrite(pinoLuzCasa, HIGH);
        request->send(200, "text/plain", "Luz da Casa ligada!");
    } else if (action == "desligar") {
        luzCasaLigada = false;
        digitalWrite(pinoLuzCasa, LOW);
        request->send(200, "text/plain", "Luz da Casa desligada!");
    } else {
        request->send(400, "text/plain", "Ação inválida!");
    }
    saveEstadoLuz(arquivoEstadoCasa, luzCasaLigada);
}

void toggleLuzRua(String action, AsyncWebServerRequest *request) {
    if (action == "ligar") {
        luzRuaLigada = true;
        digitalWrite(pinoLuzRua, HIGH);
        request->send(200, "text/plain", "Luz da Rua ligada!");
    } else if (action == "desligar") {
        luzRuaLigada = false;
        digitalWrite(pinoLuzRua, LOW);
        request->send(200, "text/plain", "Luz da Rua desligada!");
    } else {
        request->send(400, "text/plain", "Ação inválida!");
    }
    saveEstadoLuz(arquivoEstadoRua, luzRuaLigada);
}

void toggleLuzPasto(String action, AsyncWebServerRequest *request) {
    if (action == "ligar") {
        luzPastoLigada = true;
        digitalWrite(pinoLuzPasto, HIGH);
        request->send(200, "text/plain", "Luz do Pasto ligada!");
    } else if (action == "desligar") {
        luzPastoLigada = false;
        digitalWrite(pinoLuzPasto, LOW);
        request->send(200, "text/plain", "Luz do Pasto desligada!");
    } else {
        request->send(400, "text/plain", "Ação inválida!");
    }
    saveEstadoLuz(arquivoEstadoPasto, luzPastoLigada);
}

void toggleLuzGeral(String action, AsyncWebServerRequest *request) {
    if (action == "ligar") {
        luzGeralLigada = true;
        digitalWrite(pinoLuzCasa, HIGH);
        digitalWrite(pinoLuzRua, HIGH);
        digitalWrite(pinoLuzPasto, HIGH);
        request->send(200, "text/plain", "Luz Geral ligada! Todas as luzes foram ligadas.");
    } else if (action == "desligar") {
        luzGeralLigada = false;
        digitalWrite(pinoLuzCasa, LOW);
        digitalWrite(pinoLuzRua, LOW);
        digitalWrite(pinoLuzPasto, LOW);
        request->send(200, "text/plain", "Luz Geral desligada! Todas as luzes foram desligadas.");
    } else {
        request->send(400, "text/plain", "Ação inválida!");
    }
    saveEstadoLuz(arquivoEstadoGeral, luzGeralLigada);
}
