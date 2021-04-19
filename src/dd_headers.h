// Every header lives here

// Macro prevents including header files multiple times. Not sure if this is
// necessary, I don't do a lot of C.
#ifndef DD_INCLUDES
#define DD_INCLUDES

// Includes copied from FreeRTOS Project 0

/* Standard includes. */
#include "stm32f4_discovery.h"
#include <stdint.h>
#include <stdio.h>

/* Kernel includes. */
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"
#include "stm32f4xx.h"

// From our PDF doc. Everything is on GPIOC.
// Red/Amber/Green
#define TRAFFIC_LIGHT_R GPIO_Pin_0
#define TRAFFIC_LIGHT_A GPIO_Pin_1
#define TRAFFIC_LIGHT_G GPIO_Pin_2

#define DD_PRIORITY_UNSCHEDULED (1)
// Timer service is priority 3; be higher than that...
#define DD_PRIORITY_USER_BASELINE (5)
// Never interrupt these...
#define DD_PRIORITY_MONITOR_TASK (configMAX_PRIORITIES - 3)
#define DD_PRIORITY_GENERATOR_TASK (configMAX_PRIORITIES - 2)
#define DD_PRIORITY_SCHEDULER_TASK (configMAX_PRIORITIES - 1)

typedef enum DD_Task_Enum_t { PERIODIC, APERIODIC } DD_Task_Enum_t;

typedef enum DD_Message_Enum_t {
  DD_API_Req_Task_Create,
  DD_API_Req_Task_Delete,
  DD_API_Req_Fetch_Task_List,
  DD_API_Res_OK,
} DD_Message_Enum_t;

typedef struct DD_Task_t {
  TaskHandle_t task_handle;
  DD_Task_Enum_t type;
  uint32_t id;
  uint32_t release_time;
  uint32_t absolute_deadline;
  uint32_t completion_time;
} DD_Task_t;

typedef struct DD_Message_t {
  DD_Message_Enum_t type;
  void *data;
} DD_Message_t;

// Linked list implementation

typedef struct DD_LL_Node_t {
  DD_Task_t *task;
  struct DD_LL_Node_t *next;
} DD_LL_Node_t;

typedef struct DD_LL_Leader_t {
  DD_LL_Node_t *head;
  DD_LL_Node_t *cursor;
  DD_LL_Node_t *cursor_prev;
  unsigned int length;
  unsigned int add_count;
} DD_LL_Leader_t;

DD_LL_Leader_t *ll_leader();
DD_LL_Node_t *ll_node(DD_Task_t *dd_task);
void ll_cur_head(DD_LL_Leader_t *leader);
void ll_cur_next(DD_LL_Leader_t *leader);
void ll_cur_tail(DD_LL_Leader_t *leader);
void ll_cur_prepend(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add);
void ll_cur_append(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add);
DD_LL_Node_t *ll_cur_unlink(DD_LL_Leader_t *leader);
void ll_print(DD_LL_Leader_t *leader);
void ll_empty(DD_LL_Leader_t *leader);

void DD_Scheduler_Task(void *pvParameters);
void DD_Monitor_Task(void *pvParameters);

void Periodic_Task_Generator_1(void *pvParameters);
void Periodic_Task_Generator_2(void *pvParameters);
void Periodic_Task_Generator_3(void *pvParameters);
#endif
