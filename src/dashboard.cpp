#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "dashboard.h"
#include "ligadesliga.h"
#include "autenticador.h"

// Simplificação dos estados das luzes
bool luzesLigadas[] = {false, false, false, false}; // 0: Casa, 1: Rua, 2: Pasto, 3: Geral

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
            background-color: #fff;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 5px rgba(0, 0, 0, 0.1);
            width: 90%;
            max-width: 600px;
            text-align: center;
        }
        .dashboard-title {
            font-size: 24px;
            margin-bottom: 15px;
            color: #007bff;
        }
        .btn {
            display: inline-block;
            padding: 10px;
            font-size: 14px;
            font-weight: bold;
            text-align: center;
            text-decoration: none;
            border-radius: 5px;
            margin: 5px;
            width: 100%;
            max-width: 180px;
            cursor: pointer;
            border: none;
        }
        .btn-luz-casa { background-color: #28a745; color: #fff; }
        .btn-luz-rua { background-color: #007bff; color: #fff; }
        .btn-luz-pasto { background-color: #ffc107; color: #fff; }
        .btn-luz-geral { background-color: #6c757d; color: #fff; }
        .btn-desligar { background-color: #dc3545; color: #fff; }
    </style>
</head>
<body>
    <div class="dashboard-container">
        <h2 class="dashboard-title">Bem-vindo ao Dashboard</h2>
        <button class="btn btn-luz-casa" id="toggleButton1">Luz da Casa</button>
        <button class="btn btn-luz-rua" id="toggleButton2">Luz da Rua</button>
        <button class="btn btn-luz-pasto" id="toggleButton3">Luz do Pasto</button>
        <button class="btn btn-luz-geral" id="toggleButton4">Luz Geral</button>
        <a href="/logout" class="btn btn-desligar">Logout</a>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            var buttons = document.querySelectorAll('.btn');
            buttons.forEach(function(button) {
                if (!button.classList.contains('btn-desligar')) {
                    button.addEventListener('click', function(event) {
                        event.preventDefault();
                        var action = button.classList.contains('btn-desligar') ? 'desligar' : 'ligar';
                        var buttonId = button.id;
                        fetch('/toggle?action=' + action + '&id=' + buttonId)
                            .then(response => response.text())
                            .then(data => {
                                console.log('Resposta do servidor:', data);
                                updateButtonState(button, action === 'ligar');
                            })
                            .catch(error => console.error('Erro ao enviar requisição:', error));
                    });
                }
            });

            function updateButtonState(button, isOn) {
                if (isOn) {
                    button.innerHTML = 'Desligar';
                    button.classList.remove('btn-luz-casa', 'btn-luz-rua', 'btn-luz-pasto', 'btn-luz-geral');
                    button.classList.add('btn-desligar');
                } else {
                    resetButton(button);
                }
            }

            function resetButton(button) {
                var buttonMap = {
                    'toggleButton1': 'btn-luz-casa',
                    'toggleButton2': 'btn-luz-rua',
                    'toggleButton3': 'btn-luz-pasto',
                    'toggleButton4': 'btn-luz-geral'
                };
                var defaultText = {
                    'toggleButton1': 'Luz da Casa',
                    'toggleButton2': 'Luz da Rua',
                    'toggleButton3': 'Luz do Pasto',
                    'toggleButton4': 'Luz Geral'
                };
                button.innerHTML = defaultText[button.id];
                button.classList.remove('btn-desligar');
                button.classList.add(buttonMap[button.id]);
            }
        });
    </script>
</body>
</html>
        )rawliteral";

        request->send(200, "text/html", html); });

    server.on("/luzes-estados", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String stateJson = "{\"luzCasaLigada\":" + String(luzesLigadas[0]) +
                           ", \"luzRuaLigada\":" + String(luzesLigadas[1]) +
                           ", \"luzPastoLigada\":" + String(luzesLigadas[2]) +
                           ", \"luzGeralLigada\":" + String(luzesLigadas[3]) + "}";
        request->send(200, "application/json", stateJson); });
}
