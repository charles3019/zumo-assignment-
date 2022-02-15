/*
******** LIBRARIES ********
*/
#include <Zumo32U4.h>
#include <QTRSensors.h>
#include <Zumo32U4Buzzer.h>
#include <Wire.h>
#include "TurnSensor.h"
#include <L3G.h>

Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors sensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LCD lcd;
L3G gyro;
Zumo32U4ButtonA buttonA; //required by TurnSensor.h  
/*
******** VARIABLES ********
*/
const uint16_t moveDuration = 155;
int motorspeed = 100;
bool start = false; 
char input, lastInput;
int calibratedValue[3];                 // the calibrated QTR_THRESHOLD of the black line
bool objectFound = false; 
int corridorCounter = 1;                // used to count the number of corridors
int previousCorridor = 0;               // used to store previous corridor when entering side corridors
String rooms[50];                       // used to store the direction of each room
String message;                         // used to store a specific message to send over to GUI
bool roomScan = false;                  // used to store the state of scanning a room
bool isAtEnd = false;                   // checks whether the robot is at the end of a side-corridor
bool isSideCorridor = false;            // checks whether the robot is inside a side-corridor
int roomCounter = 0;
// Define an array for holding sensor values.
//https://github.com/pololu/zumo-shield/blob/master/ZumoReflectanceSensorArray/examples/SensorCalibration/SensorCalibration.ino
#define NUM_SENSORS 3
unsigned int sensorValues[NUM_SENSORS];
/*
******** setup FUNCTION ********
*/

void setup() {
  Serial1.begin(9600);
  delay(1000);
  buzzer.play(">g32>>c32");
  sensors.initThreeSensors();
  proxSensors.initThreeSensors();
  char input = (char) Serial.read();  // waiting for GUI to send calibrate command, 'g'
  while (input != 'g')
  {
    input = (char) Serial1.read();
  }
  calibrateZumo();
  turnSensorSetup();
  delay(500);
  turnSensorReset();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    calibratedValue[i] = sensors.calibratedMaximumOn[i];
  }

}

/*
******** loop FUNCTION ********
*/

void loop() {
  inputGUI();
  if(start){
    detectWall();
    }

}
void calibrateZumo()
{
  
  // Wait 1 second and then begin automatic sensor calibration
  delay(1000);

  //loop used for rotating the zumo in place to sweep the sensors over the line
  for (int i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    sensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0, 0);   //stop it from moving

  // Turn off LED and play buzzer to indicate we are through with calibration
  digitalWrite(13, LOW);
  buzzer.play(">g32>>c32");
}

void manualMove(int speedl, int speedr){
  motors.setSpeeds(speedl, speedr);
  delay(moveDuration);
  motors.setSpeeds(0, 0);
}

void zumoMove(){
  char zumoDirection = ' ';
  Serial1.print("You have control!");
  while (!(zumoDirection == 'c' || zumoDirection == 't' || zumoDirection == 'm')){
    
    zumoDirection = (char) Serial1.read();
    //forward
    //Serial1.print("loop");
    if (zumoDirection == 'w'){
      manualMove(motorspeed, motorspeed);
    }
    else if(zumoDirection == 'a'){
      manualMove(-motorspeed, -motorspeed);
    }
    else if(zumoDirection == 'l'){
      manualMove(-motorspeed, motorspeed);
    }
    else if(zumoDirection == 'r'){
      manualMove(motorspeed, -motorspeed);
    }
    //90 degree righ turn
    else if(zumoDirection == 't'){
      turnRight(90);
    }
    //90 degree left turn
    else if(zumoDirection == 'm'){
      turnLeft(90);
    }
  }
  lastInput = 'c';    //request that a 'complete' command needs to be done OR reactivate auto movement
}

void inputGUI(){
  while (Serial1.available() > 0 && lastInput != 'c'){
    start = true;  // start the zumo border detection as we have input
    input = (char) Serial1.read();
    //stop zumo
    if (input == 'x')
    {
      motors.setSpeeds(0, 0);
      zumoMove(); // move the zumo manually
    }
    //used to automatically move the robot until it hits a wall
    else if (input == 'p')
    {
      motors.setSpeeds(motorspeed, motorspeed);
    }
    else if (input == 'z')
    {
      motors.setSpeeds(0, 0);
      Serial1.print("Please show me the direction of the room!");
      do{
        input = (char) Serial1.read();
      }while (!(input == 'l' || input == 'r'));
      if (input == 'l'){
        rooms[roomCounter] = "left";                //save the direction into that room
        turnLeft(90);
      }
      //if the direction of the room is right
      else{
        rooms[roomCounter] = "right";               //save the direction into that room
        turnRight(90);
      }
      message = "Room: ";
      //send the room details to GUI
      Serial1.print(message + (roomCounter + 1));    
      message = " on the " + rooms[roomCounter] + " side of corridor ";
      Serial1.print(message + corridorCounter);
      // Serial1.print(STRING_TERMINATOR);
      delay(10);
      roomScan = true;    //used to show that the room needs to be scanned
      roomCounter++;      //increment the room number
      scanRoom();

      turnRight(180); // turn 180 degrees so that the zumo is facing door of the room.
      motors.setSpeeds(110, 110); // move forward so that zumo is inside Corridor
      delay(185);
      //Turn left or right based on the previous input.
      if (input == 'l'){
        turnLeft(90);
      }
      else{
         turnRight(90);
      }
      // zumoMove();         // move the zumo manually
    }

  }

    if (lastInput == 'c')
  {
    motors.setSpeeds(motorspeed, motorspeed);
    Serial1.print("Automatic zumo control!");
    delay(20);
    lastInput = ' ';  //reset the 'complete' command
  }
}

// Turn left
void turnLeft(int degrees) {
  turnSensorReset();
  motors.setSpeeds(-motorspeed, motorspeed);
  int angle = 0;
  do {
    delay(1);
    turnSensorUpdate();
    angle = (((int32_t)turnAngle >> 16) * 360) >> 16;
  } while (angle < degrees);
  motors.setSpeeds(0, 0);
}

// Turn right
void turnRight(int degrees) {
  turnSensorReset();
  motors.setSpeeds(motorspeed, -motorspeed);
  int angle = 0;
  do {
    delay(1);
    turnSensorUpdate();
    angle = (((int32_t)turnAngle >> 16) * 360) >> 16;
  } while (angle > -degrees);
  motors.setSpeeds(0, 0);
}
//used to detect wall/corner and set behaviour
void detectWall()
{
  sensors.read(sensorValues);   //read the raw values from sensors
  if (!checkCorner())           //if we did not detect a wall
  {
    //if the leftmost sensor detects line
    if (sensorValues[0] > calibratedValue[0])
    {
      motors.setSpeeds(-motorspeed, -motorspeed);//go reverse
      delay(moveDuration);
      motors.setSpeeds(motorspeed, -motorspeed);//turn right
      delay(moveDuration);
      motors.setSpeeds(motorspeed, motorspeed);
    }
    //if the rightmost sensor detects line
    if (sensorValues[2] >=  calibratedValue[2])
    {
      motors.setSpeeds(-motorspeed, -motorspeed);
      delay(moveDuration);
      motors.setSpeeds(-motorspeed, motorspeed);
      delay(moveDuration);
      motors.setSpeeds(motorspeed, motorspeed);
    }
  }
}
bool checkCorner()
{
  //check leftmost and rightmost sensors first
  if (sensorValues[0] >= calibratedValue[0] || sensorValues[2] >= calibratedValue[2])
  {
    delay(30);
    sensors.read(sensorValues);
    delay(5);
    //if middle sensor detect the line
    if (sensorValues[1] >= calibratedValue[1])
    {
      motors.setSpeeds(0, 0);
      motors.setSpeeds(-motorspeed, -motorspeed);
      delay(moveDuration);
      motors.setSpeeds(0, 0);
      // buzzer.playNote(NOTE_A(5), 200, 15);
      //check if the robot is inside a side-corridor
      if (isSideCorridor){
        isSideCorridor = false;
        Serial1.print("Exisiting side corridor number ");
        Serial1.print(corridorCounter);
        // Serial1.print(STRING_TERMINATOR);
        delay(10);
        message = "Zumo is at corridor number ";
        Serial1.print(message + previousCorridor);
        // Serial1.print(STRING_TERMINATOR);
        delay(10);
        previousCorridor = 0;   //reset the previous corridor
      }
      else if (isAtEnd)
      {
        isSideCorridor = true;
        isAtEnd = false;
        Serial1.print("End of side-corridor number ");
        Serial1.print(corridorCounter);
        // Serial1.print(STRING_TERMINATOR);
        delay(10);
      }
      else
      {
        Serial.print("Corner ahead. Manual mode activated!");    //Display a message showing a corner has been found
        delay(10);
        corridorCounter++;
        message = "Zumo is at corridor number ";
        Serial.print(message + corridorCounter);
        // Serial.print(STRING_TERMINATOR);
        delay(10);
      }
      zumoMove(); // move the zumo manually
      return true;
    }
    return false;
  }
}

void scanRoom()
{
  objectFound = false;
  motors.setSpeeds(100, 100);
  delay(170);
  proxSensors.read();
  //turn right
  for (int i = 0; i < 24 && objectFound == false; i++)
  {
    motors.setSpeeds(motorspeed, -motorspeed);
    delay(40);
    motors.setSpeeds(0, 0);
    //scan for object
    if (proxSensors.countsLeftWithLeftLeds() > 0 || proxSensors.countsFrontWithLeftLeds() > 0 ||
        proxSensors.countsFrontWithRightLeds() > 0 || proxSensors.countsRightWithRightLeds()>0)
    {
      //send message to GUI
      personFoundMessage();
      break;
    }
  }
  //turn left similar to 180 degrees
  for (int i = 0; i < 48 && objectFound == false; i++)
  {
    motors.setSpeeds(-motorspeed, +motorspeed);
    delay(40);
    motors.setSpeeds(0, 0);
    if (proxSensors.countsLeftWithLeftLeds() > 0 || proxSensors.countsFrontWithLeftLeds() > 0 ||
        proxSensors.countsFrontWithRightLeds() > 0 || proxSensors.countsRightWithRightLeds()>0)
    {
      personFoundMessage();
      break;
    }
  }
  if (objectFound == false)
  {
    Serial1.print("No object detected!");
    delay(10);
  }
  Serial1.print("Driving outside the room!");
  delay(10);
  // zumoMove(); // move the zumo manually
}
//used to reset global variables and communicate with GUI
void personFoundMessage()
{
  delay(5);
  objectFound = true;
  roomScan = false;
  message = "Found a person at room ";
  Serial1.print(message + roomCounter);
  // Serial1.print(STRING_TERMINATOR);
  delay(10);
  buzzer.play("! V10 cdefgab>cbagfedc");
}
