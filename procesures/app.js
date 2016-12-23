//app.js
"use strict"

const p_server = require('ws')

var https_server = require('./httpssrv.js')
var wss_server = new ws.server({server: https_server})
wss_server.on('connection', function(conn){

})
