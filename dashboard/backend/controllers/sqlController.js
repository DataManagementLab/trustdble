class SqlController {
  constructor(sqlService) {
    this.sqlService = sqlService;
  }

  async sql(req, res) {
    try {
      const results = await this.sqlService.sql(req);
      res.status(200).json(results);
    } catch (err) {
      if (err.code === 'ECONNREFUSED') res.status(500).json(err); else res.status(400).json(err);
    }
  }
}

module.exports = SqlController;
