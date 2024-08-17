#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "dashboard.h"
#include "ligadesliga.h"
#include "autenticador.h"
#include "localizacao.h"

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
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f4f7f9;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .dashboard-container {
            background-color: #fff;
            padding: 30px;
            border-radius: 12px;
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.1);
            text-align: center;
            width: 90%;
            max-width: 400px;
        }
        h2 {
            margin-bottom: 20px;
            font-size: 24px;
            color: #333;
        }
        .btn {
            display: block;
            padding: 14px;
            margin: 10px auto;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            width: 100%;
            max-width: 220px;
            color: #fff;
            transition: background-color 0.3s, transform 0.2s;
        }
        .btn-luz-casa { background-color: #28a745; }
        .btn-luz-rua { background-color: #007bff; }
        .btn-luz-pasto { background-color: #ffc107; }
        .btn-luz-geral { background-color: #6c757d; }
        .btn-desligar { background-color: #dc3545; }
        .btn:hover { opacity: 0.9; transform: scale(1.02); }

        .toggle-settings {
            display: none;
            margin-top: 20px;
            text-align: left;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: #f9f9f9;
            margin-left: auto;
            margin-right: auto;
        }
        .toggle-settings p {
            margin: 0 0 15px 0;
            font-size: 16px;
            color: #333;
        }
        .toggle-settings input[type="text"] {
            width: calc(100% - 20px);
            padding: 10px;
            margin-top: 10px;
            border-radius: 8px;
            border: 1px solid #ccc;
            font-size: 14px;
        }
        .toggle-settings button {
            display: block;
            margin: 20px auto 0;
            padding: 12px;
            border: none;
            border-radius: 8px;
            background-color: #007bff;
            color: #fff;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.3s;
            max-width: 220px;
        }
        .toggle-settings button:hover {
            background-color: #0056b3;
        }

        .switch-container {
            display: flex;
            align-items: center;
            margin-top: 20px;
            justify-content: center;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            border-radius: 50%;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
        }
        input:checked + .slider {
            background-color: #007bff;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        .slider-text {
            margin-left: 10px;
            font-size: 16px;
            color: #333;
        }

        .localizacao-info {
            margin-top: 20px;
            padding: 15px;
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: #f9f9f9;
            text-align: center;
        }
        .localizacao-info h3 {
            margin: 0;
            font-size: 20px;
            color: #333;
        }
        .localizacao-info p {
            margin: 5px 0;
            font-size: 16px;
            color: #555;
        }
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

        <div class="toggle-settings">
            <p>Deseja definir um horário automático baseado no pôr do sol e nascer do sol para ligar e desligar as luzes?</p>
            <input type="text" id="cidade-estado" placeholder="Exemplo: Uberaba-MG">
            <button id="definir-localizacao">Definir Localização</button>
        </div>

        <div class="switch-container">
            <label class="switch">
                <input type="checkbox" id="toggle-settings">
                <span class="slider"></span>
            </label>
            <span class="slider-text">Definir Localização Automática</span>
        </div>

        <div class="localizacao-info" id="localizacao-info" style="display: none;">
            <h3 id="cidade-nome"></h3>
            <p>Nascer do Sol: <span id="nascer-do-sol"></span></p>
            <p>Pôr do Sol: <span id="por-do-sol"></span></p>
        </div>
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

    document.getElementById('toggle-settings').addEventListener('change', function() {
        document.querySelector('.toggle-settings').style.display = this.checked ? 'block' : 'none';
    });

    document.getElementById('definir-localizacao').addEventListener('click', function() {
        console.log('Botão Definir Localização clicado');
        const cidadeEstado = document.getElementById('cidade-estado').value.trim();
        if (!cidadeEstado) {
            alert('Por favor, insira uma cidade e estado.');
            return;
        }
        // Substitui espaços por hífens e remove espaços adicionais
        const formattedLocal = cidadeEstado.replace(/\s+/g, '-') 
        const url = `/definir-horarios?local=${encodeURIComponent(formattedLocal)}`;
        console.log('URL gerada para definir localização: ' + url);
        fetch(url)
            .then(response => response.json())
            .then(data => {
                console.log('Resposta do servidor:', data);
                if (data.success) {
                    document.getElementById('cidade-nome').textContent = data.localizacao.cidade;
                    document.getElementById('nascer-do-sol').textContent = data.localizacao.nascerDoSol;
                    document.getElementById('por-do-sol').textContent = data.localizacao.porDoSol;
                    document.getElementById('localizacao-info').style.display = 'block';
                    alert('Horários definidos com sucesso!');
                } else {
                    alert('Erro ao definir horários. Verifique a cidade/estado e tente novamente.');
                }
            })
            .catch(err => console.error('Erro ao definir localização: ' + err));
    });

    updateButtonStates();
});
</script>
</body>
</html>
)rawliteral";

        request->send(200, "text/html", html); });
}
