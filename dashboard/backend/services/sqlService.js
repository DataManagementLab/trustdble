class SqlService {
  constructor(databasesModel) {
    this.databasesModel = databasesModel;
  }

  sql(req) {
    return this.databasesModel.sql(req.body.sqlCommand);
  }
}

module.exports = SqlService;
