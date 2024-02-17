/* ************************************************ */
/* CodeGenerator: Target Kinetis K20 uController    */
/* ************************************************ */

/* CompileDES 3.011 (using libFAUDES 2.28a) */
/* Configuration: Blink_K20_2017_03 */



/* ************************************************ */
/* * prepend code snippet from configuration      * */


// validated to compile with arm-none-eabi-gcc  
// using "-Os -mmcu=cortex-m4 -mthumb -nostdlib -MMD -D__MK20DX256__ -DF_CPU=72000000 "

// std k20 includes (need to link with mk20dx256.o/usb_serial.o)
#include "mk20dx256.h"
#include "usb_serial.h"

// forward declaration of lookup tables
const char* FCG_event_lookup[];
const char* FCG_state_lookup_0[];

// diagnosis support: print string
void print_str(const char* str) {
  while(*str)  usb_serial_putchar(*(str++));
}

// diagnosis support: print int (max 32bit, signed)
void print_int(long int num) {
  char sig= num<0;
  if(sig) num *= -1;
  char str[12];
  int i=12;
  str[--i]=0;
  do {
    unsigned char digit= num % 10;
    str[--i]= digit + '0';
    num = num/10;
  } while((num>0) && (i>1));
  if(sig) str[--i]='-';    
  print_str(str+i);
}

// diagnosis: event execution hook
void report_event(int16_t tidx) {
  print_str("ev [");
  print_str(FCG_event_lookup[tidx]);
  print_str("]\r\n");
}

// dummy actuator "beep"
void beep(int dt) {
  (void) dt;
};



/* ************************************************ */
/* * declaration of variables and constants       * */

/* status (0x00 <> waiting, 0x01<>executing, 0x02<>err) */
static uint32_t FCG_status;

/* external reset */
static int16_t FCG_reset=0;

/* recent event (0<>none) */
static int16_t FCG_recent_event;

/* parallel state */
static int16_t FCG_parallel_state_0;
static int16_t FCG_parallel_state_1;
static int16_t FCG_parallel_state_2;

/* pending input events incl timer elapse */
uint32_t FCG_pending_events[1];

/* events enabled by all generators */
uint32_t FCG_enabled_events[1];

/* executed event (-1<>init, 0<>none) */
static int16_t FCG_exec_event=-1;

/* scheduled event (0<>none) */
static int16_t FCG_sched_event;

/* line levels */
static int16_t FCG_line_level_0;

/* loop flag */
static int16_t FCG_first_loop;

/* timer states */
static int FCG_timer_Tick_cnt;
static char FCG_timer_Tick_run;

/* parse generator */
uint32_t FCG_aux_locenabled[1];
static int16_t FCG_aux_parsetrans;
static int16_t FCG_aux_parseevent;

/* enabled events that can be executed */
uint32_t FCG_aux_executables[1];

/* aux maths and stack variables  */
static uint32_t FCG_aux_wordret;
static int16_t FCG_aux_edge;
static int16_t FCG_aux_wordaddr;
static int16_t FCG_aux_bitaddr;
static uint32_t FCG_aux_bitmask;

/* generator [Blink3] */
const int16_t FCG_generator_states_0[] = 
  { 0, 0, 3, 6, 9, 12, 15, 18 };
const int16_t FCG_generator_transitions_0[] = 
  { 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 7, 0, 8, 6, 0, 9, 1, 0, 7, 5, 0 };

/* generator [Delay] */
const int16_t FCG_generator_transitions_1[] = 
  { 3, 13, 4, 13, 5, 13, 7, 13, 8, 13, 9, 13, 0, 2, 0, 0 };

/* generator [WaitButton] */
const int16_t FCG_generator_transitions_2[] = 
  { 9, 0, 1, 5, 0, 3, 5, 9, 0, 1, 5, 0 };

/* event name lookup table */
const char* FCG_event_lookup[] = {
  "", 
  "PushButton", 
  "Tick", 
  "OnLedA", 
  "OnLedB", 
  "OnLedC", 
  "Beep", 
  "OffLedA", 
  "OffLedB", 
  "OffLedC" };

/* state name lookup for generator [Blink3] */
const char* FCG_state_lookup_0[] = {
  "", 
  "Idle", 
  "sA", 
  "sAB", 
  "sABC", 
  "dAB", 
  "dA", 
  "#7" };



void FCG_cyclic(void) { 
/* ************************************************ */
/* * executor core cyclic code: reset/initialise  * */
/* ************************************************ */


/* ************************************************ */
/* * reset internal state                         * */

/* set internal reset flag */
if( FCG_reset ) {
    FCG_exec_event = -1;
};

/* do reset core */
if( FCG_exec_event == -1 ) {
  /* clear status */
  FCG_status = 0;
  /* set initial state */
  FCG_parallel_state_0 = 1;
  FCG_parallel_state_1 = 0;
  FCG_parallel_state_2 = 0;
  /* clear scheduled event */
  FCG_sched_event = 0;
  /* clear recent event */
  FCG_recent_event = 0;
  /* clear pending/enabled events */
  FCG_pending_events[0] = 0x00UL;
  FCG_enabled_events[0] = 0x00UL;
  /* reset timer */
  FCG_timer_Tick_run = 0;
  FCG_timer_Tick_cnt = 25;
};



/* ************************************************ */
/* * bail out on external reset                   * */

if( FCG_reset ) {
    return;
};



/* ************************************************ */
/* * executor core cyclic code: begin             * */
/* ************************************************ */



/* ************************************************ */
/* * sense input events                           * */

/* init: reset all line data and generate statics */
if( FCG_exec_event == -1 ) {
  FCG_line_level_0 = ( GPIOC_PDIR & ( 1L << 7 ) );
  if( ! ( FCG_line_level_0 ) ) {
    /* negative value: trigger init event [PushButton] */
    FCG_pending_events[0] |= 0x1UL;
  };
};

/* normal operation: read lines and detect edges */
if( FCG_exec_event != -1 ) {
  /* read line [PORTC7] */
  FCG_aux_edge = ( GPIOC_PDIR & ( 1L << 7 ) );
  if( FCG_aux_edge != FCG_line_level_0 ) {
    if( FCG_line_level_0 ) {
      /* negative edge trigger input event [PushButton] */
      FCG_pending_events[0] |= 0x1UL;
      FCG_sched_event = 0;
    };
    FCG_line_level_0 = FCG_aux_edge;
  };
};



/* ************************************************ */
/* * sense timer elapse                           * */

if( FCG_timer_Tick_run && (FCG_timer_Tick_cnt <= 0) ) {
  FCG_timer_Tick_run = 0;
  FCG_pending_events[0] |= 0x2UL;
  FCG_sched_event = 0;
};



/* ************************************************ */
/* * event execution loop                         * */

/* clear status to waiting */
FCG_status = 0;
/* clear event report */
FCG_recent_event = 0;
/* set entry flag */
FCG_first_loop = 1;
while(1) {



/* ************************************************ */
/* * update enabled events after execution        * */

/* if event was executed (and on init) */
if( FCG_exec_event != 0 ) {

  /* set all to enabled */
  FCG_enabled_events[0] = 0x1ffUL;

  /* setup enabled events by [Blink3] */
  FCG_aux_locenabled[0] = 0x3UL;
  FCG_aux_parsetrans = FCG_generator_states_0[FCG_parallel_state_0];
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_0[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 32;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 32;
    FCG_aux_bitmask = (0x1UL << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];

  /* setup enabled events by [Delay] */
  FCG_aux_locenabled[0] = 0x21UL;
  FCG_aux_parsetrans = FCG_parallel_state_1;
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_1[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 32;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 32;
    FCG_aux_bitmask = (0x1UL << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];

  /* setup enabled events by [WaitButton] */
  FCG_aux_locenabled[0] = 0xfaUL;
  FCG_aux_parsetrans = FCG_parallel_state_2;
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_2[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_bitaddr = FCG_aux_parseevent;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 32;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 32;
    FCG_aux_bitmask = (0x1UL << FCG_aux_bitaddr);
    FCG_aux_locenabled[FCG_aux_wordaddr] |= FCG_aux_bitmask;
    FCG_aux_parsetrans += 2;
  };
  /* restrict enabled events */
  FCG_enabled_events[0] &= FCG_aux_locenabled[0];

  /* dispose event */
  FCG_exec_event = 0;
  FCG_sched_event = 0;
};



/* ************************************************ */
/* * schedule next event to execute               * */

if( FCG_sched_event == 0 ) {

  /* setup candidate set to "pending or internal" */
  FCG_aux_executables[0] = FCG_pending_events[0] | 0x1fcUL;
  /* restrict candidate set by "enabled" */
  FCG_aux_executables[0] &= FCG_enabled_events[0];

  /* find highest priority event (lowest bitaddress) */
  FCG_sched_event = 0;
  if( FCG_aux_executables[0] != 0x00UL ) {
    if( ( FCG_aux_executables[0] & 0xffUL ) != 0 ) {
      if( ( FCG_aux_executables[0] & 0xfUL ) != 0 ) {
        if( ( FCG_aux_executables[0] & 0x3UL ) != 0 ) {
          if( ( FCG_aux_executables[0] & 0x1UL ) != 0 ) {
            FCG_sched_event = 1;
          } else {
            FCG_sched_event = 2;
          };
        } else {
          if( ( FCG_aux_executables[0] & 0x4UL ) != 0 ) {
            FCG_sched_event = 3;
          } else {
            FCG_sched_event = 4;
          };
        };
      } else {
        if( ( FCG_aux_executables[0] & 0x30UL ) != 0 ) {
          if( ( FCG_aux_executables[0] & 0x10UL ) != 0 ) {
            FCG_sched_event = 5;
          } else {
            FCG_sched_event = 6;
          };
        } else {
          if( ( FCG_aux_executables[0] & 0x40UL ) != 0 ) {
            FCG_sched_event = 7;
          } else {
            FCG_sched_event = 8;
          };
        };
      };
    } else {
      FCG_sched_event = 9;
    };
  };

  /* remove scheduled event from pending events */
  if( FCG_sched_event > 0 ) {
    FCG_aux_bitaddr = FCG_sched_event;
    FCG_aux_bitaddr--;
    FCG_aux_wordaddr = FCG_aux_bitaddr / 32;
    FCG_aux_bitaddr = FCG_aux_bitaddr % 32;
    FCG_aux_bitmask = (0x1UL << FCG_aux_bitaddr);
    FCG_pending_events[FCG_aux_wordaddr] &= ~(FCG_aux_bitmask);
  };

  /* detect sync error */
  if( FCG_sched_event == 0 ) {
    FCG_aux_wordret = FCG_pending_events[0];
    if( FCG_aux_wordret != 0x00UL ) {
      FCG_status |= 0x2UL;
    };
  };

  /* cancel pending events if no event could be scheduled  */
  if( FCG_sched_event == 0 ) {
    FCG_pending_events[0] = 0x00UL;
  };

};



/* ************************************************ */
/* * break execution loop                         * */

if( FCG_sched_event == 0 ) break;
if( ! ( FCG_first_loop ) ) {
  if( FCG_sched_event > 2 ) break;
};
FCG_first_loop = 0;



/* ************************************************ */
/* * execute scheduled event                      * */

if( FCG_sched_event != 0 ) {

  FCG_exec_event = FCG_sched_event;
  FCG_sched_event = 0;
  FCG_status |= 0x1UL;

  /* execute event for [Blink3] */
  FCG_aux_parsetrans = FCG_generator_states_0[FCG_parallel_state_0];
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_0[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_0 = FCG_generator_transitions_0[FCG_aux_parsetrans];
  };

  /* execute event for [Delay] */
  FCG_aux_parsetrans = FCG_parallel_state_1;
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_1[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_1 = FCG_generator_transitions_1[FCG_aux_parsetrans];
  };

  /* execute event for [WaitButton] */
  FCG_aux_parsetrans = FCG_parallel_state_2;
  while(1) {
    FCG_aux_parseevent = FCG_generator_transitions_2[FCG_aux_parsetrans];
    if( FCG_aux_parseevent == FCG_exec_event ) break;
    if( FCG_aux_parseevent == 0 ) break;
    FCG_aux_parsetrans += 2;
  };
  if( FCG_aux_parseevent > 0 ) {
    FCG_aux_parsetrans++;
    FCG_parallel_state_2 = FCG_generator_transitions_2[FCG_aux_parsetrans];
  };

  /* record */
  FCG_recent_event = FCG_exec_event;
  /* call hooks */
  report_event(FCG_exec_event);

};



/* ************************************************ */
/* * operate timers                               * */

if( FCG_exec_event > 0 ) {
  switch(FCG_exec_event) {
  case 3:
  case 4:
  case 5:
  case 7:
  case 8:
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
    /* outputs for [OnLedA] */
    GPIOB_PSOR = ( 1L << 0 );
    break;
  case 4:
    /* outputs for [OnLedB] */
    GPIOD_PSOR = ( 1L << 1 );
    break;
  case 5:
    /* outputs for [OnLedC] */
    GPIOC_PSOR = ( 1L << 0 );
    break;
  case 6:
    /* outputs for [Beep] */
    beep(100);
    break;
  case 7:
    /* outputs for [OffLedA] */
    GPIOB_PCOR = ( 1L << 0 );
    break;
  case 8:
    /* outputs for [OffLedB] */
    GPIOD_PCOR = ( 1L << 1 );
    break;
  case 9:
    /* outputs for [OffLedC] */
    GPIOC_PCOR = ( 1L << 0 );
    break;
  default:
    break;
  };
};



/* ************************************************ */
/* * end execution loop                           * */

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
  PORTB_PCR0 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
  GPIOB_PDDR |= 0x1UL;
  PORTC_PCR0 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
  GPIOC_PDDR |= 0x1UL;
  PORTD_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
  GPIOD_PDDR |= 0x2UL;
  PORTC_PCR7 = PORT_PCR_PFE | PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_MUX(1);
};


/* ************************************************ */
/* * append code snippet from configuration       * */


// initialise monitor pio
void init_mpio(void) {
  //PC5, output, red led on Teensy   
  PORTC_PCR5 =  PORT_PCR_MUX(1);
  GPIOC_PDDR |= (1 << 5);
  //PC6, input with internal pullup, right keyswitch on FGDES board
  PORTC_PCR6 = PORT_PCR_PFE | PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_MUX(1); 
}  

// initialise timer3 (run at F_BUS=36MHz with 20sec period)
void init_timer3(void) {
  SIM_SCGC6 |= SIM_SCGC6_PIT;
  PIT_MCR = 0x00;
  PIT_LDVAL3 =  20L * F_BUS; // 20sec overflow
  PIT_TCTRL3 |= 0x01;        // start Timer 3 
}

// elapsed time in ticks (F_BUS ticks at 36MHz)
inline unsigned long time_elapsed3(void) {
  static unsigned long recent = 0 ;
  unsigned long now = PIT_CVAL3;
  if(recent < now) recent += 20L * F_BUS;
  unsigned int inc = recent - now;
  recent = now;
  return inc;
}

// elapsed time in 10ms ticks
inline unsigned long time_elapsed10ms(void) {
  static unsigned long recent = 0 ;
  unsigned long now = PIT_CVAL3;
  unsigned long inc = 0;
  if(recent < now) recent += 20L * F_BUS;
  if(recent - now >= (F_BUS / 100)) {
    inc=(recent-now)/(F_BUS/100);
    recent -= inc * (F_BUS / 100);
  }
  return inc;
}

// entry point
int main(void) {
  // init support
  init_mpio();
  init_timer3();

  // init payload
  FCG_initpio();

  // system-time to operate with 10ms ticks and an intended overflow at 24h
  unsigned long systime = 0;
  const unsigned long systime_sup = 100L*3600L*24L;

  // min/max performance monitor
  long max_c=-1;
  long min_c=-1;

  // loop forever
  while(1) {

    // track performance in timer3 ticks (F_BUS=36MHz)
    unsigned int elapsed = time_elapsed3();
    if( (elapsed < min_c) || (min_c==0) ) min_c=elapsed;
    if( (elapsed > max_c) && (max_c>=0) ) max_c=elapsed;
    if(min_c<0) min_c=0;
    if(max_c<0) max_c=0;
    
    // update faudes 100Hz timers and systime
    unsigned int elapsed10ms = time_elapsed10ms();
    if(elapsed10ms) {
      FCG_timerdec(elapsed10ms);
      systime+=elapsed10ms;
      if(systime>=systime_sup)
	systime -= systime_sup;
    }

    // systime flash on Teensy led at 1Hz
    if(systime % 100 < 5) {
      GPIOC_PSOR = (1 << 5);
    } else {
      GPIOC_PCOR = (1 << 5);
    };

    // payload
    FCG_cyclic();

    // report state on (potential) change
    if(FCG_recent_event>0) {
      print_str("st [");
      print_str(FCG_state_lookup_0[FCG_parallel_state_0]);
      print_str("]\r\n");
    }  

    // trigger monitor output
    int monitor_trigger=0;

    // trigger by negative edge on keyswitch PC6 with 2ms debounce
    static int keyswitch_lowcyc=0;
    int keyswitch_line= GPIOC_PDIR & ( 1L << 6 );
    if(keyswitch_line) keyswitch_lowcyc=0;
    if( (!keyswitch_line) && (keyswitch_lowcyc>=0) ) keyswitch_lowcyc+=elapsed10ms;
    if(keyswitch_lowcyc>2) monitor_trigger=1;
    if(monitor_trigger) keyswitch_lowcyc=-1;

    // trigger periodically everey 10 secs
    static unsigned long monitor_scheduled=0;
    if(systime >= monitor_scheduled) { 
      monitor_scheduled+=10*100;
      monitor_trigger=1;
    }  
    if(systime+10*100 < monitor_scheduled)
      monitor_scheduled -= systime_sup;

    // no monitor
    if(!monitor_trigger) continue;

    // monitor
    print_str("monitor:   systime=");
    print_int(systime*10);
    print_str("[ms]");
    if(max_c>0) {
      print_str("  cycletime=");
      print_int(min_c/(F_BUS/1000000));
      print_str("/");
      print_int(max_c/(F_BUS/1000000));
      print_str("[min/max usecs]");
    }
    print_str("\r\n");
    min_c=-1;
    max_c=-1;
  }

  // never get here
  return 0;
}



/* ************************************************ */
/* CodeGenerator: Generated Code Ends Here          */
/* ************************************************ */
