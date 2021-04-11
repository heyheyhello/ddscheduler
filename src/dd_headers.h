// Every header lives here

// Macro prevents including header files multiple times. Not sure if this is
// necessary, I don't do a lot of C.
#ifndef DD_INCLUDES
#define DD_INCLUDES

// Includes copied from FreeRTOS Project 0

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

typedef enum DD_Task_Enum_t
{
  PERIODIC,
  APERIODIC
} DD_Task_Enum_t;

typedef enum DD_Message_Enum_t
{
  DD_API_Message_Task_Create,
  DD_API_Message_Task_Delete,
  DD_API_Message_Fetch_Task_List,
} DD_Message_Enum_t;

typedef struct DD_Task_t
{
  TaskHandle_t task_handle;
  DD_Task_Enum_t type;
  uint32_t id;
  uint32_t release_time;
  uint32_t absolute_deadline;
  uint32_t completion_time;
} DD_Task_t;

typedef struct DD_Message_t
{
  DD_Message_Enum_t type;
  TaskHandle_t sender;
  void *data;
} DD_Message_t;

// Linked list implementation

typedef struct DD_LL_Leader_t
{
  DD_LL_Node_t *head;
  DD_LL_Node_t *cursor;
  DD_LL_Node_t *cursor_prev;
  unsigned int length;
} DD_LL_Leader_t;

typedef struct DD_LL_Node_t
{
  DD_Task_t *task;
  struct DD_LL_Node_t *next;
} DD_LL_Node_t;

DD_LL_Leader_t *ll();
DD_LL_Node_t *ll_node(DD_Task_t *dd_task);
void ll_next(DD_LL_Leader_t *leader);
void ll_cursor_prepend(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add);
void ll_cursor_append(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add);
DD_LL_Node_t *ll_cursor_unlink(DD_LL_Leader_t *leader);
void ll_print(DD_LL_Leader_t *leader);
void ll_empty(DD_LL_Leader_t *leader);

#endif
