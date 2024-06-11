/*! @mainpage Parcial
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink at different rates, using FreeRTOS tasks and timer interrupts.
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
#define T1 3000000 //3 SEGUNDOS
#define T2 5000000 //5 SEGUNDOS
/*==================[internal data definition]===============================*/
TaskHandle_t control_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
float ph;

bool medir = true;
bool detener = true;
bool pHA =true;
bool pHB =true;

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

void read_switch1(void)
{
    medir = !medir;
}
void read_switch2(void)
{
    detener = !detener;
}

/**
 * @brief Tarea encargada de blinkear el LED_1
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
            ph = (14 * valorVolt) / 3;
            if (ph < 6.0)
            {
                GPIOOn(BOMBA_BASICA);
                pHB=true;
            }
            else pHB=false;
            if (ph > 6.7)
            {
                GPIOOn(BOMBA_ACIDA);
                pHA=true;
            }
            else pHA = false;
            
            if ((GPIORead(HUMEDAD))==true){
                GPIOOn(BOMBA_AGUA);
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

/**
 * @brief Tarea encargada de blinkear el LED_2
 */
static void mostrar(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        if(medir == true && detener == false){

            if(HUMEDAD==true){

            UartSendString(UART_PC, (char*)"El pH: ");
            UartSendString(UART_PC, (char*) UartItoa(ph, 10));
			UartSendString(UART_PC, (char*)"humedad es incorrecta");
            UartSendString(UART_PC, (char*) "\r\n");
            UartSendString(UART_PC, (char*)"Bomba de agua encendida");
            UartSendString(UART_PC, (char*) "\r\n");
            }
            else if(HUMEDAD==false){
            UartSendString(UART_PC, (char*)"El pH: ");
            UartSendString(UART_PC, (char*) UartItoa(ph, 10));
			UartSendString(UART_PC, (char*)"humedad es correcta");
            UartSendString(UART_PC, (char*) "\r\n");
            }
            else if()

    }}
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
    xTaskCreate(&control, "controla el estado de la planta", 512, NULL, 5, &control_task_handle);
    xTaskCreate(&mostrar, "muestra datos", 512, NULL, 5, &mostrar_task_handle);
    /* Inicialización del conteo de timers */
    TimerStart(timer_1.timer);
    TimerStart(timer_2.timer);
}