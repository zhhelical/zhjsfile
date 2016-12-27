//app.js
"use strict"

const ws = require('ws')
const request = require('request')
const client = require('./service/client.js')
const clients = require('./service/clients.js')
const mysql = require('./service/mysqldata.js')
const redis = require('./service/rediscache.js')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})

wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        var mData = JSON.parse(message)
        if(mData.key == 'sessionKey'){
            var double_leap = '//'
            var options = {
                url: `https:${double_leap}api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.content}&grant_type=authorization_code`,
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body)
                    var local_session = session_random.shellFunc.then(function(value){
                        var feedback = {key:mData.key, content:value}
                        var send_str = JSON.stringify(feedback)
                        conn.send(send_str)
                    }).then(function(err){
                        var feedback = {key:mData.key, content:err}
                        var send_str = JSON.stringify(feedback)
                        conn.send(send_str)
                    })
                }
                else{
                    var feedback = {key:mData.key, content:'发现未知错误'}
                    conn.send(JSON.stringify(feedback))
                }
            })

        }
        else if(mData.key == 'submitInfo'){
            var log_client = new client(mData)
        }
        else if(mData.key == 'ownedKey'){
            var joinor = clients.findEvent(ownedKey)
            joinor.dealData(mData.content)
        }

    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    })
})
