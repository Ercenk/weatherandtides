#include <Arduino.h>
#include <GxEPD.h>

#include <GxGDE0213B72B/GxGDE0213B72B.h> 
#include <Wire.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>  
#include <esp_wifi.h>

#include <time.h>
#include <RTClib.h>

#include <ArduinoJson.h>

#include <ArduinoOTA.h>

#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17

typedef enum {
    RIGHT_ALIGNMENT = 0,
    LEFT_ALIGNMENT,
    CENTER_ALIGNMENT,
} Text_alignment;

#define DEFAULT_FONT FreeSerif9pt7b

#define maxChanges 10

#define sectionOffset 8