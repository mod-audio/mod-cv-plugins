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


inline float INV_Operator::process(float A, float)
{
    return (A == high) ? false_value : true_value;
}


#endif //_H_INV_
