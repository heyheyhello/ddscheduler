// Using FreeRTOS v9.0.0

#include "dd_headers.h"

// From our PDF doc. Everything is on GPIOC.
// Red/Amber/Green
#define TRAFFIC_LIGHT_R GPIO_Pin_0
#define TRAFFIC_LIGHT_A GPIO_Pin_1
#define TRAFFIC_LIGHT_G GPIO_Pin_2

static void prvSetupHardware(void);

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file.
 */
static void Manager_Traffic_LED_Task(void *pvParameters);
static void Red_Traffic_LED_Controller_Task(void *pvParameters);
static void Amber_Traffic_LED_Controller_Task(void *pvParameters);
static void Green_Traffic_LED_Controller_Task(void *pvParameters);

int main(void)
{
  printf("System clock: %u Hz\n", SystemCoreClock);
  printf("portTICK_PERIOD_MS: %u\n", portTICK_PERIOD_MS);

  prvSetupHardware();

  vTaskStartScheduler();
  return 0;
}

static void Manager_Traffic_LED_Task(void *pvParameters)
{
  uint16_t LEDS[] = {TRAFFIC_LIGHT_R, TRAFFIC_LIGHT_A, TRAFFIC_LIGHT_G};
  int i = 0;
  while (1)
  {
    uint16_t led = LEDS[i];
    printf("Manager: LED ON %u\n", led);
    GPIO_SetBits(GPIOC, led);
    if (xQueueSend(xQueue_handle, &led, 1000))
    {
      printf("Manager xQueueSend ok\n");
      i++;
      if (i >= 3)
      {
        i = 0;
      }
      vTaskDelay(1000);
    }
    else
    {
      printf("Manager xQueueSend error\n");
    }
  }
}

static void Red_Traffic_LED_Controller_Task(void *pvParameters)
{
  uint16_t rx_data;
  while (1)
  {
    if (xQueueReceive(xQueue_handle, &rx_data, 500))
    {
      if (rx_data == TRAFFIC_LIGHT_R)
      {
        vTaskDelay(250);
        GPIO_ResetBits(GPIOC, TRAFFIC_LIGHT_R);
        printf("Red off\n");
      }
      else
      {
        if (xQueueSend(xQueue_handle, &rx_data, 1000))
        {
          printf("RedTask GRP (%u).\n", rx_data); // Got wrong Package
          vTaskDelay(500);
        }
      }
    }
  }
}

static void Amber_Traffic_LED_Controller_Task(void *pvParameters)
{
  uint16_t rx_data;
  while (1)
  {
    if (xQueueReceive(xQueue_handle, &rx_data, 500))
    {
      if (rx_data == TRAFFIC_LIGHT_A)
      {
        vTaskDelay(250);
        GPIO_ResetBits(GPIOC, TRAFFIC_LIGHT_A);
        printf("Amber off\n");
      }
      else
      {
        if (xQueueSend(xQueue_handle, &rx_data, 1000))
        {
          printf("AmberTask GRP (%u).\n", rx_data); // Got wrong Package
          vTaskDelay(500);
        }
      }
    }
  }
}

static void Green_Traffic_LED_Controller_Task(void *pvParameters)
{
  uint16_t rx_data;
  while (1)
  {
    if (xQueueReceive(xQueue_handle, &rx_data, 500))
    {
      if (rx_data == TRAFFIC_LIGHT_G)
      {
        vTaskDelay(250);
        GPIO_ResetBits(GPIOC, TRAFFIC_LIGHT_G);
        printf("Green off\n");
      }
      else
      {
        if (xQueueSend(xQueue_handle, &rx_data, 1000))
        {
          printf("GreenTask GRP (%u).\n", rx_data); // Got wrong Package
          vTaskDelay(500);
        }
      }
    }
  }
}

void vApplicationMallocFailedHook(void)
{
  /* The malloc failed hook is enabled by setting
  configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

  Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  for (;;)
  {
  }
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
  (void)pcTaskName;
  (void)pxTask;

  /* Run time stack overflow checking is performed if
  configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected.  pxCurrentTCB can be
  inspected in the debugger if the task name passed into this function is
  corrupt. */
  for (;;)
  {
  }
}

void vApplicationIdleHook(void)
{
  volatile size_t xFreeStackSpace;

  /* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
  FreeRTOSConfig.h.

  This function is called on each cycle of the idle task.  In this case it
  does nothing useful, other than report the amount of FreeRTOS heap that
  remains unallocated. */
  xFreeStackSpace = xPortGetFreeHeapSize();

  if (xFreeStackSpace > 100)
  {
    /* By now, the kernel has allocated everything it is going to, so
    if there is a lot of heap remaining unallocated then
    the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
    reduced accordingly. */
  }
}

static void prvSetupHardware(void)
{
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
  GPIO_InitStruct.GPIO_Pin = TRAFFIC_LIGHT_R | TRAFFIC_LIGHT_A | TRAFFIC_LIGHT_G;
  GPIO_InitStruct.GPIO_OType = GPIO_PuPd_UP;
  // For LEDs we're told to use a high speed
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  // Using the full RCC_AH...GPIOC is bad and causes a macro definition error
  GPIO_Init(GPIOC, &GPIO_InitStruct);
}
