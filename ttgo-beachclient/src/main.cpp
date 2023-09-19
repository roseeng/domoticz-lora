#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include "ttgov21new.h"

/*
  Detta är den enhet som ska sitta på bryggan och tända/släcka lampor och mäta vattentemperatur.
  Koden är skriven för en Lilygo TTGO Lora32 V2.1_1.6
*/

#define BAND  868E6

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

String rssi = "RSSI --";
String packSize = "--";
String packet ;

void refreshScreen(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "-- LoRa Beach node --"); 
  display.drawString(0 , 15 , "Received "+ packSize + " bytes");
  display.drawStringMaxWidth(0 , 26 , 128, packet);
  display.drawString(0, 38, rssi);  
  display.display();
}

void onReceive(int packetSize) {
  digitalWrite(HAS_LED, HIGH);  
  
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); 
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
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
  delay(1500);

  Serial.println();+
  Serial.println("Domoticz-LoRa Beach node ver 0.2");
  
  pinMode(LORA_IRQ, INPUT);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  
  delay(1500);

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "-- LoRa Beach node --"); 
  display.display();

// For interrupt-driven:
  LoRa.onReceive(onReceive);
  LoRa.receive();
  
  Serial.println("init ok - listening...");
}

void loop() {
  // For polling:
/*  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    onReceive(packetSize);  
  }
  delay(10);
  */
  refreshScreen();
  delay(1000);
  digitalWrite(HAS_LED, LOW);  
//  delay(1000);
}
