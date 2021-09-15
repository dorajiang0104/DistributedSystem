const crypto = require('crypto')

// check if key is in (low, high) 
const in_open_range = (key, low, high) => {
  if (low === high) {
    return key !== low
  }

  if (low < high) {
    return key > low && key < high
  }

  if (low > high) {
    return key > low || key < high
  }
}

// check if key is in (low, high]
const in_left_open_range = (key, low, high) => {
  if (low === high) {
    return true
  }

  if (low < high) {
    return key > low && key <= high
  }

  if (low > high) {
    return key > low || key <= high
  }
}


// check if key is in [low, high)
const in_right_open_range = (key, low, high) => {
  if (low === high) {
    return true
  }

  if (low < high) {
    return key >= low && key < high
  }

  if (low > high) {
    return key >= low || key < high
  }
}

const mod = (n, m) => {
  return ((n % m) + m) % m;
}

const get_finger_start = (n, i, m) => {
  return mod((n + Math.pow(2, i - 1)), Math.pow(2, m))
}

const build_url = node => {
  return node.port ? `${node.ip}:${node.port}` : node.ip
}

let peer_id
const get_id = () => {
  if (!peer_id) {
    peer_id = crypto.randomBytes(20)
    Buffer.from('-AT0001-').copy(peer_id, 0)
  }
  return peer_id;
}

//we use the first digit of hash key as the key of the data record
const calculate_key = (hash, m) => {
  const d = parseInt(hash[0], 16)
  return mod(d, Math.pow(2, m))
}

module.exports.in_open_range = in_open_range
module.exports.in_left_open_range = in_left_open_range
module.exports.build_url = build_url
module.exports.get_finger_start = get_finger_start
module.exports.get_id = get_id
module.exports.calculate_key = calculate_key