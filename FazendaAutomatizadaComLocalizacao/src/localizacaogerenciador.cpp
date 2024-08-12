#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "localizacaogerenciador.h"
#include "localizacaointerface.h"

// Configurações da API
const String GEOCODING_API_KEY = "0e8b513d65e2ea28b67c6091e42ae317";
const String GEOCODING_API_ENDPOINT = "http://api.openweathermap.org/data/2.5/weather?q=";
const String CITY_SUGGESTIONS_API_ENDPOINT = "http://api.openweathermap.org/data/2.5/find?q=";
const String SUN_TIMES_API_ENDPOINT = "https://api.sunrise-sunset.org/json?formatted=0";

// Função para codificar a URL
String encodeURIComponent(const String &value) {
    String encoded;
    for (char c : value) {
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            encoded += '%';
            encoded += String(c, HEX);
        }
    }
    return encoded;
}

// Função para tratar erros de HTTP
void handleHttpError(int httpCode) {
    if (httpCode < 0) {
        Serial.printf("HTTP Error: %d\n", httpCode);
    }
}

// Função para fazer uma requisição HTTP e obter o payload
String makeHttpRequest(const String &url) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, url);
    int httpCode = http.GET();
    handleHttpError(httpCode);

    String payload = "{}";
    if (httpCode == 200) {
        payload = http.getString();
    }
    http.end();
    return payload;
}

// Função para buscar a localização em latitude e longitude usando OpenWeatherMap Geocoding API
String geocodeLocation(const String &location) {
    String url = GEOCODING_API_ENDPOINT + encodeURIComponent(location) + "&appid=" + GEOCODING_API_KEY;
    return makeHttpRequest(url);
}

// Função para buscar o horário do nascer e pôr do sol usando a Sunrise-Sunset API
String fetchSunTimes(float lat, float lng) {
    String url = SUN_TIMES_API_ENDPOINT + "&lat=" + String(lat) + "&lng=" + String(lng);
    return makeHttpRequest(url);
}

// Função para buscar sugestões de cidades usando OpenWeatherMap Geocoding API
String fetchCitySuggestions(const String &query) {
    String url = CITY_SUGGESTIONS_API_ENDPOINT + encodeURIComponent(query) + "&appid=" + GEOCODING_API_KEY;
    return makeHttpRequest(url);
}

// Função para lidar com a requisição de localização
void handleLocationRequest(const String &location, AsyncWebServerRequest *request) {
    Serial.print("Procurando localização para: ");
    Serial.println(location);

    String geocodeResponse = geocodeLocation(location);
    StaticJsonDocument<512> tempDoc;
    DeserializationError error = deserializeJson(tempDoc, geocodeResponse);

    if (error) {
        Serial.print("Erro ao analisar JSON de geocodificação: ");
        Serial.println(error.c_str());
        request->send(500, "application/json", "{\"error\":\"Erro ao analisar JSON de geocodificação.\"}");
        return;
    }

    if (!tempDoc["coord"].isNull()) {
        float lat = tempDoc["coord"]["lat"].as<float>();
        float lng = tempDoc["coord"]["lon"].as<float>();

        Serial.print("Latitude: ");
        Serial.println(lat);
        Serial.print("Longitude: ");
        Serial.println(lng);

        String sunTimesJson = fetchSunTimes(lat, lng);
        StaticJsonDocument<512> sunTimesDoc;
        error = deserializeJson(sunTimesDoc, sunTimesJson);

        if (error) {
            Serial.print("Erro ao analisar JSON de horários: ");
            Serial.println(error.c_str());
            request->send(500, "application/json", "{\"error\":\"Erro ao analisar JSON de horários.\"}");
            return;
        }

        String sunrise = sunTimesDoc["results"]["sunrise"].as<String>();
        String sunset = sunTimesDoc["results"]["sunset"].as<String>();

        Serial.print("Horário do nascer do sol: ");
        Serial.println(sunrise);
        Serial.print("Horário do pôr do sol: ");
        Serial.println(sunset);

        request->send(200, "application/json", sunTimesJson);
    } else {
        request->send(404, "application/json", "{\"error\":\"Localização não encontrada.\"}");
    }
}

// Função para lidar com a requisição de sugestões
void handleAutocompleteRequest(const String &query, AsyncWebServerRequest *request) {
    String suggestionsJson = fetchCitySuggestions(query);
    StaticJsonDocument<512> suggestionsDoc;
    DeserializationError error = deserializeJson(suggestionsDoc, suggestionsJson);

    if (error) {
        Serial.print("Erro ao analisar JSON de sugestões: ");
        Serial.println(error.c_str());
        request->send(500, "application/json", "{\"error\":\"Erro ao analisar JSON de sugestões.\"}");
        return;
    }

    JsonArray suggestionsArray = suggestionsDoc["list"].as<JsonArray>();
    String resultJson = "{\"suggestions\":[";
    bool first = true;

    for (JsonVariant suggestion : suggestionsArray) {
        if (!first) {
            resultJson += ",";
        }
        resultJson += "\"" + suggestion["name"].as<String>() + "\"";
        first = false;
    }
    resultJson += "]}";

    request->send(200, "application/json", resultJson);
}
