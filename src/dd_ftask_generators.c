// Deadline-Driven Task Generator
// Periodically creates DD-Tasks that need to be scheduled by the DD Scheduler

#include "dd_headers.h"
#include "dds_api.h"
#include "dd_tests.h"

void Periodic_Task_1(void *pvParameters);

void Periodic_Task_Generator_1(void *pvParameters) {
  while (1) {
	printf("Launching task 1 via generator\n");
    TaskHandle_t task_handle;
    xTaskCreate(Periodic_Task_1,          // TaskFunction_t Function
                "Periodic_Task_1",        // const char *const pcName
                configMINIMAL_STACK_SIZE, // configSTACK_DEPTH_TYPE usStackDepth
                NULL,                     // void *const pvParameters
                DD_PRIORITY_UNSCHEDULED,  // UBaseType_t uxPriority
                &(task_handle)            // TaskHandle_t *const pxCreatedTask
    );
    if (task_handle == NULL) {
      printf("Generator xTaskCreate task handle is NULL\n");
      return;
    }
    // Let the DDS start it later with a new priority
    vTaskSuspend(task_handle);

    create_dd_task(
        task_handle,                        // TaskHandle_t task_handle,
        PERIODIC,                           // DD_Task_Enum_t type,
        1,                                  // uint32_t id,
        xTaskGetTickCount() + TASK_1_PERIOD // uint32_t absolute_deadline);
    );

    // Delay THIS generator task. Not the created one
    vTaskDelay(TASK_1_PERIOD);
  }
}

void Periodic_Task_1(void *pvParameters) {
  TickType_t current_tick = xTaskGetTickCount();
  TickType_t previous_tick = 0;
  TickType_t execution_time = TASK_1_EXEC_TIME / portTICK_PERIOD_MS;
  printf("Task 1 (F-Task Priority: %u; Tick: %u)\n",
         (unsigned int)uxTaskPriorityGet(NULL), (unsigned int)current_tick);
  printf("Busy loop for %d\n", (int)execution_time);
  while (execution_time) {
	current_tick = xTaskGetTickCount();
	if (current_tick == previous_tick)
      continue;
    execution_time--;
  }
  printf("Busy loop done; deleting\n");

  // There's no need to vTaskDelay here. If we're done we're done.
  delete_dd_task(1);
}
