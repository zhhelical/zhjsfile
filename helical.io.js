//helical.io.js

const WxSocketIO = require('./wxsocket.io.js')

var socket = new WxSocketIO()
socket.connect('http://localhost:9595/node_modules/socket.io/?EIO=3&transport=websocket')

