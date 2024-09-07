#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifiinterface.h"
#include "wifigerenciador.h"

const char *ap_ssid = "FazendaAutomatica";
const char *ap_password = "12345678";
IPAddress local_ip(192, 168, 26, 7);
IPAddress gateway(192, 168, 26, 1);
IPAddress subnet(255, 255, 255, 0);

char ssid[32] = "";
char password[32] = "";

bool isAPMode = false;
bool connectionAttempted = false;

void formatLittleFS()
{
    if (LittleFS.begin())
    {
        if (LittleFS.format())
            Serial.println("Formatação LittleFS concluída.");
        LittleFS.end();
    }
}

bool initializeLittleFS()
{
    if (!LittleFS.begin())
    {
        formatLittleFS();
        if (!LittleFS.begin())
        {
            return false;
        }
    }
    return true;
}

void enterAPMode()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);

    // Exibe o nome do AP e o IP
    Serial.printf("Modo AP ativado\n");
    Serial.printf("AP SSID: %s\n", ap_ssid);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

bool connectToWiFi(const String &ssid, const String &password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    for (int attempts = 0; attempts < 20; ++attempts)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            // Exibe o SSID conectado e o IP local
            Serial.printf("Conectado à rede\n");
            Serial.printf("SSID: %s\n", ssid.c_str());
            Serial.printf("IP Local: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
            Serial.printf("Máscara de Sub-rede: %s\n", WiFi.subnetMask().toString().c_str());
            return true;
        }
        delay(500);
    }
    return false;
}

bool connectToSavedNetworks()
{
    if (!initializeLittleFS())
    {
        return false;
    }

    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file)
    {
        return false;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1)
        {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            if (connectToWiFi(savedSSID, savedPassword))
            {
                file.close();
                return true;
            }
        }
    }

    file.close();
    return false;
}

void loadSavedWiFiNetworks()
{
    if (!initializeLittleFS())
    {
        enterAPMode();
        return;
    }

    if (!LittleFS.exists("/wifiredes.txt"))
    {
        enterAPMode();
        return;
    }

    if (!connectToSavedNetworks())
    {
        enterAPMode();
    }
}
