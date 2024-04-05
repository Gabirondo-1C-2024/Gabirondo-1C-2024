/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
#define CONFIG_BLINK_PERIOD 100
#define ECHO GPIO_3
#define TRIGGER GPIO_2

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500
/*==================[internal data definition]===============================*/
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;

void modificarLed(uint16_t distancia){
	if(distancia<10){
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia>10 && distancia<20)
	{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia>20 && distancia<30)
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else if (distancia>30)
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void Led1Task(void *pvParameter)
{
	uint16_t distancia;

	while (true)
	{
		distancia = HcSr04ReadDistanceInCentimeters();
		
		modificarLed(distancia);

		//printf("La distancia: %d \n", distancia);
		/* 	LedOn(LED_1);
			vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
			printf("LED_1 OFF\n");
			LedOff(LED_1);*/
		vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{

	HcSr04Init(ECHO, TRIGGER);
	LedsInit();
	xTaskCreate(&Led1Task, "medir y prender y apagar led", 2048, NULL, 5, &led1_task_handle);
	// xTaskCreate(&Led2Task, "prender y apagar con teclas y mostrar por pantalla", 512, NULL, 5, &led2_task_handle);
	// xTaskCreate(&Led3Task, "", 512, NULL, 5, &led3_task_handle);
	
}
/*==================[end of file]============================================*/