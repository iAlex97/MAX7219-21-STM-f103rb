/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "MAX7219.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#include "stm32f10x.h"
#include <stdlib.h>
#include <string.h>

#define LSB 2
#define MSB 1


// Definitii
GPIO_InitTypeDef GPIO_InitStructure; // IO initialization structure

static volatile __IO uint32_t TimingDelay;

void SysTick_Handler(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}

void Delay(__IO uint32_t nTime)
{
	 TimingDelay = nTime;
	 while(TimingDelay != 0);
}

void initGPIO_LED()
{	// enable clock to GPIOA
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	// enable clock to GPIOB
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

	// enable AFIO clock
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
	// disable JTAG and JTAG-DP
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE );
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_JTAGDisable, ENABLE );

	// save pin speed and pin mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

		// save pin speed and pin mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
}


void shiftOut(uint8_t bitOrder, uint16_t val)
{
	uint16_t i;

	GPIOB->BRR	= GPIO_Pin_5; // chip set enable

	for (i = 0; i < 16; i++)  {
		if ( bitOrder == LSB)
				if (val & (1 << i))
					GPIOA->BSRR = GPIO_Pin_10; //bit set
				else
					GPIOA->BRR = GPIO_Pin_10; // bit reset

		else
				if (val & (1 << (15 - i)))
					GPIOA->BSRR = GPIO_Pin_10; //bit set
				else
					GPIOA->BRR = GPIO_Pin_10; // bit reset

		GPIOB->BSRR = GPIO_Pin_4; //clock signal
		//Delay(10);
		GPIOB->BRR	= GPIO_Pin_4;
	}

	GPIOB->BSRR = GPIO_Pin_5; // chip set reset
}

void displayLeds(uint16_t *vec, int sz) {
	int i;

	for (i = 0; i < sz; i++) {
		shiftOut(MSB, vec[i]);
		Delay(500);
	}
}

void displayLedsNoDelay(uint16_t *vec, int sz) {
	int i;

	for (i = 0; i < sz; i++) {
		shiftOut(MSB, vec[i]);
		Delay(10);
	}
}

void displayLoader(uint16_t *vec, int sz) {
	int i;

	for (i = 0; i < sz; i++) {
		shiftOut(MSB, vec[i]);

		Delay(100);
	}
}

void joc5() {
	int i;
	int afisare = 1;

	uint16_t intensity_arr[] = {
			CONST_INTENSITY_1,
			CONST_INTENSITY_2,
			CONST_INTENSITY_3,
			CONST_INTENSITY_4,
			CONST_INTENSITY_5,
			CONST_INTENSITY_6,
			CONST_INTENSITY_7,
			CONST_INTENSITY_8,
			CONST_INTENSITY_9,
			CONST_INTENSITY_A,
			CONST_INTENSITY_B,
			CONST_INTENSITY_C,
			CONST_INTENSITY_D,
			CONST_INTENSITY_E,
			CONST_INTENSITY_F,

	};

	uint16_t init_disp[] = {
			REG_SHUTDOWN | CONST_NORMAL_OP, // shutdown register - normal op
			REG_DECODE_MODE | CONST_DECODE_MODE_1, // no decode
			REG_SCAN_LIMIT | CONST_SCAN_LIMIT_7, // scan mode 8 digits
	};

	uint16_t rst_disp[] = {
			REG_DIGIT_7 | CONST_NONE,
			REG_DIGIT_6 | CONST_NONE,
			REG_DIGIT_5 | CONST_NONE,
			REG_DIGIT_4 | CONST_NONE,
			REG_DIGIT_3 | CONST_NONE,
			REG_DIGIT_2 | CONST_NONE,
			REG_DIGIT_1 | CONST_NONE,
			REG_DIGIT_0 | CONST_NONE,
	};

	uint16_t load_disp[] = {
			REG_DIGIT_7 | CONST_SEG_A,
			REG_DIGIT_6 | CONST_SEG_A,
			REG_DIGIT_5 | CONST_SEG_A,
			REG_DIGIT_4 | CONST_SEG_A,
			REG_DIGIT_3 | CONST_SEG_A,
			REG_DIGIT_2 | CONST_SEG_A,
			REG_DIGIT_1 | CONST_SEG_A,
			REG_DIGIT_0 | CONST_SEG_A,
			REG_DIGIT_0 | CONST_SEG_B | CONST_SEG_A,
			REG_DIGIT_0 | CONST_SEG_C | CONST_SEG_B | CONST_SEG_A,
			REG_DIGIT_0 | CONST_SEG_C | CONST_SEG_B | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_1 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_2 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_3 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_4 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_5 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_6 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_7 | CONST_SEG_D | CONST_SEG_A,
			REG_DIGIT_7 | CONST_SEG_D | CONST_SEG_E | CONST_SEG_A,
			REG_DIGIT_7 | CONST_SEG_D | CONST_SEG_E | CONST_SEG_F | CONST_SEG_A,
	};

	uint16_t c1 = CONST_SEG_A | CONST_SEG_B | CONST_SEG_F | CONST_SEG_E;
	uint16_t c2 = CONST_SEG_A | CONST_SEG_B | CONST_SEG_F | CONST_SEG_C;

	uint16_t u = CONST_SEG_F | CONST_SEG_E | CONST_SEG_D | CONST_SEG_C | CONST_SEG_B;
	uint16_t ic = CONST_SEG_B | CONST_SEG_C;
	uint16_t e = CONST_SEG_A | CONST_SEG_G | CONST_SEG_F | CONST_SEG_E | CONST_SEG_D;

	uint16_t p = CONST_SEG_A | CONST_SEG_G | CONST_SEG_F | CONST_SEG_E | CONST_SEG_B;
	uint16_t s = CONST_SEG_A | CONST_SEG_G | CONST_SEG_F | CONST_SEG_C | CONST_SEG_D;
	uint16_t d = CONST_SEG_B | CONST_SEG_C | CONST_SEG_D | CONST_SEG_E | CONST_SEG_G;


	/*uint16_t vec[] = {
			REG_DIGIT_7 | CONST_ONE | CONST_SEG_DP,
			REG_DIGIT_6 | CONST_TWO,
			REG_DIGIT_5 | CONST_THREE | CONST_SEG_DP,
			REG_DIGIT_4 | CONST_FOUR,
			REG_DIGIT_3 | CONST_FIVE | CONST_SEG_DP,
			REG_DIGIT_2 | CONST_SIX ,
			REG_DIGIT_1 | CONST_SEVEN | CONST_SEG_DP,
			REG_DIGIT_0 | CONST_EIGHT,
			REG_DIGIT_7 | CONST_NINE| CONST_SEG_DP,
			REG_DIGIT_6 | CONST_ZERO,
	};*/
	uint16_t vec[] = {
				REG_DIGIT_7 | c1,
				REG_DIGIT_6 | c2,
				REG_DIGIT_5 | u,
				REG_DIGIT_4 | ic,
				REG_DIGIT_3 | e,
				REG_DIGIT_2 | p,
				REG_DIGIT_1 | s,
				REG_DIGIT_0 | d
		};

	displayLedsNoDelay(init_disp, sizeof(init_disp) / sizeof(uint16_t));
	displayLedsNoDelay(rst_disp, sizeof(rst_disp) / sizeof(uint16_t));

	displayLoader(load_disp, sizeof(load_disp) / sizeof(uint16_t));

	while (1) {
		displayLedsNoDelay(rst_disp, sizeof(rst_disp) / sizeof(uint16_t));
		displayLeds(vec, sizeof(vec) / sizeof(uint16_t));

		for (i = 1; i < 16; i++) {
			shiftOut(MSB, REG_INTENSITY | intensity_arr[i]);
			Delay(50);
		}

		for (i = 14; i >= 0; i--) {
			shiftOut(MSB, REG_INTENSITY | intensity_arr[i]);
			Delay(50);
		}
	}
}

int main ()
{
	SysTick_Config( SystemCoreClock / 1000 ); //init timer

	initGPIO_LED();
	joc5();

}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
