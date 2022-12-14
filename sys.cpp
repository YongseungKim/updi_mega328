/*
 * sys.cpp
 *
 * Created: 02-10-2018 13:07:52
 *  Author: JMR_2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include "sys.h"
#include "UPDI_lo_lvl.h"
#include "dbg.h"
#include <stdio.h>
#include <string.h>


#include <stdio.h>
#include <string.h>



void SYS::init(void) {
  #ifdef DEBUG_ON
    DBG::initDebug();
  #endif

  #ifdef XAVR
  #ifdef __AVR_DA__
  #if (F_CPU == 24000000)
  /* No division on clock */
  _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x09<< CLKCTRL_FREQSEL_gp ));

  #elif (F_CPU == 20000000)
  /* No division on clock */
  _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x08<< CLKCTRL_FREQSEL_gp ));

  #elif (F_CPU == 16000000)
  /* No division on clock */
  _PROTECTED_WRITE(CLKCTRL_OSCHFCTRLA, (CLKCTRL_OSCHFCTRLA & ~CLKCTRL_FREQSEL_gm ) | (0x07<< CLKCTRL_FREQSEL_gp ));
  #else
  #error "F_CPU defined as an unsupported value"
  #endif
  #else //0-series or 1-series
  _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);
  #endif
  #else
  #if defined(ARDUINO_AVR_LARDU_328E)
  #include <avr/power.h>
  clock_prescale_set ( (clock_div_t) __builtin_log2(32000000UL / F_CPU));
  #endif
  PORT(UPDI_PORT) = 1<<UPDI_PIN;
  #endif


  DDR(LED_PORT) |= (1 << LED_PIN);
  #ifdef LED2_PORT
  DDR(LED2_PORT) |= (1 << LED2_PIN);
  #endif
  #ifndef DISABLE_HOST_TIMEOUT
  TIMER_HOST_MAX=HOST_TIMEOUT;
  #endif
  #ifndef DISABLE_TARGET_TIMEOUT
  TIMER_TARGET_MAX=TARGET_TIMEOUT;
  #endif
  #if defined(DEBUG_ON)
  DBG::debug(0x18,0xC0,0xFF, 0xEE);
  #endif

  #if defined(__AVR_ATmega328P__)
  // all programming
  PORTD |= 0b00111100; // pullup Arduino pins D2-5
  // standard programming only
  #if not defined (USE_HV_PROGRAMMING)
  PORTB |= 0b00011111; // pullup Arduino pins D8-12
  PORTC |= 0b00111111; // pullup Arduino pins A0-5
  #else // hv programming only
  // Dickson charge pump - Bit 4,3,2,1,0: HVPWR4 Power, HVSD3 Shutdown, HVCP2 Clock, HVCP1 Clock, HVLED
  DDRB |=   0b00011111; // configure HVPWR4, HVSD3, HVCP2, HVCP1, HVLED as outputs
  PORTB &= ~0b00011110; // clear HVPWR4, HVSD3, HVCP2, HVCP1
  PORTB |=  0b00001000; // set HVSD3
  #endif

  #elif defined(__AVR_ATmega_Mega__)
  // all programming
  // for working with spare pins common to HV and standard programmers
  // standard programming only
  #if not defined (USE_HV_PROGRAMMING)
  // for working with spare pins on standard programmers that are only used for HV type
  #else // hv programming only
  // Dickson charge pump - Bit 4,3,2,1,0: HVPWR4 Power, HVSD3 Shutdown, HVCP2 Clock, HVCP1 Clock, HVLED
  DDRK |=   0b00011111; // configure HVPWR4, HVSD3, HVCP2, HVCP1, HVLED as outputs
  PORTK &= ~0b00011110; // clear HVPWR4, HVSD3, HVCP2, HVCP1
  PORTK |=  0b00001000; // set HVSD3
  #endif

  #elif defined(__AVR_ATtiny_Zero_One__)
  // all programming
  #if defined(__AVR_ATtinyxy6__) || defined(__AVR_ATtinyxy7__)
  PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN3CTRL = PORT_PULLUPEN_bm;
  #elif defined(__AVR_ATtinyxy7__)
  PORTB.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN7CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN5CTRL = PORT_PULLUPEN_bm;
  #endif
  // standard programming only
  #if not defined (USE_HV_PROGRAMMING)
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
  #else // hv programming only
  // Output Pins
  PORTA.DIRSET = PIN2_bm | cpp | PIN6_bm | PIN7_bm; // Power Switch, ChargePumpPower-HVenable, LED2-HVLED, LED
  PORTA.OUTSET = PIN2_bm;   // enable power switch
  PORTA.DIRSET = cp1 | cp2; // set charge pump clock1 and clock2 as output
  PORTB.DIRSET = cps;       // set charge pump shutdown pin as output
  PORTB.OUTSET = cps;       // enable charge pump shutdown
  #endif
  
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  // all programming (28 pins or more)
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTF.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTF.PIN1CTRL = PORT_PULLUPEN_bm;
  #if defined(__AVR_32PIN__) || defined(__AVR_48PIN__) || defined(__AVR_64PIN__)
  PORTF.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTF.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTF.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTF.PIN5CTRL = PORT_PULLUPEN_bm;
  #elif defined(__AVR_48PIN__) || defined(__AVR_64PIN__)
  PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN7CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN3CTRL = PORT_PULLUPEN_bm;
  #elif defined(__AVR_64PIN__)
  PORTB.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTB.PIN7CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTE.PIN7CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTG.PIN7CTRL = PORT_PULLUPEN_bm;
  #endif
  // standard programming only
  #if not defined (USE_HV_PROGRAMMING)
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTC.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN0CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN1CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN2CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN4CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTD.PIN7CTRL = PORT_PULLUPEN_bm;
  #else // hv programming only
  // Output Pins
  PORTA.DIRSET = PIN2_bm | PIN6_bm | PIN7_bm; // Power Switch, LED2-HVLED, LED
  PORTA.OUTSET = PIN2_bm;   // enable power switch
  PORTC.DIRSET = cpp | cp1 | cp2 | cps; // set charge pump power, clock1, clock2 and shutdown as output
  PORTC.OUTSET = cps;       // enable charge pump shutdown
  PORTD.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm; // set target power port as output
  PORTD.OUTSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm; // turn on target power
  #endif
  #endif
}

void SYS::setLED(void){
  PORT(LED_PORT) |= 1 << LED_PIN;
}

void SYS::clearLED(void){
  PORT(LED_PORT) &= ~(1 << LED_PIN);
}

void SYS::setVerLED(void){
  #ifdef LED2_PORT
  PORT(LED2_PORT) |= 1 << LED2_PIN;
  #endif
}

void SYS::clearVerLED(void){
  #ifdef LED2_PORT
  PORT(LED2_PORT) &= ~(1 << LED2_PIN);
  #endif
}

void SYS::setHVLED(void){
  #if defined(USE_HV_PROGRAMMING)
  PORT(HVLED_PORT) |= 1 << HVLED_PIN;
  #endif
}

void SYS::clearHVLED(void){
  #if defined(USE_HV_PROGRAMMING)
  PORT(HVLED_PORT) &= ~(1 << HVLED_PIN);
  #endif
}

void SYS::pulseHV(void) {
  #if defined(USE_HV_PROGRAMMING)
  SYS::setHVLED();
  _delay_ms(1); // initial delay after startup
  #if defined(__AVR_ATmega328P__)
  PORTB &= ~0b00001000;   // clear cps
  PORTB |=  0b00010000;   // set cpp
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTB.OUTCLR = cps;     // clear cps
  PORTA.OUTSET = cpp;     // set cpp
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTC.OUTCLR = cps;     // clear cps
  PORTC.OUTSET = cpp;     // set cpp
  #endif
  for (int j = 0; j <= 200; j++) {
    #if defined(__AVR_ATmega328P__)
    PORTB &= ~0b00000100; // clear cp2
    PORTB |=  0b00000010; // set cp1
    #elif defined (__AVR_ATtiny_Zero_One__)
    PORTA.OUTCLR = cp1 | cp2;
    __builtin_avr_nops(4);
    PORTA.OUTSET = cp1;
    #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
    PORTC.OUTCLR = cp1 | cp2;
    __builtin_avr_nops(4);
    PORTC.OUTSET = cp1;
    #endif
    __builtin_avr_nops(9);
    #if defined(__AVR_ATmega328P__)
    PORTB &= ~0b00000010; // clear cp1
    PORTB |=  0b00000100; // set cp2
    #elif defined(__AVR_ATtiny_Zero_One__)
    PORTA.OUTCLR = cp1 | cp2;
    __builtin_avr_nops(4);
    PORTA.OUTSET = cp2;
    #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
    PORTC.OUTCLR = cp1 | cp2;
    __builtin_avr_nops(4);
    PORTC.OUTSET = cp2;
    #endif
    __builtin_avr_nops(9);
  }
  #if defined(__AVR_ATmega328P__)
  PORTB &= ~0b00000100;   // clear cp2
  PORTB &= ~0b00010000;   // clear cpp
  PORTB |=  0b00001000;   // set cps
  PORTD &= ~0b01000000;   // UPDI pullup disabled
  DDRD  &= ~0b01000000;   // UPDI rx enable
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTA.OUTCLR = cp2;     // clear cp2
  PORTA.OUTCLR = cpp;     // clear cpp
  PORTB.PIN0CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  PORTB.DIRSET = PIN0_bm; // UPDI rx enable
  PORTB.OUTSET = cps;     // set cps
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTC.OUTCLR = cp2;     // clear cp2
  PORTC.OUTCLR = cpp;     // clear cpp
  PORTA.PIN3CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  PORTA.DIRSET = PIN3_bm; // UPDI rx enable
  PORTC.OUTSET = cps;     // set cps
  #endif
  _delay_ms(49);          // tri-state duration after HV pulse
  SYS::clearHVLED();
  #endif
}

void SYS::updiTriState(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__)
  DDRD  &= ~0b01000000;   // UPDI rx enable
  PORTD &= ~0b01000000;   // low
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTB.DIRCLR = PIN0_bm; // UPDI rx enable
  PORTB.PIN0CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTA.DIRCLR = PIN3_bm; // UPDI rx enable
  PORTA.PIN3CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  #endif
  #endif
}

void SYS::updiHigh(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__)
  PORTD |= 0b01000000;    // enable pullup
  DDRD  |= 0b01000000;    // UPDI tx enable, goes high
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTB.OUTSET = PIN0_bm; // high
  PORTB.DIRSET = PIN0_bm; // UPDI tx enable
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTA.OUTSET = PIN3_bm; // high
  PORTA.DIRSET = PIN3_bm; // UPDI tx enable
  #endif
  _delay_us(20);
  #endif
}

void SYS::updiIdle(void) {
  #if defined(USE_HV_PROGRAMMING)
  SYS::updiHigh();
  SYS::updiTriState();
  _delay_us(521);
  SYS::updiHigh();
  #endif
}

void SYS::updiInitiate(void) {
  #if defined(USE_HV_PROGRAMMING)
  // Release UPDI Reset and initiate UPDI Enable by driving low (0.7??s) then tri-state
  #if defined(__AVR_ATmega328P__)
  SYS::updiHigh();
  PORTD &= ~0b01000000;  // low
  __builtin_avr_nops(5);
  SYS::updiTriState();
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTB.DIRSET = PIN0_bm; // UPDI tx enable
  PORTB.OUTSET = PIN0_bm; // high
  PORTB.OUTCLR = PIN0_bm; // low
  __builtin_avr_nops(5);
  PORTB.PIN0CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  PORTB.DIRCLR = PIN0_bm; // UPDI rx enable (tri-state)
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTA.DIRSET = PIN3_bm; // UPDI tx enable
  PORTA.OUTSET = PIN3_bm; // high
  PORTA.OUTCLR = PIN3_bm; // low
  __builtin_avr_nops(5);
  PORTA.PIN3CTRL &= ~PORT_PULLUPEN_bm; // UPDI pullup disabled
  PORTA.DIRCLR = PIN3_bm; // UPDI rx enable (tri-state)
  #endif
  _delay_us(521);         // tri-state duration after UPDI Enable trigger pulse
  #endif
}

void SYS::updiEnable(void) {

  #if defined(USE_HV_PROGRAMMING)
  SYS::updiInitiate();
  SYS::updiHigh();
  UPDI::write_key(UPDI::NVM_Prog);
  SYS::updiIdle();
  UPDI_io::put(UPDI::SYNCH);
  SYS::updiHigh();
  SYS::updiTriState();
  _delay_us(521);
  #endif
}

void SYS::setPOWER(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__)
  DDRC |= 0b00111111;     // enable pullups (VTG)
  PORTC |= 0b00111111;    // set as outputs (VTG)
  #elif defined(__AVR_ATmega_Mega__)
  DDRF |= 0b00111111;     // enable pullups (VTG)
  PORTF |= 0b00111111;    // set as outputs (VTG)
  DDRB |= 0b01000000;     // enable pullup (power switch)
  PORTB |= 0b01000000;    // set as output (power switch)
  #elif defined(__AVR_ATtiny_Zero_One__) || defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTA.OUTSET = PIN2_bm; // power switch high
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTD.OUTSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm; // turn on target power port
  #endif
  _delay_us(10);
  #endif
}

void SYS::clearPOWER(void) {
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__)
  DDRC &= 0b11000000;     // disable pullups (VTG)
  PORTC &= 0b11000000;    // set as inputs (VTG)
  #elif defined(__AVR_ATmega_Mega__)
  DDRF &= 0b11000000;     // disable pullups (VTG)
  PORTF &= 0b11000000;    // set as inputs (VTG)
  DDRB &= 0b10111111;     // disable pullup (power switch)
  PORTB &= 0b10111111;    // set as input (power switch)
  #elif defined(__AVR_ATtiny_Zero_One__) || defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTA.OUTCLR = PIN2_bm; // power switch low
  #elif defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  PORTD.OUTCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm; // turn off target power port
  #endif
  #endif
}

void SYS::cyclePOWER(void) {
  #if defined(USE_HV_PROGRAMMING)
  SYS::clearPOWER();
  _delay_ms(115);
  SYS::setPOWER();
  #endif
}

void SYS::checkOVERLOAD(void) {                 // Sense overload on target power port
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega_Mega__)
  ADCSRA = (1 << ADEN);                         // turn ADC on
  ADCSRA |= (1 << ADPS0) | (1 << ADPS2);        // prescaler of 32
  ADMUX = (1 << REFS0) | (1 << ADLAR) | (6 & 0x07); // AVcc reference, 8-bit result, select A6
  uint16_t sum = 0;
  for (int i = 0 ; i < 250 ; i++) {             // totalize 250 8-bit readings
    ADCSRA  |= (1 << ADSC);                     // start a conversion
    while (ADCSRA &  (1 << ADSC));              // wait while busy
    sum += ADCH;                                // totalize ADC result
  }
  #elif defined(__AVR_ATmega_Zero__)
  PORTD_PIN6CTRL = 0x04;                        // disable digital input buffer on PD6
  ADC0_CTRLA = ADC_RESSEL_8BIT_gc;              // 8-bit resolution
  ADC0_CTRLC = 0x54;                            // reduced capacitance, Vdd ref, prescaler of 32
  ADC0_MUXPOS = 0x06;                           // select AIN6
  ADC0_CTRLA |= ADC_ENABLE_bm;                  // turn ADC on
  uint16_t sum = 0;
  for (int i = 0 ; i < 250 ; i++) {             // totalize 250 8-bit readings
    ADC0_COMMAND |= ADC_STCONV_bm;              // start a conversion
    while (ADC0_COMMAND & ADC_STCONV_bm);       // wait while busy
    sum += ADC0_RESL;                           // totalize ADC result
  }
  #elif defined(__AVR_DA__)
  PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital input buffer for PD6
  PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;          // disable pull-up resistor for PD6
  ADC0.MUXPOS = ADC_MUXPOS_AIN6_gc;             // Select ADC channel AIN6 (PD6)
  ADC0.CTRLC = ADC_PRESC_DIV32_gc;              // CLK_PER divided by 32
  VREF.ADC0REF = VREF_REFSEL_VDD_gc;            // VDD as reference
  uint16_t sum = 0;
  for (int i = 0 ; i < 63 ; i++) {              // totalize 63 10-bit readings
    ADC0.CTRLA = ADC_ENABLE_bm                   /* ADC Enable: enabled */
    | ADC_RESSEL_10BIT_gc;                       /* 10-bit mode */
    ADC0.COMMAND = ADC_STCONV_bm;               // start a conversion
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));   // wait while busy
    sum += ADC0_RES;                            // totalize ADC result
  }
  # endif
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega_Mega__) || defined(__AVR_ATmega_Zero__) || defined(__AVR_DA__)
  if ((sum / 250) <= 230) {                     // if voltage on bridged port outputs <= 4.5V
    while (1) {                                 // OVERLOAD (fix circuit then press Reset)
      clearPOWER();                             // turn off target power then flash LED at 4Hz
      setLED();
      _delay_ms(50);
      clearLED();
      _delay_ms(200);
    }
  }
  # endif
  #endif
}

uint8_t SYS::checkHVMODE() {                    // Check HV Programming Mode Switch
  #if defined(USE_HV_PROGRAMMING)
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega_Mega__)
  ADCSRA =  (1 << ADEN);                        // turn ADC on
  ADCSRA |= (1 << ADPS0) | (1 << ADPS2);        // prescaler of 32
  ADMUX = (1 << REFS0) | (1 << ADLAR) | (7 & 0x07); // use AVcc reference, 8-bit result, select A7
  ADCSRA  |= (1 << ADSC);                       // start a conversion
  while (ADCSRA &  (1 << ADSC));                // wait while busy
  return ADCH;                                  // return HV mode jumper setting
  #elif defined(__AVR_ATtiny_Zero_One__)
  PORTA_PIN1CTRL = 0x04;                        // disable digital input buffer for PA1
  ADC0_CTRLA = ADC_RESSEL_8BIT_gc;              // 8-bit resolution
  ADC0_CTRLC = 0x54;                            // reduced capacitance, Vdd ref, prescaler of 32
  ADC0_MUXPOS = 0x01;                           // select AIN1
  ADC0_CTRLA |= ADC_ENABLE_bm;                  // turn ADC on
  ADC0_COMMAND |= ADC_STCONV_bm;                // start a conversion
  while (ADC0_COMMAND & ADC_STCONV_bm);         // wait while busy
  return ADC0_RESL;                             // return HV mode jumper setting
  #elif defined(__AVR_ATmega_Zero__)
  PORTD_PIN7CTRL = 0x04;                        // disable digital input buffer for PD7
  ADC0_CTRLA = ADC_RESSEL_8BIT_gc;              // 8-bit resolution
  ADC0_CTRLC = 0x54;                            // reduced capacitance, Vdd ref, prescaler of 32
  ADC0_MUXPOS = 0x07;                           // select AIN7
  ADC0_CTRLA |= ADC_ENABLE_bm;                  // turn ADC on
  ADC0_COMMAND |= ADC_STCONV_bm;                // start a conversion
  while (ADC0_COMMAND & ADC_STCONV_bm);         // wait while busy
  return ADC0_RESL;                             // return HV mode jumper setting
  #elif defined(__AVR_DA__)
  PORTD.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;  // disable digital input buffer for PD7
  PORTD.PIN7CTRL &= ~PORT_PULLUPEN_bm;          // disable pull-up resistor for PD7
  ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;             // Select ADC channel AIN7 (PD7)
  ADC0.CTRLC = ADC_PRESC_DIV32_gc;              // CLK_PER divided by 32
  VREF.ADC0REF = VREF_REFSEL_VDD_gc;            // VDD as reference
  ADC0.CTRLA = ADC_ENABLE_bm                     /* ADC Enable: enabled */
  | ADC_RESSEL_10BIT_gc;                         /* 10-bit mode */
  ADC0.COMMAND = ADC_STCONV_bm;                 // start a conversion
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));     // wait while busy
  return ADC0.RES >> 2;                         // return HV mode jumper setting
  #else
  return 0;
  # endif
  #else
  return 0;
  #endif
}
