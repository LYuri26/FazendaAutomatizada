#include <ESPAsyncWebServer.h>
#include "tempo.h"

void setupIndexPage(AsyncWebServer &server)
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
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
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            color: #333;
        }
        .login-container {
            background: #fff;
            padding: 30px;
            border: 1px solid #ddd;
            border-radius: 8px;
            text-align: center;
            width: 100%;
            max-width: 400px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
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
        .form-group input {
            width: 100%;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 6px;
            font-size: 16px;
            box-sizing: border-box;
            transition: border-color 0.3s ease;
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
            background-color: #6e8efb;
            color: #fff;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.3s ease;
        }
        .btn:hover {
            background-color: #5a7be0;
        }
        .btn-secondary {
            background-color: #6c757d;
        }
        .btn-secondary:hover {
            background-color: #5a6268;
        }
        .btn-warning {
            background-color: #ffc107;
        }
        .btn-warning:hover {
            background-color: #e0a800;
        }
        .text-danger {
            color: #dc3545;
            margin-top: 10px;
            font-size: 14px;
        }
        #current-time {
            font-size: 16px;
            margin-top: 20px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="login-container">
        <h2 class="login-title">Faça o Login</h2>
        <form action="/login" method="post">
            <div class="form-group">
                <input type="text" name="username" placeholder="Usuário" required>
            </div>
            <div class="form-group">
                <input type="password" name="password" placeholder="Senha" required>
            </div>
            <button type="submit" class="btn">Entrar</button>
            <div class="text-danger">%ERROR_MESSAGE%</div>
        </form>
        <button onclick="window.location.href='/creditos'" class="btn btn-secondary">Créditos</button>
        <button onclick="window.location.href='/wifigerenciamento'" class="btn btn-warning">Gerenciamento Wi-Fi</button>
        <div id="current-time">Carregando hora...</div>
    </div>
    <script>
        function fetchCurrentTime() {
            fetch('/currenttime')
                .then(response => response.text())
                .then(time => {
                    document.getElementById('current-time').textContent = 'Hora Atual: ' + time;
                })
                .catch(() => {
                    document.getElementById('current-time').textContent = 'Erro ao obter a hora.';
                });
        }

        function updateClock() {
            fetchCurrentTime();
            setTimeout(updateClock, 500); // Atualiza a cada 500 ms
        }

        document.addEventListener('DOMContentLoaded', function() {
            updateClock(); // Inicia a atualização da hora imediatamente
        });
    </script>
</body>
</html>
        )rawliteral";

        String errorMessage;
        if (request->hasParam("login_failed")) {
            errorMessage = "Usuário ou senha incorretos.";
        } else {
            errorMessage = "";
        }

        html.replace("%ERROR_MESSAGE%", errorMessage);
        request->send(200, "text/html", html); });

    server.on("/currenttime", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String time = getTimeClient(); // Certifique-se de que essa função está correta
        request->send(200, "text/plain", time); });
}
