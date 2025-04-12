const { ethers } = require("hardhat");

const deposit = async (LP, dexInterface, tokenAInterface, tokenBInterface, amountA, amountB, lpTokenInterface) => {
    await tokenAInterface.methods.approve(dexInterface.options.address, amountA).send({from : LP});
    await tokenBInterface.methods.approve(dexInterface.options.address, amountB).send({from : LP});
    await dexInterface.methods.depositTokens(amountA, amountB).send({from : LP});
    console.log("Deposit " + "LP:" + LP + ", amountA:" + amountA + ", amountB:" + amountB);
    console.log("LP Token Balance: " + await lpTokenInterface.methods.balanceOf(LP).call());
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

const analyze =  async() => {
    try {

        const MyToken = await ethers.getContractFactory("MyToken");

        const TokenA = await MyToken.deploy("TokenA", "TKA");
        await TokenA.deployed();
        const tokenAAddress = TokenA.address;

        const TokenB = await MyToken.deploy("TokenB", "TKB");
        await TokenB.deployed();
        const tokenBAddress = TokenB.address;

        const DEX = await ethers.getContractFactory("DEX");
        const dex = await DEX.deploy(tokenAAddress, tokenBAddress);
        await dex.deployed();
        const dexAddress = dex.address;

        const LPToken = await ethers.getContractFactory("LPToken");
        const lpToken = await LPToken.deploy(dexAddress);
        await lpToken.deployed();
        const lpTokenAddress = lpToken.address;
        
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
        
        if (!dexAddress || !tokenAAddress || !tokenBAddress || !lpTokenAddress || !web3.utils.isAddress(dexAddress) || !web3.utils.isAddress(tokenAAddress) || !web3.utils.isAddress(tokenBAddress) || !web3.utils.isAddress(lpTokenAddress)) {
            throw new Error("Please enter a valid contract address");
        }

        const dexInterface = new web3.eth.Contract(dexABI, dexAddress);
        const tokenAInterface = new web3.eth.Contract(myTokenABI, tokenAAddress);
        const tokenBInterface = new web3.eth.Contract(myTokenABI, tokenBAddress);
        const lpTokenInterface = new web3.eth.Contract(lpABI, lpTokenAddress);

        const accounts = await web3.eth.getAccounts();
        const dexOwner = accounts[0];
        const LPs = accounts.slice(1, 6);
        const users = accounts.slice(6, 14);

        const N = Math.floor(Math.random() * 51) + 50;

        await dexInterface.methods.setLPTokenAddress(lpTokenAddress).send({from : dexOwner});

        // LPToken Owner deposits tokens in DEX
        await deposit(dexOwner, dexInterface, tokenAInterface, tokenBInterface, 100000000, 100000000, lpTokenInterface);

        for (const user of users) {
            await tokenAInterface.methods.transfer(user, 100000000).send({from : dexOwner});
            await tokenBInterface.methods.transfer(user, 100000000).send({from : dexOwner});
        }
        
        for (const LP of LPs) {
            await tokenAInterface.methods.transfer(LP, 100000000).send({from : dexOwner});
            await tokenBInterface.methods.transfer(LP, 100000000).send({from : dexOwner});
        }

        const operationTypes = {"swap": 0, "deposit": 1, "withdraw": 2};

        for (let i = 0; i < N; i++) {
            const operationType = Math.floor(Math.random() * 3);
            
            if (operationType === operationTypes.swap) {
                const user = users[Math.floor(Math.random() * users.length)];
                const tokenIn = Math.floor(Math.random() * 2);

                if (tokenIn === 0) {
                    const tokenAUserBalance = await tokenAInterface.methods.balanceOf(user).call();
                    const tokenADexBalance = await tokenAInterface.methods.balanceOf(dexAddress).call();

                    const amountIn = Math.floor(Math.random() * Math.min(tokenAUserBalance, tokenADexBalance * 0.1));
                    await swap(user, dexInterface, tokenAInterface, tokenBInterface, amountIn, 0);
                } else {
                    const tokenBUserBalance = await tokenBInterface.methods.balanceOf(user).call();
                    const tokenBDexBalance = await tokenBInterface.methods.balanceOf(dexAddress).call();

                    const amountIn = Math.floor(Math.random() * Math.min(tokenBUserBalance, tokenBDexBalance * 0.1));
                    await swap(user, dexInterface, tokenAInterface, tokenBInterface, amountIn, 1);
                }

            } else if (operationType === operationTypes.deposit) {
                const LP = LPs[Math.floor(Math.random() * LPs.length)];
                const tokenADexBalance = await tokenAInterface.methods.balanceOf(dexAddress).call();
                const tokenBDexBalance = await tokenBInterface.methods.balanceOf(dexAddress).call();

                const tokenALPBalance = await tokenAInterface.methods.balanceOf(LP).call();
                const tokenBLPBalance = await tokenBInterface.methods.balanceOf(LP).call();

                const tokenAUpperBound = Math.min(tokenALPBalance, Math.floor((tokenADexBalance * tokenBLPBalance) / tokenBDexBalance));

                const amountA = Math.floor(Math.random() * tokenAUpperBound);
                const amountB = Math.floor((tokenBDexBalance * amountA) / tokenADexBalance);

                await deposit(LP, dexInterface, tokenAInterface, tokenBInterface, amountA, amountB, lpTokenInterface);

            } else if (operationType === operationTypes.withdraw) {
                const LPsWithBalance = await Promise.all(LPs.map(async lp => {
                    const balance = await lpTokenInterface.methods.balanceOf(lp).call();
                    return balance > 0 ? lp : null;
                }));
                const validLPs = LPsWithBalance.filter(lp => lp !== null);
            
                if (validLPs.length === 0) {
                    i--;
                    continue;
                }

                const LP = validLPs[Math.floor(Math.random() * validLPs.length)];
                
                const LPTokenBalance = await lpTokenInterface.methods.balanceOf(LP).call();
                const amount = Math.floor(Math.random() * LPTokenBalance);
                await withdraw(LP, dexInterface, amount);
            }
                
        }

    } catch (err){
        console.error(err)
    } finally{
        process.exit(0);
    }
}
analyze();