
// This template shows how to use the UART Callback in the HAL to not interrupt the "task"
// until something is in the UART Rx Fifo

#include "semphr.h"

SemaphoreHandle_t uartSemaphore;

void uartCallback(void *callback_arg, cyhal_uart_event_t event)
{
  BaseType_t xHigherPriorityTaskWoken;
  cyhal_uart_enable_event(&cy_retarget_io_uart_obj,CYHAL_UART_IRQ_RX_NOT_EMPTY,7,false);
  xSemaphoreGiveFromISR(uartSemaphore,&xHigherPriorityTaskWoken);
}

void uartTask(void *arg)
{
  setvbuf(stdin, NULL, _IONBF, 0);

  uartSemaphore = xSemaphoreCreateCounting(5,0); // Max=5 - Initial=0

  cyhal_uart_register_callback(&cy_retarget_io_uart_obj, uartCallback, 0);
  cyhal_uart_enable_event(&cy_retarget_io_uart_obj,CYHAL_UART_IRQ_RX_NOT_EMPTY,7,true);

  while(1)
    {
      xSemaphoreTake( uartSemaphore,0xFFFFFFF );
      while(cyhal_uart_readable(&cy_retarget_io_uart_obj))
	{
	  char c = getchar();
	  printf("C=%c\n",c);
	}
      cyhal_uart_enable_event(&cy_retarget_io_uart_obj,CYHAL_UART_IRQ_RX_NOT_EMPTY,7,true);

    }
}
