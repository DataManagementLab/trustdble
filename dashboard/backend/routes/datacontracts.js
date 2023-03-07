const { Router } = require('express');

module.exports = (datacontractsController) => {
  const router = Router();

  /**
   * @swagger
   * /api/datacontracts:
   *  post:
   *    summary: Create Data Contract
   *    tags:
   *      - Data Contract File
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 name:
   *                   type: string
   *                   description: Data Contract Name
   *                   example: myDataContract
   *                 owner:
   *                   type: string
   *                   description: Data Contract Owner
   *                   example: Simon
   *                 user:
   *                   type: string
   *                   description: Data Contract User
   *                   example: Leila
   *                 logicPath:
   *                   type: string
   *                   description: Path to a logic file
   *                   example: /home/simon/my_logic.txt
   *                 publicKey:
   *                   type: string
   *                   description: Public key of the user
   *                   example: MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==...
   *    responses:
   *      200:
   *        description: Download data contract file
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                encryptedDC:
   *                  type: object
   *                  properties:
   *                    data_contract:
   *                      type: string
   *                      description: Encrypted Data Contract
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
  router.post('/', (req, res) => datacontractsController.createDataContract(req, res));

  /**
   * @swagger
   * /api/datacontracts:
   *  post:
   *    summary: Create Data Contract
   *    tags:
   *      - Data Contract File
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 name:
   *                   type: string
   *                   description: Data Contract Name
   *                   example: myDataContract
   *                 owner:
   *                   type: string
   *                   description: Data Contract Owner
   *                   example: Simon
   *                 user:
   *                   type: string
   *                   description: Data Contract User
   *                   example: Leila
   *                 logicPath:
   *                   type: string
   *                   description: Path to a logic file
   *                   example: /home/simon/my_logic.txt
   *                 publicKey:
   *                   type: string
   *                   description: Public key of the user
   *                   example: MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==...
   *    responses:
   *      200:
   *        description: Download data contract file
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                encryptedDC:
   *                  type: object
   *                  properties:
   *                    data_contract:
   *                      type: string
   *                      description: Encrypted Data Contract
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
  router.post('/:dcName/invite', (req, res) => datacontractsController.encryptDataContract(req, res));

  /**
   * @swagger
   * /api/datacontracts:
   *  post:
   *    summary: Create Data Contract
   *    tags:
   *      - Data Contract File
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 name:
   *                   type: string
   *                   description: Data Contract Name
   *                   example: myDataContract
   *                 owner:
   *                   type: string
   *                   description: Data Contract Owner
   *                   example: Simon
   *                 user:
   *                   type: string
   *                   description: Data Contract User
   *                   example: Leila
   *                 logicPath:
   *                   type: string
   *                   description: Path to a logic file
   *                   example: /home/simon/my_logic.txt
   *                 publicKey:
   *                   type: string
   *                   description: Public key of the user
   *                   example: MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==...
   *    responses:
   *      200:
   *        description: Download data contract file
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                encryptedDC:
   *                  type: object
   *                  properties:
   *                    data_contract:
   *                      type: string
   *                      description: Encrypted Data Contract
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
  router.post('/:dcName/execute', (req, res) => datacontractsController.callDataContract(req, res));

  /**
   * @swagger
   * /api/datacontracts/add:
   *  post:
   *    summary: Add a data contract from file
   *    tags:
   *      - Data Contract File
   *    requestBody:
   *      required: true
   *      content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                 encryptedDC:
   *                   type: object
   *                   description: Encrypted Data Contract from file
   *                   properties:
   *                    encryptedDC:
   *                      type: object
   *                      description: Encrypted Data Contract
   *                      example: {"key":"...","iv":"...","data_contract":"..."}
   *    responses:
   *      200:
   *        description: Data Contract added in data contract table
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
   *        description: Regarding bad requests, incorrect data contract file
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
   *                  example: ADD DATA CONTRACT encrypted_data_contract={\"key\":\"...\"
   *                sqlState:
   *                  type: string
   *                  example: HY000
   *                sqlMessage:
   *                  type: string
   *                  example: Decrypting using private key failed
   */
  router.post('/add', (req, res) => datacontractsController.addDataContract(req, res));

  /**
   * @swagger
   * /api/datacontracts:
   *  get:
   *    summary: Shows all data contracts
   *    tags:
   *      - DataContracts
   *    responses:
   *      200:
   *        description: Return all data contracts
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
   *                      name:
   *                        type: string
   *                      dataOwner:
   *                        type: string
   *                      dataUser:
   *                        type: string
   *                  example:
   *                    - name: myDataContract
   *                      dataOwner: Simon
   *                      dataUser: Leila
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
  router.get('/local', (req, res) => datacontractsController.showDataContracts(req, res));

  /**
   * @swagger
   * /api/datacontracts/dcName:
   *  get:
   *    summary: Get a data contract by name
   *    tags:
   *      - Table
   *    parameters:
   *      - in: path
   *        name: dcName
   *        required: true
   *        description: Data Contract Name
   *        schema:
   *          type: string
   *          example: myDataContract
   *    responses:
   *      200:
   *        description: Return data contract
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                datacontract:
   *                  type: object
   *                  properties:
   *                      name:
   *                        type: string
   *                      owner:
   *                        type: string
   *                      user:
   *                        type: string
   *                  example:
   *                    - name: myDataContract
   *                      owner: Simon
   *                      user: Leila
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
   *        description: Regarding bad requests, incorrect data contract name
   *        content:
   *          application/json:
   *            schema:
   *              type: object
   *              properties:
   *                message:
   *                  type: string
   *                  example: Unknown data contract
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
  router.get('/local/:dcName', (req, res) => datacontractsController.getDataContract(req, res));

  /**
   * @swagger
   * /api/datacontracts:
   *  get:
   *    summary: Shows all data contracts
   *    tags:
   *      - DataContracts
   *    responses:
   *      200:
   *        description: Return all data contracts
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
   *                      name:
   *                        type: string
   *                      dataOwner:
   *                        type: string
   *                      dataUser:
   *                        type: string
   *                  example:
   *                    - name: myDataContract
   *                      dataOwner: Simon
   *                      dataUser: Leila
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
  router.get('/remote', (req, res) => datacontractsController.showRemoteDataContracts(req, res));

  /**
    * @swagger
    * /api/datacontracts/dcName:
    *  get:
    *    summary: Get a data contract by name
    *    tags:
    *      - Table
    *    parameters:
    *      - in: path
    *        name: dcName
    *        required: true
    *        description: Data Contract Name
    *        schema:
    *          type: string
    *          example: myDataContract
    *    responses:
    *      200:
    *        description: Return data contract
    *        content:
    *          application/json:
    *            schema:
    *              type: object
    *              properties:
    *                datacontract:
    *                  type: object
    *                  properties:
    *                      name:
    *                        type: string
    *                      owner:
    *                        type: string
    *                      user:
    *                        type: string
    *                  example:
    *                    - name: myDataContract
    *                      owner: Simon
    *                      user: Leila
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
    *        description: Regarding bad requests, incorrect data contract name
    *        content:
    *          application/json:
    *            schema:
    *              type: object
    *              properties:
    *                message:
    *                  type: string
    *                  example: Unknown data contract
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
  router.get('/remote/:dcName', (req, res) => datacontractsController.getRemoteDataContract(req, res));

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
  router.delete('/:dcName', (req, res) => datacontractsController.deleteDataContract(req, res));

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
  router.delete('/remote/:dcName', (req, res) => datacontractsController.deleteRemoteDataContract(req, res));

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
  router.delete('/:dcName/result', (req, res) => datacontractsController.deleteDataContractResults(req, res));

  return router;
};
