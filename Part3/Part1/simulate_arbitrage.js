const getInterface = async (contractName, args) => {
    const contract = await ethers.getContractFactory(contractName);
    const instance = await contract.deploy(...args);
    await instance.deployed();
    return instance.interface;
}

const arbitrage = async () => {
    const tokenAInterface = await getInterface("MyToken", ["TokenA", "TA"]);
    const tokenBInterface = await getInterface("MyToken", ["TokenB", "TB"]);
    const lpTokenInterface = await getInterface("LPToken", []);
    const dex1Interface = await getInterface("DEX", []);
    const dex2Interface = await getInterface("DEX", []);
    const arbitrageInterface = await getInterface("Arbitrage", [dex1Interface.address, dex2Interface.address]);

    const accounts = await web3.eth.getAccounts();
    const dexOwner = accounts[0];
}

arbitrage();