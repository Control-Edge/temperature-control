#include <cppmodel/CModel.h>
#include "tempControl.h"

static TempControl_ts controller;
static float roomTemperature, ambientTemperature, heatingRate, lossRate;
static CppModelRule_t morningWarmUp;

static void Init(CModelSimulation_ts *self, unsigned long stepTime_ms)
{
    (void)stepTime_ms;

    /* Controller tuning */
    tempControlInit(&controller,
                    CppModel_getParameterF32(self, "ProportionalGain [%/C]", 30.0f),
                    CppModel_getParameterF32(self, "IntegralTime [s]", 0.0f),
                    CppModel_getParameterU8(self, "AntiWindup", 0));

    /* Plant: the room */
    ambientTemperature = CppModel_getParameterF32(self, "AmbientTemperature [C]", 5.0f);
    heatingRate = CppModel_getParameterF32(self, "HeatingRate [C/s]", 0.02f);
    lossRate = CppModel_getParameterF32(self, "LossRate [1/s]", 0.001f);
    roomTemperature = CppModel_getParameterF32(self, "InitialTemperature [C]", 16.0f);

    /* Requirement: after a set point change, the room is within 0.5 °C of it within 30 min,
     * stays there for 10 min, and never overshoots it by more than 1 °C. */
    morningWarmUp = CppModel_ruleSettle(self, "settles at set point", 0.5, 30 * 60, 10 * 60, 1.0);
    CppModel_ruleSignal(morningWarmUp, "RoomTemperature [C]");
}

void RunCyclic(CModelSimulation_ts *self, unsigned long currentTime_ms)
{
    float dt_s = self->stepTime_ms / 1000.0f;

    /* Night setback at 16 °C, then the day set point of 21 °C from 06:00 (30 min into the run) */
    controller.setPoint = CppModel_getInputF32(self, "Setpoint [C]", currentTime_ms < 30 * 60 * 1000 ? 16.0f : 21.0f);
    controller.temperature = roomTemperature;

    /* Controller under test */
    tempControlUpdate(&controller, dt_s);

    /* Plant model: the heater adds heat, the walls leak it to the outside */
    roomTemperature += dt_s * (heatingRate * controller.heaterPower / 100.0f -
                               lossRate * (roomTemperature - ambientTemperature));

    CppModel_setOutputF32(self, "RoomTemperature [C]", roomTemperature);
    CppModel_setOutputF32(self, "HeaterPower [%]", controller.heaterPower);
    CppModel_setOutputF32(self, "IntegralTerm [%]", controller.integral);

    CppModel_updateSettle(morningWarmUp, roomTemperature, controller.setPoint);
}

int main(void)
{
    CModelSimulation_ts *simulation = CppModel_create("TempControl", 150 * 60 * 1000, 1000);
    if (simulation == NULL)
        return 1;

    CppModel_setInitFunction(simulation, Init);
    CppModel_setRunStepFunction(simulation, RunCyclic);
    CppModel_Simulate(simulation);

    int result = CppModel_getSimulationResult(simulation);
    CppModel_destroy(simulation);

    return result == 0 ? 0 : 1;
}
