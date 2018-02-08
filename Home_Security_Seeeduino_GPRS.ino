/********************************************************************************/
/* Home security system for Seeeduino GPRS										*/
/********************************************************************************/

#include "Home_Security_Seeeduino_GPRS.h"

#define PIRLED    13

#include <gprs.h>
#include <SoftwareSerial.h>

#include <time.h>

#include <Adafruit_SleepyDog.h>

GPRS gprs;

void setup()
{
    /* Main setup */
	Serial.begin(9600);
	setup_pir();
    
	setup_sdcard();
	load_phonebook(F("phonebook.txt");

	setup_gprs();
	
	setup_monitor();
}



void loop()
{
    /* Main loop */
    char gprsBuffer[64];
    char *s = NULL;
	char message[MESSAGE_LENGTH];
	int messageIndex;	
	
	/* Surveillance via PIR sensor */
	if(isPeopPIRLEDetected()) 
	{
		core.value = core.value + core.step;
		turnOnPIRLED();
	}
    else 
	{
		turnOffPIRLED();
	}
	surveillance();
	
	/* Communication */
    if(gprs.serialSIM800.available()) 
	{
		gprs.readBuffer(gprsBuffer, 32, DEFAULT_TIMEOUT);
        Serial.print(gprsBuffer);

        if(NULL != strstr(gprsBuffer, "RING")) 
		{
            /* Answer the incoming call - not used */
			gprs.answer();
		}
        else if(NULL != (s = strstr(gprsBuffer, "+CMTI: \"SM\""))) 
		{ 
            /* Incoming SMS */
            messageIndex = atoi(s+12);
            gprs.readSMS(messageIndex, message, MESSAGE_LENGTH);
            INFO("Received SMS");
			INFOV(message);

            process_message(message);
            
        }
        gprs.cleanBuffer(gprsBuffer,32);
    }
    else 
	{
        /* Go to sleep */
		INFO("Going to sleep now!");
		delay(100);
		int sleep_ms = Watchdog.sleep(8000);
		INFO("I'm awake now!");	
    }
}

void setup_pir()
{
    pinMode(PIRMOTIONSENSOR, INPUT);
    pinMode(PIRLED,OUTPUT);
}

void turnOnPIRLED()
{
    digitalWrite(PIRLED,HIGH);
}
void turnOffPIRLED()
{
    digitalWrite(PIRLED,LOW);
}

boolean isPeopPIRLEDetected()
{
    /* Detect whether anyone moves in it's detecting range */
	
	int sensorValue = digitalRead(PIRMOTIONSENSOR);
    if(sensorValue == HIGH)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void setup_gprs() 
{
    int init;
    gprs.preInit();
    delay(1000);
    while(0 != (init = gprs.init())) {
        delay(1000);
        ERROR("GPRS initialization error - (");
        ERRORV(init);
        ERROR(") \r\n");
    }
    INFO("GPRS initialization successful!");

    // Get current date in format yy/MM/dd,hh:mm:ss
    char tmBuffer[sizeof("yy/MM/dd,hh:mm:ss")];
    gprs.getCurrentTime(tmBuffer);
    INFOV(tmBuffer);

    // Send message
    INFO("Init success, sending message to administrator only");
    send_message(MSG("GPRS initialization successful!"), 0);
 
}

void process_message(char *message)
{
  if(NULL != strstr(message,"ALIVE"))
  {
    /* I am ALIVE, ready to respond back */
	/* In future this could be sent only to a requester */
    INFO("Sending confirmation ... I am ALIVE, ready to respond back!");
	if (core.enabPIRLED) 
	{
		send_message(MSG("I am alive! Alarm enabPIRLED!"), 1);
	}
	else
	{
		send_message(MSG("I am alive! Alarm disabPIRLED!"), 1);
	}
  }
  
  if(NULL != strstr(message,"DISABLE"))
  {
    /* Disable alarm*/
	core.enabPIRLED = 0;
	INFO("Sending confirmation ... Alarm disabPIRLED!");
	send_message(MSG("Alarm disabPIRLED!"), 1);
	
  }
  
  if(NULL != strstr(message,"ENABLE"))
  {
    /* Enable alarm*/
	core.enabPIRLED = 1;
	INFO("Sending confirmation ... Alarm enabPIRLED!");
	send_message(MSG("Alarm enabPIRLED!"), 1);
	
  }
  
  if(NULL != strstr(message,"RESET"))
  {
    /* Reset alarm */
	core.value = 0;
	core.enabPIRLED = 1;
	INFO("Sending confirmation ... Reseting alarm!");
	send_message(MSG("Alarm reset!"), 1);
  }
  
  if(NULL != strstr(message,"PARAMS"))
  {
    /* Setting parameters */
	
  }
  
  if(NULL != strstr(message,"STATUS"))
  {
    /* Full status */
	
  }
}

void surveillance()
{
	/* Trigger alarm */
	if (core.value > core.threshold)
	{
		send_message(MSG("Attention!!! Alarm triggered!"), 1);
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
	
	/* Battery low detection */
	/* TODO */
}
 

void send_message(char *message, uint8_t roleThrs)
{
	/* Send message via SMS */
	struct phone *p;
	
	/* Loop through the phone numbers */
	p = phonebook;
	
	while (phonebook != NULL)
	{
		if (p->role <= roleThrsh)
		{
			/* Send it only to desired role */
			gprs.sendSMS(p->number,message);	
		}
		
		/* Next one */
		p = p->next;
	}
} 
 
#define MONITORTHRESHOLD 20
#define MONITORSTEP		 1 
#define MONITORFORGET	 0.001

struct monitor
{
	/* Parameters */
	float threshold;
	float step;
	float forget;	
	
	/* Status */
	uint8_t enabPIRLED;
	float value;
};

/* Core monitor */
struct monitor core;

void setup_monitor()
{
	/* Setup the monitor */
	core->threshold = MONITORTHRESHOLD;
	core->step = MONITORSTEP;
	core->forget = MONITORFORGET;
	
	core->enabPIRLED = 1;
	core->value = 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR(x) 	Serial.println(F(x))
#define ERRORV(x) 	Serial.println(x)
#define INFO(x)		Serial.println(F(x))
#define INFOV(x)	Serial.println(x)
#define MSG(x) 		F(x)

#define PHONENUMBERLENGTH 13

#define SDCARDCHIPSELECT SS;

SdFat sd;

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

int load_phonebook(char *file)
{
	/* Loading phone book from a file */
	char buffer[PHONENUMBERLENGTH];
	SdFile f(file, O_READ);
	
	INFO("Loading phone numbers");
	
	// Check for open error
    if (!rdfile.isOpen()) 
	{
		ERROR("Cannot open file on SD card!");
	}
	
    while (f.fgets(buffer, PHONENUMBERLENGTH) == NULL) 
	{
		if (line[0] != '\n' && line[0] != '\r' && line[0] != '#')
	    {
			/* Add it to phone book */
			struct phone *n;
			n = (struct phone *) malloc(sizeof(struct phone));
			
			if (n == NULL)
			{
				ERROR("Cannot allocate memory for a new phone number!");
			}
			
			strncpy(n->number,buffer,PHONENUMBERLENGTH);
			INFOV(n->number);
			if (phonebook == NULL)
			{
				phonebook = n;
				last = n;
				n->role = 0; /* Admin */
			}
			else
			{
				n->role = 1;
				last->next = n;
				last = n;
			}
			
		}
    }
	f.close();
}


void setup_sdcard()
{
  /* Setup SD Card */
  /* Initialize at the highest speed supported by the board that is
     not over 50 MHz. Try a lower speed if SPI errors occur. */
  
  if (!sd.begin(SDCARDCHIPSELECT, SD_SCK_MHZ(50))) 
  {
    ERROR("Cannot initialize SD card!");
  }
}

/* Sleep functions */

#define DTR800 11

void sleep_gprs()
{
   digitalWrite(DTR800,HIGH);
   
   gprs.serialSIM800.println("AT+CSCLK=1");
   delay(100);
   gprs.serialSIM800.println();
}

void wakeup_gprs()
{
	digitalWrite(DTR800,LOW);
	delay(10);
	gprs.serialSIM800.println();
	gprs.serialSIM800.println("AT+CSCLK=0");
}


