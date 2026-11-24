#include "tempControl.h"

void tempControlInit(TempControl_ts *const ctx, float proportionalGain, float integralTime_s, uint8_t antiWindup)
{
    ctx->proportionalGain = proportionalGain;
    ctx->integralTime_s = integralTime_s;
    ctx->antiWindup = antiWindup;
    ctx->setPoint = 0.0f;
    ctx->temperature = 0.0f;
    ctx->heaterPower = 0.0f;
    ctx->integral = 0.0f;
}

void tempControlUpdate(TempControl_ts *const ctx, float dt_s)
{
    float error = ctx->setPoint - ctx->temperature;
    float proportional = ctx->proportionalGain * error;
    float output = proportional + ctx->integral;

    /* Integral action: keeps adding power for as long as there is an error */
    if (ctx->integralTime_s > 0.0f)
    {
        /* Anti-windup: while the heater is saturated, integrating further only stores up an overshoot */
        uint8_t saturated = (output >= 100.0f && error > 0.0f) || (output <= 0.0f && error < 0.0f);

        if (!(ctx->antiWindup && saturated))
            ctx->integral += ctx->proportionalGain * error * dt_s / ctx->integralTime_s;
        output = proportional + ctx->integral;
    }

    /* The heater can only deliver 0..100 % */
    if (output > 100.0f)
        output = 100.0f;
    else if (output < 0.0f)
        output = 0.0f;
    ctx->heaterPower = output;
}
