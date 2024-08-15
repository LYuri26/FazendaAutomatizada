#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "localizacaoInterface.h"
#include "autenticador.h"
#include "localizacaogerenciador.h"

const String apiKey = "0e8b513d65e2ea28b67c6091e42ae317";

void createDefaultLocationFile()
{
    if (LittleFS.begin())
    {
        if (!LittleFS.exists("/localizacao.txt"))
        {
            File file = LittleFS.open("/localizacao.txt", "w");
            if (file)
            {
                file.println("Cidade: ");
                file.println("Nascer do Sol: ");
                file.println("Pôr do Sol: ");
                file.close();
            }
        }
        LittleFS.end();
    }
}

bool readSunriseSunsetTimes(String &sunrise, String &sunset)
{
    if (LittleFS.begin())
    {
        File file = LittleFS.open("/localizacao.txt", "r");
        if (file)
        {
            String line;
            while (file.available())
            {
                line = file.readStringUntil('\n');
                if (line.startsWith("Nascer do Sol: "))
                {
                    sunrise = line.substring(15);
                }
                else if (line.startsWith("Pôr do Sol: "))
                {
                    sunset = line.substring(12);
                }
            }
            file.close();
            LittleFS.end();
            return true;
        }
    }
    return false;
}

void updateSunriseSunsetTimes()
{
    String cidade;
    if (LittleFS.begin())
    {
        File file = LittleFS.open("/localizacao.txt", "r");
        if (file)
        {
            String line;
            while (file.available())
            {
                line = file.readStringUntil('\n');
                if (line.startsWith("Cidade: "))
                {
                    cidade = line.substring(8);
                    break;
                }
            }
            file.close();
        }
        LittleFS.end();
    }

    if (cidade.length() > 0)
    {
        int separatorIndex = cidade.indexOf(" - ");
        if (separatorIndex != -1)
        {
            String cidadeNome = cidade.substring(0, separatorIndex);
            String estado = cidade.substring(separatorIndex + 3);

            String geoApiUrl = "https://api.openweathermap.org/data/2.5/weather?q=" + cidadeNome + "," + estado + ",BR&appid=" + apiKey + "&lang=pt_br";

            HTTPClient http;
            http.begin(geoApiUrl);
            int httpCode = http.GET();

            if (httpCode == 200)
            {
                String payload = http.getString();
                DynamicJsonDocument doc(2048);
                DeserializationError error = deserializeJson(doc, payload);

                if (!error)
                {
                    float latitude = doc["coord"]["lat"];
                    float longitude = doc["coord"]["lon"];
                    String sunriseSunsetApiUrl = "https://api.sunrise-sunset.org/json?lat=" + String(latitude) + "&lng=" + String(longitude) + "&formatted=0";

                    http.begin(sunriseSunsetApiUrl);
                    int sunriseSunsetHttpCode = http.GET();
                    if (sunriseSunsetHttpCode == 200)
                    {
                        String sunriseSunsetPayload = http.getString();
                        DynamicJsonDocument sunriseSunsetDoc(2048);
                        DeserializationError sunriseSunsetError = deserializeJson(sunriseSunsetDoc, sunriseSunsetPayload);

                        if (!sunriseSunsetError)
                        {
                            String sunrise = sunriseSunsetDoc["results"]["sunrise"].as<String>();
                            String sunset = sunriseSunsetDoc["results"]["sunset"].as<String>();

                            if (LittleFS.begin())
                            {
                                File file = LittleFS.open("/localizacao.txt", "w");
                                if (file)
                                {
                                    file.print("Cidade: ");
                                    file.println(cidade);
                                    file.print("Nascer do Sol: ");
                                    file.println(sunrise);
                                    file.print("Pôr do Sol: ");
                                    file.println(sunset);
                                    file.close();
                                }
                                LittleFS.end();
                            }
                        }
                    }
                }
                http.end();
            }
        }
    }
}