#include "logic-operators.hpp"

START_NAMESPACE_DISTRHO


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
    paramHigh = 5.0;
    paramLow = 0.0;
    paramEqualOrHigher = 1.0;

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
    case paramSetLow:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Set Low";
        parameter.symbol     = "SetLow";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -10.f;
        parameter.ranges.max = 10.f;
        break;
    case paramSetHigh:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Set High";
        parameter.symbol     = "SetHigh";
        parameter.unit       = "";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -10.f;
        parameter.ranges.max = 10.f;
        break;
    case paramSetEqualOrHigher:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Equal and Higher";
        parameter.symbol     = "EqualAndHigher";
        parameter.unit       = "";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.f;
        parameter.ranges.max = 1.f;
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
    case paramSetLow:
        return paramLow;
    case paramSetHigh:
        return paramHigh;
    case paramSetEqualOrHigher:
        return paramEqualOrHigher;
    }
}

void LogicOperators::setParameterValue(uint32_t index, float value)
{
    switch (index)
    {
    case paramSelectOperator:
        selectOperator = value;
        break;
    case paramSetLow:
        paramLow = value;
        break;
    case paramSetHigh:
        paramHigh = value;
        break;
    case paramSetEqualOrHigher:
        paramEqualOrHigher = value;
        break;
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

    // Main processing body
    for (unsigned l = 0; l < NUM_OPERATORS; l++) {
        logicOperators[l]->setLow(paramLow);
        logicOperators[l]->setHigh(paramHigh);
    }
    for (uint32_t f = 0; f < frames; ++f)
    {
        float a = input1[f];
        float b = input2[f];

        a = (a > paramHigh && (bool)paramEqualOrHigher) ? paramHigh : a;
        a = (a < paramLow  && (bool)paramEqualOrHigher) ? paramLow  : a;
        b = (b > paramHigh && (bool)paramEqualOrHigher) ? paramHigh : b;
        b = (b < paramLow  && (bool)paramEqualOrHigher) ? paramLow  : b;

        output[f] = logicOperators[(int)selectOperator]->process(a, b);
    }
}



// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new LogicOperators();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
