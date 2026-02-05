#include "TradingBook.hpp"
#include "Instruments.hpp"

int main() {
    // 1. Market Setup
    YieldCurve curve;
    curve.addRate(1.0, 0.03); 
    curve.addRate(5.0, 0.04);
    curve.addRate(10.0, 0.05);

    // Define Reference Data (The universe of tradable bonds)
    // We use shared_ptr because the Book and the Database share them.
    auto bond2Y = std::make_shared<VanillaBond>("UST_2Y_2026", 100.0, 2.0, 0.03, 1);
    auto bond10Y = std::make_shared<VanillaBond>("UST_10Y_2034", 100.0, 10.0, 0.045, 1);
    auto corpBond = std::make_shared<FloatingRateNote>("GS_FRN_2027", 100.0, 3.0, 0.015, 2);

    TradingBook myBook;

    // 2. Register them in the book (Inventory setup)
    myBook.addKnownInstrument(bond2Y);
    myBook.addKnownInstrument(bond10Y);
    myBook.addKnownInstrument(corpBond);

    // 3. Trading Activity
    std::cout << "--- MARKET OPEN ---" << std::endl;
    // The TradingBook looks up "UST_10Y_2034" in its map to find the Bond object
    myBook.bookTrade({"UST_10Y_2034", 1000, 99.50});
    myBook.bookTrade({"GS_FRN_2027", 500, 100.10});

    // 4. Intraday Risk Report
    myBook.printRiskReport(curve);

    // 5. Market Moves! (Rates go UP by 10bps)
    std::cout << "--- NEWS FLASH: INFLATION DATA HIGHER THAN EXPECTED ---" << std::endl;
    curve.parallelShift(10.0); // Rates up, Bond prices down.

    // 6. End of Day Report
    myBook.printRiskReport(curve);

    return 0;
}