/**
 * Interface of a general BlockchainAdapter.
 *
 * Each adapter that implements reading data from a specific blockchain
 * should inherit from this class and implements its functions.
 *
 * We are not passing any parameter here because javascript does not have function overloading
 * if two functions are defined with same name then
 * the last defined function will overwrite the former function.
 */
class BlockchainAdapter {
  constructor() {
    if (new.target === BlockchainAdapter) {
      throw new Error('BlockchainAdapter is an abstract class.');
    }
  }

  // The only necessary function is readHistory from last processed block to latest block which is
  // expected to return the whole history of a table that are present in the blockchain.
  readHistory() {
    throw new Error(`Method 'readHistory' is not implemented for class ${this.constructor.name}`);
  }

  // Get current/latest block number from a blockchain
  getLatestBlockNr() {
    throw new Error(`Method 'getLatestBlockNr' is not implemented for class ${this.constructor.name}`);
  }
}

module.exports = { BlockchainAdapter };
