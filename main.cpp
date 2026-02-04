#include <iostream>
#include <vector>
#include <memory> // std::unique_ptr, std::make_unique

// Headers
#include "YieldCurve.hpp"
#include "Instruments.hpp" // VanillaBond, ZeroCouponBond, FloatingRateNote
#include "RiskEngine.hpp"

int main()
{
    try
    {
        // 1. Initialize Market Data
        std::cout << "Initialising Market Data..." << std::endl;

        YieldCurve curve;
        curve.addRate(1.0, 0.03);  // 1Y
        curve.addRate(5.0, 0.04);  // 5Y
        curve.addRate(10.0, 0.05); // 10Y


        // 2. Portfolio
        std::vector<std::unique_ptr<Bond>> portfolio;

        // Vanilla Bond (1000, 5Y, 4%, Annual)
        portfolio.push_back(std::make_unique<VanillaBond>(1000.0, 5.0, 0.04, 1));

        // ZC (1000 Notional, 10Y Maturity)
        portfolio.push_back(std::make_unique<ZeroCouponBond>(1000.0, 10.0));

        // Floating Rate Note (1000 Notional, 3Y Maturity, Spread 1%, Semi-Annual)
        portfolio.push_back(std::make_unique<FloatingRateNote>(1000.0, 3.0, 0.01, 2));

        std::cout << "Portfolio constructed with " << portfolio.size() << " instruments.\n"
                  << std::endl;

        // 3. Pricing & Analytics
        std::cout << "--- PRICING REPORT ---" << std::endl;

        for (const auto &bond : portfolio)
        {
            double price = bond->calculatePrice(curve);

            // Calculate Risk (PV01) using the RiskEngine static method
            double pv01 = RiskEngine::calculatePV01(*bond, curve);

            std::cout << "Instrument: " << bond->getName() << "\n"
                      << "  Price: " << price << "\n"
                      << "  PV01:  " << pv01 << "\n"
                      << "----------------------" << std::endl;
        }

        // 4. Portfolio Risk Management
        // Run a +50bps Stress Test on the entire portfolio
        RiskEngine::runStressTest(portfolio, curve, 50.0);
    }
    catch (const std::exception &e)
    {
        // Robustness: Catch any standard errors (like memory issues or logic errors)
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}