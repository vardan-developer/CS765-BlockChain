const analyze =  async() => {
    try {
        const dexMetadata = JSON.parse(await remix.call('fileManager', 'getFile', 'browser/contracts/artifacts/DEX.json'));
        if (!dexMetadata) {
            throw new Error("Could not find DEX.json artifact. Please compile the contract first.");
        }
        const dexABI = dexMetadata.abi;

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
        
        const accounts = await web3.eth.getAccounts();
        const dexOwner = accounts[0];
        
        const dexAddress = "0x7EF2e0048f5bAeDe046f6BF797943daF4ED8CB47";
        const tokenAAddress = "0xf8e81D47203A594245E36C48e151709F0C19fBe8"
        const tokenBAddress = "0xD7ACd2a9FD159E69Bb102A1ca21C9a3e3A5F771B"
        const lpTokenAddress = "0xDA0bab807633f07f013f94DD0E6A4F96F8742B53"
        
        if (!dexAddress || !tokenAAddress || !tokenBAddress || !lpTokenAddress || !web3.utils.isAddress(dexAddress) || !web3.utils.isAddress(tokenAAddress) || !web3.utils.isAddress(tokenBAddress) || !web3.utils.isAddress(lpTokenAddress)) {
            throw new Error("Please enter a valid contract address");
        }

        const dexInterface = new web3.eth.Contract(dexABI, dexAddress);
        const tokenAInterface = new web3.eth.Contract(myTokenABI, tokenAAddress);
        const tokenBInterface = new web3.eth.Contract(myTokenABI, tokenBAddress);
        const lpTokenInterface = new web3.eth.Contract(lpABI, lpTokenAddress);

        const balanceADexOwner = await tokenAInterface.methods.balanceOf("0x5B38Da6a701c568545dCfcB03FcB875f56beddC4").call();
        console.log(balanceADexOwner);

        await tokenAInterface.methods.transfer("0xAb8483F64d9C6d1EcF9b849Ae677dD3315835cb2", 10000000).send({from : "0x5B38Da6a701c568545dCfcB03FcB875f56beddC4"});

        console.log(await tokenAInterface.methods.balanceOf("0xAb8483F64d9C6d1EcF9b849Ae677dD3315835cb2").call());
        
        



    } catch (err){
        console.error(err)
    } finally{
    process.exit(0);
    }
}
analyze();