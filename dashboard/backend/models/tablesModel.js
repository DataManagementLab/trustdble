const PREFIX_HISTORY_TABLE_NAME = 'history_';

const HISTORY_TABLE_COLS = {
  BLOCK_NUMBER: 'blockNumber',
  EDITOR: 'editor',
  PRIMARY_KEY_HASH: 'primaryKeyHash',
  TIMESTAMP: 'datetime',
};

class TablesModel {
  constructor(db) {
    this.db = db;
  }

  createTable(dbName, tableName, isSharedTable, qArgument) {
    return this.db.query(`USE ${dbName}; create ${isSharedTable ? 'shared' : ''} table ${tableName} (${qArgument})`);
  }

  getTableType(dbName, tableName) {
    return this.db.query('SELECT (CASE WHEN ENGINE=\'BLOCKCHAIN\' THEN TRUE ELSE FALSE END) AS isShared '
                        + 'FROM INFORMATION_SCHEMA.TABLES '
                        + `WHERE TABLE_SCHEMA='${dbName}' AND TABLE_NAME='${tableName}'`);
  }

  showColumns(dbName, tableName) {
    return this.db.query(`SHOW COLUMNS FROM ${dbName}.${tableName}`);
  }

  deleteTable(dbName, tableName) {
    return this.db.query(`DROP TABLE ${dbName}.${tableName}`);
  }

  getTuples(dbName, tableName) {
    return this.db.query(`USE ${dbName}; SELECT * FROM ${tableName}`);
  }

  insertTuples(dbName, tableName, columns, values) {
    return this.db.query(`USE ${dbName}; INSERT INTO ${tableName} (${columns}) VALUES ${values}`);
  }

  getTableHistoryStates(dbName, tableName) {
    return this.db.query(`SELECT DISTINCT(${HISTORY_TABLE_COLS.TIMESTAMP}) AS ${HISTORY_TABLE_COLS.TIMESTAMP} `
                        + `FROM ${dbName}.${PREFIX_HISTORY_TABLE_NAME}${tableName} `
                        + `ORDER BY ${HISTORY_TABLE_COLS.TIMESTAMP}`);
  }

  getTableHistoryAtState(dbName, tableName, state) {
    return this.db.query(
      `SELECT a.* FROM ${dbName}.${PREFIX_HISTORY_TABLE_NAME}${tableName} a INNER JOIN (SELECT ${HISTORY_TABLE_COLS.PRIMARY_KEY_HASH},`
            + ` MAX(${HISTORY_TABLE_COLS.TIMESTAMP}) AS ${HISTORY_TABLE_COLS.TIMESTAMP} `
            + `FROM ${dbName}.${PREFIX_HISTORY_TABLE_NAME}${tableName} `
            + `WHERE ${HISTORY_TABLE_COLS.TIMESTAMP} <= '${state}' `
            + `GROUP BY ${HISTORY_TABLE_COLS.PRIMARY_KEY_HASH} `
            + `) b ON a.${HISTORY_TABLE_COLS.PRIMARY_KEY_HASH} = b.${HISTORY_TABLE_COLS.PRIMARY_KEY_HASH} AND`
            + ` a.${HISTORY_TABLE_COLS.TIMESTAMP} = b.${HISTORY_TABLE_COLS.TIMESTAMP} `
            + `ORDER BY a.${HISTORY_TABLE_COLS.TIMESTAMP} `,
    );
  }

  getTupleHistory(dbName, tableName, primaryKeyHash) {
    return this.db.query(`SELECT * FROM ${dbName}.${PREFIX_HISTORY_TABLE_NAME}${tableName} a WHERE a.${HISTORY_TABLE_COLS.PRIMARY_KEY_HASH} = '${primaryKeyHash}'`);
  }

  getEditorHistory(dbName, tableName, editor) {
    return this.db.query(
      ' SELECT * '
            + `FROM ${dbName}.${PREFIX_HISTORY_TABLE_NAME}${tableName} `
            + `WHERE ${HISTORY_TABLE_COLS.EDITOR} = '${editor}' `
            + `ORDER BY ${HISTORY_TABLE_COLS.BLOCK_NUMBER}`,
    );
  }

  loadSharedTable(dbName, tableName) {
    return this.db.query(`USE ${dbName}; load shared table ${tableName}`);
  }

  loadSharedTables(dbName) {
    return this.db.query(`USE ${dbName}; load shared tables`);
  }

  async createTableInvite(dbName, tableName, publicKey) {
    await this.db.query(`USE ${dbName};`);
    return this.db.query(`CREATE TABLE INVITE ${tableName} public_key=${publicKey};`);
  }

  getInviteFileContent() {
    return this.db.query('SELECT table_name as tableName, encrypted_invite as invite '
                        + 'FROM trustdble.encrypted_table_invite;');
  }

  addTableInvite(dbName, encryptedInvite) {
    return this.db.query(`USE ${dbName}; ADD TABLE INVITE encrypted_invite=${JSON.stringify(encryptedInvite)}`);
  }

  updateTuple(dbName, tableName, colValues, selector) {
    return this.db.query(`USE ${dbName}; UPDATE ${tableName} SET ${colValues} WHERE ${selector}`);
  }

  deleteTuple(dbName, tableName, selector) {
    return this.db.query(`USE ${dbName}; DELETE FROM ${tableName} WHERE ${selector}`);
  }
}

module.exports = TablesModel;
