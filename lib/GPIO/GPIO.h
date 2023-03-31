#ifndef __GPIO_H__
#define __GPIO_H__


#include <Arduino.h>

#define LOWLED 0x1
#define HIGHLED 0x0

#if !defined(useRev2)
#define Input1 D7 
#define Input2 D6 
#define Input3 D5 
#elif defined(useRev2)
#define Input1 D5 
#define Output1 D7 
#define Output2 D6
#endif

#define LED_BUILTIN 2 //Esp8266: D4 ESP32: D2

#define IO1 "001"
#define IO2 "002"
#define IO3 "003"

#define windowOnOffOn 800
#define windowLed 200
#define sizeGpioBuffer 10

void gpioReadInput1();
#if !defined(useRev2)
void gpioReadInput2();

void gpioReadInput3();
#endif
void gpio_setup();

void gpio_loop();

void gpio_cambiar_salida(byte salida, byte estado);
#endif // __GPIO_H__