//app.js
"use strict"

const ws = require('ws')
const xml2js = require('xml2js')
const request = require('request')
const mysql = require('../../databases/mysqldata.js')
const redis = require('../../databases/rediscache.js')
const co = require('co')
const joiners = require('./service/clients.js')
const txgeo = require('./service/txgeo.js')
const payprocess = require('./service/payunits.js')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})

wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        var mData = JSON.parse(message)
        if(mData.key == 'sessionKey'){
            var double_leap = '//'
            var options = {
                url: `https:${double_leap}api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.value}&grant_type=authorization_code`
            }
            request(options, function(error, response, body){
                if (!error && response.statusCode == 200) {
                    var info = JSON.parse(body)
                    if (!redis.redis_srv)
                        redis.construct()
                    redis.getCacheKey(`*${info.openid}*`).then(function(r_key){
                        if(!r_key || !r_key.length){
                            mysql.newComerTest(`${info.openid}`).then(function(key) {
                                if (key.length) {
                                    var key_obj = JSON.parse(key)
                                    joiners.joinEvent(key_obj, conn, true, false, function(j_res){
                                        var feedback = {key: mData.key, value: j_res.value}
                                        if(j_res.key == 'join success') {
                                            var send_str = JSON.stringify(feedback)
                                            conn.send(send_str)
                                        }
                                        else{
                                            joiners.appOptErr(info.openid, null, `${j_res}`, 'app.sessionKey.*.mysql.newComerTest.joiners.joinEvent', 'null', mData.value, 'null')
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
                                        joiners.joinEvent(first_store, conn, false, false, function(f_res){
                                            var send_str = JSON.stringify(feedback)
                                            conn.send(send_str)
                                        })
                                    }).then(function (err) {
                                        if(err){
                                            joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.session_random.shellFunc', 'null', mData.value, 'null')
                                            var feedback = {key: mData.key, value: 'sessionKey failed'}
                                            conn.send(JSON.stringify(feedback))
                                        }
                                    })
                                }
                            }).then(function(err){
                                if(err) {
                                    joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.mysql.newComerTest', 'null', mData.value, 'null')
                                    var feedback = {key: mData.key, value: 'sessionKey failed'}
                                    conn.send(JSON.stringify(feedback))
                                }
                            })
                        }
                        else{
                            var key_obj = JSON.parse(r_key)
                            joiners.joinEvent(key_obj, conn, true, true, function(j_res){
                                var feedback = {key: mData.key, value: j_res.value}
                                if(j_res.key == 'join success') {
                                    var send_str = JSON.stringify(feedback)
                                    conn.send(send_str)
                                }
                                else{
                                    joiners.appOptErr(info.openid, null, `${j_res.key}`, 'app.sessionKey.*.joiners.joinEvent', 'null', mData.value, 'null')
                                    feedback = {key: mData.key, value: 'sessionKey failed'}
                                    conn.send(JSON.stringify(feedback))
                                }
                            })
                        }
                    }).then(function(err){
                        if(err){
                            joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.redis.getCacheKey', 'null', mData.value, 'null')
                            var feedback = {key: mData.key, value: 'sessionKey failed'}
                            conn.send(JSON.stringify(feedback))
                        }
                    })
                }
                else {
                    joiners.appOptErr(mData.value, null, `${error}||${response.statusCode}`, 'app.sessionKey.request', 'null', mData.value, 'null')
                    conn.send(JSON.stringify({key: mData.key, value: '网站升级中...'}))
                }
            })
        }
        else if(mData.key == 'submitInfo'){
            joiners.updateConn(mData.value.localKey, conn)
            txgeo(mData.value.nest.location, function (c_res) {
                if(c_res != 'error') {
                    var detail_addr = c_res.split('市')[0] + '市'
                    mData.value.nest.address = detail_addr
                    redis.getCacheKey(`*${mData.value.localKey}*`).then(function (key) {
                        if (key.length) {
                            redis.getCacheValue(key).then(function (val) {
                                var cache_objs = JSON.parse(val)
                                cache_objs.push(mData.value.nest)
                                redis.storeCache(key, JSON.stringify(cache_objs)).then(function (res) {
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit received'}))
                                }).then(function (err) {
                                    if(err) {
                                        var js_key = JSON.parse(key)
                                        joiners.appOptErr(js_key.openid, null, err, 'app.submitInfo.redis.storeCache', 'people', mData.value, 'null')
                                        conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                    }
                                })
                            }).then(function(err){
                                if(err){
                                    var js_key = JSON.parse(key)
                                    joiners.appOptErr(js_key.openid, null, `${err}`, 'app.submitInfo.redis.getCacheValue', 'people', mData.value, 'null')
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                }
                            })
                        }
                        else{
                            joiners.appOptErr(null, mData.value.localKey, `${key.length}`, 'app.submitInfo.redis.getCacheKey', 'people', mData.value, 'null')
                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                        }
                    }).then(function(err){
                        if(err){
                            joiners.appOptErr(null, mData.value.localKey, `${err}`, 'app.submitInfo.redis.getCacheKey', 'people', mData.value, 'null')
                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                        }
                    })
                }
                else {
                    joiners.appOptErr(null, mData.value.localKey, `${c_res}`, 'app.submitInfo.txgeo', 'people', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                }
            })

        }
        else if(mData.key == 'getSubmits'){
            joiners.updateConn(mData.value.localKey, conn)
            var which_gate = mData.value.gate
            redis.getCacheKey(`*${mData.value.localKey}*`).then(function(key){
                redis.getCacheValue(key).then(function (cacheres) {
                    var chk_obj = JSON.parse(cacheres)
                    if (chk_obj.length) {
                        var send_array = []
                        for(var ech in chk_obj){
                            if(ech == 0)
                                continue
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
                }).then(function(err){
                    if(err) {
                        var js_key = JSON.parse(key)
                        joiners.appOptErr(js_key.openid, null, `${err}`, 'app.getSubmits.redis.getCacheValue', 'people', mData.value, 'null')
                        conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
                    }
                })
            }).then(function(err){
                if(err){
                    joiners.appOptErr(null, mData.value.localKey, `${err}`, 'app.getSubmits.redis.getCacheKey', 'people', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'getSubmits', value: 'failed getSubmits'}))
                }
            })
        }
        else if(mData.key == 'getImagesProps'){
            joiners.updateConn(mData.value.key, conn)
            mysql.select_picsprops(mData.value.time, mData.value.key).then(function(res_rows){
                conn.send(JSON.stringify({key: 'getImagesProps', value: {imgs_rows:res_rows, time:mData.value.time}}))
            }).then(function(res_err){
                if(res_err) {
                    joiners.appOptErr(null, mData.value.key, `${res_err}`, 'app.getImagesProps.mysql.select_picsprops', 'pictures', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'getImagesProps', value: 'failed getImagesProps'}))
                }
            })
        }
        else if(mData.key == 'payRequest'){
            joiners.updateConn(mData.value.localkey, conn)
            redis.getCacheKey(`*${mData.value.localkey}*`).then(function(key){
                var openid = JSON.parse(key).openid
                var pay_good = payprocess.helicalGoods(mData.value.type, mData.value.price)
                var local_order = payprocess.helicalOrder(mData.value.type, mData.value.stime)
                payprocess.options(openid, local_order, pay_good, mData.value.price, function(res_opt){
                    if(typeof(res_opt.value) !== 'string'){
                        request(res_opt.value.urlobj, function(error, response, body){
                            if (!error && response.statusCode == 200) {
                                var xmlparser = new xml2js.Parser({explicitArray : false, ignoreAttrs : true})
                                xmlparser.parseString(body, function (err, result) {
                                    var payid = result.xml.prepay_id
                                    var currentsecs = Math.round(mData.value.stime/1000)
                                    payprocess.genSecondsSign(payid, currentsecs, function(getsign){
                                        if(getsign) {
                                            conn.send(JSON.stringify({key: 'payRequest',
                                                value: {
                                                    timeStamp: currentsecs,
                                                    nonceStr: getsign.random,
                                                    package: payid,
                                                    paySign: getsign.sign
                                                }
                                            }))
                                        }
                                        else {
                                            joiners.appOptErr(openid, null, `${getsign}`, 'app.payRequest.*.payprocess.genSecondsSign', 'payment', mData.value, 'null')
                                            conn.send(JSON.stringify({key: 'payRequest', value: 'failed for payRequest sign random'}))
                                        }
                                    })
                                })
                            }
                            else {
                                joiners.appOptErr(openid, null, `${error}||${response.statusCode}`, 'app.payRequest.*.request', 'payment', mData.value, 'null')
                                conn.send(JSON.stringify({key: 'payRequest', value: 'failed for payRequest txrespond'}))
                            }
                        })
                    }
                    else {
                        joiners.appOptErr(openid, null, `${res_opt.value}`, 'app.payRequest.*.payprocess.options', 'payment', mData.value, 'null')
                        conn.send(JSON.stringify({key: 'payRequest', value: 'failed for payRequest'}))
                    }
                })
            }).then(function(err){
                if(err) {
                    joiners.appOptErr(null, mData.value.localkey, `${err}`, 'app.payRequest.redis.getCacheKey', 'payment', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'payRequest', value: 'failed for payRequest'}))
                }
            })
        }
        else if(mData.key == 'searchPeople') {
            joiners.updateConn(mData.value.localKey, conn)
            var lat = mData.value.latitude, lng = mData.value.longitude, scope = mData.value.scope
            var chk_local = {latitude: mData.value.latitude, longitude: mData.value.longitude}
            txgeo(chk_local, function(res){
                if(res != 'error') {
                    var detail_addr = res.split('市')[0] + '市'
                    redis.searchScope(`*${mData.value.localKey}*`, mData.value.latitude, mData.value.longitude, mData.value.old_scope, mData.value.scope, {
                        city: detail_addr,
                        gate: mData.value.gate
                    }).then(function (res) {
                        if (res.length) {
                            if (res.length > 60)
                                conn.send(JSON.stringify({key: 'searchPeople', value: 'so large'}))
                            else
                                conn.send(JSON.stringify({key: 'searchPeople', value: res}))
                        }
                        else
                            conn.send(JSON.stringify({key: 'searchPeople', value: 'no person now'}))
                    }).then(function(err){
                        if(err){
                            joiners.appOptErr(null, mData.value.localKey, `${err}`, 'app.searchPeople.*.redis.searchScope', 'people', mData.value, 'null')
                            conn.send(JSON.stringify({key: 'searchPeople', value: 'searchPeople failed'}))
                        }
                    })
                }
                else{
                    joiners.appOptErr(null, mData.value.localKey, `${res}`, 'app.searchPeople.txgeo', 'people', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'searchPeople', value: 'searchPeople failed'}))
                }
            })
        }
        else if(mData.key == 'delSubmits') {
            if(mData.value == 'I am leaving')
                joiners.leaveEvent(conn)
        }
        else if(mData.key == 'closing') {
            if(mData.value == 'I am leaving')
                joiners.leaveEvent(conn)
        }
    })
    conn.on('error', function() {
        joiners.appOptErr(null, null, Array.prototype.join.call(arguments, ", "), 'app.conn.on(error)', 'all_table', 'null', 'null')
    })
    conn.on('close', function() {
        var key_id = joiners.findEvent(conn)
        if(key_id) {
            redis.getCacheValue(JSON.stringify(key_id)).then(function (cacheres) {
                var cache_objs = JSON.parse(cacheres)
                if (cache_objs.length > 0) {
                    mysql.text_exec(key).then(function (result) {
                        if (result.length) {
                            if (result != cacheres)
                                mysql.update_exec(key, cacheres).then(function (res) {}).then(function(err){
                                    if(err)
                                        joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.mysql.update_exec', 'people', result, cacheres)
                                })
                            console.log('mysql update')
                        }
                        else {
                            mysql.insert_exec(key, cacheres).then(function () {}).then(function(err){
                                if(err)
                                    joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.mysql.insert_exec', 'people', 'null', cacheres)
                            })
                            console.log('mysql insert')
                        }
                    })
                }
            }).then(function(err){
                if(err)
                    joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).redis.getCacheValue', 'people', 'null', 'null')
            })
        }
        else
            joiners.appOptErr(null, null, `${key_id}`, 'app.conn.on(close).joiners.findEvent', 'people', 'null', 'null')
    })
})
