#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include "ttgov21new.h"

/*
  Detta är den enhet som ska sitta kopplad till Domoticz.
  Planen är att skriva en plugin som kommunicerar via usb uart.
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

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;

void setup() {
  //pinMode(OLED_RST, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HAS_LED, OUTPUT);
  
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("Domoticz-LoRa Base ver 0.1");
  
//  SPI.begin(SCK,MISO,MOSI,SS);
//  LoRa.setPins(SS,RST,DI0);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
//  LoRa.receive();
  Serial.println("init ok");
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
   
  delay(1500);  
}

void loop() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  display.drawString(0, 0, "-- LoRa Base node --");
  display.drawString(0, 15, "Sending packet: ");
  display.drawString(90, 15, String(counter));
  Serial.println(String(counter));
  display.display();

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello kotte ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}

