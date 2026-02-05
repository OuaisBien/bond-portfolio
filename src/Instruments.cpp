#include "Instruments.hpp"
#include <vector>


// Vanilla Bond
// =========================================================

VanillaBond::VanillaBond(std::string id, double n, double m, double c, int f)
    : Bond(id, n, m), couponRate(c), frequency(f) {}

std::vector<CashFlow> VanillaBond::getCashFlows(const YieldCurve & /*curve*/) const
{
    // Note: The curve argument is unused here because coupons are fixed,
    // but the interface requires it.

    std::vector<CashFlow> flows;
    double dt = 1.0 / frequency; // e.g., 0.5 for semi-annual
    double couponAmount = notional * couponRate * dt;

    // Generate periodic coupon payments
    for (double t = dt; t <= maturity + 0.001; t += dt)
    {
        flows.push_back({couponAmount, t});
    }

    // Add the Principal repayment (Notional) at the end
    if (!flows.empty())
    {
        flows.back().amount += notional;
    }
    else
    {
        // Edge case: if maturity is very short
        flows.push_back({notional + couponAmount, maturity});
    }

    return flows;
}

std::string VanillaBond::getDescription() const
{
    return "Vanilla Bond " + std::to_string(couponRate * 100) + "%";
}


// Zero Coupon Bond
// =========================================================

ZeroCouponBond::ZeroCouponBond(std::string id, double n, double m) : Bond(id, n, m) {}

std::vector<CashFlow> ZeroCouponBond::getCashFlows(const YieldCurve & /*curve*/) const
{
    // Only one cash flow: Notional at Maturity
    return {{notional, maturity}};
}

std::string ZeroCouponBond::getDescription() const
{
    return "Zero Coupon";
}


// Floating Rate Note
// =========================================================

FloatingRateNote::FloatingRateNote(std::string id, double n, double m, double s, int f)
    : Bond(id, n, m), spread(s), frequency(f) {}

std::vector<CashFlow> FloatingRateNote::getCashFlows(const YieldCurve &curve) const
{
    std::vector<CashFlow> flows;
    double dt = 1.0 / frequency;

    for (double t = dt; t <= maturity + 0.001; t += dt)
    {
        // 1. Estimate the Forward Rate for this period
        // (Simplification: using the spot rate at time t from the curve)
        double forwardRate = curve.getRate(t);

        // 2. Calculate the variable coupon
        double couponAmount = notional * (forwardRate + spread) * dt;

        flows.push_back({couponAmount, t});
    }

    // Add Principal repayment
    if (!flows.empty())
    {
        flows.back().amount += notional;
    }

    return flows;
}

std::string FloatingRateNote::getDescription() const
{
    return "Floating Rate Note";
}
