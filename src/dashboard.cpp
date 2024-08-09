#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "dashboard.h"
#include "ligadesliga.h"
#include "autenticador.h"

void setupDashboardPage(AsyncWebServer &server)
{
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (!isAuthenticated(request))
        {
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
            margin: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            flex-direction: column;
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
            margin-bottom: 10px;
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
        .footer {
            width: 100%;
            background-color: #007bff;
            color: #fff;
            text-align: center;
            padding: 10px 0;
            position: fixed;
            bottom: 0;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="dashboard-container">
        <h2 class="dashboard-title">Bem-vindo ao Dashboard</h2>
        <button class="btn btn-luz-casa" id="toggleButton1">Luz da Casa</button>
        <button class="btn btn-luz-rua" id="toggleButton2">Luz da Rua</button>
        <button class="btn btn-luz-pasto" id="toggleButton3">Luz do Pasto</button>
        <button class="btn btn-luz-geral" id="toggleButton4">Luz Geral</button>
        <a href="/logout" class="btn btn-danger">Logout</a>
    </div>
    <div id="messageBox"></div>
    <div class="footer">
        <p>Aplicação desenvolvida pela Turma de Informática Para Internet Trilhas de Futuro 2024</p>
        <p>Instrutor: Lenon Yuri</p>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            var buttons = document.querySelectorAll('.btn');
            buttons.forEach(function(button) {
                if (button.classList.contains('btn-danger')) {
                    // Skip the logout button
                    return;
                }
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
            });

            function updateButtonState(button, isOn) {
                if (isOn) {
                    button.innerHTML = 'Desligar';
                    button.classList.remove('btn-luz-casa', 'btn-luz-rua', 'btn-luz-pasto', 'btn-luz-geral');
                    button.classList.add('btn-desligar');
                } else {
                    button.innerHTML = 'Ligar';
                    button.classList.remove('btn-desligar');
                    if (button.id === 'toggleButton1') {
                        button.classList.add('btn-luz-casa');
                    } else if (button.id === 'toggleButton2') {
                        button.classList.add('btn-luz-rua');
                    } else if (button.id === 'toggleButton3') {
                        button.classList.add('btn-luz-pasto');
                    } else if (button.id === 'toggleButton4') {
                        button.classList.add('btn-luz-geral');
                    }
                }
            }
        });
    </script>
</body>
</html>
        )rawliteral";

        request->send(200, "text/html", html); });

    server.on("/compressor-state", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String stateJson = "{\"compressorLigado\":" + String(compressorLigado) + "}";
        request->send(200, "application/json", stateJson); });
}
