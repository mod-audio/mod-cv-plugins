#ifndef _H_OPERATOR_
#define _H_OPERATOR_

class Operator {
public:
    Operator();
    ~Operator();
    void setHigh(float high);
    void setLow(float low);
    float getHigh();
    float getLow();
    virtual float process(float A, float B) = 0;
protected:
    float high = 5.0;
    float low  = 0.0;
private:
};


#endif //_H_OPERATOR_
