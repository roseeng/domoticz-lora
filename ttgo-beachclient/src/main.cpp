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

/*
Gamla värden?

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
*/
#define BAND  868E6
/*
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
*/

#define OLED_SDA MY_DISPLAY_SDA
#define OLED_SCL MY_DISPLAY_SCL

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
  Serial.println("onReceive");

  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); 
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  refreshScreen();
}

void setup() {
  //pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、
  
  Serial.begin(115200);
  while (!Serial);
  delay(1500);

  Serial.println();
  Serial.println("Domoticz-LoRa Beach node ver 0.1");
  
//  pinMode(DI0, INPUT);
//  SPI.begin(SCK, MISO, MOSI, SS);
//  LoRa.setPins(SS, RST, DI0);   
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
//  LoRa.onReceive(onReceive);
//  LoRa.receive();
  
  Serial.println("init ok - listening...");
}

void loop() {
  // For polling:
  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    onReceive(packetSize);  
  }
  delay(10);
  
}
