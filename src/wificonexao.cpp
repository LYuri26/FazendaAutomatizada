#include <ESP8266WiFi.h> // Biblioteca para gerenciar a conexão Wi-Fi no ESP8266
#include <LittleFS.h>    // Biblioteca para gerenciar o sistema de arquivos LittleFS
#include <vector>        // Biblioteca para usar vetores
#include "wificonexao.h" // Certifique-se de incluir o cabeçalho onde a função é declarada

// -------------------------------------------------------------------------
// Configurações Globais
// -------------------------------------------------------------------------
const char *ssid = "";
const char *password = "";
const char *ap_ssid = "CompressorWeb";
const char *ap_password = "12345678";

// Configurações do Access Point
IPAddress local_ip(192, 168, 26, 7);
IPAddress gateway(192, 168, 26, 1);
IPAddress subnet(255, 255, 255, 0);

bool isAPMode = false;

void connectToWiFi(const char *ssid, const char *password) {
    if (!ssid || !password || strlen(ssid) == 0 || strlen(password) == 0) {
        Serial.println("SSID ou senha inválidos.");
        enterAPMode();
        return;
    }

    WiFi.begin(ssid, password);

    for (int attempts = 0; attempts < 5 && WiFi.status() != WL_CONNECTED; attempts++) {
        delay(1000);
        Serial.print(".");
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
        return;
    }

    char line[64];
    while (file.available()) {
        file.readBytesUntil('\n', line, sizeof(line) - 1);
        line[file.position()] = '\0'; // Garante que a string esteja terminada em NULL

        char *commaPos = strchr(line, ',');
        if (commaPos) {
            *commaPos = '\0'; // Termina o SSID
            const char *savedSsid = line;
            const char *savedPassword = commaPos + 1;

            connectToWiFi(savedSsid, savedPassword);
            if (WiFi.status() == WL_CONNECTED) {
                file.close();
                return;
            }
        }
    }

    file.close();
    enterAPMode();
}