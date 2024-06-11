/*! @mainpage Parcial
 *
 * \section 
 *
 * En este proyecto se realiza el control a una planta dependido del ph se enciende las bombas de 
 * ph acido o de basico y dependiendo la humedad se prende o apaga la bomba de agua. Se informa por UART si las bombas
 * estan encendidas, el ph de la planata y si la humedad es correcta o incorrecta.
 *
 *  @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	BOMBA_ACIDA	 | 	GPIO_19		|
 * | 	HUMEDAD	 	| 	GPIO_20		|
 * | 	BOMBA_AGUA	 | 	GPIO_21		|
 * | 	BOMBA_BASICA| 	GPIO_22		|
 * | 	SENSOR DE pH| 	 CH1		|
 *
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Valentina Gabirondo (valentina.gabirondo@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "gpio_mcu.h"
#include "analog_io_mcu.h"
#include "switch.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define T1 3000000 // 3 SEGUNDOS
#define T2 5000000 // 5 SEGUNDOS
/*==================[internal data definition]===============================*/
TaskHandle_t control_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
float ph;

bool medir = true;   // bandera para medir
bool detener = true; // bandera para detener
bool pHA = true;     // bandera para saber si esta encendida la bomba
bool pHB = true;     // bandera para saber si esta encendida la bomba

// defino los gpio a usar

#define BOMBA_BASICA GPIO_22
#define BOMBA_ACIDA GPIO_19
#define HUMEDAD GPIO_20
#define BOMBA_AGUA GPIO_21
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void *param)
{
    vTaskNotifyGiveFromISR(control_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void *param)
{
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_2 */
}
/** @fn void read_switch1(void)
 * @brief Funcion que utilizo para darle un valor a la variable medir que luego uso para medir o no
 * (Prendido/apagado)
 */
void read_switch1(void)
{
    medir = !medir;
}
/** @fn void read_switch2(void)
 * @brief Funcion para darle un valor a la variable detener que utilizo para apagar el sistema
 *
 */
void read_switch2(void)
{
    detener = !detener;
}

/** @fn static void control(void *pvParameter)
 * @brief Tarea encargada de controlar el estado de la planata, que el ph este en el rango correcto y la humedad tambien.
 * Esto lo realizo prendiendo y apagando los Gpio de las respectivas bombas.
 */
static void control(void *pvParameter)
{
    float valorVolt;

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        if (medir == true && detener == false)
        {

            AnalogInputReadSingle(CH1, &valorVolt);
            ph = (14.0 * valorVolt) / 3000.0;

            if (ph < 6.0)
            {
                GPIOOn(BOMBA_BASICA);
                pHB = true;
            }
            else
            {
                GPIOOff(BOMBA_BASICA);
                pHB = false;
            }
            if (ph > 6.7)
            {
                GPIOOn(BOMBA_ACIDA);
                pHA = true;
            }
            else
            {
                GPIOOff(BOMBA_ACIDA);
                pHA = false;
            }
            if ((GPIORead(HUMEDAD)) == true)
            {
                GPIOOn(BOMBA_AGUA);
            }
            else
            {
                GPIOOff(BOMBA_AGUA);
            }
        }
        else if (medir == false && detener == true)
        {
            GPIOOff(BOMBA_ACIDA);
            GPIOOff(BOMBA_ACIDA);
            GPIOOff(BOMBA_AGUA);
        }
    }
}


/** @fn static mostrar(void* pvParameter)
 * @brief Tarea encargada de mostrar los diferentes mensajes por UART cada 5 segundos
 */
static void mostrar(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        if (medir == true && detener == false)
        {

            if (HUMEDAD == true)
            {

                UartSendString(UART_PC, (char *)"El pH: ");
                UartSendString(UART_PC, (char *)UartItoa(ph, 10));
                UartSendString(UART_PC, (char *)",humedad incorrecta");
                UartSendString(UART_PC, (char *)"\r\n");
                UartSendString(UART_PC, (char *)"Bomba de agua encendida");
                UartSendString(UART_PC, (char *)"\r\n");
            }
            else if (HUMEDAD == false)
            {
                UartSendString(UART_PC, (char *)"El pH: ");
                UartSendString(UART_PC, (char *)UartItoa(ph, 10));
                UartSendString(UART_PC, (char *)", humedad correcta");
                UartSendString(UART_PC, (char *)"\r\n");
            }
            if (pHA == true)
            {
                UartSendString(UART_PC, (char *)"Bomba de pH acido encendida");
                UartSendString(UART_PC, (char *)"\r\n");
            }
            else if (pHB == true)
            {
                UartSendString(UART_PC, (char *)"Bomba de pH basico encendida");
                UartSendString(UART_PC, (char *)"\r\n");
            }
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

    GPIOInit(BOMBA_ACIDA, GPIO_OUTPUT);
    GPIOInit(BOMBA_BASICA, GPIO_OUTPUT);
    GPIOInit(BOMBA_AGUA, GPIO_OUTPUT);
    GPIOInit(HUMEDAD, GPIO_INPUT);

    SwitchActivInt(SWITCH_1, &read_switch1, NULL);
    SwitchActivInt(SWITCH_2, &read_switch2, NULL);

    analog_input_config_t analogInput1 = {
        .input = CH1,
        .mode = ADC_SINGLE,

    };

    serial_config_t serial_global = {
        .port = UART_PC,
        .baud_rate = 115200, // unidad de transmision de datos, vel de l a se;al
        .func_p = NULL,
        .param_p = NULL};

    UartInit(&serial_global);

    /* Inicialización de timers */
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = T1,
        .func_p = FuncTimerA,
        .param_p = NULL};
    TimerInit(&timer_1);
    timer_config_t timer_2 = {
        .timer = TIMER_B,
        .period = T2,
        .func_p = FuncTimerB,
        .param_p = NULL};
    TimerInit(&timer_2);
    /* Creación de tareas */
    xTaskCreate(&control, "controla el estado de la planta", 2048, NULL, 5, &control_task_handle);
    xTaskCreate(&mostrar, "muestra datos", 2048, NULL, 5, &mostrar_task_handle);
    /* Inicialización del conteo de timers */
    TimerStart(timer_1.timer);
    TimerStart(timer_2.timer);
}