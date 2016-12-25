//app.js
"use strict"

const ws = require('ws')
const request = require('request')
const client = require('./service/clients.js')

var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})
var wxUrl = 'https://api.weixin.qq.com/sns/jscode2session?appid=APPID&secret=SECRET&js_code=JSCODE&grant_type=authorization_code'
wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        //传输内容为JSON
        var mData = JSON.parse(message)
        if(mData.key == 'code'){
            var options = {
                url: wxUrl,
                headers: {
                    'appid': 'wxf9a75ea1c3517fbe',
                    'secret': '9aceb733968d171ed70207f87c5dcb9e',
                    'js_code': mData.content,
                    'grant_type': 'authorization_code'
                }
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body);
                    console.log(info)
                }
            })
        }
        var log_client = new client(mData)
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    });
})
