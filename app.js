//app.js
"use strict"

const ws = require('ws')
const request = require('request')
const client = require('./service/clients.js')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})

wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        //传输内容为JSON
        var mData = JSON.parse(message)
        var feedback = {}
        if(mData.key == 'code'){
            var options = {
                url: `https://api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.content}&grant_type=authorization_code`,
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body)
                    console.log(info)
                    var local_session = session_random('head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168')
                    console.log(local_session)
                }
            })
            feedback = {key:'sessionKey', content:local_session}
        }
        var send_str = JSON.stringify(feedback)
        conn.send(send_str)
        var log_client = new client(mData)
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    })
})
