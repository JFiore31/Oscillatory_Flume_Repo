// #include "EasyNextionLibrary.h"
// EasyNex myNex(Serial);  //create a new EasyNex object named myNex to be used with EasyNextionLibrary

const int TX0 = 1;
const int RX0 = 3;
const int TX1 = 25;
const int RX1 = 32;
uint8_t endBytes[] = {0xFF, 0xFF, 0xFF};

int waveType = 1; //define an int variable named wave type to set if the wave will be square or sinusodial.
                  //Sqaure = 0. Sinusodial = 1.
int controlType;  //define an int variable named control type to determine if wave input parameters will be Speed & Amplitude, Amplitude & Frequency, or Frequency & Speed.
                  //Speed & Amplitude = 0. Amplitude & Frequency = 1. Frequency & Speed = 2.
int frequency;    //define an int variable to store the wave frequency in Hz
int speed;        //define an int variable to store the wave speed in m/s
int amplitude;    //define an int variable to store the wave amplitude in m
bool enable;      //define a boolean variable to determine motor On/Off state
int steps;        //define an int variable to be sent to motor for amount of steps to turn

// TaskHandle_t waveGenTask;

// void squareWaveGen(void* pvParameters);    //declare a function in memory to create a square wave
// void sinusoidWaveGen(void* pvParameters);  //declare a function in memory to create a sinusodial wave

// void waveGen(void* pvParameters){
//   for(;;){
//     if(enable){
//       if (waveType == 0) {
//         squareWaveGen();
//       } else if (waveType == 1) {
//         sinusoidWaveGen();
//       } else {
//         Serial.println("Error: wave type not set!");
//       }
//     } else {}
//   }
// }
// void sinusoidWaveGen(void* pvParameters){
//   for(;;){
//     Serial.println("Sinusoid Wave running on core ");
//     Serial.println(xPortGetCoreID());
//     delay(700);
//   }

void setup() {
  Serial.begin(9600);     //Start Serial communications to the touchpad
  Serial.begin(9600, SERIAL_8N1, RX0, TX0);
  //Serial2.begin(38400, SERIAL_7N1, TX1, RX1);  //Start serial communications to the Motor
  //Here I will define he GPIO pins used as constant ints
  
  // xTaskCreatePinnedToCore(
  //   waveGen,
  //   "Wave Maker",
  //   10000,
  //   NULL,
  //   1,
  //   &waveGenTask,
  //   0);
  //   delay(500);
}

void loop() {
  // myNex.NextionListen();
  while(Serial.available()){

  }
}
// }
void squareWaveGen() {
  //do things
}
void sinusoidWaveGen() {
  //do other things
}
void solveFrequency(int speed, int amplitude) {
  frequency = speed / amplitude; //will need to truncate to an int
}

void solveSpeed(int amplitude, int frequency) {
  speed = amplitude * frequency;
}

void solveAmplitude(int speed,int frequency) {
  amplitude = speed / frequency;
}

//Note that all triggerX functions are already declared in the Header file
void trigger0() {  //called when Sinusodial Wave button on Page0 is pressed
  waveType = 0;    //set wave type to 0 for sinusoid
  //Serial.println("Trigger0");
  myNex.writeNum("Trigger0");
}

void trigger1() {  //called when Square Wave button on Page0 is pressed
  waveType = 1;    //set wave type to 1 for square
  Serial.println("Trigger1");
}

void trigger2() {   //called when Speed and Amplitude button on page1 is pressed
  controlType = 0;  //set control type to 0 to define with speed and amplitude
  Serial.println("Trigger2");
}

void trigger3() {   //called when Amplitude and Frequency button on page1 is pressed
  controlType = 1;  //set control type to 1 to define with Amplitude and Frequency
  Serial.println("Trigger3");
}

void trigger4() {   //called when Frequency and Speed button on page1 is pressed
  controlType = 2;  //set control type to 2 to define with Frequency and Speed
  Serial.println("Trigger4");
}

void trigger5() { //Called when Speed and Amp "Run" is clicked
  speed = myNex.readNumber("speed1.val"); //may need to convert to int
  amplitude = myNex.readNumber("amp1.val");
  if(speed == 777777 || amplitude == 777777){
    //Error has occured, do something
  }
  solveFrequency(speed, amplitude);
  myNex.writeNum("n0.val", speed);
  myNex.writeNum("n1.val", amplitude);
  myNex.writeNum("n2.val", frequency);
  Serial.println("Trigger5");
}

void trigger6() { //Called when Amp and Freq page "Run" is clicked
  amplitude = myNex.readNumber("amp2.val");
  frequency = myNex.readNumber("freq1.val");
  if(amplitude == 777777 || frequency == 777777){
    //Error has occured, do something
  }
  solveSpeed(amplitude, frequency);
  myNex.writeNum("n0.val", speed);
  myNex.writeNum("n1.val", amplitude);
  myNex.writeNum("n2.val", frequency);
  Serial.println("Trigger6");
}

void trigger7() { //Called when Speed and Freq "Run" is clicked
  speed = myNex.readNumber("speed2.val");
  frequency = myNex.readNumber("freq2.val");
  if(speed == 777777 || frequency == 777777){
    //Error has occured, do something
  }
  solveAmplitude(speed, frequency);
  myNex.writeNum("n0.val", speed);
  myNex.writeNum("n1.val", amplitude);
  myNex.writeNum("n2.val", frequency);
  Serial.println("Trigger7");
}

void trigger8() { //called when any "Confirm Run" is clicked
  myNex.writeNum("n0.val", speed);
  myNex.writeNum("n1.val", amplitude);
  myNex.writeNum("n2.val", frequency);
  //some math may be in if statements here depending on wave type being sinusoid or sqaure
  steps = amplitude / 5;  //convert the amplitude to the amount of steps the motor has to achieve to do this
                          //motor microstepping is X and gear ratio is 1:1
  //next calculate the max speed and acceleration needed by motor
  enable = true;
  Serial.println("Trigger8");
}

void trigger9() { //called when "End Program" is clicked
  enable = false;
  Serial.println("Trigger9");
}
