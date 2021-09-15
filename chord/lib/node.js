// create chord node
const bodyParser = require('body-parser')
const express = require('express')
const ip = require('ip')
const { networkInterfaces } = require('os')
const AbortController = require("abort-controller")
const fetch = require('node-fetch')
const EventEmitter = require('events')
const entries = require('object.entries')
const fromEntries = require('object.fromentries')
const isEmpty = require('lodash.isempty')
const sha1 = require('simple-sha1')
const util = require('./util')
const data_bootstraper = require('./data-bootstrap')

const app = express()
const nets = networkInterfaces()
const controller = new AbortController()
const signal = controller.signal
class Emitter extends EventEmitter {}
const emitter = new Emitter()
let timestamp = 0
// Since the chord is formed as a circle
// and we only sync up time with a node's successor
// we will set one node's time stamp as the standard time
// time for the other nodes will be discard
const standard_time_id = 1
let time_id

module.exports = ({ port, id, m }) => {
  let self = {
    meta: {
      id,
      port,
      ip: ip.address(),
    },
    predecessor: null,
    successor: null,
    finger: {}
  }

  let records = {} // stored data
  let backup = {} // back up data, this will be push into active records again when predecessor lost connection

  let stabilize_timer
  let fix_finger_timer
  let check_predecessor_timer
  let sync_record_timer
  let timestamp_timer

  app.use(bodyParser.json())

  // find_successor()
  const find_successor = async (key) => {
    if (self.successor && util.in_left_open_range(key, self.meta.id, self.successor.id)) {
      return self.successor
    }

    const node = closest_preceding_finger(key)
    try {
      const successor = await find_successor_from(node, key)
      return successor
    } catch (e) {
      console.log('find_successor: ', e.message)
      return null
    }
  }

  // call remote api to find key's successor 
  const find_successor_from = async (remote, key) => {
    if (remote.id === self.meta.id) {
      return self.successor
    }

    let res = null

    // we use http protocol
    try {
      res_raw = await fetch(`http://${util.build_url(remote)}/successor?key=${key}`, {
        method: 'GET'
      })
      res = res_raw.json()
    } catch (e) {
      console.log('find_successor_from: ', e.message)
    }

    return res
  }

  const find_predecessor_from = async (remote) => {
    if (remote.id === self.meta.id) {
      return self.predecessor
    }

    let res = null

    try {
      // we use http protocol
      res_raw = await fetch(`http://${util.build_url(remote)}/predecessor`, {
        method: 'GET'
      })
      res = res_raw.json()
    } catch (e) {
      console.log('find_predecessor_from: ', e.message)
    }

    return res
  }

  // closest_preceding_finger()
  const closest_preceding_finger = (key) => {
    const { id } = self.meta;
    if (Object.keys(self.finger) === m) {
      for (let i = m; i >= 1; i --) {
        if (util.in_open_range(self.finger[i].node.id), id, key) {
          return self.finger[i].node
        }
      }
    } else if (self.successor && util.in_open_range(self.successor.id, id, key)) {
        return self.successor;
    }
    return self.meta
  }

  const notify = (node) => {
    if (self.predecessor === null || util.in_open_range(node.id, self.predecessor.id, self.meta.id)) {
      self.predecessor = node
    }
  }

  const notify_from = (remote, node) => {
    const timeout = setTimeout(() => {
        controller.abort();
      }, 3000)

    try {
      return fetch(`http://${util.build_url(remote)}/notify`, {
        method: 'POST',
        body: JSON.stringify({ node }),
        headers: {
          'content-type': 'application/json'
        },
        signal: controller.signal
      }).catch(e => {
        console.log('notify_from: ', e.message)
      })
    } catch (e) {
      console.log('notify_from: ', e.message)
    } finally {
      clearTimeout(timeout)
    }
  }

  const add_new_record = (key, value, tm) => {
    if (records[key]){
      if (tm > records[key].timestamp) {
        records[key] = { value, timestamp: tm }
        return true
      }
      return false
    } else {
      records[key] = { value, timestamp: tm }
      return true
    }
  }

  const add_to_backup = (key, value) => {
    if (backup[key]) {
      if (value.timestamp > backup[key].timestamp ) {
        backup[key] = value
      }
    } else {
      backup[key] = value
    }
  }

  const migrate_data_to = (remote, data) => {
    return fetch(`http://${util.build_url(remote)}/migrate`, {
      method: 'PUT',
      body: JSON.stringify({ data }),
      headers: {
        'content-type': 'application/json'
      }
    }).catch(e => {
      console.log('migrate_data_to: ', e.message)
      return e
    })
  }

  const get_from = async (remote, hash, with_timestamp = false) => {
    const query = with_timestamp ? '&with_timestamp=true' : ''
    return fetch(`http://${util.build_url(remote)}/get?hash=${hash}${query}`, {
      method: 'GET'
    })
    .then(data => data.json())
    .catch(e => {
      console.log('get: ', e.message)
      return undefined
    })
  }

  const get = async (hash, with_timestamp) => {
    const k = util.calculate_key(hash ,m)
    const node = await find_successor(k)
    let result = {}
    if (node) {
      if (node.id === self.meta.id) {
        if (with_timestamp) {
          if (records[hash]) {
            result[hash] = records[hash]
          } else {
            result[hash] = undefined
            return result
          }
        } else {
          if (records[hash]) {
            result[hash] = records[hash].value
          } else {
            result[hash] = undefined
            return result
          }
        }
      } else {
        result = await get_from(node, hash, with_timestamp)
      }

      return result
    } else {
      result[hash] = undefined
      return result
    }
  } 

  // to check the node data from browser
  app.get('/', (req, res) => {
    res.send({
      timestamp, 
      meta: self.meta,
      predecessor: self.predecessor,
      successor: self.successor,
      finger: self.finger,
    })
  })
  
  // check succcessor for specific key
  app.get('/successor', async (req, res) => {
    // response with successor
    const { key } = req.query
    const successor = await find_successor(key)
    res.send(successor)
  })

  // return the predecessor of this node
  app.get('/predecessor', (req, res) => {
    res.send(self.predecessor ? self.predecessor : { id: null })
  })

  // heartbeat function, should be called by node's successor
  // and send the data to its successor for backing up
  app.get('/heartbeat', (req, res) => {
    const response = { ok: true }
    console.log('server running: ', self.meta)
    if (!isEmpty(records)) {
      response.records = records
    }
    res.send(response)
  })

  // check active record on this node
  app.get('/records', (req, res) => {
    res.send(records)
  })

  // check all data on this node
  app.get('/data', (req, res) => {
    res.send({ records, backup })
  })

  // get data for specify key or known hash
  app.get('/get', async (req, res) => {
    const { key = null, hash = null, with_timestamp = false } = req.query
    if (key) { // get data from key
      const h = sha1.sync(key)
      const result = await get(h, with_timestamp)
      res.send(result)
    } else if (hash) {
      const result = await get(hash, with_timestamp)
      res.send(result)
    }
  })

  // set data for specify key
  app.post('/set', (req, res) => {
    const { body } = req
    entries(body).forEach(([key, value])=> {
      sha1(key, hash => {
        records[hash] = { value, timestamp }
        res.send({ hash })
      })
    })
  })

  // call notify function in this node
  app.post('/notify', (req, res) => {
    const { body } = req
    const { node } = body
    notify(node)
    res.send({ ok: true })
  })

  app.post('/time', (req, res) => {
    //record time when request arrive
    const t2 = timestamp
    const { body } = req

    res.send({
      id: timeid,
      t1: body.t1,
      t2,
      t3: timestamp,
    })
  })

  // receive record from other nodes, 
  // try to get the latest value in dht
  // if the result is newer, alway add to active record
  // data will be migrated again by sync_data function if need

  // the different between migrate and set
  // set will calculate input key's hash and always overwrite the data
  // migrate will directly get hash key from key and check the timestamp
  // if input data is newer, then set it.
  app.put('/migrate', (req, res) => {
    const { body } = req
    const { data } = body

    const result = entries(data).map(async ([key, value]) => {
      const result = await get(key, true)
      if (result[key]) {
        if (result[key].timestamp < value.timestamp) {
          add_new_record(key, value.value, value.timestamp)
          res.send({ success: true })
        } else {
          add_new_record(key, result[key].value, result[key].timestamp)
          res.send({ success: false, message: 'found data with latest timestamp' })
        }
      } else {
        add_new_record(key, value.value, value.timestamp)
        res.send({ success: true })
      }
    })
  })

  // stabilization code, node will ask its successor for successor for predecessor
  // and check if it could be node's predecessor, it will be run every second on the node
  const stabilize = () => setInterval(() => {
    find_predecessor_from(self.successor)
      .then(node => {
        if (node && node.id) {
          if (util.in_open_range(node.id, self.meta.id, self.successor.id)) {
            self.successor = node
          } 
        }
        notify_from(self.successor, self.meta)
      })
      .catch(e => {
        console.log('stabilize: ', e.message)
      })
  }, 1000).unref()

  // fix finger table, it will be run every second on the node
  let next = 0
  const fix_fingers = () => setInterval(async () => {
    next = next + 1
    if (next > m) {
      next = 1
    }
    self.finger[next] = self.finger[next]? self.finger[next] : {}
    const start = util.get_finger_start(self.meta.id, next, m)
    self.finger[next].start = start
    self.finger[next].node = await find_successor(start)
  }, 1000).unref()

  // ping predecessor every second to see if it is still alive
  // in our application, predecessor will also send back its data
  // node will store the data into backup data group
  let predecessor_live_count = 10
  const check_predecessor = () => setInterval(async () => {
    if (self.predecessor && self.predecessor.id !== self.meta.id) {
      const timeout = setTimeout(() => {
        controller.abort();
      }, 3000)

      try {
        const res = await fetch(`http://${util.build_url(self.predecessor)}/heartbeat`, {
          method: 'GET',
          signal: controller.signal
        })
        const data = await res.json()
        if (data.ok) {
          predecessor_live_count = 10
          if (data.records) {
            entries(data.records).forEach(([key, value]) => {
              add_to_backup(key, value)
            })
          }
        }
      } catch (e) {
        predecessor_live_count = predecessor_live_count - 1
        if (predecessor_live_count === 0 ) {
          console.log('lost connection to ', `http://${util.build_url(self.predecessor)}`, 'reset predecessor')
          // if lost connection, reset the predecessor
          // also, push all the records from backup into active record and re sync up again to prevent record lost
          // this may result to a data rolling back
          self.predecessor = null
          self.successor = self.meta
          predecessor_live_count = 10

          entries(backup).forEach(([key,value]) => {
            add_new_record(key, value.value, value.timestamp)
          })
          backup = {} // clear back up
        }
      } finally {
        clearTimeout(timeout)
      }
    }
  }, 1000).unref()

  // check if the record should be stored in this node
  // if data should be migrated, send the record to node's successor and push the data into node's backup records
  const sync_record = () => setInterval(async () => {
    // find one record which should be migrated
    let foundKey
    let foundNode
    let foundValue
    const entriesArray = entries(records)
    for (let i = 0; i < entriesArray.length; i++) {
      const [key, value] = entriesArray[i]
      const k = util.calculate_key(key, m)
      const node = await find_successor(k)

      if (node && node.id !== self.meta.id) {
        foundKey = key
        foundValue = value
        foundNode = node
        break
      }
    }

    if (foundKey && foundNode) {
      migrate_data_to(foundNode, fromEntries([[foundKey, foundValue]]))
        .then(res => {
          if (res.ok) {
            delete records[foundKey]
          }
        })
    }
  }, 1000).unref()


  // increase timestamp every 0.1 second
  const start_timestamp = () => setInterval(() => {
    timestamp = timestamp + 1
  }, 100).unref()

  const sync_up_time = () => setInterval(() => {
    // only set sync up time when presecessor is existed
    if (self.predecessor && self.predecessor.id !== self.meta.id) {
      fetch(`http://${util.build_url(self.predecessor)}/time`, {
        method: 'POST',
        body: JSON.stringify({ t1: timestamp }),
        headers: {
          'content-type': 'application/json' 
        }
      })
      .then(res => res.json())
      .then(data => {
        if (data.id === standard_time_id) {
          const t4 = timestamp
          const { t1, t2, t3 } = data
          timestamp = timestamp + ((t2 - t1) + (t3 - t4)) / 2
          timeid = standard_time_id
        }
      })
      .catch(e => {
        console.log(e.message)
      })
    }
  }, 1000)

  emitter.on('data', (key, value) => {
    add_new_record(key, value, timestamp)
  })

  self.start = () => {
    app.listen(port, async () => {
      console.log('Start Chord..')
      console.log(`Node ${id} running at port ${self.meta.ip}:${port}..`)
      self.successor = self.meta

      stablize_timer = stabilize()
      fix_finger_timer = fix_fingers()
      check_predecessor_timer = check_predecessor()
      sync_record_timer = sync_record()
      timeid = id
      timestamp_timer = start_timestamp()

      sync_up_time()
      data_bootstraper(emitter)
    })
  }

  self.join = (remote) => {
    app.listen(port, async () => {
      console.log(`Node ${id} running at ${self.meta.ip}:${port}..`)
      console.log(`Join Chord from Node ${remote.id}(${remote.ip}:${remote.port})..`)
      try {
        self.successor = await find_successor_from(remote, remote.id)
        console.log(`Found successor: Node ${self.successor.id}(${self.successor.ip}:${self.successor.port})..`)
        if (self.successor.id === self.meta.id) {
          console.log('A node with same id already existed in this DHT', self.successor)
          process.exit(1)
        }

        stablizeTimer = stabilize()
        fix_finger_timer = fix_fingers()
        check_predecessor_timer = check_predecessor()
        sync_record_timer = sync_record()
        timeid = id
        timestamp_timer = start_timestamp()
        sync_up_time()
      } catch (e) {
        console.log('No peer found in this Chord, please run a node without the -j|--join argument.')
        process.exit(1)
      }
    })
  }

  return self
}
