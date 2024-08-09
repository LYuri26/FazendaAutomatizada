#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "index.h"

// Função para configurar a página de login
void setupIndexPage(AsyncWebServer& server) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // HTML da página de login com placeholders para mensagens de erro
        String html = R"rawliteral(
            <!DOCTYPE html>
            <html lang="pt-br">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Login</title>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #f8f9fa;
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        height: 100vh;
                        margin: 0;
                    }
                    .login-container {
                        background-color: #fff;
                        padding: 20px;
                        border: 1px solid #ddd;
                        border-radius: 4px;
                        text-align: center;
                        width: 100%;
                        max-width: 300px;
                    }
                    .login-title {
                        font-size: 20px;
                        margin-bottom: 15px;
                        color: #333;
                    }
                    .form-group {
                        margin-bottom: 10px;
                        text-align: left;
                    }
                    .form-group input {
                        width: 100%;
                        padding: 8px;
                        border: 1px solid #ccc;
                        border-radius: 4px;
                        font-size: 14px;
                    }
                    .btn {
                        display: block;
                        width: 100%;
                        padding: 8px;
                        margin-top: 10px;
                        background-color: #007bff;
                        color: #fff;
                        border: none;
                        border-radius: 4px;
                        cursor: pointer;
                        text-align: center;
                    }
                    .btn-secondary {
                        background-color: #6c757d;
                        margin-top: 10px;
                    }
                    .btn-warning {
                        background-color: #ffc107;
                        margin-top: 10px;
                    }
                    .text-danger {
                        color: #dc3545;
                        margin-top: 10px;
                    }
                </style>
            </head>
            <body>
                <div class="login-container">
                    <h2 class="login-title">Faça o Login</h2>
                    <form action="/login" method="post" onsubmit="storeLoginData(event)">
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

        // Verifica se há um parâmetro na URL que indica falha de login
        String errorMessage;
        if (request->hasParam("login_failed")) {
            errorMessage = "Usuário ou senha incorretos. Tente novamente.";
        } else {
            errorMessage = "";
        }

        // Substitui o placeholder com a mensagem de erro se houver
        html.replace("%ERROR_MESSAGE%", errorMessage);
        request->send(200, "text/html", html);
    });
}
