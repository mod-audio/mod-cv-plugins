#ifndef _H_AND_
#define _H_AND_
#include "operator.hpp"

class AND_Operator : public Operator
{
public:
    AND_Operator(): Operator() {};
    ~AND_Operator(){};

    float process(float A, float B) override;
};


inline float AND_Operator::process(float A, float B)
{
    return (A == high && B == high) ? true_value : false_value;
}


#endif
