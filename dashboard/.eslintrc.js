module.exports = {
  env: {
    es6: true,
    node: true,
  },
  extends: ['airbnb-base', 'eslint:recommended'],
  parserOptions: {
    ecmaVersion: 2020,
    sourceType: 'module',
  },
  rules: {
    'no-underscore-dangle': ["error", { "allowAfterThis": true, "allowAfterThisConstructor": true }],
    'no-console': ["error", { allow: ["info", "debug", "error"] }],
    'no-param-reassign': ["error", { "props": false }],
    'no-restricted-syntax': 0,
    'no-await-in-loop': 0
  }
}