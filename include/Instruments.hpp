#pragma once
#include "Bond.hpp"

class VanillaBond : public Bond {
private:
    double couponRate;
    int frequency;
public:
    VanillaBond(double n, double m, double c, int f);
    std::vector<CashFlow> getCashFlows(const YieldCurve& curve) const override;
    std::string getName() const override;
};

class ZeroCouponBond : public Bond {
public:
    ZeroCouponBond(double n, double m);
    std::vector<CashFlow> getCashFlows(const YieldCurve& curve) const override;
    std::string getName() const override;
    
};

class FloatingRateNote : public Bond {
private:
    double spread;
    int frequency;

public:
    FloatingRateNote(double n, double m, double s, int f);
    std::vector<CashFlow> getCashFlows(const YieldCurve &curve) const override;
    std::string getName() const override;
};