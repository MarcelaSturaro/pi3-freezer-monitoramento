/*
*Projeto Integrador III - UNIVESP.
*Monitoramnento de temperatura de freezers comerciais com IoT.
*Grupo:
* - Marcela Rodrigues Siqueira Sturaro (2004774)
* - Luiz Fernando Couto Correa
* - Marcel Gomes
*
* Data: 12/04/2026.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

//Sensor DS18B20 no pino 4
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Controle de tempo (envio a cada 60 segundos)
unsigned long ultimoEnvio = 0;
const long intervaloEnvio = 60000;

//Função para conectar MQTT
void conectarMQTT() {
  espClient.setInsecure();
  mqttClient.setServer(mqtt_server, mqtt_port);
  while (!mqttClient.connected()) {
    String clientId = "ESP32-Freezer-" + WiFi.macAddress();
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("MQTT conectado");
    } else {
      Serial.print("Falha MQTT, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando novamnete em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Sistema de monitoramento de freezer iniciado");


  sensors.begin();
  if (sensors.getDeviceCount() == 0) {
    Serial.println("Erro: Nenhum sensor DS18B20 encontrado. Verifique as conexões.");
  } else {
    Serial.print("Sensor DS18B20 detectado. Dispositivos conectados: ");
    Serial.println(sensors.getDeviceCount());

  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  int tentativas = 0;
  while (WiFi.status() !=WL_CONNECTED&& tentativas <40) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  if (WiFi.status() ==WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha na conexão Wi-Fi. Verifique as credenciais.");
    return;
  }

  conectarMQTT();
}

//Loop principal

void loop() {
  //Reconecta o Wi-fi se cair
  if (WiFi.status() !=WL_CONNECTED) {
    Serial.println("Wi-Fi perdido. Tentando reconexão...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  // Verifica e mantém conexão MQTT
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

//Envio periódico
unsigned long agora = millis();
if (agora - ultimoEnvio >= intervaloEnvio) {
  ultimoEnvio = agora;

  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  if (temperatura != DEVICE_DISCONNECTED_C) {
    Serial.print("Temperatura lida: ");
    Serial.print(temperatura);
    Serial.println(" C°");

    //Publica via MQTT
    char payload[10];
    dtostrf(temperatura, 4, 2, payload);
    if (mqttClient.publish(mqtt_topic, payload)) {
      Serial.print("Publicado via MQTT: ");
      Serial.println(payload);
    } else {
      Serial.println("Falha na publicação MQTT");
    }
  } else {
    Serial.println("Erro na leitura do sensor DS182B20.");
  }
}

delay(100);
}
