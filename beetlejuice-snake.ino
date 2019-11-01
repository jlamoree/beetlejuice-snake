#include <AccelStepper.h>
#include <Servo.h>

const int servoPin = 4;
const int triggerSwitchPin = 12;
const int stepperPulsePin = 7;
const int stepperDirectionPin = 6;
const int waitBeforeDeploy = 6000;
const int deployedPosition = -225;
const unsigned long deployDurationMillis = 2000;

unsigned long deployMillis = 0;
bool triggerSwitchPressed = false;
int routineState = 0; // 0: Ready, 1: Deploying, 2: Deployed, 3: Undeploying

AccelStepper stepper(1, stepperPulsePin, stepperDirectionPin);
Servo servo;

void startRoutine() {
  if (routineState != 0) {
    Serial.println("Error: Cannot start routine while not in ready state.");
    return;
  }
  routineState = 1;
  delay(waitBeforeDeploy);
  Serial.println("Deploying.");
}

void runRoutine() {
  if (routineState == 1) {
    if (stepper.targetPosition() != deployedPosition) {
      stepper.setMaxSpeed(800);
      stepper.setAcceleration(60);
      stepper.moveTo(deployedPosition);
      stepper.run();
      return;
    }
    if (abs(stepper.currentPosition()) >= abs(deployedPosition)) {
      routineState = 2;
      deployMillis = millis();
      Serial.println("Deployed.");
    } else {
      stepper.run();    
    }
  }
  if (routineState == 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - deployMillis >= deployDurationMillis) {
      deployMillis = currentMillis;
      routineState = 3;
      Serial.println("Undeploying.");
    }
  }
  if (routineState == 3) {
    if (stepper.targetPosition() != 0) {
      stepper.setMaxSpeed(500);
      stepper.setAcceleration(45);
      stepper.moveTo(0);
      stepper.run();
      return;
    }
    if (stepper.currentPosition() == 0) {
      routineState = 0;
      Serial.print("Ready.");
    } else {
      
      stepper.run();
    }
  }
}

void handleTriggerSwitch() {
  boolean currentTriggerSwitchPinValue = digitalRead(triggerSwitchPin);
  if (currentTriggerSwitchPinValue == LOW && !triggerSwitchPressed) {
    triggerSwitchPressed = true;
    Serial.println("Starting routine.");
    delay(200);
    startRoutine();
  } else if (currentTriggerSwitchPinValue == HIGH && triggerSwitchPressed) {
    triggerSwitchPressed = false;
    delay(200);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(triggerSwitchPin, INPUT);
  //servo.attach(servoPin);
}

void loop() {
  if (routineState == 0) {
    handleTriggerSwitch();
  } else {
    runRoutine();
  }
}
