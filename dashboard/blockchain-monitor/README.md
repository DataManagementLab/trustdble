## TrustDBle Blockchain Monitor
We use JavaScript to communicate with the mysql server and Ethereum data node

### Set Up
* install node
* install npm

### How to run?
* Change directory:
    ```cd blockchain-monitor/```
* Install all node packages with: 
    ```npm install```
* Run it with: 
    * Simple start with ```npm start```
    * Start process and restart when file are changed ```npm run dev```
    * Start process and auto restart after crash or file changes```npm run auto_restart```

### Monitor functionality (pseudo code)
```
sqlAdapter = sqlAdapter(dbConnection)
decoder = HexDecoder()
databasesDict = sqlAdapter.getSharedDatabases()
for database, networkURL in databasesDict:
	tableNamesDict, tableSchemasDict = sqlAdapter.getTablesInfo(database)
	sqlAdapter.initTables(database, tableNamesDict.values)
	ethAdapter = ethereumAdapter(networkURL)
	lastBlock = sqlAdapter.getLastBlock(database)
	latestBlock = ethAdapter.getMaxBlock()
	tuples = ethAdapter.readHistory(lastBlock, latestBlock)
	decodedTuples = decoder.decode(tuples, tableSchemasDict)
	for tuple in decodedTuples:
		table = tableNamesDict[tuple.address]
		sqlAdapter.writeTuple(database, table, tuple)
```