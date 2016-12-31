//app.js
"use strict"

const ws = require('ws')
const request = require('request')
const mysql = require('./service/mysqldata.js')
const redis = require('./service/rediscache.js')
const co = require('co')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})

wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        var mData = JSON.parse(message)
        if(mData.key == 'sessionKey'){
            var double_leap = '//'
            var options = {
                url: `https:${double_leap}api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.value}&grant_type=authorization_code`,
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body)
                    if(!mysql.private_sql)
                        mysql.connectDb()
                    if (!redis.redis_srv)
                        redis.construct()
                    mysql.newComerTest(`${info.openid}`, function(data) {
                        if (data.length) {
                            var has_keys = JSON.parse(data)
                            var feedback = {key: mData.key, value: has_keys.localKey}
                            conn.send(JSON.stringify(feedback))
                        }
                        else {
                            var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
                            session_random.shellFunc(sh_order).then(function (result) {
                                var feedback = {key: mData.key, value: result}
                                info.localKey = result
                                console.log(result, 'randomok')
                                var redis_keys = JSON.stringify(info)
                                var st_test = JSON.stringify(['test'])
                                redis.storeCache(redis_keys, st_test).then(function(res){
                                    var send_str = JSON.stringify(feedback)
                                    conn.send(send_str)
                                }).then(function(err){
                                    var send_str = JSON.stringify({key: mData.key, value: 'sessionKey failed'})
                                    conn.send(send_str)
                                })
                            }).then(function (err) {
                                console.log(result, 'randomerr')
                                var feedback = {key: mData.key, value: 'sessionKey failed'}
                                conn.send(JSON.stringify(feedback))
                            })
                        }
                    })
                }
                else{
                    var feedback = {key:mData.key, value:'网站升级中...'}
                    conn.send(JSON.stringify(feedback))
                }
            })
        }
        else if(mData.key == 'submitInfo'){
            redis.getCacheKey(`*${mData.value.localKey}*`, function(res){
                var srv_keys = res
                redis.getCacheValue(srv_keys, function(res){
                    if(res){
                        redis.storeCache(srv_keys, JSON.stringify(mData.value.nest))
                        mysql.insert_exec(srv_keys, JSON.stringify(mData.value.nest)).then(function(res){
                            conn.send(JSON.stringify({key:'submitInfo', value:'submit received'}))
                        }).then(function(err){
                            conn.send(JSON.stringify({key:'submitInfo', value:'submit failed'}))
                        })
                    }
                })
            })
        }
        else if(mData.key == 'getSubmits'){
            if(!redis.redis_srv)
                redis.construct()
            redis.getCacheKey(`*${mData.value}*`, function(reply){
                if(reply != 'ERRGETKEY') {
                    redis.getCacheValue(reply, function (cacheres) {
                        if (cacheres[0] != 'test') {
                            mysql.text_exec(reply).then(function(sqlres) {
                                if (cacheres != sqlres) {
                                    var combines = redis.cacheSqlComp(cacheres, sqlres)
                                    conn.send(JSON.stringify({key: 'getSubmits', value: combines}))
                                }
                                else {
                                    var info_submitteds = cacheres
                                    conn.send(JSON.stringify({key: 'getSubmits', value: info_submitteds}))
                                }
                            }).then(function(err){
                                conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
                            })
                        }
                        else
                            conn.send(JSON.stringify({key: 'getSubmits', value: 'no submits now'}))
                    })
                }
                else
                    conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
            })
        }
        else if(mData.key == 'checKey'){
            if(!redis.redis_srv)
                redis.construct()
            redis.getCacheKey(`*${mData.value}*`, function(reply){
                var get_key = JSON.parse(reply)
                if(get_key.localKey==mData.value){
                    redis.getCacheValue(JSON.stringify(get_key), function(reply){
                        if(reply[0] != 'test')
                            conn.send(JSON.stringify({key:'checKey', value:'old user'}))
                        else
                            conn.send(JSON.stringify({key:'checKey', value:'new user'}))
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
