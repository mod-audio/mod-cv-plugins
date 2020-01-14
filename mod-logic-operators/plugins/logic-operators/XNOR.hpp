#ifndef _H_XNOR_
#define _H_XNOR_
#include "operator.hpp"

class XNOR_Operator : public Operator
{
public:
    XNOR_Operator(): Operator() {};
    ~XNOR_Operator(){};

    float process(float A, float B) override;
};


inline float XNOR_Operator::process(float A, float B)
{
    return ((A == low && B == low) || (A == high && B == high)) ? high : low;
}


#endif
