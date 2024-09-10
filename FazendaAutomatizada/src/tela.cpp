#include <TFT_eSPI.h> // Inclua a biblioteca do TFT
#include <WiFi.h>     // Inclua a biblioteca do Wi-Fi (se necessário)

#include "tela.h"
#include "tempo.h"
#include "wificonexao.h"
#include "ligadesliga.h"
#include "localizacao.h"

// Definições de pinos para o display TFT ILI9163
#define PINO_SCK 18  // Pino do Clock (SCK) para o TFT
#define PINO_SDA 23  // Pino de Dados (MOSI) para o TFT
#define PINO_RESET 5 // Pino de Reset para o TFT
#define PINO_CS 15   // Pino de Seleção de Chip (CS) para o TFT
#define PINO_DC 2    // Pino de Data/Command (DC) para o TFT
#define PINO_LED 13  // Pino do LED Backlight para o TFT

// Inicializa o display TFT
TFT_eSPI tft = TFT_eSPI(); // Cria um objeto TFT

// Funções de desenho para sol e lua
void desenharSol();
void desenharLua();

// Função para inicializar a tela TFT
void inicializarTela()
{
    Serial.begin(115200);
    Serial.println("Inicializando a tela TFT...");

    // Configura o pino do LED da tela
    pinMode(PINO_LED, OUTPUT);
    digitalWrite(PINO_LED, HIGH); // Liga o LED da tela para brilho máximo

    // Configura o display TFT com os pinos fornecidos
    tft.init();
    tft.setRotation(1);          // Define a rotação da tela
    tft.fillScreen(TFT_BLACK);   // Preenche a tela com a cor preta
    tft.setTextColor(TFT_WHITE); // Define a cor do texto como branco
    tft.setTextSize(1);          // Define o tamanho do texto

    Serial.println("Display TFT inicializado.");
    Serial.println("Configurações do display: ");
    Serial.print("Rotação: ");
    Serial.println(tft.getRotation());
    Serial.println("Tela preenchida com a cor preta.");

    // Atualiza e exibe as informações na tela
    atualizarTela();
}
// Função para atualizar a tela com as informações mais recentes
void atualizarTela()
{
    Serial.println("Atualizando a tela...");

    // Obtém a hora atual da internet e interna
    String horaAtual = getTimeClient();         // Hora atual do RTC/NTP
    String horaAtualInterna = getHoraInterna(); // Hora interna

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
    tft.setCursor(0, 160);
    tft.println("Estado das Luzes:");
    tft.println("Luz Casa: " + String(luzCasa ? "Ligada" : "Desligada"));
    tft.println("Luz Rua: " + String(luzRua ? "Ligada" : "Desligada"));
    tft.println("Luz Pasto: " + String(luzPasto ? "Ligada" : "Desligada"));
}

// Função para exibir informações da rede conectada
void exibirInformacoesRede()
{
    tft.setCursor(0, 200);
    tft.println("SSID Conectado: " + WiFi.SSID());
    tft.println("IP Local: " + WiFi.localIP().toString());
}

// Função para desenhar o sol
void desenharSol()
{
    int x = 120;
    int y = 30;
    int raio = 20;
    tft.fillCircle(x, y, raio, TFT_YELLOW);
}

// Função para desenhar a lua
void desenharLua()
{
    int x = 120;
    int y = 30;
    int raio = 20;
    tft.fillCircle(x, y, raio, TFT_WHITE);
}

// Função para verificar se a hora atual está entre o nascer do sol e o pôr do sol
bool isEntreAmanhecerEPorDoSol(String horaAtual, String nascerDoSol, String porDoSol)
{
    int minutosHoraAtual = converterParaMinutos(horaAtual);
    int minutosNascerDoSol = converterParaMinutos(nascerDoSol);
    int minutosPorDoSol = converterParaMinutos(porDoSol);
    return (minutosHoraAtual >= minutosNascerDoSol && minutosHoraAtual <= minutosPorDoSol);
}

// Função para converter hora no formato "HH:MM" para minutos desde a meia-noite
int converterParaMinutos(String hora)
{
    int horas = hora.substring(0, 2).toInt();
    int minutos = hora.substring(3, 5).toInt();
    return horas * 60 + minutos;
}
