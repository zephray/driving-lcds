/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_mailbox.h"
#include "fsl_ctimer.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "stnlcd.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PRIMARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1

#define START_EVENT 1234

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);

ctimer_callback_t ctimer_callback_table[] = {
     NULL, ctimer_match0_callback, NULL, NULL, NULL, NULL, NULL, NULL};

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_msg;
static ctimer_match_config_t matchConfig0;

/*******************************************************************************
 * Code
 ******************************************************************************/
void MAILBOX_IRQHandler()
{
    g_msg = MAILBOX_GetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID);
    MAILBOX_ClearValueBits(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID, 0xffffffff);
    g_msg++;
    MAILBOX_SetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, g_msg);
}

void ctimer_match0_callback(uint32_t flags)
{
    STN_Line();
}

/*!
 * @brief Main function
 */
int main(void)
{
    ctimer_config_t config;

    /* Init board hardware.*/
    BOARD_InitPins_Core1();

    /* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

    /* Let the other side know the application is up and running */
    MAILBOX_SetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, (uint32_t)START_EVENT);

    /* Initialize screen driver */
    STN_Init();

    /* Use 12 MHz clock for some of the Ctimers */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER2);

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER2, &config);

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CLOCK_GetFreq(kCLOCK_CTmier2) / 13600; // Hsync = 13.6kHz
    matchConfig0.outControl         = kCTIMER_Output_NoAction;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    CTIMER_RegisterCallBack(CTIMER2, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(CTIMER2, kCTIMER_Match_1, &matchConfig0);
    CTIMER_StartTimer(CTIMER2);

    while (1)
    {
        __WFI();
    }
}
