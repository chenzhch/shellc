console.log("0:" + process.argv[1]);
console.log("1:" + process.argv[2]);
console.log("2:" + process.argv[3]);
const fs = require('fs');
const device = fs.createReadStream('/dev/tty');
console.log('input data:');
device.on('data', (data) => {
  console.log(`data is ${data} `);
  process.exit();
});

