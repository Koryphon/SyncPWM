/*
 * SyncPWM library.
 * © Jean-Luc Béchennec 2015. Based on a design of Pierre Molinaro
 * 
 * This library allows to use a PWM which is synchronized with the same
 * PWM of other Arduino.
 *
 * Currently timer2 is used so the PWM output is on pin 3
 */

#include "SyncPWM.h"

enum { SPWM_NO_INIT, SPWM_MASTER, SPMW_SLAVE };

byte SyncPWM::syncPWMState = SPWM_NO_INIT;

static uint8_t bit;
static uint8_t port;

const byte OCR2ANominalValue = 254;


void SyncPWM::startPWM()
{
    // setup  timer 2 in fast PWM with OCR2A as TOP
    // the only PWM output is Arduino pin 3
    pinMode(3,OUTPUT);
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS22);
    OCR2A = 254;
    OCR2B = 0;  /* PWM set to 0 */
}

/*
 * beginMasterClock starts the synchronization signal on pin 3
 */
void SyncPWM::beginMasterClock()
{
	if (syncPWMState == SPWM_NO_INIT) {
		syncPWMState = SPWM_MASTER;
		startPWM();
		OCR2B = 127; /* 50% */
	}
}

    
/*
 * begin starts the synchronized PWM on pin 3. By default, the PWM
 * is initialized at 0. The argument gives the synchronization pin
 * where the signal coming from the master is received
 */
void SyncPWM::begin(const byte pin)
{
	if (syncPWMState == SPWM_NO_INIT && pin != 3) {
        bit = digitalPinToBitMask(pin);
        port = digitalPinToPort(pin);
        if (port == NOT_A_PIN) return;
        
		syncPWMState = SPMW_SLAVE;
		pinMode(pin,INPUT);
		startPWM();
		
		/* interrupt on compare match OCR2A */
        TIMSK2 = _BV(OCIE2A);
	}
}
    
/*
 * analogWrite changed the value of the PWM
 */
void SyncPWM::analogWrite(byte value)
{
    if (syncPWMState == SPMW_SLAVE) {
	    if (value > OCR2ANominalValue) value = OCR2ANominalValue;
		OCR2B = value;
	}
}

/*
 * ISR
 */
ISR(TIMER2_COMPA_vect) {
    byte signal = (*portInputRegister(port) & bit) << 1;
    signal |= (*portInputRegister(port) & bit);
    switch (signal) {
        case 0b00:
		case 0b01:
        	// en retard ou opposition de phase, on diminue la période
        	OCR2A = OCR2ANominalValue - 1;
        	break;
      	case 0b11:
        	// en avance, on augmente la periode
        	OCR2A = OCR2ANominalValue + 1;
    }    
    // acknowledge the interrupt
    TIFR2 = _BV(OCF2A);
}
