// DD Monitor (F-Task)

#include "dd_headers.h"
#include "dds_api.h"

void DD_Monitor_Task(void *pvParameters) {
  // Let everything start up first. Often tasks will all want to start at t=0 so
  // we should yield to the main thread for that.
  vTaskDelay(1000);
  DD_LL_Leader_t *ll;
  while (1) {
    printf("## Monitoring (F-Task Priority: %u; Tick: %u)\n",
           (unsigned int)uxTaskPriorityGet(NULL),
		   (unsigned int)xTaskGetTickCount());

    // TODO: Print information on the deadlines, creation times etc.
    ll = get_active_dd_task_list();
    printf("## Active (%d):\n", ll->length);
    for (ll_cur_head(ll); ll->cursor; ll_cur_next(ll))
      printf("## - ID:%d R:%d D:%d C:%d\n",
        ll->cursor->task->id,
		ll->cursor->task->release_time,
		ll->cursor->task->absolute_deadline,
		ll->cursor->task->completion_time);

    ll = get_overdue_dd_task_list();
    printf("## Overdue (%d):\n", ll->length);
    for (ll_cur_head(ll); ll->cursor; ll_cur_next(ll))
      printf("## - ID:%d R:%d D:%d C:%d\n",
        ll->cursor->task->id,
  		ll->cursor->task->release_time,
  		ll->cursor->task->absolute_deadline,
  		ll->cursor->task->completion_time);

    ll = get_complete_dd_task_list();
    printf("## Complete (%d):\n", ll->length);
    for (ll_cur_head(ll); ll->cursor; ll_cur_next(ll))
      printf("## - ID:%d R:%d D:%d C:%d\n",
        ll->cursor->task->id,
  		ll->cursor->task->release_time,
  		ll->cursor->task->absolute_deadline,
  		ll->cursor->task->completion_time);

    vTaskDelay(500);
  }
}
