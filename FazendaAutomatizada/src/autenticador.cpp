#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// Variáveis de sessão
String sessionId = "";
unsigned long sessionStartTime = 0;
const unsigned long sessionTimeout = 5 * 60 * 1000; // 5 minutos

// Função para verificar a autenticação do usuário
bool isAuthenticated(AsyncWebServerRequest *request)
{
    if (request->hasHeader("Cookie"))
    {
        String cookie = request->header("Cookie");
        int sessionIndex = cookie.indexOf("session_id=");
        if (sessionIndex != -1)
        {
            String sessionValue = cookie.substring(sessionIndex + 11);
            if (sessionValue.equals(sessionId))
            {
                if (millis() - sessionStartTime < sessionTimeout)
                {
                    Serial.println("Sessão autenticada com sucesso.");
                    return true;
                }
                else
                {
                    Serial.println("Sessão expirada.");
                    sessionId = "";
                }
            }
            else
            {
                Serial.println("ID de sessão inválido.");
            }
        }
        else
        {
            Serial.println("Cabeçalho de cookie não contém 'session_id'.");
        }
    }
    else
    {
        Serial.println("Nenhum cookie encontrado.");
    }
    return false;
}

// Função para lidar com o login
void handleLogin(AsyncWebServerRequest *request)
{
    if (request->hasParam("username", true) && request->hasParam("password", true))
    {
        String username = request->getParam("username", true)->value();
        String password = request->getParam("password", true)->value();

        Serial.print("Tentativa de login com usuário: ");
        Serial.println(username);

        if (username == "admin" && password == "admin123")
        {
            sessionId = String(millis(), HEX);
            sessionStartTime = millis();
            Serial.println("Login bem-sucedido.");
            AsyncWebServerResponse *response = request->beginResponse(302, "text/html", "");
            response->addHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
            response->addHeader("Location", "/dashboard");
            request->send(response);
        }
        else
        {
            Serial.println("Login falhou: usuário ou senha incorretos.");
            request->redirect("/?login_failed=true");
        }
    }
    else
    {
        Serial.println("Parâmetros de login ausentes.");
        request->redirect("/?login_failed=true");
    }
}

// Função para lidar com o logout
void handleLogout(AsyncWebServerRequest *request)
{
    if (isAuthenticated(request))
    {
        Serial.println("Logout bem-sucedido.");
        sessionId = "";
        AsyncWebServerResponse *response = request->beginResponse(302, "text/html", "");
        response->addHeader("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/; HttpOnly");
        response->addHeader("Location", "/");
        request->send(response);
    }
    else
    {
        Serial.println("Tentativa de logout sem autenticação.");
        request->redirect("/");
    }
}

// Função para lidar com o acesso ao Dashboard
void handleDashboard(AsyncWebServerRequest *request)
{
    if (isAuthenticated(request))
    {
        Serial.println("Acesso ao Dashboard concedido.");
        request->send(200, "text/html", "Bem-vindo ao Dashboard!");
    }
    else
    {
        Serial.println("Acesso ao Dashboard negado: sessão não autenticada.");
        request->redirect("/");
    }
}
