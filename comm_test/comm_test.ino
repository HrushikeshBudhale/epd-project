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
String inString = "";    // string to hold input

void setup(){
     //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial);      // wait for serial port to connect. Needed for native USB port only
    Serial.println("Serial Communication OK!");    
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

void loop(){
    while (Serial.available() > 0) {
        int inChar = Serial.read();
        inString += (char)inChar;
        if (inChar == '\n') {
            Serial.println("String: ");
            Serial.print(inString);
            Serial.println("Response : ");
            understand(inString);
            // clear the string for new input:
            inString = "";
        }
    }
}

void motor_on(void){
    //digitalWrite(motorPin,HIGH);                // Turn on Motor pump
    //Alarm.timerOnce(motorT, motor_off);         // called once after 'motorT' seconds
    Serial.println("Motor is turned on"); 
}

void motor_off(void){
    //digitalWrite(motorPin,LOW);                // Turn off Motor pump
    Serial.println("Motor is turned off");
}

void lights_on(void){
    //digitalWrite(lightPin,HIGH);                // Turn on Lights
    //Alarm.timerOnce(lightT, lights_off);        // called once after 'lightT' seconds  
    Serial.println("light is turned on");
}

void lights_off(void){
    //digitalWrite(lightPin,LOW);                 //turn off Lights
    Serial.println("light is turned off");
}

// function for enabling/disabling motor alarm functionality
void toggle_motorAlarm(void){
    if(motorAlarm_stat == true){ // if motor alarm is on 
        //Alarm.disable(motorAlarm_ID);   // disable it
        motorAlarm_stat = false;
        Serial.println("Motor alarm is off");
    }
    else{
        //Alarm.enable(motorAlarm_ID);    // enable it
        motorAlarm_stat = true;
        Serial.println("Motor alarm is turned on");
    }
}

// function for enabling/disabling light alarm functionality
void toggle_lightAlarm(void){
    if(lightAlarm_stat == true){ // if light alarm is on 
        //Alarm.disable(lightAlarm_ID);   // disable it
        lightAlarm_stat = false;
        Serial.println("light alarm is turned off");
    }
    else{
        //Alarm.enable(lightAlarm_ID);    // enable it
        lightAlarm_stat = true;
        Serial.println("light alarm is turned on");
    }
}

// function for enabling/disabling logging alarm functionality
void toggle_loggingAlarm(void){
    if(loggingAlarm_stat == true){ // if logging alarm is on 
        //Alarm.disable(loggingAlarm_ID);   // disable it
        loggingAlarm_stat = false;
        Serial.println("logging alarm is turned off");
    }
    else{
        //Alarm.enable(loggingAlarm_ID);    // enable it
        loggingAlarm_stat = true;
        Serial.println("logging alarm is turned on");
    }
}

void set_motorAlarm(String msg){
    num[3] = extractNum(msg,3);
    //Alarm.free(motorAlarm_ID);
    //AlarmId motorAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],motorAlarm);
    Serial.println("motor alarm set to ");
    Serial.print(num[0]);
    Serial.print(",");
    Serial.print(num[1]);
    Serial.print(",");
    Serial.print(num[2]);
}

void set_lightAlarm(String msg){
    num[3] = extractNum(msg,3);
    //Alarm.free(lightAlarm_ID);
    //AlarmId lightAlarm_ID = Alarm.alarmRepeat(num[0],num[1],num[2],lightAlarm);
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
            }
            else{   // surely this alarm is for motor-pump
                set_motorAlarm(msg);
            }
        }
        else if(msg.indexOf("MOTOR") != -1){    // if want to set motor on time      
            motorT = (num[0]*3600) + (num[1]*60) + num[2]; 
            Serial.print("motor time = ");
            Serial.println(motorT); 
        }
        else if(msg.indexOf("LIGHT") != -1){    // if want to set light on time      
            lightT = (num[0]*3600) + (num[1]*60) + num[2]; 
            Serial.println("light time = "); 
            Serial.println(lightT);
        }
        else if((msg.indexOf("SAMPL") != -1)|(msg.indexOf("LOG") != -1)){    // if  want to set sampling interval
            sampleT = (num[0]*3600) + (num[1]*60) + num[2];
            Serial.println("Sampling time = "); 
            Serial.println(sampleT);
        }
        else if(msg.indexOf("TIME") != -1){      // if want to change time of RTC module
            num[6] = extractNum(msg,6);
            //setTime(num[0],num[1],num[2],num[4],num[3],num[5]); // Sets RTC time to user specified value
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

