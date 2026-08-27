#include <Servo.h>

//==================================================
// Servo Position Variables
//==================================================

int shoulderPos = 90;
int elbowPos = 90;
int wrist1Pos = 90;
int wrist2Pos = 90;
int handPos = 90;
int basePos = 90;

// Emergency Stop State
volatile bool emergencyActive = false;


//==================================================
// Servo Objects
//==================================================

Servo shoulder;
Servo elbow;
Servo wrist1;
Servo wrist2;
Servo hand;
Servo base;

// Forward declarations
void triggerEmergencyStop();
bool checkEmergency();
bool safeDelay(unsigned long ms);
void homeState();
void moveServoSlow(Servo &servo, int &currentPosVar, int targetPos, int speedDelay);
void moveShoulderEased(int currentPos, int targetPos, int baseDelay);
void pickUpObject();
void throwPlastic();
void throwPaper();
void throwGlass();
void throwMetal();
void throwCardboard();


//==================================================
// EMERGENCY STOP HANDLER
//==================================================

void triggerEmergencyStop()
{
  emergencyActive = true;

  // Immediately HALT continuous rotation base motor
  base.write(90);
  basePos = 90;

  // Lock and freeze all joints at their current positions
  shoulder.write(shoulderPos);
  elbow.write(elbowPos);
  wrist2.write(wrist2Pos);
  wrist1.write(wrist1Pos);
  hand.write(handPos);

  // Turn ON Pin 13 LED as visual Emergency indicator
  digitalWrite(13, HIGH);

  Serial.println("[EMERGENCY] !!! EMERGENCY SAFETY STOP ACTIVATED ('E') !!!");
  Serial.println("[EMERGENCY] Continuous base motor halted instantly.");
  Serial.println("[EMERGENCY] All 6-DOF servo joints locked at current coordinates.");
  Serial.println("[EMERGENCY] Press 'Reset Arm' [R] or 'Home' [H] to clear E-Stop.");
  Serial.println("Done");
}

bool checkEmergency()
{
  if (emergencyActive) return true;
  if (Serial.available())
  {
    char c = Serial.peek();
    if (c == 'E' || c == 'e')
    {
      Serial.read(); // consume 'E'
      triggerEmergencyStop();
      return true;
    }
  }
  return false;
}

bool safeDelay(unsigned long ms)
{
  if (emergencyActive) return false;
  unsigned long start = millis();
  while (millis() - start < ms)
  {
    if (checkEmergency()) return false;
    delay(5);
  }
  return true;
}


//==================================================
// SETUP
//==================================================

void setup()
{
  Serial.begin(19200);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // Servo Connections
  base.attach(4);
  shoulder.attach(11);
  elbow.attach(12);
  wrist2.attach(6);
  wrist1.attach(10);
  hand.attach(5);

  Serial.println("[FIRMWARE] Robotic Arm 6-DOF Controller Ready @ 19200 Baud");
  Serial.println("[PINS] Base:4, Shoulder:11, Elbow:12, Wrist2:6, Wrist1:10, Hand:5");
  Serial.println("[HOMING] Arm moving to Initial Home Position (90 deg)...");

  // Move to Home Position
  homeState();

  Serial.println("[READY] Robotic Arm calibrated & ready for segregation commands.");
}


//==================================================
// LOOP
//==================================================

void loop()
{
  // Wait for command from Serial Monitor

  if (Serial.available())
  {
    char command = Serial.read();

    // Emergency Stop
    if (command == 'E' || command == 'e')
    {
      triggerEmergencyStop();
    }

    // Plastic
    else if ((command == 'P' || command == 'p') && !emergencyActive)
    {
      Serial.println("[CMD: 'P'] Plastic Bottle Selected -> Target: Right Bin (Blue)");
      pickUpObject();
      if (!emergencyActive) throwPlastic();
      if (!emergencyActive) {
        safeDelay(400);
        homeState();
        Serial.println("[COMPLETE] Plastic sorting routine finished.");
        Serial.println("Done");
      }
    }

    // Paper
    else if ((command == 'A' || command == 'a') && !emergencyActive)
    {
      Serial.println("[CMD: 'A'] Paper Sheet Selected -> Target: Far Right Bin (Green)");
      pickUpObject();
      if (!emergencyActive) throwPaper();
      if (!emergencyActive) {
        safeDelay(400);
        homeState();
        Serial.println("[COMPLETE] Paper sorting routine finished.");
        Serial.println("Done");
      }
    }

    // Cardboard
    else if ((command == 'C' || command == 'c') && !emergencyActive)
    {
      Serial.println("[CMD: 'C'] Cardboard Box Selected -> Target: Back Bin (Brown)");
      pickUpObject();
      if (!emergencyActive) throwCardboard();
      if (!emergencyActive) {
        safeDelay(400);
        homeState();
        Serial.println("[COMPLETE] Cardboard sorting routine finished.");
        Serial.println("Done");
      }
    }

    // Glass
    else if ((command == 'G' || command == 'g') && !emergencyActive)
    {
      Serial.println("[CMD: 'G'] Glass Jar Selected -> Target: Left Bin (Gray)");
      pickUpObject();
      if (!emergencyActive) throwGlass();
      if (!emergencyActive) {
        safeDelay(400);
        homeState();
        Serial.println("[COMPLETE] Glass sorting routine finished.");
        Serial.println("Done");
      }
    }

    // Metal
    else if ((command == 'M' || command == 'm') && !emergencyActive)
    {
      Serial.println("[CMD: 'M'] Metal Can Selected -> Target: Far Left Bin (Yellow)");
      pickUpObject();
      if (!emergencyActive) throwMetal();
      if (!emergencyActive) {
        safeDelay(400);
        homeState();
        Serial.println("[COMPLETE] Metal sorting routine finished.");
        Serial.println("Done");
      }
    }

    // Home / Reset Position (Clears Emergency Stop)
    else if (command == 'H' || command == 'h' || command == 'R' || command == 'r')
    {
      emergencyActive = false;
      digitalWrite(13, LOW);
      Serial.println("[CMD: 'RESET'] Emergency cleared. Moving arm to Home Position...");
      homeState();
      safeDelay(200);
      Serial.println("[COMPLETE] Arm returned to Home Position.");
      Serial.println("Done");
    }
  }
}


//==================================================
// SLOW SERVO MOVEMENT FUNCTION (generic, used for smooth joint interpolation)
//==================================================

void moveServoSlow(Servo &servo, int &currentPosVar, int targetPos, int speedDelay)
{
  if (currentPosVar == targetPos) return;

  if (currentPosVar < targetPos)
  {
    for (int i = currentPosVar; i <= targetPos; i++)
    {
      if (checkEmergency()) return;
      servo.write(i);
      currentPosVar = i;
      if (!safeDelay(speedDelay)) return;
    }
  }
  else
  {
    for (int i = currentPosVar; i >= targetPos; i--)
    {
      if (checkEmergency()) return;
      servo.write(i);
      currentPosVar = i;
      if (!safeDelay(speedDelay)) return;
    }
  }
  currentPosVar = targetPos;
}


//==================================================
// EASED SHOULDER MOVEMENT FUNCTION
// Starts slow, speeds up smoothly, slows down gently near target.
//==================================================

void moveShoulderEased(int currentPos, int targetPos, int baseDelay)
{
  int totalSteps = abs(targetPos - currentPos);
  if (totalSteps == 0) return;

  int direction = (targetPos > currentPos) ? 1 : -1;

  for (int step = 0; step <= totalSteps; step++)
  {
    if (checkEmergency()) return;

    int pos = currentPos + (direction * step);
    shoulder.write(pos);
    shoulderPos = pos;

    // Distance from nearest end of the motion (0 = at an endpoint)
    int distFromStart = step;
    int distFromEnd = totalSteps - step;
    int distFromEdge = min(distFromStart, distFromEnd);

    // Ease window: steps at each end that receive smooth deceleration
    int easeWindow = min(20, totalSteps / 3);

    int stepDelay;
    if (easeWindow > 0 && distFromEdge < easeWindow)
    {
      int extra = map(distFromEdge, 0, easeWindow, baseDelay * 2, 0);
      stepDelay = baseDelay + extra;
    }
    else
    {
      stepDelay = baseDelay;
    }

    if (!safeDelay(stepDelay)) return;
  }

  shoulderPos = targetPos;
}


//==================================================
// PICK UP OBJECT (~3.8 seconds)
//==================================================

void pickUpObject()
{
  if (checkEmergency()) return;

  // Open Gripper smoothly
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(400)) return;

  // Move Wrist & Elbow gently down
  moveServoSlow(wrist2, wrist2Pos, 110, 15);
  if (!safeDelay(300)) return;

  moveServoSlow(elbow, elbowPos, 140, 15);
  if (!safeDelay(300)) return;

  // Shoulder moves slowly & gracefully down (eased, 22ms base delay)
  moveShoulderEased(shoulderPos, 32, 22);
  if (checkEmergency()) return;
  if (!safeDelay(400)) return;

  // Close Gripper gently onto the waste item
  moveServoSlow(hand, handPos, 15, 12);
  if (!safeDelay(600)) return;
}


//==================================================
// THROW PLASTIC (RIGHT) (~5.5 seconds)
//==================================================

void throwPlastic()
{
  if (checkEmergency()) return;

  // Shoulder lifts waste smoothly upwards
  moveShoulderEased(shoulderPos, 100, 22);
  if (checkEmergency()) return;

  moveServoSlow(elbow, elbowPos, 130, 15);
  moveServoSlow(wrist2, wrist2Pos, 110, 15);
  if (!safeDelay(400)) return;

  // RIGHT BIN: Rotate base motor steadily
  base.write(110);
  basePos = 110;
  if (!safeDelay(1900)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;

  // Release Object into Right Bin
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(600)) return;

  // Return base smoothly to center
  base.write(75);
  basePos = 75;
  if (!safeDelay(1200)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;
}


//==================================================
// THROW PAPER (FAR RIGHT) (~5.8 seconds)
//==================================================

void throwPaper()
{
  if (checkEmergency()) return;

  // Shoulder lifts waste smoothly upwards
  moveShoulderEased(shoulderPos, 95, 22);
  if (checkEmergency()) return;

  moveServoSlow(elbow, elbowPos, 130, 15);
  moveServoSlow(wrist2, wrist2Pos, 110, 15);
  if (!safeDelay(400)) return;

  // FAR RIGHT BIN: Rotate base motor steadily
  base.write(120);
  basePos = 120;
  if (!safeDelay(2500)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;

  // Release Object into Far Right Bin
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(600)) return;

  // Return base smoothly to center
  base.write(70);
  basePos = 70;
  if (!safeDelay(1900)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;
}


//==================================================
// THROW GLASS (LEFT) (~5.5 seconds)
//==================================================

void throwGlass()
{
  if (checkEmergency()) return;

  // Shoulder lifts waste smoothly upwards
  moveShoulderEased(shoulderPos, 100, 22);
  if (checkEmergency()) return;

  moveServoSlow(elbow, elbowPos, 130, 15);
  moveServoSlow(wrist2, wrist2Pos, 140, 15);
  if (!safeDelay(400)) return;

  // LEFT BIN: Rotate base motor steadily
  base.write(70);
  basePos = 70;
  if (!safeDelay(2100)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;

  // Release Object into Left Bin
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(600)) return;

  // Return base smoothly to center
  base.write(120);
  basePos = 120;
  if (!safeDelay(1500)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;
}


//==================================================
// THROW METAL (FAR LEFT) (~5.5 seconds)
//==================================================

void throwMetal()
{
  if (checkEmergency()) return;

  // Shoulder lifts waste smoothly upwards
  moveShoulderEased(shoulderPos, 95, 22);
  if (checkEmergency()) return;

  moveServoSlow(elbow, elbowPos, 130, 15);
  moveServoSlow(wrist2, wrist2Pos, 140, 15);
  if (!safeDelay(400)) return;

  // FAR LEFT BIN: Rotate base motor steadily
  base.write(75);
  basePos = 75;
  if (!safeDelay(1800)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;

  // Release Object into Far Left Bin
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(600)) return;

  // Return base smoothly to center
  base.write(110);
  basePos = 110;
  if (!safeDelay(1600)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;
}


//==================================================
// THROW CARDBOARD (BACK) (~5.6 seconds)
//==================================================

void throwCardboard()
{
  if (checkEmergency()) return;

  // Shoulder lifts waste smoothly upwards
  moveShoulderEased(shoulderPos, 105, 22);
  if (checkEmergency()) return;

  moveServoSlow(elbow, elbowPos, 130, 15);
  if (!safeDelay(300)) return;

  // BACK BIN: Rotate base motor steadily
  base.write(125);
  basePos = 125;
  if (!safeDelay(2200)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;

  moveShoulderEased(shoulderPos, 85, 20);
  moveServoSlow(elbow, elbowPos, 110, 15);
  if (!safeDelay(300)) return;

  // Release Object into Back Bin
  moveServoSlow(hand, handPos, 160, 10);
  if (!safeDelay(600)) return;

  // Return base smoothly to center
  base.write(60);
  basePos = 60;
  if (!safeDelay(1600)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);
  basePos = 90;
  if (!safeDelay(400)) return;
}


//==================================================
// HOME POSITION (~2.8 seconds)
//==================================================

void homeState()
{
  moveServoSlow(wrist2, wrist2Pos, 160, 15);
  moveServoSlow(elbow, elbowPos, 160, 15);
  moveShoulderEased(shoulderPos, 85, 22);
  moveServoSlow(hand, handPos, 120, 12);
  safeDelay(400);
}
