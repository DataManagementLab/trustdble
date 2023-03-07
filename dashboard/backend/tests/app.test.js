const {
  jest, describe, test, expect,
} = require('@jest/globals');
const request = require('supertest');
const makeApp = require('../app');

const databaseAlive = jest.fn();
const query = jest.fn();

const app = makeApp({
  databaseAlive,
  query,
});

describe('/api/databases', () => {
  describe('Get all databases', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          name: 'mysql',
          type: 'LOCAL',
        },
        {
          name: 'ethDB',
          type: 'ETHEREUM',
        },
      ]);
      const response = await request(app).get('/api/databases');
      expect(response.statusCode).toBe(200);
      expect(response.body.databases).toBeDefined();
      expect(response.body.databases).toEqual([
        {
          name: 'mysql',
          type: 'LOCAL',
        },
        {
          name: 'ethDB',
          type: 'ETHEREUM',
        },
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Unknown column 'shared_database' in 'field list'",
        code: 'ER_BAD_FIELD_ERROR',
      });
      const response = await request(app).get('/api/databases');
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Unknown column 'shared_database' in 'field list'");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get('/api/databases');
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });

  describe('Create a shared/local database', () => {
    const requestBody = {
      dbName: 'ethDB',
      dbType: 'ETHEREUM',
      shards: 3,
    };
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).post('/api/databases').send(requestBody);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' already exist",
      });
      const response = await request(app).post('/api/databases').send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' already exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post('/api/databases').send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}', () => {
  describe('Get database type and table count', () => {
    const dbName = 'ethDB';
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([{ isShared: 1 }])
        .mockResolvedValueOnce([{ isShared: 1 }])
        .mockResolvedValueOnce([
          {},
          [
            {
              name: 'eth2Table',
            },
            {
              name: 'eth3Table',
            },
          ],
        ]).mockResolvedValueOnce([
          {
            name: 'ethTable',
            isShared: true,
          },
          {
            name: 'history_ethTable',
            isShared: false,
          },
          {
            name: 'history_eth2Table',
            isShared: false,
          },
          {
            name: 'history_eth3Table',
            isShared: false,
          },
          {
            name: 'local2Table',
            isShared: false,
          },
          {
            name: 'localTable',
            isShared: false,
          },
          {
            name: 'meta_table',
            isShared: true,
          },
        ]);
      const response = await request(app).get(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(200);
      expect(response.body).toEqual({
        isShared: true,
        numTables: 7,
        numUnloadedTables: 2,
        numSharedTables: 2,
        numLocalTables: 5,
        numHistoryTables: 3,
        numMetaTable: 1,
      });
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });

  describe('Delete a shared/local database', () => {
    const dbName = 'ethDB';
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([{ isShared: 1 }]).mockResolvedValueOnce({});
      const response = await request(app).delete(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' not exist",
      });
      const response = await request(app).delete(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).delete(`/api/databases/${dbName}`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables', () => {
  describe('Get tables of a database', () => {
    const dbName = 'ethDB';
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          name: 'myTable',
          isShared: 0,
        },
        {
          name: 'ethTable',
          isShared: 1,
        },
      ]).mockResolvedValueOnce([{ isShared: 1 }])
        .mockResolvedValueOnce([
          {},
          [
            {
              name: 'eth2Table',
            },
            {
              name: 'eth3Table',
            },
          ],
        ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables`);
      expect(response.statusCode).toBe(200);
      expect(response.body.tables).toBeDefined();
      expect(response.body.tables).toEqual([
        {
          name: 'myTable',
          isShared: false,
        },
        {
          name: 'ethTable',
          isShared: true,
        },
      ]);
      expect(response.body.unloaded_tables).toBeDefined();
      expect(response.body.unloaded_tables).toEqual(['eth2Table', 'eth3Table']);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });

  describe('Create a shared/local table in a database', () => {
    const dbName = 'ethDB';
    const requestBody = {
      tableName: 'ethTable',
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
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' already exist",
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' already exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables`).send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/load', () => {
  describe('Load shared tables', () => {
    const dbName = 'ethDB';
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).post(`/api/databases/${dbName}/tables/load`);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connection lost',
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables/load`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual('connection lost');
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables/load`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  describe('Get a tables type and columns', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([{ isShared: 1 }])
        .mockResolvedValueOnce([
          {
            Field: 'id',
            Type: 'int',
            Key: 'PRI',
            Null: 'NO',
          },
          {
            Field: 'name',
            Type: 'varchar(20)',
            Key: '',
            Null: 'YES',
          },
          {
            Field: 'gender',
            Type: 'char(2)',
            Key: '',
            Null: 'NO',
          },
        ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}`);
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
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });

  describe('Delete a shared/local table', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).delete(`/api/databases/${dbName}/tables/${tableName}`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/load', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  describe('Load a shared table', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/load`);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/load`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post(`/api/databases/${dbName}/tables/${tableName}/load`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/tuples', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  describe('Get columns and tuples of a table', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([{ isShared: 1 }])
        .mockResolvedValueOnce([
          {
            Field: 'id',
            Type: 'int',
            Key: '',
            Null: 'YES',
          },
          {
            Field: 'name',
            Type: 'varchar(20)',
            Key: '',
            Null: 'YES',
          },
        ]).mockResolvedValueOnce([
          {},
          [
            {
              id: 0,
              name: 'Sam',
            },
            {
              id: 1,
              name: 'Max',
            },
          ],
        ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
      expect(response.statusCode).toBe(200);
      expect(response.body.isShared).toBeDefined();
      expect(response.body.isShared).toEqual(true);
      expect(response.body.columns).toBeDefined();
      expect(response.body.columns).toEqual([
        {
          name: 'id',
          type: 'int',
          isPrimaryKey: false,
          isNotNull: false,
        },
        {
          name: 'name',
          type: 'varchar(20)',
          isPrimaryKey: false,
          isNotNull: false,
        },
      ]);
      expect(response.body.tuples).toBeDefined();
      expect(response.body.tuples).toEqual([
        {
          id: 0,
          name: 'Sam',
        },
        {
          id: 1,
          name: 'Max',
        },
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/tuples`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });

  describe('Insert values into a table', () => {
    const requestBody = {
      tuples: [
        {
          id: 0,
          name: 'Sam',
        },
        {
          id: 1,
          name: 'Max',
        },
      ],
    };
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).put(`/api/databases/${dbName}/tables/${tableName}/tuples`).send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/history/states', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  describe('Get all history states of a table', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          datetime: '2022-07-29 08:55:09',
        },
        {
          datetime: '2022-07-29 08:56:58',
        },
      ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states`);
      expect(response.statusCode).toBe(200);
      expect(response.body.states).toBeDefined();
      expect(response.body.states).toEqual(['2022-07-29 08:55:09', '2022-07-29 08:56:58']);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/history/states/{state}', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  const state = 230;
  describe('Get table history at state', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          datetime: '2022-01-20T07:37:23.000Z',
          blockNumber: 201,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '0',
          dataValue: 'Sam',
        },
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '1',
          dataValue: 'Max',
        },
      ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states/${state}`);
      expect(response.statusCode).toBe(200);
      expect(response.body.tuples).toBeDefined();
      expect(response.body.tuples).toEqual([
        {
          datetime: '2022-01-20T07:37:23.000Z',
          blockNumber: 201,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '0',
          dataValue: 'Sam',
        },
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '1',
          dataValue: 'Max',
        },
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states/${state}`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/states/${state}`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/history/tuples/{primaryKeyHash}/changes', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  const primaryKeyHash = '6466336636313938303461393...';
  describe('Get history of a tuple', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          primaryKeyHash: '6466336636313938303461393...',
          dataValue: '1, Max',
        },
      ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/tuples/${primaryKeyHash}/changes`);
      expect(response.statusCode).toBe(200);
      expect(response.body.tuple_states).toBeDefined();
      expect(response.body.tuple_states).toEqual([
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          primaryKeyHash: '6466336636313938303461393...',
          dataValue: '1, Max',
        },
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/tuples/${primaryKeyHash}/changes`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/tuples/${primaryKeyHash}/changes`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/tables/{tableName}/history/editors/{editor}/changes', () => {
  const dbName = 'ethDB';
  const tableName = 'ethTable';
  const editor = '0xC5D459235a401603a4A742a14cAc30fE...';
  describe('Get history of an editor', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          datetime: '2022-01-20T07:37:23.000Z',
          blockNumber: 201,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '0',
          dataValue: 'Sam',
        },
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '1',
          dataValue: 'Max',
        },
      ]);
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/editors/${editor}/changes`);
      expect(response.statusCode).toBe(200);
      expect(response.body.changes).toBeDefined();
      expect(response.body.changes).toEqual([
        {
          datetime: '2022-01-20T07:37:23.000Z',
          blockNumber: 201,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '0',
          dataValue: 'Sam',
        },
        {
          datetime: '2022-01-20T07:39:23.000Z',
          blockNumber: 230,
          editor: '0xC5D459235a401603a4A742a14cAc30fE...',
          transactionId: '0xf7b66f49cdab51cdf8ea31263...',
          dataKey: '1',
          dataValue: 'Max',
        },
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/editors/${editor}/changes`);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).get(`/api/databases/${dbName}/tables/${tableName}/history/editors/${editor}/changes`);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/invite', () => {
  const dbName = 'ethDB';
  const requestBody = {
    publicKey: 'MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICC==',
  };
  describe('Get invite file content for a given database', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({})
        .mockResolvedValueOnce([
          {
            database_name: 'ethDB',
            encrypted_key: 'a93874c6890089b14298d9a0e6c786304e97dad84',
            encrypted_iv: 'ac16604916fcdf5bc939a323eafdf7f20b72fa99',
            encrypted_invite: '2501dfb2a04ca533c1fedc848442c2472b690754a92c9e7a5b',
          },
        ]);
      const response = await request(app).post(`/api/databases/${dbName}/invite`).send(requestBody);
      expect(response.statusCode).toBe(200);
      expect(response.body.inviteFileContent).toBeDefined();
      expect(response.body.inviteFileContent).toEqual({
        dbName: 'ethDB',
        key: 'a93874c6890089b14298d9a0e6c786304e97dad84',
        iv: 'ac16604916fcdf5bc939a323eafdf7f20b72fa99',
        invite: '2501dfb2a04ca533c1fedc848442c2472b690754a92c9e7a5b',
      });
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' not exist",
      });
      const response = await request(app).post(`/api/databases/${dbName}/invite`).send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post(`/api/databases/${dbName}/invite`).send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/databases/{dbName}/load', () => {
  describe('Load a shared database based on invite file content', () => {
    const dbName = 'ethDB';
    const requestBody = {
      inviteFileContent: {
        key: 'a93874c6890089b14298d9a0e6c786304e97dad84',
        iv: 'ac16604916fcdf5bc939a323eafdf7f20b72fa99',
        invite: '2501dfb2a04ca533c1fedc848442c2472b690754a92c9e7a5b',
      },
    };
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce({});
      const response = await request(app).post(`/api/databases/${dbName}/load`).send(requestBody);
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Database 'ethDB' already exist",
      });
      const response = await request(app).post(`/api/databases/${dbName}/load`).send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Database 'ethDB' already exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post(`/api/databases/${dbName}/load`).send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});

describe('/api/sqlServer/status', () => {
  describe('Get availability status of SQL sever', () => {
    test('Should respond with a 200 status code', async () => {
      databaseAlive.mockResolvedValueOnce(true);
      const response = await request(app).get('/api/sqlServer/status');
      expect(response.statusCode).toBe(200);
    });

    test('Should respond with a 500 status code', async () => {
      databaseAlive.mockResolvedValueOnce(false);
      const response = await request(app).get('/api/sqlServer/status');
      expect(response.statusCode).toBe(500);
    });
  });
});

describe('/api/sql', () => {
  const requestBody = {
    sqlCommand: 'USE ethDB; SELECT * FROM ethTable',
  };
  describe('Execute a SQL statement', () => {
    test('Should respond with a 200 status code', async () => {
      query.mockResolvedValueOnce([
        {
          fieldCount: 0,
          affectedRows: 0,
          insertId: 0,
          info: '',
          serverStatus: 16394,
          warningStatus: 0,
          stateChanges: {
            systemVariables: {},
            schema: 'ethDB',
            trackStateChange: null,
          },
        },
        [
          {
            id: 0,
            name: 'Sam',
          },
          {
            id: 1,
            name: 'Max',
          },
        ],
      ]);
      const response = await request(app).post('/api/sql').send(requestBody);
      expect(response.statusCode).toBe(200);
      expect(response.body).toEqual([
        {
          fieldCount: 0,
          affectedRows: 0,
          insertId: 0,
          info: '',
          serverStatus: 16394,
          warningStatus: 0,
          stateChanges: {
            systemVariables: {},
            schema: 'ethDB',
            trackStateChange: null,
          },
        },
        [
          {
            id: 0,
            name: 'Sam',
          },
          {
            id: 1,
            name: 'Max',
          },
        ],
      ]);
    });

    test('Should respond with a 400 status code', async () => {
      query.mockRejectedValueOnce({
        message: "Table 'ethTable' not exist",
      });
      const response = await request(app).post('/api/sql').send(requestBody);
      expect(response.statusCode).toBe(400);
      expect(response.body.message).toEqual("Table 'ethTable' not exist");
    });

    test('Should respond with a 500 status code', async () => {
      query.mockRejectedValueOnce({
        message: 'connect ECONNREFUSED 127.0.0.1:3305',
        code: 'ECONNREFUSED',
      });
      const response = await request(app).post('/api/sql').send(requestBody);
      expect(response.statusCode).toBe(500);
      expect(response.body.message).toEqual('connect ECONNREFUSED 127.0.0.1:3305');
    });
  });
});
