/*
 * =====================================================================
 *  shared_state.h
 *  NOTE: This is a reference stub of what Member 4 (UART/Integration)
 *  needs to define and share with the team BEFORE Task 1/2/3 are
 *  finalized. Coordinate the exact field names/order with them so
 *  everyone compiles against the same struct.
 * =====================================================================
 */

#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Identifies which subsystem a UART message came from */
typedef enum {
    SOURCE_LIGHTING,
    SOURCE_OVEN,
    SOURCE_OVERRIDE,
    SOURCE_SYSTEM
} MessageSource_t;

/* Message format pushed onto the central UART queue by every task */
typedef struct {
    MessageSource_t source;
    uint32_t        numericValue;   /* raw sensor/ADC reading, if applicable */
    bool            actuatorState;  /* ON/OFF state being reported */
    bool            isOverridden;   /* true if manual override is active */
    bool            isFault;        /* true if this message is a fault alert */
} UARTMessage_t;

/* Shared system state, written by Override Task, read by Lighting/Oven tasks.
 * Access MUST be protected by xSharedStateMutex. */
typedef struct {
    bool lightingOverrideActive;
    bool lightingOverrideValue;   /* true = force ON, false = force OFF */
    bool ovenOverrideActive;
    bool ovenOverrideValue;
} SystemState_t;

/* Globals - defined once in shared_state.c */
extern SystemState_t   g_SystemState;
extern QueueHandle_t    xUARTQueue;
extern SemaphoreHandle_t xSharedStateMutex;

#endif /* SHARED_STATE_H */