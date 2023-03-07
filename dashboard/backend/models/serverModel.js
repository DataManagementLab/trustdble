class ServerModel {
  constructor(db) {
    this.db = db;
  }

  getServerStatus() {
    return this.db.databaseAlive();
  }
}

module.exports = ServerModel;
