#pragma once

#include <vector>
#include <memory>
#include "Bond.hpp"       // Required to know what a 'Bond' is
#include "YieldCurve.hpp" // Required to know what a 'YieldCurve' is

class RiskEngine {
public:
    // Calculates the Price Value of a Basis Point (PV01)
    // Returns the change in price for a +1 basis point parallel shift
    static double calculatePV01(const Bond& bond, const YieldCurve& baseCurve);

    // Runs a scenario analysis on a full portfolio
    // Prints the P&L impact to the console
    static void runStressTest(const std::vector<std::unique_ptr<Bond>>& portfolio, 
                              const YieldCurve& baseCurve, 
                              double shiftBps);
};