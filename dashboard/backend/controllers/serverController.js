class ServerController {
  constructor(serverService) {
    this.serverService = serverService;
  }

  async getServerStatus(req, res) {
    if (await this.serverService.getServerStatus()) {
      res.status(200).send();
    } else {
      res.status(500).send();
    }
  }
}

module.exports = ServerController;
