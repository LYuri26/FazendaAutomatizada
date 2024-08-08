#include <ESP8266WiFi.h> // Biblioteca para gerenciar a conexão Wi-Fi no ESP8266
#include <LittleFS.h>    // Biblioteca para gerenciar o sistema de arquivos LittleFS
#include <vector>        // Biblioteca para usar vetores
#include "wificonexao.h" // Certifique-se de incluir o cabeçalho onde a função é declarada

// -------------------------------------------------------------------------
// Configurações Globais
// -------------------------------------------------------------------------
const char *ssid = "";                 // SSID da rede Wi-Fi para conexão
const char *password = "";             // Senha da rede Wi-Fi para conexão
const char *ap_ssid = "CompressorWeb"; // SSID do Access Point
const char *ap_password = "12345678";  // Senha do Access Point

// Configurações do Access Point
IPAddress local_ip(192, 168, 26, 7); // IP local do Access Point
IPAddress gateway(192, 168, 26, 1);  // Gateway do Access Point (corrigido para estar na mesma sub-rede)
IPAddress subnet(255, 255, 255, 0); // Máscara de sub-rede do Access Point

// Variáveis para o gerenciamento de escaneamento de redes Wi-Fi
bool scanning = false;                   // Indica se o escaneamento está em andamento
unsigned long lastScan = 0;              // Tempo do último escaneamento
const unsigned long scanInterval = 5000; // Intervalo entre escaneamentos (em milissegundos)

bool isAPMode = false; // Indica se o dispositivo está no modo Access Point
bool scanDone = false; // Indica se o escaneamento foi concluído
int scanResults = 0;   // Número de redes encontradas

// -------------------------------------------------------------------------
// Função para Conectar ao Wi-Fi
// -------------------------------------------------------------------------
void connectToWiFi(const char *ssid, const char *password)
{
    if (!ssid || !password || strlen(ssid) == 0 || strlen(password) == 0)
    {
        Serial.println("SSID ou senha inválidos.");
        enterAPMode();
        return;
    }

    Serial.print("Tentando conectar ao WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password); // Inicia a conexão com o Wi-Fi

    int attempts = 0;
    const int maxAttempts = 5;

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
    {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("Conexão feita! Endereço IP: ");
        Serial.println(WiFi.localIP());
        isAPMode = false;
    }
    else
    {
        Serial.println();
        Serial.println("Conexão falha.");
        enterAPMode();
    }
}

// -------------------------------------------------------------------------
// Função para Entrar no Modo Access Point
// -------------------------------------------------------------------------
void enterAPMode()
{
    Serial.println("Entrando no modo Access Point...");
    WiFi.mode(WIFI_AP);                           // Configura o ESP8266 para o modo Access Point
    WiFi.softAPConfig(local_ip, gateway, subnet); // Configura o IP, gateway e máscara
    WiFi.softAP(ap_ssid, ap_password);            // Inicia o Access Point com SSID e senha
    Serial.print("Modo AP iniciado. Endereço IP: ");
    Serial.println(WiFi.softAPIP()); // Imprime o IP do Access Point
    isAPMode = true;
}

// -------------------------------------------------------------------------
// Função para Iniciar o Escaneamento de Redes Wi-Fi
// -------------------------------------------------------------------------
void startScanWiFiNetworks()
{
    if (!scanning && millis() - lastScan > scanInterval)
    {
        scanning = true;
        WiFi.scanNetworks(true); // Inicia o escaneamento de redes Wi-Fi
        lastScan = millis();
    }
}

// -------------------------------------------------------------------------
// Função para Obter os Resultados do Escaneamento
// -------------------------------------------------------------------------
String getScanResults()
{
    int n = WiFi.scanComplete(); // Obtém o número de redes encontradas
    if (n == -2 || n == -1)
    {
        return "[]";
    }
    else if (n == 0)
    {
        return "[]";
    }
    else
    {
        String json = "[";
        for (int i = 0; i < n; i++)
        {
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\"}";
            if (i < n - 1)
                json += ","; // Adiciona vírgula entre os itens, exceto no último
        }
        json += "]";
        scanning = false;
        return json;
    }
}

// -------------------------------------------------------------------------
// Função para Verificar se o Escaneamento foi Concluído
// -------------------------------------------------------------------------
void checkScanResults()
{
    if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
    {
        scanDone = true;
        scanResults = WiFi.scanComplete(); // Armazena o número de redes encontradas
        scanning = false;
    }
}

// -------------------------------------------------------------------------
// Função para Carregar Redes Wi-Fi Salvas e Tentar Conexão
// -------------------------------------------------------------------------
void loadSavedWiFiNetworks()
{
    File file = LittleFS.open("/wifiredes.txt", "r");
    if (!file)
    {
        Serial.println("Erro ao abrir o arquivo de redes Wi-Fi");
        return;
    }

    String content = file.readString();
    file.close();

    String ssid;
    String password;
    std::vector<String> networks;

    size_t start = 0;  // Use size_t para compatibilidade com length()
    size_t length = content.length();
    while (start < length)
    {
        size_t end = content.indexOf('\n', start);
        if (end == -1)
        {
            end = length;
        }
        String line = content.substring(start, end);
        networks.push_back(line);
        start = end + 1;
    }

    for (int i = networks.size() - 1; i >= 0; i--)
    {
        String line = networks[i];
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1)
        {
            ssid = line.substring(0, commaIndex);
            password = line.substring(commaIndex + 1);
            connectToWiFi(ssid.c_str(), password.c_str());
            if (WiFi.status() == WL_CONNECTED)
            {
                return;
            }
        }
    }
}
