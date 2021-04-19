// DD Scheduler (F-Task)

#include "dd_headers.h"
#include "dds_api.h"

// The static keyword means this is only available to this file, which means
// these data structures are private to the scheduler task and its API functions
static DD_LL_Leader_t *ll_active;
static DD_LL_Leader_t *ll_overdue;
static DD_LL_Leader_t *ll_complete;

// This is how API functions queue up work for the DDS to complete
static QueueHandle_t *qh_request;
// This is how the DDS returns work to the API functions
static QueueHandle_t *qh_response;

void DD_Scheduler_Task(void *pvParameters) {
  // For initialization I'll flex that we're part of the OS, and as such it's
  // fine to internally create queues here rather than in main(). Ultimately
  // this is an API design decision, but it's similar to vTaskStartScheduler().
  ll_active = ll_leader();
  ll_overdue = ll_leader();
  ll_complete = ll_leader();

  // Queues are pointer queues https://www.freertos.org/a00118.html
  // Mostly because we're not taught in SENG anything other than heap/malloc
  // with pointers so the idea of copying an entire struct is...confusing.
  qh_request = xQueueCreate(5, sizeof(void *));
  qh_response = xQueueCreate(5, sizeof(void *));
  vQueueAddToRegistry(qh_request, "DDS Req");
  vQueueAddToRegistry(qh_response, "DDS Res");

  DD_Message_t *req_message;
  while (1) {
    if (xQueueReceive(qh_request, &req_message, 500) == pdFALSE) {
      // No incoming messages but timeout expired. That's fine, loop again...
      continue;
    }

    // We have a message! First check for overdue tasks. I'm avoiding needing to
    // implement software timers since the monitor task is the only one reading
    // overdue tasks, and it will send us a message, which calls this overdue
    // check. This way the work is only performed on an as needed basis.
    // printf("DDS recv message: %u\n", req_message);
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
      while (ll_overdue->length >= 5) {
        DD_LL_Node_t *node_overdue = ll_cur_unlink(ll_overdue);
        // Overdue tasks have already had their F-Task cleaned up not D-Task
        vPortFree(node_overdue->task);
        ll_cur_head(ll_overdue);
      }
      ll_cur_tail(ll_overdue);
      // Move to end of overdue list
      ll_cur_append(ll_overdue, node_active);
    }
    // Memory is shared, don't allocate a new message
    DD_Message_t *res_message;
    // Point to same struct...
    res_message = req_message;
    // Back to processing that incoming message
    switch (req_message->type) {
    case (DD_API_Req_Task_Create): {
      DD_Task_t *task_ins = (DD_Task_t *)req_message->data;
      DD_LL_Node_t *node = ll_node(task_ins);
      ll_cur_head(ll_active);
      uint32_t prio =
          ll_active->cursor
              ? uxTaskPriorityGet(ll_active->cursor->task->task_handle)
              : DD_PRIORITY_USER_BASELINE;
      // Consider prepending each item
      for (; ll_active->cursor; ll_cur_next(ll_active)) {
        DD_Task_t *t = ll_active->cursor->task;
        if (task_ins->absolute_deadline >= t->absolute_deadline) {
          prio = prio == 0 ? 0 : prio - 1;
          continue;
        }
        // Add the task
        ll_cur_prepend(ll_active, node);
        break;
      }
      // Ran off the end of the list, so append to tail since prepend failed
      if (ll_active->cursor == NULL) {
        ll_active->cursor = ll_active->cursor_prev;
        // If NULL, adds first element, else, doesn't use cursor_prev
        ll_cur_append(ll_active, node);
      }
      task_ins->release_time = time_now;
      vTaskPrioritySet(task_ins->task_handle, prio);
      vTaskResume(task_ins->task_handle);
      // Return address of the DD_Task_t
      res_message->data = task_ins;
      break;
    }
    case (DD_API_Req_Task_Delete): {
      // Hate pointers but I want to keep the queue as a generic pointer queue.
      uint32_t task_id = (uint32_t)req_message->data;
      ll_cur_head(ll_active);
      DD_Task_t *t = NULL;
      DD_Task_t *t_removed = NULL;
      uint32_t prio = 0;
      if (ll_active->length > 0) {
        // Otherwise this throws a hard fault on the CPU
        prio = uxTaskPriorityGet(ll_active->cursor->task->task_handle);
      }
      for (; ll_active->cursor; ll_cur_next(ll_active)) {
        t = ll_active->cursor->task;
        if (t->id != task_id)
          continue;
        t_removed = t;
        // Previous tick it was done. This current tick is the scheduler.
        t_removed->completion_time = xTaskGetTickCount() - 1;
        DD_LL_Node_t *node_active = ll_cur_unlink(ll_active);
        ll_cur_head(ll_complete);
        // Too full? Remove head of list
        while (ll_complete->length >= 5) {
          DD_LL_Node_t *node_complete = ll_cur_unlink(ll_complete);
          // Complete tasks have already had their F-Task cleaned up not D-Task
          vPortFree(node_complete->task);
          ll_cur_head(ll_complete);
        }
        ll_cur_tail(ll_complete);
        // Move to end of complete list
        ll_cur_append(ll_complete, node_active);
        break;
      }
      // If there was a task to remove, then decrement the priorities before it
      if (t_removed != NULL) {
        for (ll_cur_head(ll_active); ll_active->cursor;
             ll_cur_next(ll_active)) {
          t = ll_active->cursor->task;
          if (t->id == task_id)
            break;
          prio = prio == 0 ? 0 : prio - 1;
          vTaskPrioritySet(t->task_handle, prio);
        }
      } else {
        printf("DDS task not in active list %d\n", (int)task_id);
      }
      // Return address of the DD_Task_t (or NULL if not found)
      res_message->data = t_removed;
      break;
    }
    case (DD_API_Req_Fetch_Task_List): {
      DD_LL_Leader_t *list_to_return = (DD_LL_Leader_t *)req_message->data;
      // If I was going to modify the list, make a copy, redact things, etc, I'd
      // do that here and return a pointer to a new list. I'm not doing that
      // though, since all the code so far is trusted to not free things that
      // aren't theirs. This is a performance tradeoff.
      res_message->data = list_to_return;
      break;
    }
    }
    res_message->type = DD_API_Res_OK;
    // printf("DDS returning message: %u\n", res_message);
    if (xQueueSend(qh_response, (void *)&res_message, portMAX_DELAY) == pdFALSE)
      printf("DDS error responding to DD_API enum %d\n", req_message->type);
  }
}

// Long but I wanted to covert all bases about freeing memory on error
static void *dd_api_call(DD_Message_Enum_t type, void *message_data) {
  DD_Message_t *message = (DD_Message_t *)pvPortMalloc(sizeof(DD_Message_t));
  message->type = type;
  message->data = message_data;
  // printf("dd_api_call %d sending %u\n", type, message);
  // (..., 0) means don't block if the queue is already full.
  BaseType_t req = xQueueSend(qh_request, (void *)&message, 0);
  // printf("dd_api_call %d back from xQSend\n", type);
  if (req != pdTRUE) {
    vPortFree(message);
    return NULL; // Shouldn't happen.
  }
  DD_Message_t *ret_message;
  // Likewise, portMAX_DELAY means wait as long as needed for the DDS to reply.
  BaseType_t res = xQueueReceive(qh_response, &ret_message, portMAX_DELAY);
  // printf("dd_api_call %d back from xQRecv; message is %u\n", type,
  // ret_message);
  if (res != pdTRUE) {
    vPortFree(message);
    return NULL; // Shouldn't happen.
  }
  if (ret_message != message) {
    printf("dd_api_call %d message pointer mismatch\n", type);
    vPortFree(message);
    vPortFree(ret_message);
    return NULL;
  }
  void *return_data;
  if (message->type == DD_API_Res_OK) {
    // printf("dd_api_call %d OK\n", type);
    return_data = message->data;
  } else {
    printf("dd_api_call %d NOT OK\n", type);
    return_data = NULL;
  }
  vPortFree(message);
  return return_data;
};

void create_dd_task(TaskHandle_t task_handle, DD_Task_Enum_t type, uint32_t id,
                    uint32_t absolute_deadline) {
  // Immediately stop the F-Task. DDS will start it later.
  vTaskSuspend(task_handle);
  DD_Task_t *task = (DD_Task_t *)pvPortMalloc(sizeof(DD_Task_t));
  task->task_handle = task_handle;
  task->type = type;
  task->id = id;
  task->absolute_deadline = absolute_deadline;
  task->release_time = NULL;
  task->completion_time = NULL;

  DD_Task_t *ret_task = dd_api_call(DD_API_Req_Task_Create, task);
  // printf("create_dd_task: task release time: %d\n", (int)task->release_time);
  if (task != ret_task) {
    printf("create_dd_task: task pointer mismatch\n");
  }
};

// Don't free the task since it's being used in the complete_list. Memory is
// cleaned up in the DDS by removing list heads for lists > 10 items.
void delete_dd_task(uint32_t id) {
  DD_Task_t *task = dd_api_call(DD_API_Req_Task_Delete, id);
  if (task) {
    // printf("delete_dd_task: task completion time: %d\n",
    // (int)task->completion_time);
    vTaskDelete(task->task_handle);
  } else {
    printf("delete_dd_task: task not found\n");
  }
};

DD_LL_Leader_t *get_active_dd_task_list(void) {
  return dd_api_call(DD_API_Req_Fetch_Task_List, ll_active);
};

DD_LL_Leader_t *get_overdue_dd_task_list(void) {
  return dd_api_call(DD_API_Req_Fetch_Task_List, ll_overdue);
};

DD_LL_Leader_t *get_complete_dd_task_list(void) {
  return dd_api_call(DD_API_Req_Fetch_Task_List, ll_complete);
};
