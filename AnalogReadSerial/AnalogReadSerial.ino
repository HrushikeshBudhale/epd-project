// the setup routine runs once when you press reset:
void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial);      // wait for serial port to connect. Needed for native USB port only
    Serial.println("Serial Communication OK!");
    pinMode(13,OUTPUT);
    pinMode(12,OUTPUT);
    digitalWrite(13,HIGH);
    digitalWrite(12,LOW);
    delay(3000);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  if(sensorValue > 156){
    digitalWrite(13,LOW);
    digitalWrite(12,HIGH);
    Serial.println("bulb low");
  }
  else{
    digitalWrite(12,LOW);
    digitalWrite(13,HIGH);
    Serial.println("LED low");
  }
  Serial.println(sensorValue);
  delay(500);        // delay in between reads for stability
}
