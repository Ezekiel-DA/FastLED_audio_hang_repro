#include <Arduino.h>

#include <SPIFFS.h>
#include <FastLED.h>
#include <Audio.h>

const char* sampleFilename = "/sample.mp3";

QueueHandle_t audioSetQueue = NULL;
Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);

void audioTask(void *parameter) {
    audioSetQueue = xQueueCreate(10, 0);

    while(true){
        if(xQueueReceive(audioSetQueue, nullptr, 1) == pdPASS) {
          audio.connecttoFS(SPIFFS, sampleFilename);
        }
        audio.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,             /* Function to implement the task */
        "audioplay",           /* Name of the task */
        5000,                  /* Stack size in words */
        NULL,                  /* Task input parameter */
        5 | portPRIVILEGE_BIT, /* Priority of the task */
        NULL,                  /* Task handle. */
        1                      /* Core where the task should run */
    );
}

CRGB strip1Leds[3];
CRGB strip2Leds[70];
CRGB strip3Leds[155];
CRGB strip4Leds[120];

void fastLEDShowTask(void* params) {
  static uint32_t loopNum = 0;

  static auto prev = millis();
  while (1) {
    auto now = millis();
    if (now - prev >= 10) {
      FastLED.show();
      prev = now;

      if (loopNum++ % 100 == 0) {
        Serial.print("FastLED Task Loop #"); Serial.println(loopNum);
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  SPIFFS.begin();

  FastLED.addLeds<WS2812B, 12, GRB>(strip1Leds, 3);
  FastLED.addLeds<WS2812B, 13, GRB>(strip2Leds, 70);
  FastLED.addLeds<WS2812B, 15, GRB>(strip3Leds, 155);
  FastLED.addLeds<WS2812B, 16, GRB>(strip4Leds, 120);

  audioInit();

  xTaskCreate(fastLEDShowTask, "fastLED", 10000, nullptr, 3, nullptr);
}

void loop()
{
  static uint32_t loopNum = 0;

  if (loopNum == 300000) {
    xQueueSend(audioSetQueue, nullptr, portMAX_DELAY);
    Serial.println("Music starts; FastLED.show() task will stop responding.");
  }
  
  if (loopNum++ % 50000 == 0) {
    Serial.print("Loop #"); Serial.println(loopNum);
  }
}
