const {
  jest, describe, test, expect, beforeAll, afterAll,
} = require('@jest/globals');
const request = require('supertest');
const fs = require('fs');
const { DatabaseAdapter } = require('@trustdble-utils/database-adapter');

const server0host = process.argv.filter((arg) => arg.startsWith('server0host='))[0].split('=')[1];
const server1host = process.argv.filter((arg) => arg.startsWith('server1host='))[0].split('=')[1];

const server0port = process.argv.filter((arg) => arg.startsWith('server0port='))[0].split('=')[1];
const server1port = process.argv.filter((arg) => arg.startsWith('server1port='))[0].split('=')[1];

const db = new DatabaseAdapter(server0port, server0host);
const makeApp = require('../app');

const app = makeApp(db);
jest.setTimeout(10 * 60 * 1000);

const dbType = process.argv.filter((arg) => arg.startsWith('dbType='))[0].split('=')[1];
const dbName = `_testDB_${dbType}`;
const shards = process.argv.filter((arg) => arg.startsWith('shards='))[0].split('=')[1];

let Server1PublicKey = fs.readFileSync('./tests/tdb-server-key.pub', 'utf8');
Server1PublicKey = Server1PublicKey?.split('\n').slice(1, -2).join('');

let dbInvite = null;
let encryptedDC = null;
const tableInvites = {};

// https://stackoverflow.com/questions/53935108/jest-did-not-exit-one-second-after-the-test-run-has-completed-using-express
beforeAll((done) => {
  done();
});

afterAll((done) => {
  db.close();
  done();
});

const message = (error) => ({
  code: error.code,
  message: error.message,
  sql: error.sql,
});

const createTableInvite = async (tableName) => {
  const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/invite`).send({ publicKey: Server1PublicKey });
  if (response.statusCode !== 200) console.error(message(response.body));
  expect(response.statusCode).toBe(200);
  delete response.body.inviteFileContent.tableName;
  tableInvites[tableName] = response.body;
};

const addTable = async (tableName, tableInvite) => {
  db.recreatePool(server1port, server1host);
  const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/add`).send(tableInvite);
  if (response.statusCode !== 200) console.error(message(response.body));
  expect(response.statusCode).toBe(200);
};

const loadAllTables = () => {
  test('Load all unloaded shared tables', async () => {
    const response = await request(app).post(`/api/databases/${dbName}/tables/load`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    const response = await request(app).post(`/api/databases/xyz${dbName}/tables/load`);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_BAD_DB_ERROR');
  });
};

describe('Create a shared/local database', () => {
  test('Create a shared database', async () => {
    const requestBody = {
      dbName: `${dbName}`,
      dbType: `${dbType}`,
      shards: `${shards}`,
    };
    const response = await request(app).post('/api/databases').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Check a new entry of a shared DB to the trustdble.shared_databases table with all columns', async () => {
    const requestBody = {
      sqlCommand: `SELECT * FROM trustdble.shared_databases WHERE database_name='${dbName}'`,
    };
    const response = await request(app).post('/api/sql').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body[0]).toHaveProperty('database_name', `${dbName}`);
    expect(response.body[0]).toHaveProperty('blockchain_type', `${dbType}`);
    expect(response.body[0]).toHaveProperty('database_state', 'CREATED');
    expect(response.body[0]).toHaveProperty('meta_table_address');
    expect(response.body[0]).toHaveProperty('encryption_key');
    expect(response.body[0]).toHaveProperty('meta_chain_config.Network');
    expect(response.body[0].meta_table_address).not.toBe(null);
    expect(response.body[0].encryption_key).not.toBe(null);
    expect(response.body[0].meta_chain_config).not.toBe(null);
  });

  test('Check meta_table is created for a shared', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tables).toBeDefined();
    expect(response.body.tables.map((table) => table.name)).toEqual(expect.arrayContaining(
      ['meta_table', 'data_chains', 'shared_tables'],
    ));
  });

  test('Check DB.meta_table have meta info of data_chain and shared_tables', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/meta_table/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    expect(response.body.tuples.map((table) => table.tablename)).toEqual(expect.arrayContaining(
      ['data_chains', 'shared_tables'],
    ));
  });

  test('Should respond with a 400 status code', async () => {
    const requestBody = {
      dbName: `${dbName}`,
      dbType: `${dbType}`,
    };
    const response = await request(app).post('/api/databases').send(requestBody);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_DB_CREATE_EXISTS');
  });
});

describe('Get all databases', () => {
  test('Get all databases', async () => {
    const response = await request(app).get('/api/databases');
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.databases).toBeDefined();
    expect(response.body.databases.filter((database) => database.name === `${dbName}`)).toEqual([{
      name: `${dbName}`,
      type: `${dbType}`,
    }]);
  });
});

describe('Create a shared/local table in a database', () => {
  const requestBody = {
    tableName: 'myTable',
    isSharedTable: true,
    columns: [
      {
        name: 'id',
        type: 'int',
        isPrimaryKey: true,
        isNotNull: false,
      },
      {
        name: 'name',
        type: 'varchar(20)',
        isPrimaryKey: false,
        isNotNull: false,
      },
      {
        name: 'gender',
        type: 'char(2)',
        isPrimaryKey: false,
        isNotNull: true,
      },
    ],
  };
  test('Create a shared table', async () => {
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Check shared table myTable entry in shared_tables', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/shared_tables/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.partition_id.split('/')[0] === 'myTable')[0];
    expect(tuple).toHaveProperty('tableaddress');
    expect(tuple.tableaddress).not.toBe(null);
    expect(tuple).toHaveProperty('tableschema');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Check encrption keys for table myTable entry in key_store', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/key_store/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.tablename === 'myTable')[0];
    expect(tuple).toHaveProperty('encryptionkey');
    expect(tuple.tableschema).not.toBe(null);
    expect(tuple).toHaveProperty('iv');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Create a local table', async () => {
    requestBody.isSharedTable = false;
    requestBody.tableName = 'localTable';
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Create 2nd shared table', async () => {
    requestBody.isSharedTable = true;
    requestBody.tableName = 'myTable2';
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Check shared table myTable2 entry in shared_tables', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/shared_tables/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.partition_id.split('/')[0] === 'myTable2')[0];
    expect(tuple).toHaveProperty('tableaddress');
    expect(tuple.tableaddress).not.toBe(null);
    expect(tuple).toHaveProperty('tableschema');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Check encrption keys for table myTable2 entry in key_store', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/key_store/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.tablename === 'myTable2')[0];
    expect(tuple).toHaveProperty('encryptionkey');
    expect(tuple.tableschema).not.toBe(null);
    expect(tuple).toHaveProperty('iv');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Create 3rd shared table', async () => {
    requestBody.isSharedTable = true;
    requestBody.tableName = 'myTable3';
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Check shared table myTable3 entry in shared_tables', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/shared_tables/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.partition_id.split('/')[0] === 'myTable3')[0];
    expect(tuple).toHaveProperty('tableaddress');
    expect(tuple.tableaddress).not.toBe(null);
    expect(tuple).toHaveProperty('tableschema');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Check encrption keys for table myTable3 entry in key_store', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/key_store/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tuples).toBeDefined();
    const tuple = response.body.tuples.filter((table) => table.tablename === 'myTable3')[0];
    expect(tuple).toHaveProperty('encryptionkey');
    expect(tuple.tableschema).not.toBe(null);
    expect(tuple).toHaveProperty('iv');
    expect(tuple.tableschema).not.toBe(null);
  });

  test('Create 2nd local table', async () => {
    requestBody.isSharedTable = false;
    requestBody.tableName = 'local2Table';
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    requestBody.isSharedTable = true;
    requestBody.tableName = 'localTable';
    const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_TABLE_EXISTS_ERROR');
  });
});

describe('Delete a shared/local table', () => {
  test('Delete a shared table - myTable2', async () => {
    const tableName = 'myTable2';
    const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Delete a shared table - myTable3', async () => {
    const tableName = 'myTable3';
    const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Delete a local table - local2Table', async () => {
    const tableName = 'local2Table';
    const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    const tableName = 'myTable2';
    const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_BAD_TABLE_ERROR');
  });
});

describe('Get database type and table count', () => {
  test('Get database type and table count', async () => {
    const response = await request(app).get(`/api/databases/${dbName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.isShared).toBeDefined();
    expect(response.body.isShared).toEqual(true);
    expect(response.body.numTables).toBeDefined();
    expect(response.body.numTables).toEqual(6);
    expect(response.body.numUnloadedTables).toBeDefined();
    expect(response.body.numUnloadedTables).toEqual(2);
    expect(response.body.numSharedTables).toBeDefined();
    expect(response.body.numSharedTables).toEqual(4);
    expect(response.body.numHistoryTables).toBeDefined();
    const { numHistoryTables } = response.body;
    expect(response.body.numLocalTables).toBeDefined();
    if (numHistoryTables) {
      expect(response.body.numLocalTables).toEqual(4);
    } else {
      expect(response.body.numLocalTables).toEqual(2);
    }
    expect(response.body.numMetaTable).toBeDefined();
    expect(response.body.numMetaTable).toEqual(3);
  });
});

describe('Get tables of a database', () => {
  test('Get tables of a database', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.tables).toBeDefined();
    expect(response.body.tables.filter((table) => table.name === 'localTable')).toEqual([{
      isShared: false,
      name: 'localTable',
    }]);
    expect(response.body.tables.filter((table) => table.name === 'myTable')).toEqual([{
      isShared: true,
      name: 'myTable',
    }]);
    expect(response.body.tables.filter((table) => table.name === 'meta_table')).toEqual([{
      isShared: true,
      name: 'meta_table',
    }]);
    expect(response.body.tables.filter((table) => table.name === 'data_chains')).toEqual([{
      isShared: true,
      name: 'data_chains',
    }]);
    expect(response.body.tables.filter((table) => table.name === 'shared_tables')).toEqual([{
      isShared: true,
      name: 'shared_tables',
    }]);
    expect(response.body.tables.filter((table) => table.name === 'local2Table')).toEqual([]);
    expect(response.body.tables.filter((table) => table.name === 'myTable2')).toEqual([]);
    expect(response.body.tables.filter((table) => table.name === 'myTable3')).toEqual([]);
    expect(response.body.unloaded_tables).toBeDefined();
    expect(response.body.unloaded_tables).toEqual(expect.arrayContaining(
      ['myTable2', 'myTable3'],
    ));
  });
});

describe('Get table type and columns', () => {
  const tableName = 'myTable';
  test('Get table type and columns', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toEqual({
      isShared: true,
      columns: [
        {
          name: 'id',
          type: 'int',
          isPrimaryKey: true,
          isNotNull: true,
        },
        {
          name: 'name',
          type: 'varchar(20)',
          isPrimaryKey: false,
          isNotNull: false,
        },
        {
          name: 'gender',
          type: 'char(2)',
          isPrimaryKey: false,
          isNotNull: true,
        },
      ],
    });
  });

  test('Should respond with a 400 status code', async () => {
    const response = await request(app).get(`/api/databases/xyz${dbName}/tables/${tableName}`);
    expect(response.statusCode).toBe(400);
  });
});

describe('Load a shared table', () => {
  let tableName = 'myTable2';
  test('Load a shared table - myTable2', async () => {
    const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/load`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    tableName = 'xyzTables';
    const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/load`);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_KEY_NOT_FOUND');
  });
});

describe('Load all unloaded shared tables', loadAllTables);

describe('Insert values into a table', () => {
  let tableName = 'myTable';
  const requestBody = {
    tuples: [
      {
        id: 0,
        name: 'Simon',
        gender: 'M',
      },
      {
        id: 1,
        name: 'Benedikt',
        gender: 'M',
      },
      {
        id: 2,
        name: 'Eya',
        gender: 'F',
      },
    ],
  };
  test('Insert values into a table - myTable', async () => {
    const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Insert values into a table - localTable', async () => {
    tableName = 'localTable';
    const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    tableName = 'xyzTable';
    const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_NO_SUCH_TABLE');
  });
});

describe('Get table type, columns and tuples', () => {
  let tableName = 'myTable';
  test('Get table type, columns and tuples - myTable', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.isShared).toBeDefined();
    expect(response.body.isShared).toEqual(true);
    expect(response.body.columns).toBeDefined();
    expect(response.body.columns).toEqual([
      {
        name: 'id',
        type: 'int',
        isPrimaryKey: true,
        isNotNull: true,
      },
      {
        name: 'name',
        type: 'varchar(20)',
        isPrimaryKey: false,
        isNotNull: false,
      },
      {
        name: 'gender',
        type: 'char(2)',
        isPrimaryKey: false,
        isNotNull: true,
      },
    ]);
    expect(response.body.tuples).toBeDefined();
    expect(response.body.tuples.filter((tuple) => tuple.id === 0)).toEqual([
      {
        id: 0,
        name: 'Simon',
        gender: 'M',
      },
    ]);
    expect(response.body.tuples.filter((tuple) => tuple.id === 1)).toEqual([
      {
        id: 1,
        name: 'Benedikt',
        gender: 'M',
      },
    ]);
    expect(response.body.tuples.filter((tuple) => tuple.id === 2)).toEqual([
      {
        id: 2,
        name: 'Eya',
        gender: 'F',
      },
    ]);
  });

  test('Should respond with a 400 status code', async () => {
    tableName = 'xyzTable';
    const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
    expect(response.statusCode).toBe(400);
  });
});

describe('Get invite file content for a given database', () => {
  test('Get invite file content for a given database', async () => {
    const response = await request(app).post(`/api/databases/${dbName}/invite`).send({ publicKey: Server1PublicKey });
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    dbInvite = response.body;
    delete dbInvite.inviteFileContent.dbName;
  });
});

describe('Get invite file content for a given table', () => {
  test('Get invite file content for a myTable', () => createTableInvite('myTable'));
  test('Get invite file content for a myTable2', () => createTableInvite('myTable2'));
  test('Get invite file content for a myTable3', () => createTableInvite('myTable3'));
  test('Get invite file content for a federated local table localTable', () => createTableInvite(`${dbName}.localTable`));
});

describe('Add a shared database on server 1', () => {
  test('Add a shared database using invite file', async () => {
    db.recreatePool(server1port, server1host);
    const response = await request(app).post(`/api/databases/${dbName}/add`).send(dbInvite);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });
});

describe('Add shared tables on server 1', () => {
  test('Add a shared table "myTable" using invite file', () => addTable('myTable', tableInvites.myTable));
  test('Add a shared table "myTable2" using invite file', () => addTable('myTable2', tableInvites.myTable2));
  test('Add a shared table "myTable3" using invite file', () => addTable('myTable3', tableInvites.myTable3));
  test('Add a federated table "localTable" using invite file', () => addTable(`${dbName}.localTable`, tableInvites[`${dbName}.localTable`]));
});

describe('Get table type, columns and tuples on server1', () => {
  const tableName = 'localTable';
  test('Get table type, columns and tuples - localTable', async () => {
    const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.isShared).toBeDefined();
    expect(response.body.isShared).toEqual(false);
    expect(response.body.tuples).toBeDefined();
    expect(response.body.tuples.filter((tuple) => tuple.id === 0)).toEqual([
      {
        id: 0,
        name: 'Simon',
        gender: 'M',
      },
    ]);
    expect(response.body.tuples.filter((tuple) => tuple.id === 1)).toEqual([
      {
        id: 1,
        name: 'Benedikt',
        gender: 'M',
      },
    ]);
    expect(response.body.tuples.filter((tuple) => tuple.id === 2)).toEqual([
      {
        id: 2,
        name: 'Eya',
        gender: 'F',
      },
    ]);
  });
});

describe('Load all unloaded shared tables on server 1', loadAllTables);

describe('Delete shared/local databases', () => {
  test('Delete a shared database from server 0', async () => {
    db.recreatePool(server0port, server0host);
    const response = await request(app).delete(`/api/databases/${dbName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Check a database entry is removed from trustdble.shared_databases from server 0', async () => {
    const requestBody = {
      sqlCommand: `SELECT * FROM trustdble.shared_databases WHERE database_name='${dbName}'`,
    };
    const response = await request(app).post('/api/sql').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toEqual([]);
  });

  test('Delete encrypted invites from "trustdble"', async () => {
    const requestBody = {
      sqlCommand: 'DELETE FROM trustdble.encrypted_invite; DELETE FROM trustdble.encrypted_table_invite',
    };
    const response = await request(app).post('/api/sql').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Should respond with a 400 status code', async () => {
    const response = await request(app).delete(`/api/databases/xyz${dbName}`);
    expect(response.statusCode).toBe(400);
    expect(response.body.code).toEqual('ER_DB_DROP_EXISTS');
  });

  test('Delete a shared database from server 1', async () => {
    db.recreatePool(server1port, server1host);
    const response = await request(app).delete(`/api/databases/${dbName}`);
    if (response.statusCode !== 200) console.error(message(response.body));//
    expect(response.statusCode).toBe(200);
  });

  test('Check a database entry is removed from trustdble.shared_databases from server 1', async () => {
    const requestBody = {
      sqlCommand: `SELECT * FROM trustdble.shared_databases WHERE database_name='${dbName}'`,
    };
    const response = await request(app).post('/api/sql').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toEqual([]);
  });

  test('Delete invites from "trustdble"', async () => {
    const requestBody = {
      sqlCommand: 'DELETE FROM trustdble.invites',
    };
    const response = await request(app).post('/api/sql').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });
});

/**
 * To run data contract tests successfully:
 * - Setup a server with number 1 on sgx-n04 or sgx-n05
 * - Copy the /home/sgx_user/trustdble-server/server/build-debug/plugin/data_contracts/server_cert.pem into
 *            /home/sgx_user/trustdble-server/server/build-debug/plugin/data_contracts/remote_server_cert.pem
 */
describe('Data Contracts on server 1', () => {
  const requestBody = {
    name: 'dcName',
    owner: 'dcOwner',
    user: 'dcUser',
    logicPath: '/home/simon/Projects/dm/trustdble-dashboard/backend/tests/logic.txt',
    parameters: 'tablename,fieldname',
    publicKey: Server1PublicKey,
  };

  const params = {
    parameterList: 'demodb.employee,salary',
  };

  const databaseRequest = {
    dbName: 'demodb',
    dbType: 'STUB',
    shards: '1',
  };

  const tableRequest = {
    isSharedTable: false,
    tableName: 'employee',
    columns: [
      {
        name: 'id',
        type: 'int',
        isPrimaryKey: true,
        isNotNull: false,
      },
      {
        name: 'name',
        type: 'varchar(20)',
        isPrimaryKey: false,
        isNotNull: false,
      },
      {
        name: 'salary',
        type: 'int',
        isPrimaryKey: false,
        isNotNull: true,
      },
    ],
  };

  const insertRequest = {
    tuples: [
      {
        id: 0,
        name: 'Simon',
        salary: 2500,
      },
      {
        id: 1,
        name: 'Leila',
        salary: 2000,
      },
      {
        id: 2,
        name: 'Eya',
        salary: 1500,
      },
    ],
  };

  test('Create a shared database', async () => {
    db.recreatePool(server1port, server1host);
    const response = await request(app).post('/api/databases').send(databaseRequest);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Create a local table', async () => {
    const response = await request(app).post(`/api/databases/${databaseRequest.dbName}/tables`).send(tableRequest);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Insert values into a table - myTable', async () => {
    const response = await request(app).put(`/api/databases/${databaseRequest.dbName}/tables/${tableRequest.tableName}/tuples`).send(insertRequest);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Create data contract', async () => {
    const response = await request(app).post('/api/datacontracts').send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.body.owner).toEqual(requestBody.owner);
    expect(response.body.user).toEqual(requestBody.user);
    expect(response.body.name).toEqual(requestBody.name);
    expect(response.body.parameters).toEqual(requestBody.parameters);
  });

  test('Encrypt a data contract', async () => {
    const response = await request(app).post(`/api/datacontracts/${requestBody.name}/invite`).send(requestBody);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toBeDefined();
    expect(response.body.encryptedDC).toBeDefined();
    expect(response.body.encryptedDC.key).toBeDefined();
    expect(response.body.encryptedDC.iv).toBeDefined();
    expect(response.body.encryptedDC.data_contract).toBeDefined();
    encryptedDC = response.body;
  });

  test('Add a data contract using encrypted data contract', async () => {
    const response = await request(app).post('/api/datacontracts/add').send(encryptedDC);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Get added data contract', async () => {
    const response = await request(app).get(`/api/datacontracts/local/${requestBody.name}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toBeDefined();
    expect(response.body.owner).toEqual(requestBody.owner);
    expect(response.body.user).toEqual(requestBody.user);
    expect(response.body.name).toEqual(requestBody.name);
    expect(response.body.parameters).toEqual(requestBody.parameters);
  });

  test('Get all data contracts', async () => {
    const response = await request(app).get('/api/datacontracts/local');
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toBeDefined();
  });

  test('Get added remote data contract', async () => {
    const response = await request(app).get(`/api/datacontracts/remote/${requestBody.name}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toBeDefined();
    expect(response.body.owner).toEqual(requestBody.owner);
    expect(response.body.user).toEqual(requestBody.user);
    expect(response.body.name).toEqual(requestBody.name);
  });

  test('Get all remote data contracts', async () => {
    const response = await request(app).get('/api/datacontracts/remote');
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body).toBeDefined();
  });

  test('Execute added data contract', async () => {
    const response = await request(app).post(`/api/datacontracts/${requestBody.name}/execute`).send(params);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
    expect(response.body.result).toEqual("[(Decimal('2000.0000'),)]");
  });

  test('Delete all data contracts', async () => {
    const response1 = await request(app).delete(`/api/datacontracts/${requestBody.name}`);
    if (response1.statusCode !== 200) console.error(message(response1.body));
    expect(response1.statusCode).toBe(200);
    const response2 = await request(app).get(`/api/datacontracts/local/${requestBody.name}`);
    if (response2.statusCode !== 200) console.error(message(response2.body));
    expect(response2.statusCode).toBe(200);
    expect(response2.body).toBeDefined();
    expect(response2.body).toEqual('');
  });

  test('Delete all remote data contracts', async () => {
    const response1 = await request(app).delete(`/api/datacontracts/remote/${requestBody.name}`);
    if (response1.statusCode !== 200) console.error(message(response1.body));
    expect(response1.statusCode).toBe(200);
    const response2 = await request(app).get(`/api/datacontracts/remote/${requestBody.name}`);
    if (response2.statusCode !== 200) console.error(message(response2.body));
    expect(response2.statusCode).toBe(200);
    expect(response2.body).toBeDefined();
    expect(response2.body).toEqual('');
  });

  test('Delete result of data contract', async () => {
    const response = await request(app).delete(`/api/datacontracts/${requestBody.name}/result`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });

  test('Delete database demodb', async () => {
    const response = await request(app).delete(`/api/databases/${databaseRequest.dbName}`);
    if (response.statusCode !== 200) console.error(message(response.body));
    expect(response.statusCode).toBe(200);
  });
});
