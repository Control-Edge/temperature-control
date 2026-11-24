#pragma once
#include <stdint.h>

typedef struct TempControl_s
{
    /* Configuration */
    float proportionalGain; /* %/°C */
    float integralTime_s;   /* 0: no integral action, a P controller */
    uint8_t antiWindup;     /* 1: don't integrate while the heater is saturated */

    /* Inputs */
    float setPoint;
    float temperature;

    /* Output */
    float heaterPower; /* 0..100 % */

    /* State */
    float integral; /* the integral term, in % */
} TempControl_ts;

void tempControlInit(TempControl_ts *const ctx, float proportionalGain, float integralTime_s, uint8_t antiWindup);
void tempControlUpdate(TempControl_ts *const ctx, float dt_s);
