# Decentralized Exchange (DEX) Implementation

This project implements a Constant Product Automated Market Maker (AMM) DEX with arbitrage functionality and simulation tools.

## Smart Contracts (Solidity)

### Core Contracts
- `DEX.sol`: Main DEX contract implementing the AMM functionality with constant product formula (x*y=k)
  - Handles token swaps, deposits, and withdrawals
  - Maintains token reserves and spot prices

- `LPTokens.sol`: LP (Liquidity Provider) token contract
  - ERC20 token representing liquidity shares
  - Handles minting/burning of LP tokens when users add/remove liquidity

- `MyToken.sol`: Simple ERC20 token implementation
  - Used for creating the trading pairs (Token A and Token B)
  - Fixed total supply of 1 billion tokens

- `arbitrage.sol`: Arbitrage contract for exploiting price differences
  - Implements arbitrage logic between two DEX instances
  - Checks for profitable opportunities across DEXes
  - Executes trades when profit exceeds threshold

## Simulation Scripts (JavaScript)

- `simulate_DEX.js`: Simulates DEX operations
  - Deploys contracts and sets up trading environment
  - Simulates deposits, withdrawals, and swaps
  - Collects metrics like trading volume, fees, and TVL
  - Generates data points for visualization

- `simulate_arbitrage.js`: Simulates arbitrage scenarios
  - Sets up two DEX instances with different prices
  - Tests both profitable and unprofitable arbitrage conditions

## Visualization Tools (Python)

- `plot.py`: Main plotting script that generates various charts:
  - Total Value Locked (TVL) over time
  - Swap/Trading volume for both tokens
  - Fee Accumulation
  - Reserve ratio
  - Spot price
  - LP token distribution
  - Slippage

- `plot_slippage_vs_trade_lot_fraction.py`: Specialized plot for slippage analysis
  - Shows relationship between trade size and price impact
  - Helps visualize the AMM's price impact function

## Metrics Tracked

- Total Value Locked (TVL)
- Trading Volume
- Fee Collection
- Reserve Ratios
- Spot Prices
- Slippage
- LP Token Distribution

## Usage

1. Run simulations using the JavaScript files (Contracts are deployed automatically)
2. Generate visualizations using the Python scripts