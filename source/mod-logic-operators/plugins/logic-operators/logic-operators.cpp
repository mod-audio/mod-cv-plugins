#include "logic-operators.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

#include <iostream>

// -----------------------------------------------------------------------

LogicOperators::LogicOperators()
    : Plugin(paramCount, 1, 0) // 1 program, 0 states
{
    loadProgram(0);

    sampleRate = (float)getSampleRate();

    logicOperators = new Operator*[NUM_OPERATORS];

    logicOperators[0] = new AND_Operator();
    logicOperators[1] = new NAND_Operator();
    logicOperators[2] = new INV_Operator();
    logicOperators[3] = new OR_Operator();
    logicOperators[4] = new NOR_Operator();
    logicOperators[5] = new XOR_Operator();
    logicOperators[6] = new XNOR_Operator();

    selectOperator = 0.0;
    paramSwitchPoint = 5.0;
    paramHysteresis = 0.0;
    logic1 = 0.0;
    logic2 = 0.0;
    logicOut = 0.0;

    reset();
}

LogicOperators::~LogicOperators()
{
}

// -----------------------------------------------------------------------
// Init

void LogicOperators::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index)
    {
    case paramSelectOperator:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Logic Operator";
        parameter.symbol     = "LogicOperator";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 6.f;
        break;
    case paramSetSwitchPoint:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Logic High";
        parameter.symbol     = "LogicHigh";
        parameter.unit       = "";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.f;
        parameter.ranges.max = 1.f;
        break;
    case paramSetHysteresis:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Hysteresis";
        parameter.symbol     = "Hysteresis";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.f;
        parameter.ranges.max = 5.f;
        break;
    }
}

void LogicOperators::initProgramName(uint32_t index, String& programName)
{
    if (index != 0)
        return;

    programName = "Default";
}

// -----------------------------------------------------------------------
// Internal data

float LogicOperators::getParameterValue(uint32_t index) const
{
    switch (index)
    {
    case paramSelectOperator:
        return selectOperator;
    case paramSetSwitchPoint:
        return paramSwitchPoint;
    case paramSetHysteresis:
        return paramHysteresis;
    }
}

void LogicOperators::setParameterValue(uint32_t index, float value)
{
    switch (index)
    {
    case paramSelectOperator:
        selectOperator = value;
        break;
    case paramSetSwitchPoint:
        paramSwitchPoint = value;
        break;
    case paramSetHysteresis:
        paramHysteresis = value;
    }
}

void LogicOperators::loadProgram(uint32_t index)
{
}

void LogicOperators::reset()
{
}

// -----------------------------------------------------------------------
// Process

void LogicOperators::activate()
{
}

void LogicOperators::deactivate()
{
}


void LogicOperators::run(const float** inputs, float** outputs, uint32_t frames)
{
    const float* input1 = inputs[0];
    const float* input2 = inputs[1];
    float* output = outputs[0];

    for (uint32_t f = 0; f < frames; ++f)
    {
        float a = input1[f];
        float b = input2[f];

        logic1 = (a >= paramSwitchPoint + (paramHysteresis / 2)) ? 5.0 : logic1;
        logic2 = (b >= paramSwitchPoint + (paramHysteresis / 2)) ? 5.0 : logic2;
        if (paramHysteresis == 0.0) {
            logic1 = (a < paramSwitchPoint - (paramHysteresis / 2)) ? 0.0 : logic1;
            logic2 = (b < paramSwitchPoint - (paramHysteresis / 2)) ? 0.0 : logic2;
        } else {
            logic1 = (a <= paramSwitchPoint - (paramHysteresis / 2)) ? 0.0 : logic1;
            logic2 = (b <= paramSwitchPoint - (paramHysteresis / 2)) ? 0.0 : logic2;
        }

        logicOut = logicOperators[(int)selectOperator]->process(logic1, logic2);

        output[f] = logicOut;
    }
}



// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new LogicOperators();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
