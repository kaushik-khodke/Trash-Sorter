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
// SLOW SERVO MOVEMENT FUNCTION (generic, used for non-shoulder joints)
//==================================================

void moveServoSlow(Servo &servo, int currentPos, int targetPos, int speedDelay)
{
  if (currentPos < targetPos)
  {
    for (int i = currentPos; i <= targetPos; i++)
    {
      if (checkEmergency()) return;
      servo.write(i);
      if (!safeDelay(speedDelay)) return;
    }
  }
  else
  {
    for (int i = currentPos; i >= targetPos; i--)
    {
      if (checkEmergency()) return;
      servo.write(i);
      if (!safeDelay(speedDelay)) return;
    }
  }
}


//==================================================
// EASED SHOULDER MOVEMENT FUNCTION
// Starts slow, speeds up in the middle, slows down again
// near the target -> feels gentle/"easy" instead of a sudden snap.
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

    // Ease window: how many steps at each end get the slow treatment
    int easeWindow = min(15, totalSteps / 3);

    int stepDelay;
    if (easeWindow > 0 && distFromEdge < easeWindow)
    {
      // Slower near start/end: extra delay tapers off linearly
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
// PICK UP OBJECT
//==================================================

void pickUpObject()
{
  if (checkEmergency()) return;

  // Open Gripper
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  // Move Down
  wrist2.write(110);
  wrist2Pos = 110;
  if (!safeDelay(500)) return;

  elbow.write(140);
  elbowPos = 140;
  if (!safeDelay(500)) return;

  // Shoulder moves slowly + gently (eased)
  moveShoulderEased(shoulderPos, 32, 12);
  if (checkEmergency()) return;

  if (!safeDelay(500)) return;

  // Close Gripper
  hand.write(15);
  handPos = 15;
  if (!safeDelay(200)) return;

  hand.write(15);
  handPos = 15;
  if (!safeDelay(1000)) return;
}


//==================================================
// THROW PLASTIC (RIGHT)
//==================================================

void throwPlastic()
{
  if (checkEmergency()) return;

  // Shoulder moves slowly + gently upwards
  moveShoulderEased(shoulderPos, 100, 15);
  if (checkEmergency()) return;

  elbow.write(130);
  elbowPos = 130;

  wrist2.write(110);
  wrist2Pos = 110;

  // RIGHT BIN: Rotate base motor
  base.write(110);   // Move to the right
  basePos = 110;
  if (!safeDelay(1900)) {
    base.write(90);  // Halt motor immediately on emergency
    basePos = 90;
    return;
  }
  base.write(90);   // STOP the motor
  basePos = 90;

  if (!safeDelay(400)) return;

  // Release Object
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  // Return base
  base.write(75);   
  basePos = 75;
  if (!safeDelay(1200)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90); 
  basePos = 90;
}


//==================================================
// THROW PAPER (FAR RIGHT)
//==================================================

void throwPaper()
{
  if (checkEmergency()) return;

  elbow.write(140);
  elbowPos = 140;
  if (!safeDelay(500)) return;

  // Shoulder moves slowly + gently upwards
  moveShoulderEased(shoulderPos, 95, 15);
  if (checkEmergency()) return;

  elbow.write(130);
  elbowPos = 130;
  if (!safeDelay(500)) return;

  wrist2.write(110);
  wrist2Pos = 110;
  if (!safeDelay(500)) return;

  // RIGHT BIN (Far Right)
  base.write(120);   // Move to the far right
  basePos = 120;
  if (!safeDelay(2600)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);   // STOP the motor
  basePos = 90;

  if (!safeDelay(400)) return;

  // Release Object
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  base.write(70); 
  basePos = 70;
  if (!safeDelay(2000)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90); 
  basePos = 90;
}


//==================================================
// THROW GLASS (LEFT)
//==================================================

void throwGlass()
{
  if (checkEmergency()) return;

  // Shoulder moves slowly + gently upwards
  moveShoulderEased(shoulderPos, 100, 15);
  if (checkEmergency()) return;

  if (!safeDelay(500)) return;

  elbow.write(130);
  elbowPos = 130;
  if (!safeDelay(500)) return;

  wrist2.write(140);
  wrist2Pos = 140;
  if (!safeDelay(500)) return;

  // LEFT BIN
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
  
  // Release Object
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  base.write(120);  
  basePos = 120;
  if (!safeDelay(1500)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90); 
  basePos = 90;
}


//==================================================
// THROW METAL (FAR LEFT)
//==================================================

void throwMetal()
{
  if (checkEmergency()) return;

  hand.write(15);
  handPos = 15;

  moveShoulderEased(shoulderPos, 95, 15);
  if (checkEmergency()) return;

  if (!safeDelay(500)) return;

  elbow.write(130);
  elbowPos = 130;
  if (!safeDelay(1000)) return;

  wrist2.write(140);
  wrist2Pos = 140;
  if (!safeDelay(500)) return;

  // LEFT BIN
  base.write(75);   
  basePos = 75;
  if (!safeDelay(1500)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);  
  basePos = 90;

  if (!safeDelay(400)) return;

  // Release Object
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  base.write(110);   
  basePos = 110;
  if (!safeDelay(1600)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90); 
  basePos = 90;
}


//==================================================
// THROW CARDBOARD (BACK)
//==================================================

void throwCardboard()
{
  if (checkEmergency()) return;

  moveShoulderEased(shoulderPos, 105, 15);
  if (checkEmergency()) return;

  if (!safeDelay(500)) return;

  elbow.write(130);
  elbowPos = 130;
  if (!safeDelay(1000)) return;
  
  // RIGHT BIN
  base.write(125);   // Move to the right
  basePos = 125;
  if (!safeDelay(2350)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90);   // STOP the motor
  basePos = 90;

  if (!safeDelay(400)) return;

  moveShoulderEased(shoulderPos, 85, 15);
  if (checkEmergency()) return;

  if (!safeDelay(300)) return;

  elbow.write(110);
  elbowPos = 110;
  if (!safeDelay(500)) return;

  // Release Object
  hand.write(160);
  handPos = 160;
  if (!safeDelay(500)) return;

  base.write(60);   
  basePos = 60;
  if (!safeDelay(1600)) {
    base.write(90);
    basePos = 90;
    return;
  }
  base.write(90); 
  basePos = 90;
}


//==================================================
// HOME POSITION
//==================================================

void homeState()
{
  wrist2.write(160);
  wrist2Pos = 160;
  delay(200);

  elbow.write(160);
  elbowPos = 160;
  delay(200);

  // Shoulder moves slowly + gently back home
  moveShoulderEased(shoulderPos, 85, 15);

  delay(200);

  // Half Open Waiting Position
  hand.write(120);
  handPos = 120;
  delay(200);
}
