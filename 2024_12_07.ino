#include <AFMotor.h>

// Define the motors
AF_DCMotor mL(4);  // Left motor
AF_DCMotor mR(2);  // Right motor

// Define the IR sensor pins
const int irPins[5] = { A5, A4, A3, A2, A1 };  // Array to hold the IR sensor pins
int irSensorAnalog[5] = { 0, 0, 0, 0, 0 };     // Array to store the analog readings from the sensors
int threshold = 700;                           // Threshold value for IR sensor detection

// Directional and state variables
bool isLeft = false;
bool isRight = false;
bool isForward = false;
bool finish = false;

// Variables for handling turnback logic
int turnbackcount = 0;
int turnbackcountlimit = 5;
int fastspeed = 160;  // Speed for fast movement
int slowspeed = 150;  // Speed for slow movement
int delaytime = 5;    // Delay time in milliseconds

// Variables for serial input handling
String SerialInput = "";           // Initialize a string to store incoming data
int var1 = 0, var2 = 0, var3 = 0;  // Variables for '1 left_deadend', '2 straight_deadend', '3 right_deadend'
int varA = 0, varB = 0, varC = 0;  // Variables for 'a left_finish', 'b straight_finish', 'c right_finish'

// LED pin definitions
int Led1 = 2, Led2 = 7, Led3 = 9;     // Variables for '1', '2', '3'
int LedA = 10, LedB = 13, LedC = A0;  // Variables for 'a', 'b', 'c'

// Path-related variables
int letter = 0;
String path = "";
String sortPath = "";

void setup() {
  Serial.begin(9600);  // Start serial communication

  // Initialize IR sensor pins as input
  for (int i = 0; i < 5; i++) {
    pinMode(irPins[i], INPUT);
  }

  // Initialize LED pins as output
  pinMode(Led1, OUTPUT);
  pinMode(Led2, OUTPUT);
  pinMode(Led3, OUTPUT);
  pinMode(LedA, OUTPUT);
  pinMode(LedB, OUTPUT);
  pinMode(LedC, OUTPUT);
}

void loop() {
  // Check if there is data available on the serial port
  if (Serial.available() > 0) {
    char singleChar = Serial.read();  // Read one character from Serial

    // Update variables based on character
    // switch (singleChar) {
    //   case '1':
    //     var1 = 1;
    //     digitalWrite(Led1, HIGH);
    //     break;
    //   case '2':
    //     var2 = 1;
    //     digitalWrite(Led2, HIGH);
    //     break;
    //   case '3':
    //     var3 = 1;
    //     digitalWrite(Led3, HIGH);
    //     break;
    //   case 'a':
    //     varA = 1;
    //     digitalWrite(LedA, HIGH);
    //     break;
    //   case 'b':
    //     varB = 1;
    //     digitalWrite(LedB, HIGH);
    //     break;
    //   case 'c':
    //     varC = 1;
    //     digitalWrite(LedC, HIGH);
    //     break;
    //   default:
    //     break;
    // }
    if (singleChar == '1') {
      var1 = 1;
      digitalWrite(Led1, HIGH);
    } else if (singleChar == '2') {
      var2 = 1;
      digitalWrite(Led2, HIGH);
    } else if (singleChar == '3') {
      var3 = 1;
      digitalWrite(Led3, HIGH);
    } else if (singleChar == 'a') {
      varA = 1;
      digitalWrite(LedA, HIGH);
    } else if (singleChar == 'b') {
      varB = 1;
      digitalWrite(LedB, HIGH);
    } else if (singleChar == 'c') {
      varC = 1;
      digitalWrite(LedC, HIGH);
    }
  }
  // Read IR sensor values and print them
  for (int i = 0; i < 5; i++) {
    irSensorAnalog[i] = analogRead(irPins[i]);
    Serial.print(irSensorAnalog[i]);
    Serial.print("  ");
  }
  Serial.print(letter);
  Serial.print("    ");
  Serial.print(path[letter]);
  Serial.print("    ");
  Serial.println(path);

  // Handle various conditions based on sensor readings and state variables
  // Turn back on 3 deadends
  if (var1 == 1 && var2 == 1 && var3 == 1) {
    turnright();
    delay(delaytime);
    if (!path.endsWith("B")) {
      path += "B";
    }
  } else if (turnbackcount == turnbackcountlimit) {
    if (irSensorAnalog[1] > threshold) {
      turnbackcount = 0;
    }
    turnright();
    delay(delaytime);
    if (!path.endsWith("B")) {
      path += "B";
    }
  } else if ((irSensorAnalog[0] > threshold || irSensorAnalog[4] > threshold) && (varA == 1 || varB == 1 || varC == 1)) {
    if (varA) {
      isLeft = true;
      turnbackcount = 0;
      if (!path.endsWith("L")) {
        path += "L";
      }
      pinLow();
    } else if (varB) {
      forward();
      delay(delaytime);
      turnbackcount = 0;
      if (!path.endsWith("S") && ((irSensorAnalog[0] > threshold) || (irSensorAnalog[4] > threshold))) {
        path += "S";
      }
      pinLow();
    } else if (varC) {
      isRight = true;
      if (!path.endsWith("R") && (turnbackcount == 0)) {
        path += "R";
      }
      turnbackcount = 0;
      pinLow();
    }
  }


  if (isLeft && var1 == 0) {
    pinLow();
    if (irSensorAnalog[0] > threshold) {
      turnleft();
      delay(delaytime);
      turnbackcount = 0;
    } else if (irSensorAnalog[1] > threshold) {
      isLeft = false;
    }
    turnleft();
    delay(delaytime);
    turnbackcount = 0;
  } else if ((irSensorAnalog[0] > threshold) && (irSensorAnalog[1] > threshold) && var1 == 0) {
    isLeft = true;
    turnbackcount = 0;
    if (!path.endsWith("L")) {
      path += "L";
    }
    pinLow();
  } else if (irSensorAnalog[2] > threshold && var2 == 0) {
    pinLow();
    forward();
    delay(delaytime);
    turnbackcount = 0;
    if (!path.endsWith("S") && ((irSensorAnalog[0] > threshold) || (irSensorAnalog[4] > threshold))) {
      path += "S";
    }
  } else if (isRight && var3 == 0) {
    pinLow();
    if (irSensorAnalog[4] > threshold) {
      turnright();
      delay(delaytime);
      turnbackcount = 0;
    } else if (irSensorAnalog[3] > threshold) {
      isRight = false;
    }
    turnright();
    delay(delaytime);
    turnbackcount = 0;
  } else if ((irSensorAnalog[4] > threshold) && (irSensorAnalog[3] > threshold) && var3 == 0) {
    pinLow();
    isRight = true;
    if (!path.endsWith("R") && (turnbackcount == 0)) {
      path += "R";
    }
    turnbackcount = 0;
  } else if ((irSensorAnalog[0] < threshold) && (irSensorAnalog[1] < threshold) && (irSensorAnalog[2] < threshold) && (irSensorAnalog[3] < threshold) && (irSensorAnalog[4] < threshold)) {
    turnbackcount++;
  } else if ((irSensorAnalog[1] < threshold) && (irSensorAnalog[3] < threshold)) {
    forward();
    delay(delaytime);
    turnbackcount = 0;
  } else if (irSensorAnalog[1] > threshold) {
    left();
    delay(delaytime);
    turnbackcount = 0;
  } else if (irSensorAnalog[3] > threshold) {
    right();
    delay(delaytime);
    turnbackcount = 0;
  }
}

// Functions for motor movement
void forward() {
  mL.setSpeed(fastspeed);
  mR.setSpeed(fastspeed);

  mL.run(FORWARD);
  mR.run(FORWARD);
}

void left() {
  mL.setSpeed(0);
  mR.setSpeed(fastspeed);

  mL.run(RELEASE);
  mR.run(FORWARD);
}

void turnleft() {
  mL.setSpeed(slowspeed);
  mR.setSpeed(fastspeed);

  mL.run(BACKWARD);
  mR.run(FORWARD);
}

void right() {
  mL.setSpeed(fastspeed);
  mR.setSpeed(0);

  mL.run(FORWARD);
  mR.run(RELEASE);
}

void turnright() {
  mL.setSpeed(fastspeed);
  mR.setSpeed(slowspeed);

  mL.run(FORWARD);
  mR.run(BACKWARD);
}

void stop() {
  mL.setSpeed(0);
  mR.setSpeed(0);

  mL.run(RELEASE);
  mR.run(RELEASE);
}
void pinLow() {
  digitalWrite(Led1, LOW);
  digitalWrite(Led2, LOW);
  digitalWrite(Led3, LOW);
  digitalWrite(LedA, LOW);
  digitalWrite(LedB, LOW);
  digitalWrite(LedC, LOW);
  var1 = var2 = var3 = varA = varB = varC = 0;
}

String shortPath() {
  path.replace("LBL", "S");
  path.replace("LBS", "R");
  path.replace("RBL", "B");
  path.replace("SBS", "B");
  path.replace("SBL", "R");
  path.replace("LBR", "B");
  return path;
}
