#ifndef HOME_SECURITY_SEEEDUINO_GPRS_H
#define HOME_SECURITY_SEEEDUINO_GPRS_H

/* Macros */
#define ERROR(x)    Serial.println(F(x))
#define ERRORV(x)   Serial.println(x)
#define INFO(x)     Serial.println(F(x))
#define INFOV(x)    Serial.println(x)
#define MSG(x)      F(x)

/* PIR */
#define PIRMOTIONSENSOR   2
#define PIRLED            13 

/* Core Monitor */
#define MONITORTHRESHOLD  20
#define MONITORSTEP       1 
#define MONITORFORGET     0.1

/* SIM800 */
#define DTR800            11
#define MAXBUFFERLENGTH   255
#define PHONENUMBERLENGTH 14

/* SD card */
#define SDCARDCHIPSELECT  5

/* Debug */
#define DEBUG
// #define NOSLEEP
#define NOMESSAGE 

/********************************************************************************/
/* Data types                                                                   */
/********************************************************************************/

/* Core monitor */
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

/* Dynamic memory phone book */
struct phone
{
  char number[PHONENUMBERLENGTH];
  uint8_t role;
  /* Pointer to next item */
  struct phone *next;
};

/********************************************************************************/
/* Functions                                                                    */
/********************************************************************************/
void sleep_gprs();
void wakeup_gprs();

#endif
