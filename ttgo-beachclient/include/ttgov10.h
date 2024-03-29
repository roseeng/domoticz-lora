// clang-format off
// upload_speed 921600
// board esp32dev

#ifndef _TTGOV10_H
#define _TTGOV10_H

#include <stdint.h>

/* 
// Hardware related definitions for TTGO V1.0 Board
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/assets/image/lora32_v1.0_pinmap.jpg
// https://randomnerdtutorials.com/ttgo-lora32-sx1276-arduino-ide/
// ATTENTION: check your board version!
// These settings are for the first boards with no labelling
*/

#define HAS_LORA         1 // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio 1 // HPD13A LoRa SoC

#define HAS_SDCARD  0      // this board has no SD MMC card-reader/writer
#define SDCARD_SLOTWIDTH 0 // 4-line interface

#define HAS_DISPLAY 1
#define HAS_LED (25) // green on board LED
//#define HAS_LED NOT_A_PIN

#define BAT_MEASURE_ADC ADC1_GPIO35_CHANNEL // battery probe GPIO pin -> ADC1_CHANNEL_7
#define BAT_VOLTAGE_DIVIDER 2 // voltage divider 100k/100k on board

// Pins for I2C interface of OLED Display
#define OLED_SDA (4)
#define OLED_SCL (15)
#define OLED_RST (16)

// Pins for LORA chip SPI interface, reset line and interrupt lines
#define LORA_SCK  (5) 
#define LORA_CS   (18)
#define LORA_MISO (19)
#define LORA_MOSI (27)
#define LORA_RST  (14)
#define LORA_IRQ  (26)
#define LORA_IO1  (?)
#define LORA_IO2  (?)

#endif
