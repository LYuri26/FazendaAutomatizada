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
        Serial.println("Formatando LittleFS...");
        if (LittleFS.format())
            Serial.println("Formatação concluída.");
        else
            Serial.println("Falha ao formatar LittleFS.");
        LittleFS.end();
    }
    else
    {
        Serial.println("LittleFS não iniciado.");
    }
}

bool initializeLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao inicializar LittleFS. Tentando formatar...");
        formatLittleFS();
        if (!LittleFS.begin())
        {
            Serial.println("Falha ao inicializar LittleFS após formatação.");
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

    Serial.println("Modo AP ativado");
    Serial.printf("SSID do AP: %s\nIP do AP: %s\n", ap_ssid, WiFi.softAPIP().toString().c_str());
}

bool connectToWiFi(const String &ssid, const String &password)
{
    Serial.printf("Tentando conectar à rede: %s...\n", ssid.c_str());

    WiFi.begin(ssid.c_str(), password.c_str());
    for (int attempts = 0; attempts < 20; ++attempts)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.printf("Conectado à rede: %s\nIP Local: %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nFalha na conexão Wi-Fi");
    return false;
}

bool connectToSavedNetworks()
{
    Serial.println("Tentando conectar às redes Wi-Fi salvas...");

    if (!initializeLittleFS())
    {
        Serial.println("Não foi possível inicializar o LittleFS.");
        return false;
    }

    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file)
    {
        Serial.println("Arquivo de redes Wi-Fi não encontrado");
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
            Serial.printf("Tentando conectar à rede salva: %s\n", savedSSID.c_str());
            if (connectToWiFi(savedSSID, savedPassword))
            {
                Serial.println("Conexão bem-sucedida com uma das redes salvas.");
                file.close();
                return true;
            }
            else
                Serial.println("Falha ao conectar na rede salva.");
        }
    }

    Serial.println("Não foi possível conectar a nenhuma rede salva.");
    file.close();
    return false;
}

void loadSavedWiFiNetworks()
{
    Serial.println("Carregando redes Wi-Fi salvas...");

    if (!initializeLittleFS())
    {
        Serial.println("Não foi possível inicializar o LittleFS. Entrando no modo AP.");
        enterAPMode();
        return;
    }

    if (!LittleFS.exists("/wifiredes.txt"))
    {
        Serial.println("Arquivo de redes Wi-Fi não encontrado. Entrando no modo AP.");
        enterAPMode();
        return;
    }

    if (!connectToSavedNetworks())
    {
        Serial.println("Não foi possível conectar às redes salvas. Entrando no modo AP.");
        enterAPMode();
    }
}
