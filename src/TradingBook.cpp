#include "TradingBook.hpp"
#include <iomanip>

// Position Logic

void Position::addTrade(const Trade& trade) {
    if (trade.quantity == 0) return;

    // Logic: If increasing position (Long -> More Long, or Short -> More Short)
    // We update the Average Cost (VWAP).
    // P&L is realized in reducing position.

    bool sameSign = (quantity >= 0 && trade.quantity >= 0) || (quantity < 0 && trade.quantity < 0);
    
    if (quantity == 0 || sameSign) {
        // Increasing Position: Update Weighted Average Cost
        double totalCost = (quantity * averageCost) + (trade.quantity * trade.price);
        quantity += trade.quantity;
        averageCost = totalCost / quantity;
    } 
    else {
        // Decreasing Position (Closing out): Calculate P&L on the portion sold
        // We sold 'trade.quantity' at 'trade.price', but our cost was 'averageCost'.
        
        // Example: Own 100 @ 100. Sell 10 @ 102.
        // P&L on the 10 sold = 10 * (102 - 100) = 20.
        // Remaining qty = 90. Avg Cost is still 100.
        
        double quantityClosed = std::abs(trade.quantity); // Size of the close
        if (quantityClosed > std::abs(quantity)) {
            // Flip position case (Long to Short) - omitted for brevity in this snippet
            // In a pro system, you'd split this into "Close to zero" then "Open new leg"
            quantity += trade.quantity; 
        } else {
            // Standard partial close
            double pnlOnTrade = quantityClosed * (trade.price - averageCost);
            // If we were short, profit is (Entry - Exit). If long, (Exit - Entry).
            if (quantity < 0) pnlOnTrade = quantityClosed * (averageCost - trade.price);
            
            realizedPnL += pnlOnTrade;
            quantity += trade.quantity;
        }
    }
}

double Position::getMarketValue(const YieldCurve& market) const {
    double price = instrument->calculatePrice(market);
    // Price is typically per 100 or per 1 unit. Assuming price is percentage (e.g. 1.02 for 102%) 
    // or absolute (102). Let's assume price is absolute for 1 unit of notional.
    // If price is 100.00 and Quantity is 1000, MV is 100,000.
    // Assume calculatePrice returns the value of 1 unit of Bond.
    
    // Since our bond class calculates price for its internal notional, 
    // we need to scale it to the position quantity.
    // Scale = Holding / Bond_Definition_Notional
    
    // Hack for this example: Assume Bond definition has Notional 100.0
    // and Position Quantity is total face value.
    // Ideally, bond.calculatePrice() should return % of par, but currently it returns absolute cash.

    return quantity * instrument->calculatePrice(market); 
}

double Position::getTotalPV01(const YieldCurve& market) const {
    double unitPV01 = RiskEngine::calculatePV01(*instrument, market);
    return quantity * unitPV01;
}

double Position::getUnrealizedPnL(const YieldCurve& market) const {
    double currentPrice = instrument->calculatePrice(market);
    // (Market Price - Avg Cost) * Quantity
    return (currentPrice - averageCost) * quantity;
}


// =======================
// TradingBook Logic
// =======================

double TradingBook::getPosition(const std::string &ticker) const
{
    auto it = positions.find(ticker);
    if (it != positions.end())
    {
        return it->second.quantity;
    }
    return 0.0;
}

void TradingBook::addKnownInstrument(std::shared_ptr<Bond> bond) {
    // Create an empty position for this bond
    if (positions.find(bond->getTicker()) == positions.end())
    {
        positions.emplace(bond->getTicker(), Position(bond));
        std::cout << "Registered Instrument: " << bond->getTicker() << std::endl;
    }
}

void TradingBook::bookTrade(const Trade &trade, double midPrice) {
    auto it = positions.find(trade.bondName);
    if (it == positions.end())
    {
        std::cerr << "Error: Bond not found." << std::endl;
        return;
    }

    // 1. Calculate
    double edgeCaptured = 0.0;

    if (trade.quantity > 0)
    {
        // Client sells
        // Edge = Mid - PricePaid
        edgeCaptured = (midPrice - trade.price) * trade.quantity;
    }
    else
    {
        // Client buys
        // Edge = PriceReceived - Mid
        // Note: quantity is negative
        edgeCaptured = (trade.price - midPrice) * std::abs(trade.quantity);
    }

    // 3. Update Metrics
    realizedSpreadPnL += edgeCaptured;
    it->second.addTrade(trade); // Accounting update

    std::cout << "[TRADE] " << (trade.quantity > 0 ? "BUY " : "SELL ")
            << std::abs(trade.quantity) << " of " << trade.bondName
            << " @ " << trade.price
            << " (Mid: " << midPrice << ")"
            << " | Edge Captured: " << edgeCaptured << std::endl;
}

void TradingBook::printRiskReport(const YieldCurve& market) const {
    std::cout << "\n================ MARKET MAKER RISK BLOTTER ================" << std::endl;
    std::cout << std::left << std::setw(20) << "Bond"
              << std::right << std::setw(10) << "Net Qty"
              << std::setw(12) << "Mkt Price"
              << std::setw(12) << "Avg Cost"
              << std::setw(12) << "Unreal P&L"
              << std::setw(12) << "Total PV01" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    double totalPnL = 0;
    double totalRisk = 0;

    for (const auto& [name, pos] : positions) {
        if (pos.quantity == 0) continue; // Skip flat positions

        double price = pos.instrument->calculatePrice(market);
        double unrlzd = pos.getUnrealizedPnL(market);
        double risk = pos.getTotalPV01(market);

        totalPnL += unrlzd;
        totalRisk += risk;

        std::cout << std::left << std::setw(20) << name
                  << std::right << std::setw(10) << pos.quantity
                  << std::setw(12) << std::fixed << std::setprecision(2) << price
                  << std::setw(12) << pos.averageCost
                  << std::setw(12) << unrlzd
                  << std::setw(12) << risk << std::endl;
    }
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "TOTAL BOOK P&L (Unrealized): " << totalPnL << std::endl;
    std::cout << "TOTAL BOOK RISK (PV01):      " << totalRisk << " (Loss if rates +1bp)" << std::endl;
    std::cout << "===============================================================================\n" << std::endl;
}
