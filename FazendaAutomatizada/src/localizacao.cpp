#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "localizacao.h"
#include "autenticador.h"
#include "dashboard.h"
#include "tempo.h"
#include "tela.h" // Incluir o cabeçalho da tela

const char *API_KEY = "0e8b513d65e2ea28b67c6091e42ae317"; // Substitua pela sua chave da API
const char *filePath = "/localizacao.txt";

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2 * 60 * 60 * 1000; // Atualizar a cada 2 horas

String cidadeSalva = "";
String nascerDoSol = "";
String porDoSol = "";

String timeStampToBrasiliaTime(unsigned long timestamp)
{
    time_t rawtime = (time_t)timestamp;
    struct tm *ti = localtime(&rawtime);
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", ti);
    return String(buffer);
}

void storeSunTimes(const String &sunrise, const String &sunset, const String &cidade)
{
    File file = LittleFS.open(filePath, "w");
    if (file)
    {
        file.println("{\"cidade\":\"" + cidade + "\",\"sunrise\":\"" + sunrise + "\",\"sunset\":\"" + sunset + "\"}");
        file.close();
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
        else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            encoded += c;
        }
        else
        {
            encoded += '%';
            char hex[3];
            sprintf(hex, "%02X", (unsigned char)c);
            encoded += hex;
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
            request->send(400, "application/json", "{\"success\": false, \"message\":\"Parâmetro 'local' é necessário\"}");
            return;
        }

        String local = request->getParam("local")->value();

        if (local.equalsIgnoreCase("Excluir")) {
            if (LittleFS.exists(filePath)) {
                LittleFS.remove(filePath);
                request->send(200, "application/json", "{\"success\": true, \"message\":\"Localização excluída.\"}");
                return;
            } else {
                request->send(404, "application/json", "{\"success\": false, \"message\":\"Nenhuma cidade armazenada para excluir.\"}");
                return;
            }
        }

        int hyphenIndex = local.indexOf('-');
        if (hyphenIndex == -1) {
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
                request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao analisar resposta da API.\"}");
                return;
            }

            String sunriseTime = timeStampToBrasiliaTime(jsonDoc["sys"]["sunrise"].as<unsigned long>());
            String sunsetTime = timeStampToBrasiliaTime(jsonDoc["sys"]["sunset"].as<unsigned long>());

            storeSunTimes(sunriseTime, sunsetTime, cidade);

            // Atualiza as variáveis globais
            cidadeSalva = cidade;
            nascerDoSol = sunriseTime;
            porDoSol = sunsetTime;
            String response = "{\"success\": true, \"localizacao\": {\"cidade\": \"" + cidade + "\", \"nascerDoSol\": \"" + sunriseTime + "\", \"porDoSol\": \"" + sunsetTime + "\"}}";
            request->send(200, "application/json", response);
        } else {
            request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao buscar dados da API.\"}");
        } });
}

void checkAndUpdateSunTimes()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime < updateInterval)
        return; // Atualiza a cada 2 horas

    if (!LittleFS.exists(filePath))
    {
        lastUpdateTime = currentMillis;
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        lastUpdateTime = currentMillis;
        return;
    }

    String fileContent = file.readString();
    file.close();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error)
    {
        lastUpdateTime = currentMillis;
        return;
    }

    if (!doc.containsKey("cidade"))
    {
        lastUpdateTime = currentMillis;
        return;
    }

    cidadeSalva = doc["cidade"].as<String>();

    String cidadeCodificada = encodeURIComponent(cidadeSalva);
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
            lastUpdateTime = currentMillis;
            return;
        }

        nascerDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunrise"].as<unsigned long>());
        porDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunset"].as<unsigned long>());

        storeSunTimes(nascerDoSol, porDoSol, cidadeSalva);

        // Exibe apenas as informações essenciais
        Serial.println("Horários atualizados para a cidade: " + cidadeSalva);
        Serial.println("Nascer do sol: " + nascerDoSol);
        Serial.println("Pôr do sol: " + porDoSol);
    }

    lastUpdateTime = currentMillis;
}

void obterDadosLocalizacao()
{
    if (!LittleFS.exists(filePath))
    {
        return;
    }

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        return;
    }

    String fileContent = file.readString();
    file.close();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error)
    {
        return;
    }

    if (!doc.containsKey("cidade"))
    {
        return;
    }

    cidadeSalva = doc["cidade"].as<String>();
    nascerDoSol = doc["sunrise"].as<String>();
    porDoSol = doc["sunset"].as<String>();
}
