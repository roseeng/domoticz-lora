#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <WiFi.h>
#include "SSD1306.h" 
#include "ttgov21new.h"

/*
  Detta är den enhet som ska sitta kopplad till Domoticz.
  Planen är att skriva en plugin som kommunicerar via usb uart.
  Koden är skriven för en Lilygo TTGO Lora32 V2.1_1.6
*/

#define BAND  868E6

const char *ssid       = "";
const char *password   = "";

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

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

    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
       
  delay(1500);  

  // For interrupt-driven:
  LoRa.onReceive(onReceive);
  LoRa.receive();
  
  Serial.println("listening...");
}

const byte numChars = 32;
char receivedChars[numChars];

boolean dataToSend = false;

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && dataToSend == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                dataToSend = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewCommand() {
    if (dataToSend == true) {
        Serial.print("Received via serial: ");
        Serial.println(receivedChars);
    }
}

void updateDisplay(String header)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, header);
  display.drawString(0, 15, "Sending packet: ");
  display.drawString(90, 15, receivedChars);
  display.drawString(0, 26, rssi);   
  display.drawString(0, 38, "Received: ");
  display.drawString(90, 38, packet);

  display.display();  
}

void sendLora(String msg)
{
    // send packet
    LoRa.beginPacket();
    LoRa.print(msg);
//    LoRa.print(counter);
    LoRa.endPacket();

    LoRa.receive();
}

void loop() {

  recvWithStartEndMarkers();

  updateDisplay("-- LoRa Base node --");

  if (dataToSend) {
    showNewCommand();
    sendLora(receivedChars);
    dataToSend = false;

    counter++;
  }

  if (havePacket)
  {
    Serial.print("Received: ");
    Serial.println(packet);

    havePacket = false;
    digitalWrite(HAS_LED, LOW);  
  }

   delay(1000);
}

