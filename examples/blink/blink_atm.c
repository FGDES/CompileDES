/* ************************************************ */
/* CodeGenerator: Target ATmega Microcontroller     */
/* ************************************************ */

/* libFAUDES 2.27e --- Blink_Atm_2016_03 */



/* ************************************************ */
/* * prepend code snippet from configuration      * */


// validated to compile with avr-gcc 4.8.1 
// using "-Os -mmcu=atmega328p"

// clock effective after re-initialisation of prescacler
#define F_CPU 16000000UL  // 16MHz

// std includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// initialise system clock (ATmega328)
void  FCG_initclk(void) {
  CLKPR = (1<<CLKPCE);   // enable prescaler write access
  CLKPR = 0x00;          // no clock prescale, i.e. f_cpu 16MHz
}

// initialise beep pio
void  FCG_initbp(void) {
  DDRC |= (1 << PC5);
}

// run 8bit timer0 at 16kHz to drive 100Hz interrupt (ATmega328)
void  FCG_initt0(void) {
  sei();                             // enable interrupts
  TCNT0=0;                           // reset count
  OCR0A=160;                         // set top to 160
  TIMSK0= (1 << OCIE0A);             // interrupt on match top
  TCCR0A= (1 << WGM01);              // clear-on-top mode
  TCCR0B= (1 << CS00) | (1 << CS02); // run at clk_IO/1024 
}

// system time as 8bit value (updated by 100Hz interrupt)
volatile unsigned char  FCG_systime;

// time stemp of last request
unsigned char  FCG_systime_recent;

// read time elapse since last request
unsigned char  FCG_systime_delta(void) {
  unsigned char now= FCG_systime;
  unsigned char dt= now-FCG_systime_recent;
  FCG_systime_recent+= dt;
  return dt;
}

// duration for which to beep (handled by 100Hz interrupt)
volatile unsigned char FCG_beepcnt=0;

// trigger beep by setting positve duration
void FCG_beep(unsigned char dur) {
  FCG_beepcnt=dur;
}

// 100Hz interrupt driven by timer0
ISR(TIMER0_COMPA_vect){
  // systime 100Hz
  FCG_systime++;
  // beep at 100Hz
  if(FCG_beepcnt!=0) {
    if(! ( PORTC & (1 << PC5) )) {
      PORTC |= (1 << PC5);
    } else {
      PORTC &= ~(1 << PC5);
      FCG_beepcnt--;
    }
  }
}



/* ************************************************ */
/* * declaration of variables and constants       * */

/* executed event (-1<>init, 0<>none) */
static int FCG_exec_event=-1;

/* scheduled event (0<>none) */
static int FCG_sched_event=0;

/* status (0 <> waiting, 1<>executing, 2<>err) */
static int FCG_status=0;

/* parallel state */
static int FCG_parallel_state_0=0;
static int FCG_parallel_state_1=0;
static int FCG_parallel_state_2=0;

/* line levels */
static int FCG_line_level_0;


/* faudes event sets */
unsigned char FCG_pending_events[] = { 0x00, 0x00 };
unsigned char FCG_enabled_events[2];
unsigned char FCG_executable_events[2];

/* timer states */
static int FCG_timer_Tick_cnt= 25;
static char FCG_timer_Tick_run= 0;

/* parse generator */
unsigned char FCG_aux_locenabled[2];
static int FCG_aux_parsetrans;
static int FCG_aux_parseevent;

/* aux maths and stack variables  */
static unsigned char FCG_aux_wordret;
static int FCG_aux_edge;
static int FCG_aux_wordaddr;
static int FCG_aux_bitaddr;
static unsigned char FCG_aux_bitmask;

/* generator [Blink3] */
const int FCG_generator_vector_0[] PROGMEM = 
  { 3, 3, 0, 4, 6, 0, 5, 9, 0, 6, 12, 0, 7, 15, 0, 8, 18, 0, 9, 0, 0 };

/* generator [Delay] */
const int FCG_generator_vector_1[] PROGMEM = 
  { 3, 13, 4, 13, 5, 13, 7, 13, 8, 13, 9, 13, 0, 2, 0, 0 };

/* generator [WaitButton] */
const int FCG_generator_vector_2[] PROGMEM = 
  { 7, 0, 1, 5, 0, 3, 5, 7, 0, 1, 5, 0 };



void FCG_cyclic(void) { 
/* ************************************************ */
/* * executor core cyclic code: begin             * */
/* ************************************************ */



/* ************************************************ */
/* * sense input events                           * */

/* init: reset all line data and generate statics */
if( FCG_exec_event == -1 ) {
  FCG_line_level_0 = ( PINB & ( 1 << PB4 ) );
  if( ! ( FCG_line_level_0 ) ) {
    /* negative value: trigger init event [PushButton] */
    FCG_pending_events[0] |= 0x01;
  };
};

/* normal operation: read lines and detect edges */
if( FCG_exec_event != -1 ) {
  /* read line [PB4] */
  FCG_aux_edge = ( PINB & ( 1 << PB4 ) );
  if( FCG_aux_edge != FCG_line_level_0 ) {
    if( FCG_line_level_0 ) {
      /* negative edge trigger input event [PushButton] */
      FCG_pending_events[0] |= 0x01;
      FCG_sched_event = 0;
    };
    FCG_line_level_0 = FCG_aux_edge;
  };
};



/* ************************************************ */
/* * sense timer elapse                           * */

if( FCG_timer_Tick_run && (FCG_timer_Tick_cnt <= 0) ) {
  FCG_timer_Tick_run = 0;
  FCG_pending_events[0] |= 0x02;
  FCG_sched_event = 0;
};



/* ************************************************ */
/* * update enabled events after execution        * */

/* if event was executed (and on init) */
if( FCG_exec_event != 0 ) {
  
  /* set all to enabled */
  FCG_enabled_events[0] = 0xff;
  FCG_enabled_events[1] = 0x01;
  
  /* setup enabled events by [Blink3] */
  FCG_aux_locenabled[0] = 0x03;
  FCG_aux_locenabled[1] = 0x00;
  FCG_aux_parsetrans = FCG_parallel_state_0;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_0[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 8;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 8;
    FCG_aux_bitmask = (0x01 << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];
  FCG_enabled_events[1] &= FCG_aux_locenabled[1];
  
  /* setup enabled events by [Delay] */
  FCG_aux_locenabled[0] = 0x21;
  FCG_aux_locenabled[1] = 0x00;
  FCG_aux_parsetrans = FCG_parallel_state_1;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_1[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 8;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 8;
    FCG_aux_bitmask = (0x01 << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];
  FCG_enabled_events[1] &= FCG_aux_locenabled[1];
  
  /* setup enabled events by [WaitButton] */
  FCG_aux_locenabled[0] = 0xba;
  FCG_aux_locenabled[1] = 0x01;
  FCG_aux_parsetrans = FCG_parallel_state_2;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_2[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 8;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 8;
    FCG_aux_bitmask = (0x01 << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];
  FCG_enabled_events[1] &= FCG_aux_locenabled[1];
  
  /* dispose event */
  FCG_exec_event = 0;
  FCG_sched_event = 0;
};



/* ************************************************ */
/* * schedule next event to execute               * */

if( FCG_sched_event == 0 ) {
  
  /* * setup candidate set to "pending or internal" */
  FCG_executable_events[0] = FCG_pending_events[0] | 0xfc;
  FCG_executable_events[1] = FCG_pending_events[1] | 0x01;
  FCG_executable_events[0] &= FCG_enabled_events[0];
  FCG_executable_events[1] &= FCG_enabled_events[1];
  
  /* find highest priority event (lowest bitaddress) */
  FCG_sched_event = 0;
  if( FCG_executable_events[0] != 0x00 ) {
    if( ( FCG_executable_events[0] & 0x0f ) != 0 ) {
      if( ( FCG_executable_events[0] & 0x03 ) != 0 ) {
        if( ( FCG_executable_events[0] & 0x01 ) != 0 ) {
          FCG_sched_event = 1;
        } else {
          FCG_sched_event = 2;
        };
      } else {
        if( ( FCG_executable_events[0] & 0x04 ) != 0 ) {
          FCG_sched_event = 3;
        } else {
          FCG_sched_event = 4;
        };
      };
    } else {
      if( ( FCG_executable_events[0] & 0x30 ) != 0 ) {
        if( ( FCG_executable_events[0] & 0x10 ) != 0 ) {
          FCG_sched_event = 5;
        } else {
          FCG_sched_event = 6;
        };
      } else {
        if( ( FCG_executable_events[0] & 0x40 ) != 0 ) {
          FCG_sched_event = 7;
        } else {
          FCG_sched_event = 8;
        };
      };
    };
  };
  if( FCG_sched_event == 0 ) {
    if( FCG_executable_events[1] != 0x00 ) {
      FCG_sched_event = 9;
    };
  };
  
  /* remove scheduled event from pending events */
  if( FCG_sched_event > 0 ) {
    FCG_aux_bitaddr = FCG_sched_event;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 8;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 8;
    FCG_aux_bitmask = (0x01 << FCG_aux_bitaddr);
    FCG_pending_events[FCG_aux_wordaddr] &= ~(FCG_aux_bitmask);
  };
  
  /* detect sync error */
  if( FCG_sched_event == 0 ) {
    FCG_aux_wordret = FCG_pending_events[0];
    FCG_pending_events[1] &= 0x01;
    FCG_aux_wordret |= FCG_pending_events[1];
    if( FCG_aux_wordret != 0x00 ) {
      FCG_status = 2;
    };
  };
  
  /* cancel pending events if no event could be scheduled  */
  if( FCG_sched_event == 0 ) {
    FCG_pending_events[0] = 0x00;
    FCG_pending_events[1] = 0x00;
  };
  
};



/* ************************************************ */
/* * execute scheduled event                      * */

if( FCG_sched_event != 0 ) {
  
  FCG_exec_event = FCG_sched_event;
  FCG_sched_event = 0;
  
  /* execute event for [Blink3] */
  FCG_aux_parsetrans = FCG_parallel_state_0;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_0[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_0 = pgm_read_word( &( FCG_generator_vector_0[FCG_aux_parsetrans] ) );
  };
  
  /* execute event for [Delay] */
  FCG_aux_parsetrans = FCG_parallel_state_1;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_1[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_1 = pgm_read_word( &( FCG_generator_vector_1[FCG_aux_parsetrans] ) );
  };
  
  /* execute event for [WaitButton] */
  FCG_aux_parsetrans = FCG_parallel_state_2;
  while(1) {
    FCG_aux_parseevent = pgm_read_word( &( FCG_generator_vector_2[FCG_aux_parsetrans] ) );
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_2 = pgm_read_word( &( FCG_generator_vector_2[FCG_aux_parsetrans] ) );
  };
  
};



/* ************************************************ */
/* * operate timers                               * */

if( FCG_exec_event > 0 ) {
  switch(FCG_exec_event) {
  case 3:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  case 4:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  case 5:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  case 7:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  case 8:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  case 9:
    FCG_timer_Tick_cnt = 25;
    FCG_timer_Tick_run = 1;
    break;
  default:
    break;
  };
};



/* ************************************************ */
/* * operate outputs                              * */

if( FCG_exec_event > 2 ) {
  switch(FCG_exec_event) {
  case 3:
    /* execute [OnLedA] */
    PORTC |= ( 1 << PC0 );
    break;
  case 4:
    /* execute [OnLedB] */
    PORTC |= ( 1 << PC1 );
    break;
  case 5:
    /* execute [OnLedC] */
    PORTC |= ( 1 << PC2 );
    break;
  case 6:
    /* execute [Beep] */
    FCG_beep(100);;
    break;
  case 7:
    /* execute [OffLedA] */
    PORTC &= ~( 1 << PC0 );
    break;
  case 8:
    /* execute [OffLedB] */
    PORTC &= ~( 1 << PC1 );
    break;
  case 9:
    /* execute [OffLedC] */
    PORTC &= ~( 1 << PC2 );
    break;
  default:
    break;
  };
};




/* ************************************************ */
/* * executor core cyclic code: end               * */
/* ************************************************ */
}; /* end function FCG_cyclic() */



/* ************************************************ */
/* * update timer states                          * */
/* ************************************************ */


void FCG_timerdec(int dec) { 
  if( FCG_timer_Tick_run ) FCG_timer_Tick_cnt -= dec;
};



/* ************************************************ */
/* * initialise input/output pins                 * */
/* ************************************************ */
void FCG_initpio(void) { 
  DDRC |= ( 1 << PC0 ) | ( 1 << PC1 ) | ( 1 << PC2 );
  PORTB |= ( 1 << PB4 );
};


/* ************************************************ */
/* * append code snippet from configuration       * */


int main(void){
  FCG_initclk();
  FCG_initpio();
  FCG_initt0();
  FCG_initbp();
  while(1) {
   FCG_cyclic();
   FCG_timerdec(FCG_systime_delta());
  }
  return 0;
}



/* ************************************************ */
/* CodeGenerator: Generated Code Ends Here          */
/* ************************************************ */
