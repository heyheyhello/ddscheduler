// Deadline-Driven Task Generator
// Periodically creates DD-Tasks that need to be scheduled by the DD Scheduler

#include "dd_headers.h"
#include "dd_tests.h"
#include "dds_api.h"

void Periodic_Task_1(void *pvParameters);
void Periodic_Task_2(void *pvParameters);
void Periodic_Task_3(void *pvParameters);

void Periodic_Task_Generator_1(void *pvParameters) {
  while (1) {
    TaskHandle_t task_handle;
    xTaskCreate(Periodic_Task_1,          // TaskFunction_t Function
                "Task_1",                 // const char *const pcName
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
  while (execution_time) {
    current_tick = xTaskGetTickCount();
    if (current_tick == previous_tick)
      continue;
    previous_tick = current_tick;
    execution_time--;
  }
  // There's no need to vTaskDelay here. If we're done we're done.
  delete_dd_task(1);
}

void Periodic_Task_Generator_2(void *pvParameters) {
  while (1) {
    TaskHandle_t task_handle;
    xTaskCreate(Periodic_Task_2,          // TaskFunction_t Function
                "Task_2",                 // const char *const pcName
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
        2,                                  // uint32_t id,
        xTaskGetTickCount() + TASK_2_PERIOD // uint32_t absolute_deadline);
    );
    // Delay THIS generator task. Not the created one
    vTaskDelay(TASK_2_PERIOD);
  }
}

void Periodic_Task_2(void *pvParameters) {
  TickType_t current_tick = xTaskGetTickCount();
  TickType_t previous_tick = 0;
  TickType_t execution_time = TASK_2_EXEC_TIME / portTICK_PERIOD_MS;
  printf("Task 2 (F-Task Priority: %u; Tick: %u)\n",
         (unsigned int)uxTaskPriorityGet(NULL), (unsigned int)current_tick);
  while (execution_time) {
    current_tick = xTaskGetTickCount();
    if (current_tick == previous_tick)
      continue;
    previous_tick = current_tick;
    execution_time--;
  }
  // There's no need to vTaskDelay here. If we're done we're done.
  delete_dd_task(2);
}

void Periodic_Task_Generator_3(void *pvParameters) {
  while (1) {
    TaskHandle_t task_handle;
    xTaskCreate(Periodic_Task_3,          // TaskFunction_t Function
                "Task_3",                 // const char *const pcName
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
        3,                                  // uint32_t id,
        xTaskGetTickCount() + TASK_3_PERIOD // uint32_t absolute_deadline);
    );
    // Delay THIS generator task. Not the created one
    vTaskDelay(TASK_3_PERIOD);
  }
}

void Periodic_Task_3(void *pvParameters) {
  TickType_t current_tick = xTaskGetTickCount();
  TickType_t previous_tick = 0;
  TickType_t execution_time = TASK_3_EXEC_TIME / portTICK_PERIOD_MS;
  printf("Task 1 (F-Task Priority: %u; Tick: %u)\n",
         (unsigned int)uxTaskPriorityGet(NULL), (unsigned int)current_tick);
  while (execution_time) {
    current_tick = xTaskGetTickCount();
    if (current_tick == previous_tick)
      continue;
    previous_tick = current_tick;
    execution_time--;
  }
  // There's no need to vTaskDelay here. If we're done we're done.
  delete_dd_task(3);
}
