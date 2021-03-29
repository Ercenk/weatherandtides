#include "definitions.h"
#include "smartConfig.h"

// constructor for AVR Arduino, copy from GxEPD_Example else
GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8 * 60 * 60;
const int daylightOffset_sec = 0;

int16_t textYStart = 15;
int16_t textY = 0;
int16_t lineHeight = 15;

void displayText(const String &str, int16_t y, uint8_t alignment, const GFXfont *f)
{
  int16_t x = 0;
  int16_t x1, y1;
  uint16_t w, h;

  display.setFont(f);
  display.setCursor(x, y);
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h);

  switch (alignment)
  {
  case RIGHT_ALIGNMENT:
    display.setCursor(display.width() - w - x1, y);
    break;
  case LEFT_ALIGNMENT:
    display.setCursor(0, y);
    break;
  case CENTER_ALIGNMENT:
    display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
    break;
  default:
    break;
  }
  display.println(str);
}

void initializeDisplay()
{
  static bool isInit = false;
  if (isInit)
  {
    return;
  }
  isInit = true;
  display.init();
  display.setRotation(0);
  display.eraseDisplay();
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(0);
}

void displayTides(String todayString)
{
  String noaaUrl = "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=predictions&application=Custom&datum=MLLW&station=9445326&time_zone=lst_ldt&units=english&interval=hilo&format=json&begin_date=";
  noaaUrl += todayString;
  noaaUrl += "&end_date=";
  noaaUrl += todayString;

  Serial.println(noaaUrl);

  HTTPClient http;

  http.begin(noaaUrl.c_str());

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  Serial.print("Status: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);

    const int capacity = JSON_OBJECT_SIZE(1) +
                         JSON_ARRAY_SIZE(4) +
                         4 * JSON_OBJECT_SIZE(3) +
                         3 * JSON_OBJECT_SIZE(1) + 123;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }

    int len = doc["predictions"].size();
    Serial.print("Len: ");
    Serial.println(len);

    textY = textYStart + sectionOffset;

    for (int i = 0; i < len; i++)
    {
      textY += lineHeight;

      String message = doc["predictions"][i]["type"].as<String>();
      message += ": ";
      String timeString = doc["predictions"][i]["t"].as<String>();
      timeString = timeString.substring(11, 16);
      message += timeString;
      displayText(message, textY, LEFT_ALIGNMENT, &DEFAULT_FONT);
      Serial.println(message);

      String heightString = message = doc["predictions"][i]["v"].as<String>();
      displayText(heightString, textY, RIGHT_ALIGNMENT, &DEFAULT_FONT);
      Serial.println(timeString);
    }
  }
}

void displayWeather()
{
  String weatherUrl = "https://api.openweathermap.org/data/2.5/onecall?lat=47.601420&lon=-122.989437&exclude=minutely,daily,alerts&appid=7b52d11e8f0e1238ff68a06d5cc4cd9f&units=imperial";

  HTTPClient http;

  http.begin(weatherUrl.c_str());

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  Serial.print("Status: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);

    const size_t capacity =
        49 * JSON_ARRAY_SIZE(1) +
        JSON_ARRAY_SIZE(48) +
        7 * JSON_OBJECT_SIZE(1) +
        49 * JSON_OBJECT_SIZE(4) +
        JSON_OBJECT_SIZE(6) +
        41 * JSON_OBJECT_SIZE(13) +
        7 * JSON_OBJECT_SIZE(14) +
        JSON_OBJECT_SIZE(15) +
        1600;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    int timezone_offset = doc["timezone_offset"]; // -28800

    JsonObject current = doc["current"];
    long current_dt = current["dt"];                  // 1607200805
    long current_sunrise = current["sunrise"];        // 1607183089
    long current_sunset = current["sunset"];          // 1607214076
    float current_temp = current["temp"];             // 54.34
    float current_feels_like = current["feels_like"]; // 51.87
    int current_pressure = current["pressure"];       // 1016
    int current_humidity = current["humidity"];       // 62
    float current_dew_point = current["dew_point"];   // 41.61
    float current_uvi = current["uvi"];               // 0.86
    int current_clouds = current["clouds"];           // 11
    int current_visibility = current["visibility"];   // 10000
    float current_wind_speed = current["wind_speed"]; // 1.01
    int current_wind_deg = current["wind_deg"];       // 315
    float current_wind_gust = current["wind_gust"];   // 5.99

    JsonObject current_weather = current["weather"][0];
    int current_weather_id = current_weather["id"];                           // 801
    const char *current_weather_main = current_weather["main"];               // "Clouds"
    const char *current_weather_description = current_weather["description"]; // "few clouds"
    const char *current_weather_icon = current_weather["icon"];

    int8_t changes[maxChanges];
    int currentChange = 0;
    JsonArray hourly = doc["hourly"];

    int changedDescription = -1;
    bool record = false;

    for (int8_t i = 0; i < hourly.size(); i++)
    {
      JsonObject hour = hourly[i];
      long hour_dt = hour["dt"];                  // 1607198400
      float hour_temp = hour["temp"];             // 54.34
      float hour_feels_like = hour["feels_like"]; // 49.57
      int hour_pressure = hour["pressure"];       // 1016
      int hour_humidity = hour["humidity"];       // 62
      float hour_dew_point = hour["dew_point"];   // 41.61
      float hour_uvi = hour["uvi"];               // 0.93
      int hour_clouds = hour["clouds"];           // 11
      int hour_visibility = hour["visibility"];   // 10000
      float hour_wind_speed = hour["wind_speed"]; // 5.08
      int hour_wind_deg = hour["wind_deg"];       // 189

      JsonObject hour_weather = hour["weather"][0];
      int hour_weather_id = hour_weather["id"];                           // 801
      const char *hour_weather_main = hour_weather["main"];               // "Clouds"
      const char *hour_weather_description = hour_weather["description"]; // "few clouds"
      const char *hour_weather_icon = hour_weather["icon"];               // "02d"

      int hour_pop = hour["pop"]; // 0

      Serial.printf("Hour: %s\n", hour_weather_description);

      if (changedDescription < 0)
      {
        if (strcmp(current_weather_description, hour_weather_description) != 0 && currentChange < maxChanges)
        {
          record = true;
          changedDescription = i;
        }
      }
      else
      {
        if (strcmp(hourly[changedDescription]["weather"][0]["description"], hour_weather_description) != 0 && currentChange < maxChanges)
        {
          record = true;
          changedDescription = i;
        }
      }
      if (record)
      {
        record = false;
        changes[currentChange++] = i;
        Serial.printf("Record: %i - %i\n", currentChange, i);
      }
    }

    textY += sectionOffset;

    for (int i = 0; i < maxChanges; i++)
    {
      JsonObject hour = hourly[changes[i]];
      long hour_dt = hour["dt"];
      JsonObject hour_weather = hour["weather"][0];
      DateTime datetime(hour_dt + timezone_offset);
      String displayString = String(datetime.hour());
      displayString += ' ';
      displayString += hour_weather["description"].as<String>().substring(0, 12);
      textY += lineHeight;
      displayText(displayString, textY, LEFT_ALIGNMENT, &DEFAULT_FONT);
      Serial.printf("Change: %i \n", changes[i]);
      Serial.println(displayString);
    }
  }
}

void getPredictions()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // String formattedTime = timeClient.getFormattedTime();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      return;
    }
    char todayString[9] = {'\0'};
    strftime(todayString, 9, "%Y%m%d", &timeinfo);

    char todayStringToDisplay[17] = {'\0'};
    strftime(todayStringToDisplay, 17, "%m/%d %I:%M%p", &timeinfo);
    displayText(String(todayStringToDisplay), textYStart, LEFT_ALIGNMENT, &DEFAULT_FONT);

    displayTides(todayString);

    displayWeather();
  }
}

void setupOTA()
{
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}

void setup()
{
  pinMode(CONFIG_DONE_PIN, OUTPUT);
  digitalWrite(CONFIG_DONE_PIN, LOW);

  Serial.begin(115200);

  initializeDisplay();

  initializeWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  getPredictions();

  display.update();

  Serial.println(WiFi.localIP());
  setupOTA();
}

void loop()
{
  ArduinoOTA.handle();

  digitalWrite(CONFIG_DONE_PIN, HIGH);
  delay(1);
  digitalWrite(CONFIG_DONE_PIN, LOW);
}