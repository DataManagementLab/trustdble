class ServerServices {
  constructor(serverModel) {
    this.serverModel = serverModel;
  }

  getServerStatus() {
    return this.serverModel.getServerStatus();
  }
}

module.exports = ServerServices;
