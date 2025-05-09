#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SERVO_PIN 5
#define ENCODER_UP_PIN 2
#define ENCODER_DOWN_PIN 3
#define ENCODER_PRESS_PIN 4

Servo feederServo;

const int SERVO_DOWN_ANGLE = 50;  // Angle for the down position
const int SERVO_UP_ANGLE = 130;   // Angle for the up position
int servoSpeed = 2;  // Speed of the servo (the smaller, the slower)

// Function to gradually move the servo
void moveServoGradually(int startAngle, int endAngle, int speed) {
  if (startAngle < endAngle) {
    for (int pos = startAngle; pos <= endAngle; pos++) {
      feederServo.write(pos);
      delay(speed);
    }
  } else {
    for (int pos = startAngle; pos >= endAngle; pos--) {
      feederServo.write(pos);
      delay(speed);
    }
  }
}

enum State {
  INITIAL,
  TIME_SELECTION,
  COUNTDOWN,
  COMPLETED
};

State currentState = INITIAL;

unsigned long lastActivityTime = 0;
int selectedIndex = 0;  // Index for selected time interval

const int timeIntervals[] = {60, 120, 180, 240, 300, 600, 900, 1200, 1800, 2400, 3000, 3600, 5400, 7200, 9000, 10800, 12600, 14400, 16200, 18000, 19800, 21600, 23400, 25200, 27000, 28800, 30600, 32400};
const int timeIntervalsCount = sizeof(timeIntervals) / sizeof(timeIntervals[0]);

bool lastEncoderUpState = HIGH;
bool lastEncoderDownState = HIGH;
bool lastEncoderPressState = HIGH;

void setup() {
  pinMode(ENCODER_UP_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DOWN_PIN, INPUT_PULLUP);
  pinMode(ENCODER_PRESS_PIN, INPUT_PULLUP);

  feederServo.attach(SERVO_PIN);
  feederServo.write(SERVO_UP_ANGLE);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.clearDisplay();
  display.setRotation(2);  // Upside down
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  showInitialMessage();
}

void loop() {
  if (currentState == INITIAL) {
    handleInitialState();
  } 
  else if (currentState == TIME_SELECTION) {
    handleTimeSelectionState();
  } 
  else if (currentState == COUNTDOWN) {
    handleCountdownState();
  } 
  else if (currentState == COMPLETED) {
    handleCompletedState();
  }
}

void showInitialMessage() {
  display.clearDisplay();
  display.setCursor(0, SCREEN_HEIGHT / 2 - 8);
  display.print("Schale einsetzen");
  display.setCursor(0, SCREEN_HEIGHT / 2 + 8);
  display.print("und Drehknopf druecken");
  display.display();
}

void handleInitialState() {
  if (digitalRead(ENCODER_PRESS_PIN) == LOW) {
    delay(100);  // Debounce delay
    if (digitalRead(ENCODER_PRESS_PIN) == LOW) {
      currentState = TIME_SELECTION;
      moveServoGradually(SERVO_UP_ANGLE, SERVO_DOWN_ANGLE, servoSpeed);  // Slow down the servo movement
      showTimeSelection();
    }
  }
}

void showTimeSelection() {
  display.clearDisplay();
  display.setCursor(0, SCREEN_HEIGHT / 2 - 8);
  display.print("Select Time:");
  display.setCursor(0, SCREEN_HEIGHT / 2 + 8);
  
  int minutes = timeIntervals[selectedIndex] / 60;
  int hours = minutes / 60;
  minutes = minutes % 60;

  if (hours > 0) {
    display.print(hours);
    display.print("h ");
  }
  if (minutes > 0) {
    display.print(minutes);
    display.print("m");
  }
  
  display.display();
}

void handleTimeSelectionState() {
  bool currentEncoderUpState = digitalRead(ENCODER_UP_PIN);
  bool currentEncoderDownState = digitalRead(ENCODER_DOWN_PIN);
  bool currentEncoderPressState = digitalRead(ENCODER_PRESS_PIN);

  if (lastEncoderUpState == HIGH && currentEncoderUpState == LOW) {
    delay(10);  // Debounce delay
    if (digitalRead(ENCODER_UP_PIN) == LOW) {
      selectedIndex++;
      if (selectedIndex >= timeIntervalsCount) selectedIndex = 0;
      showTimeSelection();
    }
  }
  lastEncoderUpState = currentEncoderUpState;

  if (lastEncoderDownState == HIGH && currentEncoderDownState == LOW) {
    delay(10);  // Debounce delay
    if (digitalRead(ENCODER_DOWN_PIN) == LOW) {
      selectedIndex--;
      if (selectedIndex < 0) selectedIndex = timeIntervalsCount - 1;
      showTimeSelection();
    }
  }
  lastEncoderDownState = currentEncoderDownState;

  if (lastEncoderPressState == HIGH && currentEncoderPressState == LOW) {
    delay(100);  // Debounce delay
    if (digitalRead(ENCODER_PRESS_PIN) == LOW) {
      currentState = COUNTDOWN;
      lastActivityTime = millis();
      showCountdown();
    }
  }
  lastEncoderPressState = currentEncoderPressState;
}

void showCountdown() {
  display.clearDisplay();
  display.setCursor(0, SCREEN_HEIGHT / 2 - 8);
  display.print("Countdown:");
  display.display();
}

void handleCountdownState() {
  unsigned long elapsedTime = (millis() - lastActivityTime) / 1000;
  int timeRemaining = timeIntervals[selectedIndex] - elapsedTime;
  
  if (timeRemaining <= 0) {
    currentState = COMPLETED;
    moveServoGradually(SERVO_DOWN_ANGLE, SERVO_UP_ANGLE, servoSpeed);  // Slow down the servo movement
    showCompletedMessage();
  } else {
    display.clearDisplay();
    display.setCursor(0, SCREEN_HEIGHT / 2 - 8);
    display.print("Restzeit:");
    display.setCursor(0, SCREEN_HEIGHT / 2 + 8);
    display.print(timeRemaining / 3600);
    display.print("h ");
    display.print((timeRemaining % 3600) / 60);
    display.print("m ");
    display.print(timeRemaining % 60);
    display.print("s");
    display.display();
  }
}

void showCompletedMessage() {
  display.clearDisplay();
  display.setCursor(0, SCREEN_HEIGHT / 2 - 8);
  display.print("Abgeschlossen");
  display.display();
}

void handleCompletedState() {
  if (digitalRead(ENCODER_PRESS_PIN) == LOW) {
    delay(100);  // Debounce delay
    if (digitalRead(ENCODER_PRESS_PIN) == LOW) {
      currentState = INITIAL;
      moveServoGradually(SERVO_DOWN_ANGLE, SERVO_UP_ANGLE, servoSpeed);  // Gradual servo movement for opening the lid
      showInitialMessage();
    }
  }
}
