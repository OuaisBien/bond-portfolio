#pragma once
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include "Bond.hpp"
#include "YieldCurve.hpp"
#include "RiskEngine.hpp"

// Asymmetric spread
struct Quote
{
    double bid;
    double ask;
    double skew; // For debugging
};

// Represents a single transaction
struct Trade
{
    std::string bondName;
    double quantity; // Positive for BUY, Negative for SELL
    double price;    // Execution price (clean price)
};

// Represents our current holding in a specific bond
class Position
{
public:
    std::shared_ptr<Bond> instrument;
    double quantity;    // Face Value of Current holdings
    double averageCost; // VWAP
    double realizedPnL; // Cash banked from closing positions

    Position(std::shared_ptr<Bond> bond)
        : instrument(bond), quantity(0), averageCost(0), realizedPnL(0) {}

    void addTrade(const Trade &trade);

    // Market Value = Quantity * Current Market Price
    double getMarketValue(const YieldCurve &market) const;

    // Position Risk = PV01 (per unit) * Quantity
    double getTotalPV01(const YieldCurve &market) const;

    // Unrealized P&L = (Current Price - Cost) * Quantity
    double getUnrealizedPnL(const YieldCurve &market) const;
};

// The Aggregated Portfolio
class TradingBook
{
private:
    // Map Bond Name -> Position Object
    std::map<std::string, Position> positions;
    double realizedSpreadPnL = 0; // Spread profit from market-making
    double riskAversion = 0.01;

public:
    // Helper to register a bond in the system (Reference Data)
    void addKnownInstrument(std::shared_ptr<Bond> bond);

    // Execute a trade
    void bookTrade(const Trade& trade, double midPrice);

    std::pair<double, double> getBidAsk(const Bond& bond, const YieldCurve& market) const {
        double mid = bond.calculatePrice(market);
        double halfSpread = 0.05; // spread fixed at 0.5 per 100 face value
        return {mid - halfSpread, mid + halfSpread};
    }

    double getPosition(const std::string& ticker) const;

    double getSpreadPnL() const { return realizedSpreadPnL; }

    Quote getQuotedSpread(const std::string& ticker, double midPrice, double unitPV01, double baseSpread) const {
        double currentInventory = 0.0;
        double riskMagnitude = std::abs(unitPV01);

        // 1. Check current invetory
        auto it = positions.find(ticker);
        if (it != positions.end()) {
            currentInventory = it->second.quantity;
        }

        // 2. Calculate Skew (-1 * aversion * inventory * bond risk)
        double rawSkew = -1.0 * riskAversion * currentInventory * riskMagnitude;

        double maxSkew = baseSpread * 1.5;

        if (rawSkew > maxSkew)
            rawSkew = maxSkew;
        if (rawSkew < -maxSkew)
            rawSkew = -maxSkew;

        // 3. Generate Final Quotes
        double halfSpread = baseSpread / 2.0;

        return {
            midPrice - halfSpread + rawSkew, // Bid
            midPrice + halfSpread + rawSkew, // Ask
            rawSkew                          
        };
    }

    // Market Maker Report
    void printRiskReport(const YieldCurve &market) const;
};