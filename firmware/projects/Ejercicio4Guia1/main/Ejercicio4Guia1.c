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

/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=digits;i>0;i--){

		bcd_number[i-1] = data % 10;
		data = data/10;
	}
}

void cambioEstado(uint32_t data, gpioConf_t *vectorGPIO){
	for(int i=0;i<4;i++){
		if((data &(1<<i)) == 0){
			GPIOOff(vectorGPIO[i].pin);}
		else {GPIOOn(vectorGPIO[i].pin);
		}
	}
}

void prendeLEDS(uint32_t data, uint8_t digito, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_LCD){

uint8_t arreglo [digito];
convertToBcdArray (data, digito, arreglo);

for(int i=0;1<digito;i++){
	
	cambioEstado(arreglo[i],vectorGPIO);

	GPIOOn(vectorGPIO_LCD[i].pin);
	GPIOOff(vectorGPIO_LCD[i].pin);

}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	
uint32_t numero = 123;
int digito = 3;
/*uint8_t arreglo [2];
convertToBcdArray (numero, 3, arreglo);
printf("El numero decimal: %ld",numero);*/


for(int j=0;j<digito;j++){
	printf("%d",arreglo[j]);
}

gpioConf_t vectorGPIO[4]={{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};
GPIOInit(vectorGPIO[0].pin, vectorGPIO[0].dir);
GPIOInit(vectorGPIO[1].pin, vectorGPIO[1].dir);
GPIOInit(vectorGPIO[2].pin, vectorGPIO[2].dir);
GPIOInit(vectorGPIO[3].pin, vectorGPIO[3].dir);

gpioConf_t vectorGPIO_LCD[3]={{GPIO_19,GPIO_OUTPUT},{GPIO_18,GPIO_OUTPUT},{GPIO_9,GPIO_OUTPUT}};
GPIOInit(vectorGPIO_LCD[0].pin, vectorGPIO_LCD[0].dir);
GPIOInit(vectorGPIO_LCD[1].pin, vectorGPIO_LCD[1].dir);
GPIOInit(vectorGPIO_LCD[2].pin, vectorGPIO_LCD[2].dir);



for(int j=0;j<digito;j++){
	cambioEstado(arreglo[j],vectorGPIO);
}




}
/*==================[end of file]============================================*/