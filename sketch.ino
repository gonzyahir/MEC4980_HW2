#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>

// Buttons
int stateCycleButton = 4;
int upButton = 5;
int downButton = 6;

enum machineState {
  stepsTaken,
  distanceTraveled,
  strideLength,
  rawData
};

volatile machineState currentState = stepsTaken;

// Code for OLED
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

constexpr int SCLPin = 1;
constexpr int SDAPin = 2;

char text[32];

float stride = 75; // Average step length in cm

float xDistance;
float yDistance;
float zDistance;
float Distance;

float xVelocity;
float yVelocity;
float zVelocity;
float velocity;

float ax, ay, az;
float aTotal;

int steps = 0;

float prevTime;
float currentTime;
float dt;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MPU6050 mpu;

void setup() {
  // Serial Begin
  Serial.begin(115200);

  // Buttons
  pinMode(stateCycleButton, INPUT_PULLDOWN);
  pinMode(upButton, INPUT_PULLDOWN);
  pinMode(downButton, INPUT_PULLDOWN);

  // Begin I2C communication
  Wire.begin(SDAPin, SCLPin);

  // OLED Begin
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
  }
  Serial.println("OLED worked");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Accel Begin
  while (!mpu.begin()) {
    Serial.println("Accel failed");
    delay(100);
  }
  Serial.println("Accel worked");

  prevTime = millis();

}

void loop() {
  if (digitalRead(stateCycleButton)) {
    currentState = (machineState)(((int)currentState + 1) % 4);
  }
  delay(100);

  display.clearDisplay();

  // Accelerometer data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  currentTime = millis();
  dt = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  xVelocity += ax * dt;
  yVelocity += ay * dt;
  zVelocity += az * dt;

  xDistance += xVelocity * dt;
  yDistance += yVelocity * dt;
  zDistance += zVelocity * dt;

  Distance = sqrt(xDistance*xDistance + yDistance*yDistance + zDistance*zDistance);

  steps = Distance / (stride / 100);

  switch (currentState) {
    case stepsTaken:
      sprintf(text, " Steps Taken: %i", steps);

      display.setCursor(0,0);

      break;
    case distanceTraveled:
      sprintf(text, " Distance Traveled:\n %.2f m", Distance);
      
      display.setCursor(0,0);

      break;
    case strideLength:
      if (digitalRead(upButton)) { // each increment is one cm
        stride++;
      }
      else if (digitalRead(downButton)) {
        stride--;
      }

      sprintf(text, " Stride Length: %.0f cm", stride);
      
      display.setCursor(0,0);

      break;
    case rawData:
      sprintf(text, " ax: %.2f\n ay: %.2f\n az: %.2f", ax, ay, az);
      
      display.setCursor(0,0);

      break;
    default:
      break;
  }
  display.println(text);
  display.display();
}
