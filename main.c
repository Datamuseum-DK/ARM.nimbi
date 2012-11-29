/**************************************************************************/
/*! 
    @file     main.c
    @author   K. Townsend (microBuilder.eu)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2011, microBuilder SARL
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "projectconfig.h"
#include "sysinit.h"

#include "core/uart/uart.h"
#include "core/gpio/gpio.h"
#include "core/systick/systick.h"

#ifdef CFG_INTERFACE
  #include "core/cmd/cmd.h"
#endif

#include "core/usbcdc/cdcuser.h"


#define NLAMPS	14

//				A  B  C  D   E  F  G  H   I  J  K  L  M  N
//				------------------------------------------
static const int lp[NLAMPS] = { 0, 0, 0, 0, 0,  0, 1, 1, 1, 1,  1, 2, 2, 2 };
static const int lb[NLAMPS] = { 2, 8, 5, 7, 9, 11, 1, 5, 7, 9, 11, 1, 3, 5 };

#define NSWITCH 14

//				 A  B  C  D  E  F  G  H   I  J  K  L  M   N
//				 ------------------------------------------
static const int sp[NSWITCH] = { 1, 1, 1, 1, 1,  1, 2, 2, 2, 2, 2, 2, 2,  2 };
static const int sb[NSWITCH] = { 0, 2, 4, 6, 8, 10, 0, 2, 4, 6, 7, 8, 9, 10 };

static void
init_bits(void)
{
	int i;


	IOCON_JTAG_TDI_PIO0_11 = 0
	    | IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO
	    | IOCON_JTAG_TDI_PIO0_11_ADMODE_DIGITAL;

	IOCON_JTAG_TDO_PIO1_1 = 0
	    | IOCON_JTAG_TDO_PIO1_1_FUNC_GPIO
	    | IOCON_JTAG_TDO_PIO1_1_ADMODE_DIGITAL;

	for (i = 0; i < NLAMPS; i++)
		gpioSetDir(lp[i], lb[i], gpioDirection_Output);

	for (i = 0; i < NSWITCH; i++)
		gpioSetDir(sp[i], sb[i], gpioDirection_Input);

}
	


int
main(void)
{
	int i, j, s;
	int last[NSWITCH];
	int last_when[NSWITCH];
	int now;
	int currentSecond, lastSecond;

	systemInit();

	init_bits(); 

	for (i = 0; i < NLAMPS; i++)
		gpioSetValue(lp[i], lb[i], 0);
	currentSecond = systickGetSecondsActive();
	do 
		i = systickGetSecondsActive();
	while (i == currentSecond);
	currentSecond = i;
	for (i = 0; i < NLAMPS; i++)
		gpioSetValue(lp[i], lb[i], 1);
	do 
		i = systickGetSecondsActive();
	while (i == currentSecond);
	currentSecond = i;
	for (i = 0; i < NLAMPS; i++)
		gpioSetValue(lp[i], lb[i], 0);

	s = 0;
	while (1) {
		// Toggle LED once per second
		currentSecond = systickGetSecondsActive();
		if (currentSecond != lastSecond) {
			lastSecond = currentSecond;
			gpioSetValue(CFG_LED_PORT, CFG_LED_PIN, !(gpioGetValue(CFG_LED_PORT, CFG_LED_PIN)));
			CDC_putchar('*');
			CDC_BulkIn();
		}

		i = CDC_getchar();
		if (i >= 'A' && i < 'A' + NLAMPS) {
			i -= 'A';
			gpioSetValue(lp[i], lb[i], 1);
		} else if (i >= 'a' && i < 'a' + NLAMPS) {
			i -= 'a';
			gpioSetValue(lp[i], lb[i], 0);
		} else if (i != -1) {
			printf("<%d>", i);
		}

		i = s++;
		if (s == NSWITCH)
			s = 0;
		
		now = systickGetTicks() & ~15;
		j = gpioGetValue(sp[i], sb[i]);
		if (last_when[i] != now) {
			if (j != last[i]) {
				CDC_putchar((j ? 'a' : 'A') + i);
			}
			last[i] = j;
			last_when[i] = now;
		}
	}

	return 0;
}
