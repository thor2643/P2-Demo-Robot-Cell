/*
  Example sketch to control a stepper motor with A4988 stepper motor driver, 
  AccelStepper library and Arduino: number of steps or revolutions. 
  More info: https://www.makerguides.com 
*/

// Include the AccelStepper library:
#include "AccelStepper.h"

// Motor interface type must be set to 1 when using a the A4988 driver or similar
#define motorInterfaceType 1

// Define stepper motor connections and motor interface type. 
#define stepPin_11 5
#define dirPin_11 22
#define enablePin_11 23

#define stepPin_12 3
#define dirPin_12 24
#define enablePin_12 25

#define stepPin_13 4
#define dirPin_13 26
#define enablePin_13 27

#define stepPin_21 8
#define dirPin_21 30
#define enablePin_21 31

#define stepPin_22 9
#define dirPin_22 32
#define enablePin_22 33

#define stepPin_23 10
#define dirPin_23 34
#define enablePin_23 35


// Create a new instance of the AccelStepper class:
/*
AccelStepper stepper_11 = AccelStepper(motorInterfaceType, stepPin_11, dirPin_11);
AccelStepper stepper_12 = AccelStepper(motorInterfaceType, stepPin_12, dirPin_12);
AccelStepper stepper_13 = AccelStepper(motorInterfaceType, stepPin_13, dirPin_13);
AccelStepper stepper_21 = AccelStepper(motorInterfaceType, stepPin_21, dirPin_21);
AccelStepper stepper_22 = AccelStepper(motorInterfaceType, stepPin_22, dirPin_22);
AccelStepper stepper_23 = AccelStepper(motorInterfaceType, stepPin_23, dirPin_23);

AccelStepper top_cover_motors[3] = {stepper_11, stepper_12, stepper_13};
AccelStepper bot_cover_motors[3] = {stepper_21, stepper_22, stepper_23};
*/

AccelStepper* top_cover_motors[3] = {0};
AccelStepper* bot_cover_motors[3] = {0};

int top_enable_pins[3] = {enablePin_11, enablePin_12, enablePin_13}; 
int bot_enable_pins[3] = {enablePin_21, enablePin_22, enablePin_23};

bool run = true;
bool top_done = false;
bool bot_done = false;
const int steps_per_revolution = 2048;
signed int steps_per_dispense = 10 * steps_per_revolution; //10 pr. dispense
signed int step_speed = 1500;
int step_acc = 500;



// A list with the double digits that are allowed, and the size of the list
int valid_combinations[9] = {11, 12, 13, 21, 22, 23, 31, 32, 33};
int valid_combinations_len = 9;

// An int and an int array, which will hold the motor_num being used, and the digits in the double digit
int motor_Num;
int digits[2];



void setup() {
    top_cover_motors[0] = new AccelStepper(motorInterfaceType, stepPin_11, dirPin_11);
    top_cover_motors[1] = new AccelStepper(motorInterfaceType, stepPin_12, dirPin_12);
    top_cover_motors[2] = new AccelStepper(motorInterfaceType, stepPin_13, dirPin_13);

    bot_cover_motors[0] = new AccelStepper(motorInterfaceType, stepPin_21, dirPin_21);
    bot_cover_motors[1] = new AccelStepper(motorInterfaceType, stepPin_22, dirPin_22);
    bot_cover_motors[2] = new AccelStepper(motorInterfaceType, stepPin_23, dirPin_23);

    // Set the maximum speed in steps per second:
    for (int i=0; i<3; i++){
        top_cover_motors[i]->setMaxSpeed(step_speed);
        bot_cover_motors[i]->setMaxSpeed(step_speed);

        top_cover_motors[i]->setAcceleration(step_acc);
        bot_cover_motors[i]->setAcceleration(step_acc);

        pinMode(top_enable_pins[i], OUTPUT);
        pinMode(bot_enable_pins[i], OUTPUT);

        // Disable all motors to avoid unnecessary current draw and noise.
        // Enabled when used.
        digitalWrite(top_enable_pins[i], HIGH);
        digitalWrite(bot_enable_pins[i], HIGH);
    }

    // Start serial communication
    Serial.begin(9600);
}

void loop() { 
    // Wait for a doubledigit to be received over the serial port. It is made into an integer
    if (Serial.available()) {
        motor_Num = Serial.parseInt();
        
        // As int casting rounds down this method works well to get the first digit. Second digit is found with modulus.
        digits[0] = (int)(motor_Num / 10);
        digits[1] = motor_Num % 10;

        // It is checked with the double digits is valid, meaning that is is contained within the combination list. If it is, 
        // then the digits will be passed on
        if (Is_Valid_Input(motor_Num)) {
            Stepper_Drive(digits[0], digits[1]);
            Serial.println("FINISHED");
        } else {
            Serial.println("ERROR");
        }

    }
}

// This function will use the split up double digit to drive the dispensing sequence by the requested stepper motors 
void Stepper_Drive(int top_motor_num, int bot_motor_num){
    int top_idx = top_motor_num - 1;
    int bot_idx = bot_motor_num - 1;

    // Activate respective motors
    digitalWrite(top_enable_pins[top_idx], LOW);
    digitalWrite(bot_enable_pins[bot_idx], LOW);

    //Serial.println(top_idx);
    //Serial.println((void)top_cover_motors[top_idx]);
    
    // Push the cover out and retreat
    for (int i=0; i<2; i++){
        // Will switch the sign for the two iterations. Push (-) and pull (+)
        step_speed = step_speed * -1;
        steps_per_dispense = steps_per_dispense * -1;


        // Firsly the speed is set for both the motors.
        //top_cover_motors[top_idx]->setSpeed(step_speed);
        //bot_cover_motors[bot_idx]->setSpeed(step_speed);

        // Set the current position to 0:
        top_cover_motors[top_idx]->setCurrentPosition(0);
        bot_cover_motors[bot_idx]->setCurrentPosition(0);

        // Firsly the speed is set for both the motors.
        top_cover_motors[top_idx]->moveTo(steps_per_dispense);
        bot_cover_motors[bot_idx]->moveTo(steps_per_dispense);

        top_done = false;
        bot_done = false;

        // Run the motors
        while(true)
        {   
            if (top_cover_motors[top_idx]->currentPosition() == steps_per_dispense){
                top_done = true;        
            } else {
                //top_cover_motors[top_idx]->setSpeed(step_speed);
                //top_cover_motors[top_idx]->runSpeed();
                top_cover_motors[top_idx]->run();
            }

            if (bot_cover_motors[bot_idx]->currentPosition() == steps_per_dispense){
                bot_done = true;        
            } else {
                //bot_cover_motors[bot_idx]->setSpeed(step_speed);
                //bot_cover_motors[bot_idx]->runSpeed();
                bot_cover_motors[bot_idx]->run();
            }

            if (top_done && bot_done){
                break;
            }
        }

        if (i==0){
            Serial.println("OUT");
        }

        delay(100);
    }

    // Deactivate respective motors
    digitalWrite(top_enable_pins[top_motor_num - 1], HIGH);
    digitalWrite(bot_enable_pins[bot_motor_num - 1], HIGH);
}

// This boolean function checks if the double digits are valid i.e. contained within the array.
bool Is_Valid_Input(int numb){
  for (int i = 0; i < valid_combinations_len; i++) {
    if (numb == valid_combinations[i]) {
      return true; // Return true if the number is found in the list
    }
  }
  return false; // Return false if the number is not found in the list
}