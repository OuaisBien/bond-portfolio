#include "Instruments.hpp"
#include <random>
#include <memory>
#include <vector>
#include <string>

class PortfolioGenerator {
private:
    std::mt19937 rng; // Mersenne Twister Random Engine

public:
    PortfolioGenerator() : rng(std::random_device{}()) {}

    std::shared_ptr<Bond> generateRandomBond(int id) {
        // Randomly decide bond characteristics
        std::uniform_int_distribution<int> typeDist(0, 2); // 0=Vanilla, 1=FRN, 2=Zero
        std::uniform_int_distribution<int> matDist(2, 30); // 2 to 30 years
        std::uniform_real_distribution<double> cpnDist(0.01, 0.06); // 1% to 6% coupon

        int maturity = matDist(rng);
        double coupon = cpnDist(rng);
        std::string ticker = "BOND_" + std::to_string(id) + "_" + std::to_string(maturity) + "Y";

        int type = typeDist(rng);
        
        if (type == 0) { // Vanilla
             // Govt bonds often pay Semi-Annual (freq=2)
            return std::make_shared<VanillaBond>(ticker, 100.0, (double)maturity, coupon, 2);
        } 
        else if (type == 1) { // FRN
            // FRNs usually have a spread over SOFR/Euribor (e.g., 0.5% to 2.5%)
            double spread = cpnDist(rng) * 0.5; 
            return std::make_shared<FloatingRateNote>(ticker + "_FRN", 100.0, (double)maturity, spread, 4); // Quarterly
        } 
        else { // Zero Coupon
            return std::make_shared<ZeroCouponBond>(ticker + "_ZERO", 100.0, (double)maturity);
        }
    }

    std::vector<std::shared_ptr<Bond>> generatePortfolio(int count) {
        std::vector<std::shared_ptr<Bond>> port;
        for (int i = 0; i < count; ++i) {
            port.push_back(generateRandomBond(i + 1)); // IDs start at 1
        }
        return port;
    }
};