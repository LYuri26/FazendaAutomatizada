#include "wificonexao.h"
#include <LittleFS.h>

const char *ap_ssid = "CompressorWeb";
const char *ap_password = "12345678";

IPAddress local_ip(192, 168, 26, 7);
IPAddress gateway(192, 168, 26, 1);
IPAddress subnet(255, 255, 255, 0);

char ssid[32] = "";
char password[32] = "";

bool isAPMode = false;
unsigned long lastMessageTime = 0;

void connectToWiFi() {
    unsigned long currentMillis = millis();

    if (strlen(ssid) == 0 || strlen(password) == 0) {
        if (currentMillis - lastMessageTime >= 300000) { // 300000 ms = 5 minutes
            Serial.println("SSID ou senha inválidos.");
            lastMessageTime = currentMillis; // Atualiza o timestamp
        }
        enterAPMode();
        return;
    }

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Conexão feita! Endereço IP: ");
        Serial.println(WiFi.localIP());
        isAPMode = false;
    } else {
        Serial.println();
        Serial.println("Conexão falha.");
        enterAPMode();
    }
}

void enterAPMode() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Já conectado a uma rede Wi-Fi. Não é necessário entrar no modo AP.");
        return;
    }

    Serial.println("Entrando no modo Access Point...");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("Modo AP iniciado. Endereço IP: ");
    Serial.println(WiFi.softAPIP());
    isAPMode = true;
}

void loadSavedWiFiNetworks() {
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file) {
        Serial.println("Erro ao abrir o arquivo de redes Wi-Fi");
        enterAPMode();
        return;
    }

    while (file.available()) {
        char line[64];
        file.readStringUntil('\n').toCharArray(line, sizeof(line)); // Lê a linha e converte para char[]

        char *commaPos = strchr(line, ',');
        if (commaPos) {
            *commaPos = '\0';
            strncpy(ssid, line, sizeof(ssid) - 1);                // Copia o SSID para a variável global
            strncpy(password, commaPos + 1, sizeof(password) - 1); // Copia a senha para a variável global

            connectToWiFi();
            if (WiFi.status() == WL_CONNECTED) {
                file.close();
                return;
            }
        }
    }

    file.close();
    enterAPMode();
}
