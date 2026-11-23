#include <Arduino_FreeRTOS.h>


// ==========================================
// Pin Definitions
// ==========================================
const int LED_1      = 2;  // EC.3: Digital Pin 2
const int LED_2      = 3;  // EC.3: Digital Pin 3
const int BUTTON_PIN = 4;  // Momentary push button (active LOW with INPUT_PULLUP)
const int MOTOR_PIN  = 5;  // PWM-capable pin for DC motor fan

// ==========================================
// Shared Variables (volatile for RTOS safety)
// ==========================================
volatile unsigned long interval1 = 0; // LED 1 blink interval in ms (0 = OFF)
volatile unsigned long interval2 = 0; // LED 2 blink interval in ms (0 = OFF)

enum FanState {
  FAN_OFF    = 0,
  FAN_LOW    = 1,
  FAN_MEDIUM = 2,
  FAN_HIGH   = 3
};

volatile FanState currentFanState = FAN_OFF;

// PWM duty cycles for 4 fan states (0 - 255)
const uint8_t FAN_PWM_VALUES[4] = {0, 85, 170, 255};

// Serial input state machine
enum SerialState {
  WAITING_FOR_LED,
  WAITING_FOR_INTERVAL
};

// ==========================================
// FreeRTOS Task Prototypes (HL.19, HL.20)
// ==========================================
void TaskSerialInput(void *pvParameters);
void TaskLED1(void *pvParameters);
void TaskLED2(void *pvParameters);
void TaskButton(void *pvParameters);
void TaskMotorFan(void *pvParameters);

void setup() {
  // Configure hardware pins
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Assumes button connects pin to GND

  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  analogWrite(MOTOR_PIN, 0);

  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port
  }

  Serial.println("FreeRTOS System Ready (PROJECT-USERTOS)!");
  Serial.println("What LED? (1 or 2)");

  // Create FreeRTOS Tasks
  xTaskCreate(TaskSerialInput, "SerialIO", 256, NULL, 2, NULL);
  xTaskCreate(TaskLED1,        "LED1",     128, NULL, 1, NULL);
  xTaskCreate(TaskLED2,        "LED2",     128, NULL, 1, NULL);
  xTaskCreate(TaskButton,      "Button",   128, NULL, 2, NULL);
  xTaskCreate(TaskMotorFan,    "MotorFan", 128, NULL, 1, NULL);

  // FreeRTOS scheduler starts automatically on Arduino Uno R3
}

void loop() {
  // Empty: All execution is managed by the FreeRTOS Task Dispatcher (HL.20)
}

// ==========================================
// Task 1: Asynchronous Serial Input
// ==========================================
void TaskSerialInput(void *pvParameters) {
  (void) pvParameters;
  SerialState currentState = WAITING_FOR_LED;
  int selectedLED = 0;

  for (;;) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();

      if (input.length() > 0) {
        long value = input.toInt();

        if (currentState == WAITING_FOR_LED) {
          if (value == 1 || value == 2) {
            selectedLED = (int)value;
            currentState = WAITING_FOR_INTERVAL;
            Serial.println("What interval (in msec)?");
          } else {
            Serial.println("Invalid LED. Please enter 1 or 2.");
            Serial.println("What LED? (1 or 2)");
          }
        }
        else if (currentState == WAITING_FOR_INTERVAL) {
          if (value < 0) {
            Serial.println("Invalid interval.");
          } else {
            unsigned long newInterval = (unsigned long)value;
            if (selectedLED == 1) {
              interval1 = newInterval;
              Serial.print("LED 1 set to blink at ");
              Serial.print(interval1);
              Serial.println(" ms interval");
            } else if (selectedLED == 2) {
              interval2 = newInterval;
              Serial.print("LED 2 set to blink at ");
              Serial.print(interval2);
              Serial.println(" ms interval");
            }
          }
          currentState = WAITING_FOR_LED;
          Serial.println("What LED? (1 or 2)");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// ==========================================
// Task 2: LED 1 Blinking (Pin 2)
// ==========================================
void TaskLED1(void *pvParameters) {
  (void) pvParameters;
  bool ledState = LOW;

  for (;;) {
    unsigned long currentInterval = interval1;
    if (currentInterval > 0) {
      ledState = !ledState;
      digitalWrite(LED_1, ledState);
      vTaskDelay(pdMS_TO_TICKS(currentInterval / 2));
    } else {
      ledState = LOW;
      digitalWrite(LED_1, LOW);
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

// ==========================================
// Task 3: LED 2 Blinking (Pin 3)
// ==========================================
void TaskLED2(void *pvParameters) {
  (void) pvParameters;
  bool ledState = LOW;

  for (;;) {
    unsigned long currentInterval = interval2;
    if (currentInterval > 0) {
      ledState = !ledState;
      digitalWrite(LED_2, ledState);
      vTaskDelay(pdMS_TO_TICKS(currentInterval / 2));
    } else {
      ledState = LOW;
      digitalWrite(LED_2, LOW);
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

// ==========================================
// Task 4: Push-Button State Machine (Debounced)
// Cycles: OFF -> LOW -> MEDIUM -> HIGH -> OFF
// ==========================================
void TaskButton(void *pvParameters) {
  (void) pvParameters;
  bool lastReading = HIGH;

  for (;;) {
    bool currentReading = digitalRead(BUTTON_PIN);

    // Detect falling edge (button pressed)
    if (lastReading == HIGH && currentReading == LOW) {
      vTaskDelay(pdMS_TO_TICKS(50)); // Debounce delay

      if (digitalRead(BUTTON_PIN) == LOW) {
        // Advance fan state sequentially
        currentFanState = (FanState)((currentFanState + 1) % 4);
      }
    }

    lastReading = currentReading;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// ==========================================
// Task 5: PWM DC Motor Fan Control
// ==========================================
void TaskMotorFan(void *pvParameters) {
  (void) pvParameters;
  FanState appliedState = FAN_OFF;

  for (;;) {
    FanState targetState = currentFanState;

    if (targetState != appliedState) {
      analogWrite(MOTOR_PIN, FAN_PWM_VALUES[targetState]);
      appliedState = targetState;
    }

    vTaskDelay(pdMS_TO_TICKS(30));
  }
}