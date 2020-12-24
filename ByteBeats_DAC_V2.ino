/*
                              ByteBeat DAC
                         Vernon Billingsley 2020

                    A program to generate byte beats using a  
                    74HC595 serial to parallel shift register  
                    and 8 bit R2R dac.... Also a test platform 
                    for a non standard,terminated  DAC and 
                    differential op amp output.

   Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission
    notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

*/

/************************* Defines ********************************/
#define DEBUG 0

#if DEBUG == 1
#define dprint(expression) Serial.print("# "); Serial.print( #expression ); Serial.print( ": " ); Serial.println( expression )
#define dshow(expression) Serial.println( expression )
#else
#define dprint(expression)
#define dshow(expression)
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define dataPin 11    /*Pin 14 on the 74HC595 */
#define clockPin 12   /*Pin 12 on the 74HC595 */
#define latchPin 8    /*Pin 11 on the 74HC595 */
#define syncPin 9


/************************** Variables *****************************/
unsigned long t;
boolean next_byte = false;
boolean clkUp = false;


/**************************  Functions ****************************/
void updateShiftRegister(byte dac_value) {
  /*Take the latch pin LOW */
  PORTB &= ~_BV (0);
  /*Send the data byte */
  shiftOut(dataPin, clockPin, MSBFIRST, dac_value);
  /*Take the Latch pin HIGH to send the data from the register to the dac */
  PORTB |= _BV (0);

}

/******************************************************************/
/*************************** Setup ********************************/
/******************************************************************/
void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }
  /************************* Setup Pins ***************************/
  /*Latch Pin 8 as OUTPUT */
  DDRB |= _BV (0);
  /*Sync Pin 9 as OUTPUT */
  DDRB |= _BV (1);
  /*Data Pin 11 as OUTPUT */
  DDRB |= _BV (3);
  /*Clock Pin 12 as OUTPUT */
  DDRB |= _BV (4);

  /*************************  Setup Timer1 ************************/
  cli();                /*  stop interrupts */
  /*set timer1  */
  TCCR1A = 0;/* set entire TCCR1A register to 0 */
  TCCR1B = 0;/* same for TCCR1B */
  TCNT1  = 0;/* initialize counter value to 0 */
  /* set compare match register */
  OCR1A = 1999; /*8000 Hz */
  /* turn on CTC mode */
  sbi(TCCR1B, WGM12);
  /*  Set prescaler to 1 */
  cbi(TCCR1B, CS12);
  cbi(TCCR1B, CS11);
  sbi(TCCR1B, CS10);
  /* enable timer compare interrupt */
  sbi(TIMSK1, OCIE1A);
  sei();                /*  allow interrupts  */


}/**************************  End Setup **************************/


ISR(TIMER1_COMPA_vect) {
  /* Clock the sync pin at 4 kHz  */
  clkUp = !clkUp;
  if (clkUp) {
    PORTB |= _BV (1); // digitalWrite (9, HIGH);
  } else {
    PORTB &= ~_BV (1); // digitalWrite (9, LOW);
  }
  /*  Send the next byte */
  next_byte = true;
}


/******************************************************************/
/**************************** Loop ********************************/
/******************************************************************/
void loop() {
  if (next_byte) {
    /* Each bytebeat is a single line...Comment out, uncomment, */
    /* or write your own...End with a " ; "                     */
    unsigned long temp_t =
      /*One I'm working on */
      //((t << 3) ^ (t * 2 >> 1) + (t * 2 >> 5) & (t >> 10)) | (t >> 2) ^ (t >> 4) | (t >> 4) + 255 | (t * 3) >> 11 & ((t >> 16) + 1);
    /*A couple from the internet I found interresting */
    //((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7;
    ((((t/(2+((t>>17)&3)))&(((t>>11)&1)|((t>>12)&2)|((t>>9)&4)))|((t/(2+((t>>15)&3)))&(((t>>10)&1)|((t>>8)&2)|((t>>11)&4))))&7)*32;


    /* Send the byte to the dac */
    updateShiftRegister(temp_t);
    /*  increment t */
    t++;
    /*  Reset the boolean */
    next_byte = false;

  }

}/*************************** End Loop *****************************/
