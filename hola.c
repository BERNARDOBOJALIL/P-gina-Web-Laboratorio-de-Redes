#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <cmath> // Incluir esta librería

const char* ssid = "LAPTOP-AJFEJ7IT-6883";
const char* pass = "12345678";
const char* serverUrl = "https://lab-redes-orm.vercel.app/api/sensordata";

const int sensorPin = 34;  // Pin analógico del sensor de sonido
const int ledPin = 25;     // Pin del LED
const int umbral = 2000;   // Umbral para encender el LED

const int dhtPin = 26;     // Pin del sensor DHT
const int dhtType = DHT22; // Tipo de sensor (DHT11 o DHT22)
DHT dht(dhtPin, dhtType);

unsigned long tiempoDHT = 0;
const int intervaloDHT = 2000;

unsigned long tiempoPOST = 0;
const int intervaloPOST = 20000;

float temperatura = NAN, humedad = NAN;

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    dht.begin();
    
    WiFi.begin(ssid, pass);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi");
}

void loop() {
    int valor = analogRead(sensorPin);
    float decibelios = 0.1273 * valor - 218.18;
    if (decibelios <= 0) {
        decibelios = 75;
    }
    decibelios = constrain(decibelios, 0, 75);
    
    Serial.print("Valor del sensor de sonido: ");
    Serial.print(valor);
    Serial.print(" | Decibeles: ");
    Serial.println(decibelios);

    digitalWrite(ledPin, valor > umbral ? HIGH : LOW);

    if (millis() - tiempoDHT >= intervaloDHT) {
        tiempoDHT = millis();
        float nuevaTemp = dht.readTemperature();
        float nuevaHumedad = dht.readHumidity();

        if (!isnan(nuevaTemp) && !isnan(nuevaHumedad)) {
            temperatura = nuevaTemp;
            humedad = nuevaHumedad;
            Serial.print("Temperatura: ");
            Serial.print(temperatura);
            Serial.print(" °C | Humedad: ");
            Serial.print(humedad);
            Serial.println(" %");
        } else {
            Serial.println("Error al leer el sensor DHT");
        }
    }

    if (millis() - tiempoPOST >= intervaloPOST) {
        tiempoPOST = millis();
        if (!isnan(temperatura) && !isnan(humedad)) {
            enviarDatos(decibelios, temperatura, humedad);
        } else {
            Serial.println("Datos no válidos, no se envió el POST");
        }
    }

    delay(100);
}

void enviarDatos(float sonido, float temperatura, float humedad) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        String jsonPayload = "{";
        jsonPayload += "\"sensor_id\":3,";
        jsonPayload += "\"soundValue\":" + String(sonido) + ",";
        jsonPayload += "\"temperatureValue\":" + String(temperatura) + ",";
        jsonPayload += "\"humidityValue\":" + String(humedad) + "}";
        
        int httpResponseCode = http.POST(jsonPayload);
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.println("Payload enviado: " + jsonPayload);
        
        http.end();
    } else {
        Serial.println("Error: No conectado a WiFi");
    }
}