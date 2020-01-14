#ifndef _H_NAND_
#define _H_NAND_
#include "operator.hpp"

class NAND_Operator : public Operator
{
public:
    NAND_Operator(): Operator() {};
    ~NAND_Operator(){};

    float process(float A, float B) override;
};


inline float NAND_Operator::process(float A, float B)
{
    return (A == low || B == low) ? high : low;
}


#endif
