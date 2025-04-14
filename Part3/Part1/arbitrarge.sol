// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

interface DEX {
    function reserveRatio() external view returns (uint256);
    function getTokenABalance() external view returns (uint256);
    function getTokenBBalance() external view returns (uint256);
    function depositTokens(uint256 amt1, uint256 amt2) external;
    function withdrawTokens(uint256 amt1, uint256 amt2) external;
    function swap(string memory token , uint256 amount) external returns (uint256);
    function spotPrice(string memory token) external view returns (uint256);
    function getCallerBalance() external view returns (uint256, uint256);
}

contract Arbitrage {
    address internal DEX1;
    address internal DEX2;
    uint internal constant fee = 3;
    uint internal constant threshold = 10*10**18;
    constructor(address _dex1, address _dex2) {
        DEX1 = _dex1;
        DEX2 = _dex2;
    }

    function abs(int256 x) internal pure returns (uint256) {
        return x < 0 ? uint256(-x) : uint256(x);
    }

    function getArbitrargeProfit(address _dex1, address _dex2, bool ABA) internal view returns (uint256) {
        
        if (ABA) {
            uint256 spotPriceA1 = DEX(_dex1).spotPrice("TokenA")*(1000-fee)/1000;
            uint256 spotPriceB2 = DEX(_dex2).spotPrice("TokenB")*(1000-fee)/1000;
            uint256 initialTokenA = 1*10**18;
            uint256 finalTokenA = spotPriceA1*spotPriceB2/(10**18);
            uint256 arbitrageProfit = finalTokenA - initialTokenA;
            return arbitrageProfit;
        } else {
            uint256 spotPriceB1 = DEX(_dex1).spotPrice("TokenB")*(1000-fee)/1000;
            uint256 spotPriceA2 = DEX(_dex2).spotPrice("TokenA")*(1000-fee)/1000;
            uint256 initialTokenB = 1*10**18;
            uint256 finalTokenB = spotPriceB1*spotPriceA2/(10**18);
            uint256 arbitrageProfit = finalTokenB - initialTokenB;
            return arbitrageProfit;
        }
    }

    function max(uint256 a, uint256 b) internal pure returns (uint256) {
        return a > b ? a : b;
    }

    function bestExchangeRate(address _dex1, address _dex2, bool BperA) internal view returns (uint256) {
        if (BperA) {
            return max(DEX(_dex1).spotPrice("TokenA"), DEX(_dex2).spotPrice("TokenA"));
        } else {
            return max(DEX(_dex1).spotPrice("TokenB"), DEX(_dex2).spotPrice("TokenB"));
        }
    }

    function arbitrage() external returns (uint256, uint256, string memory) {
        uint256 spotPriceA1 = DEX(DEX1).spotPrice("TokenA");
        uint256 spotPriceA2 = DEX(DEX2).spotPrice("TokenA");
        
        if (spotPriceA1 == spotPriceA2) return (0,0, "No arbitrage opportunity");
        if (spotPriceA1 < spotPriceA2) {
            address tmp = DEX1;
            DEX1 = DEX2;
            DEX2 = tmp;
        }
        
        uint256 profitABA = getArbitrargeProfit(DEX1, DEX2, true); // returns the profit if we do arbitrage in terms of token A
        uint256 profitBAB = getArbitrargeProfit(DEX2, DEX1, false); // returns the profit if we do arbitrage in terms of token B
        if(profitABA < threshold && profitBAB < threshold) return (0,0, "No arbitrage opportunity");
        if(profitABA < threshold) profitABA = 0;
        if(profitBAB < threshold) profitBAB = 0;

        uint256 maxBperA = bestExchangeRate(DEX1, DEX2, true); // returns the best exchange rate at which we can exchange token A to token B
        uint256 maxAperB = bestExchangeRate(DEX1, DEX2, false); // returns the best exchange rate at which we can exchange token B to token A

        uint256 profitABAinTermsB = maxBperA*profitABA;
        uint256 profitBABinTermsA = maxAperB*profitBAB;

        if(profitABA > profitBABinTermsA) {
            uint256 receivedB = DEX(DEX1).swap("TokenA", 1*10**18);
            uint256 receivedA = DEX(DEX2).swap("TokenB", receivedB);
            uint256 profit = receivedA - 1*10**18;
            return (1*10**18, profit, "TokenA");

        } else if(profitBAB >= profitABAinTermsB) {
            uint256 receivedA = DEX(DEX2).swap("TokenB", 1*10**18);
            uint256 receivedB = DEX(DEX1).swap("TokenA", receivedA);
            uint256 profit = receivedB - 1*10**18;
            return (profit, 1*10**18, "TokenB");
        } else {
            return (0,0, "No arbitrage opportunity");
        }
    }
}
    