class DataContractsModel {
  constructor(db) {
    this.db = db;
  }

  createDataContract(name, owner, user, logicPath, parameters) {
    return this.db.query(`CREATE DATA CONTRACT name=${name} owner=${owner} user=${user} logic_path=${logicPath} parameters=${parameters}`);
  }

  encryptDataContract(name, publicKey) {
    return this.db.query(`ENCRYPT DATA CONTRACT name=${name} public_key=${publicKey}`);
  }

  addDataContract(encryptedDC) {
    return this.db.query(`ADD DATA CONTRACT encrypted_data_contract=${JSON.stringify(encryptedDC)}`);
  }

  callDataContract(name, parameterlist) {
    return this.db.query(`SELECT call_data_contract("${name}","${parameterlist}");`);
  }

  getDataContractResult(name) {
    return this.db.query(`SELECT * FROM trustdble.data_contracts_result WHERE data_contract_name='${name}';`);
  }

  getEncryptedDataContract(name) {
    return this.db.query(`SELECT encrypted_key, encrypted_iv, encrypted_data_contract FROM trustdble.encrypted_data_contracts WHERE name='${name}'`);
  }

  getRemoteDataContracts() {
    return this.db.query('SELECT name, owner, user, logic, parameters, server_address FROM trustdble.remote_data_contracts');
  }

  getRemoteDataContract(name) {
    return this.db.query(`SELECT * FROM trustdble.remote_data_contracts WHERE name='${name}'`);
  }

  getDataContracts() {
    return this.db.query('SELECT * FROM trustdble.data_contracts');
  }

  getDataContract(dcName) {
    return this.db.query(`SELECT * FROM trustdble.data_contracts WHERE name='${dcName}'`);
  }

  deleteDataContract(name) {
    return this.db.query(`DELETE FROM trustdble.data_contracts WHERE name='${name}'; DELETE FROM trustdble.encrypted_data_contracts WHERE name='${name}'`);
  }

  deleteRemoteDataContract(name) {
    return this.db.query(`DELETE FROM trustdble.remote_data_contracts WHERE name='${name}';`);
  }

  deleteDataContractResults(name) {
    return this.db.query(`DELETE FROM trustdble.data_contracts_result WHERE data_contract_name='${name}';`);
  }
}

module.exports = DataContractsModel;
