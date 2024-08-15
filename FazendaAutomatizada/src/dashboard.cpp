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
        <button class="btn btn-luz-casa" id="luz0">Luz da Casa</button>
        <button class="btn btn-luz-rua" id="luz1">Luz da Rua</button>
        <button class="btn btn-luz-pasto" id="luz2">Luz do Pasto</button>
        <button class="btn btn-luz-geral" id="luz3">Luz Geral</button>
        <a href="/logout" class="btn btn-desligar">Logout</a>
    </div>
    <script>
document.addEventListener('DOMContentLoaded', function() {
    function updateButtonAppearance(button, isOn) {
        if (!button.dataset.originalClass) {
            button.dataset.originalClass = button.className;
        }
        button.textContent = isOn ? 'Desligar' : {
            'luz0': 'Luz da Casa',
            'luz1': 'Luz da Rua',
            'luz2': 'Luz do Pasto',
            'luz3': 'Luz Geral'
        }[button.id];
        button.className = isOn ? 'btn btn-desligar' : button.dataset.originalClass;
    }

    function updateButtonStates() {
        fetch('/luzes-estados')
            .then(response => response.json())
            .then(states => {
                updateButtonAppearance(document.getElementById('luz0'), states.luzCasaLigada);
                updateButtonAppearance(document.getElementById('luz1'), states.luzRuaLigada);
                updateButtonAppearance(document.getElementById('luz2'), states.luzPastoLigada);
                updateButtonAppearance(document.getElementById('luz3'), states.luzGeralLigada);
            })
            .catch(err => console.error('Erro ao atualizar estados das luzes: ' + err));
    }

    document.querySelectorAll('.btn').forEach(button => {
        button.addEventListener('click', function() {
            const buttonId = button.id.replace('luz', '');
            const action = button.textContent === 'Desligar' ? 'desligar' : 'ligar';

            fetch(`/toggle?action=${action}&id=${buttonId}`)
                .then(response => {
                    if (!response.ok) throw new Error('Erro ao alternar luz, código de status: ' + response.status);
                    return response.text();
                })
                .then(() => {
                    if (buttonId === '3') {
                        updateButtonStates();
                    } else {
                        updateButtonAppearance(button, action === 'ligar');
                    }
                })
                .catch(err => console.error('Erro ao alternar luz: ' + err));
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
        bool luzCasaLigada = readStateFromFile("/estadoLuzCasa.txt");
        bool luzRuaLigada = readStateFromFile("/estadoLuzRua.txt");
        bool luzPastoLigada = readStateFromFile("/estadoLuzPasto.txt");
        bool luzGeralLigada = readStateFromFile("/estadoLuzGeral.txt"); 

        String stateJson = "{\"luzCasaLigada\":" + String(luzCasaLigada) +
                           ", \"luzRuaLigada\":" + String(luzRuaLigada) +
                           ", \"luzPastoLigada\":" + String(luzPastoLigada) +
                           ", \"luzGeralLigada\":" + String(luzGeralLigada) + "}";
        request->send(200, "application/json", stateJson); });
}
