#include "RiskEngine.hpp"
#include <iostream>
#include <iomanip>

double RiskEngine::calculatePV01(const Bond& bond, const YieldCurve& baseCurve) {
    // 1. Calculate price with the base curve
    double priceBase = bond.calculatePrice(baseCurve);

    // 2. Create a copy of the curve and apply a +1 bp shift (0.01%)
    YieldCurve shockedCurve = baseCurve;
    shockedCurve.parallelShift(1.0); 

    // 3. Calculate price with the shocked curve
    double priceShock = bond.calculatePrice(shockedCurve);

    // 4. Return the difference
    return priceShock - priceBase;
}

void RiskEngine::runStressTest(const std::vector<std::unique_ptr<Bond>>& portfolio, 
                               const YieldCurve& baseCurve, 
                               double shiftBps) {
    
    std::cout << " STRESS TEST REPORT (Shift: " << shiftBps << " bps)" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    // Create the stressed market environment
    YieldCurve stressedCurve = baseCurve;
    stressedCurve.parallelShift(shiftBps);

    double totalBaseVal = 0.0;
    double totalStressedVal = 0.0;

    // Formatting for clean table output
    std::cout << std::left << std::setw(20) << "Instrument" 
              << std::right << std::setw(12) << "Base Price" 
              << std::setw(12) << "New Price" 
              << std::setw(12) << "P&L" << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    for (const auto& bond : portfolio) {
        double pBase = bond->calculatePrice(baseCurve);
        double pStress = bond->calculatePrice(stressedCurve);
        double diff = pStress - pBase;
        
        totalBaseVal += pBase;
        totalStressedVal += pStress;

        std::cout << std::left << std::setw(20) << bond->getName() 
                  << std::right << std::fixed << std::setprecision(2) 
                  << std::setw(12) << pBase 
                  << std::setw(12) << pStress 
                  << std::setw(12) << diff << std::endl;
    }

    std::cout << std::string(56, '-') << std::endl;
    double totalPnL = totalStressedVal - totalBaseVal;
    
    std::cout << "TOTAL PORTFOLIO P&L IMPACT: " << totalPnL << std::endl;
    std::cout << "==============================================\n" << std::endl;
}