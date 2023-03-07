const { EthereumAdapter } = require('../blockchain-adapters');
const { SQLAdapter } = require('../output-adapters');
const { HexDecoder } = require('../keyvalue-decoders');

/**
 * Monitoring of multiple blockchains
 *
 * @param {object} dbConnection Database connection
 * @param {object} monitorState Dictionary of last processed block numbers of all databases
 */
async function monitorBlockchains(dbConnection, lastProcessedBlocks) {
  if (!(await dbConnection.databaseAlive())) {
    console.error('SQL database unavailable. Please check your configuration setting.');
    return;
  }

  // Initializing database Adapter
  const sqlAdapter = new SQLAdapter(dbConnection);

  // Initializing decoder
  const decoder = new HexDecoder();

  // Get list of databases and NetworkURLs
  const databases = await sqlAdapter.getSharedDatabases();

  // Using Promise.all() with Map function to apply await. This does not work for forEach
  // Reference: https://advancedweb.hu/how-to-use-async-functions-with-array-foreach-in-javascript/
  await Promise.all(databases.map(async (databaseName) => {
    // Get list of table names and contract addresses of a database
    const {
      tableNamesDict,
      keysAndSchemaDict,
    } = await sqlAdapter.getSharedTables(databaseName);

    // Initializing all history_tables
    await sqlAdapter.initHistoryTables(
      databaseName,
      Object.keys(keysAndSchemaDict),
    );

    // Get list of network URLs of ethereum data chains
    const networkURLs = await sqlAdapter.getNetworkURLs(databaseName);

    await Promise.all(networkURLs.map(async ({ dataChainId, networkURL }) => {
      // Initializing blockchain Adapter
      const ethAdapter = new EthereumAdapter(networkURL);

      // Get latest block number from blockchain
      let latestBlockNr = null;
      try {
        latestBlockNr = await ethAdapter.getLatestBlockNr();
      } catch (err) {
        console.error(`Blockchain node at ${networkURL} is not available.`);
        return;
      }

      // Get Last processed block number from existed history tables or from 'lastProcessedBlocks'
      // If history table is empty, it returns -1
      const lastProcessedBlockNr = !(`${databaseName}/${dataChainId}` in lastProcessedBlocks)
        ? await sqlAdapter.getLastProcessedBlockNr(databaseName, dataChainId)
        : lastProcessedBlocks[`${databaseName}/${dataChainId}`];

      let tuples;
      try {
        // Start read history form last processed block number to latest block of a blockchain
        tuples = await ethAdapter.readHistory(lastProcessedBlockNr + 1, latestBlockNr);
      } catch (err) {
        console.error(err);
        return;
      }

      // Decode encoded data received from blockchain
      const decodedTuples = decoder.decode(tuples, tableNamesDict[dataChainId],
        keysAndSchemaDict);

      for (const tuple of decodedTuples) {
        // Get table name using contract address from tuple
        const tableName = tableNamesDict[dataChainId][tuple.contractAddress];

        // Insert tuple using database name and table name
        try {
          await sqlAdapter.writeTuple(databaseName, tableName, dataChainId, tuple);
          console.info(`Successfully wrote transaction from block ${tuple.blockNumber} to history_${tableName}`);
          lastProcessedBlocks[databaseName] = tuple.blockNumber;
        } catch (err) {
          console.error(err);
          return;
        }
      }

      // Update block number so we can start from same blocknumber next time
      lastProcessedBlocks[`${databaseName}/${dataChainId}`] = latestBlockNr;
    }));
  }));
}

module.exports = {
  monitorBlockchains,
};
