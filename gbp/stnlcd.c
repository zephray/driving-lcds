/*
 * Copyright (C) 2019 Wenting Zhang <zephray@outlook.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_gpio.h"
#include "pin_mux.h"

extern const unsigned char gImage_160144test2bpp[23040];
unsigned char framebuffer[160*160];
static unsigned y;

#define STN_CPG_H() GPIO->SET[BOARD_INITPINS_CORE1_STN_CPG_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CPG_PIN
#define STN_CPG_L() GPIO->CLR[BOARD_INITPINS_CORE1_STN_CPG_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CPG_PIN
#define STN_CPG_W(x) GPIO->B[BOARD_INITPINS_CORE1_STN_CPG_PORT][BOARD_INITPINS_CORE1_STN_CPG_PIN] = x
#define STN_CPL_H() GPIO->SET[BOARD_INITPINS_CORE1_STN_CPL_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CPL_PIN
#define STN_CPL_L() GPIO->CLR[BOARD_INITPINS_CORE1_STN_CPL_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CPL_PIN
#define STN_ST_H() GPIO->SET[BOARD_INITPINS_CORE1_STN_ST_PORT] = 1u << BOARD_INITPINS_CORE1_STN_ST_PIN
#define STN_ST_L() GPIO->CLR[BOARD_INITPINS_CORE1_STN_ST_PORT] = 1u << BOARD_INITPINS_CORE1_STN_ST_PIN
#define STN_CP_H() GPIO->SET[BOARD_INITPINS_CORE1_STN_CP_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CP_PIN
#define STN_CP_L() GPIO->CLR[BOARD_INITPINS_CORE1_STN_CP_PORT] = 1u << BOARD_INITPINS_CORE1_STN_CP_PIN
#define STN_FR_T() GPIO->NOT[BOARD_INITPINS_CORE1_STN_FR_PORT] = 1u << BOARD_INITPINS_CORE1_STN_FR_PIN
#define STN_S_W(x)  GPIO->B[BOARD_INITPINS_CORE1_STN_S_PORT][BOARD_INITPINS_CORE1_STN_S_PIN] = x
#define STN_D0_W(x) GPIO->B[BOARD_INITPINS_CORE1_STN_D0_PORT][BOARD_INITPINS_CORE1_STN_D0_PIN] = x
#define STN_D1_W(x) GPIO->B[BOARD_INITPINS_CORE1_STN_D1_PORT][BOARD_INITPINS_CORE1_STN_D1_PIN] = x

void STN_Wait(unsigned long t)
{
	int i;
	while(t--) {
	    for (i = 0; i < 5; i++)
	        asm("nop");
	}
}

void STN_Line() {
    unsigned x;

    if (y == 0) STN_FR_T();

    // Line start sequence
    STN_CPG_H();
    STN_CPL_H();
    STN_Wait(2);
    STN_CPL_L();

    STN_Wait(1);
    STN_S_W((y == 0) ? 1 : 0); // Frame Sync
    STN_Wait(1);
    STN_CPG_L();
    STN_Wait(2);
    STN_CPG_H();
    STN_Wait(2);
    STN_CPG_L();

    // Before Line start
    STN_Wait(16);
    STN_ST_H();
    STN_Wait(2);
    STN_CP_H();
    STN_Wait(1);
    STN_CP_L();
    STN_Wait(2);
    STN_ST_L();
    STN_Wait(1);

    int ptr = y * 160;

    // Shift out pixels
    for (x = 0; x < 160; x++) {
        STN_CP_H();
        STN_CPG_W(((x >= 84) && (x < 88)) ? 1 : 0);
        STN_D0_W((framebuffer[ptr] >> 6) & 1);
        STN_D1_W((framebuffer[ptr] >> 7) & 1);
        ptr ++;
        STN_CP_L();
    }

    // Nothing to do now
    STN_Wait(0);
    STN_CPG_H();
    STN_Wait(2);
    STN_CPG_L();

    // Increment line counter
    y = (y < 159) ? (y+1) : 0;
}

void STN_Init() {
	// Actually, there is nothing much to initialize
	memcpy(framebuffer, gImage_160144test2bpp, 23040);
    /*for (int i = 0; i < 23040; i++) {
        framebuffer[i] = (i & 1) ? 0xff : 0x00;
    }*/
	y = 0;
}
