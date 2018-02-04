/* Project:- Field monitoring for Precision farming 
 * Created date:- 28/10/17
 * Group members:-
 *    Hrushikesh Budhale
 *    Pravin Nilewad
 *    Tejashree Ghogale
 *
 * Board:- Arduino UNO 
 * Sensors/Accesories:-
 *  DHT22 (temp. and humidity sensor)
 *  GSM Module
 *  DS1307 (Real time clock)
 *  Soil moisture sensor
 *  Lights
 *  
 * Caution :- set correct time and validUser before uploading program for proper functioning
 */

//################ libraries ######################
#include <Wire.h>
#include <SoftwareSerial.h>
#include <GPRS_Shield_Arduino.h>
#include "DHT.h"
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <TimeLib.h>
#include <TimeAlarms.h>

//################ Global declaration ###############
//=============RTC module===================
// Creates ID for each alarm 
// ID is used for creating and deleting alarms in runtime
AlarmId motorAlarm_ID;
AlarmId lightAlarm_ID;
AlarmId loggingAlarm_ID;

// Values are in seconds
unsigned int sampleT = 10;      // Sensor Data is acquired after this much time
unsigned int motorT = 15;       // Motor runs for this much time
unsigned int lightT = 20;       // Light are on for this much time

// Alarm status(keeps track wherher enabled or disabled)
boolean motorAlarm_stat = true;
boolean lightAlarm_stat = true;
boolean loggingAlarm_stat = true;

// Stores the array of numbers extracted  from received SMS
int num[6];

//============= Temp/humidity sensor =========
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float HumidityThresh = 50;
int TempThresh = 29;

//=========== Relay for motor/lights =========
#define motorPin 13
#define lightPin 12

//=============Soil moisture sensor=========
#define soilSensorPin A0 
float MoistureThresh = 10;

//=============GSM module===================
/*#define PINNUMBER ""
GSM gsmAccess;
GSM_SMS sms;
GSMVoiceCall vcs;*/
#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600
#define MESSAGE_LENGTH 40
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char datetime[24];
GPRS gprsTest(PIN_TX,PIN_RX,BAUDRATE);//RX,TX,PWR,BaudRate


char senderNumber[16];  // Array to hold the number a SMS is retreived from
boolean notConnected = true;        // connection state
char validUser[16] = "+919623928230";   // user's phone number

char gprsBuffer[45];
int i = 0;
char *s = NULL;


//################ main code ######################
void setup() {
    Serial.begin(9600);
    while (!Serial);      // wait for serial port to connect. Needed for native USB port only
    Serial.println("Serial Communication OK!");    
    
//===========Relay for motor/lights=========
    pinMode(motorPin,OUTPUT);
    pinMode(lightPin,OUTPUT);
    digitalWrite(motorPin,LOW);
    digitalWrite(lightPin,LOW);

//=============Soil moisture sensor=========
    pinMode(soilSensorPin,INPUT);    

//=============Temp/humidity sensor=========
    dht.begin();  // also sets resp. pin as input

//=============RTC module===================
    //format: HH,MM,SS,mm,DD,YY
    setTime(6,10,0,12,6,17);                 // set time to Saturday 8:29:00am Jan 1 2011
    
    // create the alarm, to trigger at specific time
    motorAlarm_ID = Alarm.alarmRepeat(7,13,0,motorAlarm);     // 8:30am every day
    lightAlarm_ID = Alarm.alarmRepeat(7,15,0,lightAlarm);   // 5:45pm every day
    
    // create timer to trigger relative to when it is created
    loggingAlarm_ID = Alarm.timerRepeat(sampleT,chk_status);  // timer for every 'sampleT' sec

//=============GSM module===================
    // Start GSM connection
    while(!gprsTest.init()) {
        delay(1000);
    }
    Serial.println("GPRS initialized");
    delay(3000);
}

void loop() {
    
    //String msg;
    int inComing = 0;
    /*if(Serial.available() > 0) {
        inComing = 1;
        while (Serial.available() > 0) {
            int inChar = Serial.read();
            inString += (char)inChar;
            if (inChar == '\n') {
                Serial.println("String: ");
                Serial.print(inString);
                Serial.println("Response : ");
                inString.toCharArray(gprsBuffer,63);
                break;
            }
        }
    }*/
    if(gprsTest.readable()) {
        inComing = 1;
    }
    else{
        Alarm.delay(1000);
        Serial.println("in delay");
    }
    if(inComing){
        sim900_read_buffer(gprsBuffer,40,DEFAULT_TIMEOUT);
        Serial.println("execuing");
        if(NULL != strstr(gprsBuffer,"RING")) { // if call
            gprsTest.hangup();  // end call
            String str = chk_status();
            str.toCharArray(message,MESSAGE_LENGTH);
            gprsTest.sendSMS(validUser,message);    // send sms to valid user
            Serial.println(str);
        }
        else if(NULL != (s = strstr(gprsBuffer,"+CMTI: \"SM\""))) { // if SMS
            Serial.println("SMS received");
            char message[MESSAGE_LENGTH];
            int messageIndex = atoi(s+12);
            gprsTest.readSMS(messageIndex, message, MESSAGE_LENGTH, senderNumber, datetime);           
            Serial.println(String(message));
            Serial.println(String(senderNumber));
            if(strcmp(senderNumber, validUser) == 0){  // if valid user
                /*while (char c = sms.read()) {
                    msg = String(msg + c); 
                }*/
                understand(String(message));
                
            }
            gprsTest.deleteSMS(1);  // free memory
        }
        sim900_clean_buffer(gprsBuffer,40);  
        inComing = 0;
    }
}

//################ Alarm functions ######################
void motorAlarm(void){
    gprsTest.sendSMS(validUser,"Turning Motor ON");    // send sms to valid user
    motor_on();
}

void motor_on(void){
    digitalWrite(motorPin,HIGH);                // Turn on Motor pump
    Alarm.timerOnce(motorT, motor_off);         // called once after 'motorT' seconds
    Serial.println("Motor is turned on"); 
}

void motor_off(void){
    digitalWrite(motorPin,LOW);                // Turn off Motor pump
    Serial.println("Motor is turned off");
}

void lightAlarm(void){
    gprsTest.sendSMS(validUser,"Turning Lights ON");    // send sms to valid user
    lights_on();
}

void lights_on(void){
    digitalWrite(lightPin,HIGH);                // Turn on Lights
    Alarm.timerOnce(lightT, lights_off);        // called once after 'lightT' seconds  
    Serial.println("light is turned on");
}

void lights_off(void){
    digitalWrite(lightPin,LOW);                 //turn off Lights
    Serial.println("light is turned off");
}

// function for enabling/disabling motor alarm functionality
void toggle_motorAlarm(void){
    if(motorAlarm_stat == true){ // if motor alarm is on 
        Alarm.disable(motorAlarm_ID);   // disable it
        motorAlarm_stat = false;
        Serial.println("Motor alarm is off");
    }
    else{
        Alarm.enable(motorAlarm_ID);    // enable it
        motorAlarm_stat = true;
        Serial.println("Motor alarm is turned on");
    }
}

// function for enabling/disabling light alarm functionality
void toggle_lightAlarm(void){
    if(lightAlarm_stat == true){ // if light alarm is on 
        Alarm.disable(lightAlarm_ID);   // disable it
        lightAlarm_stat = false;
        Serial.println("light alarm is turned off");
    }
    else{
        Alarm.enable(lightAlarm_ID);    // enable it
        lightAlarm_stat = true;
        Serial.println("light alarm is turned on");
    }
}

// function for enabling/disabling logging alarm functionality
void toggle_loggingAlarm(void){
    if(loggingAlarm_stat == true){ // if logging alarm is on 
        Alarm.disable(loggingAlarm_ID);   // disable it
        loggingAlarm_stat = false;
        Serial.println("logging alarm is turned off");
    }
    else{
        Alarm.enable(loggingAlarm_ID);    // enable it
        loggingAlarm_stat = true;
        Serial.println("logging alarm is turned on");
    }
}

void set_motorAlarm(String msg){
    num[3] = extractNum(msg,3);
    Alarm.free(motorAlarm_ID);
    AlarmId motorAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],motorAlarm);
    Serial.println("motor alarm set to ");
    Serial.print(num[0]);
    Serial.print(",");
    Serial.print(num[1]);
    Serial.print(",");
    Serial.print(num[2]);
}

void set_lightAlarm(String msg){
    num[3] = extractNum(msg,3);
    Alarm.free(lightAlarm_ID);
    AlarmId lightAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],lightAlarm);
    Serial.println("light alarm set to ");
    Serial.print(num[0]);
    Serial.print(",");
    Serial.print(num[1]);
    Serial.print(",");
    Serial.print(num[2]);
}

//################ communication functions ######################
// set threshhold for message trigger
// if sent once dont send on same day for critical situation

// function for understanding received message
/* This function supports 2 modes of operation 
 *      1.Command mode:
 *          Consists of messages with only one character. 
 *          predefined functions can be called by this method.
 *          Specially desigened for phones with keypads.
 *      2.Program mode:
 *          Consists of messages with some predefined keywords.
 *          System parameters can be tuned using these keywords.
 *          Specially desiged for dynamic behaviour of system.
 */
void understand(String msg){
    msg.trim();                 // removes any white spaces in string
    msg.toUpperCase();          // for convenient reading and debugging
    if(msg.length() < 3){ // i.e. if message is of type 'command-mode'
        switch(msg.charAt(0)){  // decide command based on first letter
            case '.':   // button 1 is pressed
            case '1':
                        motor_off();    // function for motor off
                        break;   
            case 'G':   // button 4 is pressed
            case '4':
                        motor_on();    // function for motor on
                        break;
            case 'P':   // button 7 is pressed
            case '7':
                        // function for enabling/disabling motor alarm functionality
                        toggle_motorAlarm();
                        break;
            case 'A':   // button 2 is pressed
            case '2':
                        lights_off();   // function for lights off
                        break;
            case 'J':   // button 5 is pressed
            case '5':
                        lights_on();   // function for lights on
                        break;
            case 'T':   // button 8 is pressed
            case '8':
                        // function for enabling/disabling light alarm functionality
                        toggle_lightAlarm();
                        break;     
            case 'D':   // button 3 is pressed
            case '3':
                        // for future use
                        break;
            case 'M':   // button 6 is pressed
            case '6':
                        // for future use
                        break;
            case 'W':   // button 9 is pressed
            case '9':
                        // function for enabling/disabling logging alarm functionality
                        toggle_loggingAlarm();
                        break;                                                                                           
        }
    }
    else{   // i.e. if message is of type 'program-mode'
        num[3] = extractNum(msg,3);  // returns array of three numbers from SMS
        if(msg.indexOf("ALARM") != -1){  // if want to set alarm
            if(msg.indexOf("LIGHT") != -1){     // is this alarm for lights
                set_lightAlarm(msg);
                Serial.print("light Alarm time = ");
                Serial.print(num[0]);
                Serial.print(",");
                Serial.print(num[1]);
                Serial.print(",");
                Serial.print(num[2]);
            }
            else{   // surely this alarm is for motor-pump
                set_motorAlarm(msg);
                Serial.print("motor Alarm time = ");
                Serial.print(num[0]);
                Serial.print(",");
                Serial.print(num[1]);
                Serial.print(",");
                Serial.print(num[2]);
            }
        }
        if(msg.indexOf("THRESH") != -1){  // if want to set threshold values
            if(msg.indexOf("TEMP") != -1){    // set threshold for Temperature      
                TempThresh = num[0];
                Serial.print("temperature Threshold = ");
                Serial.println(num[0]); 
            }
            else if(msg.indexOf("HUMI") != -1){     // set threshold for Humidity
                HumidityThresh = num[0];
                Serial.print("Humidity Threshold = ");
                Serial.println(num[0]);
            }
            else if(msg.indexOf("MOIS") != -1){    // set threshold for Moisture    
                MoistureThresh = num[0];
                Serial.print("MOisture Threshold = ");
                Serial.println(num[0]);
            }
        }
        else if(msg.indexOf("MOTOR") != -1){    // if want to set motor on time      
            motorT = (num[0]*3600) + (num[1]*60) + num[2]; 
            Serial.print("motor ON time = ");
            Serial.println(motorT); 
        }
        else if(msg.indexOf("LIGHT") != -1){    // if want to set light on time      
            lightT = (num[0]*3600) + (num[1]*60) + num[2]; 
            Serial.println("light ON time = "); 
            Serial.println(lightT);
        }
        else if((msg.indexOf("SAMPL") != -1)|(msg.indexOf("LOG") != -1)){    // if  want to set sampling interval
            sampleT = (num[0]*3600) + (num[1]*60) + num[2];
            Serial.println("Sampling interval = "); 
            Serial.println(sampleT); 
        }
        else if(msg.indexOf("TIME") != -1){      // if want to change time of RTC module
            num[6] = extractNum(msg,6);
            setTime(num[0],num[1],num[2],num[4],num[3],num[5]); // Sets RTC time to user specified value
            Serial.print(num[0]);
            Serial.print(",");
            Serial.print(num[1]);
            Serial.print(",");
            Serial.print(num[2]);
            Serial.print(",");
            Serial.print(num[4]);
            Serial.print(",");
            Serial.print(num[3]);
            Serial.print(",");
            Serial.print(num[5]);
        }
    }
}

// returns array of dot('.') seperated value ahead of '=' sign
int extractNum(String msg, char values){
    boolean remaining = true;
    char from;
    for(char i=0; i<values; i++){
        if(i == 0)
            from = msg.indexOf('=')+1;
        else if(from != 0)
            from = msg.indexOf('.',from)+1;
        if(from != 0)
            num[i] = (msg.substring(from)).toInt();
        else
            num[i] = 0;
    }
    return num;
}

String digitalClockDisplay() {
    // digital clock display of the time
    String t;
    t = String(hour());
    t = t + strDigits(minute());
    t = t + strDigits(second());
    return(t);
}

String strDigits(int digits) {
    String t;
    t = ":";
    if (digits < 10)
    t = t +'0';
    t = t + digits;
    return(t);
}

// Reads all sensor values and notify if necessary
// act=1(take action) || act=0(return string)
String chk_status(void){
    String reply;
    Serial.println("Monitoring Sensor data");
    float humi = 86;//dht.readHumidity();
    float temp =26.7; //dht.readTemperature();    // Read temperature as Celsius (the default)   
    int mois =130;//analogRead(soilSensorPin);  // Read moisture range 1-1023
    mois = map(mois, 0, 1023, 0, 100);     // Convert to percentage
    
    reply = reply + "Time: " + digitalClockDisplay() + "\n";
    reply = reply + "H = " + humi + "\n";
    reply = reply + "T = " + temp + "\n";
    reply = reply + "M = " + mois + "\n";
    if((mois < MoistureThresh)&&(temp > TempThresh)&&(humi < HumidityThresh)){  // if condition is severe
            motorAlarm();   // turn motor on
            reply = "Turning Moror ON\n" + reply;
            reply.toCharArray(message,MESSAGE_LENGTH);
            gprsTest.sendSMS(validUser,message);    // send sms to valid user
    }
    Serial.println(digitalClockDisplay());
    
    return(reply);
}

