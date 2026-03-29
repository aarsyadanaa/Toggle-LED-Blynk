#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID " "
#define BLYNK_TEMPLATE_NAME " "
#define BLYNK_AUTH_TOKEN " "

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define LED1 4
#define LED2 2
#define BUTTON 5

enum Mode { 
  OFFLINE_MODE, 
  ONLINE_MODE 
};
volatile Mode currentMode = OFFLINE_MODE;

volatile bool buttonPressedFlag = false;
bool led1State = false;
bool led2State = false;

void taskButton(void *pvParameters){
  bool lastState = HIGH;
  bool reading;
  while(1){
    reading = digitalRead(BUTTON);
    if(reading != lastState){
      vTaskDelay(50 / portTICK_PERIOD_MS);
      reading = digitalRead(BUTTON);
      if(reading == LOW && lastState == HIGH){
        buttonPressedFlag = !buttonPressedFlag;
        currentMode = OFFLINE_MODE; 
      }
    }
    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void taskLED(void *pvParameters){
  bool blinkState = false;
  while(1){
    if(currentMode == OFFLINE_MODE){
      if(buttonPressedFlag){
        blinkState = !blinkState;
        led1State = blinkState;
        led2State = !blinkState;
        digitalWrite(LED1, led1State);
        digitalWrite(LED2, led2State);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }else{
        led1State = false;
        led2State = false;
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }
    }else{
      digitalWrite(LED1, led1State);
      digitalWrite(LED2, led2State);
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}

BLYNK_WRITE(V2){ //led 1
  led1State = param.asInt();
  currentMode = ONLINE_MODE;
}

BLYNK_WRITE(V3){ //led 2
  led2State = param.asInt();
  currentMode = ONLINE_MODE;
}

void taskBlynk(void *pvParams){
  static bool lastButton = false;
  static bool lastLed1 = false;
  static bool lastLed2 = false;
  static int lastCurrentMode = ONLINE_MODE;
  static bool firstRun = true;

  while(1){
    Blynk.run();

    if(firstRun){
      Blynk.virtualWrite(V4, buttonPressedFlag);
      Blynk.virtualWrite(V0, led1State);
      Blynk.virtualWrite(V1, led2State);
      Blynk.virtualWrite(V5, currentMode);
      firstRun = false;
    }

    if(buttonPressedFlag != lastButton){
      Blynk.virtualWrite(V4, buttonPressedFlag);
      Serial.println(buttonPressedFlag);
      lastButton = buttonPressedFlag;
    }

    if(led1State != lastLed1){
      Blynk.virtualWrite(V0, led1State);
      lastLed1 = led1State;
    }

    if(led2State != lastLed2){
      Blynk.virtualWrite(V1, led2State);
      lastLed2 = led2State;
    }

    if(lastCurrentMode != currentMode){
      Blynk.virtualWrite(V5, currentMode);
      lastCurrentMode = currentMode;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup(){
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  xTaskCreate(taskButton, "Button Task", 2048, NULL, 3, NULL);
  xTaskCreate(taskLED, "LED Task", 2048, NULL, 2, NULL);
  xTaskCreate(taskBlynk, "Blynk Task", 4096, NULL, 1, NULL);
}

void loop(){
}