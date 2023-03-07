const { DatabaseAdapter } = require('@trustdble-utils/database-adapter');

const dbConnection = new DatabaseAdapter();
const { monitorBlockchains } = require('./monitor');

const delay = 1 * 1000;

// last state of monitor
const lastProcessedBlocks = {};
// Resources about Nested setTimeout:
// * https://javascript.info/settimeout-setinterval#nested-settimeout
// * https://developer.mozilla.org/en-US/docs/Web/API/setTimeout#reasons_for_delays_longer_than_specified
// * https://developer.mozilla.org/en-US/docs/Learn/JavaScript/Asynchronous/Choosing_the_right_approach#settimeout

setTimeout(async function request() {
  await monitorBlockchains(dbConnection, lastProcessedBlocks);
  setTimeout(request, delay);
}, delay);
