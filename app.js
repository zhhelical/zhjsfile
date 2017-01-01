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
                            var data_obj = JSON.parse(data)
                            var feedback = {key: mData.key, value: data_obj.localKey}
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
                                    if(typeof(err)!='undefined'){
                                        var send_str = JSON.stringify({key: mData.key, value: 'sessionKey failed'})
                                        conn.send(send_str)
                                    }
                                })
                            }).then(function (err) {
                                if(typeof(err)!='undefined'){
                                    var feedback = {key: mData.key, value: 'sessionKey failed'}
                                    conn.send(JSON.stringify(feedback))
                                }
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
            redis.getCacheKey(`*${mData.value.localKey}*`, function(key){
                redis.getCacheValue(key, function(val){
                    if(val){
                        var cache_objs = JSON.parse(val)
                        if(cache_objs.length == 1)
                            cache_objs[0] = mData.value.nest
                        else
                            cache_objs.push(mData.value.nest)
                        redis.storeCache(key, JSON.stringify(cache_objs))
                        mysql.text_exec(key).then(function(result){//这里要变成用户下线时批量插入
                            if(result.length){
                                var sql_objs = JSON.parse(result)
                                var merge = redis.cacheSqlComp(cache_objs, sql_objs)
                                mysql.update_exec(key, merge).then(function(res){
                                    conn.send(JSON.stringify({key:'submitInfo', value:'submit received'}))
                                }).then(function(err){
                                    conn.send(JSON.stringify({key:'submitInfo', value:'submit failed'}))
                                })
                            }
                            else{
                                mysql.insert_exec(key, val).then(function(res){
                                    conn.send(JSON.stringify({key:'submitInfo', value:'submit received'}))
                                }).then(function(err){
                                    conn.send(JSON.stringify({key:'submitInfo', value:'submit failed'}))
                                })
                            }
                        }).then(function(err){
                            if(err)
                                conn.send(JSON.stringify({key:'submitInfo', value:'submit failed'}))
                        })
                    }
                    else
                        conn.send(JSON.stringify({key:'submitInfo', value:'submit failed'}))
                })
            })
        }
        else if(mData.key == 'getSubmits'){
            redis.getCacheKey(`*${mData.value}*`, function(reply){
                if(reply != 'ERRGETKEY') {
                    redis.getCacheValue(reply, function (cacheres) {
                        var cache_obj = JSON.parse(cacheres)
                        if (cache_str[0] != 'test') {
                            mysql.text_exec(reply).then(function(sqlres) {
                                var sql_obj = JSON.parse(sqlres)
                                if (cacheres != sqlres) {
                                    var combines = redis.cacheSqlComp(cache_obj, sql_obj)
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
                    redis.getCacheValue(reply, function(res){
                        if(res[0] != 'test')
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
