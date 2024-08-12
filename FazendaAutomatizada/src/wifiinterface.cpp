#include <ESPAsyncWebServer.h>
#include <LittleFS.h>


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
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            max-width: 360px;
            margin: auto;
            padding: 15px;
            background: #fff;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
        h2 {
            text-align: center;
            font-size: 1.2em;
            margin-bottom: 20px;
        }
        input[type="text"], input[type="password"], button {
            width: 100%;
            padding: 8px;
            margin: 10px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        button {
            background-color: #28a745;
            color: #fff;
            cursor: pointer;
            border: none;
        }
        .btn-back {
            background-color: #007bff;
        }
        .btn-delete {
            color: #dc3545;
            text-decoration: none;
            font-size: 0.9em;
        }
        #saved-networks, #device-ip {
            margin-top: 15px;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Gerenciamento de Redes Wi-Fi</h2>
        <div id="message"></div>
        <form id="save-form" action="/salvarwifi" method="post">
            <label for="ssid">SSID:</label>
            <input type="text" id="ssid" name="ssid" required>
            <label for="password">Senha:</label>
            <input type="password" id="password" name="password" required>
            <button type="button" id="toggle-password">Mostrar</button>
            <button type="submit" class="btn-submit">Salvar</button>
            <button type="button" onclick="window.history.back()" class="btn-back">Voltar</button>
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
                passwordField.type = passwordField.type === 'password' ? 'text' : 'password';
                this.textContent = passwordField.type === 'password' ? 'Mostrar' : 'Ocultar';
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
