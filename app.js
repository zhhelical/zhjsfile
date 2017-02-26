//app.js
"use strict"

const ws = require('ws')
const xml2js = require('xml2js')
const request = require('request')
const mysql = require('../../databases/mysqldata.js')
const redis = require('../../databases/rediscache.js')
const co = require('co')
const master = require('./service/master.js')
const joiners = require('./service/clients.js')
const txgeo = require('./service/txgeo.js')
const payprocess = require('./service/payunits.js')
var session_random = require('./service/shell.js')
var https_server = require('./httpssrv.js')
var wss_server = new ws.Server({server: https_server})
wss_server.setMaxListeners(100)
wss_server.on('connection', function(conn){
    conn.on('message', function(message) {
        var mData = JSON.parse(message)
        if(mData.key == 'sessionKey'){
            if (!redis.redis_srv)
                redis.construct()
            mysql.newComerTest(`${mData.value}`).then(function(sql_key) {
                if(sql_key.length) {
                    var key_obj = JSON.parse(sql_key)
                    mysql.loggersSave(key_obj.openid)
                    var feedback = {key: mData.key, value: key_obj.localKey}
                    redis.getCacheKey(`${sql_key}`).then(function (r_key) {
                        if (!r_key.length) {
                            joiners.joinEvent(key_obj, conn, true, false, function (j_res) {
                                if (j_res.key == 'join success') {
                                    if(j_res.value != key_obj.localKey)
                                        feedback.value = j_res.value
                                    var send_str = JSON.stringify(feedback)
                                    conn.send(send_str)
                                }
                                else {
                                    joiners.appOptErr(key_obj.openid, key_obj.localKey, `${j_res}`, 'app.sessionKey.mysql.newComerTest.redis.getCacheKey.joiners.joinEvent_cache_false', 'null', mData.value, 'null')
                                    feedback = {key: mData.key, value: 'sessionKey failed'}
                                    conn.send(JSON.stringify(feedback))
                                }
                            })
                        }
                        else {
                            joiners.joinEvent(key_obj, conn, true, true, function (j_res) {
                                if (j_res.key == 'join success') {
                                    if(j_res.value != key_obj.localKey)
                                        feedback.value = j_res.value
                                    conn.send(JSON.stringify(feedback))
                                }
                                else {
                                    joiners.appOptErr(key_obj.openid, key_obj.localKey, `${j_res}`, 'app.sessionKey.mysql.newComerTest.redis.getCacheKey.joiners.joinEvent_cache_true', 'null', mData.value, 'null')
                                    feedback = {key: mData.key, value: 'sessionKey failed'}
                                    conn.send(JSON.stringify(feedback))
                                }
                            })
                        }
                    }).then(function (err) {
                        if (err) {
                            joiners.appOptErr(key_obj.openid, null, `${err}`, 'app.sessionKey.*.redis.getCacheKey', 'null', mData.value, 'null')
                            var feedback = {key: mData.key, value: 'sessionKey failed'}
                            conn.send(JSON.stringify(feedback))
                        }
                    })
                }
                else{
                    var double_leap = '//'
                    var options = {
                        url: `https:${double_leap}api.weixin.qq.com/sns/jscode2session?appid=wxf9a75ea1c3517fbe&secret=9aceb733968d171ed70207f87c5dcb9e&js_code=${mData.value}&grant_type=authorization_code`
                    }
                    request(options, function(error, response, body){
                        if (!error && response.statusCode == 200) {
                            var info = JSON.parse(body)
                            mysql.loggersSave(info.openid)
                            mysql.newComerTest(`${info.openid}`).then(function(sql_key) {
                                if(sql_key.length) {
                                    var key_obj = JSON.parse(sql_key)
                                    var feedback = {key: mData.key, value: key_obj.localKey}
                                    redis.getCacheKey(`${sql_key}`).then(function (r_key) {//here should be not usable because redis has cleaned user
                                        if (!r_key.length) {
                                            joiners.joinEvent(key_obj, conn, true, false, function (j_res) {
                                                if (j_res.key == 'join success') {
                                                    if(j_res.value != key_obj.localKey)
                                                        feedback.value = j_res.value
                                                    conn.send(JSON.stringify(feedback))
                                                }
                                                else {
                                                    joiners.appOptErr(key_obj.openid, key_obj.localKey, `${j_res}`, 'app.sessionKey.*.mysql.newComerTest.redis.getCacheKey.joiners.joinEvent_cache_false', 'null', mData.value, 'null')
                                                    feedback = {key: mData.key, value: 'sessionKey failed'}
                                                    conn.send(JSON.stringify(feedback))
                                                }
                                            })
                                        }
                                        else {
                                            joiners.joinEvent(key_obj, conn, true, true, function (j_res) {
                                                if (j_res.key == 'join success') {
                                                    if(j_res.value != key_obj.localKey)
                                                        feedback.value = j_res.value
                                                    conn.send(JSON.stringify(feedback))
                                                }
                                                else {
                                                    joiners.appOptErr(key_obj.openid, key_obj.localKey, `${j_res}`, 'app.sessionKey.*.request.*.redis.getCacheKey.joiners.joinEvent_cache_true', 'null', mData.value, 'null')
                                                    feedback.value = 'sessionKey failed'
                                                    conn.send(JSON.stringify(feedback))
                                                }
                                            })
                                        }
                                    }).then(function (err) {
                                        if (err) {
                                            joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.request.*.redis.getCacheKey', 'null', mData.value, 'null')
                                            feedback.value = 'sessionKey failed'
                                            conn.send(JSON.stringify(feedback))
                                        }
                                    })
                                }
                                else{
                                    var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
                                    session_random.shellFunc(sh_order).then(function (result) {
                                        var feedback = {key: mData.key, value: result}
                                        var in_person = {localKey:result, openid:info.openid, session_key:info.session_key, expires_in:info.expires_in}
                                        joiners.joinEvent(in_person, conn, false, false, function (f_res) {
                                            var send_str = JSON.stringify(feedback)
                                            conn.send(send_str)
                                        })
                                    }).then(function (err) {
                                        if (err) {
                                            joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.session_random.shellFunc', 'null', mData.value, 'null')
                                            var feedback = {key: mData.key, value: 'sessionKey failed'}
                                            conn.send(JSON.stringify(feedback))
                                        }
                                    })
                                }
                            }).then(function (err) {
                                if (err) {
                                    joiners.appOptErr(info.openid, null, `${err}`, 'app.sessionKey.*.request.mysql.newComerTest', 'null', mData.value, 'null')
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
            }).then(function (err) {
                if (err) {
                    joiners.appOptErr(mData.value, null, `${err}`, 'app.sessionKey.mysql.newComerTest', 'null', mData.value, 'null')
                    var feedback = {key: mData.key, value: 'sessionKey failed'}
                    conn.send(JSON.stringify(feedback))
                }
            })
        }
        else if(mData.key == 'submitInfo'){
            joiners.updateConn(mData.value.localKey, conn)
            txgeo(mData.value.nest.location, function (c_res) {
                if(c_res != 'error') {
                    var detail_addr = c_res.split('市')[0] + '市'
                    mData.value.nest.address = detail_addr
                    redis.getCacheKey(`*${mData.value.localKey}*`).then(function (c_key) {
                        if (c_key.length) {
                            redis.getCacheValue(c_key).then(function (val) {
                                if(val) {
                                    var cache_objs = eval('('+val+')')
                                    cache_objs.push(mData.value.nest)
                                    redis.storeCache(c_key, JSON.stringify(cache_objs)).then(function (res) {
                                        conn.send(JSON.stringify({key: 'submitInfo', value: 'submit received'}))
                                    }).then(function (err) {
                                        if (err) {
                                            var js_key = JSON.parse(c_key)
                                            joiners.appOptErr(js_key.openid, null, err, 'app.submitInfo.redis.storeCache', 'people', mData.value, 'null')
                                            conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                        }
                                    })
                                }
                            }).then(function(err){
                                if(err){
                                    var js_key = JSON.parse(c_key)
                                    joiners.appOptErr(js_key.openid, null, `${err}`, 'app.submitInfo.redis.getCacheValue', 'people', mData.value, 'null')
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                }
                            })
                        }
                        else{
                            var new_keys = joiners.findByKey(mData.value.localKey).key
                            var first_store = {localKey:mData.value.localKey, openid:new_keys.openid}
                            var first_value = {session_key:new_keys.session_key, expires_in:new_keys.expires_in}
                            var first_array = []
                            first_array.push(first_value)
                            first_array.push(mData.value.nest)
                            redis.storeCache(JSON.stringify(first_store), JSON.stringify(first_array)).then(function (res) {
                                conn.send(JSON.stringify({key: 'submitInfo', value: 'submit received'}))
                            }).then(function (err) {
                                if(err) {
                                    joiners.appOptErr(new_openid, null, err, 'app.submitInfo.redis.storeCache', 'people', 'null', mData.value)
                                    conn.send(JSON.stringify({key: 'submitInfo', value: 'submit failed'}))
                                }
                            })
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
            var chk_keys = joiners.findByKey(mData.value.localKey).key
            var send_array = []
            redis.getCacheValue(JSON.stringify(chk_keys)).then(function (cacheres) {
                if(cacheres) {
                    var chk_obj = eval('('+cacheres+')')
                    for (var ech in chk_obj) {
                        if (ech == 0)
                            continue
                        if (which_gate == chk_obj[ech].content.add_key)
                            send_array.push(chk_obj[ech])
                    }
                    if(send_array.length)
                        conn.send(JSON.stringify({key: 'getSubmits', value: send_array}))
                    else
                        conn.send(JSON.stringify({key: 'getSubmits', value: 'no submits now'}))
                }
                else
                    conn.send(JSON.stringify({key: 'getSubmits', value: 'no submits now'}))
            }).then(function (err) {
                if (err) {
                    joiners.appOptErr(chk_keys.openid, mData.value.localKey, `${err}`, 'app.getSubmits.redis.getCacheValue', 'people', mData.value, 'null')
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
            var openid = joiners.findByKey(mData.value.localkey).key.openid
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
        }
        else if(mData.key == 'searchPeople') {
            joiners.updateConn(mData.value.localKey, conn)
            var chk_id = joiners.findByKey(mData.value.localKey).key.openid
            if(chk_id == master.id_master)
                master.getSuitableSubmits(mData.value.from).then(function(g_res){
                    conn.send(JSON.stringify({key: mData.key, value:{recs:g_res, master:true}}))
                })
            else {
                var lat = mData.value.latitude, lng = mData.value.longitude, scope = mData.value.scope
                var chk_local = {latitude: mData.value.latitude, longitude: mData.value.longitude}
                txgeo(chk_local, function (res) {
                    if (res != 'error') {
                        var detail_addr = res.split('市')[0] + '市'
                        redis.searchScope(mData.value.latitude, mData.value.longitude, mData.value.old_scope, mData.value.scope, {city: detail_addr, gate: mData.value.gate}).then(function (res) {
                            if (res.length) {
                                if (res.length > 60)
                                    conn.send(JSON.stringify({key: 'searchPeople', value: 'so large'}))
                                else
                                    conn.send(JSON.stringify({key: 'searchPeople', value: res}))
                            }
                            else
                                conn.send(JSON.stringify({key: 'searchPeople', value: 'no person now'}))
                        }).then(function (err) {
                            if (err) {
                                joiners.appOptErr(null, mData.value.localKey, `${err}`, 'app.searchPeople.*.redis.searchScope', 'people', mData.value, 'null')
                                conn.send(JSON.stringify({key: 'searchPeople', value: 'searchPeople failed'}))
                            }
                        })
                    }
                    else {
                        joiners.appOptErr(null, mData.value.localKey, `${res}`, 'app.searchPeople.txgeo', 'people', mData.value, 'null')
                        conn.send(JSON.stringify({key: 'searchPeople', value: 'searchPeople failed'}))
                    }
                })
            }
        }
        else if(mData.key == 'delSubmits') {
            if(mData.value.hasOwnProperty('master')){
                mysql.masterDelete(mData.value.time, JSON.stringify(mData.value.local), mData.value.addr).then(function(s_res){
                    if(s_res.length) {
                        redis.storeCache(s_res[0], s_res[1]).then(function (res) {
                            conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits finished'}))
                        }).then(function (err) {
                            if (err) {
                                var js_key = JSON.parse(s_res[0])
                                joiners.appOptErr(js_key.openid, js_key.localKey, err, 'app.delSubmits.*.redis.storeCache', 'people', 'null', s_res[1])
                                conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits failed'}))
                            }
                        })
                    }
                    else
                        conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits finished'}))
                }).then(function(err){
                    if (err) {
                        conn.send(JSON.stringify({key: 'delSubmits', value: 'failed'}))
                        joiners.appOptErr(null, null, `${err}`, 'app.delSubmits.mysql.masterDelete', 'people', 'null', JSON.stringify(mData.value))
                    }
                })
            }
            else {
                joiners.updateConn(mData.value.localKey, conn)
                var key = JSON.stringify(joiners.findByKey(mData.value.localKey).key)
                redis.getCacheValue(key).then(function (val) {
                    if (val) {
                        var cache_objs = eval('('+val+')')
                        var old_caches = cache_objs.slice(0)
                        cache_objs.pop()
                        redis.storeCache(key, JSON.stringify(cache_objs)).then(function (res) {
                            conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits finished'}))
                        }).then(function (err) {
                            if (err) {
                                var js_key = JSON.parse(key)
                                joiners.appOptErr(js_key.openid, null, err, 'app.delSubmits.redis.storeCache', 'people', JSON.stringify(old_caches), JSON.stringify(cache_objs))
                                conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits failed'}))
                            }
                        })
                    }
                }).then(function (err) {
                    if (err) {
                        var js_key = JSON.parse(key)
                        joiners.appOptErr(js_key.openid, null, `${err}`, 'app.delSubmits.redis.getCacheValue', 'people', mData.value, 'null')
                        conn.send(JSON.stringify({key: 'delSubmits', value: 'delSubmits failed'}))
                    }
                })
            }
        }
        else if(mData.key == 'delUnpayeds') {
            joiners.updateConn(mData.value.localKey, conn)
            var delsends = mData.value.unpayeds
            var tol_keys = joiners.findByKey(mData.value.localKey).key
            var key = JSON.stringify(tol_keys)
            redis.getCacheValue(key).then(function (val) {
                if(val) {
                    var cache_objs = eval('('+val+')')
                    for(var ti in delsends) {
                        for (var oi in cache_objs) {
                            if (oi == 0)
                                continue
                            if (delsends[ti]==cache_objs[oi].time) {
                                cache_objs.splice(oi, 1)
                                break
                            }
                        }
                    }
                    redis.storeCache(key, JSON.stringify(cache_objs)).then(function (res) {
                        conn.send(JSON.stringify({key: 'delUnpayeds', value: 'delUnpayeds finished'}))
                    }).then(function (err) {
                        if (err) {
                            joiners.appOptErr(js_key.openid, null, err, 'app.delUnpayeds.redis.storeCache', 'people', JSON.stringify(cache_objs), JSON.stringify(d_time))
                            conn.send(JSON.stringify({key: 'delUnpayeds', value: 'delUnpayeds failed'}))
                        }
                    })
                    joiners.asynDelPics(delsends, tol_keys.openid)
                }
            }).then(function(err){
                if(err){
                    joiners.appOptErr(tol_keys.openid, mData.value.localKey, `${err}`, 'app.delUnpayeds.redis.getCacheValue', 'people', mData.value, 'null')
                    conn.send(JSON.stringify({key: 'delUnpayeds', value: 'delUnpayeds failed'}))
                }
            })
        }
        else if(mData.key == 'submitMsgs'){
            var openid = joiners.findByKey(mData.value.localKey).key.openid
            var msg_time = new Date().getTime()
            mysql.msgsSave(openid, mData.value.contents, msg_time, true).then(function(s_res){
                conn.send(JSON.stringify({key: 'submitMsgs', value: s_res}))
                joiners.msgAutoReply(openid, msg_time)
            }).then(function(err){
                if (err) {
                    conn.send(JSON.stringify({key: 'submitMsgs', value: 'failed'}))
                    joiners.appOptErr(openid, null, `${err}`, 'app.submitMsgs.mysql.msgsSave', 'messages', 'null', mData.value.contents)
                }
            })
        }
        else if(mData.key == 'getMsgs'){
            var openid = joiners.findByKey(mData.value.localKey).key.openid
            if(openid == master.id_master)
                master.getSuitableMsgs(mData.value.from).then(function(g_res){
                    conn.send(JSON.stringify({key: mData.key, value: g_res}))
                })
            else {
                mysql.msgsGetting(openid).then(function (s_res) {
                    if(s_res.length) {
                        var obj_arrs = eval('('+s_res.contents+')')
                        var obj_times = JSON.parse(s_res.time)
                        conn.send(JSON.stringify({key: 'getMsgs', value: obj_arrs, time: obj_times}))
                    }
                    else
                        conn.send(JSON.stringify({key: 'getMsgs', value: 'no msgs now'}))
                }).then(function (err) {
                    if (err) {
                        conn.send(JSON.stringify({key: 'getMsgs', value: 'failed'}))
                        joiners.appOptErr(openid, null, `${err}`, 'app.getMsgs.mysql.msgsGetting', 'messages', 'null', 'null')
                    }
                })
            }
        }
        else if(mData.key == 'closing') {
            if(mData.value.onoff == 'I am leaving')
                joiners.leaveEvent(mData.value.localKey)
        }
    })
    conn.on('error', function() {
        joiners.appOptErr(null, null, Array.prototype.join.call(arguments, ", "), 'app.conn.on(error)', 'all_table', 'null', 'null')
    })
    conn.on('close', function() {
        var key_id = joiners.findEvent(conn)
        if(key_id) {
            var ids_str = JSON.stringify({localKey:key_id.localKey, openid:key_id.openid})
            redis.getCacheValue(ids_str).then(function (cacheres) {
                if(cacheres) {
                    if(key_id.hasOwnProperty('expires_in')){
                        delete key_id.expires_in
                        delete key_id.session_key
                    }
                    var cache_objs = eval('('+cacheres+')')
                    if (cache_objs.length > 0) {
                        mysql.text_exec(ids_str).then(function (result) {
                            if (result.length) {
                                if (result != cacheres) {
                                    var sql_vals = eval('('+result+')')
                                    if (cache_objs.length < sql_vals.length) {
                                        var rec_sql = sql_vals.pop()
                                        if (sql_vals.length == 1) {
                                            mysql.deletePeopleRow(ids_str).then(function (s_res) {
                                            }).then(function (err) {
                                                if (err)
                                                    joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.mysql.deletePeopleRow', 'people', result, cacheres)
                                            })
                                            redis.deleteData(ids_str).then(function (rd_res) {
                                            }).then(function (err) {
                                                if (err)
                                                    joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.redis.deleteData', 'people', result, cacheres)
                                            })
                                        }
                                        var arr_time = [rec_sql]
                                        joiners.asynDelPics(arr_time, key_id.openid)
                                    }
                                    mysql.update_exec(ids_str, cacheres).then(function (res) {
                                        joiners.recordsExpired(ids_str)
                                        console.log('mysql update')
                                    }).then(function (err) {
                                        if (err)
                                            joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.mysql.update_exec', 'people', result, cacheres)
                                    })
                                }
                            }
                            else {
                                mysql.insert_exec(ids_str, cacheres).then(function () {
                                }).then(function (err) {
                                    if (err)
                                        joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).*.mysql.insert_exec', 'people', 'null', cacheres)
                                })
                                console.log('mysql insert')
                            }
                        })
                    }
                }
            }).then(function(err){
                if(err)
                    joiners.appOptErr(key_id.openid, null, `${err}`, 'app.conn.on(close).redis.getCacheValue', 'people', 'null', 'null')
            })
        }
    })
})
