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
    height: auto;
    margin: 0;
    transition: background-color 0.3s, color 0.3s;
}

.dashboard-container {
    background-color: #fff;
    padding: 30px;
    border-radius: 12px;
    box-shadow: 0 8px 16px rgba(0, 0, 0, 0.1);
    text-align: center;
    width: 100%;
    max-width: 600px;
    margin: 40px 0;
    box-sizing: border-box;
    transition: background-color 0.3s, border 0.3s;
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
    background-color: #c8971c;
    transition: background-color 0.3s, transform 0.2s;
}

.btn-luz-casa { background-color: #28a745; }
.btn-luz-rua { background-color: #007bff; }
.btn-luz-pasto { background-color: #ffc107; }
.btn-luz-geral { background-color: #6c757d; }
.btn-desligar, .btn-logout { background-color: #dc3545; }

.btn:hover { 
    opacity: 0.9; 
    transform: scale(1.02); 
}

.toggle-settings {
    display: none;
    margin-top: 20px;
    text-align: left;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 8px;
    background-color: #f9f9f9;
}

.toggle-settings p {
    margin: 0 0 15px;
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
    background-color: #2ab7b7;
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

.message {
    margin-top: 20px;
    padding: 10px;
    border-radius: 8px;
    text-align: center;
    font-size: 16px;
    display: none;
}

.message.success {
    background-color: #d4edda;
    color: #155724;
    border: 1px solid #c3e6cb;
}

.message.error {
    background-color: #f8d7da;
    color: #721c24;
    border: 1px solid #f5c6cb;
}

/* Modo Noturno */
.dark-mode {
    background-color: #121212;
    color: #e0e0e0;
}
.dark-mode .localizacao-info h3 {
        color: #ffffff;
}
.dark-mode .dashboard-container {
    background-color: #1e1e1e;
    border: 1px solid #333;
}

.dark-mode .btn {
    color: #fff; /* Manter as cores dos botões iguais */
}

.dark-mode .btn-luz-casa { background-color: #28a745; }
.dark-mode .btn-luz-rua { background-color: #007bff; }
.dark-mode .btn-luz-pasto { background-color: #ffc107; }
.dark-mode .btn-luz-geral { background-color: #6c757d; }
.dark-mode .btn-desligar, .dark-mode .btn-logout { background-color: #dc3545; }

.dark-mode .btn:hover {
    opacity: 0.9;
    transform: scale(1.02);
}

.dark-mode .toggle-settings {
    background-color: #333;
    border-color: #444;
}

.dark-mode .localizacao-info {
    background-color: #333;
    border-color: #444;
}

.dark-mode h2, .dark-mode p, .dark-mode label {
    color: #ffffff;
}

/* Modo Alto Contraste */
.high-contrast-mode {
    background-color: #000;
    color: #fff;
}

.high-contrast-mode .dashboard-container {
    background-color: #111;
    border: 1px solid #fff;
}

.high-contrast-mode .btn {
    color: #fff; /* Manter as cores dos botões iguais */
}

.high-contrast-mode .btn-luz-casa { background-color: #28a745; }
.high-contrast-mode .btn-luz-rua { background-color: #007bff; }
.high-contrast-mode .btn-luz-pasto { background-color: #ffc107; }
.high-contrast-mode .btn-luz-geral { background-color: #6c757d; }
.high-contrast-mode .btn-desligar, .high-contrast-mode .btn-logout { background-color: #dc3545; }

.high-contrast-mode .btn:hover {
    opacity: 0.9;
    transform: scale(1.02);
}

.high-contrast-mode .toggle-settings {
    background-color: #000;
    border-color: #fff;
}

.high-contrast-mode .localizacao-info {
    background-color: #000;
    border-color: #fff;
}

.high-contrast-mode h2, .high-contrast-mode p, .high-contrast-mode label {
    color: #ffffff;
}

/* Botões de Modo Noturno e Alto Contraste no Final */
.mode-buttons {
    margin-top: 30px;
    display: flex;
    justify-content: center;
    gap: 10px;
}

.btn-mode-dark, .btn-mode-contrast {
    background-color: #007bff;
    color: #fff;
    border: none;
    border-radius: 8px;
    cursor: pointer;
    padding: 14px;
    font-size: 18px;
    font-weight: bold;
    transition: background-color 0.3s, transform 0.2s;
}

.btn-mode-dark {
    background-color: #ff5722;
}

.btn-mode-contrast {
    background-color: #4caf50;
}

.btn-mode-dark:hover {
    background-color: #e64a19;
}

.btn-mode-contrast:hover {
    background-color: #388e3c;
}
      </style>
    </head>
    <body>
        <div class="dashboard-container">
            <h2 id="mainTitle">Painel de Controle</h2>

            <!-- Botões de luzes -->
            <button id="luz0" class="btn btn-luz-casa">Luz do Primeiro
                Andar</button>
            <button id="luz1" class="btn btn-luz-rua">Luz do Segundo
                Andar</button>
            <button id="luz2" class="btn btn-luz-pasto">Luz Externa</button>
            <button id="luz3" class="btn btn-luz-geral">Luz Geral</button>

            <!-- Botão de logout -->
            <button id="logout" class="btn btn-logout"
                onclick="location.href='/logout'">Logout</button>

            <!-- Mensagem de status -->
            <div id="message" class="message" aria-live="assertive"></div>

            <!-- Botões de modo noturno e alto contraste -->
            <div class="mode-buttons">
                <button id="modeSwitch" class="btn btn-mode-dark">Modo
                    Noturno</button>
                <button id="contrastSwitch" class="btn btn-mode-contrast">Alto
                    Contraste</button>
            </div>

            <!-- Alternância de configuração de localização automática -->
            <div class="switch-container">
                <button id="toggle-settings-btn" class="btn">Definir Localização
                    Automática</button>
            </div>

            <div class="toggle-settings" id="toggle-settings-container">
                <p>Deseja definir um horário automático baseado no pôr do sol e
                    nascer do sol para ligar e desligar as luzes?</p>
                <input type="text" id="cidade-estado"
                    placeholder="Exemplo: Uberaba-MG, para excluir digite 'Excluir'">
                <button id="definir-localizacao" class="btn">Definir
                    Localização</button>
            </div>

            <!-- Informações de localização -->
            <div class="localizacao-info" id="localizacao-info">
                <h3 id="cidade-nome">Cidade</h3>
                <p>Nascer do Sol: <span id="nascer-do-sol">Horário</span></p>
                <p>Pôr do Sol: <span id="por-do-sol">Horário</span></p>
            </div>
        </div>

        <script>
document.addEventListener('DOMContentLoaded', function () {
    // Função para exibir mensagens
    function displayMessage(message, type) {
        const messageBox = document.getElementById('message');
        messageBox.textContent = message;
        messageBox.className = 'message ' + (type === 'success' ? 'success' : 'error');
        messageBox.style.display = 'block';
    }

    // Função para atualizar aparência dos botões
    function updateButtonAppearance(button, isOn) {
        if (button.id === 'logout') return;
        if (!button.dataset.originalClass) button.dataset.originalClass = button.className;
        button.textContent = isOn ? 'Desligar' : {
            'luz0': 'Luz do Primeiro Andar',
            'luz1': 'Luz do Segundo Andar',
            'luz2': 'Luz Externa',
            'luz3': 'Luz Geral'
        }[button.id];
        button.className = isOn ? 'btn btn-desligar' : button.dataset.originalClass;
    }

    // Atualizar estados dos botões de luzes
    function updateButtonStates() {
        fetch('/luzes-estados')
            .then(response => response.json())
            .then(states => {
                updateButtonAppearance(document.getElementById('luz0'), states.luzCasaLigada);
                updateButtonAppearance(document.getElementById('luz1'), states.luzRuaLigada);
                updateButtonAppearance(document.getElementById('luz2'), states.luzPastoLigada);
                updateButtonAppearance(document.getElementById('luz3'), states.luzGeralLigada);
            });
    }

    // Função para verificar se todas as luzes estão ligadas
    function areAllLightsOn() {
        const luz0 = document.getElementById('luz0').textContent === 'Desligar';
        const luz1 = document.getElementById('luz1').textContent === 'Desligar';
        const luz2 = document.getElementById('luz2').textContent === 'Desligar';
        return luz0 && luz1 && luz2; // Retorna true se todas as luzes estiverem ligadas
    }

    // Função para ligar ou desligar todas as luzes
    function toggleAllLights(action) {
        // Itera sobre todos os botões de luz, exceto "Luz Geral"
        document.querySelectorAll('.btn').forEach(button => {
            if (button.id.startsWith('luz') && button.id !== 'luz3') {
                fetch(`/toggle?action=${action}&id=${button.id.replace('luz', '')}`)
                    .then(() => {
                        updateButtonAppearance(button, action === 'ligar');
                    });
            }
        });
    }

    // Adicionando funcionalidade aos botões de luz
    document.querySelectorAll('.btn').forEach(button => {
        if (button.id.startsWith('luz')) {
            button.addEventListener('click', function () {
                const buttonId = button.id.replace('luz', '');
                const action = button.textContent === 'Desligar' ? 'desligar' : 'ligar';

                if (buttonId === '3') {
                    // Se for o botão de "Luz Geral", verifica o estado atual das luzes
                    if (areAllLightsOn()) {
                        // Desliga todas as luzes se todas estiverem ligadas
                        toggleAllLights('desligar');
                        updateButtonAppearance(button, false); // Atualiza a Luz Geral para desligada
                    } else {
                        // Liga todas as luzes se alguma estiver desligada
                        toggleAllLights('ligar');
                        updateButtonAppearance(button, true); // Atualiza a Luz Geral para ligada
                    }
                } else {
                    // Alterna apenas a luz individual
                    fetch(`/toggle?action=${action}&id=${buttonId}`)
                        .then(response => response.text())
                        .then(() => {
                            updateButtonAppearance(button, action === 'ligar');
                        });
                }
            });
        }
    });

    // Alternar o modo noturno
    document.getElementById('modeSwitch').addEventListener('click', function () {
        document.body.classList.toggle('dark-mode');
    });

    // Alternar o modo de alto contraste
    document.getElementById('contrastSwitch').addEventListener('click', function () {
        document.body.classList.toggle('high-contrast-mode');
    });

    // Definir localização automática
    document.getElementById('definir-localizacao').addEventListener('click', function () {
        const cidadeEstado = document.getElementById('cidade-estado').value.trim();
        if (!cidadeEstado) {
            displayMessage('Por favor, insira uma cidade e estado.', 'error');
            return;
        }

        const url = `/definir-horarios?local=${encodeURIComponent(cidadeEstado)}`;
        fetch(url)
            .then(response => response.json())
            .then(data => {
                if (data.success && data.localizacao) {
                    document.getElementById('cidade-nome').textContent = data.localizacao.cidade;
                    document.getElementById('nascer-do-sol').textContent = data.localizacao.nascerDoSol;
                    document.getElementById('por-do-sol').textContent = data.localizacao.porDoSol;
                    document.getElementById('localizacao-info').style.display = 'block';
                    displayMessage('Horários definidos com sucesso!', 'success');
                } else {
                    displayMessage('Cidade/Estado não encontrado. Verifique a cidade/estado e tente novamente.', 'error');
                }
            });
    });

    // Alternar a visibilidade das configurações de localização
    document.getElementById('toggle-settings-btn').addEventListener('click', function () {
        const container = document.getElementById('toggle-settings-container');
        const isVisible = container.style.display === 'block';
        container.style.display = isVisible ? 'none' : 'block';
        document.getElementById('localizacao-info').style.display = 'none'; // Ocultar as informações anteriores
    });

    // Atualizando o estado inicial dos botões
    updateButtonStates();
});
    </script>
    </body>
</html>
        )rawliteral";

        request->send(200, "text/html", html); });
}