#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "dashboard.h"
#include "ligadesliga.h"
#include "autenticador.h"

bool readStateFromFile(const char *path)
{
    if (!LittleFS.begin())
    {
        Serial.println("Falha ao montar o sistema de arquivos");
        return false;
    }

    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");
        if (file)
        {
            String content = file.readString();
            file.close();
            return content.toInt() == 1;
        }
    }
    return false;
}

void setupDashboardPage(AsyncWebServer &server)
{
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (!isAuthenticated(request)) {
            redirectToAccessDenied(request);
            return;
        }

        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #f8f9fa;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }

        .dashboard-container {
            background-color: #ffffff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            text-align: center;
            width: 90%;
            max-width: 320px;
        }

        h2 {
            margin-bottom: 20px;
            font-size: 24px;
            color: #333333;
        }

        .btn {
            display: block;
            padding: 12px;
            margin: 10px auto;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            width: 100%;
            max-width: 220px;
            color: #ffffff;
            transition: background-color 0.3s, transform 0.2s;
        }

        .btn:hover {
            transform: scale(1.05);
        }

        .btn:active {
            transform: scale(0.98);
        }

        .btn-luz-sala { background-color: #28a745; }
        .btn-luz-externa { background-color: #007bff; }
        .btn-luz-galpao { background-color: #ffc107; }
        .btn-luz-entrada { background-color: #ff5733; }
        .btn-definir-localizacao { background-color: #6f42c1; }
        .btn-logout { background-color: #dc3545; }

        .btn-luz-sala:hover { background-color: #218838; }
        .btn-luz-externa:hover { background-color: #0069d9; }
        .btn-luz-galpao:hover { background-color: #e0a800; }
        .btn-luz-entrada:hover { background-color: #e74c3c; }
        .btn-definir-localizacao:hover { background-color: #5e35b1; }
        .btn-logout:hover { background-color: #c82333; }
    </style>
</head>
<body>
    <div class="dashboard-container">
        <h2>Dashboard</h2>
        <button class="btn btn-luz-sala" id="luzSala">Luz da Sala</button>
        <button class="btn btn-luz-externa" id="luzExterna">Luz Externa</button>
        <button class="btn btn-luz-galpao" id="luzGalpao">Luz do Galpão</button>
        <button class="btn btn-luz-entrada" id="luzEntrada">Luz da Entrada</button>
        <a href="/localizacao" class="btn btn-definir-localizacao">Definir Localização</a>
        <a href="/logout" class="btn btn-logout">Logout</a>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            function updateButtonAppearance(button, isOn) {
                if (!button.dataset.originalClass) {
                    button.dataset.originalClass = button.className;
                }
                button.textContent = isOn ? 'Desligar' : button.dataset.originalText;
                button.className = isOn ? 'btn btn-logout' : button.dataset.originalClass;
            }

            function updateButtonStates() {
                fetch('/luzes-estados')
                    .then(response => response.json())
                    .then(states => {
                        updateButtonAppearance(document.getElementById('luzSala'), states.luzSalaLigada);
                        updateButtonAppearance(document.getElementById('luzExterna'), states.luzExternaLigada);
                        updateButtonAppearance(document.getElementById('luzGalpao'), states.luzGalpaoLigada);
                        updateButtonAppearance(document.getElementById('luzEntrada'), states.luzEntradaLigada);
                    });
            }

            document.querySelectorAll('.btn').forEach(button => {
                button.dataset.originalText = button.textContent;

                button.addEventListener('click', function() {
                    const buttonId = button.id.replace('luz', '');
                    const action = button.textContent === 'Desligar' ? 'desligar' : 'ligar';

                    fetch(`/toggle?action=${action}&id=${buttonId}`)
                        .then(response => {
                            if (!response.ok) throw new Error('Erro ao alternar luz, código de status: ' + response.status);
                            return response.text();
                        })
                        .then(() => {
                            updateButtonAppearance(button, action === 'ligar');
                        });
                });
            });

            updateButtonStates(); 
        });
    </script>
</body>
</html>
        )rawliteral";

        request->send(200, "text/html", html); });

    server.on("/luzes-estados", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        bool luzSalaLigada = readStateFromFile("/estadoLuzSala.txt");
        bool luzExternaLigada = readStateFromFile("/estadoLuzExterna.txt");
        bool luzGalpaoLigada = readStateFromFile("/estadoLuzGalpao.txt");
        bool luzEntradaLigada = readStateFromFile("/estadoLuzEntrada.txt"); 

        String stateJson = "{\"luzSalaLigada\":" + String(luzSalaLigada) +
                           ", \"luzExternaLigada\":" + String(luzExternaLigada) +
                           ", \"luzGalpaoLigada\":" + String(luzGalpaoLigada) +
                           ", \"luzEntradaLigada\":" + String(luzEntradaLigada) + "}";
        request->send(200, "application/json", stateJson); });
}
