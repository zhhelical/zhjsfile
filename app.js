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
                    if(!mysql.private_sql)
                        mysql.connectDb()
                    mysql.newComerTest(`${info.openid}`, function(data) {
                        if (data.length) {
                            var has_keys = JSON.parse(data)
                            var feedback = {key: mData.key, content: has_keys.localKey}
                            conn.send(JSON.stringify(feedback))
                        }
                        else {
                            session_random.shellFunc.then(function (value) {
                                var feedback = {key: mData.key, content: value}
                                info.localKey = value
                                var redis_keys = JSON.stringify(info)
                                if (!redis.redis_srv)
                                    redis.construct(redis_keys, 'test')
                                var send_str = JSON.stringify(feedback)
                                conn.send(send_str)
                            }).then(function (err) {
                                var feedback = {key: mData.key, content: err}
                                conn.send(JSON.stringify(feedback))
                            })
                        }
                    })
                }
                else{
                    var feedback = {key:mData.key, content:'网站升级中...'}
                    conn.send(JSON.stringify(feedback))
                }
            })
        }
        else if(mData.key == 'submitInfo'){
            redis.getCacheKey(`*${mData.content.localKey}*`, function(res){
                var srv_keys = res
                redis.getCacheValue(srv_keys, function(res){
                    if(res){
                        delete mData.content.localKey
                        redis.storeCache(srv_keys, JSON.stringify(mData.content))
                        mysql.query_order.push(srv_keys)
                        mysql.query_order.push(JSON.stringify(mData.content))
                        mysql.insertText.then(function(res){
                            conn.send(JSON.stringify({key:'submitInfo', content:'submit received'}))
                            mysql.query_order.length = 0
                        }).then(function(err){
                            conn.send(JSON.stringify({key:'submitInfo', content:'submit failed'}))
                            mysql.query_order.length = 0
                        })
                        //conn.send(JSON.stringify({key:'submitInfo', content:'submit received'}))
                    }
                })
            })
        }
        else if(mData.key == 'getSubmits'){
            if(!redis.redis_srv)
                redis.construct()
            redis.getCacheKey(`*${mData.content}*`, function(reply){
                var get_key = JSON.parse(reply)
                if(get_key.localKey==mData.content){
                    redis.getCacheValue(JSON.stringify(get_key), function(reply){
                        if(reply != 'test') {
                            var info_submitteds = reply
                            conn.send(JSON.stringify({key: 'getSubmits', content: info_submitteds}))
                        }
                    })
                }
            })
        }
        else if(mData.key == 'checKey'){
            if(!redis.redis_srv)
                redis.construct()
            redis.getCacheKey(`*${mData.content}*`, function(reply){
                var get_key = JSON.parse(reply)
                if(get_key.localKey==mData.content){
                    redis.getCacheValue(JSON.stringify(get_key), function(reply){
                        if(reply != 'test')
                            conn.send(JSON.stringify({key:'checKey', content:'old user'}))
                        else
                            conn.send(JSON.stringify({key:'checKey', content:'new user'}))
                    })
                }
            })
        }
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    })
})
