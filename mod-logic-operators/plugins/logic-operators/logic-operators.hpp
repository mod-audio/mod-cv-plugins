#ifndef DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED
#define DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

#include "AND.hpp"
#include "INV.hpp"
#include "logic-operators.hpp"
#include "NAND.hpp"
#include "NOR.hpp"
#include "operator.hpp"
#include "OR.hpp"
#include "XNOR.hpp"
#include "XOR.hpp"

START_NAMESPACE_DISTRHO

class LogicOperators : public Plugin
{
public:
    enum Parameters
    {
        paramSelectOperator = 0,
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

    Operator **logicOperators;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogicOperators)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_SLPLUGIN_HPP_INCLUDED



