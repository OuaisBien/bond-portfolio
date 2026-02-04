#pragma once
#include <map>
#include <vector>

class YieldCurve {
private:
    std::map<double, double> rates; 

public:
    void addRate(double time, double rate);
    double getRate(double t) const;
    double getDiscountFactor(double t) const;
    void parallelShift(double basisPoints);
};