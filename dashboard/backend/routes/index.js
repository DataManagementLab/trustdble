const { Router } = require('express');
const databasesRouter = require('./databases');
const datacontractsRouter = require('./datacontracts');
const sqlRouter = require('./sql');
const serverRouter = require('./server');

module.exports = (controllers) => {
  const apiRouter = Router();

  apiRouter.use('/databases', databasesRouter(controllers.DatabasesController));
  apiRouter.use('/datacontracts', datacontractsRouter(controllers.DataContractsController));
  apiRouter.use('/sql', sqlRouter(controllers.SqlController));
  apiRouter.use('/sqlServer', serverRouter(controllers.ServerController));
  return apiRouter;
};
