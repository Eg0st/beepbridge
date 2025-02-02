#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <bmp3.h>
#include <SHTSensor.h>


#define DISABLE_CODE_FOR_RECEIVER
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>





const char ssid[] = "WLAN-2400G";
const char pass[] = "!Gauting-82131-Berg-92";
#define DIGITAL_PIN_TPG2200 1
#define IREMOTE_PIN_SEND 10
#define INTERN_LED 2
#define NUMPIXELS  1
#define I2C_FREQUENCY 40000
#define SDA_PIN 4
#define SCL_PIN 5
Adafruit_NeoPixel pixels(NUMPIXELS, INTERN_LED, NEO_GRB + NEO_KHZ800);
bfs::Bmp3 bmp(&Wire, bfs::Bmp3::I2C_ADDR_SEC);
SHTSensor sht(SHTSensor::SHT3X);

WiFiClient net;
MQTTClient client;


//========================================================================================================//
void connect() {
  Serial.print("[INFO] Checking wifi...");
  digitalWrite(INTERN_LED, HIGH); 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\n[INFO] Connecting...");
  while (!client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\n[INFO] Connected");
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  client.subscribe("d1101/projector_power");
  // client.unsubscribe("/hello");

}

//========================================================================================================//
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  IrSender.sendNEC(0x1308, 0x87, 1);
  pixels.setPixelColor(0, pixels.Color(255, 255, 0));
  pixels.show();

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

//========================================================================================================//
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQUENCY);
  WiFi.begin(ssid, pass);
  pinMode(DIGITAL_PIN_TPG2200, INPUT_PULLUP);
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 0, 125));
  pixels.show(); 

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("192.168.2.2", net);
  client.onMessage(messageReceived);
  connect();

  /* Initialize the BMP-3xy */
  if (!bmp.Begin()) {
    Serial.println("[ERROR] Initializing communication with BMP faild.");
  }

  /* Initialize the SHT */
  if (!sht.init()) {
    Serial.println("[ERROR] Initializing communication with SHT faild.");
  }

  IrSender.begin();
  disableLEDFeedback();
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
    Serial.print(F("Send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

//========================================================================================================//
void loop() {
  unsigned long lastMillis = 0;
  bool send_once = false;
  
  float temperature_c = 0;
  float pressure_ha = 0;
  float humidity_pct = 0;

  while(1)  {
    delay(100);  // <- fixes some issues with WiFi stability
    if (!client.connected()) {
      connect();
    }

    client.loop();
    // publish a message roughly every second.
    if (!digitalRead(DIGITAL_PIN_TPG2200) && !send_once)   {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show(); 
      Serial.print("[INFO] Alarm.");
      if(client.publish("d1101/feuerwehr", "ON")) {
        delay(100);
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
      }
      send_once = true;
    } else if (digitalRead(DIGITAL_PIN_TPG2200) && send_once)  {
      client.publish("d1101/feuerwehr", "OFF");
      Serial.print("[INFO] Alarm end");
      send_once = false;
    }

    //Send sensor value
    if(lastMillis + 1000 <= millis())
    {
      if(bmp.Read())  {
        temperature_c = bmp.die_temp_c();
        pressure_ha = bmp.pres_pa();
        client.publish("d1101/temperature_c",  String(temperature_c));
        client.publish("d1101/pressure_ha",  String(pressure_ha/100.0));
        Serial.printf("[INFO] Temerature: %.2f °C, Pressure: %.2f ha\n", temperature_c, pressure_ha);
      } else  {
        Serial.printf("[ERROR] BMP3 not available.\n");
      }

      if(sht.readSample())  {
        temperature_c = sht.getTemperature();
        humidity_pct = sht.getHumidity();
        client.publish("d1101/temperature_c",  String(temperature_c));
        client.publish("d1101/humidity_pct",  String(humidity_pct));
        Serial.printf("[INFO] Temerature: %.2f °C, Humidity: %.2f %%\n", temperature_c, humidity_pct);
      } else  {
        Serial.printf("[ERROR] SHT not available.\n");
      }
    }


    if(lastMillis + 1000 <= millis())
    {
      client.publish("d1101/heartbeet", "ON");
      lastMillis = millis();
    }
  }
}
