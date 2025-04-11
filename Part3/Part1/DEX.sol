// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;
// Import the ERC20 interface
interface IERC20 {
    function balanceOf(address account) external view returns (uint256);
    function transfer(address recipient, uint256 amount) external returns (bool);
    function approve(address spender, uint256 amount) external returns (bool);
    function transferFrom(address sender, address recipient, uint256 amount) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
    function totalSupply() external view returns (uint256);
}

interface LPTok {
    function balanceOf(address account) external view returns (uint256);
    function generateTokens(uint _amount, address receiver) external  returns (bool success, uint);
    function burn(uint _amount, address owner) external returns (bool success, uint);
    function getTotalTokens() external  view returns (uint256);
}

contract DEX {
    
    // State variables to store the addresses of the two ERC20 tokens
    address public token1;
    address public token2;
    address private LPTokens;
    address private owner;
    uint256 internal reserve1;
    uint256 internal reserve2;
    // Constructor to initialize the token addresses
    constructor(address _token1, address _token2) {
        require(_token1 != address(0) && _token2 != address(0), "Invalid token address");
        require(_token1 != _token2, "Tokens must be different");
        token1 = _token1;
        token2 = _token2;
        owner = msg.sender;
    }

    function getGCD(uint a, uint b) internal pure returns (uint) {
        if (b == 0) {
            return a;
        }
        return getGCD(b, a % b);
    }
    
    function setLPTokenAddress(address _LPTokens) public{ 
        if(msg.sender != owner){return;}
        LPTokens = _LPTokens;
    }

    // simplify by dividing by gcd
    function _simplify() internal {
        reserve1 = reserve1 / getGCD(reserve1, reserve2);
        reserve2 = reserve2 / getGCD(reserve1, reserve2);
    }

    // Function to get the balance of token1 held by the DEX contract
    function getToken1Balance() public view returns (uint256) {
        return IERC20(token1).balanceOf(getMyAddress());
    }

    // Function to get the balance of token2 held by the DEX contract
    function getToken2Balance() public view returns (uint256) {
        return IERC20(token2).balanceOf(getMyAddress());
    }

    function getMyAddress() public view returns (address) {
        return address(this);
    }
    
    function _transferToken(address _token, address sender, address receiver,uint256 amount) internal {
        IERC20(_token).transferFrom(sender, receiver, amount);
    }

    function depositTokens(uint amt1, uint amt2) public {
        if (amt1 == 0 || amt2 == 0) {
            return;
        }
        address sender = msg.sender;
        uint256 token1Balance = getToken1Balance();
        uint256 token2Balance = getToken2Balance();
        
        if(token1Balance == 0 && token2Balance == 0){
            reserve1 = amt1;
            reserve2 = amt2;
            // _simplify();
        }

        if(amt1*reserve2 != amt2*reserve1) {
            return;
        }

        uint LPTokensReward = (token1Balance == 0) ? 10**18 :  amt1 * 10**18 / token1Balance;
        LPTok(LPTokens).generateTokens(LPTokensReward, sender);
        _transferToken(token1, sender, address(this), amt1);
        _transferToken(token2, sender, address(this), amt2);
    }

    function withdrawTokens(uint256 LPAmt) public returns(uint, uint) {
        address sender = msg.sender;
        bool success = false;
        uint balance = 0;
        uint totalTokens = LPTok(LPTokens).getTotalTokens();
        (success, balance) = LPTok(LPTokens).burn(LPAmt, sender);
        if (success) {
            uint balanceA = getToken1Balance();
            uint balanceB = getToken2Balance();
            // _transferToken(token1, address(this), sender,balanceA*LPAmt/totalTokens);
            // _transferToken(token2, address(this), sender,balanceB*LPAmt/totalTokens);
            IERC20(token1).transfer(sender, balanceA*LPAmt/totalTokens);
            IERC20(token2).transfer(sender, balanceB*LPAmt/totalTokens);
            return (balanceA*LPAmt/totalTokens, balanceB*LPAmt/totalTokens);
        }
        return (0,0);
    }

    function swap(string memory token , uint amount) public {
        
    }
}