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
        /* Estilo base do corpo */
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f5f5f5;
            /* Cor neutra de fundo */
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0;
            padding-top: 50px;
            padding-bottom: 50px;
            transition: background-color 0.3s, color 0.3s;
        }

        /* Container do dashboard */
        .dashboard-container {
            background-color: #ffffff;
            /* Cor clara para contraste */
            padding: 30px;
            border-radius: 12px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            text-align: center;
            width: 100%;
            max-width: 600px;
            box-sizing: border-box;
            transition: background-color 0.3s, border 0.3s;
            border: 1px solid #dcdcdc;
            /* Borda cinza */
        }

        /* Cabeçalhos */
        h2 {
            margin-bottom: 20px;
            font-size: 24px;
            color: #333333;
            /* Cor escura para bom contraste */
        }

        /* Contêiner de Botões de Luzes */
        .light-buttons,
        .logout-container,
        .toggle-buttons,
        .toggle-settings,
        .localizacao-info,
        .horario-container {
            margin-top: 20px;
            background-color: #f9f9f9;
            /* Fundo cinza claro */
            padding: 15px;
            border: 1px solid #dcdcdc;
            /* Borda cinza */
            border-radius: 12px;
            /* Bordas arredondadas */
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            /* Sombra sutil */
            text-align: center;
            /* Centraliza o conteúdo dentro do contêiner */
            width: 100%;
            max-width: 600px;
            box-sizing: border-box;
        }

        /* Estilo base dos botões */
        .btn,
        .button-toggle,
        .btn-mode-dark,
        .btn-mode-contrast {
            display: block;
            padding: 14px;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            width: 100%;
            max-width: 300px;
            /* Largura máxima para botões maiores */
            color: #ffffff;
            transition: background-color 0.3s, transform 0.2s;
            margin: 10px auto;
            /* Centraliza horizontalmente */
            background-color: #2085ac;
        }

        /* Cores específicas dos botões */
        .btn-luz-casa {
            background-color: #4caf50;
        }

        /* Verde para Luz da Casa */
        .btn-luz-rua {
            background-color: #2196f3;
        }

        /* Azul para Luz da Rua */
        .btn-luz-pasto {
            background-color: #fbc02d;
        }

        /* Amarelo para Luz do Pasto */
        .btn-luz-geral {
            background-color: #b047a5;
        }

        /* rosa para Luz Geral */
        .btn-desligar {
            background-color: #f44336;
        }

        /* Vermelho para Desligar */
        .btn-logout {
            background-color: #e71111;
        }

        /* Laranja para Logout */

        .button-toggle {
            background-color: #2196f3;
        }

        /* Azul padrão */
        #toggle-settings-btn {
            background-color: #557fbf;
        }

        /* Verde */
        #toggle-horarios-btn {
            background-color: #ff9800;
        }

        /* Laranja */

        .btn:hover,
        .button-toggle:hover {
            opacity: 0.9;
            transform: scale(1.02);
        }

        /* Estilo da mensagem de sucesso e erro */
        .message {
            margin-top: 20px;
            padding: 10px;
            border-radius: 8px;
            text-align: center;
            font-size: 16px;
            display: none;
            background-color: #f9f9f9;
            /* Fundo cinza claro */
            border: 1px solid #dcdcdc;
            /* Borda cinza */
        }

        /* Estilo da mensagem de sucesso e erro */
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

        /* Estilo para o switch */
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
            background-color: #b0bec5;
            /* Cinza claro */
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
            background-color: #ffffff;
            transition: .4s;
        }

        input:checked+.slider {
            background-color: #2196f3;
            /* Azul */
        }

        input:checked+.slider:before {
            transform: translateX(26px);
        }

        .slider-text {
            margin-left: 10px;
            font-size: 16px;
            color: #333333;
        }

        /* Ajustes para modo noturno */
        .dark-mode {
            background-color: #121212;
            color: #e0e0e0;
        }

        .dark-mode .dashboard-container {
            background-color: #1e1e1e;
            border: 1px solid #333333;
        }

        .dark-mode .btn-luz-casa {
            background-color: #66bb6a;
        }

        /* Verde claro */
        .dark-mode .btn-luz-rua {
            background-color: #64b5f6;
        }

        /* Azul claro */
        .dark-mode .btn-luz-pasto {
            background-color: #fbc02d;
        }

        /* Amarelo claro */
        .dark-mode .btn-luz-geral {
            background-color: #b047a5;
        }

        /* Cinza claro */
        .dark-mode .btn-desligar {
            background-color: #e57373;
        }

        /* Vermelho claro */
        .dark-mode .btn-logout {
            background-color: #e71111;
        }

        /* Laranja claro */

        .dark-mode .button-toggle {
            background-color: #64b5f6;
            /* Azul claro */
        }

        .dark-mode #toggle-settings-btn {
            background-color: #557fbf;
            /* Verde claro */
        }

        .dark-mode #toggle-horarios-btn {
            background-color: #ff9800;
            /* Laranja claro */
        }

        .dark-mode .toggle-settings,
        .dark-mode .localizacao-info,
        .dark-mode .horario-container {
            background-color: #333333;
            border-color: #444444;
        }

        .dark-mode h2,
        .dark-mode p,
        .dark-mode label {
            color: #e0e0e0;
        }

        /* Ajustes para modo alto contraste */
        .high-contrast-mode {
            background-color: #000000;
            color: #ffffff;
        }

        .high-contrast-mode .dashboard-container {
            background-color: #111111;
            border: 1px solid #ffffff;
        }

        .high-contrast-mode .btn-luz-casa {
            background-color: #4caf50;
        }

        /* Verde visível */
        .high-contrast-mode .btn-luz-rua {
            background-color: #2196f3;
        }

        /* Azul visível */
        .high-contrast-mode .btn-luz-pasto {
            background-color: #fbc02d;
        }

        /* Amarelo claro */
        .high-contrast-mode .btn-luz-geral {
            background-color: #b047a5;
        }

        /* Cinza médio */
        .high-contrast-mode .btn-desligar {
            background-color: #f44336;
        }

        /* Vermelho intenso */
        .high-contrast-mode .btn-logout {
            background-color: #e71111;
        }

        /* Laranja intenso */

        .high-contrast-mode .button-toggle {
            background-color: #000000;
            color: #ffffff;
        }

        .high-contrast-mode #toggle-settings-btn {
            background-color: #557fbf;
            /* Verde visível */
        }

        .high-contrast-mode #toggle-horarios-btn {
            background-color: #ff9800;
            /* Laranja claro */
        }

        .high-contrast-mode .toggle-settings,
        .high-contrast-mode .localizacao-info,
        .high-contrast-mode .horario-container {
            background-color: #000000;
            border-color: #ffffff;
        }

        .high-contrast-mode h2,
        .high-contrast-mode p,
        .high-contrast-mode label {
            color: #ffffff;
        }

        /* Botões de Modo Noturno e Alto Contraste */
        .mode-buttons {
            margin-top: 30px;
            display: flex;
            flex-direction: column;
            /* Disposição vertical dos botões */
            align-items: center;
            gap: 10px;
        }

        .btn-mode-dark,
        .btn-mode-contrast {
            background-color: #007bff;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            padding: 14px;
            font-size: 18px;
            font-weight: bold;
            transition: background-color 0.3s, transform 0.2s;
            width: 100%;
            max-width: 300px;
            /* Largura máxima para os botões */
            margin: 10px auto;
            /* Centraliza horizontalmente */
        }

        .btn-mode-dark {
            background-color: #777;
            /* Laranja mais visível */
        }

        .btn-mode-contrast {
            background-color: #000;
            /* Verde mais visível */
        }

        .btn-mode-dark:hover {
            background-color: #777;
        }

        .btn-mode-contrast:hover {
            background-color: #000;
        }

        /* Responsividade */
        @media (max-width: 600px) {
            .dashboard-container {
                margin: 10px;
                padding: 20px;
            }

            .btn,
            .button-toggle {
                max-width: 100%;
                /* Botões ocupam toda a largura disponível */
            }

            .mode-buttons {
                flex-direction: column;
                /* Botões de modo empilhados verticalmente em telas pequenas */
                gap: 5px;
            }
        }

        /* Estilo para campos de entrada de texto e hora */
        input[type="text"],
        input[type="time"] {
            width: calc(100% - 30px);
            /* Ajusta a largura para acomodar o padding */
            padding: 10px;
            font-size: 16px;
            border: 1px solid #dcdcdc;
            /* Borda cinza */
            border-radius: 8px;
            margin-top: 10px;
            text-align: center;
        }

        input[type="text"]::placeholder,
        input[type="time"]::placeholder {
            color: #999999;
            /* Cor do texto do placeholder */
        }

        /* Ajuste para campos de texto em modo escuro e alto contraste */
        .dark-mode input[type="text"],
        .dark-mode input[type="time"],
        .high-contrast-mode input[type="text"],
        .high-contrast-mode input[type="time"] {
            background-color: #333333;
            color: #e0e0e0;
            border-color: #444444;
        }

        .dark-mode input[type="text"]::placeholder,
        .dark-mode input[type="time"]::placeholder,
        .high-contrast-mode input[type="text"]::placeholder,
        .high-contrast-mode input[type="time"]::placeholder {
            color: #bbbbbb;
            /* Placeholder mais claro */
        }
    </style>
</head>

<body>
    <div class="dashboard-container">
        <h2 id="mainTitle">Painel de Controle</h2>

        <!-- Contêiner de Botões de Luzes -->
        <div class="light-buttons">
            <button id="luz0" class="btn btn-luz-casa">Luz do Primeiro Andar</button>
            <button id="luz1" class="btn btn-luz-rua">Luz do Segundo Andar</button>
            <button id="luz2" class="btn btn-luz-pasto">Luz Externa</button>
            <button id="luz3" class="btn btn-luz-geral">Luz Geral</button>
        </div>

        <!-- Contêiner do Botão de Logout -->
        <div class="logout-container">
            <button id="logout" class="btn btn-logout" onclick="location.href='/logout'">Logout</button>
        </div>

        <!-- Contêiner da Mensagem de Status -->
        <div id="message" class="message" aria-live="assertive"></div>

        <!-- Contêiner de Botões de Modo Noturno e Alto Contraste -->
        <div class="mode-buttons">
            <button id="modeSwitch" class="btn btn-mode-dark">Modo Noturno</button>
            <button id="contrastSwitch" class="btn btn-mode-contrast">Alto Contraste</button>
        </div>

        <!-- Contêiner dos Botões para Alternar Seções -->
        <div class="toggle-buttons">
            <button id="toggle-settings-btn" class="button-toggle">Definir Localização Automática</button>
            <button id="toggle-horarios-btn" class="button-toggle">Configurações de Horários</button>
        </div>

        <!-- Seção para Definir Localização Automática -->
        <div class="toggle-settings" id="toggle-settings-container" style="display: none;">
            <p>Deseja definir um horário automático baseado no pôr do sol e nascer do sol para ligar e desligar as luzes?</p>
            <input type="text" id="cidade-estado" placeholder="Exemplo: Uberaba-MG">
            <button id="definir-localizacao" class="btn">Definir Localização</button>
        </div>

        <!-- Seção de Informações da Localização -->
        <div class="localizacao-info" id="localizacao-info" style="display: none;">
            <h3 id="cidade-nome">Cidade</h3>
            <p>Nascer do Sol: <span id="nascer-do-sol">Horário</span></p>
            <p>Pôr do Sol: <span id="por-do-sol">Horário</span></p>
        </div>

        <!-- Seção de Configurações de Horários -->
        <div class="horario-container" id="horario-container" style="display: none;">
            <h3>Configurações de Horários</h3>
            <div>
                <label for="horario-ligar">Horário para Ligar:</label>
                <input type="time" id="horario-ligar">
            </div>
            <div>
                <label for="horario-desligar">Horário para Desligar:</label>
                <input type="time" id="horario-desligar">
            </div>
            <button id="save-horarios" class="btn">Salvar Horários</button>
            <button id="disable-horarios" class="btn">Desativar Horários</button>
        </div>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            // Função para exibir mensagens
            function displayMessage(message, type) {
                const messageBox = document.getElementById('message');
                messageBox.textContent = message;
                messageBox.className = 'message ' + (type === 'success' ? 'success' : 'error');
                messageBox.style.display = 'block';
            }

            // Função para atualizar aparência dos botões
            function updateButtonAppearance(button, isOn) {
                if (button.id === 'logout' || button.id === 'luz3') return; // Ignorar o botão de "Luz Geral"
                if (!button.dataset.originalClass) button.dataset.originalClass = button.className;
                button.textContent = isOn ? 'Desligar' : {
                    'luz0': 'Luz do Primeiro Andar',
                    'luz1': 'Luz do Segundo Andar',
                    'luz2': 'Luz Externa',
                    'luz3': 'Luz Geral'
                } [button.id];
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
                        // Aqui, o estado de Luz Geral pode ser atualizado, mas sem mudar sua aparência
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
                document.querySelectorAll('.btn').forEach(button => {
                    if (button.id.startsWith('luz') && button.id !== 'luz3') { // Ignora "Luz Geral"
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
                    button.addEventListener('click', function() {
                        const buttonId = button.id.replace('luz', '');
                        const action = button.textContent === 'Desligar' ? 'desligar' : 'ligar';

                        if (buttonId === '3') {
                            // Se for o botão de "Luz Geral", verifica o estado atual das luzes
                            if (areAllLightsOn()) {
                                // Desliga todas as luzes se todas estiverem ligadas
                                toggleAllLights('desligar');
                            } else {
                                // Liga todas as luzes se alguma estiver desligada
                                toggleAllLights('ligar');
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
            document.getElementById('modeSwitch').addEventListener('click', function() {
                document.body.classList.toggle('dark-mode');
            });

            // Alternar o modo de alto contraste
            document.getElementById('contrastSwitch').addEventListener('click', function() {
                document.body.classList.toggle('high-contrast-mode');
            });

            // Alternar visibilidade da seção de localização automática
            document.getElementById('toggle-settings-btn').addEventListener('click', function() {
                const container = document.getElementById('toggle-settings-container');
                const isVisible = container.style.display === 'block';
                container.style.display = isVisible ? 'none' : 'block';
                document.getElementById('localizacao-info').style.display = 'none'; // Ocultar as informações anteriores
            });

            // Alternar visibilidade da seção de configurações de horários
            document.getElementById('toggle-horarios-btn').addEventListener('click', function() {
                const container = document.getElementById('horario-container');
                const isVisible = container.style.display === 'block';
                container.style.display = isVisible ? 'none' : 'block';
            });

            // Definir localização automática
            document.getElementById('definir-localizacao').addEventListener('click', function() {
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

            // Configurar horários
            document.getElementById('save-horarios').addEventListener('click', function() {
                const horarioLigar = document.getElementById('horario-ligar').value.trim();
                const horarioDesligar = document.getElementById('horario-desligar').value.trim();

                if (!horarioLigar || !horarioDesligar) {
                    displayMessage('Por favor, insira ambos os horários.', 'error');
                    return;
                }

                const url = `/salvar-horarios?ligar=${encodeURIComponent(horarioLigar)}&desligar=${encodeURIComponent(horarioDesligar)}`;
                fetch(url)
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            displayMessage('Horários salvos com sucesso!', 'success');
                        } else {
                            displayMessage('Erro ao salvar horários. Tente novamente.', 'error');
                        }
                    });
            });

            document.getElementById('disable-horarios').addEventListener('click', function() {
                fetch('/desativar-horarios')
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            displayMessage('Horários desativados com sucesso!', 'success');
                        } else {
                            displayMessage('Erro ao desativar horários. Tente novamente.', 'error');
                        }
                    });
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