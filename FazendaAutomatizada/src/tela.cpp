#include <TFT_eSPI.h> // Inclua a biblioteca do TFT
#include "tela.h"
#include "tempo.h"
#include "wificonexao.h"
#include "ligadesliga.h"
#include "localizacao.h"

// Definições de pinos
#define PINO_LED 4            // Pino do LED da tela TFT
#define PINO_SCK 13           // Pino do Clock (SCK) para o TFT
#define PINO_SDA 11           // Pino de Dados (SDA) para o TFT
#define PINO_AO A0            // Pino Analógico de Entrada (AO)
#define PINO_RESET 9          // Pino de Reset para o TFT
#define PINO_CS 10            // Pino de Seleção de Chip (CS) para o TFT
#define PINO_GND GND          // Pino de Terra
#define PINO_VCC VCC          // Pino de Alimentação
#define TAMANHO_TELA 128      // Tamanho da tela em pixels (largura e altura)
#define SPI_MODE SPI          // Definição para o modo SPI
#define TAMANHO_TELA_TFT 1.44 // Tamanho da tela em polegadas

// Inicializa o display TFT
TFT_eSPI tft = TFT_eSPI(); // Cria um objeto TFT

// Função para inicializar a tela TFT
void inicializarTela()
{
    Serial.println("Inicializando a tela TFT...");

    // Configura o pino do LED da tela, se necessário
    pinMode(PINO_LED, OUTPUT);
    digitalWrite(PINO_LED, LOW); // Desliga o LED da tela inicialmente

    Serial.print("Pino do LED da tela configurado como OUTPUT.");
    Serial.print("Estado inicial do LED da tela: ");
    Serial.println(digitalRead(PINO_LED) == HIGH ? "Ligado" : "Desligado");

    // Inicializa o display TFT
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);

    Serial.println("Display TFT inicializado.");
    Serial.println("Configurações do display: ");
    Serial.print("Rotação: ");
    Serial.println(tft.getRotation());
    Serial.println("Tela preenchida com a cor preta.");

    // Liga o LED da tela
    digitalWrite(PINO_LED, HIGH);
    Serial.print("Estado do LED da tela após inicialização: ");
    Serial.println(digitalRead(PINO_LED) == HIGH ? "Ligado" : "Desligado");

    // Atualiza e exibe as informações na tela
    atualizarTela();
}

// Função para desenhar o sol
void desenharSol()
{
    Serial.println("Desenhando o sol ao amanhecer...");
    int x = 120;   // Posição X do sol
    int y = 30;    // Posição Y do sol
    int raio = 20; // Raio do sol

    // Desenhe o círculo do sol
    tft.fillCircle(x, y, raio, TFT_YELLOW);
    Serial.println("Sol desenhado na tela.");
}

// Função para desenhar a lua
void desenharLua()
{
    Serial.println("Desenhando a lua ao anoitecer...");
    int x = 120;   // Posição X da lua
    int y = 30;    // Posição Y da lua
    int raio = 20; // Raio da lua

    // Desenhe o círculo da lua
    tft.fillCircle(x, y, raio, TFT_WHITE);
    Serial.println("Lua desenhada na tela.");
}

// Função para atualizar a tela com as informações mais recentes
void atualizarTela()
{
    Serial.println("Atualizando a tela...");

    // Obtém a hora atual da internet e interna
    String horaAtual = getTimeClient();         // Hora atual do RTC/NTP
    String horaAtualInterna = getHoraInterna(); // Hora interna

    Serial.print("Hora Atual (NTP/RTC): ");
    Serial.println(horaAtual);
    Serial.print("Hora Interna: ");
    Serial.println(horaAtualInterna);

    Serial.print("Cidade: ");
    Serial.println(cidadeSalva);
    Serial.print("Nascer do Sol: ");
    Serial.println(nascerDoSol);
    Serial.print("Pôr do Sol: ");
    Serial.println(porDoSol);

    Serial.print("Estado das Luzes - Casa: ");
    Serial.println(luzEstado[0] ? "Ligada" : "Desligada");
    Serial.print("Estado das Luzes - Rua: ");
    Serial.println(luzEstado[1] ? "Ligada" : "Desligada");
    Serial.print("Estado das Luzes - Pasto: ");
    Serial.println(luzEstado[2] ? "Ligada" : "Desligada");
    Serial.print("Estado da Luz Geral: ");
    Serial.println(luzGeralEstado ? "Ligada" : "Desligada");

    Serial.print("Controle Manual Casa: ");
    Serial.println(controleManual[0] ? "Ativo" : "Inativo");
    Serial.print("Controle Manual Rua: ");
    Serial.println(controleManual[1] ? "Ativo" : "Inativo");
    Serial.print("Controle Manual Pasto: ");
    Serial.println(controleManual[2] ? "Ativo" : "Inativo");
    Serial.print("Controle Manual Geral: ");
    Serial.println(controleManual[3] ? "Ativo" : "Inativo");

    Serial.println("Atualizando display TFT...");

    // Atualiza o display TFT
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);

    // Exibir cidade e horários
    tft.println("Cidade: " + cidadeSalva);
    tft.println("Nascer do Sol: " + nascerDoSol);
    tft.println("Pôr do Sol: " + porDoSol);

    // Exibir hora
    tft.println("Hora Atual: " + horaAtual);
    tft.println("Hora Interna: " + horaAtualInterna);

    // Exibir estado das luzes
    exibirEstadoLuzes(luzEstado[0], luzEstado[1], luzEstado[2]);

    // Exibir controle manual das luzes
    tft.println("Controle Manual Casa: " + String(controleManual[0] ? "Ativo" : "Inativo"));
    tft.println("Controle Manual Rua: " + String(controleManual[1] ? "Ativo" : "Inativo"));
    tft.println("Controle Manual Pasto: " + String(controleManual[2] ? "Ativo" : "Inativo"));
    tft.println("Controle Manual Geral: " + String(controleManual[3] ? "Ativo" : "Inativo"));

    // Exibir informações da rede
    exibirInformacoesRede();

    // Verifica se a hora atual está dentro do intervalo de amanhecer e anoitecer
    if (isEntreAmanhecerEPorDoSol(horaAtual, nascerDoSol, porDoSol))
    {
        desenharSol();
    }
    else
    {
        desenharLua();
    }
}

// Função para exibir o estado das luzes na tela
void exibirEstadoLuzes(bool luzCasa, bool luzRua, bool luzPasto)
{
    Serial.println("Exibindo estado das luzes na tela...");
    tft.setCursor(0, 160);
    tft.println("Estado das Luzes:");
    tft.println("Luz Casa: " + String(luzCasa ? "Ligada" : "Desligada"));
    tft.println("Luz Rua: " + String(luzRua ? "Ligada" : "Desligada"));
    tft.println("Luz Pasto: " + String(luzPasto ? "Ligada" : "Desligada"));
}

// Função para exibir informações da rede conectada e do AP
void exibirInformacoesRede()
{
    Serial.println("Exibindo informações da rede na tela...");
    tft.setCursor(0, 200);

    if (isAPMode)
    {
        // Modo AP ativo
        tft.println("Modo AP Ativado");
        tft.println("AP SSID: " + String(ap_ssid));
        tft.println("AP IP: " + WiFi.softAPIP().toString());

        Serial.println("Modo AP Ativado");
        Serial.print("AP SSID: ");
        Serial.println(ap_ssid);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP().toString());
    }
    else
    {
        // Conectado a uma rede Wi-Fi
        tft.println("Modo AP Desativado");
        if (WiFi.status() == WL_CONNECTED)
        {
            tft.println("SSID Conectado: " + String(WiFi.SSID()));
            tft.println("IP Local: " + WiFi.localIP().toString());
            tft.println("Gateway: " + WiFi.gatewayIP().toString());
            tft.println("Máscara de Sub-rede: " + WiFi.subnetMask().toString());

            Serial.println("Modo AP Desativado");
            Serial.print("SSID Conectado: ");
            Serial.println(WiFi.SSID());
            Serial.print("IP Local: ");
            Serial.println(WiFi.localIP().toString());
            Serial.print("Gateway: ");
            Serial.println(WiFi.gatewayIP().toString());
            Serial.print("Máscara de Sub-rede: ");
            Serial.println(WiFi.subnetMask().toString());
        }
        else
        {
            tft.println("Sem conexão Wi-Fi");
            Serial.println("Sem conexão Wi-Fi");
        }
    }
}

// Função para verificar se a hora atual está entre o nascer do sol e o pôr do sol
bool isEntreAmanhecerEPorDoSol(String horaAtual, String nascerDoSol, String porDoSol)
{
    int minutosHoraAtual = converterParaMinutos(horaAtual);
    int minutosNascerDoSol = converterParaMinutos(nascerDoSol);
    int minutosPorDoSol = converterParaMinutos(porDoSol);

    // Verifica se a hora atual está entre o nascer do sol e o pôr do sol
    return (minutosHoraAtual >= minutosNascerDoSol && minutosHoraAtual <= minutosPorDoSol);
}

// Função para converter hora no formato "HH:MM" para minutos desde a meia-noite
int converterParaMinutos(String hora)
{
    int horas = hora.substring(0, 2).toInt();
    int minutos = hora.substring(3, 5).toInt();
    return horas * 60 + minutos;
}
