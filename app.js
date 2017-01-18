//app.js
"use strict"

const ws = require('ws')
const request = require('request')
const mysql = require('../../databases/mysqldata.js')
const redis = require('../../databases/rediscache.js')
const co = require('co')
const joiners = require('./service/clients.js')
const txgeo = require('./service/txgeo.js')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})

wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        var chk_time = new Date()
        joiners.memoryCacheChk(chk_time).then(function(res){
            if(res.length)
                redis.clearMemory(res)
        })
        var mData = JSON.parse(message)
        if(mData.key == 'sessionKey'){
            var double_leap = '//'
            var options = {
                url: `https:${double_leap}api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.value}&grant_type=authorization_code`,
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body)
                    if (!redis.redis_srv)
                        redis.construct()
                    redis.getCacheKey(`*${info.openid}*`, function(r_key){
                        if(!r_key || !r_key.length){
                            mysql.newComerTest(`${info.openid}`, function(key) {
                                if (key.length) {
                                    var key_obj = JSON.parse(key)
                                    joiners.joinEvent(key_obj.localKey, conn, true, false, function(j_res){
                                        var feedback = {key: mData.key, value: j_res.value}
                                        if(j_res.key == 'join success') {
                                            var send_str = JSON.stringify(feedback)
                                            conn.send(send_str)
                                        }
                                        else{
                                            feedback = {key: mData.key, value: 'sessionKey failed'}
                                            conn.send(JSON.stringify(feedback))
                                        }
                                    })
                                }
                                else {
                                    var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
                                    session_random.shellFunc(sh_order).then(function (result) {
                                        var feedback = {key: mData.key, value: result}
                                        var first_store = {}
                                        first_store.localKey = result
                                        first_store.openid = info.openid
                                        var start_time = new Date()
                                        first_store.origintime = start_time.getTime()
                                        joiners.joinEvent(JSON.stringify(first_store), conn, false, false, function(f_res){
                                            var send_str = JSON.stringify(feedback)
                                            conn.send(send_str)
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
                            if(r_key != 'ERRGETKEY') {
                                var key_obj = JSON.parse(r_key)
                                joiners.joinEvent(key_obj.localKey, conn, true, true, function(j_res){
                                    var feedback = {key: mData.key, value: j_res.value}
                                    if(j_res.key == 'join success') {
                                        var send_str = JSON.stringify(feedback)
                                        conn.send(send_str)
                                    }
                                    else{
                                        feedback = {key: mData.key, value: 'sessionKey failed'}
                                        conn.send(JSON.stringify(feedback))
                                    }
                                })
                            }
                            else{
                                var feedback = {key: mData.key, value: 'sessionKey failed'}
                                conn.send(JSON.stringify(feedback))
                            }
                        }
                    })
                }
                else
                    conn.send(JSON.stringify({key:mData.key, value:'网站升级中...'}))
            })
        }
        else if(mData.key == 'submitInfo'){
            txgeo(mData.value.nest.location, function (c_res) {
                if(c_res != 'error') {
                    var detail_addr = c_res.split('市')[0] + '市'
                    mData.value.nest.address = detail_addr
                    redis.getCacheKey(`*${mData.value.localKey}*`, function (key) {
                        if(key != 'ERRGETKEY') {
                            if (key.length) {
                                redis.getCacheValue(key, function (val) {
                                    if (val != 'ERRGETVALUE') {
                                        var cache_objs = JSON.parse(val)
                                        cache_objs.push(mData.value.nest)
                                        redis.storeCache(key, JSON.stringify(cache_objs)).then(function (res) {
                                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit received'}))
                                        }).then(function (err) {
                                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                        })
                                    }
                                    else
                                        conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                })
                            }
                            else
                                redis.storeCache(key, JSON.stringify(mData.value.nest)).then(function (res) {
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit received'}))
                                }).then(function (err) {
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                })
                        }
                        else
                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                    })
                }
                else
                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
            })

        }
        else if(mData.key == 'getSubmits'){
            var which_gate = mData.value.gate
            redis.getCacheKey(`*${mData.value.localKey}*`, function(key){
                if(key != 'ERRGETKEY') {
                    redis.getCacheValue(key, function (cacheres) {
                        if(cacheres != 'ERRGETVALUE'){
                            var chk_obj = JSON.parse(cacheres)
                            if (chk_obj.length) {
                                var send_array = []
                                var test_location
                                for(var ech in chk_obj){
                                    if(ech == 0)
                                        test_location = chk_obj[ech].location
                                    if(which_gate == chk_obj[ech].content.add_key)
                                        send_array.push(chk_obj[ech])
                                }
                                if(send_array.length)
                                    conn.send(JSON.stringify({key: 'getSubmits', value: send_array}))
                                else
                                    conn.send(JSON.stringify({key: 'getSubmits', value: 'no submits now'}))
                            }
                            else
                                conn.send(JSON.stringify({key: 'getSubmits', value: 'no submits now'}))
                        }
                        else
                            conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
                    })
                }
                else
                    conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
            })
        }
        else if(mData.key == 'getImagePos'){
            mysql.select_picpos(mData.value.time, mData.value.key).then(function(res_poses){
                console.log(res_poses)
                var pos = JSON.parse(res_poses)[0]
                conn.send(JSON.stringify({key: 'getImagePos', value: pos}))
            }).then(function(res_err){
                conn.send(JSON.stringify({key: 'getImagePos', value: 'failed getImagePos'}))
            })
        }
        else if(mData.key == 'payRequest'){
            if(!redis.redis_srv)
                redis.construct()
            redis.getCacheKey(`*${mData.value.localKey}*`, function(key){
                var open_id = JSON.parse(key).openid
                if(get_key.localKey==mData.value){
                    redis.getCacheValue(key, function(res){
                        var chk_obj = JSON.parse(res)
                        if(chk_obj[0].time != 'test')
                            conn.send(JSON.stringify({key:'checKey', value:'old user'}))
                        else
                            conn.send(JSON.stringify({key:'checKey', value:'new user'}))
                    })
                }
            })
        }
        else if(mData.key == 'searchPeople') {
            var lat = mData.value.latitude, lng = mData.value.longitude, scope = mData.value.scope
            var chk_local = {latitude: mData.value.latitude, longitude: mData.value.longitude}
            txgeo(chk_local, function(res){
                var detail_addr = res.split('市')[0]+'市'
                redis.searchScope(`*${mData.value.localKey}*`, mData.value.latitude, mData.value.longitude, mData.value.scope, {city: detail_addr, gate: mData.value.gate}).then(function (res) {
                    if (res.length)
                        conn.send(JSON.stringify({key: 'searchPeople', value: res}))
                    else
                        conn.send(JSON.stringify({key: 'searchPeople', value: 'no person now'}))
                })
            })
        }
        else if(mData.key == 'getImageInfo') {//? need it?
            mysql.selectImageRow(mData.value).then(function(res){
                conn.send(JSON.stringify(res))
            }).then(function(err){
                if(err != undefined)
                    conn.send(JSON.stringify({key: 'getImageInfo', value: 'get img failed'}))
            })
        }
        else if(mData.key == 'closing') {
            if(mData.value == 'I am leaving')
                joiners.leaveEvent(conn)
        }
    })
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "))
    })
    conn.on('close', function() {
        var key_id = joiners.findEvent(conn)
        if(key_id) {
            redis.getCacheKey(`*${key_id}*`, function (key) {
                redis.getCacheValue(key, function (cacheres) {
                    var cache_objs = JSON.parse(cacheres)
                    if (cache_objs.length > 0) {
                        mysql.text_exec(key).then(function (result) {
                            if (result.length) {
                                if (result != cacheres)
                                    mysql.update_exec(key, cacheres).then(function () {})
                                console.log('mysql update')
                            }
                            else {
                                mysql.insert_exec(key, cacheres).then(function () {
                                    /*if(joiners.online == false) {
                                     redis.deleteData(key, function () {
                                     joiners.leaveEvent(conn)
                                     })
                                     }*/
                                })
                                console.log('mysql insert')
                            }
                        })
                    }
                })
            })
        }
    })
})
