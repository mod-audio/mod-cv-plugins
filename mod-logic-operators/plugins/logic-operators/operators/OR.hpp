#ifndef _H_OR_
#define _H_OR_
#include "operator.hpp"

class OR_Operator : public Operator
{
public:
    OR_Operator(): Operator() {};
    ~OR_Operator(){};

    float process(float A, float B) override;
};


inline float OR_Operator::process(float A, float B)
{
    return (A == high || B == high) ? high : low;
}


#endif
