// Be careful to use a platform-specific conditional include to only make the
// code visible for the appropriate platform.  Arduino will try to compile and
// link all .cpp files regardless of platform.
#if defined(ARDUINO_ARCH_AVR) || defined(__AVR__)

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "WatchdogAVR.h"

#include <Arduino.h>

// Define watchdog timer interrupt.
ISR(WDT_vect)
{
    // Nothing needs to be done, however interrupt handler must be defined to
    // prevent a reset.
}

int WatchdogAVR::enable(int maxPeriodMS) {
    // Pick the closest appropriate watchdog timer value.
    int actualMS;
    _setPeriod(maxPeriodMS, _wdto, actualMS);
    // Enable the watchdog and return the actual countdown value.
    wdt_enable(_wdto);
    return actualMS;
}

void WatchdogAVR::reset() {
    // Reset the watchdog.
    wdt_reset();
}

void WatchdogAVR::disable() {
    // Disable the watchdog and clear any saved watchdog timer value.
    wdt_disable();
    _wdto = -1;
}

int WatchdogAVR::sleep(int maxPeriodMS) {
    // Pick the closest appropriate watchdog timer value.
    int sleepWDTO, actualMS;
    _setPeriod(maxPeriodMS, sleepWDTO, actualMS);

    // Build watchdog prescaler register value before timing critical code.
    uint8_t wdps = ((sleepWDTO & 0x08 ? 1 : 0) << WDP3) |
                   ((sleepWDTO & 0x04 ? 1 : 0) << WDP2) |
                   ((sleepWDTO & 0x02 ? 1 : 0) << WDP1) |
                   ((sleepWDTO & 0x01 ? 1 : 0) << WDP0);

    // The next section is timing critical so interrupts are disabled.
    cli();
    // First clear any previous watchdog reset.
    MCUSR &= ~(1<<WDRF);
    // Now change the watchdog prescaler and interrupt enable bit so the watchdog
    // reset only triggers the interrupt (and wakes from deep sleep) and not a 
    // full device reset.  This is a timing critical section of code that must 
    // happen in 4 cycles.
    WDTCSR |= (1<<WDCE) | (1<<WDE);  // Set WDCE and WDE to enable changes.
    WDTCSR = wdps;                   // Set the prescaler bit values.
    WDTCSR |= (1<<WDIE);             // Enable only watchdog interrupts.
    // Critical section finished, re-enable interrupts.
    //sei();

	Serial.end();
	
    // Disable USB if it exists
		
	  #ifdef USBCON
      USBCON |= _BV(FRZCLK);  //freeze USB clock
      PLLCSR &= ~_BV(PLLE);   // turn off USB PLL
      USBCON &= ~_BV(USBE);   // disable USB
    #endif
	
	
  	// SIM800 goes to sleep
  	sleep_gprs();
  	
  	// Add external interrupt to wake on PIN2
  	attachInterrupt(digitalPinToInterrupt(PIRtp), ISR_PIR, CHANGE);
	
    // Set full power-down sleep mode and go to sleep.
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	  sei();
    sleep_mode();

    // Chip is now asleep!

    // Once awakened by the watchdog execution resumes here.  Start by disabling
    // sleep.
    sleep_disable();
	
  	// Disable external interrupt
  	detachInterrupt(digitalPinToInterrupt(PIRtp));

      
    // Routine to properly wake up
    #ifdef DEBUG
	/* If not in debug mode, the serial won't be available after first sleep */
    delay(100);
	USBDevice.attach();
	delay(100);
    Serial.begin(9600);
    delay(100);
    /* Wait for the serial monitor window */
	#ifdef WAITFORMONITOR
    while (!Serial) { }
    Serial.println("Finished waking up!");
	#endif
    #endif
	
  	// SIM800 wakes up
  	wakeup_gprs(); 

    // Check if the user had the watchdog enabled before sleep and re-enable it.
    if (_wdto != -1) {
        wdt_enable(_wdto);
    }

    // Return how many actual milliseconds were spent sleeping.
    return actualMS;
}

void WatchdogAVR::_setPeriod(int maxMS, int &wdto, int &actualMS) {
    // Note the order of these if statements from highest to lowest  is 
    // important so that control flow cascades down to the right value based
    // on its position in the range of discrete timeouts.
    if ((maxMS >= 8000) || (maxMS == 0)) {
        wdto     = WDTO_8S;
        actualMS = 8000;
    }
    else if (maxMS >= 4000) {
        wdto     = WDTO_4S;
        actualMS = 4000;
    }
    else if (maxMS >= 2000) {
        wdto     = WDTO_2S;
        actualMS = 2000;
    }
    else if (maxMS >= 1000) {
        wdto     = WDTO_1S;
        actualMS = 1000;
    }
    else if (maxMS >= 500) {
        wdto     = WDTO_500MS;
        actualMS = 500;
    }
    else if (maxMS >= 250) {
        wdto     = WDTO_250MS;
        actualMS = 250;
    }
    else if (maxMS >= 120) {
        wdto     = WDTO_120MS;
        actualMS = 120;
    }
    else if (maxMS >= 60) {
        wdto     = WDTO_60MS;
        actualMS = 60;
    }
    else if (maxMS >= 30) {
        wdto     = WDTO_30MS;
        actualMS = 30;
    }
    else {
        wdto     = WDTO_15MS;
        actualMS = 15;
    }
}

void ISR_PIR()
{
  /* Do nothing for now */
  
  /*
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(PIRMOTIONSENSOR));
  */ 	
}

#endif
