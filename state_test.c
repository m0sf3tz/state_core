#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <errno.h>

#include "global_defines.h"
#include "state_core.h"
#include "state_test.h"

/**********************************************************
*                                    FORWARD DECLARATIONS *
**********************************************************/
static state_t state_a_func();
static state_t state_b_func();


/**********************************************************
*                                        GLOBAL VARIABLES *
**********************************************************/

/**********************************************************
*                                                TYPEDEFS *
**********************************************************/

/**********************************************************
*                                        STATIC VARIABLES *
**********************************************************/
static const char        TAG[] = "NET_STATE";

// Translation Table
static state_array_s test_translation_table[test_state_len] = {
  { state_a_func ,  portMAX_DELAY },
  { state_b_func ,  5000          }, 
};

/**********************************************************
*                                               FUNCTIONS *
**********************************************************/
static state_t state_a_func() {
  ESP_LOGI(TAG, "Entering state A");
  return NULL_STATE;
}

static state_t state_b_func() {
  ESP_LOGI(TAG, "Entering state B");

  vTaskDelay( 5000 / portTICK_PERIOD_MS );
  ESP_LOGI(TAG, "forcing B->A transition!");
  return test_state_a;
}

// Returns the next state
static void next_state_func(state_t* curr_state, state_event_t event) {
  if (*curr_state == test_state_a) {
      switch(event){
        case (STATE_A2B_TRANSITION ):
            ESP_LOGI(TAG, "A->B transition!");
            *curr_state = test_state_b;
            break;
      }
  }
}

// TODO
static char* event_print_func(state_event_t event) {
    return NULL;
}

static bool event_filter_func(state_event_t event) {
  switch(event){
    case(STATE_A2B_TRANSITION)   : return true; 
  }
}

static state_init_s* get_test_handle() {
    static state_init_s parser_state = {
        .next_state        = next_state_func,
        .translation_table = test_translation_table,
        .event_print       = event_print_func,
        .starting_state    = test_state_a ,
        .state_name_string = "test_state",
        .filter_event      = event_filter_func,
        .total_states      = test_state_len,  
    };
    return &(parser_state);
}


void state_machine_driver(void * arg){
  start_new_state_machine(get_test_handle());

  for(;;){
      vTaskDelay(10000/portTICK_PERIOD_MS);
      ESP_LOGI(TAG, "Posting event STATE_A2B_TRANSITION!");
      state_post_event(STATE_A2B_TRANSITION);
  }
}

void test_init(){
  state_core_spawner();

  xTaskCreate(state_machine_driver, "driver", 1024, NULL, 5, NULL); 
}
