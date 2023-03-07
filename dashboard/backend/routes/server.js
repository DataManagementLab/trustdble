const { Router } = require('express');

module.exports = (serverController) => {
  const router = Router();

  /**
   * @swagger
   * /api/sqlServer/status:
   *  get:
   *    summary: Get availability status of SQL server
   *    tags:
   *      - Server
   *    responses:
   *      200:
   *        description: OK
   *      500:
   *        description: When sql sever is not available
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: connect ECONNREFUSED 127.0.0.1:3305
   *                code:
   *                  type: string
   *                  example: ECONNREFUSED
   *                errno:
   *                  type: string
   *                  example: ECONNREFUSED
   */
  router.get('/status', (req, res) => serverController.getServerStatus(req, res));

  return router;
};
