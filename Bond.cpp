#include "Bond.hpp"
#include <iostream> 

// Constructor implementation
Bond::Bond(double n, double m) : notional(n), maturity(m) {}

// The Pricing Engine Logic
double Bond::calculatePrice(const YieldCurve& curve) const {
    double price = 0.0;
    
    auto flows = getCashFlows(curve);

    for (const auto& cf : flows) {
        // PV = CashFlow * e^(-r*t)
        price += cf.amount * curve.getDiscountFactor(cf.time);
    }

    return price;
}