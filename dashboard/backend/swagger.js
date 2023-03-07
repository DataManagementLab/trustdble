const swaggerUi = require('swagger-ui-express');
const swaggerJSDoc = require('swagger-jsdoc');

const swaggerDefinition = {
  openapi: '3.0.0',
  info: {
    description: 'This is an api for the TRUSTDBLE dashboard',
    version: '0.2.0',
    title: 'TRUSTDBLE Dashboard API',
    contact: {
      name: 'TRUSTDBLE',
      url: 'https://trustdble.com',
    },
  },
};

const options = {
  swaggerDefinition,
  // Paths to files containing OpenAPI definitions
  apis: ['./routes/*.js'],
};

const swaggerSpec = swaggerJSDoc(options);

module.exports = {
  swaggerUi,
  swaggerSpec,
};
