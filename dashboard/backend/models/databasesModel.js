class DatabasesModel {
  constructor(db) {
    this.db = db;
  }

  showDatabases() {
    return this.db.query('SELECT dbs.SCHEMA_NAME as name, '
                          + 'IFNULL(shared_dbs.blockchain_type, \'LOCAL\') as type '
                          + 'FROM information_schema.schemata as dbs '
                          + 'LEFT JOIN trustdble.shared_databases as shared_dbs '
                          + 'ON dbs.SCHEMA_NAME = shared_dbs.database_name');
  }

  createDatabase(dbName, dbType, shards) {
    return (dbType === 'LOCAL')
      ? this.db.query(`CREATE DATABASE ${dbName}`)
      : this.db.query(`create shared database ${dbName} type=${dbType} shards=${shards}`);
  }

  getDBType(dbName) {
    return this.db.query(`SELECT EXISTS (SELECT 1 FROM trustdble.shared_databases WHERE database_name='${dbName}') AS isShared`);
  }

  deleteDatabase(dbName, isShared) {
    return this.db.query(`drop ${isShared ? 'shared' : ''} database ${dbName}`);
  }

  showTables(dbName) {
    return this.db.query('SELECT TABLE_NAME as name, '
                        + '(CASE WHEN ENGINE=\'BLOCKCHAIN\' THEN true ELSE false END) AS isShared '
                        + 'FROM INFORMATION_SCHEMA.TABLES '
                        + `WHERE TABLE_SCHEMA='${dbName}'`);
  }

  unloadedTables(dbName) {
    return this.db.query(`USE ${dbName}; SELECT DISTINCT SUBSTRING_INDEX(partition_id,'/', 1) AS name `
                        + 'FROM shared_tables '
                        + 'WHERE SUBSTRING_INDEX(partition_id,\'/\', 1) NOT IN (SELECT TABLE_NAME '
                        + 'FROM INFORMATION_SCHEMA.TABLES '
                        + `WHERE TABLE_SCHEMA='${dbName}')`);
  }

  sql(command) {
    return this.db.query(`${command}`);
  }

  addInvite(inviteString) {
    return this.db.query(`add database invite encrypted_invite=${inviteString}`);
  }

  loadSharedDatabase(dbName) {
    return this.db.query(`load shared database ${dbName}`);
  }

  encryptInviteFile(dbName, publicKey) {
    return this.db.query(`CREATE DATABASE INVITE ${dbName} public_key=${publicKey};`);
  }

  getInviteFileContent() {
    return this.db.query('SELECT database_name, encrypted_key, encrypted_iv, encrypted_invite '
                        + 'FROM trustdble.encrypted_invite;');
  }
}

module.exports = DatabasesModel;
