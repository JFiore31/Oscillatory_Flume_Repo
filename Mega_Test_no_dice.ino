const int DEenable = 22; //Driver Output Enable on MAX485. HIGH to Enable
const int REenable = 24; //Reciever Output Enable on MAX485. LOW to Enable
int incomingByte = 0;
int myTurn = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(38400);
  delay(1000);

  pinMode(DEenable, OUTPUT);
  pinMode(REenable, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(DEenable, LOW); //Disable data being sent to motor
  digitalWrite(REenable, LOW); //Enable data to be sent to Arduino
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(myTurn == 1) {
    delay(2000);
    digitalWrite(REenable, HIGH); //Disable incoming data from MAX485
    digitalWrite(DEenable, HIGH); //Enable outgoing data from MAX485
    digitalWrite(LED_BUILTIN, HIGH);
    Serial1.print("@%");
    Serial1.write('@');
    //Serial1.write("~");
    Serial1.write('%');
    Serial1.write('\r');
    delay(500);
    // Serial1.print("@0A1_100000");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0B1_1000");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0M1_4000");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0N1_500");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0T1_100");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0R8");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0+");
    // Serial1.write('\r');
    // delay(500);
    // Serial1.print("@0G1");
    // Serial1.write('\r');
    // delay(500);
    Serial.print("I sent");

    //Serial1.print("Back at you");
    //Serial1.write('\r');
    delay(500);
    digitalWrite(DEenable, LOW); //Disable outgoing data from MAX485
    digitalWrite(REenable, LOW); //Enable incoming data from MAX485
    digitalWrite(LED_BUILTIN, LOW);
    //myTurn = 0;
  }

  while(Serial1.available()){
    incomingByte = Serial1.read();
    Serial.print("I received: ");
    Serial.println(incomingByte, HEX);
    myTurn = 1;
  }
}
