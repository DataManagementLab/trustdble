const express = require('express');

const router = express.Router();

// For POST and PUT requests: You NEED express.json() for JSON Object
// and express.urlencoded() for Object as strings or arrays
router.use(express.json());
router.use(express.urlencoded({ extended: false }));

module.exports = (sqlController) => {
  /* ##### SQL ##### */

  /**
   * @swagger
   * /api/sql:
   *  post:
   *    summary: Execute a sql statement
   *    tags:
   *      - SQL
   *    requestBody:
   *      required: true
   *      content:
   *        application/json:
   *          schema:
   *            type: object
   *            properties:
   *              sqlCommand:
   *                type: string
   *                description: SQL statement
   *                example: USE ethDB; SELECT * FROM ethTable
   *    responses:
   *      200:
   *        description: List of query results for single/multiple statements in the sqlCommand
   *        content:
   *          application/json:
   *            schema:
   *              type: array
   *              items:
   *                type: object
   *
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
   *      400:
   *        description: Regarding bad requests, incorrect database, table or column name
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: Unknown column 'xyz' in 'field list'
   *                code:
   *                  type: string
   *                  example: ER_BAD_FIELD_ERROR
   *                errno:
   *                  type: string
   *                  example: 1054
   *                sql:
   *                  type: string
   *                  example: SELECT xyz FROM db.table
   *                sqlState:
   *                  type: string
   *                  example: 42S22
   *                sqlMessage:
   *                  type: string
   *                  example: Unknown column 'xyz' in 'field list
   */
  router.post('/', (req, res) => sqlController.sql(req, res));

  return router;
};
