// Get the client
const mysql = require('mysql2');

const propertiesReader = require('properties-reader');

const props = propertiesReader('./../configs/app.properties');

class DatabaseAdapter {
  constructor(
    port = props.get('database.mysql.port'),
    host = props.get('database.mysql.host'),
    user = props.get('database.mysql.user'),
    password = props.get('database.mysql.password'),
  ) {
    // Database configuration
    this.dbConfig = {
      multipleStatements: true,
      waitForConnections: true,
      connectionLimit: 10,
      queueLimit: 0,
      dateStrings: true,
      host,
      port,
      user,
      password,
    };

    // Create a pool
    this.createPool();
  }

  createPool() {
    // Create the pool
    this.pool = mysql.createPool(this.dbConfig);

    // Now get a Promise wrapped instance of that pool
    this.promisePool = this.pool.promise();
  }

  recreatePool(port, host, user, password) {
    // Update dbConfig
    if (port !== undefined) this.dbConfig.port = port;
    if (host !== undefined) this.dbConfig.host = host;
    if (user !== undefined) this.dbConfig.user = user;
    if (password !== undefined) this.dbConfig.password = password;

    // End existing pool
    this.close();

    // Create a new pool
    this.createPool();
  }

  // Check if backend can connect to database
  async databaseAlive() {
    try {
      await this.promisePool.query('SELECT 1');
      return true;
    } catch (error) {
      return false;
    }
  }

  async query(sqlQuery, params) {
    if (process.env.NODE_ENV === 'development') {
      console.debug(`Executing query: ${sqlQuery}`);
    }
    // Query database using promises
    const results = await this.promisePool.query(sqlQuery, params);
    // results contains 0: rows and 1: fields; only rows are needed
    return results[0];
  }

  close() {
    this.pool.end();
  }
}

module.exports = { DatabaseAdapter };
