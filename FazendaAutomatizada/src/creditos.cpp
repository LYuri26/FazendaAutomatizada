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
                    padding: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }

                .credits-container {
                    background-color: #ffffff;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                    text-align: center;
                    width: 90%;
                    max-width: 500px;
                }

                .credits-title {
                    font-size: 24px;
                    color: #007bff;
                    margin-bottom: 20px;
                }

                .credits-content {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                }

                .credits-content .category {
                    margin-bottom: 20px;
                }

                .credits-content h3 {
                    margin: 15px 0;
                    color: #333;
                }

                .credits-content ul {
                    list-style-type: none;
                    padding: 0;
                    display: flex;
                    flex-wrap: wrap;
                    justify-content: center;
                }

                .credits-content ul li {
                    margin: 10px;
                }

                .credits-content li a {
                    display: inline-block;
                    background-color: #007bff;
                    color: #ffffff;
                    text-decoration: none;
                    padding: 12px 20px;
                    border-radius: 5px;
                    font-size: 16px;
                    transition: background-color 0.3s, transform 0.3s;
                }

                .credits-content li a:hover {
                    background-color: #0056b3;
                    transform: scale(1.05);
                }

                .btn-link-voltar {
                    background-color: #28a745;
                    border: none;
                    color: #ffffff;
                    padding: 12px 25px;
                    font-size: 16px;
                    cursor: pointer;
                    border-radius: 5px;
                    margin-top: 20px;
                    transition: background-color 0.3s;
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
                    <div class="category">
                        <h3>Alunos</h3>
                        <ul>
                            <li><a href="#" target="_blank">Kaua de Jesus</a></li>
                            <li><a href="#" target="_blank">Kayque</a></li>
                            <li><a href="#" target="_blank">Hendryw</a></li>
                            <li><a href="#" target="_blank">Paulo Antonio</a></li>
                            <li><a href="#" target="_blank">Rian</a></li>
                        </ul>
                    </div>
                    <div class="category">
                        <h3>Instrutores</h3>
                        <ul>
                            <li><a href="https://github.com/LYuri26" target="_blank">Lenon Yuri</a></li>
                            <li><a href="#" target="_blank">Miguel</a></li>
                        </ul>
                    </div>
                </div>
                <button onclick="window.location.href='/'" class="btn-link-voltar">Voltar</button>
            </div>
        </body>
        </html>
    )rawliteral";

    server.on("/creditos", HTTP_GET, [creditsHtml](AsyncWebServerRequest *request)
              { request->send(200, "text/html", creditsHtml); });
}
