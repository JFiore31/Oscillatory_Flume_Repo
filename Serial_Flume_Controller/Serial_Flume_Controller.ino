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
uint8_t enable = 0; //Can take a value of 0 for off and 1 for on
uint8_t direction = 0; //Can take a value of 0 for CW and 1 for CCW
const int steps = 300; //The amount of steps the motor has to turn
const unsigned long defaultTimeout = 500;
int speed = 0; //Speed of wave sent from Nextion
int frequency = 0; //Frequency of wave sent from Nextion
int amplitude = 0; //Amplitude of wave sent from Nextion
int waveType = 0; //Type of wave sent from Nextion
int speedDecs = 0; //How many decimals the speed value has when sent from Nextion. Can either be 0 or 3.
int ampDecs = 0; //How many decimals the amplitude value has when sent from Nextion. Can either be 1 or 2.
int freqDecs = 0; //How many decimals the frequency value has when sent from Nextion. Can either be 3 or 4.
int readingValue = 0; //Determines which value (speed, amplitude, frequency, or run type) the Nextion just sent. 0=speed, 1=amp, 2=freq, 3=waveType


SoftwareSerial Serial2(RX2, TX2); // RX, TX Software Serial Object. Pins 7 and 2 are used for UART now

void setup() {
  Serial.begin(9600); //Begin Serial to debug with computer over USB
  Serial1.begin(38400); //Begin Serial1 on RX and TX pins to communicate with Motor. Serial1 is hardware Serial. Always set to TX/RX pins
  Serial2.begin(9600); //begin Software Serial on Pins 7 and 2 to talk to the Touch Screen
  delay(1000);        // Wait for Serial to Begin

  pinMode(DEenable, OUTPUT);
  pinMode(REenable, OUTPUT);
  pinMode(input1, OUTPUT); //I do not know what input1 really does at this point
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

  //Check for updates from stepper motor
  while(Serial1.available()){ //Only entered if there is data on the Hardware Serial RX FIFO
    incomingByte = Serial1.read(); //Read a single byte from Stepper Motor and save it to variable incomingByte
    // Serial.print("I received: ");
    // Serial.println(incomingByte, HEX);
    //myTurn = 1;

  }

  //Check for updates from Touchpad
  while(Serial2.available()){ //Only entered if there is data on the Software Serial RX FIFO (Pin 7)
    //Data should only be here in 3 known scenarios: 1. Touchpad startup (sends "00 00 00 FF FF FF \n 88 FF FF FF") 2. Motor commanded to turn on 3. Motor commanded to turn off
    //All other data on the pin will be thought of as noise

    //Data will be sent to uC in this order when a run is confirmed:
    //1) "CR=x" where x is the Confirm Run number that is active. This will determine how many decimal points the values contain.
    //2) Speed in [cm/s]
    //3) Amplitude in [cm]
    //4) Frequency in [Hz]
    //5) Wave Type: Sinusoid = 0, Square = 1
    //6) "Enable" to turn the motor on

    //Data will be sent to uC in this order when a run is stopped:
    //1) "Dis" to turn motor off

    //After each message sent to the uC, there are 3 end charcaters to signify the end of a command has been transmitted. They are "C:C"
    //Note that Nextion sends text content is sent 1 ASCII byte per character, without null byte.
    //Note that Nextion sends numeric value in 4 byte 32-bit little endian order. value = byte1+byte2*256+byte3*65536+byte4*16777216


    //Check is "CR=" is present to start
    //if not, check is "Dis" is present to start
    //if neither, junk data. Throw out
    dfd += char(Serial2.read());
    if(dfd.length()>3 && dfd.substring(0,3) =="CR=") {
      //The motor is being told to turn on. Get wave parameters
      dfd=""; //clear dfd for repurposing
      dfd += char(Serial2.read());
      if(dfd == 1){
        //Confirm Run Type 1 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 0;
        ampDecs = 1;
        freqDecs = 3;
        dfd="";
        readingValue = 0;
        readParams();
      }else if(dfd == 2) {
        //Confirm Run Type 2 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 3;
        ampDecs = 1;
        freqDecs = 3;
        dfd="";
        readingValue = 0;
        readParams();
      }else if(dfd == 3){
        //Confirm Run Type 3 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 0;
        ampDecs = 2;
        freqDecs = 4;
        dfd="";
        readingValue = 0;
        readParams();
      } //There are not other possibilities for dfd to be besides 1, 2, or 3 as CHARs here. No catch statement
    } else if(dfd.length()>3 && dfd.substring(0,3) =="Dis") {
      //The motor is being told to turn off. Shut it down. First confirm its a valid message.
      if(dfd.substring((dfd.length()-3),dfd.length()) == "C:C") {
        //Valid message has been sent. Shut down.
        enable = 0;
        dfd="";
      }
    } else if(dfd.length()>3 && dfd.substring(0,3) !="Dis" && dfd.substring(0,3) !="CR=") {
      //The uC has been sent something that is not a turn on or turn off command. Probably noise. Neglect it.
      dfd=""; 
    }
    //incomingByte = Serial2.read(); //Read a single Byte from Touchpad and save it to variable incoming Byte
  }
  delay(150);
  // if (enable ==1) {

  //   // read the incoming byte:
  //   incomingByte = Serial1.read();

  //   // say what you got:
  // }
  // delay(1000);
}

void readParams() {
    dfd += char(Serial2.read());
    if(dfd.length()>3 && dfd.substring((dfd.length()-3),dfd.length()) == "C:C" && readingValue == 0){
      //Tells us a valid parameter has been sent and that that parameter is the speed
      String SPEED = dfd.substring(0,(dfd.length()-3)); //Save the speed value without the "C:C" at the end
      speed = SPEED.toInt(); //Cast to int so we can work with it later
      readingValue = 1; //Set parsing variable to next parameter type
      dfd=""; //Reset dfd for next param
      readParams(); //Recurse through function again until all parameters are read

    }else if(dfd.length()>3 && dfd.substring((dfd.length()-3),dfd.length()) == "C:C" && readingValue == 1){
      //Tells us a valid parameter has been sent and that that parameter is the amplitude
      String AMPLITUDE = dfd.substring(0,(dfd.length()-3)); //Save the amplitude value without the "C:C" at the end
      amplitude = AMPLITUDE.toInt(); //Cast to int so we can work with it later
      readingValue = 2; //Set parsing variable to next parameter type
      dfd=""; //Reset dfd for next param
      readParams(); //Recurse through function again until all parameters are read

    }else if(dfd.length()>3 && dfd.substring((dfd.length()-3),dfd.length()) == "C:C" && readingValue == 2){
      //Tells us a valid parameter has been sent and that that parameter is the frequency
      String FREQUENCY = dfd.substring(0,(dfd.length()-3)); //Save the frequency value without the "C:C" at the end
      frequency = FREQUENCY.toInt(); //Cast to int so we can work with it later
      readingValue = 3; //Set parsing variable to next parameter type
      dfd=""; //Reset dfd for next param
      readParams(); //Recurse through function again until all parameters are read

    }else if(dfd.length()>3 && dfd.substring((dfd.length()-3),dfd.length()) == "C:C" && readingValue == 3){
      //Tells us a valid parameter has been sent and that that parameter is the waveType
      String WAVETYPE = dfd.substring(0,(dfd.length()-3)); //Save the waveType value without the "C:C" at the end
      waveType = WAVETYPE.toInt(); //Cast to int so we can work with it later
      dfd=""; //Reset dfd for next param

      return; //All the parameters are now recorded. The RX buffer should be empty. We can return now
    } else {
      readParams(); //Catch else statement to recursively loop through again until a "C:C" is found 
    }
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