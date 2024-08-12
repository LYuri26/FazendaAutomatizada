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
bool connectionAttempted = false;

void enterAPMode() {
    WiFi.mode(WIFI_AP_STA); // Configura o módulo para operar em modo AP+STA
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);

    Serial.println("Modo AP ativado");
    Serial.print("SSID do AP: ");
    Serial.println(ap_ssid);
    Serial.print("IP do AP: ");
    Serial.println(WiFi.softAPIP());
}

bool connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempts = 0;
    const int maxAttempts = 20;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    Serial.println(); // Pular linha após o loop de tentativa

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Conectado à rede: ");
        Serial.println(ssid);
        Serial.print("IP Local: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Falha na conexão Wi-Fi");
        return false;
    }
}

bool connectToSavedNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Arquivo de redes Wi-Fi não encontrado");
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
    return false;
}

void loadSavedWiFiNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Arquivo de redes Wi-Fi não encontrado. Entrando no modo AP.");
        enterAPMode();
        return;
    }

    String line;
    bool connected = false;
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim();
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1) {
            String savedSSID = line.substring(0, commaIndex);
            String savedPassword = line.substring(commaIndex + 1);
            savedSSID.toCharArray(ssid, sizeof(ssid));
            savedPassword.toCharArray(password, sizeof(password));

            if (!connectionAttempted) {
                connectionAttempted = true;
                if (connectToWiFi(savedSSID, savedPassword)) {
                    connected = true;
                    break;
                }
            }
        }
    }
    file.close();

    if (!connected) {
        Serial.println("Não foi possível conectar às redes salvas. Entrando no modo AP.");
        enterAPMode();
    }
}
