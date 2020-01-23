#ifndef _H_NOR_
#define _H_NOR_
#include "operator.hpp"

class NOR_Operator : public Operator
{
public:
    NOR_Operator(): Operator() {};
    ~NOR_Operator(){};

    float process(float A, float B) override;
};


inline float NOR_Operator::process(float A, float B)
{
    return (A == low && B == low) ? true_value : false_value;
}


#endif
