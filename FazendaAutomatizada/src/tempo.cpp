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

String getTimeClient() {
    // Exemplo de como você pode obter a hora atual de um cliente NTP
    time_t now;
    struct tm *timeinfo;
    char buffer[6];

    time(&now);
    timeinfo = localtime(&now);

    strftime(buffer, 6, "%H:%M", timeinfo);
    return String(buffer);
}

String getHoraInterna()
{
    return getTimeClient();
}

void updateTime()
{
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        char timeString[30];
        strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
        String currentTime = String(timeString);
        Serial.println("Hora atual: " + currentTime); // Imprime a hora atual para depuração
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
