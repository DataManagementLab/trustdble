class DatabasesController {
  constructor(databasesService) {
    this.databasesService = databasesService;
  }

  async showDatabases(req, res) {
    try {
      const results = await this.databasesService.showDatabases();
      res.status(200).json({ databases: results });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async createDatabase(req, res) {
    try {
      await this.databasesService.createDatabase(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getDatabase(req, res) {
    try {
      const database = await this.databasesService.getDatabase(req);
      res.status(200).json(database);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteDatabase(req, res) {
    try {
      await this.databasesService.deleteDatabase(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async showTables(req, res) {
    try {
      const results = await this.databasesService.showTables(req);
      const unloadedTables = await this.databasesService.unloadedTables(req);
      res.status(200).json({ tables: results, unloaded_tables: unloadedTables });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async createTable(req, res) {
    try {
      await this.databasesService.createTable(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getTable(req, res) {
    try {
      const isShared = await this.databasesService.getTableType(req);
      const columns = await this.databasesService.showColumns(req);
      res.status(200).json({ isShared, columns });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteTable(req, res) {
    try {
      await this.databasesService.deleteTable(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getTuples(req, res) {
    try {
      const results = await this.databasesService.getTuples(req);
      res.status(200).json(results);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async insertTuples(req, res) {
    try {
      await this.databasesService.insertTuples(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async updateTuple(req, res) {
    try {
      await this.databasesService.updateTuple(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteTuple(req, res) {
    try {
      await this.databasesService.deleteTuple(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getTableHistoryStates(req, res) {
    try {
      const results = await this.databasesService.getTableHistoryStates(req);
      res.status(200).json({ states: results });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getTableHistoryAtState(req, res) {
    try {
      const results = await this.databasesService.getTableHistoryAtState(req);
      res.status(200).json({ tuples: results });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getTupleHistory(req, res) {
    try {
      const results = await this.databasesService.getTupleHistory(req);
      res.status(200).json({ tuple_states: results });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getEditorHistory(req, res) {
    try {
      const results = await this.databasesService.getEditorHistory(req);
      res.status(200).json({ changes: results });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async loadSharedTable(req, res) {
    try {
      await this.databasesService.loadSharedTable(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async loadSharedTables(req, res) {
    try {
      await this.databasesService.loadSharedTables(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async createTableInvite(req, res) {
    try {
      const inviteFileContent = await this.databasesService.createTableInvite(req);
      res.status(200).json({ inviteFileContent });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async addTableInvite(req, res) {
    try {
      await this.databasesService.addTableInvite(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async loadSharedDatabase(req, res) {
    try {
      await this.databasesService.loadSharedDatabase(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getInviteFileContentForDatabase(req, res) {
    try {
      const inviteFileContent = await this.databasesService.getInviteFileContent(req);
      res.status(200).json({ inviteFileContent });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }
}

module.exports = DatabasesController;
