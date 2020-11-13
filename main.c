
#include "stm32f1xx.h"
#include <stdio.h>
#include "core_cm3.h"
#include "GPIO_MK.h"
#include "pwm_mk.h"
#include "motor.h"
#include "uart_mk.h"
#include "command.h"

volatile uint32_t R_blink_count = 0;
volatile uint32_t pwm_psc_value = 0;
uint32_t pwm_flag_1 = 1;
uint32_t tmp = 0;

uint32_t *temp = 0;

int main(void)
{

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_0;

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_TIM1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    GPIO_pin_config(GPIOA, PA5, GPIO_OUTPUT_PP_10HZ); // Build_In led output

    // Konfiguracja TIM2 jako wejście enkodera
    //______________________________________________________________________________
    GPIO_write(1, GPIOA, PA0);
    GPIO_write(1, GPIOA, PA1);

    GPIO_pin_config(GPIOA, PA0, GPIO_INPUT_PULL); // wyjscie TIM2 CH1 kanał enkodera
    GPIO_pin_config(GPIOA, PA1, GPIO_INPUT_PULL); // wyjscie TIM2 CH2 kanał enkodera

    TIM2->SMCR |= TIM_SMCR_SMS_1;

    TIM2->CCER |= TIM_CCER_CC1P;
    TIM2->ARR |= 720;

    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
    //_______________________________________________________________________________

    // Konfiguracja TIM1 jako wejście enkodera
    //______________________________________________________________________________
    GPIO_write(1, GPIOA, PA8);
    GPIO_write(1, GPIOA, PA9);

    GPIO_pin_config(GPIOA, PA9, GPIO_INPUT_PULL); // wyjscie TIM2 CH1 kanał enkodera
    GPIO_pin_config(GPIOA, PA8, GPIO_INPUT_PULL); // wyjscie TIM2 CH2 kanał enkodera

    TIM1->SMCR |= TIM_SMCR_SMS_1;

    TIM1->CCER |= TIM_CCER_CC1P;
    TIM1->ARR |= 720;

    TIM1->DIER |= TIM_DIER_UIE;
    TIM1->CR1 |= TIM_CR1_CEN;

    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);
    NVIC_EnableIRQ(TIM1_UP_IRQn);
    //_______________________________________________________________________________

    motor_init();
    uart1_init(9600);
    // ______SET PWM 3 CH 1 and CH2 _________________________________________________
    TIM_setup(TIM3, pwm_mode, CH1);
    TIM_prescaler_set(TIM3, 5);
    TIM_overload_set(TIM3, 500);

    TIM_setup(TIM3, pwm_mode, CH2);
    TIM_prescaler_set(TIM3, 5);
    TIM_overload_set(TIM3, 500);
    //________________________________________________________________________________

    SysTick_Config(8000000 * 0.01);

    motor_init();

    while (1)
    {
        tmp = get_from_buffer();
        if (tmp != 0)
            command_execute(tmp);
    }
}

__attribute__((interrupt)) void SysTick_Handler(void)
{

    // GPIO_toggle(GPIOA, PA5);
}

__attribute__((interrupt)) void TIM2_IRQHandler(void)
{
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CNT = 0;
    //GPIO_toggle(GPIOA, PA5);
}

__attribute__((interrupt)) void TIM1_UP_IRQHandler(void)
{
    TIM1->SR = ~TIM_SR_UIF;
    TIM1->CNT = 0;
    GPIO_toggle(GPIOA, PA5);
}
