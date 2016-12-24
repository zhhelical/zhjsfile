//app.js
"use strict"

const ws = require('ws')
const client = require('./service/client.js')

var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})
wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        //传输内容为JSON
        var mData = JSON.parse(message)
        var log_client = new client(mData)
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    });
})
