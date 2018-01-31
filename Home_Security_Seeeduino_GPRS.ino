/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
  
  modified 8 Sep 2016
  by Colby Newman
*/

/*
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(10000);                       // wait for a second
}
*/

/*******************************************************************************/
/*macro definitions of PIR motion sensor pin and LED pin*/
#define PIR_MOTION_SENSOR 2//Use pin 8 to receive the signal from the module
#define LED    13//the Grove - LED is connected to D4 of Arduino

#include <gprs.h>
#include <SoftwareSerial.h>

#include <time.h>

GPRS gprs;

struct tm lastHeartbeatMsg;

void setup()
{
    pinsInit();
    setup_gprs();
}



void loop()
{
    //Phone
    char gprsBuffer[64];
    char *s = NULL;
    int inComing;
    
    if(gprs.serialSIM800.available()) {
        inComing = 1;
    }
    else {
        inComing = 0;
        delay(100);
    }

    if(inComing){
        gprs.readBuffer(gprsBuffer,32,DEFAULT_TIMEOUT);
        Serial.print(gprsBuffer);

        if(NULL != strstr(gprsBuffer,"RING")) {
            gprs.answer();
        }else if(NULL != (s = strstr(gprsBuffer,"+CMTI: \"SM\""))) { //SMS: $$+CMTI: "SM",24$$
            char message[MESSAGE_LENGTH];
            int messageIndex = atoi(s+12);
            gprs.readSMS(messageIndex, message,MESSAGE_LENGTH);
            Serial.print(message);

            process_message(message);
            
        }
        gprs.cleanBuffer(gprsBuffer,32);
        inComing = 0;
    }

    //PIR
    if(isPeopleDetected()) 
	{
		core.value = core.value + core.step;
		turnOnLED();
	}
    else turnOffLED();
}
void pinsInit()
{
    Serial.begin(9600);
    pinMode(PIR_MOTION_SENSOR, INPUT);
    pinMode(LED,OUTPUT);
}
void turnOnLED()
{
    digitalWrite(LED,HIGH);
}
void turnOffLED()
{
    digitalWrite(LED,LOW);
}
/***************************************************************/
/*Function: Detect whether anyone moves in it's detecting range*/
/*Return:-boolean, ture is someone detected.*/
boolean isPeopleDetected()
{
    int sensorValue = digitalRead(PIR_MOTION_SENSOR);
    if(sensorValue == HIGH)//if the sensor value is HIGH?
    {
        return true;//yes,return ture
    }
    else
    {
        return false;//no,return false
    }
}



void setup_gprs() {
    int init;
    Serial.begin(9600);
    Serial.println("GPRS - Send SMS Test ...");
    gprs.preInit();
    delay(1000);
    while(0 != (init = gprs.init())) {
        delay(1000);
        Serial.print("Initialization error - (");
        Serial.print(init);
        Serial.print(") \r\n");
    }
    Serial.println("Initialization successful!");

    // Get current date in format yy/MM/dd,hh:mm:ss
    char tmBuffer[sizeof("yy/MM/dd,hh:mm:ss")];
    gprs.getCurrentTime(tmBuffer);
    Serial.println(tmBuffer);

    // Send message
    /*
    Serial.println("Init success, start to send SMS message...");
    gprs.sendSMS("+420600XXXXXX","hello,world"); //define phone number and text
    */
    
    // Make call
    /*
    Serial.println("Init success, start to call...");
    gprs.callUp("+420600XXXXXX");

    while (1) 
    {
      gprs.getPhoneActivityStatus();
    }
    */
}

void process_message(char *message)
{
  if(NULL != strstr(message,"ALIVERRB"))
  {
    /* I am ALIVE, Ready to Respond Back */
	!!! This should be sent only to a requester
    Serial.println("Sending confirmation ... I am ALIVE, Ready to Respond Back");
    gprs.sendSMS("+420600XXXXXX","OK!");
  }
  
  DISABLE
  ENABLE
  SETPARAMS (in future)
  elseif RESET
  elseif 
  else
}

void surveillance()
{
	/* Trigger alarm */
	if (core.value > core.threshold)
	{
		send message to all
		core.value = 0;
	}
	else
	{
		/* Forgetting factor to avoid noisy alarms */
		if (core.value > 0)
		{
			core.value = core.value - core.forget;
		}
		else
		{
			core.value = 0;
		}
	}
	
}
 


struct monitor
{
	/* Parameters */
	float threshold;
	float step;
	float forget;	
	
	/* Status */
	uint8_t enabled;
	float value;
};

/* Core monitor */
struct monitor core;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PHONENUMBERLENGTH 13

/* Dynamic memory phone book */
struct phone
{
	char number[PHONENUMBERLENGTH];
	uint8_t role;
	/* Pointer to next item */
	struct phone *next;
};

struct phone *phonebook = NULL;
struct phone *last = NULL;

int loadPhonebook(char *file)
{
	/* Loading phone book from a file */
	char buffer[PHONENUMBERLENGTH];
	FILE* f = fopen(file, "rt");
	
    while (fgets(buffer, MAXLINELENGTH, f) == NULL) 
	{
		if (line[0] != '\n' && line[0] != '\r' && line[0] != '#')
	    {
			/* Add it to phone book */
			struct phone *n;
			n = (struct phone *) malloc(sizeof(struct phone));
			strncpy(n->number,buffer,PHONENUMBERLENGTH);
			if (phonebook == NULL)
			{
				phonebook = n;
				last = n;
				n->role = 1; /* Admin */
			}
			else
			{
				n->role = 0;
				last->next = n;
				last = n;
			}
			
		}
    }
	fclose(f);
}







