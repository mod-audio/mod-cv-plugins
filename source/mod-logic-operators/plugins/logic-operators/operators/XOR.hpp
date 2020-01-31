#ifndef _H_XOR_
#define _H_XOR_
#include "operator.hpp"

class XOR_Operator : public Operator
{
public:
    XOR_Operator(): Operator() {};
    ~XOR_Operator(){};

    float process(float A, float B) override;
};


inline float XOR_Operator::process(float A, float B)
{
    return ((A == high || B == high) && !(A == high && B == high)) ? true_value : false_value;
}


#endif
