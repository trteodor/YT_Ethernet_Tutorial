#include "leds.h"

void nucleo_led_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GREEN_LED|BLUE_LED;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

}