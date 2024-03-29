#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  

// Select TTGO Lora32 v1.0 or v2.1 eller mkrwan1300 (only one)
//#include "ttgov10.h" 
//#include "ttgov21new.h"
#include "mkrwan1300.h"

#include "interval.h"

/*
  Detta är den enhet som ska sitta på bryggan och tända/släcka lampor och mäta vattentemperatur.
  Koden är skriven för en Lilygo TTGO Lora32 V2.1_1.6 men kan konfigureras att funka med den gamla 1.0
*/

#define BAND  868E6

//#define GR_HAVE_DISPLAY
#ifdef GR_HAVE_DISPLAY
SSD1306 display(0x3c, OLED_SDA, OLED_SCL);
#endif

String rssi = "RSSI --";
String packet ;

int command = 0; // 1 = Query, 2 = On, 3 = Off
int lastCommand = 0;
char myId = '1';

bool lampOn = false;
unsigned long showLastCommand = 0;

void refreshScreen()
{
#ifdef GR_HAVE_DISPLAY  
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
#endif
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

bool _HaveSerial = false;

void WaitForSerial(int timeoutSeconds)
{
  Serial.begin(115200);
  
  for (int retries = 0; retries < timeoutSeconds; retries++)
    if (Serial) {
      _HaveSerial = true;
      break;
    } else {
      delay(1000);
    }
}

void setup() {
  if (OLED_RST != NOT_A_PIN)
    pinMode(OLED_RST, OUTPUT);
  
  if (HAS_LED) {
    pinMode(HAS_LED, OUTPUT);
    digitalWrite(HAS_LED, LOW);  
  }

  WaitForSerial(15);

  Serial.println();
  Serial.println("Domoticz-LoRa Beach node ver 0.4");
  
#ifdef GR_HAVE_DISPLAY  
  if (OLED_RST != NOT_A_PIN)
  {
    digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
    delay(50); 
    digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、
  }
#else
  Serial.println("No OLED display.");
#endif

#ifdef LORA_IRQ
  pinMode(LORA_IRQ, INPUT);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
#endif  

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

#ifdef GR_HAVE_DISPLAY
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);  
#endif
 
  delay(1500);

//  LoRa.dumpRegisters(Serial);

#ifndef ARDUINO_SAMD_MKRWAN1300
// For interrupt-driven:
  LoRa.onReceive(onReceive);
  LoRa.receive();
#endif

  Serial.println("init ok - listening...");
}

void sendLora(String msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
//    LoRa.print(counter);
    LoRa.endPacket();

#ifndef ARDUINO_SAMD_MKRWAN1300
    LoRa.receive();
#endif 

}

void loop() {
  // For polling:
#ifdef ARDUINO_SAMD_MKRWAN1300
  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    onReceive(packetSize);  
  }
  delay(10);
#endif

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
