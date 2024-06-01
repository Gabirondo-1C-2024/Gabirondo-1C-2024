/*! @mainpage Proyecto 2 actividad 4
 *
 * @section genDesc General Description
 *Actividad que tiene como objetivo relalizar un osciloscopio. Conviertiendo señales analogicas a digitales
 *para que sean visualizadas y tambien convierte de digital a analogica. 
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
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define TIME_PERIOD 2000 //micro
#define TIME_PERIOD2 4000
uint8_t indice = 0;
/*==================[internal data definition]===============================*/
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
//TaskHandle_t main_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle1 = NULL;
TaskFunction_t task_handle2 =NULL;
/** @fn void funcTimer(void* param)
 * @brief Se relacion con el timer A y B, envia la notificacion para la interrupcion  
*/
void funcTimer1(void* param){
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE); 
}
void funcTimer2(void* param){

    vTaskNotifyGiveFromISR(task_handle2, pdFALSE);

}
/** @fn static void tarea_leer (void* pvParameter)
 * @brief Esta funcion transforma una señal analogica a digital y la envia por puerto serie 
*/
static void tarea_leer (void* pvParameter){
    uint16_t valorVolt;
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */
        AnalogInputReadSingle(CH1, &valorVolt);
        UartSendString(UART_PC, (char*) UartItoa(valorVolt, 10));
        UartSendString(UART_PC, (char*) "\r\n");
    }
}
/** @fn static void levantar_ecg(void* pvParamete)
 * @brief Transforma los datos del ecg digitales a analogicos
*/
static void levantar_ecg(void* pvParamete){
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(indice < sizeof(ecg)){
            AnalogOutputWrite(ecg[indice]);

        }
        else if(indice == sizeof(ecg)){
            indice =0;

        }
        indice++;

    }
    
}
/*==================[external functions definition]==========================*/
void app_main(void){

    analog_input_config_t analogInput1 = {
        .input = CH1,
        .mode = ADC_SINGLE,

    };

    //     analog_input_config_t analogInput2 = {
    //     .input = CH0,
    //     .mode = ADC_SINGLE,

    // };
    
    	//inicialización de timers
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = funcTimer1,
        .param_p = NULL
    };

        timer_config_t timer_2 = {
        .timer = TIMER_B,
        .period = TIME_PERIOD2,
        .func_p = funcTimer2,
        .param_p = NULL
    };

    serial_config_t serial_global = {
		.port = UART_PC,
		.baud_rate = 115200,//unidad de transmision de datos, vel de l a se;al
		.func_p = NULL,
		.param_p = NULL
    };

    TimerInit(&timer_1);
    TimerInit(&timer_2);
    AnalogOutputInit();
    AnalogInputInit(&analogInput1);
    xTaskCreate(&tarea_leer, "leer y enviar", 2048, NULL, 5, &task_handle1);
    xTaskCreate(&levantar_ecg, "leer y enviar", 2048, NULL, 5, &task_handle2);
    UartInit(&serial_global);
    TimerStart(timer_1.timer);
    TimerStart(timer_2.timer);

}
/*==================[end of file]============================================*/