#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include "ttgov21new.h"
#include "interval.h"

/*
  Detta är den enhet som ska sitta på bryggan och tända/släcka lampor och mäta vattentemperatur.
  Koden är skriven för en Lilygo TTGO Lora32 V2.1_1.6
*/

#define BAND  868E6

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

String rssi = "RSSI --";
String packet ;

int command = 0; // 1 = Query, 2 = On, 3 = Off
int lastCommand = 0;
char myId = '1';

bool lampOn = false;
unsigned long showLastCommand = 0;

void refreshScreen()
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "-- LoRa Beach node --"); 
  display.drawString(0 , 15 , "Received: "+ packet);

  display.drawString(0, 26, rssi);  

  if (showLastCommand > 0 &&  millis() < showLastCommand + 15000) {
    if (lastCommand == 1)
      display.drawString(0, 50, "Command: Query");
    if (lastCommand == 2)
      display.drawString(0, 50, "Command: On");
    if (lastCommand == 3)
      display.drawString(0, 50, "Command: Off");  
    if (lastCommand == 4)
      display.drawString(0, 50, "Command: Ping");  
  }
  display.display();
}

void onReceive(int packetSize) {  
  packet ="";

  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); 
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;

  if (packet[0] == 'q' && packet[1] == myId) // Query
    command = 1;
  if (packet[0] == 'n' && packet[1] == myId) //oN
    command = 2;  
  if (packet[0] == 'f' && packet[1] == myId) // oFf
    command = 3;  
  if (packet[0] == 'p' && packet[1] == myId) // Ping
    command = 4;  

  if (command > 0) {
    lastCommand = command;
    showLastCommand = millis();
  }
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

// For interrupt-driven:
  LoRa.onReceive(onReceive);
  LoRa.receive();
  
  Serial.println("init ok - listening...");
}

void sendLora(String msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
//    LoRa.print(counter);
    LoRa.endPacket();

    LoRa.receive();
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

  if (command ==  1) // Query
  {
    Serial.println("Command: Query");
    if (lampOn)
      sendLora("R1ON");
    else
      sendLora("R1OFF");

    command = 0;
  }

  if (command ==  2) // On
  {
    digitalWrite(HAS_LED, HIGH);  
    lampOn = true;

    Serial.println("Command: On");
    sendLora("R1ON");

    command = 0;
  }

  if (command ==  3) // Off
  {
    digitalWrite(HAS_LED, LOW); 
    lampOn = false; 

    Serial.println("Command: Off");
    sendLora("R1OFF");

    command = 0;
  }    

  if (command ==  4) // Ping
  {
    Serial.println("Command: Ping");
    sendLora("R1OK");

    command = 0;
  }    

}
