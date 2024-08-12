#include <ESPAsyncWebServer.h>
#include "localizacaointerface.h"
#include "localizacaogerenciador.h"

// HTML da página de localização como uma string C++
String localizacaoHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Definir Localização</title>
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
        .localizacao-container {
            background-color: #fff;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 5px rgba(0, 0, 0, 0.1);
            text-align: center;
            width: 90%;
            max-width: 500px;
        }
        .input-field {
            margin: 10px 0;
            padding: 10px;
            width: 100%;
            box-sizing: border-box;
        }
        .btn {
            display: block;
            padding: 10px;
            margin: 10px auto;
            font-size: 14px;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            width: 100%;
            max-width: 200px;
            color: #fff;
            background-color: #007bff;
        }
        .suggestions {
            list-style: none;
            padding: 0;
            margin: 10px 0 0;
        }
        .suggestions li {
            padding: 5px;
            cursor: pointer;
        }
        .suggestions li:hover {
            background-color: #f1f1f1;
        }
    </style>
</head>
<body>
    <div class="localizacao-container">
        <h2>Definir Localização</h2>
        <form id="localizacao-form">
            <input type="text" id="user-location" class="input-field" placeholder="Digite sua localização" required>
            <ul id="suggestions" class="suggestions"></ul>
            <button type="button" class="btn" onclick="submitLocation()">Definir Localização</button>
        </form>
        <p id="result"></p>
        <script>
            function submitLocation() {
                var location = document.getElementById('user-location').value;
                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/location?query=' + encodeURIComponent(location), true);
                xhr.onload = function() {
                    if (xhr.status === 200) {
                        document.getElementById('result').innerHTML = 'Localização definida com sucesso!';
                    } else {
                        document.getElementById('result').innerHTML = 'Erro ao definir localização.';
                    }
                };
                xhr.send();
            }

            document.getElementById('user-location').addEventListener('input', function() {
                var query = this.value;
                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/autocomplete?query=' + encodeURIComponent(query), true);
                xhr.onload = function() {
                    if (xhr.status === 200) {
                        var suggestions = JSON.parse(xhr.responseText).suggestions;
                        var suggestionsList = document.getElementById('suggestions');
                        suggestionsList.innerHTML = '';
                        suggestions.forEach(function(suggestion) {
                            var li = document.createElement('li');
                            li.textContent = suggestion;
                            li.onclick = function() {
                                document.getElementById('user-location').value = suggestion;
                                suggestionsList.innerHTML = '';
                            };
                            suggestionsList.appendChild(li);
                        });
                    }
                };
                xhr.send();
            });
        </script>
    </div>
</body>
</html>
)rawliteral";

// Função para configurar a página e rotas
void setupLocalizacaoPage(AsyncWebServer &server) {
    server.on("/localizacao", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", localizacaoHtml);
    });

    server.on("/location", HTTP_GET, [](AsyncWebServerRequest *request){
        String location = request->getParam("query")->value();
        handleLocationRequest(location, request);
    });

    server.on("/autocomplete", HTTP_GET, [](AsyncWebServerRequest *request){
        String query = request->getParam("query")->value();
        handleAutocompleteRequest(query, request);
    });
}
