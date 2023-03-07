// With require(), libraries, packages or file can be import.
// Express: is a minimal and flexible Node.js web application framework
// that provides a robust set of features for web and mobile applications.
const express = require('express');

// Middleware to proxy requests through a specified index page,
// useful for Single Page Applications that utilise the HTML5 History API.
const history = require('connect-history-api-fallback');

// Express provides configuration hooks specific to the environment
// which are automatically called based on the NODE_ENV variable value
// which can be 'production' or 'development'
const { swaggerUi, swaggerSpec } = process.env.NODE_ENV === 'development' ? require('./swagger') : {};

// Cross-origin resource sharing (CORS) is a browser security feature
// that restricts cross-origin HTTP requests
const cors = require('cors');
const bodyParser = require('body-parser');

// Import all Architectural layers
const Models = require('./models');
const Services = require('./services');
const Controllers = require('./controllers');
const Routes = require('./routes');

module.exports = (db) => {
  const models = Models(db);
  const services = Services(models);
  const controllers = Controllers(services);
  const routes = Routes(controllers);
  const app = express();

  // Use this piece of middleware with express
  app.use(history());

  // When you POST something greater than 100kb that error will be thrown
  // PayloadTooLargeError: request entity too large
  // So increase the size up to 50MB
  app.use(bodyParser.json({ limit: '50mb' }));
  app.use(bodyParser.urlencoded({ limit: '50mb', extended: true }));

  app.use(cors());
  app.use('/api', routes);

  if (process.env.NODE_ENV === 'production') {
    app.use(express.static('../frontend/dist/')); // Integrate frontend build
  }

  if (process.env.NODE_ENV === 'development') {
    app.use('/api/docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec));
  }

  return app;
};
