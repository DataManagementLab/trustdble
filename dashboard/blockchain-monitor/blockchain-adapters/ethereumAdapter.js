const Web3 = require('web3');
const { BlockchainAdapter } = require('./blockchainAdapter');

// The hash of the put method signature of trustdble ethereum contract
const ETHEREUM_METHOD_HASH_PUT = '0xdb82ecc3';

// The hash of the remove method signature of trustdble ethereum contract
const ETHEREUM_METHOD_HASH_REMOVE = '0x95bc2673';
/**
 * Read history of blocks and transactions from the Ethereum blockchain
 *
 * The contract is expected to follow a Key-Value-Store semantic and define a "PUT" function
 * that is used for writing to the store.
 */
class EthereumAdapter extends BlockchainAdapter {
  constructor(networkURL) {
    super();
    this.provider = new Web3(new Web3.providers.HttpProvider(networkURL));
  }

  /**
  * Get current/latest block number of Ethereum blockchain
  *
  * @returns {int} Current/latest block number
  */
  async getLatestBlockNr() {
    return this.provider.eth.getBlockNumber();
  }

  /**
  * Read the history form ethereum blockchain between last processed block and latest block
  *
  * @param {int} lastProcessedBlock Last processed block
  * @param {int} latestBlock Latest block
  * @returns {Promise<*[object]>} list of tuples where each tuple represents the data of one row.
  */
  async readHistory(fromBlockNr, toBlockNr) {
    const history = [];
    for (let i = fromBlockNr; i <= toBlockNr; i += 1) {
      const block = await this.provider.eth.getBlock(i);
      if (block.transactions.length > 0) {
        const tuplesInBlock = await this._extractTuplesFromBlock(block);
        history.push(...tuplesInBlock);
      }
    }
    return history;
  }

  /**
  * Process a block of the blockchain and scan it for the transactions in which we are interested
  *
  * @param {object} block Ethereum block as defined by Web3js
  * @returns {*[object]} A list of tuples that were parsed from the transactions in this block
  * @private
  */
  async _extractTuplesFromBlock(block) {
    const tuplesInBlock = [];

    for (const txHash of block.transactions) {
      const tx = await this.provider.eth.getTransaction(txHash);
      // First 10 characters of Transaction's input string represent the signature
      // of a function from smart contract
      const ethereumMethodHash = tx.to ? tx.input.substring(0, 10) : '';

      // Check if transaction is related to 'put' or 'remove' function
      if (ethereumMethodHash === ETHEREUM_METHOD_HASH_PUT
        || ethereumMethodHash === ETHEREUM_METHOD_HASH_REMOVE) {
        // Get required information from block and transaction
        // We also add contractAddress which help to insert tuple values in respective table.
        const tuple = {
          contractAddress: tx.to,
          asOf: new Date(block.timestamp * 1000),
          editor: tx.from,
          txHash,
          txType: ethereumMethodHash === ETHEREUM_METHOD_HASH_PUT ? 'PUT' : 'REMOVE',
          blockNumber: block.number,
          data: tx.input.substring(10),
        };
        tuplesInBlock.push(tuple);
      } else if (ethereumMethodHash) {
        // Skip block if there a no relevant function Signature of smart contract in transaction
        console.info(`Skipping transaction of block ${block.number}. No relevant function Signature of smart contract in transaction`);
      } else {
        // Skip block if there is a no call relevant Contract Address
        console.info(`Skipping transaction from block ${block.number}. No relevant Contract Address of transactions`);
      }
    }
    return tuplesInBlock;
  }
}

module.exports = { EthereumAdapter };
