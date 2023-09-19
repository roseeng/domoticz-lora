#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#include "SSD1306.h" 
#include "ttgov21new.h"
#include "site.h"
#include "interval.h"

/*
  Detta är den enhet som ska sitta kopplad till Domoticz.
  Planen är att skriva en plugin som kommunicerar via usb uart.
  2023-09-19: Lappkast, vi kan anropa Domoticz api istället.
  Koden är skriven för en Lilygo TTGO Lora32 V2.1_1.6
*/

#define BAND  868E6

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

int switchStatus = -1; // -1 = unknown, 0 = off, 1 = on

WiFiClient wifiClient;
HttpClient* client;
int wifiSite;

unsigned int counter = 0;
String rssi = "RSSI --";
String packet;
bool havePacket = false;

void onReceive(int packetSize) {
  digitalWrite(HAS_LED, HIGH);  
  
  packet = "";
  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); 
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC);

  havePacket = true;
}

bool ConnectToWifi(String ssid, String password)
{
    Serial.printf("Connecting to %s... ", ssid);
    WiFi.begin(ssid, password);
    int retry = 20;
    while (WiFi.status() != WL_CONNECTED && retry-- > 0) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED)
      return false;
    
    Serial.print("  CONNECTED to ");
    Serial.println(ssid);
    Serial.print("  IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void setup() {
  pinMode(HAS_LED, OUTPUT);
  digitalWrite(HAS_LED, LOW);  
  
  if (OLED_RST != NOT_A_PIN)
  {
    digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
    delay(50); 
    digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、
  }
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("Domoticz-LoRa Base ver 0.2");
  
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("init ok");
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);

  if (ConnectToWifi(sites[0].ssid, sites[0].password)) {
    wifiSite = 0;
  }
  else if (ConnectToWifi(sites[1].ssid, sites[2].password)) {
    wifiSite = 1;
  }
  else {
    Serial.println("Failed to connect to wifi!");
  }
  client = new HttpClient(wifiClient, sites[wifiSite].host);

  delay(1500);  

  LoRa.onReceive(onReceive);
  LoRa.receive();
  
  Serial.println("listening...");
}

int pollDomoticz()
{
  Serial.println("Calling Domoticz");
  char switchUrl[256];
  
  sprintf(switchUrl, "/json.htm?type=command&param=getdevices&rid=%d", sites[wifiSite].switchIdx);

  client->beginRequest();
  client->get(switchUrl);
  client->sendHeader("Content-Type", "application/json");
  // client.sendBasicAuth("username", "password");
  client->endRequest();

  int statusCode = client->responseStatusCode();
  String responseBody = client->responseBody();
  Serial.print("Response status: ");
  Serial.println(statusCode);

  if (statusCode != 200) {
    Serial.println("Response:\n" + responseBody);
    return -1;
  }

  DynamicJsonDocument doc(3072);

  DeserializationError error = deserializeJson(doc, responseBody);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return -1;
  }

  JsonObject result_0 = doc["result"][0];
  const char* status = result_0["Data"];

  Serial.print("Switchen är: ");
  Serial.println(status);

  return status[1] == 'f' ? 0 : 1; 
}

void updateDisplay(String header)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, header);
  display.drawString(0, 15, "Switch status: ");
  display.drawString(90, 15, String(switchStatus, DEC));
  display.drawString(0, 26, rssi);   
  display.drawString(0, 38, "Received: ");
  display.drawString(90, 38, packet);

  display.display();  
}

void sendLora(String msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
//    LoRa.print(counter);
    LoRa.endPacket();

    LoRa.receive();
}

Interval switchInterval;

void loop() {
  
  if (switchInterval.Every(30)) {
    int newStatus = pollDomoticz();  
  
    if (switchStatus != newStatus) {
      if (newStatus == 0)
        sendLora("f1");
      else if (newStatus == 1)
        sendLora("n1");

      switchStatus = newStatus;
      counter++;
    }
  }

  if (havePacket)
  {
    Serial.print("Recieved: ");
    Serial.println(packet);

    havePacket = false;
    digitalWrite(HAS_LED, LOW);  
  }

  updateDisplay("-- LoRa Base node --");

  delay(1000);
}

