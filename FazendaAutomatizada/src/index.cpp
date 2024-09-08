#include <ESPAsyncWebServer.h>
#include "index.h"
#include "tempo.h"

void setupIndexPage(AsyncWebServer &server)
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String errorMessage;
        if (request->hasParam("login_failed"))
            errorMessage = "Usuário ou senha incorretos.";
        else
            errorMessage = "";

        String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login</title>
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
        .login-container {
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
        .login-title {
            font-size: 24px;
            margin-bottom: 20px;
            color: #333;
        }
        .form-group {
            margin-bottom: 15px;
            text-align: left;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        .form-group input {
            width: 100%;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 6px;
            font-size: 16px;
            box-sizing: border-box;
            transition: border-color 0.3s ease, background-color 0.3s;
        }
        .form-group input:focus {
            border-color: #6e8efb;
            outline: none;
        }
        .btn {
            display: block;
            width: 100%;
            padding: 12px;
            margin-top: 10px;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.3s ease, box-shadow 0.3s ease;
        }
        .btn-primary {
            background-color: #4a90e2;
            color: #fff;
        }
        .btn-primary:hover {
            background-color: #357abd;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }
        .btn-secondary {
            background-color: #6c757d;
            color: #fff;
        }
        .btn-secondary:hover {
            background-color: #5a6268;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }
        .btn-warning {
            background-color: #ffc107;
            color: #000;
        }
        .btn-warning:hover {
            background-color: #e0a800;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
        }
        .text-danger {
            color: #dc3545;
            margin-top: 10px;
            font-size: 14px;
        }
        .button-group {
            margin-top: 20px;
        }
        .switch {
            margin-top: 20px;
            display: flex;
            flex-wrap: wrap; /* Permite que os botões se movam para a próxima linha se necessário */
            gap: 10px; /* Espaço entre os botões */
            justify-content: center; /* Centraliza os botões horizontalmente */
        }
        .switch .btn {
            padding: 10px 20px;
        }
        .switch .btn-active {
            background-color: #4a90e2;
            color: #fff;
        }
        .switch .btn-inactive {
            background-color: #ddd;
            color: #333;
        }
        .switch .btn-active:hover,
        .switch .btn-inactive:hover {
            background-color: #357abd;
            color: #fff;
        }

        /* Modo Noturno */
        .dark-mode {
            background-color: #121212;
            color: #e0e0e0;
        }
        .dark-mode .login-container {
            background: #1e1e1e;
            border-color: #333;
        }
        .dark-mode .form-group input {
            border-color: #555;
            background-color: #333;
            color: #eee;
        }
        .dark-mode .btn-primary {
            background-color: #007bff;
        }
        .dark-mode .btn-primary:hover {
            background-color: #0056b3;
        }
        .dark-mode .btn-secondary {
            background-color: #5a5a5a;
        }
        .dark-mode .btn-secondary:hover {
            background-color: #3e3e3e;
        }
        .dark-mode .btn-warning {
            background-color: #ffcc00;
            color: #000;
        }
        .dark-mode .btn-warning:hover {
            background-color: #e0a800;
        }

        /* Modo Alto Contraste */
        .high-contrast-mode {
            background-color: #000;
            color: #fff;
        }
        .high-contrast-mode .login-container {
            background: #111;
            border-color: #fff;
        }
        .high-contrast-mode .form-group input {
            border-color: #fff;
            background-color: #333;
            color: #fff;
        }
        .high-contrast-mode .btn-primary {
            background-color: #ff0;
            color: #000;
        }
        .high-contrast-mode .btn-primary:hover {
            background-color: #e0e0e0;
            color: #000;
        }
        .high-contrast-mode .btn-secondary {
            background-color: #fff;
            color: #000;
        }
        .high-contrast-mode .btn-secondary:hover {
            background-color: #e0e0e0;
            color: #000;
        }
        .high-contrast-mode .btn-warning {
            background-color: #f00;
            color: #fff;
        }
        .high-contrast-mode .btn-warning:hover {
            background-color: #c00;
            color: #fff;
        }
        .high-contrast-mode .text-danger {
            color: #f00;
        }

        @media (max-width: 600px) {
            .login-container {
                padding: 20px;
            }
            .btn {
                padding: 10px;
                font-size: 14px;
            }
            .switch .btn {
                padding: 8px 16px;
            }
        }
    </style>
</head>
<body>
    <main class="login-container" role="main" aria-labelledby="loginTitle">
        <h1 id="loginTitle" class="login-title">Faça o Login</h1>
        <form action="/login" method="post" aria-labelledby="loginTitle"
            aria-describedby="loginDesc">
            <p id="loginDesc" class="sr-only">Preencha suas credenciais para fazer login.</p>

            <div class="form-group">
                <label for="username">Usuário</label>
                <input type="text" id="username" name="username"
                    placeholder="Usuário" required aria-required="true"
                    aria-describedby="usernameHelp">
                <small id="usernameHelp" class="form-text">Digite seu nome de usuário.</small>
            </div>

            <div class="form-group">
                <label for="password">Senha</label>
                <input type="password" id="password" name="password"
                    placeholder="Senha" required aria-required="true"
                    aria-describedby="passwordHelp">
                <small id="passwordHelp" class="form-text">Digite sua senha.</small>
            </div>

            <button type="submit" class="btn btn-primary">Entrar</button>
            <div id="errorMessage" class="text-danger"
                aria-live="assertive">%ERROR_MESSAGE%</div>
        </form>

        <div class="button-group">
            <button onclick="window.location.href='/creditos'"
                class="btn btn-secondary"
                aria-label="Ir para a página de créditos">Créditos</button>
            <button onclick="window.location.href='/wifigerenciamento'"
                class="btn btn-warning"
                aria-label="Ir para a página de gerenciamento de Wi-Fi">Gerenciamento Wi-Fi</button>
        </div>

        <div class="switch" role="region" aria-labelledby="switchTitle">
            <h2 id="switchTitle" class="sr-only">Configurações de Tema</h2>
            <button id="modeSwitch" class="btn btn-inactive"
                aria-label="Alternar modo noturno">Modo Noturno</button>
            <button id="contrastSwitch" class="btn btn-inactive"
                aria-label="Alternar contraste alto">Alto Contraste</button>
        </div>
    </main>

    <script>
        // Alternar entre modo claro, escuro e alto contraste
        document.getElementById('modeSwitch').addEventListener('click', function() {
            if (document.body.classList.contains('dark-mode')) {
                document.body.classList.remove('dark-mode');
                document.getElementById('modeSwitch').classList.remove('btn-active');
                document.getElementById('modeSwitch').classList.add('btn-inactive');
                const highContrast = document.getElementById('contrastSwitch').classList.contains('btn-active');
                document.body.classList.toggle('high-contrast-mode', highContrast);
                localStorage.setItem('theme', highContrast ? 'high-contrast' : 'light');
            } else {
                document.body.classList.add('dark-mode');
                document.getElementById('modeSwitch').classList.remove('btn-inactive');
                document.getElementById('modeSwitch').classList.add('btn-active');
                localStorage.setItem('theme', 'dark');
            }
        });

        document.getElementById('contrastSwitch').addEventListener('click', function() {
            if (document.body.classList.contains('high-contrast-mode')) {
                document.body.classList.remove('high-contrast-mode');
                document.getElementById('contrastSwitch').classList.remove('btn-active');
                document.getElementById('contrastSwitch').classList.add('btn-inactive');
                const darkMode = document.getElementById('modeSwitch').classList.contains('btn-active');
                document.body.classList.toggle('dark-mode', darkMode);
                localStorage.setItem('theme', darkMode ? 'dark' : 'light');
            } else {
                document.body.classList.add('high-contrast-mode');
                document.getElementById('contrastSwitch').classList.remove('btn-inactive');
                document.getElementById('contrastSwitch').classList.add('btn-active');
                localStorage.setItem('theme', 'high-contrast');
            }
        });

        // Aplicar o tema salvo no armazenamento local
        window.addEventListener('load', function() {
            const savedTheme = localStorage.getItem('theme');
            if (savedTheme === 'dark') {
                document.body.classList.add('dark-mode');
                document.getElementById('modeSwitch').classList.remove('btn-inactive');
                document.getElementById('modeSwitch').classList.add('btn-active');
            } else if (savedTheme === 'high-contrast') {
                document.body.classList.add('high-contrast-mode');
                document.getElementById('contrastSwitch').classList.remove('btn-inactive');
                document.getElementById('contrastSwitch').classList.add('btn-active');
            }
        });
    </script>
</body>
</html>
        )rawliteral";

        html.replace("%ERROR_MESSAGE%", errorMessage);
        request->send(200, "text/html", html); });
}
