#include "TradingBook.hpp"
#include "PortfolioGenerator.cpp"
#include <thread>
#include <chrono>
#include <random>

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
        myBook.bookTrade({bond->getTicker(), qty, price}, price);
    }

    // Parameters
    double BASE_SPREAD = 0.10; // 10 cents
    std::mt19937 rng(std::random_device{}());

    // Random distributions
    // Trade size: Normal distribution centered at 500, sigma 200
    std::normal_distribution<double> sizeDist(500.0, 200.0);
    // Trade direction: 50/50 Buy or Sell
    std::uniform_int_distribution<int> sideDist(0, 1);

    // 3. Monte Carlo Simulation Loop
    std::cout << "\n--- STARTING LIVE SIMULATION (Press Ctrl+C to stop) ---" << std::endl;

    for (int hour = 1; hour <= 10; ++hour)
    { // Simulate 10 trades
        std::cout << "\n[HOUR " << hour << "]" << std::endl;

        applyRandomMarketMove(curve, rng);

        /// 1. Pick a random bond
        auto bond = marketUniverse[rand() % marketUniverse.size()];
        std::string ticker = bond->getTicker();

        // 2. Calculate Market Analytics
        double midPrice = bond->calculatePrice(curve);
        double unitPV01 = RiskEngine::calculatePV01(*bond, curve);

        // 3. Get OUR Quotes (Inventory Aware)
        Quote quote = myBook.getQuotedSpread(ticker, midPrice, unitPV01, BASE_SPREAD);

        // 4. Generate Random Client Order
        bool clientBuys = sideDist(rng) == 1; // 1 = Sell, 0 = Buy
        double tradeSize = std::abs(sizeDist(rng));

        // 5. Execution Logic
        double executePrice;
        double quantityForUs; // Positive if we buy, Negative if we sell

        if (clientBuys)
        {
            executePrice = quote.ask;
            quantityForUs = -tradeSize;
        }
        else
        {
            executePrice = quote.bid;
            quantityForUs = tradeSize;
        }

        if (hour % 1 == 0)
        {
            std::cout << "Ticker: " << ticker
                      << " | Inv: " << myBook.getPosition(ticker)
                      << " | Mid: " << midPrice
                      << " | Skew: " << quote.skew
                      << " | Quote: " << quote.bid << " / " << quote.ask << std::endl;
        }

        // 4. Book it
        double proposedPrice = clientBuys ? quote.ask : quote.bid;
        double deviation = std::abs(proposedPrice - midPrice);

        // Client's sensitivity to spread magnitude
        // Probability drops as price moves away from Mid
        double probOfTrade = std::exp(- deviation); // Decay function

        if (std::uniform_real_distribution<>(0, 1)(rng) < probOfTrade)
        {
            myBook.bookTrade({ticker, quantityForUs, executePrice}, midPrice);

            // 5. Print Report
            myBook.printRiskReport(curve);

            // Slow down to read output
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        else
        {
            std::cout << "Client rejected quote (Spread too wide/skewed)" << std::endl;
        }
    }

    return 0;
}