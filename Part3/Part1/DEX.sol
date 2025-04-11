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
    address public tokenA;
    address public tokenB;
    address private LPTokens;
    address private owner;
    uint256 internal reserve1;
    uint256 internal reserve2;
    // Constructor to initialize the token addresses
    constructor(address _tokenA, address _tokenB) {
        require(_tokenA != address(0) && _tokenB != address(0), "Invalid token address");
        require(_tokenA != _tokenB, "Tokens must be different");
        tokenA = _tokenA;
        tokenB = _tokenB;
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

    // Function to get the balance of tokenA held by the DEX contract
    function getTokenABalance() public view returns (uint256) {
        return IERC20(tokenA).balanceOf(getMyAddress());
    }

    // Function to get the balance of tokenB held by the DEX contract
    function getTokenBBalance() public view returns (uint256) {
        return IERC20(tokenB).balanceOf(getMyAddress());
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
        uint256 tokenABalance = getTokenABalance();
        uint256 tokenBBalance = getTokenBBalance();
        
        if(tokenABalance == 0 && tokenBBalance == 0){
            reserve1 = amt1;
            reserve2 = amt2;
        }

        if(amt1*reserve2 != amt2*reserve1) {
            return;
        }

        uint LPTokensReward = (tokenABalance == 0) ? 10**18 :  amt1 * 10**18 / tokenABalance;
        LPTok(LPTokens).generateTokens(LPTokensReward, sender);
        _transferToken(tokenA, sender, address(this), amt1);
        _transferToken(tokenB, sender, address(this), amt2);
    }

    function withdrawTokens(uint256 LPAmt) public returns(uint, uint) {
        address sender = msg.sender;
        bool success = false;
        uint balance = 0;
        uint totalTokens = LPTok(LPTokens).getTotalTokens();
        (success, balance) = LPTok(LPTokens).burn(LPAmt, sender);
        if (success) {
            uint balanceA = getTokenABalance();
            uint balanceB = getTokenBBalance();
            // _transferToken(tokenA, address(this), sender,balanceA*LPAmt/totalTokens);
            // _transferToken(tokenB, address(this), sender,balanceB*LPAmt/totalTokens);
            IERC20(tokenA).transfer(sender, balanceA*LPAmt/totalTokens);
            IERC20(tokenB).transfer(sender, balanceB*LPAmt/totalTokens);
            return (balanceA*LPAmt/totalTokens, balanceB*LPAmt/totalTokens);
        }
        return (0,0);
    }

    function swap(string memory token , uint amount) public {
        address sender = msg.sender;
        uint newTransferAmount = (amount * 97)/100;
        if ( keccak256(abi.encodePacked(token)) == keccak256(abi.encodePacked("TokenA")) ) {
            uint newAmountA = getTokenABalance() + newTransferAmount;
            uint newAmountB = (getTokenABalance() * getTokenBBalance()) / newAmountA;
            uint transferAmountB = getTokenBBalance() - newAmountB;
            reserve1 = newAmountA;
            reserve2 = newAmountB;
            IERC20(tokenA).transferFrom(sender, address(this), amount);
            IERC20(tokenB).transfer(sender, transferAmountB);
        } else if ( keccak256(abi.encodePacked(token)) == keccak256(abi.encodePacked("TokenB")) ) {
            uint newAmountB = getTokenBBalance() + newTransferAmount;
            uint newAmountA = (getTokenABalance() * getTokenBBalance()) / newAmountB;
            uint transferAmountA = getTokenABalance() - newAmountA;
            reserve1 = newAmountA;
            reserve2 = newAmountB;
            IERC20(tokenA).transfer(sender, transferAmountA);
            IERC20(tokenB).transferFrom(sender, address(this), amount);
        } else {
            return;
        }   
    }
}