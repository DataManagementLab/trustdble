const express = require('express');

const router = express.Router();

// For POST and PUT requests: You NEED express.json() for JSON Object
// and express.urlencoded() for Object as strings or arrays
router.use(express.json());
router.use(express.urlencoded({ extended: false }));

module.exports = (databasesController) => {
  /* ##### DATABASES ##### */

  /**
   * @swagger
   * /api/databases:
   *  get:
   *    summary: Get all databases
   *    tags:
   *      - Database
   *    responses:
   *      200:
   *        description: Return a list of databases and the corresponding database type
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                databases:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      name:
   *                        type: string
   *                        description: Database name
   *                        example: mysql
   *                      type:
   *                        type: string
   *                        description: Database type (LOCAL, ETHEREUM, STUB)
   *                        enum:
   *                          - LOCAL
   *                          - STUB
   *                          - ETHEREUM
   *                        example: LOCAL
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
  router.get('/', (req, res) => databasesController.showDatabases(req, res));

  /**
   * @swagger
   * /api/databases:
   *  post:
   *    summary: Create a shared/local database
   *    tags:
   *      - Database
   *    requestBody:
   *      required: true
   *      content:
   *        application/json:
   *          schema:
   *            type: object
   *            properties:
   *              dbName:
   *                type: string
   *                description: Database Name
   *                example: ethDB
   *              dbType:
   *                type: string
   *                enum:
   *                  - LOCAL
   *                  - STUB
   *                  - ETHEREUM
   *              shards:
   *                type: number
   *                description: Number of Shards
   *                example: 1
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
  router.post('/', (req, res) => databasesController.createDatabase(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}:
   *  get:
   *    summary: Get database type and table count
   *    tags:
   *      - Database
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *    responses:
   *      200:
   *        description: Return type of database and table count
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                isShared:
   *                  type: boolean
   *                  description: Table is shared or not
   *                  example: true
   *                numTables:
   *                  type: integer
   *                  description: Number of existing tables in a database
   *                  example: 7
   *                numUnloadedTables:
   *                  type: integer
   *                  description: Number of unloaded tables in a database but listed in meta_table
   *                  example: 2
   *                numSharedTables:
   *                  type: integer
   *                  description: Number of shared tables in a database
   *                  example: 2
   *                numLocalTables:
   *                  type: integer
   *                  description: Number of local tables in a database
   *                  example: 5
   *                numHistoryTables:
   *                  type: integer
   *                  description: Number of history tables in a database
   *                  example: 3
   *                numMetaTable:
   *                  type: integer
   *                  description: Number of meta_table in a database
   *                  example: 1
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
  router.get('/:dbName', (req, res) => databasesController.getDatabase(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}:
   *  delete:
   *    summary: Delete a shared/local database
   *    tags:
   *      - Database
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
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
  router.delete('/:dbName', (req, res) => databasesController.deleteDatabase(req, res));

  /* ##### TABLES ##### */

  /**
   * @swagger
   * /api/databases/{dbName}/tables:
   *  get:
   *    summary: Get tables of a database
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *    responses:
   *      200:
   *        description:
   *          Returns a list of all tables within the given database
   *          with information if the table is shared or not.
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                tables:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      name:
   *                        type: string
   *                        description: table name
   *                        example: ethTable
   *                      isShared:
   *                        type: boolean
   *                        description: Table is shared or not
   *                        example: true
   *                unloaded_tables:
   *                  type: array
   *                  items:
   *                    type: string
   *                  example: [table1, table2, table3]
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
  router.get('/:dbName/tables', (req, res) => databasesController.showTables(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables:
   *  post:
   *    summary: Create a shared/local table in a database
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                tableName:
   *                  type: string
   *                  description: Table Name
   *                  example: ethTable
   *                isSharedTable:
   *                  type: boolean
   *                  description: Table is shared or not
   *                  example: true
   *                columns:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      name:
   *                        type: string
   *                        description: column name
   *                      type:
   *                        type: string
   *                        description: column data type
   *                      isPrimaryKey:
   *                        type: boolean
   *                        description: Is a primary key
   *                      isNotNull:
   *                        type: boolean
   *                        description: Is not null
   *                  example:
   *                    - name: id
   *                      type: int
   *                      isPrimaryKey: true
   *                      isNotNull: false
   *                    - name: name
   *                      type: varchar(20)
   *                      isPrimaryKey: false
   *                      isNotNull: false
   *                    - name: gender
   *                      type: char(2)
   *                      isPrimaryKey: false
   *                      isNotNull: true
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
  router.post('/:dbName/tables', (req, res) => databasesController.createTable(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/tuples/delete:
   *  delete:
   *    summary: Delete a tuple from table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                selector:
   *                  type: string
   *                  description: Column Name where selecting the tuple to be deleted
   *                  example: "Number = '1'"
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
  router.delete('/:dbName/tables/:tableName/tuples/delete', (req, res) => databasesController.deleteTuple(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/tuples/update:
   *  post:
   *    summary: Edit/Update a tuple from table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                colValues:
   *                  type: string
   *                  description: consists of all the values that are supposed to be changed
   *                  example: "Number = 1"
   *                selector:
   *                  type: string
   *                  description: where clause
   *                  example: "Number = 2"
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
  router.post('/:dbName/tables/:tableName/tuples/update', (req, res) => databasesController.updateTuple(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/load:
   *  post:
   *    summary: load shared tables
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
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
  router.post('/:dbName/tables/load', (req, res) => databasesController.loadSharedTables(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}:
   *  get:
   *    summary: Get a tables type and columns
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    responses:
   *      200:
   *        description: Return type of table and columns schema
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                isShared:
   *                  type: boolean
   *                  description: Table is shared or not
   *                  example: true
   *                columns:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      name:
   *                        type: string
   *                        description: column name
   *                      type:
   *                        type: string
   *                        description: column data type
   *                      isPrimaryKey:
   *                        type: boolean
   *                        description: Is a primary key
   *                      isNotNull:
   *                        type: boolean
   *                        description: Is not null
   *                  example:
   *                    - name: id
   *                      type: int
   *                      isPrimaryKey: true
   *                      isNotNull: true
   *                    - name: name
   *                      type: varchar(20)
   *                      isPrimaryKey: false
   *                      isNotNull: false
   *                    - name: gender
   *                      type: char(2)
   *                      isPrimaryKey: false
   *                      isNotNull: true
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
  router.get('/:dbName/tables/:tableName', (req, res) => databasesController.getTable(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}:
   *  delete:
   *    summary: Delete a shared/local table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
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
  router.delete('/:dbName/tables/:tableName', (req, res) => databasesController.deleteTable(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/load:
   *  post:
   *    summary: load a shared table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
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
  router.post('/:dbName/tables/:tableName/load', (req, res) => databasesController.loadSharedTable(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/tuples:
   *  get:
   *    summary: Get the columns and tuples of a table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    responses:
   *      200:
   *        description: Return columns schema and content of table
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                isShared:
   *                  type: boolean
   *                  description: Table is shared or not
   *                  example: true
   *                columns:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      name:
   *                        type: string
   *                        description: column name
   *                      type:
   *                        type: string
   *                        description: column data type
   *                      isPrimaryKey:
   *                        type: boolean
   *                        description: Is a primary key
   *                      isNotNull:
   *                        type: boolean
   *                        description: Is not null
   *                  example:
   *                    - name: id
   *                      type: int
   *                      isPrimaryKey: true
   *                      isNotNull: true
   *                    - name: name
   *                      type: varchar(20)
   *                      isPrimaryKey: false
   *                      isNotNull: false
   *                    - name: gender
   *                      type: char(2)
   *                      isPrimaryKey: false
   *                      isNotNull: true
   *                tuples:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      id:
   *                        type: int
   *                      name:
   *                        type: string
   *                      gender:
   *                        type: string
   *                  example:
   *                    - id: 0
   *                      name: Simon
   *                      gender: M
   *                    - id: 1
   *                      name: Benedikt
   *                      gender: M
   *                    - id: 2
   *                      name: Eya
   *                      gender: F
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
  router.get('/:dbName/tables/:tableName/tuples', (req, res) => databasesController.getTuples(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/tuples:
   *  put:
   *    summary: Insert values into a table
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                tuples:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      id:
   *                        type: int
   *                        description: column name and value
   *                        example: 0
   *                      name:
   *                        type: string
   *                        description: column name and value
   *                        example: Simon
   *                      gender:
   *                        type: string
   *                        description: column name and value
   *                        example: M
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
  router.put('/:dbName/tables/:tableName/tuples', (req, res) => databasesController.insertTuples(req, res));

  /* ##### TABLES HISTORY ##### */

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/history/states:
   *  get:
   *    summary: Get all history states of a table
   *    tags:
   *      - History
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    responses:
   *      200:
   *        description: Return list of history states of a table
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                states:
   *                  type: array
   *                  items:
   *                    type: string
   *                  example: [2022-07-29 08:55:09, 2022-07-29 08:56:58, 2022-07-29 08:57:53]
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
  router.get('/:dbName/tables/:tableName/history/states', (req, res) => databasesController.getTableHistoryStates(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/history/states/{state}:
   *  get:
   *    summary: Get table history at state
   *    tags:
   *      - History
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *      - in: path
   *        name: state
   *        required: true
   *        description: state
   *        schema:
   *          type: string
   *          example: 2022-07-29 08:57:53
   *    responses:
   *      200:
   *        description: Return table history at state
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                tuples:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      datetime:
   *                        type: string
   *                        description: Timestamp
   *                        example: 2022-07-29 08:57:53
   *                      dataChainId:
   *                        type: int
   *                        description: Data Chain Id
   *                        example: 0
   *                      blockNumber:
   *                        type: int
   *                        description: Block Number
   *                        example: 7801
   *                      editor:
   *                        type: string
   *                        description: Editor ID
   *                        example: 0xC5D459235a401603a4A742a14cAc30fE...
   *                      transactionId:
   *                        type: string
   *                        description: Transaction ID
   *                        example: 0xf7b66f49cdab51cdf8ea31263...
   *                      transactionType:
   *                        type: string
   *                        description: Transaction Type
   *                        example: PUT
   *                      primaryKeyHash:
   *                        type: string
   *                        description: Primary Key Hash
   *                        example: 3964396632393035323761....
   *                      dataValue:
   *                        type: string
   *                        description: Data value
   *                        example: 0,Benedikt,M
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
  router.get('/:dbName/tables/:tableName/history/states/:state', (req, res) => databasesController.getTableHistoryAtState(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/history/tuples/{primaryKeyHash}/changes:
   *  get:
   *    summary: Get history of a tuple
   *    tags:
   *      - History
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *      - in: path
   *        name: primaryKeyHash
   *        required: true
   *        description: Primary Key Hash
   *        schema:
   *          type: string
   *          example: '0x6466336636313938303461393266646234303537313932646334336464373438'
   *    responses:
   *      200:
   *        description: Get the history of changes for the tuple with tupleId.
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                tuple_states:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      datetime:
   *                        type: string
   *                        description: Timestamp
   *                        example: 2022-07-29 08:57:53
   *                      dataChainId:
   *                        type: int
   *                        description: Data Chain Id
   *                        example: 0
   *                      blockNumber:
   *                        type: int
   *                        description: Block Number
   *                        example: 7801
   *                      editor:
   *                        type: string
   *                        description: Editor ID
   *                        example: 0xC5D459235a401603a4A742a14cAc30fE...
   *                      transactionId:
   *                        type: string
   *                        description: Transaction ID
   *                        example: 0xf7b66f49cdab51cdf8ea31263...
   *                      transactionType:
   *                        type: string
   *                        description: Transaction Type
   *                        example: PUT
   *                      primaryKeyHash:
   *                        type: string
   *                        description: Primary Key Hash
   *                        example: 3964396632393035323761....
   *                      dataValue:
   *                        type: string
   *                        description: Data value
   *                        example: 0,Benedikt,M
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
  router.get('/:dbName/tables/:tableName/history/tuples/:primaryKeyHash/changes', (req, res) => databasesController.getTupleHistory(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/history/editors/{editor}/changes:
   *  get:
   *    summary: Get history of an editor
   *    tags:
   *      - History
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *      - in: path
   *        name: editor
   *        required: true
   *        description: Editor
   *        schema:
   *          type: string
   *          example: '0xC5D459235a401603a4A742a14cAc30fEE5ab54d3'
   *    responses:
   *      200:
   *        description: Get history of changes performed by an editor
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                changes:
   *                  type: array
   *                  items:
   *                    type: object
   *                    properties:
   *                      datetime:
   *                        type: string
   *                        description: Timestamp
   *                        example: 2022-07-29 08:57:53
   *                      dataChainId:
   *                        type: int
   *                        description: Data Chain Id
   *                        example: 0
   *                      blockNumber:
   *                        type: int
   *                        description: Block Number
   *                        example: 7801
   *                      editor:
   *                        type: string
   *                        description: Editor ID
   *                        example: 0xC5D459235a401603a4A742a14cAc30fE...
   *                      transactionId:
   *                        type: string
   *                        description: Transaction ID
   *                        example: 0xf7b66f49cdab51cdf8ea31263...
   *                      transactionType:
   *                        type: string
   *                        description: Transaction Type
   *                        example: PUT
   *                      primaryKeyHash:
   *                        type: string
   *                        description: Primary Key Hash
   *                        example: 3964396632393035323761....
   *                      dataValue:
   *                        type: string
   *                        description: Data value
   *                        example: 0,Benedikt,M
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
  router.get('/:dbName/tables/:tableName/history/editors/:editor/changes', (req, res) => databasesController.getEditorHistory(req, res));

  /* ##### Invite File ##### */

  /**
   * @swagger
   * /api/databases/{dbName}/invite:
   *  post:
   *    summary: Create an invite file for a given database
   *    tags:
   *      - Invite File
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 publicKey:
   *                   type: string
   *                   description: Public key of the user
   *                   example: MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==...
   *    responses:
   *      200:
   *        description:
   *          Invite file content for a given database
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                inviteFileContent:
   *                  type: object
   *                  properties:
   *                    key:
   *                      type: string
   *                      description: Encrypted Key
   *                      example: "46ef75b61c297c1d2d3ae93af491bbf9b5979659d95742a092c..."
   *                    iv:
   *                      type: string
   *                      description: Encrypted iv
   *                      example: "2d9965d9356e9fff348a6e21b244be2f8776fc9f93f859a68991b15..."
   *                    invite:
   *                      type: string
   *                      description: Encrypted Invite
   *                      example: "001c9cc0788d06270f72a0faf7e624a2c3dd2a311f9299bd99deeffba76..."
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
   *        description: Regarding bad requests, incorrect database
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: Unknown database abc
   *                code:
   *                  type: string
   *                  example: ER_BAD_DB_ERROR
   *                errno:
   *                  type: string
   *                  example: 1049
   *                sql:
   *                  type: string
   *                  example: ENCRYPT INVITE OF DATABASE abc public_key=MIICIjANBgkqhkiG9AOCA==..;
   *                sqlState:
   *                  type: string
   *                  example: 42000
   *                sqlMessage:
   *                  type: string
   *                  example: Unknown database abc
   */
  router.post('/:dbName/invite', (req, res) => databasesController.getInviteFileContentForDatabase(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/add:
   *  post:
   *    summary: Add a shared database based on invite file content
   *    tags:
   *      - Invite File
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                inviteFileContent:
   *                  type: object
   *                  properties:
   *                    key:
   *                      type: string
   *                      description: Encrypted Key
   *                      example: "46ef75b61c297c1d2d3ae93af491bbf9b5979659d95742a092c..."
   *                    iv:
   *                      type: string
   *                      description: Encrypted iv
   *                      example: "2d9965d9356e9fff348a6e21b244be2f8776fc9f93f859a68991b15..."
   *                    invite:
   *                      type: string
   *                      description: Encrypted Invite
   *                      example: "001c9cc0788d06270f72a0faf7e624a2c3dd2a311f9299bd99deeffba76..."
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
  router.post('/:dbName/add', (req, res) => databasesController.loadSharedDatabase(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/invite:
   *  post:
   *    summary: Create Invite for Shared Table
   *    tags:
   *      - Invite File
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 publicKey:
   *                   type: string
   *                   description: Public key of the user
   *                   example: MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==...
   *    responses:
   *      200:
   *        description: Download invite file for Shared table
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                inviteFileContent:
   *                  type: object
   *                  properties:
   *                    invite:
   *                      type: string
   *                      description: Encrypted Invite
   *                      example: "001c9cc0788d06270f72a0faf7e624a2c3dd2a311f9299bd99deeffba76..."
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
   *        description: Regarding bad requests, incorrect Key
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: Public key is not valid
   *                code:
   *                  type: string
   *                  example: ER_YES
   *                errno:
   *                  type: number
   *                  example: 1003
   *                sql:
   *                  type: string
   *                  example: CREATE TABLE INVITE Tabl1 public_key=sdnkfnaknkwnce;
   *                sqlState:
   *                  type: string
   *                  example: HY000
   *                sqlMessage:
   *                  type: string
   *                  example: Public key is not valid
   */
  router.post('/:dbName/tables/:tableName/invite', (req, res) => databasesController.createTableInvite(req, res));

  /**
   * @swagger
   * /api/databases/{dbName}/tables/{tableName}/add:
   *  post:
   *    summary: Add a shared table based on invite file
   *    tags:
   *      - Invite File
   *    parameters:
   *      - in: path
   *        name: dbName
   *        required: true
   *        description: Database Name
   *        schema:
   *          type: string
   *          example: ethDB
   *      - in: path
   *        name: tableName
   *        required: true
   *        description: Table Name
   *        schema:
   *          type: string
   *          example: ethTable
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 encryptedInvite:
   *                   type: object
   *                   description: Encrypted Invite from Shared invite file
   *                   properties:
   *                    invite:
   *                      type: object
   *                      description: Encrypted Invite
   *                      example: 4293db0be3727541245d479c610c...
   *    responses:
   *      200:
   *        description: Shared table entry added in Key_store
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
   *        description: Regarding bad requests, incorrect shared file
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: Decrypting using private key failed
   *                code:
   *                  type: string
   *                  example: ER_YES
   *                errno:
   *                  type: number
   *                  example: 1003
   *                sql:
   *                  type: string
   *                  example: USE Eth1; ADD TABLE INVITE encrypted_invite={\"invite\":\"4293db0b
   *                sqlState:
   *                  type: string
   *                  example: HY000
   *                sqlMessage:
   *                  type: string
   *                  example: Decrypting using private key failed
   */
  router.post('/:dbName/tables/:tableName/add', (req, res) => databasesController.addTableInvite(req, res));

  return router;
};
