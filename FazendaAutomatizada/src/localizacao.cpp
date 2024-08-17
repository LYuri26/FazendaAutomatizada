#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "localizacao.h"
#include "autenticador.h"
#include "dashboard.h"

const char *API_KEY = "0e8b513d65e2ea28b67c6091e42ae317";
const char *filePath = "/localizacao.txt";

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 30 * 60 * 1000;

String timeStampToBrasiliaTime(unsigned long timestamp)
{
    time_t rawtime = timestamp;
    struct tm *ti = gmtime(&rawtime);
    ti->tm_hour -= 3;
    mktime(ti);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ti);
    return String(buffer);
}

void storeSunTimes(const String &sunrise, const String &sunset)
{
    File file = LittleFS.open(filePath, "w");
    if (file)
    {
        file.println("sunrise:" + sunrise);
        file.println("sunset:" + sunset);
        file.close();
        Serial.println("Horários armazenados com sucesso.");
    }
    else
    {
        Serial.println("Falha ao abrir o arquivo para escrita.");
    }
}

String encodeURIComponent(const String &value)
{
    String encoded;
    for (size_t i = 0; i < value.length(); ++i)
    {
        char c = value[i];
        if (c == ' ')
        {
            encoded += '+';
        }
        else if (c == '-' || c == '_' || c == '.' || c == '~' ||
                 (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z'))
        {
            encoded += c;
        }
        else
        {
            encoded += '%';
            encoded += String(c >> 4, HEX);
            encoded += String(c & 0x0F, HEX);
        }
    }
    return encoded;
}

void setupDefinirHorarios(AsyncWebServer &server)
{
    server.on("/definir-horarios", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request);
            return;
        }

        if (!request->hasParam("local")) {
            Serial.println("Parâmetro 'local' não fornecido.");
            request->send(400, "application/json", "{\"success\": false, \"message\":\"Parâmetro 'local' é necessário\"}");
            return;
        }

        String local = request->getParam("local")->value();
        Serial.print("Solicitação para definir horários com a localização: ");
        Serial.println(local);

        if (local.equalsIgnoreCase("Excluir")) {
            if (LittleFS.exists(filePath)) {
                LittleFS.remove(filePath);
                Serial.println("Localização excluída.");
                request->send(200, "application/json", "{\"success\": true, \"message\":\"Localização excluída.\"}");
            } else {
                Serial.println("Nenhuma cidade armazenada para excluir.");
                request->send(404, "application/json", "{\"success\": false, \"message\":\"Nenhuma cidade armazenada para excluir.\"}");
            }
            return;
        }

        int hyphenIndex = local.indexOf('-');
        if (hyphenIndex == -1) {
            Serial.println("Formato de local inválido. Use 'cidade-estado'.");
            request->send(400, "application/json", "{\"success\": false, \"message\":\"Formato inválido para 'local'. Use 'cidade-estado'\"}");
            return;
        }

        String cidade = local.substring(0, hyphenIndex);
        String estado = local.substring(hyphenIndex + 1);

        String cidadeCodificada = encodeURIComponent(cidade);
        String estadoCodificado = encodeURIComponent(estado);

        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + cidadeCodificada + "," + estadoCodificado + ",BR&appid=" + String(API_KEY) + "&units=metric";
        HTTPClient http;
        http.begin(url);
        int httpCode = http.GET();
        String payload = http.getString();
        http.end();

        if (httpCode == 200) {
            DynamicJsonDocument jsonDoc(2048);
            DeserializationError error = deserializeJson(jsonDoc, payload);
            if (error) {
                Serial.println("Falha ao analisar resposta da API: " + String(error.c_str()));
                request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao analisar resposta da API.\"}");
                return;
            }

            String nascerDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunrise"].as<unsigned long>()).substring(11, 16);
            String porDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunset"].as<unsigned long>()).substring(11, 16);

            storeSunTimes(nascerDoSol, porDoSol);

            Serial.println("Horários definidos com sucesso.");
            Serial.println("Nascer do sol: " + nascerDoSol);
            Serial.println("Pôr do sol: " + porDoSol);

            String response = "{\"success\": true, \"localizacao\": {\"cidade\": \"" + cidade + "\", \"nascerDoSol\": \"" + nascerDoSol + "\", \"porDoSol\": \"" + porDoSol + "\"}}";
            request->send(200, "application/json", response);
        } else {
            Serial.print("Erro ao buscar dados da API, código HTTP: ");
            Serial.println(httpCode);
            request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao buscar dados da API.\"}");
        } });
}

void checkAndUpdateSunTimes()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime < updateInterval)
        return;

    if (!LittleFS.exists(filePath))
    {
        Serial.println("Nenhum local armazenado. Ignorando atualização.");
        lastUpdateTime = currentMillis; 
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Falha ao abrir o arquivo.");
        lastUpdateTime = currentMillis; 
        return;
    }

    String fileContent = file.readString();
    file.close();

    Serial.println("Linha lida do arquivo: " + fileContent);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error)
    {
        Serial.println("Falha ao analisar o arquivo de localização: " + String(error.c_str()));
        lastUpdateTime = currentMillis; 
        return;
    }

    if (!doc.containsKey("cidade"))
    {
        Serial.println("Falha ao encontrar dados necessários no arquivo.");
        lastUpdateTime = currentMillis;
        return;
    }

    String cidade = doc["cidade"].as<String>();
    String cidadeCodificada = encodeURIComponent(cidade);
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + cidadeCodificada + ",BR&appid=" + String(API_KEY) + "&units=metric";
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();
    http.end();

    if (httpCode == 200)
    {
        DynamicJsonDocument jsonDoc(2048);
        DeserializationError error = deserializeJson(jsonDoc, payload);
        if (error)
        {
            Serial.println("Falha ao analisar resposta da API: " + String(error.c_str()));
            lastUpdateTime = currentMillis;
            return;
        }

        String nascerDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunrise"].as<unsigned long>()).substring(11, 16);
        String porDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunset"].as<unsigned long>()).substring(11, 16);

        storeSunTimes(nascerDoSol, porDoSol);

        Serial.println("Horários atualizados com sucesso.");
        Serial.println("Nascer do sol: " + nascerDoSol);
        Serial.println("Pôr do sol: " + porDoSol);
    }
    else
    {
        Serial.print("Erro ao buscar dados da API, código HTTP: ");
        Serial.println(httpCode);
    }

    lastUpdateTime = currentMillis;
}
