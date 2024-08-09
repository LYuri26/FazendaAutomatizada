const char* getWiFiGerenciamentoPage() {
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
            background-color: #f8f9fa;
        }
        .container {
            max-width: 400px;
            margin: 0 auto;
            background-color: #ffffff;
            padding: 15px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        h2 {
            text-align: center;
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
        }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 8px;
            margin-bottom: 10px;
            border: 1px solid #ccc;
            border-radius: 3px;
            box-sizing: border-box;
        }
        button {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: none;
            border-radius: 3px;
            cursor: pointer;
        }
        .btn-success {
            background-color: #28a745;
            color: white;
        }
        .btn-blue {
            background-color: #007bff;
            color: white;
        }
        .btn-danger {
            background-color: #dc3545;
            color: white;
            display: inline-block;
            padding: 5px 10px;
            margin-top: 5px;
            text-align: center;
            text-decoration: none;
            font-size: 14px;
        }
        #saved-networks {
            margin-top: 20px;
            max-height: 150px;
            overflow-y: auto;
            border-top: 1px solid #ddd;
            padding-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Gerenciamento de Redes Wi-Fi</h2>
        <div id="message"></div>
        <form id="save-form" action="/salvarwifi" method="post">
            <div class="form-group">
                <label for="ssid">SSID da Rede Wi-Fi:</label>
                <input type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label for="password">Senha:</label>
                <input type="password" id="password" name="password" required>
                <button type="button" id="toggle-password">Mostrar</button>
            </div>
            <button type="submit" class="btn-success">Conectar</button>
            <button type="button" onclick="window.history.back()" class="btn-blue">Voltar</button>
        </form>
        <div id="saved-networks">Aguardando redes salvas...</div>
    </div>
    <script>
        function fetchSavedNetworks() {
            fetch('/listadewifi')
                .then(response => response.text())
                .then(data => {
                    var savedNetworks = document.getElementById('saved-networks');
                    savedNetworks.innerHTML = '';
                    if (data.trim()) {
                        var networks = data.trim().split('\n');
                        networks.forEach(network => {
                            var parts = network.split(',');
                            savedNetworks.innerHTML += '<p><strong>SSID:</strong> ' + parts[0] + ' <a class="btn-danger" href="/excluirwifi?ssid=' + parts[0] + '">Delete</a></p>';
                        });
                    } else {
                        savedNetworks.innerHTML = '<p>Nenhuma rede salva encontrada.</p>';
                    }
                })
                .catch(error => {
                    document.getElementById('saved-networks').innerHTML = '<p>Erro ao buscar redes salvas.</p>';
                });
        }

        document.addEventListener('DOMContentLoaded', fetchSavedNetworks);

        document.getElementById('toggle-password').addEventListener('click', function() {
            var passwordField = document.getElementById('password');
            passwordField.type = passwordField.type === 'password' ? 'text' : 'password';
            this.textContent = this.textContent === 'Mostrar' ? 'Ocultar' : 'Mostrar';
        });

        document.getElementById('save-form').addEventListener('submit', function(event) {
            event.preventDefault();
            fetch(this.action, {
                method: this.method,
                body: new FormData(this)
            })
            .then(response => response.text())
            .then(data => {
                document.getElementById('message').textContent = 'Rede conectada com sucesso!';
                this.reset();
                fetchSavedNetworks();
            })
            .catch(error => {
                document.getElementById('message').textContent = 'Erro ao conectar na rede';
            });
        });
    </script>
</body>
</html>
    )rawliteral";
}
