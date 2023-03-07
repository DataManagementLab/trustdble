module.exports = {
  root: true,
  env: {
    node: true,
    browser: true
  },
  parser: 'vue-eslint-parser',
  parserOptions: {
    parser: 'babel-eslint',
    sourceType: 'module'
  },
  extends: [
    'plugin:vue/recommended',
    "@vue/standard",
    'eslint:recommended'
  ],
  rules: {
    'vue/component-name-in-template-casing': ['error', 'PascalCase']
  }
}