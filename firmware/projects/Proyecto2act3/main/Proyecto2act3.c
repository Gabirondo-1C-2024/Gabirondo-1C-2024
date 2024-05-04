/*! @mainpage Proyecto 2 actividad 3
 *
 * @section genDesc General Description
 *En esta actividad se modifico la actividad 2, mide la distancia y ahora se lee el dato por puerto serie, 
 * y se reemplaza la funcion de las teclas 1 y 2 por funciones apretando la letra H (congela el la medicion) 
 * y O (Prende o apaga la medicion) 
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
 * | 03/05/2024 | Document creation		                         |
 *
 * @author Gabirondo Valentina (valentina.gabirondo@ingenieria.uner.edu.ar)
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
#include "lcditse0803.h"
#include "timer_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/


#define ECHO GPIO_3
#define TRIGGER GPIO_2
#define TIME_PERIOD 1000000

/*==================[internal data definition]===============================*/

bool medir = true;
bool hold = true;

/*==================[internal functions declaration]=========================*/
/** @fn void modificarLed(uint16_t distancia)
 * @brief Esta funcion modifica los leds segun que ditancia este midiendo el sensor   
 * @param distancia varible en donde se guarga la ditancia medida
*/
void modificarLed(uint16_t distancia)
{
	if (distancia < 10)
	{
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia > 10 && distancia < 20)
	{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia > 20 && distancia < 30)
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else if (distancia > 30)
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}
/*==================[external functions definition]==========================*/
/** @fn void Led1Task(void *pvParameter)
 * @brief Funcion que tiene una tarea que es medir la ditancia, mostrar los datos por puerto serie y controlar los led, 
 * haciendo que se congele la medida y mida o no. 
*/
void Led1Task(void *pvParameter)
{
	uint16_t distancia;
	

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */
		if (medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			modificarLed(distancia);
			UartSendString(UART_PC, (char*) UartItoa(distancia, 10));
			UartSendString(UART_PC, (char*)" ");
			UartSendString(UART_PC, (char*)"cm");
			UartSendString(UART_PC, (char*) "\r\n");
			if (!hold)
			{
				LcdItsE0803Write(distancia);
			}
		}
		else
		{
			LcdItsE0803Off();
			LedsOffAll();
		}

		//vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
	}
}
/** @fn void read_switch1(void)
 * @brief Funcion que utilizo para darle un valor a la variable medir que luego uso para medir o no 
 * (Prendido/apagado) 
*/
void read_switch1(void){
	medir = !medir;
}
/** @fn void read_switch2(void)
 * @brief Funcion para darle un valor a la variable hold que utilizo para congeler 
 * la pantalla cuando hold es true
*/
void read_switch2(void){
	hold = !hold;
}
/** @fn void funxteclado()
 * @brief Funcion para darle un valor a la variable hold y medir a traves del teclado.
*/
void funxteclado(){
	uint8_t tecla;
	
	UartReadByte(UART_PC, &tecla);
	if(tecla == 'O'){
		medir = !medir;
	}
	else if (tecla =='H'){
		hold = !hold;
	}
}

TaskHandle_t task_handle_medir = NULL;
//TaskHandle_t task_handle_OnOff_medir = NULL;

/** @fn void funcTimer(void* param)
 * @brief Se relacion con el timer A, envia la notificacion para la interrupcion
*/
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle_medir, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE); 
}


void app_main(void)
{

	//inicialización de timers
    timer_config_t timer_global = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = funcTimer,
        .param_p = NULL
    };

	serial_config_t serial_global = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = funxteclado,
		.param_p = NULL

	};

	TimerInit(&timer_global);
	UartInit(&serial_global);
	HcSr04Init(ECHO, TRIGGER);
	LcdItsE0803Init();
	LedsInit();
	SwitchesInit();

	SwitchActivInt(SWITCH_1, &read_switch1, NULL); //on_off
    SwitchActivInt(SWITCH_2, &read_switch2, NULL); //hold

	xTaskCreate(&Led1Task, "medir, prender y apagar led y mostrar", 2048, NULL, 5, &task_handle_medir);
	//xTaskCreate(&on_off_medicion2Task, "prender y apagar con teclas", 512, NULL, 5, NULL);
	
	TimerStart(timer_global.timer);
}
/*==================[end of file]============================================*/