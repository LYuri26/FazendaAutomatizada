#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

// Definição do servidor web
extern AsyncWebServer server;

// Função que retorna a página de gerenciamento Wi-Fi
const char *getWiFiGerenciamentoPage()
{
    return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gerenciamento Wi-Fi</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f9f9f9;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            max-width: 300px;
            padding: 15px;
            background: #fff;
            border-radius: 5px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            text-align: center;
        }
        h2 {
            margin-bottom: 15px;
            font-size: 1.1em;
            color: #333;
        }
        input, button {
            width: 100%;
            padding: 8px;
            margin: 6px 0;
            border: 1px solid #ddd;
            border-radius: 3px;
            box-sizing: border-box;
        }
        button {
            background-color: #007bff;
            color: #fff;
            border: none;
            cursor: pointer;
        }
        .btn-delete {
            color: #dc3545;
            font-size: 0.9em;
            display: block;
            margin-top: 5px;
            text-decoration: none;
        }
        #saved-networks, #device-ip {
            margin-top: 10px;
            font-size: 0.9em;
            color: #555;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Gerenciamento de Redes Wi-Fi</h2>
        <div id="message"></div>
        <form id="save-form" action="/salvarwifi" method="post">
            <input type="text" id="ssid" name="ssid" placeholder="SSID" required>
            <input type="password" id="password" name="password" placeholder="Senha" required>
            <button type="button" id="toggle-password">Mostrar</button>
            <button type="submit">Salvar</button>
        </form>
        <div id="saved-networks">Aguardando redes salvas...</div>
        <div id="device-ip">Aguardando IP...</div>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            fetchSavedNetworks();
            fetchDeviceIP();

            document.getElementById('toggle-password').addEventListener('click', function() {
                const passwordField = document.getElementById('password');
                const isPassword = passwordField.type === 'password';
                passwordField.type = isPassword ? 'text' : 'password';
                this.textContent = isPassword ? 'Ocultar' : 'Mostrar';
            });

            document.getElementById('save-form').addEventListener('submit', function(event) {
                event.preventDefault();
                fetch(this.action, {
                    method: this.method,
                    body: new FormData(this)
                })
                .then(() => {
                    document.getElementById('message').textContent = 'Rede conectada com sucesso!';
                    this.reset();
                    fetchSavedNetworks();
                    fetchDeviceIP();
                })
                .catch(() => {
                    document.getElementById('message').textContent = 'Erro ao conectar na rede';
                });
            });
        });

        function fetchSavedNetworks() {
            fetch('/listadewifi')
                .then(response => response.text())
                .then(data => {
                    const savedNetworks = document.getElementById('saved-networks');
                    savedNetworks.innerHTML = data.trim() ? 
                        data.trim().split('\n').map(line => {
                            const [ssid] = line.split(',');
                            return `<p>SSID: ${ssid} <a class="btn-delete" href="/excluirwifi?ssid=${ssid}">Excluir</a></p>`;
                        }).join('') : '<p>Nenhuma rede salva encontrada.</p>';
                })
                .catch(() => {
                    document.getElementById('saved-networks').innerHTML = '<p>Erro ao buscar redes salvas.</p>';
                });
        }

        function fetchDeviceIP() {
            fetch('/getip')
                .then(response => response.text())
                .then(ip => {
                    document.getElementById('device-ip').innerHTML = `<p>IP do Dispositivo: ${ip}</p>`;
                })
                .catch(() => {
                    document.getElementById('device-ip').innerHTML = '<p>Erro ao buscar IP do dispositivo.</p>';
                });
        }
    </script>
</body>
</html>
    )rawliteral";
}
