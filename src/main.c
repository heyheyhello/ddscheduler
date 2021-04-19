// Using FreeRTOS v9.0.0

#include "dd_headers.h"
#include "../FreeRTOS_Source/portable/MemMang/heap_4.c"

static void prvSetupHardware(void);

int main(void) {
  printf("System clock: %u Hz\n", SystemCoreClock);
  printf("portTICK_PERIOD_MS: %u\n", portTICK_PERIOD_MS);

  prvSetupHardware();

  xTaskCreate(DD_Scheduler_Task, "DD Scheduler Task", configMINIMAL_STACK_SIZE,
              NULL, DD_PRIORITY_SCHEDULER_TASK, NULL);
//  xTaskCreate(DD_Monitor_Task, "DD Monitor Task", configMINIMAL_STACK_SIZE,
//              NULL, DD_PRIORITY_MONITOR_TASK, NULL);

  xTaskCreate(Periodic_Task_Generator_1, "Periodic_Gen_1",
              configMINIMAL_STACK_SIZE, NULL, DD_PRIORITY_GENERATOR_TASK, NULL);

  vTaskStartScheduler();
  return 0;
}

void vApplicationMallocFailedHook(void) {
  /* The malloc failed hook is enabled by setting
  configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

  Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  for (;;)
    ;
}

void vApplicationStackOverflowHook(xTaskHandle pxTask,
                                   signed char *pcTaskName) {
  (void)pcTaskName;
  (void)pxTask;

  /* Run time stack overflow checking is performed if
  configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected.  pxCurrentTCB can be
  inspected in the debugger if the task name passed into this function is
  corrupt. */
  for (;;)
    ;
}

void vApplicationIdleHook(void) {
  volatile size_t xFreeStackSpace;

  /* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
  FreeRTOSConfig.h.

  This function is called on each cycle of the idle task.  In this case it
  does nothing useful, other than report the amount of FreeRTOS heap that
  remains unallocated. */
  xFreeStackSpace = xPortGetFreeHeapSize();

  if (xFreeStackSpace > 100) {
    /* By now, the kernel has allocated everything it is going to, so
    if there is a lot of heap remaining unallocated then
    the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
    reduced accordingly. */
  }
}

static void prvSetupHardware(void) {
  /* Ensure all priority bits are assigned as preemption priority bits.
  http://www.freertos.org/RTOS-Cortex-M3-M4.html */
  NVIC_SetPriorityGrouping(0);

  // Enable the TRAFFIC LIGHTS ONLY. These will represent the 3 tasks doing work
  // in the DD scheduler.

  // Tables says to use /PC\d/ so GPIOC
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitTypeDef GPIO_InitStruct;

  // First do LEDs
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  // Traffic Red/Amber/Green and then Shift Reset/Shift Clock/Shift Data
  GPIO_InitStruct.GPIO_Pin =
      TRAFFIC_LIGHT_R | TRAFFIC_LIGHT_A | TRAFFIC_LIGHT_G;
  GPIO_InitStruct.GPIO_OType = GPIO_PuPd_UP;
  // For LEDs we're told to use a high speed
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  // Using the full RCC_AH...GPIOC is bad and causes a macro definition error
  GPIO_Init(GPIOC, &GPIO_InitStruct);
}
