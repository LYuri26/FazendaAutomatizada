#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "localizacaoInterface.h"
#include "autenticador.h"
#include "localizacaogerenciador.h"

String getLocalizacaoPage()
{
    return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Definir Localização</title>
    <style>
        body { font-family: Arial, sans-serif; background: #f8f9fa; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .container { background: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 0 5px rgba(0,0,0,0.1); text-align: center; width: 90%; max-width: 400px; }
        input[type="text"], .btn { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }
        .btn { background: #28a745; color: #fff; border: none; cursor: pointer; }
        #resultado { margin-top: 20px; }
        input[type="checkbox"] { margin-right: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Definir Localização</h2>
        <label><input type="checkbox" id="definirHorario" onclick="toggleForm()"> Definir horário automático</label>
        <form id="localizacaoForm" action="/salvar-localizacao" method="POST" style="display:none;">
            <input type="text" name="cidade" placeholder="Cidade - Estado (Ex: São Paulo - SP)" required>
            <button type="submit" class="btn">Obter Informações</button>
        </form>
        <div id="resultado"></div>
    </div>
    <script>
        function toggleForm() {
            document.getElementById('localizacaoForm').style.display = document.getElementById('definirHorario').checked ? 'block' : 'none';
        }
        
        document.getElementById('localizacaoForm').addEventListener('submit', function(e) {
            e.preventDefault();
            fetch('/salvar-localizacao', { method: 'POST', body: new FormData(this) })
            .then(response => response.json())
            .then(data => {
                const resultado = document.getElementById('resultado');
                resultado.innerHTML = data.success ? `<p><strong>Nascer do Sol:</strong> ${data.sunrise}</p><p><strong>Pôr do Sol:</strong> ${data.sunset}</p>` : 'Erro ao obter informações';
            })
            .catch(() => document.getElementById('resultado').textContent = 'Erro ao conectar');
        });
    </script>
</body>
</html>
)rawliteral";
}

void setupLocalizacaoPage(AsyncWebServer &server)
{
    server.on("/localizacao", HTTP_GET, [](AsyncWebServerRequest *request)
              { isAuthenticated(request) ? request->send_P(200, "text/html", getLocalizacaoPage().c_str()) : request->redirect("/acesso-invalido"); });

    server.on("/salvar-localizacao", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        String cidade;
        if (request->hasParam("cidade", HTTP_POST))
        {
            cidade = request->getParam("cidade", HTTP_POST)->value();
            
            if (LittleFS.begin())
            {
                File file = LittleFS.open("/localizacao.txt", "w");
                if (file)
                {
                    file.print("Cidade: ");
                    file.println(cidade);
                    file.print("Nascer do Sol: ");
                    file.println("Não definido");
                    file.print("Pôr do Sol: ");
                    file.println("Não definido");
                    file.close();
                }
                LittleFS.end();
            }

            updateSunriseSunsetTimes();

            request->send(200, "application/json", "{\"success\":true, \"sunrise\":\"Não definido\", \"sunset\":\"Não definido\"}");
        }
        else
        {
            request->send(400, "application/json", "{\"success\":false, \"message\":\"Cidade não fornecida\"}");
        } });
}
