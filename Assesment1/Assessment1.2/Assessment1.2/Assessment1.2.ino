#include <L3G.h>


/*
******** LIBRARIES ********
*/

#include <Zumo32U4.h>
#include <QTRSensors.h>
#include <Zumo32U4Buzzer.h>
#include <Wire.h>
#include "TurnSensor.h"

Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors sensors;
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
      zumoMove(); // move the zumo manually
      return true;
    }
    return false;
  }
}
