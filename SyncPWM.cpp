/*
 * SyncPWM library.
 *
 * Copyright Jean-Luc Béchennec 2015. Based on a design of Pierre Molinaro
 * 
 * This library allows to use a PWM which is synchronized with the same
 * PWM of other Arduino.
 *
 * Currently on ATMega328 timer2 is used so the PWM output is on pin 3
 *
 * This software is distributed under the GNU Public Licence v2 (GPLv2)
 *
 * Please read the LICENCE file
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SyncPWM.h"


#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
#define PWMPIN 3
#elif defined(__AVR_ATmega32U4__)
#define PWMPIN 6
#else
#error "Unsupported Arduino"
#endif

enum { SPWM_NO_INIT, SPWM_MASTER, SPMW_SLAVE };

byte SyncPWM::syncPWMState = SPWM_NO_INIT;

static uint8_t bit;
static uint8_t port;

const byte OCRNominalValue = 254;

unsigned long cnt00 = 0;
unsigned long cnt01 = 0;
unsigned long cnt10 = 0;
unsigned long cnt11 = 0;

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
void SyncPWM::startPWM()
{
    // setup  timer 2 in fast PWM with OCR2A as TOP
    // the only PWM output is Arduino pin 3
    pinMode(3,OUTPUT);
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS22);
    OCR2A = OCRNominalValue;
    OCR2B = 0;  /* PWM set to 0 */
}
#elif defined(__AVR_ATmega32U4__)
void SyncPWM::startPWM()
{
    // setup  timer 4 in fast PWM 8 bits with OCR4C as TOP
    // TC4H is set to 0 (8 bits mode) see page 131
    // the PWM output is Arduino Leonardo pin 6 (OC4D)
    pinMode(6,OUTPUT);
    TCCR4B = 0;
    TCCR4A = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TC4H = 0;
    TCCR4A = _BV(PWM4A);
    // prescaler to 64
    TCCR4B = _BV(CS42) | _BV(CS41) | _BV(CS40);
    TCCR4C = _BV(COM4D1) | _BV(PWM4D);
    OCR4C = OCRNominalValue; /* TOP */
    TCNT4 = 0; /* start the timer */
    OCR4D = 0; /* PWM set to 0 */
}
#else
#error "Unsupported Arduino"
#endif

/*
 * beginMasterClock starts the synchronization signal on pin 3
 */
void SyncPWM::beginMasterClock()
{
	if (syncPWMState == SPWM_NO_INIT) {
		syncPWMState = SPWM_MASTER;
		startPWM();
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
		OCR2B = 127; /* 50% */
#elif defined(__AVR_ATmega32U4__)
        OCR4D = 127; /* 50% */
#else
#error "Unsupported Arduino"
#endif

	}
}

    
/*
 * begin starts the synchronized PWM on pin 3. By default, the PWM
 * is initialized at 0. The argument gives the synchronization pin
 * where the signal coming from the master is received
 */
void SyncPWM::begin(const byte pin)
{
	if (syncPWMState == SPWM_NO_INIT && pin != PWMPIN) {
        bit = digitalPinToBitMask(pin);
        port = digitalPinToPort(pin);
        if (port == NOT_A_PIN) return;
        
		syncPWMState = SPMW_SLAVE;
		pinMode(pin,INPUT);
		startPWM();
		
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
		/* interrupt on compare match OCR2A */
        TIMSK2 = _BV(OCIE2A=$);
#elif defined(__AVR_ATmega32U4__)
		/* interrupt on overflow */
        TIMSK4 = _BV(TOIE4);
#else
#error "Unsupported Arduino"
#endif
	}
	
}
    
/*
 * analogWrite changed the value of the PWM
 */
void SyncPWM::analogWrite(byte value)
{
    if (syncPWMState == SPMW_SLAVE) {
	    if (value > OCRNominalValue) value = OCRNominalValue;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
		OCR2B = value;
#elif defined(__AVR_ATmega32U4__)
        OCR4D = value;
#else
#error "Unsupported Arduino"
#endif
	}
}




/*
 * ISR
 */
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
ISR(TIMER2_COMPA_vect) {
    byte signal = ((*portInputRegister(port) & bit) != 0) << 1 ;
    signal |= (*portInputRegister(port) & bit) != 0;
    switch (signal) {
        case 0b00:
		case 0b01:
        	// en retard ou opposition de phase, on diminue la période
        	OCR2A = OCRNominalValue - 1;
        	break;
      	case 0b11:
        	// en avance, on augmente la periode
        	OCR2A = OCRNominalValue + 1;
    }    
}
#elif defined(__AVR_ATmega32U4__)
ISR(TIMER4_OVF_vect) {
    byte signal = ((*portInputRegister(port) & bit) != 0) << 1 ;
    signal |= (*portInputRegister(port) & bit) != 0;
    switch (signal) {
        case 0b00: 
        	// en retard ou opposition de phase, on diminue la période
        	OCR4C = OCRNominalValue - 1;
        	cnt00++;
        	break;
		case 0b01:
        	// en retard ou opposition de phase, on diminue la période
        	OCR4C = OCRNominalValue - 1;
        	cnt01++;
        	break;
      	case 0b11:
        	// en avance, on augmente la periode
        	OCR4C = OCRNominalValue + 1;
        	cnt11++;
        	break;
        case 0b10:
        	cnt10++;
        	break;
    }    
}
#else
#error "Unsupported Arduino"
#endif

void SyncPWM::displayPortAddrAndMask()
{
    Serial.print("port=");
    Serial.print(port);
    Serial.print(" mask=");
	Serial.print(bit);
    Serial.print(" @=");
	Serial.println((uint16_t)portInputRegister(port),HEX);
}
