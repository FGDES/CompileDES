(* ************************************************ *)
(* CodeGenerator: Target IEC 61131 Structured Text  *)
(* ************************************************ *)

(* CompileDES 3.09 (using libFAUDES 2.28a) *)
(* Configuration: Blink_Geb_2017_02 *)



(* ************************************************ *)
(* * prepend code snippet from configuration      * *)


(* main program: instantiate and invoke cyclic function *)
PROGRAM main

VAR
  stat : BYTE;
  ps   : fcg_parallel_state_t;
  cyc  : fcg_cyclic_fb;
  s0   : STRING;
  s1   : STRING;
  s2   : STRING;
  ev   : INT;
  es   : STRING;
END_VAR

cyc(STATUS => stat, PARALLEL_STATE => ps, RECENT_EVENT => ev);

s0 := fcg_state_lookup_f(0,ps[0]);
s1 := fcg_state_lookup_f(1,ps[1]);
s2 := fcg_state_lookup_f(2,ps[2]);
IF ev > 0 THEN
  es := fcg_event_lookup_f(ev);
END_IF;  

END_PROGRAM



(* ************************************************ *)
(* * array types --- GEB IDE specific             * *)
(* ************************************************ *)
TYPE

(* parallel state *)
fcg_parallel_state_t : ARRAY[0..2] OF INT;

(* pending input events incl timer elapse *)
fcg_pending_events_t : ARRAY[0..1] OF BYTE;

(* events enabled by all generators *)
fcg_enabled_events_t : ARRAY[0..1] OF BYTE;

(* generator [Blink3] *)
fcg_generator_states_0_t : ARRAY[0..7] OF INT := 
  [ 0, 0, 3, 6, 9, 12, 15, 18 ];
fcg_generator_transitions_0_t : ARRAY[0..20] OF INT := 
  [ 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 9, 1, 0 ];

(* generator [Delay] *)
fcg_generator_states_1_t : ARRAY[0..2] OF INT := 
  [ 0, 0, 13 ];
fcg_generator_transitions_1_t : ARRAY[0..15] OF INT := 
  [ 3, 2, 4, 2, 5, 2, 7, 2, 8, 2, 9, 2, 0, 2, 1, 0 ];

(* generator [WaitButton] *)
fcg_generator_states_2_t : ARRAY[0..2] OF INT := 
  [ 0, 0, 5 ];
fcg_generator_transitions_2_t : ARRAY[0..11] OF INT := 
  [ 7, 1, 1, 2, 0, 3, 2, 7, 1, 1, 2, 0 ];

(* mask vectors *)
fcg_wordaddr_vector_t : ARRAY[0..8] OF INT := 
  [ 0, 0, 0, 0, 0, 0, 0, 0, 1 ];
fcg_bitmask_vector_t : ARRAY[0..8] OF BYTE := 
  [ BYTE#16#01, BYTE#16#02, BYTE#16#04, BYTE#16#08, BYTE#16#10, BYTE#16#20, BYTE#16#40, BYTE#16#80, BYTE#16#01 ];

(* other type fixes *)
fcg_aux_locenabled_t : ARRAY[0..1] OF BYTE;
fcg_aux_executables_t : ARRAY[0..1] OF BYTE;


END_TYPE



(* ************************************************ *)
(* * function block to host cyclic code           * *)
(* ************************************************ *)


FUNCTION_BLOCK fcg_cyclic_fb

VAR_INPUT

(* external reset *)
RESET : BOOL := false;

END_VAR

VAR_OUTPUT

(* status (0x00 <> waiting, 0x01<>executing, 0x02<>err) *)
STATUS : BYTE;

(* recent event (0<>none) *)
RECENT_EVENT : INT;

(* pending input events incl timer elapse *)
PENDING_EVENTS : fcg_pending_events_t;

(* events enabled by all generators *)
ENABLED_EVENTS : fcg_enabled_events_t;

(* parallel state *)
PARALLEL_STATE : fcg_parallel_state_t;


END_VAR

VAR

(* executed event (-1<>init, 0<>none) *)
fcg_exec_event : INT := -1;

(* scheduled event (0<>none) *)
fcg_sched_event : INT;

(* line levels *)
fcg_line_level_0 : BOOL;

(* loop flag *)
fcg_first_loop : BOOL;

(* timer states *)
fcg_timer_Tick_cnt : TIME;
fcg_timer_Tick_run : BOOL;

(* elapsed time since last invokation *)
fcg_time_elapsed : TIME;

(* parse generator *)
fcg_aux_locenabled : fcg_aux_locenabled_t;
fcg_aux_parsetrans : INT;
fcg_aux_parseevent : INT;

(* enabled events that can be executed *)
fcg_aux_executables : fcg_aux_executables_t;

(* aux maths and stack variables  *)
fcg_aux_wordret : BYTE;
fcg_aux_edge : BOOL;
fcg_aux_wordaddr : INT;
fcg_aux_bitaddr : INT;
fcg_aux_bitmask : BYTE;

(* mask vectors *)
fcg_wordaddr_vector : fcg_wordaddr_vector_t;
fcg_bitmask_vector : fcg_bitmask_vector_t;

(* generator [Blink3] *)
fcg_generator_states_0 : fcg_generator_states_0_t;
fcg_generator_transitions_0 : fcg_generator_transitions_0_t;

(* generator [Delay] *)
fcg_generator_states_1 : fcg_generator_states_1_t;
fcg_generator_transitions_1 : fcg_generator_transitions_1_t;

(* generator [WaitButton] *)
fcg_generator_states_2 : fcg_generator_states_2_t;
fcg_generator_transitions_2 : fcg_generator_transitions_2_t;

(* iec timer to simulate system time *)
fcg_systime_ton : TON;
fcg_systime_recent : TIME;
fcg_systime_now : TIME;

END_VAR

VAR

(* import physical i/o addresses *)
AT %IX54.1 : BOOL;
AT %QX55.1 : BOOL;
AT %QX55.2 : BOOL;
AT %QX55.3 : BOOL;

END_VAR

(* ************************************************ *)
(* * extra cyclic code from configuration         * *)

(* instantiate beep for action of beep event *)
VAR
  beep : beep_fb;
END_VAR  

(* * end of extra code from configuration         * *)
(* ************************************************ *)


(* ************************************************ *)
(* * executor core cyclic code: reset/initialise  * *)
(* ************************************************ *)


(* ************************************************ *)
(* * reset internal state                         * *)

(* set internal reset flag *)
IF RESET THEN
    fcg_exec_event := -1;
END_IF;

(* do reset core *)
IF fcg_exec_event = -1 THEN
  (* clear status *)
  STATUS := 0;
  (* set initial state *)
  PARALLEL_STATE[0] := 1;
  PARALLEL_STATE[1] := 1;
  PARALLEL_STATE[2] := 1;
  (* clear scheduled event *)
  fcg_sched_event := 0;
  (* clear recent event *)
  RECENT_EVENT := 0;
  (* clear pending/enabled events *)
  PENDING_EVENTS[0] := BYTE#0;
  PENDING_EVENTS[1] := BYTE#0;
  ENABLED_EVENTS[0] := BYTE#0;
  ENABLED_EVENTS[1] := BYTE#0;
  (* reset timer *)
  fcg_timer_Tick_run := false;
  fcg_timer_Tick_cnt := TIME#1s;
END_IF;

(* do reset/track systime *)
IF fcg_exec_event = -1  THEN
  fcg_systime_ton(IN:=false);
  fcg_systime_recent := TIME#0ms;
END_IF;
fcg_systime_ton(IN:=true, PT:=TIME#2h, ET=>fcg_systime_now);
fcg_time_elapsed := SUB_TIME(fcg_systime_now,fcg_systime_recent);
IF fcg_systime_now < TIME#1h THEN
  fcg_systime_recent := fcg_systime_now;
ELSE
  fcg_systime_ton(IN:=false, PT:=TIME#2h);
  fcg_systime_ton(IN:=true, PT:=TIME#2h);
  fcg_systime_recent := TIME#0ms;
END_IF;



(* ************************************************ *)
(* * bail out on external reset                   * *)

IF RESET THEN
    RETURN;
END_IF;



(* ************************************************ *)
(* * update timer states                          * *)


IF fcg_time_elapsed > TIME#0ms THEN
  IF fcg_timer_Tick_run THEN
    IF fcg_timer_Tick_cnt > fcg_time_elapsed THEN
      fcg_timer_Tick_cnt := SUB_TIME(fcg_timer_Tick_cnt, fcg_time_elapsed);
    ELSE
      fcg_timer_Tick_cnt := TIME#0s;
    END_IF;
  END_IF;
END_IF;



(* ************************************************ *)
(* * executor core cyclic code: begin             * *)
(* ************************************************ *)



(* ************************************************ *)
(* * sense input events                           * *)

(* init: reset all line data and generate statics *)
IF fcg_exec_event = -1 THEN
  fcg_line_level_0 := %IX54.1;
  IF NOT fcg_line_level_0 THEN
    (* negative value: trigger init event [PushButton] *)
    PENDING_EVENTS[0] := PENDING_EVENTS[0] OR BYTE#16#01;
  END_IF;
END_IF;

(* normal operation: read lines and detect edges *)
IF fcg_exec_event <> -1 THEN
  (* read line [%IX54.1] *)
  fcg_aux_edge := %IX54.1;
  IF fcg_aux_edge <> fcg_line_level_0 THEN
    IF fcg_line_level_0 THEN
      (* negative edge trigger input event [PushButton] *)
      PENDING_EVENTS[0] := PENDING_EVENTS[0] OR BYTE#16#01;
      fcg_sched_event := 0;
    END_IF;
    fcg_line_level_0 := fcg_aux_edge;
  END_IF;
END_IF;



(* ************************************************ *)
(* * sense timer elapse                           * *)

IF fcg_timer_Tick_run AND (fcg_timer_Tick_cnt <= TIME#0ms) THEN
  fcg_timer_Tick_run := false;
  PENDING_EVENTS[0] := PENDING_EVENTS[0] OR BYTE#16#02;
  fcg_sched_event := 0;
END_IF;



(* ************************************************ *)
(* * event execution loop                         * *)

(* clear status to waiting *)
STATUS := 0;
(* set entry flag *)
fcg_first_loop := true;
WHILE true DO



(* ************************************************ *)
(* * update enabled events after execution        * *)

(* if event was executed (and on init) *)
IF fcg_exec_event <> 0 THEN

  (* set all to enabled *)
  ENABLED_EVENTS[0] := BYTE#16#ff;
  ENABLED_EVENTS[1] := BYTE#16#01;

  (* setup enabled events by [Blink3] *)
  fcg_aux_locenabled[0] := BYTE#16#03;
  fcg_aux_locenabled[1] := BYTE#0;
  fcg_aux_parsetrans := fcg_generator_states_0[PARALLEL_STATE[0]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_0[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_bitaddr := fcg_aux_parseevent;
    fcg_aux_bitaddr := fcg_aux_bitaddr - 1;
    fcg_aux_wordaddr := fcg_wordaddr_vector[fcg_aux_bitaddr];
    fcg_aux_bitmask := fcg_bitmask_vector[fcg_aux_bitaddr];
    fcg_aux_locenabled[fcg_aux_wordaddr] := fcg_aux_locenabled[fcg_aux_wordaddr] OR fcg_aux_bitmask;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  (* restrict enabled events *)
  ENABLED_EVENTS[0] := ENABLED_EVENTS[0] AND fcg_aux_locenabled[0];
  ENABLED_EVENTS[1] := ENABLED_EVENTS[1] AND fcg_aux_locenabled[1];

  (* setup enabled events by [Delay] *)
  fcg_aux_locenabled[0] := BYTE#16#21;
  fcg_aux_locenabled[1] := BYTE#0;
  fcg_aux_parsetrans := fcg_generator_states_1[PARALLEL_STATE[1]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_1[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_bitaddr := fcg_aux_parseevent;
    fcg_aux_bitaddr := fcg_aux_bitaddr - 1;
    fcg_aux_wordaddr := fcg_wordaddr_vector[fcg_aux_bitaddr];
    fcg_aux_bitmask := fcg_bitmask_vector[fcg_aux_bitaddr];
    fcg_aux_locenabled[fcg_aux_wordaddr] := fcg_aux_locenabled[fcg_aux_wordaddr] OR fcg_aux_bitmask;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  (* restrict enabled events *)
  ENABLED_EVENTS[0] := ENABLED_EVENTS[0] AND fcg_aux_locenabled[0];
  ENABLED_EVENTS[1] := ENABLED_EVENTS[1] AND fcg_aux_locenabled[1];

  (* setup enabled events by [WaitButton] *)
  fcg_aux_locenabled[0] := BYTE#16#ba;
  fcg_aux_locenabled[1] := BYTE#16#01;
  fcg_aux_parsetrans := fcg_generator_states_2[PARALLEL_STATE[2]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_2[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_bitaddr := fcg_aux_parseevent;
    fcg_aux_bitaddr := fcg_aux_bitaddr - 1;
    fcg_aux_wordaddr := fcg_wordaddr_vector[fcg_aux_bitaddr];
    fcg_aux_bitmask := fcg_bitmask_vector[fcg_aux_bitaddr];
    fcg_aux_locenabled[fcg_aux_wordaddr] := fcg_aux_locenabled[fcg_aux_wordaddr] OR fcg_aux_bitmask;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  (* restrict enabled events *)
  ENABLED_EVENTS[0] := ENABLED_EVENTS[0] AND fcg_aux_locenabled[0];
  ENABLED_EVENTS[1] := ENABLED_EVENTS[1] AND fcg_aux_locenabled[1];

  (* dispose event *)
  fcg_exec_event := 0;
  fcg_sched_event := 0;
END_IF;



(* ************************************************ *)
(* * schedule next event to execute               * *)

IF fcg_sched_event = 0 THEN

  (* setup candidate set to "pending or internal" *)
  fcg_aux_executables[0] := PENDING_EVENTS[0] OR BYTE#16#fc;
  fcg_aux_executables[1] := PENDING_EVENTS[1] OR BYTE#16#01;
  (* restrict candidate set by "enabled" *)
  fcg_aux_executables[0] := fcg_aux_executables[0] AND ENABLED_EVENTS[0];
  fcg_aux_executables[1] := fcg_aux_executables[1] AND ENABLED_EVENTS[1];

  (* find highest priority event (lowest bitaddress) *)
  fcg_sched_event := 0;
  IF NOT ( fcg_aux_executables[0] = BYTE#0 ) THEN
    IF ( fcg_aux_executables[0] AND BYTE#16#0f ) <> BYTE#0 THEN
      IF ( fcg_aux_executables[0] AND BYTE#16#03 ) <> BYTE#0 THEN
        IF ( fcg_aux_executables[0] AND BYTE#16#01 ) <> BYTE#0 THEN
          fcg_sched_event := 1;
        ELSE
          fcg_sched_event := 2;
        END_IF;
      ELSE
        IF ( fcg_aux_executables[0] AND BYTE#16#04 ) <> BYTE#0 THEN
          fcg_sched_event := 3;
        ELSE
          fcg_sched_event := 4;
        END_IF;
      END_IF;
    ELSE
      IF ( fcg_aux_executables[0] AND BYTE#16#30 ) <> BYTE#0 THEN
        IF ( fcg_aux_executables[0] AND BYTE#16#10 ) <> BYTE#0 THEN
          fcg_sched_event := 5;
        ELSE
          fcg_sched_event := 6;
        END_IF;
      ELSE
        IF ( fcg_aux_executables[0] AND BYTE#16#40 ) <> BYTE#0 THEN
          fcg_sched_event := 7;
        ELSE
          fcg_sched_event := 8;
        END_IF;
      END_IF;
    END_IF;
  END_IF;
  IF fcg_sched_event = 0 THEN
    IF NOT ( fcg_aux_executables[1] = BYTE#0 ) THEN
      fcg_sched_event := 9;
    END_IF;
  END_IF;

  (* remove scheduled event from pending events *)
  IF fcg_sched_event > 0 THEN
    fcg_aux_bitaddr := fcg_sched_event;
    fcg_aux_bitaddr := fcg_aux_bitaddr - 1;
    fcg_aux_wordaddr := fcg_wordaddr_vector[fcg_aux_bitaddr];
    fcg_aux_bitmask := fcg_bitmask_vector[fcg_aux_bitaddr];
    PENDING_EVENTS[fcg_aux_wordaddr] := PENDING_EVENTS[fcg_aux_wordaddr] AND (NOT fcg_aux_bitmask );
  END_IF;

  (* detect sync error *)
  IF fcg_sched_event = 0 THEN
    fcg_aux_wordret := PENDING_EVENTS[0];
    PENDING_EVENTS[1] := PENDING_EVENTS[1] AND BYTE#16#01;
    fcg_aux_wordret := fcg_aux_wordret OR PENDING_EVENTS[1];
    IF NOT ( fcg_aux_wordret = BYTE#0 ) THEN
      STATUS := STATUS OR BYTE#16#02;
    END_IF;
  END_IF;

  (* cancel pending events if no event could be scheduled  *)
  IF fcg_sched_event = 0 THEN
    PENDING_EVENTS[0] := BYTE#0;
    PENDING_EVENTS[1] := BYTE#0;
  END_IF;

END_IF;



(* ************************************************ *)
(* * break execution loop                         * *)

IF fcg_sched_event = 0 THEN EXIT; END_IF;
fcg_first_loop := false;



(* ************************************************ *)
(* * execute scheduled event                      * *)

IF fcg_sched_event <> 0 THEN

  fcg_exec_event := fcg_sched_event;
  fcg_sched_event := 0;
  STATUS := STATUS OR BYTE#16#01;

  (* execute event for [Blink3] *)
  fcg_aux_parsetrans := fcg_generator_states_0[PARALLEL_STATE[0]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_0[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = fcg_exec_event THEN EXIT; END_IF;
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  IF fcg_aux_parseevent > 0 THEN
    fcg_aux_parsetrans := fcg_aux_parsetrans + 1;
    PARALLEL_STATE[0] := fcg_generator_transitions_0[fcg_aux_parsetrans];
  END_IF;

  (* execute event for [Delay] *)
  fcg_aux_parsetrans := fcg_generator_states_1[PARALLEL_STATE[1]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_1[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = fcg_exec_event THEN EXIT; END_IF;
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  IF fcg_aux_parseevent > 0 THEN
    fcg_aux_parsetrans := fcg_aux_parsetrans + 1;
    PARALLEL_STATE[1] := fcg_generator_transitions_1[fcg_aux_parsetrans];
  END_IF;

  (* execute event for [WaitButton] *)
  fcg_aux_parsetrans := fcg_generator_states_2[PARALLEL_STATE[2]];
  WHILE true DO
    fcg_aux_parseevent := fcg_generator_transitions_2[fcg_aux_parsetrans];
    IF fcg_aux_parseevent = fcg_exec_event THEN EXIT; END_IF;
    IF fcg_aux_parseevent = 0 THEN EXIT; END_IF;
    fcg_aux_parsetrans := fcg_aux_parsetrans + 2;
  END_WHILE;
  IF fcg_aux_parseevent > 0 THEN
    fcg_aux_parsetrans := fcg_aux_parsetrans + 1;
    PARALLEL_STATE[2] := fcg_generator_transitions_2[fcg_aux_parsetrans];
  END_IF;

  RECENT_EVENT := fcg_exec_event;

END_IF;



(* ************************************************ *)
(* * operate timers                               * *)

IF fcg_exec_event > 0 THEN
  CASE fcg_exec_event OF
  3, 4, 5, 7, 8, 9:
    fcg_timer_Tick_cnt := TIME#1s;
    fcg_timer_Tick_run := true;
  END_CASE;
END_IF;



(* ************************************************ *)
(* * operate outputs                              * *)

IF fcg_exec_event > 2 THEN
  CASE fcg_exec_event OF
  3:
    (* outputs for [OnLedA] *)
    %QX55.1 := true;
  4:
    (* outputs for [OnLedB] *)
    %QX55.2 := true;
  5:
    (* outputs for [OnLedC] *)
    %QX55.3 := true;
  6:
    (* outputs for [Beep] *)
    beep(len:=100);
  7:
    (* outputs for [OffLedA] *)
    %QX55.1 := false;
  8:
    (* outputs for [OffLedB] *)
    %QX55.2 := false;
  9:
    (* outputs for [OffLedC] *)
    %QX55.3 := false;
  END_CASE;
END_IF;



(* ************************************************ *)
(* * end execution loop                           * *)

END_WHILE;




(* ************************************************ *)
(* * executor core cyclic code: end               * *)
(* ************************************************ *)

END_FUNCTION_BLOCK (* end of fcg_cyclic_fb *)



(* ************************************************ *)
(* * initialised array types --- GEB IDE specific * *)
(* ************************************************ *)
TYPE

(* event name lookup table *)
fcg_event_lookup_st : STRING[10];
fcg_event_lookup_t : ARRAY[0..9] OF fcg_event_lookup_st := 
  [ 
  'e0', 
  'PushButton', 
  'Tick', 
  'OnLedA', 
  'OnLedB', 
  'OnLedC', 
  'Beep', 
  'OffLedA', 
  'OffLedB', 
  'OffLedC' ];

(* state name lookup for generator [Blink3] *)
fcg_state_lookup_0_st : STRING[6];
fcg_state_lookup_0_t : ARRAY[0..7] OF fcg_state_lookup_0_st := 
  [ 
  's0', 
  'idle', 
  'start1', 
  'start2', 
  'target', 
  'stop1', 
  'stop2', 
  'stop3' ];


END_TYPE



(* ************************************************ *)
(* * function to host the event name loopup table * *)
(* ************************************************ *)


FUNCTION fcg_event_lookup_f : STRING

VAR_INPUT
IDX : INT;
END_VAR

VAR CONSTANT
(* event name lookup table *)
fcg_event_lookup : fcg_event_lookup_t;

END_VAR

fcg_event_lookup_f := fcg_event_lookup[IDX];

END_FUNCTION



(* ************************************************ *)
(* * function to host state name loopup tables    * *)
(* ************************************************ *)


FUNCTION fcg_state_lookup_f : STRING

VAR_INPUT
GID : INT;
IDX : INT;
END_VAR

VAR CONSTANT
(* state name lookup for generator [Blink3] *)
fcg_state_lookup_0 : fcg_state_lookup_0_t;

END_VAR

fcg_state_lookup_f:='out_of_range';
CASE GID OF
0: fcg_state_lookup_f := fcg_state_lookup_0[IDX];
1: fcg_state_lookup_f := CONCAT('s',INT_TO_STRING(IDX));
2: fcg_state_lookup_f := CONCAT('s',INT_TO_STRING(IDX));
END_CASE;

END_FUNCTION



(* ************************************************ *)
(* * append code snippet from configuration       * *)


(* provide beep action -- dummy *)
FUNCTION_BLOCK beep_fb 
VAR_INPUT len : INT; 
END_VAR
END_FUNCTION_BLOCK



(* ************************************************ *)
(* CodeGenerator: Generated Code Ends Here          *)
(* ************************************************ *)
