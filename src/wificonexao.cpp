#include <ESP8266WiFi.h>
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
bool connectionAttempted = false; // Flag para verificar se a conexão já foi tentada

void connectToWiFi()
{
    if (strlen(ssid) == 0 || strlen(password) == 0)
    {
        Serial.println("SSID ou senha inválidos.");
        enterAPMode();
        return;
    }

    Serial.print("Tentando conectar a rede Wi-Fi: ");
    Serial.print(ssid);
    Serial.print(" com a senha: ");
    Serial.println(password);

    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("Conectado com sucesso! IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("SSID da rede conectada: ");
        Serial.println(WiFi.SSID());
        isAPMode = false;
        connectionAttempted = true;
    }
    else
    {
        Serial.println();
        Serial.println("Falha na conexão.");
        Serial.print("Status do WiFi: ");
        Serial.println(WiFi.status());
        enterAPMode();
    }
}

void enterAPMode()
{
    Serial.println("Modo Access Point ativo...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("SSID do AP: ");
    Serial.println(ap_ssid);

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Ainda conectado a uma rede.");
    }
    else
    {
        Serial.println("Não está conectado a uma rede, apenas em modo AP.");
    }

    isAPMode = true;
}

void loadSavedWiFiNetworks()
{
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file)
    {
        Serial.println("Erro ao abrir o arquivo /wifiredes.txt.");
        enterAPMode();
        return;
    }

    String line;
    bool connected = false;
    while (file.available())
    {
        line = file.readStringUntil('\n');
        line.trim(); // Remove espaços em branco no início e no final
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1)
        {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            savedSSID.toCharArray(ssid, sizeof(ssid));          // SSID
            savedPassword.toCharArray(password, sizeof(password)); // Senha

            Serial.print("Lendo rede salva: SSID=");
            Serial.print(ssid);
            Serial.print(", Senha=");
            Serial.println(password);

            if (!connectionAttempted)
            {
                connectToWiFi();
                if (WiFi.status() == WL_CONNECTED)
                {
                    connected = true;
                    break;
                }
            }
        }
        else
        {
            Serial.println("Formato inválido no arquivo de redes.");
        }
    }
    file.close();

    if (!connected)
    {
        Serial.println("Não foi possível conectar a nenhuma rede salva. Entrando no modo AP.");
        enterAPMode();
    }
}
