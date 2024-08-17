#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "localizacao.h"

const char *API_KEY = "0e8b513d65e2ea28b67c6091e42ae317"; // Substitua pela sua chave da API do OpenWeather
const char *filePath = "/localizacao.txt";

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 6 * 60 * 60 * 1000; // 6 horas em milissegundos

// Função para converter timestamp Unix para string legível
String timeStampToString(unsigned long timestamp)
{
    // Ajuste de timezone aqui, se necessário
    time_t rawtime = timestamp;
    struct tm *ti;
    ti = gmtime(&rawtime);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ti);
    return String(buffer);
}

// Função para armazenar nascer e pôr do sol em um arquivo
void storeSunTimes(const String &local, const String &sunrise, const String &sunset)
{
    Serial.println("Armazenando horários no arquivo...");

    if (LittleFS.exists(filePath))
    {
        Serial.println("Arquivo existente encontrado, removendo...");
        LittleFS.remove(filePath); // Remove o arquivo anterior
    }

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

void setupLocationPage(AsyncWebServer &server)
{
    server.on("/definir-horarios", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (!request->hasParam("local")) {
            Serial.println("Parâmetro 'local' não fornecido.");
            request->send(400, "application/json", "{\"success\": false, \"message\":\"Parâmetro 'local' é necessário\"}");
            return;
        }

        String local = request->getParam("local")->value();
        Serial.print("Solicitação para definir horários com a localização: ");
        Serial.println(local);

        // Verifica se o usuário solicitou a exclusão da cidade
        if (local.equalsIgnoreCase("Excluir")) {
            if (LittleFS.exists(filePath)) {
                LittleFS.remove(filePath);
                Serial.println("Localização excluída e atualização automática desativada.");
                request->send(200, "application/json", "{\"success\": true, \"message\":\"Localização excluída e atualização automática desativada.\"}");
            } else {
                Serial.println("Nenhuma cidade armazenada para excluir.");
                request->send(404, "application/json", "{\"success\": false, \"message\":\"Nenhuma cidade armazenada para excluir.\"}");
            }
            return;
        }

        // Divide o parâmetro local em cidade e estado
        int hyphenIndex = local.indexOf('-');
        if (hyphenIndex == -1) {
            Serial.println("Formato de local inválido. Use 'cidade-estado'.");
            request->send(400, "application/json", "{\"success\": false, \"message\":\"Formato de local inválido. Use 'cidade-estado'\"}");
            return;
        }

        String city = local.substring(0, hyphenIndex);
        String state = local.substring(hyphenIndex + 1);

        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + state + ",BR&appid=" + String(API_KEY);
        Serial.print("Consultando URL: ");
        Serial.println(url);

        HTTPClient http;
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Resposta da API: ");
            Serial.println(payload);

            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.println("Falha ao analisar JSON: " + String(error.c_str()));
                request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao processar resposta da API.\"}");
                return;
            }

            if (doc.containsKey("sys")) {
                unsigned long sunrise = doc["sys"]["sunrise"];
                unsigned long sunset = doc["sys"]["sunset"];

                // Converte timestamps para strings legíveis
                String sunriseStr = timeStampToString(sunrise).substring(11, 16); // Horário no formato HH:MM
                String sunsetStr = timeStampToString(sunset).substring(11, 16); // Horário no formato HH:MM

                // Armazena no arquivo
                storeSunTimes(local, sunriseStr, sunsetStr);

                // Feedback via Serial
                Serial.println("Localização: " + local);
                Serial.println("Nascer do sol: " + sunriseStr);
                Serial.println("Pôr do sol: " + sunsetStr);

                String responseJson = "{\"success\": true, \"localizacao\": {\"cidade\": \"" + local + "\", \"nascerDoSol\": \"" + sunriseStr + "\", \"porDoSol\": \"" + sunsetStr + "\"}}";
                request->send(200, "application/json", responseJson);
            } else {
                Serial.println("Localização não encontrada na resposta da API.");
                request->send(400, "application/json", "{\"success\": false, \"message\":\"Localização não encontrada\"}");
            }
        } else {
            Serial.print("Erro ao acessar API. Código de resposta: ");
            Serial.println(httpResponseCode);
            request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao acessar API\"}");
        }
        http.end(); });

    server.on("/horarios-armazenados", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Solicitação recebida para horários armazenados.");

        if (!LittleFS.exists(filePath)) {
            Serial.println("Nenhum arquivo de horários encontrado.");
            request->send(404, "application/json", "{\"success\": false, \"message\":\"Nenhuma cidade armazenada\"}");
            return;
        }

        File file = LittleFS.open(filePath, "r");
        String fileContent = file.readString();
        file.close();

        Serial.println("Horários armazenados retornados com sucesso.");
        request->send(200, "application/json", fileContent); });
}

void checkAndUpdateSunTimes()
{
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval)
    {
        Serial.println("Iniciando verificação e atualização de horários...");

        if (!LittleFS.exists(filePath))
        {
            Serial.println("Nenhum local armazenado. Ignorando atualização.");
            return;
        }

        File file = LittleFS.open(filePath, "r");
        String fileContent = file.readString();
        file.close();

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, fileContent);
        if (error)
        {
            Serial.println("Falha ao analisar o arquivo de localização: " + String(error.c_str()));
            return;
        }

        if (!doc.containsKey("local"))
        {
            Serial.println("Arquivo de local inválido.");
            return;
        }

        String local = doc["local"].as<String>();

        // Divide o local em cidade e estado
        int hyphenIndex = local.indexOf('-');
        if (hyphenIndex == -1)
        {
            Serial.println("Formato de local inválido no arquivo.");
            return;
        }

        String city = local.substring(0, hyphenIndex);
        String state = local.substring(hyphenIndex + 1);

        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + state + ",BR&appid=" + String(API_KEY);
        Serial.print("Consultando URL para atualização: ");
        Serial.println(url);

        HTTPClient http;
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            Serial.println("Resposta da API para atualização: ");
            Serial.println(payload);

            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, payload);
            if (error)
            {
                Serial.println("Falha ao analisar resposta da API: " + String(error.c_str()));
                return;
            }

            if (doc.containsKey("sys"))
            {
                unsigned long sunrise = doc["sys"]["sunrise"];
                unsigned long sunset = doc["sys"]["sunset"];

                String sunriseStr = timeStampToString(sunrise).substring(11, 16); // Horário no formato HH:MM
                String sunsetStr = timeStampToString(sunset).substring(11, 16);   // Horário no formato HH:MM

                storeSunTimes(local, sunriseStr, sunsetStr);

                Serial.println("Horários de nascer e pôr do sol atualizados para " + local);
            }
            else
            {
                Serial.println("Localização não encontrada na resposta da API.");
            }
        }
        else
        {
            Serial.print("Erro ao acessar API. Código de resposta: ");
            Serial.println(httpResponseCode);
        }
        http.end();

        lastUpdateTime = currentTime; // Atualiza o tempo da última atualização
    }
}
