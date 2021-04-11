// DD Scheduler (F-Task)

#include "dd_headers.h"
#include "dds_api.h"

// The static keyword means this is only available to this file, which means
// these data structures are private to the scheduler task and its API functions
static DD_LL_Leader_t *active_list;
static DD_LL_Leader_t *overdue_list;
static DD_LL_Leader_t *complete_list;

// This is how API functions queue up work for the DDS to complete
static QueueHandle_t *qh_request;
// This is how the DDS returns work to the API functions
static QueueHandle_t *qh_response;

void DD_Scheduler_Task(void *pvParameters)
{
  DD_Message_t *incoming_message;
  while (1)
  {
    if (xQueueReceive(qh_request, &incoming_message, portMAX_DELAY) == pdFALSE)
      // No incoming messages but timeout expired. That's fine, loop again...
      continue;

    // We have a message! First check for overdue tasks. I'm avoiding needing to
    // implement software timers since the monitor task is the only one reading
    // overdue tasks, and it will send us a message, which calls this overdue
    // check. This way the work is only performed on an as needed basis.
    DD_LL_Leader_t *ll_active = &active_list;
    TickType_t time_now = xTaskGetTickCount();
    for (ll_active->cursor = ll_active->head; ll_active->cursor; ll_next(ll_active))
    {
      DD_Task_t *t = ll_active->cursor->task;
      if (time_now < t->absolute_deadline)
      {
        // Not overdue. Also! Because this list is sorted as EDF all elements
        // after this will also be not overdue, so break.
        break;
      }
      // Overdue.
      // TODO: Move this task to the overdue list
      // TODO: Prepend? Double check lab report for an order...
      // TODO: Remove items if the list is too long?
    }

    // Back to processing that incoming message
    switch (incoming_message->type)
    {
    case (DD_API_Message_Task_Create):
      DD_Task_t *task = (DD_Task_t *)incoming_message->data;

      // TODO: Walk the LL active list and put it in the right spot. This will
      // require two passes; one to insert and one to adjust all the priorities
      // of tasks based on where it was put in the list.

      if (xQueueSend(qh_response, NULL, portMAX_DELAY) == pdFALSE)
        printf("DDS error responding to DD_API_Message_Task_Create\n");
      break;

    case (DD_API_Message_Task_Delete):
      // Hate pointers but I want to keep the queue as a generic pointer queue.
      uint32_t *task_id_pointer = (void *)&incoming_message->data;
      uint32_t task_id = *task_id_pointer;

      ll_active->cursor = ll_active->head;
      DD_Task_t *t = ll_active->cursor->task;
      DD_Task_t *t_removed = NULL;
      uint32_t top_priority = uxTaskPriorityGet(t->task_handle);
      for (; ll_active->cursor; ll_next(ll_active))
      {
        t = ll_active->cursor->task;
        if (t->id == task_id)
        {
          t_removed = t;
          DD_LL_Node_t *node = ll_cursor_unlink(ll_active);
          free(node);
          printf("DDS removed task %d from active list", task_id);
          break;
        }
      }
      // If there was a task to remove, then decrement the priorities before it
      if (t_removed != NULL)
      {
        ll_active->cursor = ll_active->head;
        for (; ll_active->cursor; ll_next(ll_active))
        {
          t = ll_active->cursor->task;
          if (t->id == task_id)
            break;
          vTaskPrioritySet(t->task_handle, --top_priority);
        }
      }
      // I'll return the address of the DD_Task_t? Sure why not. Default NULL.
      if (xQueueSend(qh_response, &t_removed, portMAX_DELAY) == pdFALSE)
        printf("DDS error responding to DD_API_Message_Task_Delete\n");
      break;

    case (DD_API_Message_Fetch_Task_List):
      DD_LL_Leader_t *list_to_return = (DD_LL_Leader_t *)incoming_message->data;
      // If I was going to modify the list, make a copy, redact things, etc, I'd
      // do that here and return a pointer to a new list. I'm not doing that
      // though, since all the code so far is trusted to not free() things that
      // aren't theirs. This is a performance tradeoff.

      // DEBUG:
      if (list_to_return == active_list)
        printf("DDS returning active_list\n");
      if (list_to_return == overdue_list)
        printf("DDS returning overdue_list\n");
      if (list_to_return == complete_list)
        printf("DDS returning complete_list\n");

      if (xQueueSend(qh_response, &list_to_return, portMAX_DELAY) == pdFALSE)
        printf("DDS error responding to DD_API_Message_Fetch_Task_List\n");
      break;
    }
  }
}

void create_dd_task(
    TaskHandle_t task_handle,
    DD_Task_Enum_t type,
    uint32_t id,
    uint32_t absolute_deadline){};

void delete_dd_task(uint32_t id){};

static DD_LL_Leader_t *get_dd_task_list(DD_LL_Leader_t *requested_list)
{
  DD_Message_t *message = (DD_Message_t *)malloc(sizeof(DD_Message_t));
  if (message == NULL)
    return NULL; // Shouldn't happen.

  message->type = DD_API_Message_Fetch_Task_List;
  // TODO: Oh no is this &requested_list? I don't do pointers.
  message->data = requested_list;

  // Sending pointers in queues: https://www.freertos.org/a00118.html
  if (xQueueSend(qh_request, (void *)&message, portMAX_DELAY) != pdTRUE)
  {
    free(message);
    return NULL; // Shouldn't happen.
  }
  DD_LL_Leader_t *returned_list;
  if (xQueueReceive(qh_response, &returned_list, portMAX_DELAY) != pdTRUE)
  {
    free(message);
    return NULL; // Shouldn't happen.
  }
  free(message);
  // TODO: Oh no is this &requested_list? I don't do pointers.
  return returned_list;
};

DD_LL_Leader_t *get_active_dd_task_list(void)
{
  return get_dd_task_list(&active_list);
};

DD_LL_Leader_t *get_overdue_dd_task_list(void)
{
  return get_dd_task_list(&overdue_list);
};

DD_LL_Leader_t *get_complete_dd_task_list(void)
{
  return get_dd_task_list(&complete_list);
};
