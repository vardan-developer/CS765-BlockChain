// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";

contract MyToken is ERC20 {
    uint constant _total_supply = 100*(10**18);
    constructor() ERC20("TokenA", "TKA") {
        _mint(msg.sender, _total_supply);
    }
}
