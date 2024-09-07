// tempo.cpp
#include <WiFi.h>
#include <time.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // UTC-3 (Horário de Brasília)
const int daylightOffset_sec = 0;  // Sem horário de verão

String currentTime = "";
int lastMinute = -1;

void updateTime();
void setTimeFromNTP();
void printInternalTime();

void setupTimeClient()
{
    setTimeFromNTP(); // Atualiza a hora interna inicialmente
    updateTime();     // Atualiza a variável currentTime
    Serial.println("Hora inicial configurada: " + currentTime);
}

String getTimeClient()
{
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        char timeString[9];
        strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeInfo);
        return String(timeString);
    }
    else
    {
        return "";
    }
}

void updateTime()
{
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        int currentMinute = timeInfo.tm_min;

        // Atualiza e imprime a hora apenas se o minuto mudar
        if (currentMinute != lastMinute)
        {
            lastMinute = currentMinute;

            char timeString[30];
            strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
            currentTime = String(timeString);

            // Exibe a hora atualizada
            Serial.println("Hora atual: " + currentTime);
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
        currentTime = String(timeString);
        Serial.println("Hora interna configurada: " + currentTime);
    }
    else
    {
        Serial.println("Falha ao obter a hora com NTP.");
    }
}

void printInternalTime()
{
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        char timeString[30];
        strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);
        Serial.println("Hora interna do ESP32: " + String(timeString));
    }
    else
    {
        Serial.println("Falha ao obter a hora interna.");
    }
}
