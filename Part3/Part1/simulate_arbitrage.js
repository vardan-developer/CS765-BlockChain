const { ethers } = require("hardhat");

const deposit = async (LP, dexInterface, tokenAInterface, tokenBInterface, amountA, amountB, lpTokenInterface) => {
    console.log("A: " + amountA + " " + await tokenAInterface.methods.balanceOf(LP).call());
    if (amountA > (await tokenAInterface.methods.balanceOf(LP).call())) console.log("Panic A");
    console.log("B: " + amountB + " " + await tokenBInterface.methods.balanceOf(LP).call());
    if (amountB > (await tokenBInterface.methods.balanceOf(LP).call())) console.log("Panic B");

    await tokenAInterface.methods.approve(dexInterface.options.address, amountA).send({from : LP});
    await tokenBInterface.methods.approve(dexInterface.options.address, amountB).send({from : LP});
    console.log("Approved By " + LP)
    await dexInterface.methods.depositTokens(ethers.BigNumber.from(String(amountA)), ethers.BigNumber.from(String(amountB))).send({from : LP});
    console.log("LP Token Balance: " + await lpTokenInterface.methods.balanceOf(LP).call());
    console.log("Deposit " + "LP:" + LP + ", amountA:" + amountA + ", amountB:" + amountB);
}

const swap = async (user, dexInterface, tokenAInterface, tokenBInterface, amountIn, tokenIn) => {
    if (tokenIn === 0) {
        await tokenAInterface.methods.approve(dexInterface.options.address, amountIn).send({from : user});
        await dexInterface.methods.swap("TokenA", amountIn).send({from : user});
        console.log("Swap " + "user:" + user + ", TokenA amountIn:" + amountIn);
    } else {
        await tokenBInterface.methods.approve(dexInterface.options.address, amountIn).send({from : user});
        await dexInterface.methods.swap("TokenB", amountIn).send({from : user});
        console.log("Swap " + "user:" + user + ", TokenB amountIn:" + amountIn);
    }
}

const withdraw = async (LP, dexInterface, amount) => {
    await dexInterface.methods.withdrawTokens(ethers.BigNumber.from(String(amount))).send({from : LP});
    console.log("Withdraw " + "LP:" + LP + ", amount:" + amount);
}

const profitableArbitrage = async () => {

    try {
        const MyToken = await ethers.getContractFactory("MyToken");

        const TokenA = await MyToken.deploy("TokenA", "TKA");
        await TokenA.deployed();
        const tokenAAddress = TokenA.address;

        const TokenB = await MyToken.deploy("TokenB", "TKB");
        await TokenB.deployed();
        const tokenBAddress = TokenB.address;

        const DEXA = await ethers.getContractFactory("DEX");
        const dexA = await DEXA.deploy(tokenAAddress, tokenBAddress);
        await dexA.deployed();
        const dexAAddress = dexA.address;

        const DEXB = await ethers.getContractFactory("DEX");
        const dexB = await DEXB.deploy(tokenAAddress, tokenBAddress);
        await dexB.deployed();
        const dexBAddress = dexB.address;

        const LPTokenA = await ethers.getContractFactory("LPToken");
        const lpTokenA = await LPTokenA.deploy(dexAAddress);
        await lpTokenA.deployed();
        const lpTokenAAddress = lpTokenA.address;

        const LPTokenB = await ethers.getContractFactory("LPToken");
        const lpTokenB = await LPTokenB.deploy(dexBAddress);
        await lpTokenB.deployed();
        const lpTokenBAddress = lpTokenB.address;

        const Arbitrage = await ethers.getContractFactory("Arbitrage");
        const arbitrage = await Arbitrage.deploy(dexAAddress, dexBAddress);
        await arbitrage.deployed();
        const arbitrageAddress = arbitrage.address;

        const myTokenMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/MyToken.json'));
        if (!myTokenMetadata) {
            throw new Error("Could not find MyToken.json artifact. Please compile the contract first.");
        }
        const myTokenABI = myTokenMetadata.abi;
        
        const lpMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/LPToken.json'));
        if (!lpMetadata) {
            throw new Error("Could not find LPToken.json artifact. Please compile the contract first.");
        }
        const lpABI = lpMetadata.abi;
        
        const dexMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/DEX.json'));
        if (!dexMetadata) {
            throw new Error("Could not find DEX.json artifact. Please compile the contract first.");
        }
        const dexABI = dexMetadata.abi;

        const arbitrageMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/Arbitrage.json'));
        if (!arbitrageMetadata) {
            throw new Error("Could not find Arbitrage.json artifact. Please compile the contract first.");
        }
        const arbitrageABI = arbitrageMetadata.abi;
        
        if (!dexAAddress || !dexBAddress || !lpTokenAAddress || !lpTokenBAddress || !arbitrageAddress || !web3.utils.isAddress(dexAAddress) || !web3.utils.isAddress(dexBAddress) || !web3.utils.isAddress(lpTokenAAddress) || !web3.utils.isAddress(lpTokenBAddress) || !web3.utils.isAddress(arbitrageAddress)) {
            throw new Error("Please enter a valid contract address");
        }

        const dexAInterface = new web3.eth.Contract(dexABI, dexAAddress);
        const dexBInterface = new web3.eth.Contract(dexABI, dexBAddress);
        const lpTokenAInterface = new web3.eth.Contract(lpABI, lpTokenAAddress);
        const lpTokenBInterface = new web3.eth.Contract(lpABI, lpTokenBAddress);
        const arbitrageInterface = new web3.eth.Contract(arbitrageABI, arbitrageAddress);
        const tokenAInterface = new web3.eth.Contract(myTokenABI, tokenAAddress);
        const tokenBInterface = new web3.eth.Contract(myTokenABI, tokenBAddress);


        console.log("TokenA: " + tokenAAddress);
        console.log("TokenB: " + tokenBAddress);
        console.log("DEXA: " + dexAAddress);
        console.log("DEXB: " + dexBAddress);
        console.log("LPTokenA: " + lpTokenAAddress);
        console.log("LPTokenB: " + lpTokenBAddress);
        console.log("Arbitrage: " + arbitrageAddress);

        
        const accounts = await web3.eth.getAccounts();
        const owner = accounts[0];
        const lp_users = accounts.slice(1, 6);
        const swap_users = accounts.slice(6, 14);

        await dexAInterface.methods.setLPTokenAddress(lpTokenAAddress).send({from : owner});
        await dexBInterface.methods.setLPTokenAddress(lpTokenBAddress).send({from : owner});
        
        for (const user of swap_users) {
            await tokenAInterface.methods.transfer(user, ethers.utils.parseEther("1000")).send({from : owner});
            await tokenBInterface.methods.transfer(user, ethers.utils.parseEther("1000")).send({from : owner});
        }
        
        for (const LP of lp_users) {
            await tokenAInterface.methods.transfer(LP, ethers.utils.parseEther("10000")).send({from : owner});
            await tokenBInterface.methods.transfer(LP, ethers.utils.parseEther("10000")).send({from : owner});
        }

        await tokenAInterface.methods.transfer(arbitrageAddress, ethers.utils.parseEther("1000")).send({from : owner});
        await tokenBInterface.methods.transfer(arbitrageAddress, ethers.utils.parseEther("1000")).send({from : owner});


        for (const user of lp_users) {
            await deposit(user, dexAInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("1000"), ethers.utils.parseEther("1000"), lpTokenAInterface);
            await deposit(user, dexBInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("500"), ethers.utils.parseEther("1000"), lpTokenBInterface);
        }

        // for (const user of swap_users) {
        //     await swap(user, dexAInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("100"), 0);
        //     await swap(user, dexBInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("50"), 1);
        // }

        const data = await arbitrageInterface.methods.arbitrage().send({from: owner});
        console.log(data.events)
        // const [amountTradedA, amountTradedB, message] = Object.values(data);
        // console.log("Arbitrage: " + amountTradedA + " " + amountTradedB + " " + message);
    } catch (error) {
        console.error("Error: " + error);
    }
}

const unprofitableArbitrage = async () => {

    try {
        const MyToken = await ethers.getContractFactory("MyToken");

        const TokenA = await MyToken.deploy("TokenA", "TKA");
        await TokenA.deployed();
        const tokenAAddress = TokenA.address;

        const TokenB = await MyToken.deploy("TokenB", "TKB");
        await TokenB.deployed();
        const tokenBAddress = TokenB.address;

        const DEXA = await ethers.getContractFactory("DEX");
        const dexA = await DEXA.deploy(tokenAAddress, tokenBAddress);
        await dexA.deployed();
        const dexAAddress = dexA.address;

        const DEXB = await ethers.getContractFactory("DEX");
        const dexB = await DEXB.deploy(tokenBAddress, tokenAAddress);
        await dexB.deployed();
        const dexBAddress = dexB.address;

        const LPTokenA = await ethers.getContractFactory("LPToken");
        const lpTokenA = await LPTokenA.deploy(dexAAddress);
        await lpTokenA.deployed();
        const lpTokenAAddress = lpTokenA.address;

        const LPTokenB = await ethers.getContractFactory("LPToken");
        const lpTokenB = await LPTokenB.deploy(dexBAddress);
        await lpTokenB.deployed();
        const lpTokenBAddress = lpTokenB.address;

        const Arbitrage = await ethers.getContractFactory("Arbitrage");
        const arbitrage = await Arbitrage.deploy(dexAAddress, dexBAddress);
        await arbitrage.deployed();
        const arbitrageAddress = arbitrage.address;

        const myTokenMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/MyToken.json'));
        if (!myTokenMetadata) {
            throw new Error("Could not find MyToken.json artifact. Please compile the contract first.");
        }
        const myTokenABI = myTokenMetadata.abi;
        
        const lpMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/LPToken.json'));
        if (!lpMetadata) {
            throw new Error("Could not find LPToken.json artifact. Please compile the contract first.");
        }
        const lpABI = lpMetadata.abi;
        
        const dexMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/DEX.json'));
        if (!dexMetadata) {
            throw new Error("Could not find DEX.json artifact. Please compile the contract first.");
        }
        const dexABI = dexMetadata.abi;

        const arbitrageMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/Arbitrage.json'));
        if (!arbitrageMetadata) {
            throw new Error("Could not find Arbitrage.json artifact. Please compile the contract first.");
        }
        const arbitrageABI = arbitrageMetadata.abi;
        
        if (!dexAAddress || !dexBAddress || !lpTokenAAddress || !lpTokenBAddress || !arbitrageAddress || !web3.utils.isAddress(dexAAddress) || !web3.utils.isAddress(dexBAddress) || !web3.utils.isAddress(lpTokenAAddress) || !web3.utils.isAddress(lpTokenBAddress) || !web3.utils.isAddress(arbitrageAddress)) {
            throw new Error("Please enter a valid contract address");
        }

        const dexAInterface = new web3.eth.Contract(dexABI, dexAAddress);
        const dexBInterface = new web3.eth.Contract(dexABI, dexBAddress);
        const lpTokenAInterface = new web3.eth.Contract(lpABI, lpTokenAAddress);
        const lpTokenBInterface = new web3.eth.Contract(lpABI, lpTokenBAddress);
        const arbitrageInterface = new web3.eth.Contract(arbitrageABI, arbitrageAddress);
        const tokenAInterface = new web3.eth.Contract(myTokenABI, tokenAAddress);
        const tokenBInterface = new web3.eth.Contract(myTokenABI, tokenBAddress);

        const accounts = await web3.eth.getAccounts();
        const owner = accounts[0];
        const lp_users = accounts.slice(1, 6);
        const swap_users = accounts.slice(6, 14);
        
        for (const user of lp_users) {
            await tokenAInterface.methods.transfer(user, ethers.utils.parseEther("1000")).send({from : owner});
            await tokenBInterface.methods.transfer(user, ethers.utils.parseEther("1000")).send({from : owner});
        }
        
        for (const LP of lp_users) {
            await tokenAInterface.methods.transfer(LP, ethers.utils.parseEther("1000")).send({from : owner});
            await tokenBInterface.methods.transfer(LP, ethers.utils.parseEther("1000")).send({from : owner});
        }

        await tokenAInterface.methods.transfer(arbitrageAddress, ethers.utils.parseEther("1000")).send({from : owner});
        await tokenBInterface.methods.transfer(arbitrageAddress, ethers.utils.parseEther("1000")).send({from : owner});

        for (const user of lp_users) {
            await deposit(user, dexAInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("1000"), ethers.utils.parseEther("1000"), lpTokenAInterface);
            await deposit(user, dexBInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("1000"), ethers.utils.parseEther("1000"), lpTokenBInterface);
        }

        for (const user of swap_users) {
            await swap(user, dexAInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("100"), 0);
            await swap(user, dexBInterface, tokenAInterface, tokenBInterface, ethers.utils.parseEther("50"), 1);
        }

        const data = await arbitrageInterface.methods.arbitrage().send();
        const [amountTradedA, amountTradedB, message] = Object.values(data);
        console.log("Arbitrage: " + amountTradedA + " " + amountTradedB + " " + message);
    } catch (error) {
        console.error("Error: " + error);
    }
}

profitableArbitrage();
// unprofitableArbitrage();