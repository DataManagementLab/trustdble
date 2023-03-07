const DatabasesController = require('./databasesController');
const DataContractsController = require('./datacontractsController');
const SqlController = require('./sqlController');
const ServerController = require('./serverController');

module.exports = (services) => ({
  DatabasesController: new DatabasesController(services.DatabasesService),
  DataContractsController: new DataContractsController(services.DataContractsService),
  SqlController: new SqlController(services.SqlService),
  ServerController: new ServerController(services.ServerService),
});
