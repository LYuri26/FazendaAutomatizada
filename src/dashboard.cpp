#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "dashboard.h"
#include "ligadesliga.h"
#include "autenticador.h"

bool luzesLigadas[4] = {false, false, false, false}; // Definição da variável global

void setupDashboardPage(AsyncWebServer &server) {
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
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
            text-align: center;
            width: 90%;
            max-width: 300px;
            margin: auto;
        }
        .btn {
            display: block;
            padding: 10px;
            margin: 10px auto;
            font-size: 14px;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            width: 100%;
            max-width: 180px;
            color: #fff;
        }
        .btn-luz-casa { background-color: #28a745; }
        .btn-luz-rua { background-color: #007bff; }
        .btn-luz-pasto { background-color: #ffc107; }
        .btn-luz-geral { background-color: #6c757d; }
        .btn-desligar { background-color: #dc3545; }
    </style>
</head>
<body>
    <div class="dashboard-container">
        <h2>Dashboard</h2>
        <button class="btn btn-luz-casa" id="toggleButton1">Luz da Casa</button>
        <button class="btn btn-luz-rua" id="toggleButton2">Luz da Rua</button>
        <button class="btn btn-luz-pasto" id="toggleButton3">Luz do Pasto</button>
        <button class="btn btn-luz-geral" id="toggleButton4">Luz Geral</button>
        <a href="/logout" class="btn btn-desligar">Logout</a>
    </div>
    <script>
document.addEventListener('DOMContentLoaded', function() {
    let isLuzGeralOn = false;

    // Função para atualizar o estado de um botão específico
    function toggleButton(button, isOn) {
        if (isOn) {
            button.textContent = 'Desligar';
            button.dataset.originalClass = button.className;
            button.className = 'btn btn-desligar';
        } else {
            resetButton(button);
        }
    }

    // Função para resetar o estado de um botão específico
    function resetButton(button) {
        const buttonNames = {
            'toggleButton1': 'Luz da Casa',
            'toggleButton2': 'Luz da Rua',
            'toggleButton3': 'Luz do Pasto',
            'toggleButton4': 'Luz Geral'
        };
        button.textContent = buttonNames[button.id];
        button.className = button.dataset.originalClass || button.className;
    }

    // Função para alternar o estado de todos os botões
    function toggleAllButtons(isOn) {
        document.querySelectorAll('.btn:not(.btn-desligar)').forEach(button => {
            toggleButton(button, isOn);
        });
    }

    // Atualiza os botões com base nos estados das luzes
    function updateButtonStates() {
        fetch('/luzes-estados')
            .then(response => response.json())
            .then(states => {
                toggleButton(document.getElementById('toggleButton1'), states.luzCasaLigada);
                toggleButton(document.getElementById('toggleButton2'), states.luzRuaLigada);
                toggleButton(document.getElementById('toggleButton3'), states.luzPastoLigada);
                toggleButton(document.getElementById('toggleButton4'), states.luzGeralLigada);
                isLuzGeralOn = states.luzGeralLigada;
            });
    }

    document.querySelectorAll('.btn').forEach(function(button) {
        button.addEventListener('click', function() {
            const action = button.classList.contains('btn-desligar') ? 'desligar' : 'ligar';
            const buttonId = button.id;

            fetch('/toggle?action=' + action + '&id=' + buttonId)
                .then(() => {
                    if (buttonId === 'toggleButton4') {
                        isLuzGeralOn = (action === 'ligar');
                        toggleAllButtons(isLuzGeralOn);
                        if (!isLuzGeralOn) {
                            updateButtonStates(); // Reseta todos os botões ao estado inicial se a luz geral for desligada
                        }
                    } else {
                        toggleButton(button, action === 'ligar');
                    }
                });
        });
    });

    updateButtonStates();
});
    </script>
</body>
</html>
        )rawliteral";

        request->send(200, "text/html", html);
    });

    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
        String action = request->getParam("action")->value();
        String id = request->getParam("id")->value();
        bool isOn = (action == "ligar");

        if (id == "toggleButton4") { // Luz Geral
            bool newState = !luzesLigadas[3]; // Inverte o estado da luz geral
            for (int i = 0; i < 4; i++) luzesLigadas[i] = newState;
        } else {
            int index = id.substring(id.length() - 1).toInt() - 1;
            luzesLigadas[index] = isOn;
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/luzes-estados", HTTP_GET, [](AsyncWebServerRequest *request) {
        String stateJson = "{\"luzCasaLigada\":" + String(luzesLigadas[0]) +
                           ", \"luzRuaLigada\":" + String(luzesLigadas[1]) +
                           ", \"luzPastoLigada\":" + String(luzesLigadas[2]) +
                           ", \"luzGeralLigada\":" + String(luzesLigadas[3]) + "}";
        request->send(200, "application/json", stateJson);
    });
}
