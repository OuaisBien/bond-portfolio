#include "YieldCurve.hpp"
#include <cmath>
#include <iterator>

void YieldCurve::addRate(double time, double rate) {
    rates[time] = rate;
}

double YieldCurve::getRate(double t) const {
    if (rates.empty())
        return 0.0;

    // Lower bound renvoie le premier élément >= t
    auto it = rates.lower_bound(t);

    if (it == rates.begin())
        return it->second; // t <= premier point
    if (it == rates.end())
        return rates.rbegin()->second; // t >= dernier point

    // Interpolation
    double t2 = it->first;
    double r2 = it->second;
    auto prev = std::prev(it);
    double t1 = prev->first;
    double r1 = prev->second;

    return r1 + (r2 - r1) * ((t - t1) / (t2 - t1));
}

double YieldCurve::getDiscountFactor(double t) const {
    double r = getRate(t);
    return std::exp(-r * t);
}

void YieldCurve::parallelShift(double basisPoints) {
    double shift = basisPoints / 10000.0;
    for (auto& pair : rates) {
        pair.second += shift;
    }
}