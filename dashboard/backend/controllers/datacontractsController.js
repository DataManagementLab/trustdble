class DataContractsController {
  constructor(datacontractsService) {
    this.datacontractsService = datacontractsService;
  }

  async createDataContract(req, res) {
    try {
      const dataContract = await this.datacontractsService.createDataContract(req);
      res.status(200).json(dataContract);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async encryptDataContract(req, res) {
    try {
      await this.datacontractsService.encryptDataContract(req);
      const encryptedDC = await this.datacontractsService.getEncryptedDataContract(req);
      res.status(200).json({ encryptedDC });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async addDataContract(req, res) {
    try {
      await this.datacontractsService.addDataContract(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async callDataContract(req, res) {
    try {
      const result = await this.datacontractsService.callDataContract(req);
      res.status(200).json(result);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getEncryptedDataContract(req, res) {
    try {
      const encryptedDC = await this.datacontractsService.getEncryptedDataContract(req);
      res.status(200).json({ encryptedDC });
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getDataContract(req, res) {
    try {
      const dataContract = await this.datacontractsService.getDataContract(req);
      res.status(200).json(dataContract);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async showDataContracts(req, res) {
    try {
      const dataContracts = await this.datacontractsService.showDataContracts();
      res.status(200).json(dataContracts);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async getRemoteDataContract(req, res) {
    try {
      const dataContract = await this.datacontractsService.getRemoteDataContract(req);
      res.status(200).json(dataContract);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async showRemoteDataContracts(req, res) {
    try {
      const dataContracts = await this.datacontractsService.showRemoteDataContracts();
      res.status(200).json(dataContracts);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteDataContract(req, res) {
    try {
      await this.datacontractsService.deleteDataContract(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteRemoteDataContract(req, res) {
    try {
      await this.datacontractsService.deleteRemoteDataContract(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }

  async deleteDataContractResults(req, res) {
    try {
      await this.datacontractsService.deleteDataContractResults(req);
      res.status(200).send();
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }
}

module.exports = DataContractsController;
