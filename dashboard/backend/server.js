const propertiesReader = require('properties-reader');

const props = propertiesReader('../configs/app.properties');
const { DatabaseAdapter } = require('@trustdble-utils/database-adapter');
const makeApp = require('./app');

const db = new DatabaseAdapter();
const app = makeApp(db);
const port = props.get('main.app.port');
const server = app.listen(port, () => {
  const host = server.address().address;
  console.info(`Example app listening at http://${host}:${port}`);
});
