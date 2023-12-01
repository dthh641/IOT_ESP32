#include <Arduino.h>
#include <iostream>
#include <charconv>
#include <WiFi.h>
#include <MQTT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <GravityTDS.h>

#define temperature_topic "sensor/temperature"
#define tds_topic "sensor/tds"
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

#define TdsSensorPin 34
GravityTDS gravityTds;
float tdsValue = 0;


const char ssid[] = "Giang";
const char pass[] = "doanthihonghanh2001";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/hello");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}
float Read_DS18B20(){
  
  // Serial.print(" Requesting temperatures..."); 
  // sensors.requestTemperatures();
  // Serial.println("DONE"); 
  // Serial.print("Temperature is: "); 
  // Serial.print(sensors.getTempCByIndex(0));
  //delay(1000);

  sensors.requestTemperatures();
  float value_DS18B20 = sensors.getTempCByIndex(0);
  delay(3000);
  return value_DS18B20;
}

void setup_GravityTDS(){
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization
}

float Read_TDS(){
  float temperature = Read_DS18B20();
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate 
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print(tdsValue,0);
  Serial.println("ppm");
  delay(1000);
  return tdsValue;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("192.168.1.123", net);
  client.onMessage(messageReceived);

  connect();
  sensors.begin();
  setup_GravityTDS();
  
}

void loop() {
  float temperature = Read_DS18B20();
  float tds = Read_TDS();
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    //client.publish("/hello", "word");
    Serial.println(String(temperature).c_str());
    client.publish(temperature_topic,String(temperature).c_str(),5);
    client.publish(tds_topic,String(tds).c_str(),5);
  }


}


