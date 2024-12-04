#include <SoftwareSerial.h>

//Define Pins on the Arduino
const int TX2 = 2; //Define RX pin used for Touchpad Coms
const int RX2 = 7; //Define TX pin used for Touchpad Coms
const int dirPIN = 12;
const int enablePIN = 11;
const int DEenable = 6; //Driver Output Enable on MAX485. HIGH to Enable
const int REenable = 4; //Reciever Output Enable on MAX485. LOW to Enable
const int input1 = 8;
int myTurn = 1;

uint32_t number = 1;
uint8_t endBytes[] = {0xff, 0xff, 0xff}; //Set the ending characters that need to be sent when sending things to touchpad per Nextion User Guide
String dfd = ""; //Data from display
int incomingByte = 0;
uint8_t enable = 0;
uint8_t direction = 0;
const int steps = 300;
const unsigned long defaultTimeout = 500;


SoftwareSerial Serial2(RX2, TX2); // RX, TX Software Serial Object. Pins 7 and 2 are used for UART now

void setup() {
  Serial.begin(9600); //Begin Serial to debug with computer over USB
  Serial1.begin(38400); //Begin Serial1 on RX and TX pins to communicate with Motor. Serial1 is hardware Serial. Always set to TX/RX pins
  Serial2.begin(9600); //begin Software Serial on Pins 7 and 2 to talk to the Touch Screen
  delay(1000);        // Wait for Serial to Begin

  pinMode(DEenable, OUTPUT);
  pinMode(REenable, OUTPUT);
  pinMode(input1, OUTPUT);
  digitalWrite(input1, HIGH);
  digitalWrite(DEenable, LOW); //Disable data being sent to motor
  digitalWrite(REenable, LOW); //Enable data to be sent to Arduino

  // configurePage();
  delay(1000);
  enable = 0;

  //  // Set initial motor parameters
  // setMotorParameters();

  // // Spin the motor clockwise for the specified number of steps
  // spinClockwise(steps);
  // delay(defaultTimeout);  // Wait for the motion to complete

  // // Stop the motor
  // stopMotion();
  // delay(defaultTimeout);  // Wait before next operation

  // // Spin the motor counterclockwise for the specified number of steps
  // spinCounterClockwise(steps);
  // delay(defaultTimeout);  // Wait for the motion to complete

  // // Stop the motor again
  // stopMotion();
}


void loop() {
  // digitalWrite(input1, LOW);
  // delay(2000);
  // digitalWrite(input1, HIGH);
  // delay(3000);
  // if(direction == 0){
  //   //Serial1.print("START CCW 200\r");
  //   //Serial1.write('\r');
  //   delay(300);
  //   direction = 1;
  //   // digitalWrite(input1, LOW);
  //   // delay(2000);
  //   // digitalWrite(input1, HIGH);
  // } else {
  //   //Serial1.print("START CW 200\r");
  //   //Serial1.write('\r');
  //   delay(300);
  //   direction = 0;
  //   // digitalWrite(input1, LOW);
  //   // delay(2000);
  //   // digitalWrite(input1, HIGH);
  // }
  //digitalWrite(input1, LOW);
  // Serial1.print("t1.txt=");
  // Serial1.print(String("\"go\""));
  // Serial1.write(endBytes, sizeof(endBytes));
  // delay(200);
  // Serial1.print("get t1.txt");
  // Serial1.write(endBytes, sizeof(endBytes));
  //Serial.print("C:CCTR?");
  // if(Serial1.available() >= 0){
  //   //digitalWrite(LED_BUILTIN, HIGH);
  //   //recieveInput();
  // } else {}
  delay(5000);
  if(myTurn == 1){
    digitalWrite(REenable, HIGH); //Disable incoming data from MAX485
    digitalWrite(DEenable, HIGH); //Enable outgoing data from MAX485
    digitalWrite(input1, HIGH);
    // Serial1.println("@0A1_100000");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.println("@0B1_1000");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0M1_4000");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0N1_500");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0T1_100");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0R8");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0+");
    // Serial1.write("\r");
    // delay(500);
    // Serial1.println("@0G1");
    // Serial1.write("\r");
    // delay(500);
    Serial.print("I sent");

    Serial1.print("@1$");
    Serial1.write('\r');
    delay(500);
    digitalWrite(DEenable, LOW); //Disable outgoing data from MAX485
    digitalWrite(REenable, LOW); //Enable incoming data from MAX485
    digitalWrite(input1, LOW);
    //digitalWrite(enablePIN, HIGH);
    myTurn = 0;
  }
  while(Serial1.available()){
    incomingByte = Serial1.read();
    Serial.print("I received: ");
    Serial.println(incomingByte, HEX);
    myTurn = 1;
  }

  delay(150);
  // if (enable ==1) {

  //   // read the incoming byte:
  //   incomingByte = Serial1.read();

  //   // say what you got:
  // }
  // delay(1000);
}

void setMotorParameters() {
  // Send commands to set motor acceleration, base speed, max speed, etc.
  Serial1.print("@0A1_100000");  // Set acceleration
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0B1_1000");  // Set base speed
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0M1_4000");  // Set max speed
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0N1_");  // Set index number (steps to move)
  Serial1.print(steps);
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0T1_100");  // Set complete time
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0R8");  // Set microstep resolution (optional)
  Serial1.println();
  delay(defaultTimeout);
}

// Function to spin the motor clockwise
void spinClockwise(int numSteps) {
  Serial1.print("@0+");  // Set direction clockwise
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0G1");  // Start the motor (Index 1)
  Serial1.println();
  delay(defaultTimeout);
}

// Function to spin the motor counterclockwise
void spinCounterClockwise(int numSteps) {
  Serial1.print("@0-");  // Set direction counterclockwise
  Serial1.println();
  delay(defaultTimeout);

  Serial1.print("@0G1");  // Start the motor (Index 1)
  Serial1.println();
  delay(defaultTimeout);
}

// Function to stop the motor (hard limit)
void stopMotion() {
  Serial1.print("@0H");  // Stop motion (Hard Limit)
  Serial1.println();
  delay(defaultTimeout);
}

void configurePage(){
  Serial2.write(0x70);
  Serial2.write(0x61);
  Serial2.write(0x67);
  Serial2.write(0x65);
  Serial2.write(0x20);
  Serial2.write(0x30);
  //Serial2.print("page 0");
  Serial2.write(endBytes, sizeof(endBytes));
}

void recieveInput() {
  dfd += char(Serial.read());
  if(dfd.length()>3 && dfd.substring(0,3) !="C:C") {
    dfd=""; //Throw away junk RX data for right now
  } else {
    if(dfd.substring((dfd.length()-1),dfd.length()) == "?") { //Confirms it is a valid command
      String message = dfd.substring(3,6);
      //Serial.print(message);
      //Serial.print("t1.txt=");
      //Serial.print(String("\"go\""));
      //Serial.write(endBytes, sizeof(endBytes));
    }
    //if(){ //Confirms it is a value being sent
        //Do Stuff with the value I recieve after I send a get function
    //}
  }
}