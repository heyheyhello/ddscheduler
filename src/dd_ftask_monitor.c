// DD Monitor (F-Task)

#include "dd_headers.h"
#include "dds_api.h"

void DD_Monitor_Task(void *pvParameters)
{
  // Let everything start up first. Often tasks will all want to start at t=0 so
  // we should yield to the main thread for that.
  vTaskDelay(1000);
  DD_LL_Leader_t *ll;
  while (1)
  {
    printf("\nMonitoring (F-Task Priority: %u; Tick: %u)\n", (unsigned int)xTaskGetTickCount(), (unsigned int)uxTaskPriorityGet(NULL));

    // TODO: Print information on the deadlines, creation times etc.
    ll = get_active_dd_task_list();
    printf("Monitor Active List (%d):", ll->length);
    for (ll->cursor = ll->head; ll->cursor; ll_next(ll))
      printf(" - ID: %s\n", ll->cursor->task->id);

    ll = get_overdue_dd_task_list();
    printf("Monitor Overdue List (%d):", ll->length);
    for (ll->cursor = ll->head; ll->cursor; ll_next(ll))
      printf(" - ID: %s\n", ll->cursor->task->id);

    ll = get_complete_dd_task_list();
    printf("Monitor Complete List (%d):", ll->length);
    for (ll->cursor = ll->head; ll->cursor; ll_next(ll))
      printf(" - ID: %s\n", ll->cursor->task->id);

    vTaskDelay(1000);
  }
}
