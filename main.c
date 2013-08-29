/*
 * main.c
 *
 *  Created on: Oct 30, 2012
 *      Author: cls
 */


#include <avr/interrupt.h>
#include <stdint.h>

#define TE             834/2                       //<! half bit time = 417 usec
#define SET_DALI_TIMER_COMPARE_WAIT_ONE()  (OCR1A = TE)                 //!< Sets the timer compare register to one period.

#define RESET_DALI_TIMER()              (TCNT1 = 0x00)                  //<! Clear Timer1
#define DISABLE_DALI_INTERRUPT()        (TIMSK1 &= ~(1<< OCIE1A))       //<! Disable Output Compare A Match interrupt
#define ENABLE_DALI_INTERRUPT()         (TIMSK1 |= 1<< OCIE1A)          //<! Enable Output Compare A Match interrupt
#define CLEAR_DALI_TIMER_ON_COMPARE_MATCH()   (TCCR1B |= (1<< WGM12))   //<! Set timer control register to clear timer on compare match (CTC).

#define CLEAR_DALI_PENDING_CAPTURE()    (TIFR1 |= _BV(ICF1))            //<! Reset any pending captures
#define CLEAR_DALI_PENDING_COMPARE()    (TIFR1 |= _BV(OCF1A))           //<! Reset any pending compare timer1 interrupts
#define ENABLE_DALI_CAPT_INTR()         (TIMSK1 |= 1<< ICIE1)           //<! Enable capture interrupts
#define DISABLE_DALI_CAPT_INTR()        (TIMSK1 &= ~(1<< ICIE1))        //<! Disable capture interrupts
#define SET_DALI_FALLING_EDGE()         (TCCR1B &= ~(1<< ICES1))        //<! Set capture for the next falling edge
#define SET_DALI_RAISING_EDGE()         (TCCR1B |= (1<< ICES1))         //<! Set capture for the next raising edge

#define START_DALI_TIMER()              (TCCR1B |= 1<< CS11)            //<! Start Timer1: DIV8 prescaling: Fuse internal 8Mhz Clk => ~1Mhz)
/*(0 << CS12) | (0 << CS11) | (1 << CS10))      // prescale /1 */
#define STOP_DALI_TIMER()               (TCCR1B &= ~(1<< CS11))         //<! Stop Timer1

#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644A__) || defined (__AVR_ATmega644P__)
/* Set the DALI TX pin as outputs*/
#define SETUP_DALI_TX_DIR  DDRD |= _BV(DDD4)

/* Set the DALI RX pins as input */
#define SETUP_DALI_RX_DIR			\
  {						\
    DDRD &= ~(_BV(DDD3));			\
    DDRD &= ~(_BV(DDD6));			\
  }

#define SET_DALI_TX()       (PORTD |= 1<<PD4)          //<! Set the DALI TX pin high
#define RESET_DALI_TX()     (PORTD &= !(1<<PD4))       //<! Set the DALI TX pin low

#define DALI_RX()           (PIND & (1<<PD3))          //<! Check the DALI RX pin
#endif

uint8_t value = 0;

ISR(TIMER1_COMPA_vect)
{
  if (value)
    {
      asm volatile("sbi %0, %1" : :"I" (_SFR_IO_ADDR(PORTD)) , "I" (PD4)); //SET_DALI_TX(); // DALI output pin high
      value = 0;
      OCR1A = 600;
    }
  else
    {
      asm volatile("cbi %0, %1" : :"I" (_SFR_IO_ADDR(PORTD)) , "I" (PD4)); //RESET_DALI_TX(); // DALI output pin low
      value = 1;
      OCR1A = 260;
    }

}

void
DALI_Init(void)
{
  CLEAR_DALI_TIMER_ON_COMPARE_MATCH();
  RESET_DALI_TIMER();
  SET_DALI_TIMER_COMPARE_WAIT_ONE();

  SETUP_DALI_TX_DIR; /* set DALi TX to output */
  SET_DALI_TX();

  /*TODO: set the CAPTURE input*/
  /* CAPTURE pin is set as input*/
  SETUP_DALI_RX_DIR;

}

int main(void)
{
  DALI_Init();

  SET_DALI_TIMER_COMPARE_WAIT_ONE();    // ~2400 Hz
  DISABLE_DALI_CAPT_INTR();             // disable capture interrupt
  ENABLE_DALI_INTERRUPT();              // Enable interrupt on match, CTC
  RESET_DALI_TIMER();                   // reset timer
  START_DALI_TIMER();                   // enable timer. I assume the interrupts are enabled
  sei();

  while (1)
    ;

}
