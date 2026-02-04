#pragma once
#include "YieldCurve.hpp"
#include <vector>
#include <string>

struct CashFlow {
    double amount;
    double time;
};

class Bond {
protected:
    double notional;
    double maturity;

public:
    Bond(double n, double m);
    virtual ~Bond() = default;

    virtual std::vector<CashFlow> getCashFlows(const YieldCurve& curve) const = 0;
    
    // Defined in .cpp or inline here if it's very short
    double calculatePrice(const YieldCurve& curve) const; 
    virtual std::string getName() const = 0;
};
