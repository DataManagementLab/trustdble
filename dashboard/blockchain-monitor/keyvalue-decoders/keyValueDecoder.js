/**
 * Interface for the KeyValueDecoder that needs to be passed to the EthereumAdapter
 * for parsing the actual rows stored in the blockchain.
 *
 * Each KeyValueDecoder should inherit this class and define the four required functions
 * We are not passing any parameter here because javascript does not have function overloading
 * if two functions are defined with same name then
 * the last defined function will overwrite the former function.
 */
class KeyValueDecoder {
  constructor() {
    if (new.target === KeyValueDecoder) {
      throw new Error('KeyValueDecoder is an abstract class.');
    }
  }

  // decode: a interface for decoding list of tuples from blockchain adapter
  decode() {
    throw new Error(`Method 'decode' is not implemented for class ${this.constructor.name}`);
  }
}

module.exports = { KeyValueDecoder };
