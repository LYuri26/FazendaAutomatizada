#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "autenticador.h"
#include "ligadesliga.h"
#include "tempo.h"
#include "localizacao.h"

// Definindo pinos
const int pinoLuzCasa = 26;
const int pinoLuzRua = 27;
const int pinoLuzPasto = 14;

// Definindo arquivos
const String arquivoEstadoLuz[] = {
    "/estadoLuzCasa.txt",
    "/estadoLuzRua.txt",
    "/estadoLuzPasto.txt",
    "/estadoLuzGeral.txt"};
const String filePath = "/localizacao.txt";
const String arquivoHorarios = "/horarios.txt";

// Estado das luzes
bool luzEstado[3] = {false, false, false};
bool luzGeralEstado = false;

void initLittleFS() {
    if (!LittleFS.begin()) {
        Serial.println("Falha ao iniciar LittleFS.");
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (!LittleFS.exists(arquivoEstadoLuz[i])) {
            File file = LittleFS.open(arquivoEstadoLuz[i], "w");
            if (file) {
                file.println("0");
                file.close();
                Serial.println("Arquivo criado: " + arquivoEstadoLuz[i]);
            }
        }
    }
}

bool readEstadoLuz(int index) {
    File file = LittleFS.open(arquivoEstadoLuz[index], "r");
    if (!file) return false;
    String state = file.readStringUntil('\n');
    file.close();
    return state.toInt() == 1;
}

void saveEstadoLuz(int index, bool state) {
    File file = LittleFS.open(arquivoEstadoLuz[index], "w");
    if (file) {
        file.println(state ? "1" : "0");
        file.close();
        Serial.println("Estado da luz " + String(index) + " salvo como " + String(state ? "ligada" : "desligada"));
    }
}

void toggleLuz(int index, String action, AsyncWebServerRequest *request) {
    bool estado = (action == "ligar");
    int pin;
    
    switch (index) {
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
            for (int i = 0; i < 3; i++) {
                luzEstado[i] = luzGeralEstado;
                pin = (i == 0) ? pinoLuzCasa : (i == 1) ? pinoLuzRua : pinoLuzPasto;
                digitalWrite(pin, luzGeralEstado ? HIGH : LOW);
                saveEstadoLuz(i, luzGeralEstado);
            }
            pin = -1; // Não aplicar para luz geral diretamente
            saveEstadoLuz(3, luzGeralEstado);
            break;
        default:
            pin = -1; // Índice inválido
            break;
    }
    
    if (pin != -1) {
        digitalWrite(pin, estado ? HIGH : LOW);
        saveEstadoLuz(index, estado);
    }
    
    if (request) {
        request->send(200, "text/plain", "Luz " + String(index) + " " + (estado ? "ligada" : "desligada") + "!");
    }
    Serial.println("Luz " + String(index) + " " + (estado ? "ligada" : "desligada"));
}

void handleToggleAction(AsyncWebServer &server) {
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
        String action = request->getParam("action") ? request->getParam("action")->value() : "";
        String idParam = request->getParam("id") ? request->getParam("id")->value() : "";
        int id = idParam.toInt();
        
        if (action.isEmpty() || id < 0 || id >= 4) {
            request->send(400, "text/plain", "Parâmetros inválidos!");
            Serial.println("Parâmetros inválidos!");
            return;
        }
        
        toggleLuz(id, action, request);
    });
}

int timeToMinutes(String time) {
    int hours = time.substring(0, 2).toInt();
    int minutes = time.substring(3, 5).toInt();
    return hours * 60 + minutes;
}

void checkSunTimes() {
    if (!LittleFS.exists(filePath)) {
        Serial.println("Nenhuma localização armazenada. Ignorando a verificação.");
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file) {
        Serial.println("Falha ao abrir o arquivo de localização.");
        return;
    }

    String jsonString = file.readString();
    file.close();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print("Falha ao analisar JSON: ");
        Serial.println(error.c_str());
        return;
    }

    String sunrise = doc["sunrise"].as<String>();
    String sunset = doc["sunset"].as<String>();

    if (sunrise.isEmpty() || sunset.isEmpty()) {
        Serial.println("Falha ao encontrar horários de nascer ou pôr do sol no arquivo.");
        return;
    }

    String currentTime = getTimeClient(); // Certifique-se de que a função retorna no formato HH:MM
    if (currentTime.length() < 5) {
        Serial.println("Formato da hora atual inválido: " + currentTime);
        return;
    }

    currentTime = currentTime.substring(0, 5); // Formato HH:MM

    int currentMinutes = timeToMinutes(currentTime);
    int sunriseMinutes = timeToMinutes(sunrise);
    int sunsetMinutes = timeToMinutes(sunset);

    Serial.println("Hora atual: " + currentTime);
    Serial.println("Nascer do sol: " + sunrise);
    Serial.println("Pôr do sol: " + sunset);
    Serial.println("Hora atual em minutos: " + String(currentMinutes));
    Serial.println("Nascer do sol em minutos: " + String(sunriseMinutes));
    Serial.println("Pôr do sol em minutos: " + String(sunsetMinutes));

    if (currentMinutes == sunriseMinutes) {
        if (!luzGeralEstado) {
            toggleLuz(3, "ligar", nullptr);
            Serial.println("Luzes ligadas automaticamente com base no horário do nascer do sol.");
        }
    } else if (currentMinutes == sunsetMinutes) {
        if (luzGeralEstado) {
            toggleLuz(3, "desligar", nullptr);
            Serial.println("Luzes desligadas automaticamente com base no horário do pôr do sol.");
        }
    }
}

void saveHorarios(String ligar, String desligar) {
    File file = LittleFS.open(arquivoHorarios, "w");
    if (file) {
        file.println(ligar);
        file.println(desligar);
        file.close();
        Serial.println("Horários salvos: Ligar - " + ligar + ", Desligar - " + desligar);
    } else {
        Serial.println("Falha ao salvar os horários.");
    }
}

void disableHorarios() {
    if (LittleFS.exists(arquivoHorarios)) {
        LittleFS.remove(arquivoHorarios);
        Serial.println("Horários desativados.");
    } else {
        Serial.println("Nenhum horário para desativar.");
    }
}

void loadHorarios() {
    if (!LittleFS.exists(arquivoHorarios)) {
        Serial.println("Arquivo de horários não encontrado.");
        return;
    }

    File file = LittleFS.open(arquivoHorarios, "r");
    if (!file) {
        Serial.println("Falha ao abrir o arquivo de horários.");
        return;
    }

    String ligar = file.readStringUntil('\n');
    String desligar = file.readStringUntil('\n');
    file.close();

    Serial.println("Horários carregados: Ligar - " + ligar + ", Desligar - " + desligar);
}

void handleHorariosActions(AsyncWebServer &server) {
    server.on("/salvar-horarios", HTTP_GET, [](AsyncWebServerRequest *request) {
        String ligar = request->getParam("ligar") ? request->getParam("ligar")->value() : "";
        String desligar = request->getParam("desligar") ? request->getParam("desligar")->value() : "";

        if (ligar.isEmpty() || desligar.isEmpty()) {
            request->send(400, "text/plain", "Parâmetros inválidos!");
            Serial.println("Parâmetros inválidos!");
            return;
        }

        saveHorarios(ligar, desligar);
        request->send(200, "text/plain", "Horários salvos com sucesso!");
    });

    server.on("/desativar-horarios", HTTP_GET, [](AsyncWebServerRequest *request) {
        disableHorarios();
        request->send(200, "text/plain", "Horários desativados com sucesso!");
    });
}

void checkUserDefinedTimes() {
    if (!LittleFS.exists(arquivoHorarios)) {
        Serial.println("Arquivo de horários não encontrado.");
        return;
    }

    File file = LittleFS.open(arquivoHorarios, "r");
    if (!file) {
        Serial.println("Falha ao abrir o arquivo de horários.");
        return;
    }

    String ligar = file.readStringUntil('\n');
    String desligar = file.readStringUntil('\n');
    file.close();

    String currentTime = getTimeClient(); // Certifique-se de que a função retorna no formato HH:MM
    if (currentTime.length() < 5) {
        Serial.println("Formato da hora atual inválido: " + currentTime);
        return;
    }

    int currentMinutes = timeToMinutes(currentTime);
    int ligarMinutes = timeToMinutes(ligar);
    int desligarMinutes = timeToMinutes(desligar);

    Serial.println("Hora atual: " + currentTime);
    Serial.println("Horário de ligar: " + ligar);
    Serial.println("Horário de desligar: " + desligar);
    Serial.println("Hora atual em minutos: " + String(currentMinutes));
    Serial.println("Horário de ligar em minutos: " + String(ligarMinutes));
    Serial.println("Horário de desligar em minutos: " + String(desligarMinutes));

    if (currentMinutes == ligarMinutes) {
        if (!luzGeralEstado) {
            toggleLuz(3, "ligar", nullptr);
            Serial.println("Luzes ligadas automaticamente com base no horário do usuário.");
        }
    } else if (currentMinutes == desligarMinutes) {
        if (luzGeralEstado) {
            toggleLuz(3, "desligar", nullptr);
            Serial.println("Luzes desligadas automaticamente com base no horário do usuário.");
        }
    }
}

void setupLigaDesliga(AsyncWebServer &server) {
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

    Serial.println("Estado inicial das luzes: Casa " + String(luzEstado[0] ? "ligada" : "desligada") +
                   ", Rua " + String(luzEstado[1] ? "ligada" : "desligada") +
                   ", Pasto " + String(luzEstado[2] ? "ligada" : "desligada") +
                   ", Geral " + String(luzGeralEstado ? "ligada" : "desligada"));

    handleToggleAction(server);
    handleHorariosActions(server);
}
