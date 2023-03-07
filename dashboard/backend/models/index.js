const DatabasesModel = require('./databasesModel');
const DataContractsModel = require('./datacontractsModel');
const TablesModel = require('./tablesModel');
const ServerModel = require('./serverModel');

module.exports = (db) => ({
  DatabasesModel: new DatabasesModel(db),
  DataContractsModel: new DataContractsModel(db),
  TablesModel: new TablesModel(db),
  ServerModel: new ServerModel(db),
});
