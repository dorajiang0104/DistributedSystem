// To simplify the process
// we will use the a torrent file and call the tracker
// to get the peer ip and store it into memory

const fs = require('fs')
const path = require('path')
const parseTorrent = require('parse-torrent')
const Tracker = require('bittorrent-tracker')
const { get_id } = require('./util')

module.exports = (emitter) => {
  const torrentPath = path.resolve(__dirname, '..', 'test.torrent')
  const torrentFile = fs.readFileSync(torrentPath)
  
  const torrent = parseTorrent(torrentFile)

  const requiredOpts = {
    infoHash: torrent.infoHash,
    peerId: get_id(),
    announce: torrent.announce,
    port: 6881,
  }

  const tracker = new Tracker(requiredOpts)

  const ips = []

  tracker.start();
  tracker.on('peer', function (data) {
    // addPeer(fileId, {id, name: torrent.name, peer: data});
    ips.push(data)
    emitter.emit('data',
      torrent.infoHash,
      Array.from(new Set(ips))
    )
  })
}
