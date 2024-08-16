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
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            max-width: 350px;
            padding: 20px;
            background: #ffffff;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            text-align: center;
        }
        h2 {
            margin-bottom: 20px;
            font-size: 1.2em;
            color: #444;
        }
        input, button {
            width: calc(100% - 16px);
            padding: 10px;
            margin: 8px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        button {
            background-color: #28a745;
            color: #ffffff;
            border: none;
            cursor: pointer;
            font-size: 1em;
        }
        button:hover {
            background-color: #218838;
        }
        .btn-delete {
            color: #dc3545;
            font-size: 0.9em;
            display: block;
            margin-top: 5px;
            text-decoration: none;
        }
        .btn-back {
            background-color: #007bff;
            color: #ffffff;
            border: none;
            cursor: pointer;
            font-size: 1em;
        }
        .btn-back:hover {
            background-color: #0056b3;
        }
        #saved-networks, #device-ip, #file-contents {
            margin-top: 15px;
            font-size: 0.9em;
            color: #555;
        }
        #message {
            color: #dc3545;
            font-size: 1em;
            margin-bottom: 10px;
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
        <button class="btn-back" onclick="window.history.back()">Voltar</button>
        <div id="saved-networks">Aguardando redes salvas...</div>
        <div id="device-ip">Aguardando IP...</div>
        <div id="file-contents">Aguardando conteúdo do arquivo...</div>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            fetchSavedNetworks();
            updateDeviceIP();
            fetchFileContents();

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
                    updateDeviceIP();
                    fetchFileContents();
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

        function updateDeviceIP() {
            fetch('/getip')
                .then(response => response.text())
                .then(ip => {
                    document.getElementById('device-ip').innerHTML = `<p>IP do Dispositivo: ${ip}</p>`;
                })
                .catch(() => {
                    document.getElementById('device-ip').innerHTML = '<p>Erro ao buscar IP do dispositivo.</p>';
                });
        }

        function fetchFileContents() {
            fetch('/filecontents')
                .then(response => response.text())
                .then(content => {
                    const fileContents = document.getElementById('file-contents');
                    fileContents.innerHTML = content.trim() ? 
                        content.trim().split('\n').map(line => {
                            const [ssid] = line.split(',');
                            return `<p>SSID: ${ssid}</p>`;
                        }).join('') : '<p>Arquivo vazio.</p>';
                })
                .catch(() => {
                    document.getElementById('file-contents').innerHTML = '<p>Erro ao buscar conteúdo do arquivo.</p>';
                });
        }

        // Atualiza o IP e o conteúdo do arquivo a cada 10 segundos
        setInterval(updateDeviceIP, 10000);
        setInterval(fetchFileContents, 10000);
    </script>
</body>
</html>
    )rawliteral";
}
