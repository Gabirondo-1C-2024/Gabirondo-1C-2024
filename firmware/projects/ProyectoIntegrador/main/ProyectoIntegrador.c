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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "buzzer.h"
#include "gpio_mcu.h"
#include "led.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "ble_mcu.h"
#include "MQ2sensor.h"
#include "gpio_mcu.h"
#include "pwm_mcu.h"


/*==================[macros and definitions]=================================*/
#define TIME_PERIOD 2000000 //tiempo de sensado micro
#define BUZZER GPIO_3

uint16_t RO_ = 0; //!< resistencia R0 a aire en condiciones normales.
float valorGasLPG = 0; //!< valor de gas butano medido.
uint16_t valor_convertido = 0; //!< valor de gas butano convertido a entero.
//uint16_t umbralLPG = 1000; //!< umbral inicial de gas butano.
bool estadoMedicion = true; //!< condicion de medicion.


#define AIA GPIO_22
#define AIB GPIO_19
#define BIB GPIO_21
#define BIA GPIO_20

uint16_t valor_gas;

/*==================[internal data definition]===============================*/
char adelante[]={255,1,1,1,2,0,1,0};
char atras[] = {255,1,1,1,2,0,2,0};
char stop[] = {255,1,1,1,2,0,0,0};
char derecha[] = {255,1,1,1,2,0,8,0};
char izquierda[] = {255,1,1,1,2,0,4,0};

/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE); 
}
void funcTimer1(void* param){
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
    vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
    //vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE); 
}

//Esta tarea lee el valor de gas
static void tarea_leer_gas (void* pvParameter){
    //MQInit(GPIO_1);
    //RO_ = MQCalibration();

    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */
        if(estadoMedicion){
            float read= MQRead();
            valorGasLPG = MQGetPercentage(read/RO_);
    	    valor_convertido = (uint16_t)(valorGasLPG*100);
    	    printf("Valor concetracion:  %d, %f \n", valor_convertido, read);
        }
        //vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

//Lee la se;al de gas y encinde la alamra cunado el el nivel de gas pasaun umbral preestablecido 
void Alarma_ON(void* pvParameter){

	uint16_t umbral=93;
	uint16_t freq = 500;
	uint16_t duration = 500;
	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(valor_convertido >umbral){
			//BuzzerOn();
			BuzzerPlayTone(freq, duration);
			
		}
        //vTaskDelay(500/portTICK_PERIOD_MS);

	}

}

/*==================[external functions definition]==========================*/

void read_data(uint8_t * data, uint8_t length){
    for(int i=0;i<length;i++){
       printf("%d \n", data[i]);
    }
    printf("fin\n");
    if(data[0]==adelante[0]&&data[1]==adelante[1]&&data[2]==adelante[2]&&data[3]==adelante[3]&&data[4]==adelante[4]&&data[5]==adelante[5]&&data[6]==adelante[6]&&data[7]==adelante[7]){
        
        //PWMSetDutyCycle(PWM_3, 100);
        PWMSetDutyCycle(PWM_0, 100);
        PWMSetDutyCycle(PWM_1, 100);
        //PWMSetDutyCycle(PWM_2, 0);
        PWMOn(PWM_0);
        //PWMOn(PWM_3);
        PWMOn(PWM_1);
        //PWMOn(PWM_2);
    }
    
    // if(data[0]==atras[0]&&data[1]==atras[1]&&data[2]==atras[2]&&data[3]==atras[3]&&data[4]==atras[4]&&data[5]==atras[5]&&data[6]==atras[6]&&data[7]==atras[7]){
        
    //     PWMSetDutyCycle(PWM_1, 100);
    //     PWMSetDutyCycle(PWM_2, 100);
    //     PWMSetDutyCycle(PWM_3, 0);
    //     PWMSetDutyCycle(PWM_0, 0);
    //     PWMOn(PWM_1);
    //     PWMOn(PWM_2);
    //     PWMOn(PWM_0);
    //     PWMOn(PWM_3);
    // }
    if(data[0]==stop[0]&&data[1]==stop[1]&&data[2]==stop[2]&&data[3]==stop[3]&&data[4]==stop[4]&&data[5]==stop[5]&&data[6]==stop[6]&&data[7]==stop[7]){
        
        PWMSetDutyCycle(PWM_0, 0);
        //PWMSetDutyCycle(PWM_3, 0);
        PWMSetDutyCycle(PWM_1, 0);
        //PWMSetDutyCycle(PWM_2, 0);
        PWMOn(PWM_1);
        //PWMOn(PWM_2);
        PWMOn(PWM_0);
        //PWMOn(PWM_3);
    }
    if(data[0]==derecha[0]&&data[1]==derecha[1]&&data[2]==derecha[2]&&data[3]==derecha[3]&&data[4]==derecha[4]&&data[5]==derecha[5]&&data[6]==derecha[6]&&data[7]==derecha[7]){
        
        //PWMSetDutyCycle(PWM_1, 0);
        //PWMSetDutyCycle(PWM_2, 0);
        PWMSetDutyCycle(PWM_0, 90);
        PWMSetDutyCycle(PWM_1, 50);
        PWMOn(PWM_0);
        //PWMOn(PWM_3);
        PWMOn(PWM_1);
        //PWMOn(PWM_2);
    }
    if(data[0]==izquierda[0]&&data[1]==izquierda[1]&&data[2]==izquierda[2]&&data[3]==izquierda[3]&&data[4]==izquierda[4]&&data[5]==izquierda[5]&&data[6]==izquierda[6]&&data[7]==izquierda[7]){
        
        //PWMSetDutyCycle(PWM_1, 0);
        //PWMSetDutyCycle(PWM_2, 0);
        PWMSetDutyCycle(PWM_0, 50);
        PWMSetDutyCycle(PWM_1, 90);
        PWMOn(PWM_0);
        //PWMOn(PWM_3);
        PWMOn(PWM_1);
        //PWMOn(PWM_2);
    }
};

void app_main(void){



	//inicialización de timers
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = funcTimer,
        .param_p = NULL
    };

    ble_config_t ble_configuration = {
        "VALENTINA",
		read_data
        
    };
    //Inicializo los GPIO en salida para los motores.
    GPIOInit(AIA, GPIO_OUTPUT);
    GPIOInit(AIB, GPIO_OUTPUT);
    GPIOInit(BIA, GPIO_OUTPUT);
    GPIOInit(BIB, GPIO_OUTPUT);

    //Inicializo PWM
    PWMInit(PWM_0,AIA,50);
    //PWMInit(PWM_1,AIB,50);
    //PWMInit(PWM_2,BIA,50);
    PWMInit(PWM_1,BIB,50);

    MQInit(GPIO_1);
    RO_ = MQCalibration();

	BleInit(&ble_configuration);

	BuzzerInit(BUZZER);

	TimerInit(&timer_1);
    
    xTaskCreate(&tarea_leer_gas, "lee la senial de gas", 4069, NULL, 5, &task_handle1);
	xTaskCreate(&Alarma_ON, "hace sonar una alrma", 4069, NULL, 5, &task_handle2);

    TimerStart(timer_1.timer);
 

}

/*==================[end of file]============================================*/