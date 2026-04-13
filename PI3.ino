/*
 * Projeto Integrador III – UNIVESP
 * Monitoramento de temperatura de freezers comerciais com IoT
 * 
 * Grupo:
 * - Marcela Rodrigues Siqueira Sturaro (2004774)
 * - Luiz Fernando Couto Correa
 * - Marcel Gomes
 * 
 * Data: 12/04/2026
 */

#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"


// -------------------------------------------------------------
// Sensor DS18B20 no pino GPIO4
// -------------------------------------------------------------
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient client;

// -------------------------------------------------------------
// Controle de tempo (envio a cada 60 segundos)
// -------------------------------------------------------------
unsigned long ultimoEnvio = 0;
const long intervaloEnvio = 60000;

// -------------------------------------------------------------
// Configuração inicial
// -------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Sistema de monitoramento de freezer iniciado.");

  sensors.begin();
  if (sensors.getDeviceCount() == 0) {
    Serial.println("ERRO: Nenhum sensor DS18B20 encontrado. Verifique as conexões.");
  } else {
    Serial.print("Sensor DS18B20 detectado. Dispositivos conectados: ");
    Serial.println(sensors.getDeviceCount());
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 40) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha na conexão Wi-Fi. Verifique as credenciais.");
  }
}

// -------------------------------------------------------------
// Loop principal
// -------------------------------------------------------------
void loop() {
  // Reconecta Wi-Fi se cair
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi perdido. Tentando reconexão...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  unsigned long agora = millis();
  if (agora - ultimoEnvio >= intervaloEnvio) {
    ultimoEnvio = agora;

    sensors.requestTemperatures();
    float temperatura = sensors.getTempCByIndex(0);

    if (temperatura != DEVICE_DISCONNECTED_C) {
      Serial.print("Temperatura lida: ");
      Serial.print(temperatura);
      Serial.println(" °C");

      if (client.connect(server, 80)) {
        String url = "/update?api_key=" + writeAPIKey + "&field1=" + String(temperatura);
        client.print("GET " + url + " HTTP/1.1\r\n");
        client.print("Host: " + String(server) + "\r\n");
        client.print("Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println("Timeout na resposta do ThingSpeak.");
            client.stop();
            return;
          }
        }

        String resposta = client.readString();
        int idx = resposta.lastIndexOf('\n');
        String entryID = resposta.substring(idx + 1);
        entryID.trim();

        if (entryID.toInt() > 0) {
          Serial.print("Dados enviados com sucesso. Entry ID: ");
          Serial.println(entryID);
        } else {
          Serial.println("Erro: ThingSpeak rejeitou os dados (entry ID = 0).");
        }
        client.stop();
      } else {
        Serial.println("Falha na conexão com o ThingSpeak.");
      }
    } else {
      Serial.println("Erro na leitura do sensor DS18B20.");
    }
  }

  delay(100);
}
