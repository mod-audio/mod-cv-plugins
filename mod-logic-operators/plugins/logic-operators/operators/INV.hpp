#ifndef _H_INV_
#define _H_INV_
#include "operator.hpp"

class INV_Operator : public Operator
{
public:
    INV_Operator(): Operator() {};
    ~INV_Operator(){};

    float process(float A, float B) override;
};


inline float INV_Operator::process(float A, float B)
{
    return (A == high) ? low : high;
}


#endif //_H_INV_
