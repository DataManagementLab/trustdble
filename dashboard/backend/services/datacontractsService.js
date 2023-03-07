class DataContractsService {
  constructor(datacontractsModel) {
    this.datacontractsModal = datacontractsModel;
  }

  async createDataContract(req) {
    await this.datacontractsModal.createDataContract(req.body.name, req.body.owner, req.body.user,
      req.body.logicPath, req.body.parameters);
    const result = await this.datacontractsModal.getDataContract(req.body.name);
    return result[0];
  }

  encryptDataContract(req) {
    return this.datacontractsModal.encryptDataContract(req.params.dcName, req.body.publicKey);
  }

  addDataContract(req) {
    return this.datacontractsModal.addDataContract(req.body.encryptedDC);
  }

  async callDataContract(req) {
    const result = await this.datacontractsModal.callDataContract(req.params.dcName,
      req.body.parameterList);
    if (Object.values(result[0])[0] === 0) {
      const dcResult = await this.datacontractsModal.getDataContractResult(req.params.dcName);
      return dcResult[0];
    }
    throw new Error({ code: 'DC_FAIL', message: 'Data Contract execution failed', sql: `SELECT call_data_contract("${req.params.dcName}","${req.body.parameterList}")` });
  }

  async getEncryptedDataContract(req) {
    const encryptedDC = await this.datacontractsModal.getEncryptedDataContract(req.params.dcName);
    return {
      key: encryptedDC[0].encrypted_key,
      iv: encryptedDC[0].encrypted_iv,
      data_contract: encryptedDC[0].encrypted_data_contract,
    };
  }

  showDataContracts() {
    const results = this.datacontractsModal.getDataContracts();
    return results;
  }

  async getDataContract(req) {
    const result = await this.datacontractsModal.getDataContract(req.params.dcName);
    return result[0];
  }

  showRemoteDataContracts() {
    const results = this.datacontractsModal.getRemoteDataContracts();
    return results;
  }

  async getRemoteDataContract(req) {
    const result = await this.datacontractsModal.getRemoteDataContract(req.params.dcName);
    return result[0];
  }

  deleteDataContract(req) {
    return this.datacontractsModal.deleteDataContract(req.params.dcName);
  }

  deleteRemoteDataContract(req) {
    return this.datacontractsModal.deleteRemoteDataContract(req.params.dcName);
  }

  deleteDataContractResults(req) {
    return this.datacontractsModal.deleteDataContractResults(req.params.dcName);
  }
}

module.exports = DataContractsService;
