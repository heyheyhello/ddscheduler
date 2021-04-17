// DD Scheduler (F-Task)

#include "dd_headers.h"
#include "dds_api.h"

// The static keyword means this is only available to this file, which means
// these data structures are private to the scheduler task and its API functions
static DD_LL_Leader_t *active_list;
static DD_LL_Leader_t *overdue_list;
static DD_LL_Leader_t *complete_list;

// Queues are pointer queues https://www.freertos.org/a00118.html
// This means we can send any data since we're just sending addresses.

// TODO: WAIT NO THEY'RE NOT... They're DD_Message_t. It's the ->data that's a
// pointer... Need to fix the code then.

// This is how API functions queue up work for the DDS to complete
static QueueHandle_t *qh_request;
// This is how the DDS returns work to the API functions
static QueueHandle_t *qh_response;

  DD_Message_t *req_message;
  while (1) {
    if (xQueueReceive(qh_request, &req_message, portMAX_DELAY) == pdFALSE)
      // No incoming messages but timeout expired. That's fine, loop again...
      continue;

    // We have a message! First check for overdue tasks. I'm avoiding needing to
    // implement software timers since the monitor task is the only one reading
    // overdue tasks, and it will send us a message, which calls this overdue
    // check. This way the work is only performed on an as needed basis.
    TickType_t time_now = xTaskGetTickCount();
    for (ll_cur_head(ll_active); ll_active->cursor; ll_cur_next(ll_active)) {
      DD_Task_t *t = ll_active->cursor->task;
      if (time_now < t->absolute_deadline) {
        // Not overdue. Also! Because this list is sorted as EDF all elements
        // after this will also be not overdue, so break.
        break;
      }
      // Task is overdue
      DD_LL_Node_t *node_active = ll_cur_unlink(ll_active);
      vTaskSuspend(node_active->task->task_handle);
      vTaskDelete(node_active->task->task_handle);
      ll_cur_head(ll_overdue);
      // Too full? Remove head of list
      while (ll_overdue->length > 10) {
        print("DDS overdue list > 10 items removing head\n");
        DD_LL_Node_t *node_overdue = ll_cur_unlink(ll_overdue);
        // Overdue tasks have already had their F-Task cleaned up not D-Task
        vPortFree(node_overdue->task);
        ll_cur_head(ll_overdue);
      }
      ll_cur_tail(ll_overdue);
      // Move to end of overdue list
      ll_cur_append(ll_overdue, node_active);
      printf("DDS moved %d from active list to overdue list\n", t->id);
    }
    // Memory is shared, don't allocate a new message
    DD_Message_t *res_message = req_message;
    // Back to processing that incoming message
    switch (req_message->type) {
    case (DD_API_Req_Task_Create):
      DD_Task_t *task = (DD_Task_t *)req_message->data;

      // TODO: Walk the LL active list and put it in the right spot. This will
      // require two passes; one to insert and one to adjust all the priorities
      // of tasks based on where it was put in the list.

      // Return address of the DD_Task_t
      res_message->data = task;
      break;

    case (DD_API_Req_Task_Delete):
      // Hate pointers but I want to keep the queue as a generic pointer queue.
      uint32_t *task_id_pointer = (void *)&req_message->data;
      uint32_t task_id = *task_id_pointer;

      ll_cur_head(ll_active);
      DD_Task_t *t = ll_active->cursor->task;
      DD_Task_t *t_removed = NULL;
      uint32_t top_priority = uxTaskPriorityGet(t->task_handle);
      for (; ll_active->cursor; ll_cur_next(ll_active)) {
        t = ll_active->cursor->task;
        if (t->id != task_id)
          continue;
        t_removed = t;
        DD_LL_Node_t *node_active = ll_cur_unlink(ll_active);
        ll_cur_head(ll_complete);
        // Too full? Remove head of list
        while (ll_complete->length > 10) {
          print("DDS complete list > 10 items removing head\n");
          DD_LL_Node_t *node_complete = ll_cur_unlink(ll_complete);
          // Complete tasks have already had their F-Task cleaned up not D-Task
          vPortFree(node_complete->task);
          ll_cur_head(ll_complete);
        }
        ll_cur_tail(ll_complete);
        // Move to end of complete list
        ll_cur_append(ll_complete, node_active);
        printf("DDS moved %d from active list to complete list\n", task_id);
        break;
      }
      // TODO: Diagram this on paper to make sure I understand...
      // If there was a task to remove, then decrement the priorities before it
      if (t_removed != NULL) {
        for (ll_cur_head(ll_active); ll_active->cursor;
             ll_cur_next(ll_active)) {
          t = ll_active->cursor->task;
          if (t->id == task_id)
            break;
          vTaskPrioritySet(t->task_handle, --top_priority);
        }
      } else {
        print("DDS task not in active list %d\n", task_id);
      }
      // Return address of the DD_Task_t (or NULL if not found)
      res_message->data = t_removed;
      break;

    case (DD_API_Req_Fetch_Task_List):
      DD_LL_Leader_t *list_to_return = (DD_LL_Leader_t *)req_message->data;
      // If I was going to modify the list, make a copy, redact things, etc, I'd
      // do that here and return a pointer to a new list. I'm not doing that
      // though, since all the code so far is trusted to not free things that
      // aren't theirs. This is a performance tradeoff.

      if (list_to_return == ll_active)
        printf("DDS ll_active\n");
      if (list_to_return == ll_overdue)
        printf("DDS ll_overdue\n");
      if (list_to_return == ll_complete)
        printf("DDS ll_complete\n");

      res_message->data = list_to_return;
      break;
    }
    res_message->type = DD_API_Res_OK;
    if (xQueueSend(qh_response, &res_message, portMAX_DELAY) == pdFALSE)
      printf("DDS error responding to DD_API enum %d\n", req_message->type);
  }
}

void create_dd_task(
    TaskHandle_t task_handle,
    DD_Task_Enum_t type,
    uint32_t id,
    uint32_t absolute_deadline){};

void delete_dd_task(uint32_t id)
{
  // TODO: Send a message containing a pointer to an integer (or real int???)
  // TODO: Receive a message containing a pointer to DD_Task_t
  // TODO: Free the task and task handle
  return;
};

static DD_LL_Leader_t *get_dd_task_list(DD_LL_Leader_t *requested_list)
{
  DD_Message_t *message = (DD_Message_t *)malloc(sizeof(DD_Message_t));
  if (message == NULL)
    return NULL; // Shouldn't happen.

  message->type = DD_API_Message_Fetch_Task_List;
  // TODO: Oh no is this &requested_list? I don't do pointers.
  message->data = requested_list;

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
