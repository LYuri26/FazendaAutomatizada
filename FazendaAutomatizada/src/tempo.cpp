#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // UTC-3 (Horário de Brasília)
const int daylightOffset_sec = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, 60000);

String currentTime = "";

void updateTime();
void setTimeFromNTP();
void printInternalTime();

void setupTimeClient()
{
    timeClient.begin();
    updateTime();
    setTimeFromNTP();
    Serial.print("Hora inicial da Internet: ");
    Serial.println(currentTime);
}

String getTimeClient()
{
    timeClient.update();
    return timeClient.getFormattedTime();
}

void updateTime()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        timeClient.update();
        unsigned long epochTime = timeClient.getEpochTime();
        time_t time = static_cast<time_t>(epochTime);
        struct tm timeInfo;
        localtime_r(&time, &timeInfo);
        char timeString[30];
        snprintf(timeString, sizeof(timeString), "%02d-%02d-%04d %02d:%02d:%02d",
                 timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900,
                 timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
        currentTime = String(timeString);

        Serial.println("Hora atual da Internet: " + currentTime);
    }
    else
    {
        Serial.println("WiFi desconectado. Não é possível atualizar o tempo.");
    }
}

void setTimeFromNTP()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        struct tm timeInfo;
        if (getLocalTime(&timeInfo))
        {
            char timeString[30];
            snprintf(timeString, sizeof(timeString), "%02d-%02d-%04d %02d:%02d:%02d",
                     timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900,
                     timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
            Serial.print("Hora interna configurada para: ");
            Serial.println(timeString);
        }
        else
        {
            Serial.println("Falha ao obter hora NTP.");
        }
    }
    else
    {
        Serial.println("WiFi desconectado. Não foi possível configurar a hora interna.");
    }
}

void printInternalTime()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char timeString[30];
    snprintf(timeString, sizeof(timeString), "%02d-%02d-%04d %02d:%02d:%02d",
             timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.println("Hora interna do ESP32: " + String(timeString));
}
