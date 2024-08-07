// -------------------------------------------------------------------------
// Inclusões de Bibliotecas e Cabeçalhos
// -------------------------------------------------------------------------
#include <FS.h>                // Biblioteca para manipulação do sistema de arquivos
#include <SPIFFS.h>            // Biblioteca para o sistema de arquivos SPIFFS
#include <ESPAsyncWebServer.h> // Biblioteca para servidor web assíncrono
#include "autenticador.h"      // Cabeçalho onde a variável userLoggedIn é declarada
#include "ligadesliga.h"       // Cabeçalho para manipulação do compressor

// -------------------------------------------------------------------------
// Configurações e Variáveis Globais
// -------------------------------------------------------------------------
const int pinoLigaDesliga = 2;                        // Pino do compressor
const String arquivoEstado = "/estadocompressor.txt"; // Arquivo para salvar o estado do compressor

bool compressorLigado = false; // Estado atual do compressor (ligado ou desligado)

// -------------------------------------------------------------------------
// Funções de Manipulação do Compressor
// -------------------------------------------------------------------------

/**
 * Função para manipular a ação de ligar/desligar o compressor.
 *
 * @param server Instância do servidor web assíncrono.
 */
void handleToggleAction(AsyncWebServer &server)
{
    server.on("/toggle", HTTP_ANY, [](AsyncWebServerRequest *request)
              {
        // Verifica se o usuário está autenticado
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request); // Redireciona se não autenticado
            return;
        }

        // Lógica para alternar o estado do compressor com base no parâmetro "action"
        String action = request->getParam("action")->value();
        if (action == "ligar") {
            compressorLigado = true; // Liga o compressor
            request->send(200, "text/plain", "Compressor ligado!"); // Resposta ao cliente
        } else if (action == "desligar") {
            compressorLigado = false; // Desliga o compressor
            request->send(200, "text/plain", "Compressor desligado!"); // Resposta ao cliente
        } else {
            request->send(400, "text/plain", "Ação inválida!"); // Ação inválida
        } });
}

/**
 * Função para inicializar o sistema de arquivos SPIFFS.
 */
void initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Erro ao iniciar SPIFFS. O sistema de arquivos não pôde ser montado.");
        return;
    }
    Serial.println("SPIFFS inicializado com sucesso.");
}

/**
 * Função para ler o estado do compressor a partir do arquivo SPIFFS.
 *
 * @return Estado do compressor (ligado ou desligado).
 */
bool readCompressorState()
{
    Serial.println("Tentando ler o estado do compressor do arquivo.");
    File file = SPIFFS.open(arquivoEstado, "r");
    if (!file)
    {
        Serial.println("Arquivo de estado não encontrado, assumindo estado desligado.");
        return false; // Assumindo que o compressor está desligado se o arquivo não for encontrado
    }

    String state = file.readStringUntil('\n'); // Lê o estado do arquivo
    file.close();
    bool estado = state.toInt() == 1; // Converte o valor lido para booleano
    Serial.printf("Estado lido do arquivo: %s\n", estado ? "Ligado" : "Desligado");
    return estado;
}

/**
 * Função para salvar o estado do compressor no arquivo SPIFFS.
 *
 * @param state Estado atual do compressor (ligado ou desligado).
 */
void saveCompressorState(bool state)
{
    Serial.printf("Salvando o estado do compressor: %s\n", state ? "Ligado" : "Desligado");
    File file = SPIFFS.open(arquivoEstado, "w");
    if (!file)
    {
        Serial.println("Erro ao abrir o arquivo para escrita.");
        return;
    }

    file.println(state ? "1" : "0"); // Salva o estado como "1" para ligado e "0" para desligado
    file.close();
    Serial.println("Estado do compressor salvo com sucesso.");
}

// -------------------------------------------------------------------------
// Funções de Configuração do Servidor Web
// -------------------------------------------------------------------------

/**
 * Função para configurar o servidor Web para controle do compressor.
 *
 * @param server Instância do servidor web assíncrono.
 */
void setupLigaDesliga(AsyncWebServer &server)
{
    Serial.println("Configurando o servidor Web para controle do compressor.");
    initSPIFFS(); // Inicializa o sistema de arquivos SPIFFS

    pinMode(pinoLigaDesliga, OUTPUT);                             // Configura o pino do compressor como saída
    compressorLigado = readCompressorState();                     // Lê o estado do compressor do arquivo
    digitalWrite(pinoLigaDesliga, compressorLigado ? HIGH : LOW); // Define o estado inicial do compressor

    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  unsigned long currentMillis = millis(); // Obtém o tempo atual
                  Serial.println("Requisição recebida para alternar o estado do compressor.");

                  compressorLigado = !compressorLigado;                         // Alterna o estado do compressor
                  digitalWrite(pinoLigaDesliga, compressorLigado ? HIGH : LOW); // Atualiza o pino do compressor

                  // Mensagem de resposta com base no estado do compressor
                  String message = compressorLigado ? "Compressor ligado!" : "Compressor desligado!";

                  Serial.print("Estado do compressor: ");
                  Serial.println(message);
                  request->send(200, "text/plain", message); // Envia a resposta ao cliente

                  saveCompressorState(compressorLigado); // Salva o estado do compressor no arquivo
              });

    Serial.print("Estado inicial do compressor: ");
    Serial.println(compressorLigado ? "Ligado" : "Desligado");
}

// -------------------------------------------------------------------------
// Funções de Atualização e Desligamento
// -------------------------------------------------------------------------

/**
 * Função para desligar o dispositivo e salvar o estado do compressor.
 */
void shutdown()
{
    saveCompressorState(compressorLigado); // Salva o estado do compressor
    Serial.println("Dispositivo desligado. Estado do compressor salvo.");
    delay(1000);   // Aguarda um segundo para garantir que a mensagem seja exibida
    ESP.restart(); // Reinicia o ESP32
}
