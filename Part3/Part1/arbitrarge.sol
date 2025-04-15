// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

// Add the ERC20 interface
interface IERC20 {
    function balanceOf(address account) external view returns (uint256);
    function transfer(address recipient, uint256 amount) external returns (bool);
    function approve(address spender, uint256 amount) external returns (bool);
    function transferFrom(address sender, address recipient, uint256 amount) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
}

interface DEX_Interface {
    function tokenA() external view returns (address);
    function tokenB() external view returns (address);
    function reserveRatio() external view returns (uint256);
    function getTokenABalance() external view returns (uint256);
    function getTokenBBalance() external view returns (uint256);
    function depositTokens(uint256 amt1, uint256 amt2) external;
    function withdrawTokens(uint256 LPAmt) external returns (uint256, uint256);
    function swap(string memory token , uint256 amount) external returns (uint256);
    function spotPrice(string memory token) external view returns (uint256);
    function getCallerBalance() external view returns (uint256, uint256);
}

contract Arbitrage {
    address internal DEX1;
    address internal DEX2;
    address internal tokenA;
    address internal tokenB;
    uint internal constant fee = 3;
    uint internal constant threshold = 1*10**17;

    event Log(string message);
    event LogInt(uint value);
    event LogAddress(address addr);
    event ArbitrageResult(uint256 amount, uint256 profit, string tokenType);

    constructor(address _dex1, address _dex2) {
        DEX1 = _dex1;
        DEX2 = _dex2;
        
        // Extract token addresses from DEX1
        tokenA = DEX_Interface(DEX1).tokenA();
        tokenB = DEX_Interface(DEX1).tokenB();
    }
    
    // Function to approve DEXes to spend tokens
    function approveTokens(uint256 amount) external {
        // Approve both DEXes to spend TokenA and TokenB
        bool success1 = IERC20(tokenA).approve(DEX1, amount);
        bool success2 = IERC20(tokenA).approve(DEX2, amount);
        bool success3 = IERC20(tokenB).approve(DEX1, amount);
        bool success4 = IERC20(tokenB).approve(DEX2, amount);
        
        require(success1 && success2 && success3 && success4, "Token approval failed");
    }
    
    // Function to check token balances
    function getTokenBalances() external view returns (uint256, uint256) {
        return (
            IERC20(tokenA).balanceOf(address(this)),
            IERC20(tokenB).balanceOf(address(this))
        );
    }

    function abs(int256 x) internal pure returns (uint256) {
        return x < 0 ? uint256(-x) : uint256(x);
    }

    function getArbitrargeProfit(address _dex1, address _dex2, bool ABA) internal returns (uint256) {
        
        if (ABA) {
            uint256 spotPriceA1 = (DEX_Interface(_dex1).spotPrice("TokenA")*(1000-fee))/1000;
            uint256 spotPriceB2 = (DEX_Interface(_dex2).spotPrice("TokenB")*(1000-fee))/1000;
            uint256 initialTokenA = 1*10**18;
            uint256 finalTokenA = (spotPriceA1*spotPriceB2)/(10**18);
            if(finalTokenA < initialTokenA) return 0;
            uint256 arbitrageProfit = finalTokenA - initialTokenA;
            return arbitrageProfit;
        } else {
            uint256 spotPriceB1 = (DEX_Interface(_dex1).spotPrice("TokenB")*(1000-fee))/1000;
            uint256 spotPriceA2 = (DEX_Interface(_dex2).spotPrice("TokenA")*(1000-fee))/1000;
            uint256 initialTokenB = 1*10**18;
            uint256 finalTokenB = (spotPriceB1*spotPriceA2)/(10**18);
            if(finalTokenB < initialTokenB) return 0;
            uint256 arbitrageProfit = finalTokenB - initialTokenB;
            return arbitrageProfit;
        }
    }

    function max(uint256 a, uint256 b) internal pure returns (uint256) {
        return a > b ? a : b;
    }

    function bestExchangeRate(address _dex1, address _dex2, bool BperA) internal view returns (uint256) {
        if (BperA) {
            return max(DEX_Interface(_dex1).spotPrice("TokenA"), DEX_Interface(_dex2).spotPrice("TokenA"));
        } else {
            return max(DEX_Interface(_dex1).spotPrice("TokenB"), DEX_Interface(_dex2).spotPrice("TokenB"));
        }
    }

    function arbitrage() external {
        uint256 spotPriceA1 = DEX_Interface(DEX1).spotPrice("TokenA");
        uint256 spotPriceA2 = DEX_Interface(DEX2).spotPrice("TokenA");
        
        if (spotPriceA1 == spotPriceA2) {
            emit ArbitrageResult(0, 0, "No arbitrage opportunity");
            return;
        }
        if (spotPriceA1 < spotPriceA2) {
            address tmp = DEX1;
            DEX1 = DEX2;
            DEX2 = tmp;
        }
        
        uint256 profitABA = getArbitrargeProfit(DEX1, DEX2, true); // returns the profit if we do arbitrage in terms of token A
        uint256 profitBAB = getArbitrargeProfit(DEX2, DEX1, false); // returns the profit if we do arbitrage in terms of token B
        if(profitABA < threshold && profitBAB < threshold) {
            emit ArbitrageResult(0, 0, "No arbitrage opportunity");
            return;
        }
        if(profitABA < threshold) profitABA = 0;
        if(profitBAB < threshold) profitBAB = 0;

        uint256 maxBperA = bestExchangeRate(DEX1, DEX2, true); // returns the best exchange rate at which we can exchange token A to token B
        uint256 maxAperB = bestExchangeRate(DEX1, DEX2, false); // returns the best exchange rate at which we can exchange token B to token A

        uint256 profitABAinTermsB = maxBperA*profitABA / 10**18;
        uint256 profitBABinTermsA = maxAperB*profitBAB / 10**18;

        if(profitABA >= profitBABinTermsA) {
            IERC20(tokenA).approve(DEX1, 1*10**18);
            uint256 receivedB = DEX_Interface(DEX1).swap("TokenA", 1*10**18);
            IERC20(tokenB).approve(DEX2, receivedB);
            uint256 receivedA = DEX_Interface(DEX2).swap("TokenB", receivedB);
            uint256 profit = receivedA - 1*10**18;
            emit ArbitrageResult(1*10**18, profit, "TokenA");
            return;

        } else if(profitBAB >= profitABAinTermsB) {
            IERC20(tokenB).approve(DEX2, 1*10**18);
            uint256 receivedA = DEX_Interface(DEX2).swap("TokenB", 1*10**18);
            IERC20(tokenA).approve(DEX1, receivedA);
            uint256 receivedB = DEX_Interface(DEX1).swap("TokenA", receivedA);
            uint256 profit = receivedB - 1*10**18;
            emit ArbitrageResult(1*10**18, profit, "TokenB");
            return;
        } else {
            emit ArbitrageResult(0, 0, "No arbitrage opportunity");
            return;
        }
    }
}
    