/**
 * OutputAdapter should ensure that the necessary table exists and create it otherwise.
 * Then, we can call writeTuple for each tuple to be written to the DB.
 *
 * We are not passing any parameter here because javascript does not have function overloading
 * if two functions are defined with same name then the last defined function will overwrite.
 */
class OutputAdapter {
  constructor() {
    if (new.target === OutputAdapter) {
      throw new Error('OutputAdapter is an abstract class.');
    }
  }

  // Ensures that all necessary tables exist and creates them if not.
  initHistoryTables() {
    throw new Error(`Method 'initHistoryTables' is not implemented for class ${this.constructor.name}`);
  }

  // Each tuple to be written to the DB.
  writeTuple() {
    throw new Error(`Method 'writeTuple' is not implemented for class ${this.constructor.name}`);
  }

  // To get databases names and their Ethereum data node's network URLs.
  getSharedDatabases() {
    throw new Error(`Method 'getSharedDatabases' is not implemented for class ${this.constructor.name}`);
  }

  // To get List of shared tables and their contract addresses and schemas.
  getSharedTables() {
    throw new Error(`Method 'getSharedTables' is not implemented for class ${this.constructor.name}`);
  }

  // To get a highest processed block number from history tables of a database
  getLastProcessedBlockNr() {
    throw new Error(`Method 'getLastProcessedBlockNr' is not implemented for class ${this.constructor.name}`);
  }
}

module.exports = { OutputAdapter };
