//app.js
"use strict"

const ws = require('ws')
const request = require('request')
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
                        info.localKey = value
                        var redis_keys = JSON.stringify(info)
                        if(!redis.redis_srv)
                            redis.construct(JSON.stringify({key:redis_keys, content:null}))
                        var get_older = redis.isOldDriver(info.openid)
                        if(get_older)
                            redis.storeCache(JSON.stringify({key: redis_keys, content: get_older.data}))
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
            var cache_older = redis.getCacheValue(mData.content.key)
            if(cache_older){
                var srv_keys = redis.getCacheKey(mData.content.key)
                delete mData.content.key
                redis.storeCache(JSON.stringify({key: srv_keys, content: mData.content}))
                console.log(redis.getCacheValue(srv_keys))
            }
            conn.send('received submit')
        }
        else if(mData.key == 'ownedKey'){
            var joinor = clients.findEvent(ownedKey)
            joinor.dealData(mData.content)
        }
        else if(mData.key == 'checKey'){
            if(!redis.redis_srv)
                redis.construct()
            var redis_data = redis.getCacheValue(mData.content)
            if(redis_data)
                conn.send(JSON.stringify(redis_data))
            else{
                if(!mysql.private_sql)
                    mysql.connectDb()
                var res = mysql.select(mData.content)
                if(res){

                }
            }
        }
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    })
})
