const express = require('express');
const cors = require('cors');
const mqtt = require('mqtt');
const { InfluxDB, Point } = require('@influxdata/influxdb-client');
require('dotenv').config();

const app = express();
app.use(cors());
app.use(express.json());

//Detecta se estamos em ambiente de teste 
const modoTeste = process.env.NODE_ENV ==='test'


// ============================
// 1. InfluxDB - write e query
// ============================
let writeApi, queryApi;
if (!modoTeste) {
  const influxDB = new InfluxDB({ url: process.env.INFLUX_URL, token: process.env.INFLUX_TOKEN });
  writeApi = influxDB.getWriteApi(process.env.INFLUX_ORG, process.env.INFLUX_BUCKET, 'ns');
  queryApi = influxDB.getQueryApi(process.env.INFLUX_ORG);
  writeApi.useDefaultTags({ device: 'freezer' });
}

// ============================
// 2. MQTT Client
// ============================
let mqttClient;
if (!modoTeste) {
  mqttClient = mqtt.connect(`mqtts://${process.env.MQTT_BROKER}`, {
    port: Number(process.env.MQTT_PORT),
    username: process.env.MQTT_USER,
    password: process.env.MQTT_PASS,
  });

  mqttClient.on('connect', () => {
    console.log('Conectado ao broker MQTT');
    mqttClient.subscribe(process.env.MQTT_TOPIC, (err) => {
        if (!err) console.log(`Inscrito no tópico ${process.env.MQTT_TOPIC}`);
        else console.error('Erro na inscrição:', err);
    });
  });


  mqttClient.on('message', async (topic, message) => {
    const temperatura = parseFloat(message.toString());
    if (isNaN(temperatura)) return;

    console.log(`Recebido: ${temperatura}°C`);

    const point = new Point('temperatura')
        .floatField('value', temperatura)
        .timestamp(new Date());

    try {
        await writeApi.writePoint(point);
        await writeApi.flush();
        console.log(`Salvo no InfluxDB: ${temperatura}°C`);
    } catch (err) {
        console.error('Erro ao salvar no InfluxDB:', err.message);
    }
  });
}

// ============================
// 3. API REST
// ============================

app.get('/api/temperatura', async (req, res) => {
  if (modoTeste) {
    return res.json([]);
  }

  const fluxQuery = `
    from(bucket: "${process.env.INFLUX_BUCKET}")
        |> range(start: -24h)
        |> filter(fn: (r) => r._measurement == "temperatura" and r._field == "value")
        |> sort(columns: ["_time"], desc: false)
  `;
  try {
    const results = [];
    for await (const { values, tableMeta } of queryApi.iterateRows(fluxQuery)) {
      results.push(tableMeta.toObject(values));
    }
    res.json(results);
  } catch (err) {
    console.error('Erro na query:', err);
    res.status(500).json({ error: 'Erro ao buscar dados' });
  }
});

// ============================
// 4. Exportar app para testes
// ============================
module.exports = app;


// ============================
// 5. Iniciar servidor (se executado diretamente)
// ============================
if (require.main === module) {
    const PORT = process.env.PORT || 3001;
    app.listen(PORT, () => {
        console.log(`Backend rodando em http://localhost:${PORT}`);
    });
}