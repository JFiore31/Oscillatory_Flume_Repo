#include <SoftwareSerial.h>

//Define Pins on the Arduino
const int TX2 = 2;  //Define RX pin used for Touchpad Coms
const int RX2 = 7;  //Define TX pin used for Touchpad Coms
const int dirPIN = 12;
const int enablePIN = 11;
const int DEenable = 6;  //Driver Output Enable on MAX485. HIGH to Enable
const int REenable = 4;  //Reciever Output Enable on MAX485. LOW to Enable
const int input1 = 8;
const int profileCompleteOutput = 17; //Needs to be set to some digital pin as input

//Strings to hold data incoming from peripherals
String dfd = "";  //Data from display
String dfm = "";  //Data from motor

//variables to store motor state management
const char INITIALDIRECTION = "-";
uint8_t enable = 0;     //Can take a value of 0 for off and 1 for on
unsigned int motorAccel = 0;
unsigned int motorSpeedBase = 0;
unsigned int motorSpeedMax = 0;
unsigned int motorSteps = 0;  //The amount of steps the motor has to turn. Also called index number
unsigned int motorCompleteTime = 0;
uint8_t motorDirection = 0;  //Can take a value of 0 for CW and 1 for CCW
unsigned int checkAccel = 1;
unsigned int checkSpeedBase = 1;
unsigned int checkSpeedMax = 1;
unsigned int checkSteps = 1;
unsigned int checkCompleteTime = 1;
uint8_t checkDirection = 1;
char sendDirection = "f";


//Create a set time to allow for serial to be sent
const unsigned long DEFAULTIMEOUT = 500; //in uS. .5s

//Wave parameter variables as prescribed on Nextion
uint8_t endBytes[] = { 0xff, 0xff, 0xff };  //Set the ending characters that need to be sent when sending things to touchpad per Nextion User Guide. We will try to never send to Nextion though
unsigned int speedNex = 0;                           //Speed of wave sent from Nextion in [cm/s]
unsigned int frequencyNex = 0;                       //Frequency of wave sent from Nextion in [Hz]
unsigned int amplitudeNex = 0;                       //Amplitude of wave sent from Nextion in [cm]
uint8_t waveType = 0;                       //Type of wave sent from Nextion
uint8_t speedDecs = 0;                      //How many decimals the speed value has when sent from Nextion. Can either be 0 or 3.
uint8_t ampDecs = 0;                        //How many decimals the amplitude value has when sent from Nextion. Can either be 1 or 2.
uint8_t freqDecs = 0;                       //How many decimals the frequency value has when sent from Nextion. Can either be 3 or 4.
uint8_t readingValue = 0;                   //Determines which value (speed, amplitude, frequency, or run type) the Nextion just sent. 0=speed, 1=amp, 2=freq, 3=waveType
uint8_t confirmRunType = 0;                 //int variable so we can compare the char sent form Nextion to enter an if statement
float waveSpeed = 0.0;                      //Float to be used when math is done to keep decimal places
float waveFrequency = 0.0;                  //Float to be used when math is done to keep decimal places
float waveAmp = 0.0;                        //Float to be used when math is done to keep decimal places

//Define Physical constraints
const float horizontalAC = .00198;  //In [m^2]. 44.45 mm X 44.45mm. Note this is in the tubing, not the cross section. Cross section Ac from geometry will affect results
const float verticalAC = .0625;    //In [m^2]. 312.5 mm X 200 mm

//Create a software Serial object
SoftwareSerial Serial2(RX2, TX2);  // RX, TX Software Serial Object. Pins 7 and 2 are used for UART now

void setup() {
  Serial.begin(9600);    //Begin Serial to debug with computer over USB
  Serial1.begin(38400);  //Begin Serial1 on RX and TX pins to communicate with Motor. Serial1 is hardware Serial. Always set to TX/RX pins
  Serial2.begin(9600);   //begin Software Serial on Pins 7 and 2 to talk to the Touch Screen
  delay(1000);           // Wait for Serial to Begin

  pinMode(DEenable, OUTPUT);
  pinMode(REenable, OUTPUT);
  pinMode(profileCompleteOutput, INPUT); //Pin that will tell if the motion profile has been completed by the motor. 
  pinMode(input1, OUTPUT);  //I do not know what input1 really does at this point
  digitalWrite(input1, HIGH);
  digitalWrite(DEenable, LOW);  //Disable data being sent to motor
  digitalWrite(REenable, LOW);  //Enable data to be sent to Arduino

  // configurePage(); //We may run into errors later where if the touchpad looses power the uC and touchpad will not be in sync anymore which would be bad. Neglect for right now
  delay(1000);
  enable = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                                                                                                                  /////////
//////////                                                     ~END OF Setup~                                                               /////////
//////////                                                                                                                                  /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //Loop does 2 things:
  //1) Checks for new data from Nextion. This will either be a stop command or a go command with the new parameters to go at. We want to constantly check for updates
  //2) If we are running, send the motor the opposite direction to run after it completes its motion profile. This allows us to repeatedly go up and down with the piston
  //   the motor will tell us when it completes a motion profile. So, it keeps time we need to get accurate wave frequency as we set its "Complete Time"
  //   we do not use interrupts at all here. We reley on once the motor starts spinning, the uC CPU is fast enough to accurately identify and respond to the motor saying
  //   it is done and ready to go back in the opposite direction. This prevents Serial.print statements from being interfered with inadvertadly

  //No interrupts will be attached in this sketch. We will just use loop() constantly going through to find changes. May not be the most robust...
  //function to send the serial commands change the motor profile direction. Only entered when enable is set and the motor responds with a LOW signal that it has completed previous motion plan
  //This statement will be doing something the most and must be constantly checked.
  if (enable == 1 && profileCompleteOutput == LOW) {
    if(motorDirection == 0){ //indicates the last motion was CCW (piston down, -)
      sendDirection = "+"; //change to move piston up with CW motion
      motorDirection = 1;
    } else if(motorDirection == 1){ //indicates that the last motion was CW (piston up, +)
      sendDirection = "-"; //change to move piston down with CCW motion
      motorDirection = 0;
    }
    digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
    digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485

    Serial1.println("@0"); 
    Serial1.write(sendDirection); //Send the motor a new direction to run in. Note: The motor can only change its motion profile params when not in motion (indexer not running) like it is here
    Serial1.write("\r");
    delay(250); //This will cause some errors in the piston frequency, but neglect them. Esentially, the piston will wait at top of bottom for an extra .25s each time, but ensures communcations are sent correctly

    Serial1.println("@0G1"); //Now with direction changed, send the Go commands again. This will pull profileCompleteOutput back to HIGH
    Serial1.write("\r");

    digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
    digitalWrite(REenable, LOW);  //Enable incoming data from MAX485

    //We are not checking if the motor recieved and properly interpretted the direction change here. Not very safe, but saves time. Try it for now
  }

  //Check for updates from Touchpad. This must constantly be checked, but will only do something when we have new data (only on startup and turn off)
  if (Serial2.available()) {  //Only entered if there is data on the Software Serial RX FIFO (Pin 7). It will be sent all at once
    //Data should only be here in 3 known scenarios: 1. Touchpad startup (sends "00 00 00 FF FF FF \n 88 FF FF FF") 2. Motor commanded to turn on 3. Motor commanded to turn off
    //All other data on the pin will be thought of as noise

    //Data will be sent to uC in this order when a run is confirmed:
    //1) "CR=x" where x is the Confirm Run number that is active. This will determine how many decimal points the values contain.
    //2) Speed in [cm/s] + "C:C"
    //3) Amplitude in [cm] + "C:C"
    //4) Frequency in [Hz] + "C:C"
    //5) Wave Type: Sawtooth = 0, Square = 1 + "C:C"

    //Data will be sent to uC in this order when a run is stopped:
    //1) "Dis" to turn motor off + "C:C"

    //After each message sent to the uC, there are 3 end charcaters to signify the end of a command has been transmitted. They are "C:C"
    //Note that Nextion sends text content is sent 1 ASCII byte per character, without null byte.
    //Note that Nextion sends numeric value in 4 byte 32-bit little endian order. value = byte1+byte2*256+byte3*65536+byte4*16777216


    //Check is "CR=" is present to start
    //if not, check is "Dis" is present to start
    //if neither, junk data. Throw out.
    //To do this, we read the first 3 bytes in FIFO buffer
    dfd += char(Serial2.read());
    dfd += char(Serial2.read());
    dfd += char(Serial2.read());
    if (dfd.length() > 3 && dfd.substring(0, 3) == "CR=") {
      //The motor is being told to turn on. Get wave parameters
      dfd = "";                     //clear dfd for repurposing
      dfd += char(Serial2.read());  //Read one more char
      confirmRunType = dfd.toInt();
      if (confirmRunType == 1) {
        //Confirm Run Type 1 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 0;
        ampDecs = 1;
        freqDecs = 3;
        dfd = "";
        readingValue = 0;
        readParams();
      } else if (confirmRunType == 2) {
        //Confirm Run Type 2 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 3;
        ampDecs = 1;
        freqDecs = 3;
        dfd = "";
        readingValue = 0;
        readParams();
      } else if (confirmRunType == 3) {
        //Confirm Run Type 3 is present
        //Set param decimal count as determined by this Confirm Run Type
        speedDecs = 0;
        ampDecs = 2;
        freqDecs = 4;
        dfd = "";
        readingValue = 0;
        readParams();
      }  //There are not other possibilities for confirmRunType to be besides 1, 2, or 3 as Int here. No catch statement
    } else if (dfd.length() > 3 && dfd.substring(0, 3) == "Dis") {
      //The motor is being told to turn off. Shut it down. First confirm its a valid message.
      while(Serial2.available()){ 
        dfd += char(Serial2.read()); //Read all the Bytes out of the FIFO Buffer
      }
      if (dfd.substring((dfd.length() - 3), dfd.length()) == "C:C") {
        //Valid message has been sent. Shut down.
        //This will stop the piston at the end of its last motion profile (either at the bottom or top of its plunge). We want to ensure it always ends at the top though (top dead center)
        //We need the piston to always end at the top because it must always START at the top, or else we will break the machine. Check its location
        if(motorDirection != 1){
          digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
          digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485

          Serial1.println("@0S");
          Serial1.write("\r");
          delay(motorCompleteTime + 500); //ensures that the motion profile to plunge the piston down will be complete and the motor can recieve parameter changes again

          Serial1.println("@0"); 
          Serial1.write("+"); //Send the direction change to go back to the top
          Serial1.write("\r");
          delay(DEFAULTIMEOUT);
          Serial1.println("@0G1"); //Now with direction changed to send the piston up, send the Go commands again. This will pull profileCompleteOutput back to HIGH
          Serial1.write("\r");
          delay(motorCompleteTime / 4); //Put a break in the serial communication so the motor recieves it correctly, but make sure the stop command gets sent while traveling up
          Serial1.println("@0S"); //While the motor is going up, tell it to stop at the top. It can accept Soft Stop commands while moving
          Serial1.write("\r");
          digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
          digitalWrite(DEenable, LOW);  //Disnable outgoing data from MAX485
        } else {
          //Soft stop motion. It will finish current motion steps and then stop at the top
          digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
          digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
          Serial1.println("@0S");
          Serial1.write("\r");
          digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
          digitalWrite(DEenable, LOW);  //Disnable outgoing data from MAX485
        }
        enable = 0;
        dfd = "";
      }
    } else if (dfd.length() > 3 && dfd.substring(0, 3) != "Dis" && dfd.substring(0, 3) != "CR=") {
      //The uC has been sent something that is not a turn on or turn off command. Probably noise. Neglect it.
      while(Serial2.available()){ 
        dfd += char(Serial2.read()); //Read all the junk out of the FIFO Buffer
      }
      dfd = ""; //Delete all of the junk
    }
    //There may need to be a catch statement here for if the RX Buffer never has 3 Bytes in it. This should never happen though...
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                                                                                                                                  /////////
//////////                                                      ~END OF MAIN~                                                               /////////
//////////                                                                                                                                  /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Helper function that will digest hex values sent to uC from Nextion
void readParams() {
  dfd += char(Serial2.read()); //We must use recursion here because we don't know how many bytes the integer values will be. Thought, they are probably all 4 bytes
  if (dfd.length() > 3 && dfd.substring((dfd.length() - 3), dfd.length()) == "C:C" && readingValue == 0) {
    //Tells us a valid parameter has been sent and that that parameter is the speed
    String SPEED = dfd.substring(0, (dfd.length() - 3));  //Save the speed value without the "C:C" at the end
    speedNex = SPEED.toInt();                             //Cast to int so we can work with it later
    readingValue = 1;                                     //Set parsing variable to next parameter type
    dfd = "";                                             //Reset dfd for next param
    readParams();                                         //Recurse through function again until all parameters are read

  } else if (dfd.length() > 3 && dfd.substring((dfd.length() - 3), dfd.length()) == "C:C" && readingValue == 1) {
    //Tells us a valid parameter has been sent and that that parameter is the amplitude
    String AMPLITUDE = dfd.substring(0, (dfd.length() - 3));  //Save the amplitude value without the "C:C" at the end
    amplitudeNex = AMPLITUDE.toInt();                         //Cast to int so we can work with it later
    readingValue = 2;                                         //Set parsing variable to next parameter type
    dfd = "";                                                 //Reset dfd for next param
    readParams();                                             //Recurse through function again until all parameters are read

  } else if (dfd.length() > 3 && dfd.substring((dfd.length() - 3), dfd.length()) == "C:C" && readingValue == 2) {
    //Tells us a valid parameter has been sent and that that parameter is the frequency
    String FREQUENCY = dfd.substring(0, (dfd.length() - 3));  //Save the frequency value without the "C:C" at the end
    frequencyNex = FREQUENCY.toInt();                         //Cast to int so we can work with it later
    readingValue = 3;                                         //Set parsing variable to next parameter type
    dfd = "";                                                 //Reset dfd for next param
    readParams();                                             //Recurse through function again until all parameters are read

  } else if (dfd.length() > 3 && dfd.substring((dfd.length() - 3), dfd.length()) == "C:C" && readingValue == 3) {
    //Tells us a valid parameter has been sent and that that parameter is the waveType
    String WAVETYPE = dfd.substring(0, (dfd.length() - 3));  //Save the waveType value without the "C:C" at the end
    waveType = WAVETYPE.toInt();                             //Cast to int so we can work with it later
    dfd = "";                                                //Reset dfd for next param

    solveMotionPlan();
    return;  //All the parameters are now recorded. The RX buffer should be empty. We can return now
  } else {
    readParams();  //Catch else statement to recursively loop through again until a "C:C" is found
  }
}

//Helper function to do the math required to define movement
void solveMotionPlan() {
  //First, convert ints to floats to do math on them
  waveSpeed = speedNex;          //in [cm/s]
  waveFrequency = frequencyNex;  //in [Hz]
  waveAmp = amplitudeNex;        //in [cm]

  //Next, divide by the number of decimal places to get correct value
  waveSpeed = waveSpeed / (speedDecs * 10);
  waveFrequency = waveFrequency / (freqDecs * 10);
  waveAmp = waveAmp / (ampDecs * 10);

  //Perform unit transform to get into base units
  waveSpeed = waveSpeed / 100;  //conversion to [m/s]
  //Frequency is already in base units [Hz=1/s]
  waveAmp = waveAmp / 100;  //conversion to [m]

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  motorAccel = 5000000; //Both Square and Sawtooth waves have C2 discontinuity. Therefore, we want instantaneous acceleration. This is a good physical representation of that and it should break our system
  motorSpeedBase = 1; //We want the base speed to be 0, but 1 is the minimum value. With microstepping of 8, this is negligible
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Assume no leakages, so volume flow rate is conserved
  //Q=v*Ac
  //pistonSpeed = waveSpeed*(Horizontal_Cross_Section_Area/Vertical_Tank_Cross_Section_Area)
  float pistonSpeed = waveSpeed*horizontalAC/verticalAC;
  motorSpeedMax = (int)pistonSpeed; //Will always truncate down towards 0. Introduces error. Neglect
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //BLU [m/pulse] = .18 [Deg per step]*1/8 [steps per microstep]*.018m*2*Pi/360 [Deg]*1 [microstep/pulse]
  //.18 Deg per step is defined for this motor
  //.018m is the radius of the pinion gear
  //steps = waveAmp / BLU
  float BLU = .18/8*.018*2*PI/360;
  motorSteps = (int)(waveAmp/BLU); //I believe this to be the number of pulses to be sent. Truncated down introduces some error
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  float completeTime = (1 / waveFrequency) / 2 * 1000; //We want the complete time to be half the period of the wave. Sent to motor as uSeconds, so multiple by 1000
  motorCompleteTime = (int)completeTime; //Will always truncate down towards 0. Introduces error. Neglect
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Now that all math is done, send the values to motor
  sendMotorParams();
}

//Helper function to send over the prescribed data to the motor
void sendMotorParams() {
  //First, lets check to make sure the motor is not in an error state and is ready to recieve info
  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485

  Serial1.println("@0!"); //Ask motor for error register
  Serial1.write('\r');

  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485

  while(Serial1.available()) {      //Only entered if there is data on the Hardware Serial RX FIFO
    dfm += char(Serial1.read());  //Read a single byte from Stepper Motor and save it to String data from motor
  }
  Serial.println(dfm); //Print out error to computer terminal over USB connection. Can only be used in Debugging
  //Once we know what a passing message back to indicate no errors is, put the following code in an if statement to only allow its run if no error occurs
  //Potentially send error messages to a text box on Nextion to be used when no computer is connected


  //No error occured, lets get ready to send the motor its commands 
  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  //RS-485 circuit is now open to send commands. Send it the commands that have been determined

  //Note, @0 is included in every statement. This is because the motors address is set to 0
  //Note, after @0 a letter follows. This defines the parameter we are setting. Then a 1 follows. This is because we are always working with motion profile 1. There are 2 motion profiles.
  //Set Motion Acceleration
  Serial1.println("@0A1_");
  Serial1.write(motorAccel);
  Serial1.write('\r');
  delay(DEFAULTIMEOUT);
  //Set the Motion Base Speed
  Serial1.println("@0B1_");
  Serial1.println(motorSpeedBase);
  Serial1.write("\r");
  delay(DEFAULTIMEOUT);
  //Set the Motion Max Speed
  Serial1.println("@0M1_");
  Serial1.write(motorSpeedMax);
  Serial1.write("\r");
  delay(DEFAULTIMEOUT);
  //Set the motion index number(number of steps)
  Serial1.println("@0N1_");
  Serial1.write(motorSteps);
  Serial1.write("\r");
  delay(DEFAULTIMEOUT);
  //Set Motion Complete Time in uS
  Serial1.println("@0T1_");
  Serial1.println(motorCompleteTime);
  Serial1.write("\r");
  delay(DEFAULTIMEOUT);
  //Set motion uStep resolution
  Serial1.println("@0R8");  //We are always planning to use a microstepping of 8 to give best locational percision
  Serial1.write("\r");
  delay(DEFAULTIMEOUT);
  //Set motion rotation. CW is piston up (+). CCW is piston down (-)
  Serial1.println("@0");
  Serial1.write(INITIALDIRECTION);
  Serial1.write("\r");
  motorDirection = 0; //Set the direction variable to 0 to indicate down since it will always start this way.

  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485

  //Before we allow the motor to run, lets jsut check that it recieved our profile correctly
  if(checkMotorParams()){
    //Check is good! Turn it on
    digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
    digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
    Serial1.println("@0G1"); //Send Go command
    Serial1.write('\r');
    digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
    digitalWrite(REenable, LOW);  //Enable incoming data from MAX485

    enable = 1;
  } else {
    //Some error occured in sending via RS-485... Lets retry sending
    sendMotorParams();
  }
}

bool checkMotorParams() {
  dfm = "";
  //Send requests to motor to send back its stored parameter values over Serial1
  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  Serial1.println("@0VA1"); //Ask to send back acceleration value
  Serial1.write('\r');
  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
  delay(DEFAULTIMEOUT);; //wait for message to be sent back
  while(Serial1.available()){
    dfm += char(Serial1.read());
  }
  //Now do some substring stuff (probably) to extract acceleration value. Convert String to int. Clear dfm at the end
  String CHECKACCEL = dfm;
  checkAccel = CHECKACCEL.toInt();
  dfm = "";

  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  Serial1.println("@0VB1"); //Ask to send back Base Speed value
  Serial1.write('\r');
  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
  delay(DEFAULTIMEOUT);; //wait for message to be sent back
  while(Serial1.available()){
    dfm += char(Serial1.read());
  }
  //Now do some substring stuff (probably) to extract Base Speed value. Convert String to int. Clear dfm at the end
  String CHECKSPEEDBASE = dfm;
  checkSpeedBase = CHECKSPEEDBASE.toInt();
  dfm = "";

  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  Serial1.println("@0VM1"); //Ask to send back Max Speed value
  Serial1.write('\r');
  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
  delay(DEFAULTIMEOUT);; //wait for message to be sent back
  while(Serial1.available()){
    dfm += char(Serial1.read());
  }
  //Now do some substring stuff (probably) to extract Max Speed value. Convert String to int. Clear dfm at the end
  String CHECKSPEEDMAX = dfm;
  checkSpeedMax = CHECKSPEEDMAX.toInt();
  dfm = "";

  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  Serial1.println("@0VN1"); //Ask to send back index number or steps value
  Serial1.write('\r');
  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
  delay(DEFAULTIMEOUT);; //wait for message to be sent back
  while(Serial1.available()){
    dfm += char(Serial1.read());
  }
  //Now do some substring stuff (probably) to extract index number or steps value. Convert String to int. Clear dfm at the end
  String CHECKSTEPS = dfm;
  checkSteps = CHECKSTEPS.toInt();
  dfm = "";

  digitalWrite(REenable, HIGH);  //Disable incoming data from MAX485
  digitalWrite(DEenable, HIGH);  //Enable outgoing data from MAX485
  Serial1.println("@0V+"); //Ask to send back direction value
  Serial1.write('\r');
  digitalWrite(DEenable, LOW);  //Disable outgoing data from MAX485
  digitalWrite(REenable, LOW);  //Enable incoming data from MAX485
  delay(DEFAULTIMEOUT);; //wait for message to be sent back
  while(Serial1.available()){
    dfm += char(Serial1.read());
  }
  //Now do some substring stuff (probably) to extract direction value. Convert String to int. Clear dfm at the end. 1 is CW, 0 is CCW
  String CHECKDIRECTION = dfm;
  checkDirection = CHECKDIRECTION.toInt();
  dfm = "";

  //If the motor recieved what we sent it, return true. If not, return false
  if(checkAccel == motorAccel && checkSpeedBase == motorSpeedBase && checkSpeedMax == motorSpeedMax && checkSteps == motorSteps && checkCompleteTime == motorCompleteTime && checkDirection == motorDirection){
    return true;
  } else {
    return false;
  }
}

// Function to emergency stop the motor (hard limit). We currently do not have a switch input for this, but generally a good idea. Turning off power to Power Supply will not work due to large capacitors
// void stopMotion() {
//   Serial1.print("@0H");  // Stop motion (Hard Limit)
//   Serial1.println();
//   delay(defaultTimeout);
// }

//It may be a good idea to include a function that will put the nextion and touchpad back in sync. They may get out of sync if one of them looses power
//Currently not implemented
// void configurePage(){
//   Serial2.write(0x70); //Byte by Byte version of saying "page 0"
//   Serial2.write(0x61);
//   Serial2.write(0x67);
//   Serial2.write(0x65);
//   Serial2.write(0x20);
//   Serial2.write(0x30);
//   //Serial2.print("page 0");
//   Serial2.write(endBytes, sizeof(endBytes));
// }
