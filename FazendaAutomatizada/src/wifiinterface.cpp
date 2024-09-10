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
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 50px;
            color: #333;
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            box-sizing: border-box;
            transition: background-color 0.3s, color 0.3s;
        }

        main {
            background: #fff;
            padding: 30px;
            border: 1px solid #ddd;
            border-radius: 8px;
            text-align: center;
            width: 100%;
            max-width: 450px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            transition: background-color 0.3s, border-color 0.3s;
            box-sizing: border-box;
        }

        h2 {
            font-size: 24px;
            margin-bottom: 20px;
            color: #333;
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
            color: #ffffff;
            border: none;
            cursor: pointer;
            font-size: 1em;
            transition: background-color 0.3s, box-shadow 0.3s;
        }

        /* Botão Salvar */
        .btn-save {
            background-color: #fd3995; /* Roxo claro */
        }

        .btn-save:hover {
            background-color: #c62370; /* Roxo escuro */
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }

        /* Botão Mostrar/Ocultar Senha */
        .btn-toggle-password {
            background-color: #21d71b; /* Laranja claro */
        }

        .btn-toggle-password:hover {
            background-color: #1ab315; /* Laranja escuro */
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }

        /* Botão Voltar */
        .btn-back {
            background-color: #007bff; /* Azul */
        }

        .btn-back:hover {
            background-color: #0056b3; /* Azul escuro */
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }

        /* Botão Excluir */
        .btn-delete {
            color: #dc3545; /* Vermelho */
            font-size: 0.9em;
            display: block;
            margin-top: 5px;
            text-decoration: none;
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

        /* Modo Noturno */
        .dark-mode {
            background-color: #121212;
            color: #e0e0e0;
        }

        .dark-mode main {
            background: #1e1e1e;
            border: 1px solid #333;
        }

        .dark-mode input, .dark-mode button {
            border-color: #555;
            background-color: #333;
            color: #e0e0e0;
        }

        .dark-mode .btn-save {
            background-color: #fd3995; /* Roxo claro */
        }

        .dark-mode .btn-save:hover {
            background-color: #c62370; /* Roxo escuro */
        }

        .dark-mode .btn-toggle-password {
            background-color: #21d71b; /* Laranja claro */
        }

        .dark-mode .btn-toggle-password:hover {
            background-color: #1ab315; /* Laranja escuro */
        }

        .dark-mode .btn-back {
            background-color: #007bff; /* Azul */
        }

        .dark-mode .btn-back:hover {
            background-color: #0056b3; /* Azul escuro */
        }

        .dark-mode .btn-delete {
            color: #ff6f61; /* Laranja escuro */
        }

        /* Modo Alto Contraste */
        .high-contrast-mode {
            background-color: #000;
            color: #fff;
        }

        .high-contrast-mode main {
            background: #111;
            border: 1px solid #fff;
        }

        .high-contrast-mode input, .high-contrast-mode button {
            border-color: #fff;
            background-color: #333;
            color: #fff;
        }

        .high-contrast-mode .btn-save {
            background-color: #ff0; /* Amarelo */
            color: #000;
        }

        .high-contrast-mode .btn-save:hover {
            background-color: #e0e0e0; /* Cinza claro */
            color: #000;
        }

        .high-contrast-mode .btn-toggle-password {
            background-color: #21d71b; /* Laranja claro */
            color: #000;
        }

        .high-contrast-mode .btn-toggle-password:hover {
            background-color: #1ab315; /* Laranja escuro */
            color: #000;
        }

        .high-contrast-mode .btn-back {
            background-color: #ff7043; /* Laranja claro */
            color: #000;
        }

        .high-contrast-mode .btn-back:hover {
            background-color: #ff5722; /* Laranja escuro */
            color: #000;
        }

        .high-contrast-mode .btn-delete {
            color: #f00; /* Vermelho */
        }

        /* Botão Modo Noturno e Alto Contraste */
        #modeSwitch {
            background-color: #ff7043; /* Laranja claro */
        }

        #modeSwitch:hover {
            background-color: #ff5722; /* Laranja escuro */
        }

        /* Botão Alto Contraste */
        #contrastSwitch {
            background-color: #8e44ad; /* Roxo claro */
        }

        #contrastSwitch:hover {
            background-color: #732d91; /* Roxo escuro */
        }

        @media (max-width: 600px) {
            main {
                padding: 15px;
                width: 100%;
                max-width: 100%;
            }

            input, button {
                padding: 8px;
                font-size: 0.9em;
            }
        }
    </style>
    </head>
    <body>
        <main role="main" aria-labelledby="mainTitle">
            <h2 id="mainTitle">Gerenciamento de Redes Wi-Fi</h2>
            <div id="message" role="alert"></div>
            <form id="save-form" action="/salvarwifi" method="post"
                aria-labelledby="formTitle">
                <h3 id="formTitle" class="sr-only">Formulário de Salvamento de
                    Rede Wi-Fi</h3>
                <input type="text" id="ssid" name="ssid" placeholder="SSID"
                    required aria-required="true" aria-describedby="ssidHelp">
                <small id="ssidHelp" class="form-text">Digite o SSID da rede
                    Wi-Fi.</small>
                <input type="password" id="password" name="password"
                    placeholder="Senha" required aria-required="true"
                    aria-describedby="passwordHelp">
                <small id="passwordHelp" class="form-text">Digite a senha da
                    rede Wi-Fi.</small>
                <button type="button" id="toggle-password"
                    class="btn-toggle-password"
                    aria-label="Mostrar ou ocultar senha">Mostrar</button>
                <button type="submit" class="btn-save">Salvar</button>
            </form>
            <button class="btn-back" onclick="window.history.back()"
                aria-label="Voltar à página anterior">Voltar</button>
            <div id="saved-networks" aria-live="polite"
                aria-labelledby="savedNetworksTitle">
                <h3 id="savedNetworksTitle" class="sr-only">Redes Wi-Fi
                    Salvas</h3>
                Aguardando redes salvas...
            </div>
            <div id="device-ip" aria-live="polite"
                aria-labelledby="deviceIPTitle">
                <h3 id="deviceIPTitle" class="sr-only">IP do Dispositivo</h3>
                Aguardando IP...
            </div>
            <div id="file-contents" aria-live="polite"
                aria-labelledby="fileContentsTitle">
                <h3 id="fileContentsTitle" class="sr-only">Conteúdo do
                    Arquivo</h3>
                Aguardando conteúdo...
            </div>
            <button id="modeSwitch"
                aria-label="Alternar Modo Noturno/Diurno">Modo Noturno</button>
            <button id="contrastSwitch"
                aria-label="Alternar Modo Alto Contraste">Alto
                Contraste</button>
        </main>
        <script>
    document.addEventListener('DOMContentLoaded', () => {
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
    .then(response => response.text())
    .then(message => {
        document.getElementById('message').textContent = message;
        this.reset();
        // Tenta conectar imediatamente após salvar
        fetch('/conectarwifi', {
            method: 'POST',
            body: new FormData(this)
        })
        .then(response => response.text())
        .then(connectionMessage => {
            document.getElementById('message').textContent = connectionMessage;
        });
    })
    .catch(() => {
        document.getElementById('message').textContent = 'Erro ao conectar na rede';
    });
});

        // Modo Noturno e Alto Contraste
        document.getElementById('modeSwitch').addEventListener('click', function() {
            document.body.classList.toggle('dark-mode');
        });

        document.getElementById('contrastSwitch').addEventListener('click', function() {
            document.body.classList.toggle('high-contrast-mode');
        });
    });

    const fetchSavedNetworks = () => {
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
    };

    const updateDeviceIP = () => {
        fetch('/getip')
            .then(response => response.text())
            .then(ip => {
                document.getElementById('device-ip').innerHTML = `<p>IP do Dispositivo: ${ip}</p>`;
            })
            .catch(() => {
                document.getElementById('device-ip').innerHTML = '<p>Erro ao buscar IP do dispositivo.</p>';
            });
    };

    const fetchFileContents = () => {
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
    };
    setInterval(updateDeviceIP, 10000);
    setInterval(fetchFileContents, 10000);
</script>
    </body>
</html>
    )rawliteral";
}
