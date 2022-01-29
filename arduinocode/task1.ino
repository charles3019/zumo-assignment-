
/*
******** LIBRARIES ********
*/

#include <Zumo32U4.h>
#include <Wire.h>

Zumo32U4Motors motors;

/*
******** VARIABLES ********
*/
const uint16_t moveDuration = 1000;
// bool start = false; 
int motorspeed = 200;
/*
******** setup FUNCTION ********
*/

void setup() {
  Serial1.begin(9600);
  delay(1500);

}

/*
******** loop FUNCTION ********
*/

void loop() {
  if (Serial1.read() == 'w'){
    manualMove(motorspeed, motorspeed);
    }
  else if(Serial1.read() == 'a'){
    manualMove(-motorspeed, -motorspeed);
  }
  else if(Serial1.read() == 'l'){
    manualMove(-motorspeed, motorspeed);
  }
  else if(Serial1.read() == 'r'){
    manualMove(motorspeed, -motorspeed);
  }
}


void manualMove(int speedl, int speedr){
  motors.setSpeeds(speedl, speedr);
  delay(moveDuration);
  motors.setSpeeds(0, 0);
}
