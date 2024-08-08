#include <ESPAsyncWebServer.h>
#include "index.h"
#include <LittleFS.h>

void setupIndexPage(AsyncWebServer& server) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"rawliteral(
            <!DOCTYPE html>
            <html lang="pt-br">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Login</title>
                <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #f8f9fa;
                        height: 100vh;
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        margin: 0;
                        padding: 0;
                    }
                    .login-container {
                        background-color: #ffffff;
                        padding: 20px;
                        border-radius: 5px;
                        box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                        width: 100%;
                        max-width: 400px;
                        text-align: center;
                    }
                    .login-title {
                        font-size: 24px;
                        margin-bottom: 20px;
                        color: #007bff;
                    }
                    .footer {
                        position: fixed;
                        bottom: 0;
                        width: 100%;
                        background-color: #007bff;
                        color: white;
                        text-align: center;
                        padding: 10px 0;
                        font-size: 14px;
                    }
                    .text-danger {
                        color: #dc3545;
                    }
                </style>
            </head>
            <body>
                <div class="login-container">
                    <h2 class="login-title">Faça o Login</h2>
                    <form action="/login" method="post" onsubmit="storeLoginData(event)">
                        <div class="form-group">
                            <input type="text" name="username" class="form-control" placeholder="Usuário" required>
                        </div>
                        <div class="form-group">
                            <input type="password" name="password" class="form-control" placeholder="Senha" required>
                        </div>
                        <button type="submit" class="btn btn-primary btn-block">Entrar</button>
                        <div class="text-danger mt-2">%ERROR_MESSAGE%</div>
                    </form>
                    <button onclick="window.location.href='/creditos'" class="btn btn-secondary btn-block mt-3">Créditos</button>
                    <button onclick="window.location.href='/wifigerenciamento'" class="btn btn-warning btn-block mt-3">Gerenciamento Wi-Fi</button>
                </div>
                <div class="footer">
                    <p>Aplicação desenvolvida pela Turma de Informática Para Internet Trilhas de Futuro 2024</p>
                    <p>Instrutor: Lenon Yuri</p>
                </div>
                <script>
                    function storeLoginData(event) {
                        event.preventDefault();
                        var username = document.querySelector('input[name="username"]').value;
                        var password = document.querySelector('input[name="password"]').value;
                        console.log('Tentando login com usuário: ' + username + ' e senha: ' + password);
                        event.target.submit();
                    }
                </script>
            </body>
            </html>
        )rawliteral";

        String errorMessage;
        if (request->hasParam("login_failed")) {
            errorMessage = "Usuário ou senha incorretos. Tente novamente.";
        } else {
            errorMessage = "";
        }

        html.replace("%ERROR_MESSAGE%", errorMessage);
        request->send(200, "text/html", html);
    });
}