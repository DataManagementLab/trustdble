class DatabasesService {
  constructor(databasesModel, tablesModel) {
    this.databasesModel = databasesModel;
    this.tablesModel = tablesModel;
  }

  showDatabases() {
    return this.databasesModel.showDatabases();
  }

  createDatabase(req) {
    return this.databasesModel.createDatabase(req.body.dbName, req.body.dbType, req.body.shards);
  }

  async getDatabase(req) {
    const dbType = await this.databasesModel.getDBType(req.params.dbName);
    const unloadedTables = await this.unloadedTables(req);
    const loadedTables = await this.showTables(req);
    const metaTables = ['meta_table', 'data_chains', 'shared_tables'];
    let numSharedTables = 0;
    let numLocalTables = 0;
    let numHistoryTables = 0;
    let numMetaTable = 0;
    loadedTables.forEach((table) => {
      if (table.isShared) {
        numSharedTables += 1;
      } else {
        numLocalTables += 1;
      }
      if (metaTables.includes(table.name)) {
        numMetaTable += 1;
      }
      if (table.name.startsWith('history_')) {
        numHistoryTables += 1;
      }
    });

    return {
      isShared: Boolean(dbType[0].isShared),
      numTables: loadedTables.length,
      numUnloadedTables: unloadedTables.length,
      numSharedTables,
      numLocalTables,
      numHistoryTables,
      numMetaTable,
    };
  }

  async deleteDatabase(req) {
    const dbType = await this.databasesModel.getDBType(req.params.dbName);
    return this.databasesModel.deleteDatabase(req.params.dbName, dbType[0].isShared);
  }

  async showTables(req) {
    const tables = await this.databasesModel.showTables(req.params.dbName);
    return tables.map((table) => {
      table.isShared = Boolean(table.isShared);
      return table;
    });
  }

  async unloadedTables(req) {
    const dbType = await this.databasesModel.getDBType(req.params.dbName);
    if (!dbType[0].isShared) {
      return [];
    }
    const results = await this.databasesModel.unloadedTables(req.params.dbName);
    return results[1].map((el) => el.name);
  }

  createTable(req) {
    const qArgument = req.body.columns.map((obj) => `${obj.name} ${obj.type}`.concat(
      obj.isPrimaryKey ? ' primary key' : '',
    ).concat(obj.isNotNull ? ' not null' : '')).reduce((x, y) => x.concat(', ').concat(y));
    return this.tablesModel.createTable(
      req.params.dbName,
      req.body.tableName,
      req.body.isSharedTable,
      qArgument,
    );
  }

  async getTableType(req) {
    const tableType = await this.tablesModel.getTableType(req.params.dbName, req.params.tableName);
    return Boolean(tableType[0].isShared);
  }

  async showColumns(req) {
    const results = await this.tablesModel.showColumns(req.params.dbName, req.params.tableName);
    return results.map((el) => ({
      name: el.Field,
      type: el.Type,
      isPrimaryKey: el.Key === 'PRI',
      isNotNull: el.Null === 'NO',
    }));
  }

  deleteTable(req) {
    return this.tablesModel.deleteTable(req.params.dbName, req.params.tableName);
  }

  updateTuple(req) {
    return this.tablesModel.updateTuple(req.params.dbName, req.params.tableName, req.body.colValues,
      req.body.selector);
  }

  deleteTuple(req) {
    return this.tablesModel.deleteTuple(req.params.dbName, req.params.tableName, req.body.selector);
  }

  async getTuples(req) {
    const isShared = await this.getTableType(req);
    const columns = await this.showColumns(req);
    const results = await this.tablesModel.getTuples(req.params.dbName, req.params.tableName);
    return { isShared, columns, tuples: results[1] };
  }

  insertTuples(req) {
    const chars = { '[': '(', ']': ')' };
    let columns = '';
    let isFirstIteration = true;
    const tuples = [];
    req.body.tuples.forEach((tuple) => {
      if (isFirstIteration) {
        columns = Object.keys(tuple).toString();
        isFirstIteration = false;
      }
      const values = JSON.stringify(Object.values(tuple)).replace(/[[\]]+/g, (c) => chars[c]);
      tuples.push(values);
    });
    return this.tablesModel.insertTuples(
      req.params.dbName,
      req.params.tableName,
      columns,
      tuples.toString(),
    );
  }

  async getTableHistoryStates(req) {
    const results = await this.tablesModel.getTableHistoryStates(
      req.params.dbName,
      req.params.tableName,
    );
    return results.map((el) => el.datetime);
  }

  getTableHistoryAtState(req) {
    return this.tablesModel.getTableHistoryAtState(
      req.params.dbName,
      req.params.tableName,
      req.params.state,
    );
  }

  getTupleHistory(req) {
    return this.tablesModel.getTupleHistory(
      req.params.dbName,
      req.params.tableName,
      req.params.primaryKeyHash,
    );
  }

  getEditorHistory(req) {
    return this.tablesModel.getEditorHistory(
      req.params.dbName,
      req.params.tableName,
      req.params.editor,
    );
  }

  loadSharedTable(req) {
    return this.tablesModel.loadSharedTable(req.params.dbName, req.params.tableName);
  }

  loadSharedTables(req) {
    return this.tablesModel.loadSharedTables(req.params.dbName);
  }

  async createTableInvite(req) {
    await this.tablesModel.createTableInvite(req.params.dbName,
      req.params.tableName, req.body.publicKey);
    const invite = await this.tablesModel.getInviteFileContent();
    return invite[0];
  }

  addTableInvite(req) {
    return this.tablesModel.addTableInvite(req.params.dbName,
      req.body.inviteFileContent);
  }

  async loadSharedDatabase(req) {
    await this.databasesModel.addInvite(JSON.stringify(req.body.inviteFileContent));
    return this.databasesModel.loadSharedDatabase(req.params.dbName);
  }

  async getInviteFileContent(req) {
    await this.databasesModel.encryptInviteFile(req.params.dbName, req.body.publicKey);
    const invite = await this.databasesModel.getInviteFileContent();
    return {
      dbName: invite[0].database_name,
      key: invite[0].encrypted_key,
      iv: invite[0].encrypted_iv,
      invite: invite[0].encrypted_invite,
    };
  }
}

module.exports = DatabasesService;
