let props
if (process.env.NODE_ENV === 'development') {
  const propertiesReader = require('properties-reader')
  props = propertiesReader('../configs/app.properties')
}

module.exports = {
  publicPath: process.env.DEPLOY_ENV === 'GH_PAGES'
    ? '/frontend/'
    : '/',
  lintOnSave: process.env.NODE_ENV === 'development',
  // Remove moment.js from chart.js
  // https://www.chartjs.org/docs/latest/getting-started/integration.html#bundlers-webpack-rollup-etc
  configureWebpack: (config) => ({
    externals: {
      moment: 'moment'
    }
  }),
  devServer: {
    proxy: {
      '^/api': {
        target: process.env.NODE_ENV === 'development' ? `http://localhost:${props.get('main.app.port')}/api` : '',
        changeOrigin: true,
        logLevel: 'debug',
        pathRewrite: { '^/api': '/' }
      }
    }
  }

}
