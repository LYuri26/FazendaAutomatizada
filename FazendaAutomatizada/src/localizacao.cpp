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

// Função que converte timestamp para o horário de Brasília
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

// Função para armazenar os horários de nascer e pôr do sol
void storeSunTimes(const String &sunrise, const String &sunset)
{
    File file = LittleFS.open(filePath, "w");
    if (file)
    {
        // Salva os horários no formato desejado
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

// Função para codificar URL (substituto para encodeURIComponent)
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

// Função para configurar o endpoint /definir-horarios
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
            DynamicJsonDocument jsonDoc(2048); // Usando DynamicJsonDocument ao invés de JsonDocument
            DeserializationError error = deserializeJson(jsonDoc, payload);
            if (error) {
                Serial.println("Falha ao analisar resposta da API: " + String(error.c_str()));
                request->send(500, "application/json", "{\"success\": false, \"message\":\"Erro ao analisar resposta da API.\"}");
                return;
            }

            String nascerDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunrise"].as<unsigned long>()).substring(11, 16);
            String porDoSol = timeStampToBrasiliaTime(jsonDoc["sys"]["sunset"].as<unsigned long>()).substring(11, 16);

            // Salva os horários no formato desejado
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

// Função para verificar e atualizar os horários de nascer e pôr do sol
void checkAndUpdateSunTimes()
{
    // Verifica se o intervalo de atualização foi atingido
    if (millis() - lastUpdateTime < updateInterval)
        return;

    // Verifica se o arquivo de localização existe
    if (!LittleFS.exists(filePath))
    {
        Serial.println("Nenhum local armazenado. Ignorando atualização.");
        return;
    }

    // Abre o arquivo para leitura
    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Falha ao abrir o arquivo.");
        return;
    }

    // Lê o conteúdo do arquivo
    String fileContent = file.readString();
    file.close();

    Serial.println("Linha lida do arquivo: " + fileContent);

    // Cria um documento JSON e analisa o conteúdo do arquivo
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error)
    {
        Serial.println("Falha ao analisar o arquivo de localização: " + String(error.c_str()));
        return;
    }

    // Verifica se os campos 'local', 'sunrise' e 'sunset' estão presentes
    if (!doc.containsKey("local") || !doc.containsKey("sunrise") || !doc.containsKey("sunset"))
    {
        Serial.println("Falha ao encontrar dados necessários no arquivo.");
        return;
    }

    // Obtém os dados do arquivo
    String local = doc["local"].as<String>();
    String sunrise = doc["sunrise"].as<String>();
    String sunset = doc["sunset"].as<String>();

    Serial.println("Local: " + local);
    Serial.println("Horário de nascer do sol: " + sunrise);
    Serial.println("Horário de pôr do sol: " + sunset);

    // Se necessário, você pode fazer uma atualização com base nos dados atuais
    // Atualizar o horário de nascer e pôr do sol com base na API

    // Atualiza o tempo da última atualização
    lastUpdateTime = millis();
}