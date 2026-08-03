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


//==================================================
// Servo Objects
//==================================================

Servo shoulder;
Servo elbow;
Servo wrist1;
Servo wrist2;
Servo hand;
Servo base;


//==================================================
// SETUP
//==================================================

void setup()
{
  Serial.begin(19200);

  pinMode(13, OUTPUT);

  // Servo Connections

  base.attach(4);
  shoulder.attach(11);
  elbow.attach(12);
  wrist2.attach(6);
  wrist1.attach(10);
  hand.attach(5);

  Serial.println("Robotic Arm Ready");

  // Move to Home Position
  homeState();
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

    // Plastic
    if (command == 'P' || command == 'p')
    {
      Serial.println("Plastic Selected");

      pickUpObject();

      throwPlastic();

      delay(500);

      homeState();

      Serial.println("Done");
    }

    // Paper
    else if (command == 'A' || command == 'a')
    {
      Serial.println("Paper Selected");

      pickUpObject();

      throwPaper();

      delay(500);

      homeState();

      Serial.println("Done");
    }

    // Cardboard
    else if (command == 'C' || command == 'c')
    {
      Serial.println("Cardboard Selected");

      pickUpObject();

      throwCardboard();

      delay(500);

      homeState();

      Serial.println("Done");
    }

    // Glass
    else if (command == 'G' || command == 'g')
    {
      Serial.println("Glass Selected");

      pickUpObject();

      throwGlass();

      delay(500);

      homeState();

      Serial.println("Done");
    }

    // Metal
    else if (command == 'M' || command == 'm')
    {
      Serial.println("Metal Selected");

      pickUpObject();

      throwMetal();

      delay(500);

      homeState();

      Serial.println("Done");
    }
  }
}


//==================================================
// SLOW SERVO MOVEMENT FUNCTION
//==================================================

void moveServoSlow(Servo &servo, int currentPos, int targetPos, int speedDelay)
{
  if (currentPos < targetPos)
  {
    for (int i = currentPos; i <= targetPos; i++)
    {
      servo.write(i);
      delay(speedDelay);
    }
  }
  else
  {
    for (int i = currentPos; i >= targetPos; i--)
    {
      servo.write(i);
      delay(speedDelay);
    }
  }
}


//==================================================
// PICK UP OBJECT
//==================================================

void pickUpObject()
{
  // Rotate to Pick Position


  // Open Gripper

  hand.write(160);
  handPos = 160;
  delay(500);

  // Move Down

  wrist2.write(110);
  wrist2Pos = 110;
  delay(500);

  elbow.write(140);
  elbowPos = 140;
  delay(500);

  // Shoulder moves slowly

  moveServoSlow(shoulder, shoulderPos, 32, 30);
  shoulderPos = 32;

  delay(500);

  // Close Gripper

  hand.write(15);
  handPos = 15;
  delay(200);

    hand.write(15);
  handPos = 15;
  delay(1000);
}


//==================================================
// THROW PAPER (RIGHT)
//==================================================

void throwPlastic()
{
  // Shoulder moves slowly upwards

  moveServoSlow(shoulder, shoulderPos, 100, 60);
  shoulderPos = 100;


  elbow.write(130);
  elbowPos = 130;

  wrist2.write(110);
  wrist2Pos = 110;

  // RIGHT BIN

  base.write(110);   // Move to the right
  basePos = 110;
  delay(1500);       // Move for 300 milliseconds
  base.write(90);   // STOP the motor

  delay(400);

  // Release Object
  hand.write(160);
  handPos = 160;
  delay(500);

  base.write(75);   
  basePos = 75;
  delay(1200);      
  base.write(90); 
}



//==================================================
// THROW PLASTIC (RIGHT)
//==================================================

void throwPaper()
{
    elbow.write(140);
  elbowPos = 140;
  delay(500);
  // Shoulder moves slowly upwards

  moveServoSlow(shoulder, shoulderPos, 95, 30);
  shoulderPos = 95;

  elbow.write(130);
  elbowPos = 130;
  delay(500);

  wrist2.write(110);
  wrist2Pos = 110;
  delay(500);

  // RIGHT BIN

  base.write(120);   // Move to the right
  basePos = 120;
  delay(2000);       // Move for 300 milliseconds
  base.write(90);   // STOP the motor

  delay(400);

  // Release Object
  hand.write(160);
  handPos = 160;
  delay(500);

  base.write(70); 
  basePos = 70;
  delay(1700);     
  base.write(90); 
}



//==================================================
// THROW GLASS (LEFT) -- calibrate values below
//==================================================

void throwGlass()
{
  // Shoulder moves slowly upwards

 moveServoSlow(shoulder, shoulderPos, 100, 30);
  shoulderPos = 100;

  delay(500);

  elbow.write(130);
  elbowPos = 130;
  delay(500);

  wrist2.write(140);
  wrist2Pos = 140;
  delay(500);

  // LEFT BIN

  base.write(70);  
  basePos = 70;
  delay(2100);      
  base.write(90);  

  delay(400);
  
  // Release Object
  hand.write(160);
  handPos = 160;
  delay(500);

  base.write(120);  
  basePos = 120;
  delay(1500);      
  base.write(90); 
}




//==================================================
// THROW CARDBOARD (LEFT) -- calibrate values below
//==================================================

void throwMetal()
{
  // Shoulder moves slowly upwards
    hand.write(15);
  handPos = 15;

   moveServoSlow(shoulder, shoulderPos, 95, 30);
  shoulderPos = 95;

  delay(500);

  elbow.write(130);
  elbowPos = 130;
  delay(1000);

  wrist2.write(140);
  wrist2Pos = 140;
  delay(500);

  // LEFT BIN

  base.write(75);   
  basePos = 75;
  delay(1500);     
  base.write(90);  

  delay(400);

  // Release Object
  hand.write(160);
  handPos = 160;
  delay(500);

  base.write(110);   
  basePos = 110;
  delay(1600);       
  base.write(90); 
}




//==================================================
// THROW METAL (BACK) -- calibrate values below
//==================================================

void throwCardboard()
{
  // Shoulder moves slowly upwards
  moveServoSlow(shoulder, shoulderPos, 105, 30);
  shoulderPos = 105;

  delay(500);

  elbow.write(130);
  elbowPos = 130;
  delay(1000);
  
  // RIGHT BIN
  base.write(125);   // Move to the right
  basePos = 125;
  delay(2350);       // Move for 300 milliseconds
  base.write(90);   // STOP the motor

  delay(400);

  moveServoSlow(shoulder, shoulderPos, 85, 30);
  shoulderPos = 85;

  delay(300);

  elbow.write(110);
  elbowPos = 110;
  delay(500);

  // Release Object

  hand.write(160);
  handPos = 160;
  delay(500);

  base.write(60);   
  basePos = 60;
  delay(1600);      
  base.write(90); 
}


//==================================================
// HOME POSITION
//==================================================

void homeState()
{


  wrist2.write(160);
  wrist2Pos = 160;
  delay(500);

  elbow.write(160);
  elbowPos = 160;
  delay(500);

  // Shoulder moves slowly back home

  moveServoSlow(shoulder, shoulderPos, 85, 30);
  shoulderPos = 85;

  delay(500);

  // Half Open Waiting Position

  hand.write(120);
  handPos = 120;
  delay(500);
}