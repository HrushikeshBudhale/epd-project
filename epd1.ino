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
 *  Relays
 *  Lights
 *  
 * Caution :- set correct time before uploading program for proper functioning
 */

//################ libraries ######################
#include <Wire.h>
#include <GSM.h>
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

//=========== Relay for motor/lights =========
#define motorPin 13
#define lightPin 12

//=============Temp/humidity sensor=========
#define soilSensorPin A0 

//=============GSM module===================
#define PINNUMBER ""
GSM gsmAccess;
GSM_SMS sms;
GSMVoiceCall vcs;

char senderNumber[20];  // Array to hold the number a SMS is retreived from
char numtel[20];        // Array to hold the number for the incoming call
boolean notConnected = true;        // connection state
char validUser[12] = "987654321";   // user's phone number

//################ main code ######################
void setup() {
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
    setTime(8,29,0,1,1,11);                 // set time to Saturday 8:29:00am Jan 1 2011
    
    // create the alarm, to trigger at specific time
    motorAlarm_ID = Alarm.alarmRepeat(8,30,0,motorAlarm);     // 8:30am every day
    lightAlarm_ID = Alarm.alarmRepeat(17,45,0,lightAlarm);   // 5:45pm every day
    
    // create timer to trigger relative to when it is created
    loggingAlarm_ID = Alarm.timerRepeat(sampleT,chk_status);  // timer for every 'sampleT' sec

//=============GSM module===================
    // Start GSM connection
    while (notConnected) {
        if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
            notConnected = false;
        }
        delay(1000);
    }
    // This makes sure the modem correctly reports incoming events
    vcs.hangCall();
}

void loop() {
    String msg;
    // wait for call or sms 
    if (sms.available()){   // if sms arrived
        sms.remoteNumber(senderNumber, 20);         // Get sender's number
        if(strcmp(senderNumber, validUser) == 0){  // if valid user
            while (char c = sms.read()) {
                msg = String(msg + c); 
            }
            understand(msg);
            sms.flush();
        }
    }
    else if(vcs.getvoiceCallStatus() == RECEIVINGCALL){     //if someone is calling
        vcs.retrieveCallingNumber(senderNumber, 20);
        if(strcmp(senderNumber, validUser) == 0){  // if valid user
            vcs.hangCall();     // ends the call
            sendSMS(msg);       //  sends sms with current stats to 'validUser'
        }
    }
    Alarm.delay(1000);
}

//################ Alarm functions ######################
void motorAlarm(void){
    //sendSMS();  //send sms to user
    motor_on();
}

void motor_on(void){
    digitalWrite(motorPin,HIGH);                // Turn on Motor pump
    Alarm.timerOnce(motorT, motor_off);         // called once after 'motorT' seconds
}

void motor_off(void){
    digitalWrite(motorPin,LOW);                // Turn off Motor pump
}

void lightAlarm(void){
    //sendSMS();  //Send sms to user
    lights_on();
}

void lights_on(void){
    digitalWrite(lightPin,HIGH);                // Turn on Lights
    Alarm.timerOnce(lightT, lights_off);        // called once after 'lightT' seconds  
}

void lights_off(void){
    digitalWrite(lightPin,LOW);                 //turn off Lights
}

// function for enabling/disabling motor alarm functionality
void toggle_motorAlarm(void){
    if(motorAlarm_stat == true){ // if motor alarm is on 
        Alarm.disable(motorAlarm_ID);   // disable it
        motorAlarm_stat = false;
    }
    else{
        Alarm.enable(motorAlarm_ID);    // enable it
        motorAlarm_stat = true;
    }
}

// function for enabling/disabling light alarm functionality
void toggle_lightAlarm(void){
    if(lightAlarm_stat == true){ // if light alarm is on 
        Alarm.disable(lightAlarm_ID);   // disable it
        lightAlarm_stat = false;
    }
    else{
        Alarm.enable(lightAlarm_ID);    // enable it
        lightAlarm_stat = true;
    }
}

// function for enabling/disabling logging alarm functionality
void toggle_loggingAlarm(void){
    if(loggingAlarm_stat == true){ // if logging alarm is on 
        Alarm.disable(loggingAlarm_ID);   // disable it
        loggingAlarm_stat = false;
    }
    else{
        Alarm.enable(loggingAlarm_ID);    // enable it
        loggingAlarm_stat = true;
    }
}

void set_motorAlarm(String msg){
    num[3] = extractNum(msg,3);
    Alarm.free(motorAlarm_ID);
    AlarmId motorAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],motorAlarm);
}

void set_lightAlarm(String msg){
    num[3] = extractNum(msg,3);
    Alarm.free(lightAlarm_ID);
    AlarmId lightAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],lightAlarm);
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
            }
            else{   // surely this alarm is for motor-pump
                set_motorAlarm(msg);
            }
        }
        else if(msg.indexOf("MOTOR") != -1){    // if want to set motor on time      
            motorT = (num[0]*3600) + (num[1]*60) + num[2]; 
        }
        else if(msg.indexOf("LIGHT") != -1){    // if want to set light on time      
            lightT = (num[0]*3600) + (num[1]*60) + num[2]; 
        }
        else if((msg.indexOf("SAMPL") != -1)|(msg.indexOf("LOG") != -1)){    // if  want to set sampling interval
            sampleT = (num[0]*3600) + (num[1]*60) + num[2]; 
        }
        else if(msg.indexOf("TIME") != -1){      // if want to change time of RTC module
            num[6] = extractNum(msg,6);
            setTime(num[0],num[1],num[2],num[4],num[3],num[5]); // Sets RTC time to user specified value
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

// function for sending sms to 'validUser'
void sendSMS(String msg){
    sms.beginSMS(validUser);
    sms.print(msg); // writes message content
    sms.endSMS();   // sends sms
}

void digitalClockDisplay() {
  // digital clock display of the time
  hour();
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

//reads all sensor values and notify if necessary
void chk_status(void){
    float h = dht.readHumidity();
    float t = dht.readTemperature();    // Read temperature as Celsius (the default)   

}

