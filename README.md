# Market Making & Fixed Income Pricing Engine in C++
## Overview

A trading simulation engine developed in Modern C++. The system models a real-time Market Maker’s book, featuring an object-oriented pricing library for fixed-income instruments, a dynamic yield curve environment, and a risk management module for calculating sensitivities (PV01) and simulating stochastic market scenarios.

<ul>
  <li><b>Pricing:</b> Calculates fair value for various bond types using a dynamic Yield Curve.</li>
  <li><b>Risk Management:</b> Real-time calculation of PV01 (sensitivity) and Value-at-Risk.</li>
  <li><b>Market Making:</b> Simulates an algorithmic trader that quotes two-way prices (Bid/Ask). The engine uses an Inventory-Aware Skewing Model to dynamically adjust prices to defend against risk (similar to the Avellaneda-Stoikov framework).</li>
</ul>
 

## System Architecture
The system is designed with a clear separation between Market Data, Instrument Definitions, and Trading Logic.

### Class Interaction Diagram

<img src="https://github.com/OuaisBien/bond-portfolio/blob/main/design.png" width=700>

### The Core Componentss
<ol>
  <li><b>Market Data (<code>YieldCurve</code>)</b>
    <ul>
      <li><b>Role:</b> Represents the state of the economy (Interest Rates).</li>
      <li><b>Logic:</b> Stores key rate points (1Y, 5Y, 10Y) and performs Linear Interpolation to find the rate for any specific date $t$.</li>
      <li><b>Key Feature:</b> Can be "shocked" (shifted up/down) to simulate market moves.</li></ul></li>
  <li><b>Instruments (<code>Bond</code> Hierarchy)</b>
    <ul>
      <li><b>Bond (Abstract Base):</b> Defines the interface (calculatePrice, getCashFlows). Contains the unique Ticker (ID).</li>
      <li><b>VanillaBond:</b> Standard fixed coupon bond.</li>
      <li><b>ZeroCouponBond:</b> No coupons, pays face value at maturity. Highly sensitive to rate changes (High Duration).
      <li><b>FloatingRateNote (FRN):</b> Coupons adjust with the market rate. Very low sensitivity to rate changes (Near-zero Duration).</li></li></ul></li>
  
  <li><b>Analytics (<code>RiskEngine</code>)</b>
    <ul>
      <li><b>Role:</b> The "Brain" for math.</li>
      <li><b>Stateless:</b> It takes an Instrument and a Curve, and outputs a risk number.</li>
      <li><b>PV01 Calculation:</b> Calculates the price change for a 1 basis point (0.01%) parallel shift in the curve. Used to quantify how "risky" a bond is.</li></ul></li>
  <li><b> Trading System (<code>TradingBook</code> & <code>Position</code>)</b>
    <ul>
      <li><code>Position</code>: Tracks a specific holding.</li>
        <ul>
          <li><b>Quantity:</b> Net Long (+) or Short (-) amount.</li>
          <li><b>VWAP:</b> Volume Weighted Average Price (Cost Basis).</li>
          <li><b>Realized P&L:</b> Cash banked from closed trades.</li>
        </ul>
      </li>
      <li><code>TradingBook</code>: The aggregator.
        <ul>
          <li>Maintains the map of Ticker -> Position.</li>
          <li>Calculates Unrealized P&L (Mark-to-Market).</li>
          <li><b>Key Feature:</b> Generates the <b>Asymmetric Bid/Ask Spread</b>.</li>
        </ul></li>
      </ul></li>
</ol>


## The Market Making Model (Asymmetric Spread)
The core intelligence of this simulator is how it quotes prices. It does not just quote a static spread around the mid-price. It `skews` the price based on <b>Inventory Risk</b>.
### The Logic
<ol>
  <li><b>Theoretical Mid Price:</b> Calculated by discounting cash flows using the YieldCurve.</li>
  <li><b>Base Spread:</b> 10 cents.</li>
  <li>Inventory Skew:
    <ul>
      <li>If we are <b>Long (Inventory > 0):</b> We are exposed to rates rising. The engine shifts prices DOWN (lower Bid/Ask) to attract buyers and discourage sellers.</li>
      <li>If we are <b>Short (Inventory < 0):</b> We are exposed to rates falling. The engine shifts prices UP (higher Bid/Ask) to attract sellers and discourage buyers.</li>
    </ul></li>
</ol>
        
### The Formula
The quoted Bid ($P_b$) and Ask ($P_a$) are calculated as:

$$\text{Skew} = -1 \cdot \gamma \cdot \text{Inventory} \cdot \mid\text{PV01}\mid$$

$$P_{bid} = P_{mid} - \frac{\text{Spread}}{2} + \text{Skew}$$

$$P_{ask} = P_{mid} + \frac{\text{Spread}}{2} + \text{Skew}$$

<ul>
  <li>$\gamma$ <b>(Gamma):</b> The Risk Aversion parameter. A higher value means the trader is more "scared" of holding inventory and will skew prices more aggressively to neutralize it.</li>
  <li><b>PV01:</b> We multiply by the bond's specific risk. Being short a 30Y Bond (High Risk) creates a much larger skew than being short a 2Y Bond (Low Risk).</li>
</ul>



## Building and Running
### Prerequisites
<ul>
  <li>C++ Compiler supporting C++17 (GCC, Clang, MSVC).</li>
  <li>CMake (3.10+).</li>
</ul>

### Compilation
````
mkdir build
cd build
cmake ..
make
````
### Running the Simulator
````
./PricingEngine
````
 
## Sample Output Explanation
````
[HOUR 9]
>>> MARKET MOVED: -1.09 bps
Ticker: BOND_1_10Y_ZERO | Inv: -314.55 | Mid: 60.16 | Skew: 0.15 | Quote: 60.26 / 60.36
[TRADE] SELL 586.65 of BOND_1_10Y_ZERO @ 60.36 (Mid: 60.16) | Edge Captured: 117.33

================ MARKET MAKER RISK BLOTTER ================
Bond                   Net Qty   Mkt Price    Avg Cost  Unreal P&L  Total PV01
-------------------------------------------------------------------------------
BOND_10_5Y_FRN         -313.04      107.24      107.26        5.24        0.64
BOND_1_10Y_ZERO        -901.20       60.16       60.49      301.18       54.19
BOND_2_5Y_FRN           749.00      102.59      102.60       -5.28       -0.65
BOND_3_12Y             -503.78       98.77       98.88       55.88       45.29
BOND_4_10Y             -771.30       82.31       82.95      496.14       54.62
BOND_5_16Y_ZERO         -70.00       43.30       43.87       39.95        4.85
BOND_6_20Y_FRN          234.23      108.44      108.52      -17.21       -2.09
BOND_7_27Y              544.00       44.49       45.19     -377.34      -45.60
BOND_8_13Y             1021.19       84.14       84.53     -400.57      -88.05
BOND_9_28Y              423.00       59.09       59.92     -349.58      -42.26
-------------------------------------------------------------------------------
TOTAL BOOK P&L (Unrealized): -251.58
TOTAL BOOK RISK (PV01):      -19.08 (Loss if rates +1bp)
===============================================================================
````

<ul>
  <li><b>Inv: -314.55:</b> The trader is Short 314.55 units.</li>
  <li><b>Skew: +0.15:</b> Because the trader is Short, the algorithm adds a positive skew (15 cents).</li>
  <li><b>Quote:</b>
    <ul>
      <li>Theoretical Mid was 60.16.</li>
      <li>Standard Quote would be 60.11 / 60.21.</li>
      <li>Skewed Quote is 60.26 / 60.36.</li>
      <li>Result: The trader is bidding 60.11 (above fair value!) to desperately buy back bonds and close the short position.</li>
    </ul></li>
</ul>

Notice post-trade now the portfolio has $-314.55 - 586.65 = -901.20$ of `BOND_1_10Y_ZERO`.



