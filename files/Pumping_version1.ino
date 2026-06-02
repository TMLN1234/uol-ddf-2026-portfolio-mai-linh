#include <Button.h>

Button deflateButton(4);

const int pumpPin = 8;
const int deflatePin = 10;
const int pirPin = 2;
const int flexPin = A0;
const int joyStickX = A2;
const int joyStickY = A1;

unsigned long lastActivation = 0;
const unsigned long cooldown = 7000; // 7 sec

void setup() {

  pinMode(pumpPin, OUTPUT);
  pinMode(deflatePin, OUTPUT);
  pinMode(pirPin, INPUT);
  //pinMode(flexPin, INPUT);
  pinMode(joyStickX, INPUT);
  pinMode(joyStickY, INPUT);

  digitalWrite(pumpPin, LOW);
  digitalWrite(deflatePin, LOW);

  Serial.begin(9600);

  deflateButton.begin();
}

void loop() {

  //Deflate by moving joy stick up
  int joyStickXValue;
  int joyStickYValue;

  joyStickXValue = analogRead(joyStickX);
  joyStickYValue = analogRead(joyStickY);

  if (joyStickXValue > 1000 && joyStickYValue > 850) {
    digitalWrite(deflatePin, HIGH);
    digitalWrite(pumpPin, LOW);
    Serial.println("Joystick value x:");
    Serial.println(joyStickXValue);
    Serial.println("Joystick value y:");
    Serial.println(joyStickYValue);
    //delay(2000);
  } else if (joyStickXValue < 1000 && joyStickYValue < 850) {
    digitalWrite(deflatePin, LOW);
  }
 
  // Inflate by bending
  int flexValue;
  flexValue = analogRead(flexPin);
  
  if (flexValue > 2) {
    digitalWrite(deflatePin, LOW);
    digitalWrite(pumpPin, HIGH);
    //Serial.println("Sensor Value");
    //Serial.println(flexValue);
    delay(2000);
  } else if (flexValue == 0) {
    digitalWrite(pumpPin, LOW);
  }

  // ===== Manual Deflate =====
  if (deflateButton.read() == Button::PRESSED) {

    Serial.println("Deflating");

    digitalWrite(pumpPin, LOW);
    digitalWrite(deflatePin, HIGH);

    delay(2000);

    digitalWrite(deflatePin, LOW);
  }

  // ===== PIR Detection =====
  /*int motion = digitalRead(pirPin);

  // Only trigger if cooldown expired
  if (motion == HIGH &&
      millis() - lastActivation > cooldown) {

    lastActivation = millis();

    Serial.println("Motion detected");

    digitalWrite(deflatePin, LOW);
    digitalWrite(pumpPin, HIGH);

    delay(3000);

    digitalWrite(pumpPin, LOW);

    Serial.println("Inflation complete");
  }
  */
}
