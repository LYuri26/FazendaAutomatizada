#include <WiFi.h>
#include <time.h>
#include "tempo.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // UTC-3 (Horário de Brasília)
const int daylightOffset_sec = 0;  // Sem horário de verão

void setupTimeClient()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTimeClient()
{
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        char timeString[30];
        strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
        return String(timeString);
    }
    else
    {
        return "Erro ao obter hora";
    }
}

String getHoraInterna()
{
    return getTimeClient();
}

void updateTime()
{
    struct tm timeInfo;
    static int lastMinute = -1; // Mova lastMinute para ser estático dentro da função
    if (getLocalTime(&timeInfo))
    {
        int currentMinute = timeInfo.tm_min;

        // Atualiza e imprime a hora apenas se o minuto mudar
        if (currentMinute != lastMinute)
        {
            lastMinute = currentMinute;

            char timeString[30];
            strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
            String currentTime = String(timeString);
        }
    }
}

void setTimeFromNTP()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        char timeString[30];
        strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
        String currentTime = String(timeString);
        Serial.println("Hora interna configurada: " + currentTime);
    }
    else
    {
        Serial.println("Falha ao obter a hora com NTP.");
    }
}
