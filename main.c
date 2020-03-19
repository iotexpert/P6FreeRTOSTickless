
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

volatile int uxTopUsedPriority ;
TaskHandle_t blinkTaskHandle;
SemaphoreHandle_t xSemaphore = NULL;
#define BLINK_COUNT 6

void buttonEvent(void *callback_arg, cyhal_gpio_event_t event)
{

	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void blinkTask(void *arg)
{
    cyhal_gpio_init(CYBSP_USER_LED,CYHAL_GPIO_DIR_OUTPUT,CYHAL_GPIO_DRIVE_STRONG,0);

    int count=BLINK_COUNT;
    xSemaphore = xSemaphoreCreateBinary(  );
    for(;;)
    {
    	while(count--)
    	{
    		cyhal_gpio_toggle(CYBSP_USER_LED);
    		vTaskDelay(1000);
    	}
    	xSemaphoreTake( xSemaphore,0xFFFFFFFF );
    	count = BLINK_COUNT;

    }
}

int main(void)
{
    cy_rslt_t result;
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ; // enable OpenOCD Thread Debugging

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    cyhal_gpio_init(CYBSP_LED9,CYHAL_GPIO_DIR_OUTPUT,CYHAL_GPIO_DRIVE_STRONG,1);
    cyhal_gpio_init(CYBSP_SW2,CYHAL_GPIO_DIR_INPUT,CYHAL_GPIO_DRIVE_PULLUP,1);
    cyhal_gpio_enable_event(CYBSP_SW2, CYHAL_GPIO_IRQ_FALL, 7, true);
    cyhal_gpio_register_callback(CYBSP_SW2, buttonEvent, 0);

     __enable_irq();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    setvbuf(stdin, NULL, _IONBF, 0);

    // Stack size in WORDs
    // Idle task = priority 0
    xTaskCreate(blinkTask, "blinkTask", configMINIMAL_STACK_SIZE,0 /* args */ ,2 /* priority */, &blinkTaskHandle);
    vTaskStartScheduler();


}

/* [] END OF FILE */
