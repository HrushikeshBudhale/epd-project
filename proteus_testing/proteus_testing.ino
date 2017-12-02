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

//################ Global initialisation ###############
//=============Temp/humidity sensor=========
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//=============RTC module===================
AlarmId id;

//=============GSM module===================
#define PINNUMBER ""
GSM gsmAccess;
GSM_SMS sms;
GSMVoiceCall vcs;
char senderNumber[20];  // Array to hold the number a SMS is retreived from
char numtel[20];        // Array to hold the number for the incoming call

//################ main code ######################
void setup() {
//=============Serial Communication===================
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial);      // wait for serial port to connect. Needed for native USB port only
    Serial.println("Serial Communication OK!");
    
//=============Temp/humidity sensor=========
    dht.begin();  // also sets resp. pin as input
    Serial.println("Temperature/Humidity sensor OK!");
    
//=============RTC module===================
    //format: HH,MM,SS,mm,DD,YY
    setTime(8,29,0,1,1,11); // set time to Saturday 8:29:00am Jan 1 2011
    
    // create the alarm, to trigger at specific time
    Alarm.alarmRepeat(8,30,0,motor_on);  // 8:30am every day
    Alarm.alarmRepeat(17,45,0,lights_on);  // 5:45pm every day
    
    // create timer to trigger relative to when it is created
    Alarm.timerRepeat(7200,chk_status);           // timer for every 7200 seconds(2 hours)

  
}

void loop() {
    // wait for call or sms 
}

//################ Alarm functions ######################
void  motor_on(void){
    //Send sms to user
    //start motor
    Alarm.timerOnce(7200, motor_off);            // called once after 7200 seconds(2 hours)
}

void motor_off(void){
    //stop motor
}

void  lights_on(void){
    //Send sms to user
    //turn on lights
    Alarm.timerOnce(7200, lights_off);           // called once after 72000 seconds(2 hours)  
}

void lights_off(void){
    //turn off lights
}

void chk_status(void){
    //read all sensor values and store in array
}

//################ communication functions ######################
// check if valid user
// set threshhold for message trigger
// if sent once dont send on same day
//

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
void understand(string msg){
    msg.toUpperCase();          // for convenient reading and debug
    int totLen = msg.length();  // store length of total message
    if(totLen < 3){ // i.e. if message is of type 'command-mode'
        switch(msg.charAt(0)){  // decide command based on first letter
            case '.':   // button 1 is pressed
                        // function for motor off
                        break;   
            case 'A':   // button 2 is pressed
                        // function for lights off
                        break;
            case 'D':   // button 3 is pressed
                        // function for motor on
                        break;
            case 'G':   // button 4 is pressed
                        // function for lights on
                        break;
            case 'J':   // button 5 is pressed
                        // for future use
                        break;
            case 'M':   // button 6 is pressed
                        // for future use
                        break;
            case 'P':   // button 7 is pressed
                        // for future use
                        break;
            case 'T':   // button 8 is pressed
                        // for future use
                        break;     
            case 'W':   // button 9 is pressed
                        // for future use
                        break;                                                                                           
        }
    }
    else{       // i.e. if message is of type 'program-mode'
           if(msg.indexOf("ALARM") != -1){   // if want to set alarm
                if(msg.indexOf("LIGHT") != -1){   // is this alarm for lights
                    //function
                }
                else{   // surely this alarm is for motor-pump
                    //function
                }
           }
           else if(msg.indexOf("SAMPLE") != -1){   // if  want to set sampling interval
                //function
           }
           else if(msg.indexOf("TIME")) != -1{  // if want to change time of RTC module
                // function
           }
    }
}

