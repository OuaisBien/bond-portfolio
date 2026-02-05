#include "TradingBook.hpp"
#include "PortfolioGenerator.cpp"
#include <thread>
#include <chrono>

void applyRandomMarketMove(YieldCurve &curve, std::mt19937 &rng)
{
    // Standard Deviation of 5 basis points per step
    std::normal_distribution<double> shockDist(0.0, 5.0);

    // 1. Parallel Shift (The whole curve moves)
    double parallelMove = shockDist(rng);

    // 2. Curve Twist (Steepener/Flattener)
    // Short end moves differently than long end
    double slopeMove = shockDist(rng) * 0.5;

    // Apply strictly parallel shift for now (simpler to implement in your current class)
    // To do twists, your YieldCurve class needs a method 'addRate' that updates specific nodes.
    // Let's stick to parallel for safety unless you expanded YieldCurve.

    curve.parallelShift(parallelMove);

    std::cout << ">>> MARKET MOVED: " << (parallelMove > 0 ? "+" : "")
              << parallelMove << " bps" << std::endl;
}

int main() {
    // 1. Setup Market
    YieldCurve curve;
    curve.addRate(1.0, 0.03);
    curve.addRate(5.0, 0.04);
    curve.addRate(10.0, 0.05);
    curve.addRate(30.0, 0.055);

    // 2. Generate Random Inventory
    std::cout << "--- GENERATING INVENTORY ---" << std::endl;
    PortfolioGenerator gen;
    auto marketUniverse = gen.generatePortfolio(10); // Create 10 random bonds

    TradingBook myBook;
    // Register them all
    for (const auto &bond : marketUniverse)
    {
        myBook.addKnownInstrument(bond);

        // Initial Seed Trade: Buy some of everything to start with a portfolio
        // Random quantity between -500 (Short) and +1000 (Long)
        double qty = (rand() % 1500) - 500;
        double price = bond->calculatePrice(curve); // Buying at "Mid" price
        myBook.bookTrade({bond->getTicker(), qty, price});
    }

    // 3. Monte Carlo Simulation Loop
    std::cout << "\n--- STARTING LIVE SIMULATION (Press Ctrl+C to stop) ---" << std::endl;

    std::mt19937 rng(std::random_device{}()); // Random engine for market moves

    for (int hour = 1; hour <= 5; ++hour)
    { // Simulate 5 hours of trading
        std::cout << "\n[HOUR " << hour << "]" << std::endl;

        // A. Random Market Move
        applyRandomMarketMove(curve, rng);

        // B. Random Trading Activity
        // Pick a random bond to trade
        int idx = rand() % marketUniverse.size();
        auto bondToTrade = marketUniverse[idx];

        // Trader decides to buy/sell based on randomness
        double tradeQty = (rand() % 200) - 100; // Small adjustments
        // Market price has changed due to curve move!
        double currentPrice = bondToTrade->calculatePrice(curve);

        myBook.bookTrade({bondToTrade->getTicker(), tradeQty, currentPrice});

        // C. Print Report
        myBook.printRiskReport(curve);

        // Slow down to read output
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}