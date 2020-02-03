#ifndef DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED
#define DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

#include "logic-operators.hpp"
#include "operators/operator.hpp"
#include "operators/AND.hpp"
#include "operators/INV.hpp"
#include "operators/NAND.hpp"
#include "operators/NOR.hpp"
#include "operators/OR.hpp"
#include "operators/XNOR.hpp"
#include "operators/XOR.hpp"

START_NAMESPACE_DISTRHO

#define NUM_OPERATORS 7

class LogicOperators : public Plugin
{
public:
    enum Parameters
    {
        paramSelectOperator = 0,
        paramSetSwitchPoint,
        paramSetHysteresis,
        paramCount
    };

    LogicOperators();
    ~LogicOperators();

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "LogicOperators";
    }

    const char* getDescription() const override
    {
        return "Logic operators";
    }

    const char* getMaker() const noexcept override
    {
        return "MOD";
    }

    const char* getHomePage() const override
    {
        return "http://";
    }

    const char* getLicense() const noexcept override
    {
        return "Custom";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('M', 'D', 'L', 'O');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;
    void  loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // Process
    void activate() override;
    void deactivate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:

    void reset();

    float sampleRate;
    int selectOperator;
    float paramSwitchPoint;
    float paramHysteresis;
    float logic1;
    float logic2;
    float logicOut;

    Operator **logicOperators;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogicOperators)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED



