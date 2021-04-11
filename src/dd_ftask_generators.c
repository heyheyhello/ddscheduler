// Deadline-Driven Task Generator
// Periodically creates DD-Tasks that need to be scheduled by the DD Scheduler

#include "dd_headers.h"
#include "dd_tests.h"

void Periodic_Task_Generator_1(void *pvParameters)
{
  while (1)
  {
    TaskHandle_t *task_handle;
    xTaskCreate(Periodic_Task_1,          // TaskFunction_t Function
                "Periodic_Task_1",        // const char *const pcName
                configMINIMAL_STACK_SIZE, // configSTACK_DEPTH_TYPE usStackDepth
                NULL,                     // void *const pvParameters
                1 /* MINIMUM */,          // UBaseType_t uxPriority
                &(task_handle));          // TaskHandle_t *const pxCreatedTask
    if (task_handle == NULL)
    {
      printf("xTaskCreate task handle is NULL");
      return;
    }
    // Let the DDS start it later with a new priority
    vTaskSuspend(task_handle);

    create_dd_task(
        task_handle,                          // TaskHandle_t task_handle,
        PERIODIC,                             // DD_Task_Enum_t type,
        1,                                    // uint32_t id,
        xTaskGetTickCount() + TASK_1_PERIOD); // uint32_t absolute_deadline);

    // Delay THIS generator task. Not the created one
    vTaskDelay(TASK_1_PERIOD);
  }
}

void Periodic_Task_1(void *pvParameters)
{
  TickType_t current_tick = xTaskGetTickCount();
  TickType_t previous_tick = 0;
  TickType_t execution_time = TASK_1_EXEC_TIME / portTICK_PERIOD_MS;
  printf("Task 1 (F-Task Priority: %u; Tick: %u)\n", (unsigned int)uxTaskPriorityGet(NULL), (unsigned int)current_tick);
  int led_state = 0;
  while (execution_time)
  {
    if (current_tick == previous_tick)
      continue;
    current_tick = xTaskGetTickCount();
    previous_tick = current_tick;
    execution_time--;
    // Without this the LEDs flash too quickly
    if (current_tick % 2 == 0)
    {
      // TODO: LED on or off depending on the led_state
    }
  }
  // TODO: LED off

  // There's no need to vTaskDelay here. If we're done we're done.
  delete_dd_task(1);
}
