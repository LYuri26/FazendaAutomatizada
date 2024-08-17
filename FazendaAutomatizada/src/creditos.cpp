#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

void setupCreditosPage(AsyncWebServer &server)
{
    const String creditsHtml = R"rawliteral(
        <!DOCTYPE html>
        <html lang="pt-br">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Créditos</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    background-color: #f0f0f0;
                    margin: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }
                .credits-container {
                    background-color: #fff;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 0 8px rgba(0, 0, 0, 0.1);
                    text-align: center;
                    width: 90%;
                    max-width: 400px;
                }
                .credits-title {
                    font-size: 20px;
                    color: #007bff;
                    margin-bottom: 15px;
                }
                .credits-content a {
                    display: block;
                    color: #007bff;
                    text-decoration: none;
                    margin: 5px 0;
                }
                .credits-content a:hover {
                    text-decoration: underline;
                }
                .btn-link-voltar {
                    display: inline-block;
                    background-color: #28a745;
                    color: #fff;
                    border: none;
                    padding: 10px 15px;
                    border-radius: 5px;
                    cursor: pointer;
                    margin-top: 15px;
                }
                .btn-link-voltar:hover {
                    background-color: #218838;
                }
            </style>
        </head>
        <body>
            <div class="credits-container">
                <h2 class="credits-title">Créditos</h2>
                <div class="credits-content">
                    <h3>Alunos</h3>
                    <a href="#" target="_blank">Kaua de Jesus</a>
                    <a href="#" target="_blank">Kayque</a>
                    <a href="#" target="_blank">Hendryw</a>
                    <a href="#" target="_blank">Paulo Antonio</a>
                    <a href="#" target="_blank">Rian</a>
                    <h3>Instrutores</h3>
                    <a href="https://github.com/LYuri26" target="_blank">Lenon Yuri</a>
                    <a href="#" target="_blank">Miguel</a>
                </div>
                <button onclick="window.location.href='/'" class="btn-link-voltar">Voltar</button>
            </div>
        </body>
        </html>
    )rawliteral";

    server.on("/creditos", HTTP_GET, [creditsHtml](AsyncWebServerRequest *request)
              { request->send(200, "text/html", creditsHtml); });
}
