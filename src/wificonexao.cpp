#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wificonexao.h"
#include "wifiinterface.h"
#include "wifigerenciador.h"

// Definições de Access Point
const char *ap_ssid = "FazendaAutomatica";
const char *ap_password = "12345678";
IPAddress local_ip(192, 168, 26, 7);
IPAddress gateway(192, 168, 26, 1);
IPAddress subnet(255, 255, 255, 0);

char ssid[32] = "";
char password[32] = "";

bool isAPMode = false;
bool connectionAttempted = false;

void enterAPMode() {
    Serial.println("Modo Access Point ativo...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("SSID do AP: ");
    Serial.println(ap_ssid);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Ainda conectado a uma rede.");
    } else {
        Serial.println("Não está conectado a uma rede, apenas em modo AP.");
    }

    isAPMode = true;
}

bool connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempts = 0;
    const int maxAttempts = 20;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

bool connectToSavedNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Erro ao abrir o arquivo de redes Wi-Fi.");
        return false;
    }

    String content = file.readString();
    file.close();

    unsigned int start = 0;
    while (start < content.length()) {
        int end = content.indexOf('\n', start);
        if (end == -1) end = content.length();
        String line = content.substring(start, end);
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1) {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            if (connectToWiFi(savedSSID, savedPassword)) {
                return true;
            }
        }
        start = end + 1;
    }
    Serial.println("Nenhuma rede salva foi conectada.");
    return false;
}

void loadSavedWiFiNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Erro ao abrir o arquivo /wifiredes.txt.");
        enterAPMode();
        return;
    }

    String line;
    bool connected = false;
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim(); // Remove espaços em branco no início e no final
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1) {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            savedSSID.toCharArray(ssid, sizeof(ssid));          // SSID
            savedPassword.toCharArray(password, sizeof(password)); // Senha

            Serial.print("Lendo rede salva: SSID=");
            Serial.print(ssid);
            Serial.print(", Senha=");
            Serial.println(password);

            if (!connectionAttempted) {
                connectionAttempted = true;
                if (connectToWiFi(savedSSID, savedPassword)) {
                    connected = true;
                    break;
                }
            }
        } else {
            Serial.println("Formato inválido no arquivo de redes.");
        }
    }
    file.close();

    if (!connected) {
        Serial.println("Não foi possível conectar a nenhuma rede salva. Entrando no modo AP.");
        enterAPMode();
    }
}
