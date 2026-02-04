#include "Instruments.hpp"
#include <vector>

// =========================================================
// Vanilla Bond Implementation
// =========================================================

VanillaBond::VanillaBond(double n, double m, double c, int f)
    : Bond(n, m), couponRate(c), frequency(f) {}

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

std::string VanillaBond::getName() const
{
    return "Vanilla Bond";
}

// =========================================================
// Zero Coupon Bond Implementation
// =========================================================

ZeroCouponBond::ZeroCouponBond(double n, double m) : Bond(n, m) {}

std::vector<CashFlow> ZeroCouponBond::getCashFlows(const YieldCurve & /*curve*/) const
{
    // Only one cash flow: Notional at Maturity
    return {{notional, maturity}};
}

std::string ZeroCouponBond::getName() const
{
    return "Zero Coupon";
}

// =========================================================
// Floating Rate Note (FRN) Implementation
// =========================================================

FloatingRateNote::FloatingRateNote(double n, double m, double s, int f)
    : Bond(n, m), spread(s), frequency(f) {}

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

std::string FloatingRateNote::getName() const
{
    return "Floating Rate Note";
}