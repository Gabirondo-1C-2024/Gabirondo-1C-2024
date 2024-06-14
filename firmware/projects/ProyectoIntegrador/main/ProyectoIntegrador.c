/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Se implementa un robot móvil capaz de detectar gases peligrosos e inflamables, que al detectar el gas
 * se enciende una alarama. El robot se controla por Bluetooth mediante una aplicacion de smartphone,
 * el cual tiene las funciones de atras, adelante, derecha e izquierda.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	AIA	 	    | 	GPIO_22		|
 * | 	AIB	 	    | 	GPIO_19		|
 * | 	BIB	 	    | 	GPIO_21		|
 * | 	BIA	 	    | 	GPIO_20		|
 * 
 *
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 19/06/2024 | Fecha de entrega		                         |
 *
 * @author Valentina Gabirondo (valentina.gabirondo@ingenieria.uner.edu.ar)
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
#define TIME_PERIOD 500000 // tiempo en us

/** @brief GPIO_3 lo utilizo para el buzzer .*/
#define BUZZER GPIO_3

#define AIA GPIO_22
#define AIB GPIO_19
#define BIB GPIO_21
#define BIA GPIO_20

/** @brief R0_ resistencia R0 a aire en condiciones normales.*/
uint16_t RO_ = 0;

/** @brief valorGasLPG valor de gas butano medido.*/
float valorGasLPG = 0;

/** @brief valorGasLPG valor de gas butano convertido a entero.*/
volatile uint16_t valor_convertido = 0;

/** @brief estadoMedicion bandera condicion de medicion.*/
bool estadoMedicion = true; 

/** @brief promedio es una variable uint16_t que guarda el promedio de 5 valores de gas.*/
uint16_t promedio;

/*==================[internal data definition]===============================*/

/** @brief adelante es una lista de comando "adelante" para la aplicacion Dabble*/
char adelante[] = {255, 1, 1, 1, 2, 0, 1, 0};

/** @brief atras es una lista de comando "atras" para la aplicacion Dabble*/
char atras[] = {255, 1, 1, 1, 2, 0, 2, 0};

/** @brief stop es una lista de comando "parar" para la aplicacion Dabble*/
char stop[] = {255, 1, 1, 1, 2, 0, 0, 0};

/** @brief derecha es una lista de comando "doblar a la derecha" para la aplicacion Dabble*/
char derecha[] = {255, 1, 1, 1, 2, 0, 8, 0};

/** @brief izquierda es una lista de comando "doblar a la izquierda" para la aplicacion Dabble*/
char izquierda[] = {255, 1, 1, 1, 2, 0, 4, 0};

/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;

/**
 * @brief Función invocada en la interrupción del timer
 */
void funcTimer(void *param)
{
    vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
}

/** @fn static void tarea_leer_gas(void *pvParameter)
 * @brief Esta tarea lee el valor del gas, lo convierte a ppm y realiza un promedio con 5 muestras cada 0.5 s usando timer.
 */
static void tarea_leer_gas(void *pvParameter)
{
    uint16_t vect[5];
    uint32_t acum = 0;
    int i = 0;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        if (estadoMedicion)
        {
            float read = MQRead();
            valorGasLPG = MQGetPercentage(read / RO_);
            valor_convertido = (uint16_t)(valorGasLPG * 1000000);
            if (i < 5)
            {
                vect[i] = valor_convertido;
                acum += vect[i];
                i++;
            }
            else
            {
                promedio = acum / 5;
                acum = 0;
                i = 0;
            }

            // printf("Valor concetracion:  %d, %f \n", valor_convertido, read);
        }
    }
}

/** @fn static void Alarma_ON(void *pvParameter)
 * @brief Tarea encargada de encender una alarma cuando el valor del gas supera el umbral preestablecido cada 0.5 s usando delay.
 */
void Alarma_ON(void *pvParameter)
{

    const uint16_t umbral = 30000;
    // const uint16_t freq = 500;
    // const uint16_t duration = 200;

    while (1)
    {
        // printf("%d\n", promedio);
        if (promedio > umbral)
        {
            BuzzerOn();
            LedOn(LED_1);
        }
        else
        {
            BuzzerOff();
            LedOff(LED_1);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
/** @fn void read_data(uint8_t *data)
 * @brief Funcion que utilizo para controlar los motores con PWM y GPIO.
 * @param data uint8_t variable que guarda los caracteres que le mando por bluetooth.
 */
void read_data(uint8_t *data)
{

    if (data[0] == adelante[0] && data[1] == adelante[1] && data[2] == adelante[2] && data[3] == adelante[3] && data[4] == adelante[4] && data[5] == adelante[5] && data[6] == adelante[6] && data[7] == adelante[7])
    {

        PWMSetDutyCycle(PWM_0, 100);
        PWMSetDutyCycle(PWM_1, 100);
        GPIOOff(AIB);
        GPIOOff(BIA);

        PWMOn(PWM_0);
        PWMOn(PWM_1);
    }

    if (data[0] == atras[0] && data[1] == atras[1] && data[2] == atras[2] && data[3] == atras[3] && data[4] == atras[4] && data[5] == atras[5] && data[6] == atras[6] && data[7] == atras[7])
    {

        PWMSetDutyCycle(PWM_1, 0);
        PWMSetDutyCycle(PWM_2, 0);
        GPIOOn(AIB);
        GPIOOn(BIA);
        PWMOn(PWM_1);
        PWMOn(PWM_2);
    }
    if (data[0] == stop[0] && data[1] == stop[1] && data[2] == stop[2] && data[3] == stop[3] && data[4] == stop[4] && data[5] == stop[5] && data[6] == stop[6] && data[7] == stop[7])
    {

        PWMSetDutyCycle(PWM_0, 0);
        PWMSetDutyCycle(PWM_1, 0);

        PWMOn(PWM_1);
        PWMOn(PWM_0);

        GPIOOff(AIB);
        GPIOOff(BIA);
    }
    if (data[0] == derecha[0] && data[1] == derecha[1] && data[2] == derecha[2] && data[3] == derecha[3] && data[4] == derecha[4] && data[5] == derecha[5] && data[6] == derecha[6] && data[7] == derecha[7])
    {

        PWMSetDutyCycle(PWM_0, 90);
        PWMSetDutyCycle(PWM_1, 50);
        PWMOn(PWM_0);

        PWMOn(PWM_1);

        GPIOOff(AIB);
        GPIOOff(BIA);
    }
    if (data[0] == izquierda[0] && data[1] == izquierda[1] && data[2] == izquierda[2] && data[3] == izquierda[3] && data[4] == izquierda[4] && data[5] == izquierda[5] && data[6] == izquierda[6] && data[7] == izquierda[7])
    {

        PWMSetDutyCycle(PWM_0, 50);
        PWMSetDutyCycle(PWM_1, 90);
        PWMOn(PWM_0);

        PWMOn(PWM_1);

        GPIOOff(AIB);
        GPIOOff(BIA);
    }
};

void app_main(void)
{

    LedsInit();
    // inicialización de timers
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = funcTimer,
        .param_p = NULL};

    ble_config_t ble_configuration = {
        "VALENTINA",
        read_data

    };
    // Inicializo los GPIO en salida para los motores.
    GPIOInit(AIA, GPIO_OUTPUT);
    GPIOInit(AIB, GPIO_OUTPUT);
    GPIOInit(BIA, GPIO_OUTPUT);
    GPIOInit(BIB, GPIO_OUTPUT);

    // Inicializo PWM
    PWMInit(PWM_0, AIA, 50);
    PWMInit(PWM_1, BIB, 50);

    BuzzerInit(BUZZER);
    BuzzerSetFrec(500);

    MQInit(GPIO_1);
    RO_ = MQCalibration();

    BleInit(&ble_configuration);

    TimerInit(&timer_1);

    xTaskCreate(&tarea_leer_gas, "lee la senial de gas y hace un promedio", 4069, NULL, 5, &task_handle1);
    xTaskCreate(&Alarma_ON, "hace sonar una alarma cuando suena un umbral", 4069, NULL, 5, &task_handle2);

    TimerStart(timer_1.timer);
}

/*==================[end of file]============================================*/