/*! @mainpage Template
 *
 * @section genDesc General Description
 * 
 *Este codigo hace una función que recibe un puntero a una estructura LED. La funcion enciende, apaga y hace titilar (con cierta frecuencia) un led 1, 2 o 3.
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 *
 * @author Valentina Gabirondo (valentina.gabirondo@ingenineria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/** @def ON
 * @brief Enciende el led
*/
#define ON 1
/** @def OFF
 * @brief Apaga el led
*/
#define OFF 0
/** @def TOGGLE
 * @brief Hace itilar el led
*/
#define TOGGLE 2
/** @def CONFIG_BLINK_PERIOD
 * @brief Selecciona un periodo, para hacer titilar el led a cierta frecuencia.
*/
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
/** @struct my_leds
 * @brief Estructura de leds que que me define un led.
 * 
*/
struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds;

/*==================[internal functions declaration]=========================*/
/** @fn void controlLed(struct leds *leds_ptr)
 * @brief Funcion que controla cada Led prendiendo, apagando y modificando el estado entre prendido y apagado.   
 * @param leds_ptr puntero a un struct leds
*/
void controlLed(struct leds *leds_ptr){

		switch (leds_ptr->mode)

		{
		case ON:
			if(leds_ptr -> n_led==LED_1){
				LedOn(LED_1);
			}
			else if(leds_ptr -> n_led==LED_2){
			LedOn(LED_2);
			}
			else if(leds_ptr -> n_led==LED_3){
			LedOn(LED_3);
			}
		break;
			case OFF:
			if(leds_ptr -> n_led==LED_1){
				LedOff(LED_1);
			}
			else if(leds_ptr -> n_led==LED_2){
			LedOff(LED_2);
			}
			else if(leds_ptr -> n_led==LED_3){
			LedOff(LED_3);
			}
		break;
			case TOGGLE:
			for(int i=0;i<leds_ptr->n_ciclos;i++){
				if(leds_ptr->n_led == LED_1){
				LedToggle(LED_1);}
				else if(leds_ptr->n_led == LED_2){
				LedToggle(LED_2);}
				else if(leds_ptr->n_led == LED_3){
				LedToggle(LED_3);}
				for(int j=0;j<leds_ptr->periodo;j++){
				vTaskDelay(CONFIG_BLINK_PERIOD/portTICK_PERIOD_MS);
			}
		}
		break;
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
LedsInit();
struct leds my_leds;
my_leds.mode=2;
my_leds.n_led=LED_3;
my_leds.n_ciclos=100;
my_leds.periodo=5;
controlLed(&my_leds);


}
/*==================[end of file]============================================*/