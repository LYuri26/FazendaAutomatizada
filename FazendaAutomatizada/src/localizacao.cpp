#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "localizacao.h"
#include "autenticador.h"
#include "dashboard.h"

const char *API_KEY = "0e8b513d65e2ea28b67c6091e42ae317"; // Substitua pela sua chave da API do OpenWeather
const char *filePath = "/localizacao.txt";

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 6 * 60 * 60 * 1000; // 6 horas em milissegundos

String timeStampToBrasiliaTime(unsigned long timestamp)
{
    time_t rawtime = timestamp;
    struct tm *ti = gmtime(&rawtime);
    ti->tm_hour -= 3; // Ajusta para o horário de Brasília (UTC-3)
    mktime(ti);       // Corrige a estrutura tm
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ti);
    return String(buffer);
}

void storeSunTimes(const String &local, const String &sunrise, const String &sunset)
{
    File file = LittleFS.open(filePath, "w");
    if (file)
    {
        String jsonContent = "{\"local\":\"" + local + "\",\"sunrise\":\"" + sunrise + "\",\"sunset\":\"" + sunset + "\"}";
        file.print(jsonContent);
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
        else if (c == '-' || c == '_' || c == '.' || c == '~' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
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

        // Codifica o nome da cidade e estado para URL
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

            storeSunTimes(local, nascerDoSol, porDoSol);

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
    if (millis() - lastUpdateTime < updateInterval)
        return;

    if (!LittleFS.exists(filePath))
    {
        Serial.println("Nenhum local armazenado. Ignorando atualização.");
        return;
    }

    File file = LittleFS.open(filePath, "r");
    String fileContent = file.readString();
    file.close();

    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, fileContent) || !doc.containsKey("local"))
    {
        Serial.println("Falha ao analisar o arquivo de localização.");
        return;
    }

    String local = doc["local"].as<String>();
    int hyphenIndex = local.indexOf('-');
    if (hyphenIndex == -1)
    {
        Serial.println("Formato de local inválido no arquivo.");
        return;
    }

    String cidade = local.substring(0, hyphenIndex);
    String estado = local.substring(hyphenIndex + 1);

    // Codifica o nome da cidade e estado para URL
    String cidadeCodificada = encodeURIComponent(cidade);
    String estadoCodificado = encodeURIComponent(estado);

    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + cidadeCodificada + "," + estadoCodificado + ",BR&appid=" + String(API_KEY) + "&units=metric";
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        if (deserializeJson(doc, payload))
        {
            Serial.println("Falha ao analisar resposta da API.");
            return;
        }

        if (doc.containsKey("sys"))
        {
            String sunriseBrasilia = timeStampToBrasiliaTime(doc["sys"]["sunrise"].as<unsigned long>()).substring(11, 16);
            String sunsetBrasilia = timeStampToBrasiliaTime(doc["sys"]["sunset"].as<unsigned long>()).substring(11, 16);
            storeSunTimes(local, sunriseBrasilia, sunsetBrasilia);
            Serial.println("Horários atualizados para " + local);
        }
        else
        {
            Serial.println("Localização não encontrada na resposta da API.");
        }
    }
    else
    {
        Serial.println("Erro ao acessar API. Código de resposta: " + String(httpResponseCode));
    }
    http.end();

    lastUpdateTime = millis(); // Atualiza o tempo da última atualização
}
