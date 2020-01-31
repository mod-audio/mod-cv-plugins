#include "operator.hpp"

Operator::Operator()
{
}

Operator::~Operator()
{
}

void Operator::setHigh(float high)
{
    this->high = high;
}

void Operator::setLow(float low)
{
    this->low = low;
}

float Operator::getHigh()
{
    return high;
}

float Operator::getLow()
{
    return low;
}
