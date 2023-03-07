const DatabasesService = require('./databasesService');
const DataContractsService = require('./datacontractsService');
const SqlService = require('./sqlService');
const ServerService = require('./serverService');

module.exports = (models) => ({
  DatabasesService: new DatabasesService(models.DatabasesModel, models.TablesModel),
  DataContractsService: new DataContractsService(models.DataContractsModel),
  SqlService: new SqlService(models.DatabasesModel),
  ServerService: new ServerService(models.ServerModel),
});
