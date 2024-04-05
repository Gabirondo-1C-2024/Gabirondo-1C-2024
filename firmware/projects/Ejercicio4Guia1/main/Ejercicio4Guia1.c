/*! @mainpage Template
 *
 * @section genDesc General Description
 * El ejercicio 4 convertir el dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo 
 * pasado como puntero.
 * El ejercicio 5 cambia el estado de cada GPIO, a ‘0’ o a ‘1’, según el estado del bit correspondiente
 * en el BCD ingresado.
 * El ejercicio 6 muestra por display el valor que recibe. Reutilizando las funciones creadas en el punto 4 y 5
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 *
 * @author Valentina Gabirondo (valentina.gabirondo@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
/** @struct gpioConf_t
 * @brief Estructura que define un pin con su nuemro y si es de entrada o salida.
 * 
*/
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/** @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 * @brief Funcion que convierte un dato a BCD guardando cada uno de los dígitos de salida 
 * en el arreglo pasado como puntero.   
 * @param data valor de numero decimal
 * @param digits la cantidad de digitos que tiene el numero
 * @param bcd_number el puntero al arreglo donde se guarda cada digito del BCD
*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (int i = digits; i > 0; i--)
	{

		bcd_number[i - 1] = data % 10;
		data = data / 10;
	}
}

/** @fn void cambioEstado(uint32_t data, gpioConf_t *vectorGPIO)
 * @brief Funcion que cambia el estado de cada GPIO, a ‘0’ o a ‘1’, según el estado del bit correspondiente 
 * en el BCD ingresado   
 * @param data valor de numero decimal
 * @param vectorGPIO puntero al vector de GPIO
*/
void cambioEstado(uint32_t data, gpioConf_t *vectorGPIO)
{
	for (int i = 0; i < 4; i++)
	{
		if ((data & (1 << i)) == 0)
		{
			GPIOOff(vectorGPIO[i].pin);
		}
		else
		{
			GPIOOn(vectorGPIO[i].pin);
		}
	}
}
/** @fn void prendeLEDS(uint32_t data, uint8_t digito, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_LCD)
 * @brief función que mostra por display el valor que recibe. Tiene un vector que mapea los puertos con el 
 * dígito del LCD a donde mostrar un dato.
 * @param data valor de numero decimal
 * @param digits la cantidad de digitos que tiene el numero de data
 * @param vectorGPIO puntero al vector de GPIO
 * @param vectorGPIO_LCD puntero al vector que mapea a cada BCD-7 segmento
*/
void prendeLEDS(uint32_t data, uint8_t digito, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_LCD)
{

	uint8_t arreglo[digito];
	convertToBcdArray(data, digito, arreglo);

	for (int i = 0; 1 < digito; i++)
	{

		cambioEstado(arreglo[i], vectorGPIO);

		GPIOOn(vectorGPIO_LCD[i].pin);
		GPIOOff(vectorGPIO_LCD[i].pin);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	uint32_t numero = 555;
	int digito = 3;
	uint8_t arreglo[digito];
	convertToBcdArray(numero, digito, arreglo);
	/*printf("El numero decimal: %ld",numero);*/

	for (int j = 0; j < digito; j++)
	{
		printf("%d", arreglo[j]);
	}

	gpioConf_t vectorGPIO[4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	GPIOInit(vectorGPIO[0].pin, vectorGPIO[0].dir);
	GPIOInit(vectorGPIO[1].pin, vectorGPIO[1].dir);
	GPIOInit(vectorGPIO[2].pin, vectorGPIO[2].dir);
	GPIOInit(vectorGPIO[3].pin, vectorGPIO[3].dir);

	gpioConf_t vectorGPIO_LCD[3] = {{GPIO_19, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};
	GPIOInit(vectorGPIO_LCD[0].pin, vectorGPIO_LCD[0].dir);
	GPIOInit(vectorGPIO_LCD[1].pin, vectorGPIO_LCD[1].dir);
	GPIOInit(vectorGPIO_LCD[2].pin, vectorGPIO_LCD[2].dir);
	prendeLEDS(numero, digito, vectorGPIO, vectorGPIO_LCD);
	/*for (int j = 0; j < digito; j++)
	{
		cambioEstado(arreglo[j], vectorGPIO);
	}*/
}
/*==================[end of file]============================================*/