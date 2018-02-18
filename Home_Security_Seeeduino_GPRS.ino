/********************************************************************************/
/* Home security system for Seeeduino GPRS                                      */
/********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gprs.h>
#include <SoftwareSerial.h>

#include <Adafruit_SleepyDog.h>

#include <SPI.h>
#include <SdFat.h> 

#include "Home_Security_Seeeduino_GPRS.h"

/********************************************************************************/
/* Global Variables                                                             */
/********************************************************************************/

/* SIM800 */
GPRS gprs(9600);

/* Core monitor */
struct monitor core;
boolean lastDetection;

/* SD card */
SdFat sd;

/* Phone number */
struct phone *phonebook = NULL;
struct phone *last = NULL;

/* PIR */
const int PIRtp = PIRMOTIONSENSOR;

/********************************************************************************/
/* Functions                                                                    */
/********************************************************************************/

void setup()
{
  boolean success_sdcard;
  /* Main setup */
  Serial.begin(9600);
  delay(10000);
  INFO("Starting setup sequence");
  delay(500);
  
  setup_pir();
    
  success_sdcard = setup_sdcard();
  if (success_sdcard)
  {
    char filename[] = "phonebook.txt";
    load_phonebook(filename);
    //load_phonebook((char *)F("phonebook.txt"));
  }
  else
  {
    /* Print the error first */
    // sd.errorPrint();
    sd.initErrorPrint();
    /* Load backup phone number in case SD card fails */
  }
  
  setup_gprs();
  
  delay(500);
  
  setup_monitor();
  INFO("Completed!");
  delay(200);
}



void loop()
{
  /* Main loop */
  char gprsBuffer[MESSAGE_LENGTH];
  char *s = NULL;
  char message[MESSAGE_LENGTH];
  int messageIndex;
	  
  /* Surveillance via PIR sensor */
  if(isPeopleDetected() && !lastDetection) 
  {
    core.value = core.value + core.step;
    turnOnLED();
    lastDetection = true;
    INFO("Monitor value:");
    INFOV(core.value);
    delay(200);
  }
  else 
  {
    turnOffLED();
    lastDetection = false;
  }
  
  surveillance();
  
  #ifdef DEBUG
  INFOV(core.value);
  delay(200);
  #endif
  
  /* Communication */
  if(gprs.serialSIM800.available()) 
  {
    #ifdef DEBUG
    INFO("Something is available at SIM800");
    delay(1000);
    #endif
    gprs.readBuffer(gprsBuffer, MESSAGE_LENGTH, DEFAULT_TIMEOUT);
    delay(500);
    
    #ifdef DEBUG
    //INFOV(gprsBuffer);
    #endif
    
    if(NULL != strstr(gprsBuffer, "RING")) 
    {
      /* Answer the incoming call - not used */
      gprs.answer();
    }
    else if(NULL != (s = strstr(gprsBuffer, "+CMTI: \"SM\""))) 
    { 
      /* Incoming SMS */
      messageIndex = atoi(s+12);
      #ifdef DEBUG
      INFO("Reading incoming message");
      delay(200);
      #endif
      gprs.readSMS(messageIndex, message, MESSAGE_LENGTH);
      delay(500);
      INFO("Received SMS");
      INFOV(message);

      process_message(message);
      delay(500);

      /* Delete message */
      char at[MAXBUFFERLENGTH];
      sprintf(at,"AT+CMGD=%d",messageIndex);
      gprs.sendCmd(at);
      delay(500);
      
    }
    gprs.cleanBuffer(gprsBuffer,MESSAGE_LENGTH);
    delay(500);
  }
  else 
  {
    /* Go to sleep */
    #ifndef NOSLEEP
    INFO("Going to sleep now!");
    delay(200);
    int sleep_ms = Watchdog.sleep(8000);
    INFO("I'm awake now!");	
    #endif
  }
}

void setup_pir()
{
  pinMode(PIRMOTIONSENSOR, INPUT);
  pinMode(PIRLED,OUTPUT);
}

void turnOnLED()
{
  digitalWrite(PIRLED,HIGH);
}

void turnOffLED()
{
  digitalWrite(PIRLED,LOW);
}

boolean isPeopleDetected()
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
  while(0 != (init = gprs.init())) 
  {
    delay(1000);
    ERROR("GPRS initialization error - (");
    ERRORV(init);
    ERROR(") \r\n");
  }
  INFO("GPRS initialization successful!");
  
  // Get current date in format yy/MM/dd,hh:mm:ss
  /* TODO - getCurrent Time function needs to be ported
  char tmBuffer[sizeof("yy/MM/dd,hh:mm:ss")];
  gprs.getCurrentTime(tmBuffer);
  INFOV(tmBuffer);
  */
  
  // Send message
  INFO("Init success, sending message to administrator only");
  send_message(MSG("GPRS initialization successful!"), 0);
}

void process_message(char *message)
{
        /* AT+CMGD=1,4 */
      /* AT+CPMS="SM" */
  char at[MAXBUFFERLENGTH];
  
  if(NULL != strstr(message,"ALIVE"))
  {
    /* I am ALIVE, ready to respond back */
    /* In future this could be sent only to a requester */
    INFO("Sending confirmation ... I am ALIVE, ready to respond back!");
    if (core.enabled) 
    {
      send_message(MSG("I am alive! Alarm enabled!"), 1);
    }
    else
    {
      send_message(MSG("I am alive! Alarm disabled!"), 1);
    }
  }
  
  if(NULL != strstr(message,"DISABLE"))
  {
    /* Disable alarm*/
    core.enabled = 0;
    INFO("Sending confirmation ... Alarm disabled!");
    send_message(MSG("Alarm disabled!"), 1);
  }
  
  if(NULL != strstr(message,"ENABLE"))
  {
    /* Enable alarm*/
    core.enabled = 1;
    INFO("Sending confirmation ... Alarm enabled!");
    send_message(MSG("Alarm enabled!"), 1);
  }
  
  if(NULL != strstr(message,"RESET"))
  {
    /* Reset alarm */
    core.value = 0;
    core.enabled = 0;
    INFO("Sending confirmation ... Reseting alarm!");
    send_message(MSG("Alarm reset!"), 1);

    sprintf(at,"AT+CMGD=1,4");
    gprs.sendCmd(at);
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
		if (core.enabled)
		{
		  send_message(MSG("Attention!!! Alarm triggered!"), 1);
		}
    else
    {
      INFO("Monitor value exceeded a threshold. Reseting to zero!");
    }
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
			/* Do not allow negative */
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
	
	while (p != NULL)
	{
		if (p->role <= roleThrs)
		{
			/* Send it only to desired role */
      #ifndef NOMESSAGE
			gprs.sendSMS(p->number,message);
      #else
      INFO("Sending message");
      INFOV(message);
      #endif
      delay(500);	
		}
		
		/* Next one */
		p = p->next;
	}
} 

void send_message(const __FlashStringHelper *message, uint8_t roleThrs)
{
  size_t n = strlen_P((const char*)message);
  char buffer[n + 1]; //Size array as needed.
  
  #ifdef DEBUG
  INFO("-> __FlashStringHelper -> ");
  #endif
  
  memcpy_P( buffer, message, n);
  buffer[n] = '\0';
  /* Send the message with original function */
  send_message(buffer,roleThrs);
}

 
void setup_monitor()
{
	#ifdef DEBUG
  INFO("Monitor setup");
  #endif
  
	/* Setup the monitor */
	core.threshold = MONITORTHRESHOLD;
	core.step = MONITORSTEP;
	core.forget = MONITORFORGET;
	
  /* Monitor initially disabled */
	core.enabled = 0;
	core.value = 0;
 
  /* Other */
  lastDetection = false;
}

int load_phonebook(char *file)
{
	/* Loading phone book from a file */
	char buffer[MAXBUFFERLENGTH];
	SdFile f;
	
	INFO("Loading phone numbers");

  f.open(file, O_READ);
	// Check for open error
  if (!f.isOpen()) 
	{
		ERROR("Cannot open file on SD card!");
    ERRORV(file);
    return;
	}
	
  while (f.fgets(buffer, MAXBUFFERLENGTH) > 0) 
	{
		#ifdef DEBUG
		INFOV(buffer);
    #endif
		if (buffer[0] != '\n' && buffer[0] != '\r' && buffer[0] != '#')
	  {
			/* Add it to phone book */
			struct phone *n;
			n = (struct phone *) malloc(sizeof(struct phone));
			
			if (n == NULL)
			{
				ERROR("Cannot allocate memory for a new phone number!");
			}
			
			strncpy(n->number,buffer,PHONENUMBERLENGTH-1);
      n->number[PHONENUMBERLENGTH-1] = '\0';
			INFOV(n->number);
      n->next = NULL;
     
			if (phonebook == NULL)
			{
        n->role = 0; /* Admin */
				phonebook = n;
				last = n;
			}
			else
			{
				n->role = 1;
				last->next = n;
				last = n;
			}
			
		}
    #ifdef DEBUG
    else
    {
      INFO("Skipping");
    }
    #endif
  }
	f.close();
}


boolean setup_sdcard()
{
  /* Setup SD Card */
  /* Initialize at the highest speed supported by the board that is
     not over 50 MHz. Try a lower speed if SPI errors occur. */
  boolean success = false; 
  // SD_SCK_MHZ(50)
  // SPI_HALF_SPEED
  // SPI_EIGHTH_SPEED

  pinMode(SDCARDCHIPSELECT,OUTPUT);   
  digitalWrite(SDCARDCHIPSELECT, LOW);
  
  if (!sd.begin(SDCARDCHIPSELECT, SPI_EIGHTH_SPEED)) 
  {
    ERROR("Cannot initialize SD card!");
  }
  else
  {
    INFO("SD card ready!");
    success = true;
  }
  return success;
}

/* Sleep functions */

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


