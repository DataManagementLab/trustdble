const Web3 = require('web3');
const fs = require('fs');

var myArgs = process.argv;

// coinbasePrivateKeyJulian
const FROM_ACCOUNT=myArgs[2];
const COMPILED_CONTRACT = myArgs[3];

// Connect to Ethereum node
const web3 = new Web3(myArgs[4]);

const contractFile = JSON.parse(fs.readFileSync(COMPILED_CONTRACT, "utf-8"));
const contract = new web3.eth.Contract(contractFile.abi);

contract.deploy({data: contractFile.bytecode}).send({from: FROM_ACCOUNT, gas: 2000000}).then((newContract) => {
    const address = newContract.options.address;
    if(address) {
        console.log(address);
    } else {
        console.error("Deployment failed!");
    }
});