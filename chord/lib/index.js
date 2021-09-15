const node = require('./node')
const path = require('path')
const yargs = require('yargs')
const fs = require('fs')

const entryPath = path.resolve(__dirname, '../', 'entry.json')
const entry = JSON.parse(fs.readFileSync(entryPath).toString('utf8'))

const argv = yargs(process.argv.slice(2))
  .option('id', {
    alias: 'i',
    describe: 'node identifier, value from 0 - 15',
  })
  .option('digits', {
    alias: 'm',
    describe: 'identifier digits',
    default: 4
  })
  .option('port', {
    alias: 'p',
    describe: 'port number',
  })
  .option('join', {
    alias: 'j',
    describe: 'if join to dht',
    type: 'boolean',
  })
  .help().argv;

if (argv.join) {
  node({ port: argv.port, id: argv.id, m: argv.digits }).join(entry)
} else {
  node({ port: argv.port, id: argv.id, m: argv.digits }).start()
}
