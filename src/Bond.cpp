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
    std::string ticker; // Unique identifier
    double notional;
    double maturity;

public:
    Bond(std::string id, double n, double m);
    virtual ~Bond() = default;

    virtual std::vector<CashFlow> getCashFlows(const YieldCurve& curve) const = 0;
    
    // Defined in .cpp or inline here if it's very short
    double calculatePrice(const YieldCurve& curve) const;

    std::string getTicker() const { return ticker; }

    virtual std::string getDescription() const = 0;
};
