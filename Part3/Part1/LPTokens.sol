// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;



contract LPToken {
    string constant name = "LPToken";
    string constant symbol = "LPT";
    address internal DexAddr;
    uint256 internal totalTokens = 0;
    mapping (address => uint256) internal _balance;

    constructor(address DEXAddr){
        DexAddr = DEXAddr;
    }

    function _mint(uint256 _amount, address receiver) internal returns (uint256) {
        require(_amount != 0);
        _balance[receiver]+= _amount;
        totalTokens += _amount;
        return _balance[receiver];
    }

    function balanceOf(address account) public view returns (uint256){
       return _balance[account];
    }

    function generateTokens(uint256 _amount, address receiver) public returns (bool success, uint256) {
        if (msg.sender != DexAddr) return (false, _balance[receiver]);
        return (true, _mint(_amount, receiver));
    }

    function burn(uint256 _amount, address owner) public returns (bool success, uint256) {
        if(msg.sender != DexAddr) return (false, _balance[owner]);
        if (_balance[owner] < _amount) return (false, _balance[owner]);
        _balance[owner] -= _amount;
        totalTokens -= _amount;
        return (true, _balance[owner]);
    }

    function getTotalTokens() public view returns (uint256) {   
        return totalTokens;
    }
}